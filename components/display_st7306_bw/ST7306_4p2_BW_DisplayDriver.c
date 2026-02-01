#include "ST7306_4p2_BW_DisplayDriver.h"

#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ST7306_SPI_DEFAULT_CLOCK_HZ (40 * 1000 * 1000)

#ifndef SPI_MAX_DMA_LEN
#define SPI_MAX_DMA_LEN 4096
#endif

#define ST7306_SPI_MAX_CHUNK SPI_MAX_DMA_LEN

#define ST7306_COL_START 0x05
#define ST7306_COL_END (ST7306_COL_START + (ST7306_4P2_BW_LCD_DATA_WIDTH / 3) - 1)
#define ST7306_ROW_START 0x00
#define ST7306_ROW_END (ST7306_4P2_BW_LCD_DATA_HEIGHT - 1)

struct st7306_4p2_bw_t {
    st7306_4p2_bw_config_t cfg;
    spi_device_handle_t spi;
    uint8_t *display_buffer;
    bool hpm_mode;
    bool lpm_mode;
    bool owns_bus;
};

#define ST7306_CHECK(expr)           \
    do {                             \
        esp_err_t _err = (expr);     \
        if (_err != ESP_OK) {        \
            return _err;             \
        }                            \
    } while (0)

static void st7306_delay_ms(uint32_t ms)
{
    if (ms == 0) {
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void st7306_gpio_init(const st7306_4p2_bw_config_t *cfg)
{
    gpio_reset_pin(cfg->pin_dc);
    gpio_set_direction(cfg->pin_dc, GPIO_MODE_OUTPUT);
    gpio_set_level(cfg->pin_dc, 0);

    gpio_reset_pin(cfg->pin_rst);
    gpio_set_direction(cfg->pin_rst, GPIO_MODE_OUTPUT);
    gpio_set_level(cfg->pin_rst, 1);

    if (!cfg->use_hw_cs && cfg->pin_cs >= 0) {
        gpio_reset_pin(cfg->pin_cs);
        gpio_set_direction(cfg->pin_cs, GPIO_MODE_OUTPUT);
        gpio_set_level(cfg->pin_cs, 1);
    }
}

static void st7306_hw_reset(const st7306_4p2_bw_t *dev)
{
    gpio_set_level(dev->cfg.pin_rst, 1);
    st7306_delay_ms(10);
    gpio_set_level(dev->cfg.pin_rst, 0);
    st7306_delay_ms(10);
    gpio_set_level(dev->cfg.pin_rst, 1);
    st7306_delay_ms(10);
}

static esp_err_t st7306_spi_tx(st7306_4p2_bw_t *dev, const uint8_t *data, size_t len, bool hold_cs)
{
    if (!dev || !data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    bool bus_acquired = false;
    if (hold_cs) {
        esp_err_t err = spi_device_acquire_bus(dev->spi, portMAX_DELAY);
        if (err != ESP_OK) {
            return err;
        }
        bus_acquired = true;
    }

    if (!dev->cfg.use_hw_cs && dev->cfg.pin_cs >= 0 && hold_cs) {
        gpio_set_level(dev->cfg.pin_cs, 0);
    }

    size_t offset = 0;
    while (offset < len) {
        size_t chunk = len - offset;
        if (chunk > ST7306_SPI_MAX_CHUNK) {
            chunk = ST7306_SPI_MAX_CHUNK;
        }

        spi_transaction_t t = {0};
        t.length = chunk * 8;
        t.tx_buffer = data + offset;

        if (dev->cfg.use_hw_cs && hold_cs && (offset + chunk) < len) {
            t.flags = SPI_TRANS_CS_KEEP_ACTIVE;
        }

        if (!dev->cfg.use_hw_cs && dev->cfg.pin_cs >= 0 && !hold_cs) {
            gpio_set_level(dev->cfg.pin_cs, 0);
        }

        esp_err_t err = spi_device_polling_transmit(dev->spi, &t);

        if (!dev->cfg.use_hw_cs && dev->cfg.pin_cs >= 0 && !hold_cs) {
            gpio_set_level(dev->cfg.pin_cs, 1);
        }

        if (err != ESP_OK) {
            if (!dev->cfg.use_hw_cs && dev->cfg.pin_cs >= 0 && hold_cs) {
                gpio_set_level(dev->cfg.pin_cs, 1);
            }
            if (bus_acquired) {
                spi_device_release_bus(dev->spi);
            }
            return err;
        }

        offset += chunk;
    }

    if (!dev->cfg.use_hw_cs && dev->cfg.pin_cs >= 0 && hold_cs) {
        gpio_set_level(dev->cfg.pin_cs, 1);
    }

    if (bus_acquired) {
        spi_device_release_bus(dev->spi);
    }

    return ESP_OK;
}

static esp_err_t st7306_write_register(st7306_4p2_bw_t *dev, uint8_t value)
{
    gpio_set_level(dev->cfg.pin_dc, 0);
    return st7306_spi_tx(dev, &value, 1, false);
}

static esp_err_t st7306_write_parameter(st7306_4p2_bw_t *dev, uint8_t value)
{
    gpio_set_level(dev->cfg.pin_dc, 1);
    return st7306_spi_tx(dev, &value, 1, false);
}

static esp_err_t st7306_address(st7306_4p2_bw_t *dev)
{
    ST7306_CHECK(st7306_write_register(dev, 0x2A));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_COL_START));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_COL_END));

    ST7306_CHECK(st7306_write_register(dev, 0x2B));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_ROW_START));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_ROW_END));

    ST7306_CHECK(st7306_write_register(dev, 0x2C));
    return ESP_OK;
}

