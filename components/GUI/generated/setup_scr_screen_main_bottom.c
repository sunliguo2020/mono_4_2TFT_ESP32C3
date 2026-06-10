/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include "gui_guider.h"
#include "setup_scr_screen_main_bottom.h"

// 天气字体声明
LV_FONT_DECLARE(lv_font_weather_20);
LV_FONT_DECLARE(lv_font_weather_24);

void setup_scr_screen_main_bottom(lv_ui *ui)
{
    // ============================================================
    // 底部区域 screen_main_cont_bottom (y=282, 290x118)
    // 天气数据全宽显示在屏幕最下面一�?
    // ============================================================
    //Write codes screen_main_cont_bottom
    ui->screen_main_cont_bottom = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_bottom, 5, 248);
    lv_obj_set_size(ui->screen_main_cont_bottom, 290, 158);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_bottom, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_bottom, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont_bottom, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_bottom, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_bottom, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_bottom, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_bottom, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_bottom, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_bottom, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_bottom, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    // ====== 今日天气 (左侧) ======
    // screen_main_label_5 - 左侧竖向标题 "今日天气"
    ui->screen_main_label_5 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_5, 3, 4);
    lv_obj_set_size(ui->screen_main_label_5, 20, 96);
    lv_label_set_text(ui->screen_main_label_5, "今\n日\n天\n气");
    lv_label_set_long_mode(ui->screen_main_label_5, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_5, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_5, &lv_font_SourceHanSansSCBold_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // screen_main_label_20 - 左侧今日温度 (SMG_53)
    ui->screen_main_label_20 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_20, 26, 28);
    lv_obj_set_size(ui->screen_main_label_20, 115, 58);
    lv_label_set_text(ui->screen_main_label_20, "16~28");
    lv_label_set_long_mode(ui->screen_main_label_20, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_20, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_20, &lv_font_SMG_53, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_20, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // screen_main_label_18 - 左侧℃符号
    ui->screen_main_label_18 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_18, 130, 32);
    lv_obj_set_size(ui->screen_main_label_18, 16, 30);
    lv_label_set_text(ui->screen_main_label_18, "℃");
    lv_label_set_long_mode(ui->screen_main_label_18, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_18, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_18, &lv_font_weather_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_18, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // screen_main_label_weather - 左侧今日天气描述 (日期+天气+风向风力)
    ui->screen_main_label_weather = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_weather, 26, 90);
    lv_obj_set_size(ui->screen_main_label_weather, 115, 28);
    lv_label_set_text(ui->screen_main_label_weather, "");
    lv_label_set_long_mode(ui->screen_main_label_weather, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_weather, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_weather, &lv_font_weather_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_weather, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_weather, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    // ====== 明日天气 (右侧) ======
    // screen_main_label_2 - 右侧竖向标题 "明日天气"
    ui->screen_main_label_2 = lv_label_create(ui->screen_main_cont_bottom);
    lv_label_set_text(ui->screen_main_label_2, "明\n日\n天\n气");
    lv_obj_set_pos(ui->screen_main_label_2, 148, 4);
    lv_obj_set_size(ui->screen_main_label_2, 20, 96);
    lv_label_set_long_mode(ui->screen_main_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_2, &lv_font_SourceHanSansSCBold_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    // screen_main_label_19 - 右侧明日温度 (SMG_40)
    ui->screen_main_label_19 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_19, 171, 28);
    lv_obj_set_size(ui->screen_main_label_19, 115, 58);
    lv_label_set_text(ui->screen_main_label_19, "");
    lv_label_set_long_mode(ui->screen_main_label_19, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_19, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_19, &lv_font_SMG_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_19, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

        // screen_main_label_21 - �Ҳ�����
    ui->screen_main_label_21 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_21, 275, 32);
    lv_obj_set_size(ui->screen_main_label_21, 16, 30);
    lv_label_set_text(ui->screen_main_label_21, "℃");
    lv_label_set_long_mode(ui->screen_main_label_21, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_21, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_21, &lv_font_weather_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_21, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_21, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // screen_main_label_weather_tmrw - 右侧明日天气描述 (日期+天气+风向风力)
    ui->screen_main_label_weather_tmrw = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_weather_tmrw, 171, 90);
    lv_obj_set_size(ui->screen_main_label_weather_tmrw, 115, 28);
    lv_label_set_text(ui->screen_main_label_weather_tmrw, "");
    lv_label_set_long_mode(ui->screen_main_label_weather_tmrw, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_weather_tmrw, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_weather_tmrw, &lv_font_weather_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_weather_tmrw, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_weather_tmrw, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // screen_main_label_run_hour - 不再使用
}
