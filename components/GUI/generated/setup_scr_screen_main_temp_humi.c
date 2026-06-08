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
#include "setup_scr_screen_main_temp_humi.h"

void setup_scr_screen_main_temp_humi(lv_ui *ui)
{
    // ============================================================
    // 温湿度区域 screen_main_cont_temp_humi (x=5, y=178, 177x118)
    // 左右布局：左侧温度，右侧湿度
    // ============================================================
    ui->screen_main_cont_temp_humi = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_temp_humi, 5, 178);
    lv_obj_set_size(ui->screen_main_cont_temp_humi, 177, 118);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_temp_humi, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_border_width(ui->screen_main_cont_temp_humi, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_temp_humi, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_temp_humi, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_temp_humi, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_temp_humi, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_temp_humi, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_temp_humi, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_temp_humi, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_temp_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_temp_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_temp_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_temp_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_temp_humi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // -------- 温度（左侧）--------
    // 温度标签 - 竖排"温度"
    ui->screen_main_label_10 = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_10, 4, 4);
    lv_obj_set_size(ui->screen_main_label_10, 32, 50);
    lv_label_set_text(ui->screen_main_label_10, "温\n度");
    lv_label_set_long_mode(ui->screen_main_label_10, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_10, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_label_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_label_10, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_label_10, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_10, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_10, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_10, &lv_font_SourceHanSansSCBold_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_10, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_10, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 温度数值
    ui->screen_main_label_temp_val = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_temp_val, 38, 0);
    lv_obj_set_size(ui->screen_main_label_temp_val, 48, 56);
    lv_label_set_text(ui->screen_main_label_temp_val, "0.0");
    lv_label_set_long_mode(ui->screen_main_label_temp_val, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_temp_val, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_temp_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_temp_val, &lv_font_SMG_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_temp_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_temp_val, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // -------- 湿度（右侧）--------
    // 湿度标签 - 竖排"湿度"
    ui->screen_main_label_8 = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_8, 92, 4);
    lv_obj_set_size(ui->screen_main_label_8, 32, 50);
    lv_label_set_text(ui->screen_main_label_8, "湿\n度");
    lv_label_set_long_mode(ui->screen_main_label_8, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_8, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_label_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_label_8, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_label_8, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_8, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_8, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_8, &lv_font_SourceHanSansSCBold_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_8, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_8, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 湿度数值
    ui->screen_main_label_humi_val = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_humi_val, 126, 0);
    lv_obj_set_size(ui->screen_main_label_humi_val, 48, 56);
    lv_label_set_text(ui->screen_main_label_humi_val, "0.0");
    lv_label_set_long_mode(ui->screen_main_label_humi_val, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_humi_val, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_humi_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_humi_val, &lv_font_SMG_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_humi_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_humi_val, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
}