static esp_err_t st7306_address_window(st7306_4p2_bw_t *dev, uint8_t col_start, uint8_t col_end,
                                       uint8_t row_start, uint8_t row_end)
{
    ST7306_CHECK(st7306_write_register(dev, 0x2A));
    ST7306_CHECK(st7306_write_parameter(dev, col_start));
    ST7306_CHECK(st7306_write_parameter(dev, col_end));

    ST7306_CHECK(st7306_write_register(dev, 0x2B));
    ST7306_CHECK(st7306_write_parameter(dev, row_start));
    ST7306_CHECK(st7306_write_parameter(dev, row_end));

    ST7306_CHECK(st7306_write_register(dev, 0x2C));
    return ESP_OK;
}

static esp_err_t st7306_write_data_rows(st7306_4p2_bw_t *dev, const uint8_t *base, uint32_t stride,
                                        uint32_t row_start, uint32_t row_end,
                                        uint32_t byte_start, uint32_t byte_end)
{
    if (!dev || !base || byte_end < byte_start || row_end < row_start) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = spi_device_acquire_bus(dev->spi, portMAX_DELAY);
    if (err != ESP_OK) {
        return err;
    }

    if (!dev->cfg.use_hw_cs && dev->cfg.pin_cs >= 0) {
        gpio_set_level(dev->cfg.pin_cs, 0);
    }

    gpio_set_level(dev->cfg.pin_dc, 1);

    uint32_t len = byte_end - byte_start + 1;
    for (uint32_t row = row_start; row <= row_end; ++row) {
        const uint8_t *ptr = base + (row * stride) + byte_start;

        spi_transaction_t t = {0};
        t.length = len * 8;
        t.tx_buffer = ptr;

        if (dev->cfg.use_hw_cs && row != row_end) {
            t.flags = SPI_TRANS_CS_KEEP_ACTIVE;
        }

        err = spi_device_polling_transmit(dev->spi, &t);
        if (err != ESP_OK) {
            break;
        }
    }

    if (!dev->cfg.use_hw_cs && dev->cfg.pin_cs >= 0) {
        gpio_set_level(dev->cfg.pin_cs, 1);
    }

    spi_device_release_bus(dev->spi);
    return err;
}

static esp_err_t st7306_initial_sequence(st7306_4p2_bw_t *dev)
{
    st7306_hw_reset(dev);

    ST7306_CHECK(st7306_write_register(dev, 0xD6));
    ST7306_CHECK(st7306_write_parameter(dev, 0x17));
    ST7306_CHECK(st7306_write_parameter(dev, 0x02));

    ST7306_CHECK(st7306_write_register(dev, 0xD1));
    ST7306_CHECK(st7306_write_parameter(dev, 0x01));

    ST7306_CHECK(st7306_write_register(dev, 0xC0));
    ST7306_CHECK(st7306_write_parameter(dev, 0x12));
    ST7306_CHECK(st7306_write_parameter(dev, 0x0A));

    ST7306_CHECK(st7306_write_register(dev, 0xC1));
    ST7306_CHECK(st7306_write_parameter(dev, 115));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3E));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3C));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3C));

    ST7306_CHECK(st7306_write_register(dev, 0xC2));
    ST7306_CHECK(st7306_write_parameter(dev, 0x00));
    ST7306_CHECK(st7306_write_parameter(dev, 0x21));
    ST7306_CHECK(st7306_write_parameter(dev, 0x23));
    ST7306_CHECK(st7306_write_parameter(dev, 0x23));

    ST7306_CHECK(st7306_write_register(dev, 0xC4));
    ST7306_CHECK(st7306_write_parameter(dev, 50));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5C));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5A));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5A));

    ST7306_CHECK(st7306_write_register(dev, 0xC5));
    ST7306_CHECK(st7306_write_parameter(dev, 50));
    ST7306_CHECK(st7306_write_parameter(dev, 0x35));
    ST7306_CHECK(st7306_write_parameter(dev, 0x37));
    ST7306_CHECK(st7306_write_parameter(dev, 0x37));

    ST7306_CHECK(st7306_write_register(dev, 0xD8));
    ST7306_CHECK(st7306_write_parameter(dev, 0xA6));
    ST7306_CHECK(st7306_write_parameter(dev, 0xE9));
    ST7306_CHECK(st7306_write_register(dev, 0xB2));
    ST7306_CHECK(st7306_write_parameter(dev, 0x12));

    ST7306_CHECK(st7306_write_register(dev, 0xB3));
    ST7306_CHECK(st7306_write_parameter(dev, 0xE5));
    ST7306_CHECK(st7306_write_parameter(dev, 0xF6));
    ST7306_CHECK(st7306_write_parameter(dev, 0x17));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x71));

    ST7306_CHECK(st7306_write_register(dev, 0xB4));
    ST7306_CHECK(st7306_write_parameter(dev, 0x05));
    ST7306_CHECK(st7306_write_parameter(dev, 0x46));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x77));
    ST7306_CHECK(st7306_write_parameter(dev, 0x76));
    ST7306_CHECK(st7306_write_parameter(dev, 0x45));

    ST7306_CHECK(st7306_write_register(dev, 0x62));
    ST7306_CHECK(st7306_write_parameter(dev, 0x32));
    ST7306_CHECK(st7306_write_parameter(dev, 0x03));
    ST7306_CHECK(st7306_write_parameter(dev, 0x1F));

    ST7306_CHECK(st7306_write_register(dev, 0xB7));
    ST7306_CHECK(st7306_write_parameter(dev, 0x13));

    ST7306_CHECK(st7306_write_register(dev, 0xB0));
    ST7306_CHECK(st7306_write_parameter(dev, 0x64));

    ST7306_CHECK(st7306_write_register(dev, 0x11));
    st7306_delay_ms(120);

    ST7306_CHECK(st7306_write_register(dev, 0xC9));
    ST7306_CHECK(st7306_write_parameter(dev, 0x00));

    ST7306_CHECK(st7306_write_register(dev, 0x36));
    ST7306_CHECK(st7306_write_parameter(dev, 0x48));

    ST7306_CHECK(st7306_write_register(dev, 0x3A));
    ST7306_CHECK(st7306_write_parameter(dev, 0x11));

    ST7306_CHECK(st7306_write_register(dev, 0xB9));
    ST7306_CHECK(st7306_write_parameter(dev, 0x20));

    ST7306_CHECK(st7306_write_register(dev, 0xB8));
    ST7306_CHECK(st7306_write_parameter(dev, 0x29));

    ST7306_CHECK(st7306_write_register(dev, 0x2A));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_COL_START));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_COL_END));
    ST7306_CHECK(st7306_write_register(dev, 0x2B));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_ROW_START));
    ST7306_CHECK(st7306_write_parameter(dev, ST7306_ROW_END));

    ST7306_CHECK(st7306_write_register(dev, 0x35));
    ST7306_CHECK(st7306_write_parameter(dev, 0x00));

    ST7306_CHECK(st7306_write_register(dev, 0xD0));
    ST7306_CHECK(st7306_write_parameter(dev, 0xFF));

    ST7306_CHECK(st7306_write_register(dev, 0x38));

    dev->hpm_mode = true;
    dev->lpm_mode = false;

    ST7306_CHECK(st7306_write_register(dev, 0x29));

    ST7306_CHECK(st7306_write_register(dev, 0x20));

    ST7306_CHECK(st7306_write_register(dev, 0xBB));
    ST7306_CHECK(st7306_write_parameter(dev, 0x4F));

    return ESP_OK;
}

esp_err_t st7306_4p2_bw_init(st7306_4p2_bw_t **out, const st7306_4p2_bw_config_t *cfg)
{
    if (!out || !cfg) {
        return ESP_ERR_INVALID_ARG;
    }

    st7306_4p2_bw_t *dev = calloc(1, sizeof(st7306_4p2_bw_t));
    if (!dev) {
        return ESP_ERR_NO_MEM;
    }

    dev->cfg = *cfg;

    st7306_gpio_init(cfg);

    spi_bus_config_t buscfg = {
        .sclk_io_num = cfg->pin_sclk,
        .mosi_io_num = cfg->pin_mosi,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ST7306_SPI_MAX_CHUNK,
    };

    esp_err_t err = spi_bus_initialize(cfg->spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if (err == ESP_OK) {
        dev->owns_bus = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        free(dev);
        return err;
    }

    int clock_hz = cfg->spi_clock_hz > 0 ? cfg->spi_clock_hz : ST7306_SPI_DEFAULT_CLOCK_HZ;

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = clock_hz,
        .mode = 0,
        .spics_io_num = cfg->use_hw_cs ? cfg->pin_cs : -1,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };

    err = spi_bus_add_device(cfg->spi_host, &devcfg, &dev->spi);
    if (err != ESP_OK) {
        if (dev->owns_bus) {
            spi_bus_free(cfg->spi_host);
        }
        free(dev);
        return err;
    }

    dev->display_buffer = heap_caps_malloc(ST7306_4P2_BW_BUFFER_LEN, MALLOC_CAP_DMA);
    if (!dev->display_buffer) {
        spi_bus_remove_device(dev->spi);
        if (dev->owns_bus) {
            spi_bus_free(cfg->spi_host);
        }
        free(dev);
        return ESP_ERR_NO_MEM;
    }

    memset(dev->display_buffer, 0x00, ST7306_4P2_BW_BUFFER_LEN);

    err = st7306_initial_sequence(dev);
    if (err != ESP_OK) {
        st7306_4p2_bw_deinit(dev);
        return err;
    }

    *out = dev;
    return ESP_OK;
}

void st7306_4p2_bw_deinit(st7306_4p2_bw_t *dev)
{
    if (!dev) {
        return;
    }

    if (dev->display_buffer) {
        heap_caps_free(dev->display_buffer);
    }

    if (dev->spi) {
        spi_bus_remove_device(dev->spi);
    }

    if (dev->owns_bus) {
        spi_bus_free(dev->cfg.spi_host);
    }

    free(dev);
}

esp_err_t st7306_4p2_bw_fill(st7306_4p2_bw_t *dev, uint8_t data)
{
    if (!dev || !dev->display_buffer) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(dev->display_buffer, data, ST7306_4P2_BW_BUFFER_LEN);
    return ESP_OK;
}

esp_err_t st7306_4p2_bw_clear(st7306_4p2_bw_t *dev)
{
    return st7306_4p2_bw_fill(dev, 0x00);
}

esp_err_t st7306_4p2_bw_write_point(st7306_4p2_bw_t *dev, uint32_t x, uint32_t y, uint16_t data)
{
    if (!dev || !dev->display_buffer) {
        return ESP_ERR_INVALID_ARG;
    }

    if (x >= ST7306_4P2_BW_LCD_WIDTH || y >= ST7306_4P2_BW_LCD_HEIGHT) {
        return ESP_OK;
    }

    uint32_t real_x = x / 2;
    uint32_t real_y = y / 2;
    uint32_t write_byte_index = real_y * ST7306_4P2_BW_LCD_DATA_WIDTH + real_x;
    uint32_t one_two = (y % 2 == 0) ? 0 : 1;
    uint32_t line_bit_1 = (x % 2) * 4;
    uint32_t line_bit_0 = (x % 2) * 4 + 2;
    uint8_t write_bit_1 = (uint8_t)(7 - (line_bit_1 + one_two));
    uint8_t write_bit_0 = (uint8_t)(7 - (line_bit_0 + one_two));

    bool data_bit0 = (data & 0x0001) != 0;
    bool data_bit1 = (data & 0x0002) != 0;

    if (data_bit1) {
        dev->display_buffer[write_byte_index] |= (uint8_t)(1U << write_bit_1);
    } else {
        dev->display_buffer[write_byte_index] &= (uint8_t)~(1U << write_bit_1);
    }

    if (data_bit0) {
        dev->display_buffer[write_byte_index] |= (uint8_t)(1U << write_bit_0);
    } else {
        dev->display_buffer[write_byte_index] &= (uint8_t)~(1U << write_bit_0);
    }

    return ESP_OK;
}

esp_err_t st7306_4p2_bw_display(st7306_4p2_bw_t *dev)
{
    if (!dev || !dev->display_buffer) {
        return ESP_ERR_INVALID_ARG;
    }

    ST7306_CHECK(st7306_address(dev));
    gpio_set_level(dev->cfg.pin_dc, 1);
    return st7306_spi_tx(dev, dev->display_buffer, ST7306_4P2_BW_BUFFER_LEN, true);
}

esp_err_t st7306_4p2_bw_display_area(st7306_4p2_bw_t *dev, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
    if (!dev || !dev->display_buffer) {
        return ESP_ERR_INVALID_ARG;
    }

    if (x1 >= ST7306_4P2_BW_LCD_WIDTH || y1 >= ST7306_4P2_BW_LCD_HEIGHT) {
        return ESP_OK;
    }

    if (x2 >= ST7306_4P2_BW_LCD_WIDTH) {
        x2 = ST7306_4P2_BW_LCD_WIDTH - 1;
    }
    if (y2 >= ST7306_4P2_BW_LCD_HEIGHT) {
        y2 = ST7306_4P2_BW_LCD_HEIGHT - 1;
    }

    if (x1 > x2) {
        uint32_t tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        uint32_t tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    uint32_t real_x_start = x1 / 2;
    uint32_t real_x_end = x2 / 2;
    uint32_t real_y_start = y1 / 2;
    uint32_t real_y_end = y2 / 2;

    uint32_t byte_start = real_x_start;
    uint32_t byte_end = real_x_end;

    uint32_t col_byte_start = (byte_start / 3) * 3;
    uint32_t col_byte_end = (byte_end / 3) * 3 + 2;
    if (col_byte_end >= ST7306_4P2_BW_LCD_DATA_WIDTH) {
        col_byte_end = ST7306_4P2_BW_LCD_DATA_WIDTH - 1;
    }

    uint8_t col_start = (uint8_t)(ST7306_COL_START + (col_byte_start / 3));
    uint8_t col_end = (uint8_t)(ST7306_COL_START + (col_byte_end / 3));
    uint8_t row_start = (uint8_t)real_y_start;
    uint8_t row_end = (uint8_t)real_y_end;

    ST7306_CHECK(st7306_address_window(dev, col_start, col_end, row_start, row_end));
    return st7306_write_data_rows(dev,
                                  dev->display_buffer,
                                  ST7306_4P2_BW_LCD_DATA_WIDTH,
                                  real_y_start,
                                  real_y_end,
                                  col_byte_start,
                                  col_byte_end);
}

esp_err_t st7306_4p2_bw_low_power_mode(st7306_4p2_bw_t *dev)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->lpm_mode) {
        dev->hpm_mode = false;
        dev->lpm_mode = true;
        return ESP_OK;
    }

    dev->hpm_mode = false;
    dev->lpm_mode = true;

    ST7306_CHECK(st7306_write_register(dev, 0xC1));
    ST7306_CHECK(st7306_write_parameter(dev, 115));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3E));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3C));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3C));

    ST7306_CHECK(st7306_write_register(dev, 0xC2));
    ST7306_CHECK(st7306_write_parameter(dev, 0x00));
    ST7306_CHECK(st7306_write_parameter(dev, 0x21));
    ST7306_CHECK(st7306_write_parameter(dev, 0x23));
    ST7306_CHECK(st7306_write_parameter(dev, 0x23));

    ST7306_CHECK(st7306_write_register(dev, 0xC4));
    ST7306_CHECK(st7306_write_parameter(dev, 50));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5C));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5A));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5A));

    ST7306_CHECK(st7306_write_register(dev, 0xC5));
    ST7306_CHECK(st7306_write_parameter(dev, 50));
    ST7306_CHECK(st7306_write_parameter(dev, 0x35));
    ST7306_CHECK(st7306_write_parameter(dev, 0x37));
    ST7306_CHECK(st7306_write_parameter(dev, 0x37));

    ST7306_CHECK(st7306_write_register(dev, 0xC9));
    ST7306_CHECK(st7306_write_parameter(dev, 0x00));

    st7306_delay_ms(20);

    ST7306_CHECK(st7306_write_register(dev, 0x39));
    st7306_delay_ms(100);
    return ESP_OK;
}

esp_err_t st7306_4p2_bw_high_power_mode(st7306_4p2_bw_t *dev)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->hpm_mode) {
        dev->hpm_mode = true;
        dev->lpm_mode = false;
        return ESP_OK;
    }

    dev->hpm_mode = true;
    dev->lpm_mode = false;

    ST7306_CHECK(st7306_write_register(dev, 0x38));
    st7306_delay_ms(300);

    ST7306_CHECK(st7306_write_register(dev, 0xC1));
    ST7306_CHECK(st7306_write_parameter(dev, 115));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3E));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3C));
    ST7306_CHECK(st7306_write_parameter(dev, 0x3C));

    ST7306_CHECK(st7306_write_register(dev, 0xC2));
    ST7306_CHECK(st7306_write_parameter(dev, 0x00));
    ST7306_CHECK(st7306_write_parameter(dev, 0x21));
    ST7306_CHECK(st7306_write_parameter(dev, 0x23));
    ST7306_CHECK(st7306_write_parameter(dev, 0x23));

    ST7306_CHECK(st7306_write_register(dev, 0xC4));
    ST7306_CHECK(st7306_write_parameter(dev, 50));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5C));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5A));
    ST7306_CHECK(st7306_write_parameter(dev, 0x5A));

    ST7306_CHECK(st7306_write_register(dev, 0xC5));
    ST7306_CHECK(st7306_write_parameter(dev, 50));
    ST7306_CHECK(st7306_write_parameter(dev, 0x35));
    ST7306_CHECK(st7306_write_parameter(dev, 0x37));
    ST7306_CHECK(st7306_write_parameter(dev, 0x37));

    ST7306_CHECK(st7306_write_register(dev, 0xC9));
    ST7306_CHECK(st7306_write_parameter(dev, 0x00));

    st7306_delay_ms(20);
    return ESP_OK;
}

esp_err_t st7306_4p2_bw_display_on(st7306_4p2_bw_t *dev, bool enabled)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    if (enabled) {
        return st7306_write_register(dev, 0x29);
    }
    return st7306_write_register(dev, 0x28);
}

esp_err_t st7306_4p2_bw_display_sleep(st7306_4p2_bw_t *dev, bool enabled)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    if (enabled) {
        if (dev->lpm_mode) {
            ST7306_CHECK(st7306_write_register(dev, 0x38));
            st7306_delay_ms(300);
        }
        ST7306_CHECK(st7306_write_register(dev, 0x10));
        st7306_delay_ms(100);
        return ESP_OK;
    }

    ST7306_CHECK(st7306_write_register(dev, 0x11));
    st7306_delay_ms(100);
    return ESP_OK;
}

esp_err_t st7306_4p2_bw_display_inversion(st7306_4p2_bw_t *dev, bool enabled)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    if (enabled) {
        return st7306_write_register(dev, 0x21);
    }
    return st7306_write_register(dev, 0x20);
}

uint8_t *st7306_4p2_bw_buffer(st7306_4p2_bw_t *dev)
{
    if (!dev) {
        return NULL;
    }
    return dev->display_buffer;
}

size_t st7306_4p2_bw_buffer_len(st7306_4p2_bw_t *dev)
{
    if (!dev) {
        return 0;
    }
    return ST7306_4P2_BW_BUFFER_LEN;
}
