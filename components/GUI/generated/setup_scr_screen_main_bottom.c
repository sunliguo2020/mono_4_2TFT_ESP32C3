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
LV_FONT_DECLARE(lv_font_SMG_40);

void setup_scr_screen_main_bottom(lv_ui *ui)
{
    // ============================================================
    // 底部总容器 290x158
    // ============================================================
    ui->screen_main_cont_bottom = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_bottom, 5, 248);
    lv_obj_set_size(ui->screen_main_cont_bottom, 290, 158);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_bottom, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(ui->screen_main_cont_bottom, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_bottom, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_bottom, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_bottom, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_bottom, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_bottom, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_bottom, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_bottom, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(ui->screen_main_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_bottom, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ===================== 第一行：今日天气（左对齐）=====================
    ui->screen_main_label_5 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_5, 10, 8);
    lv_obj_set_size(ui->screen_main_label_5, 280, 24);
    lv_label_set_text(ui->screen_main_label_5, "今日天气");
    lv_label_set_long_mode(ui->screen_main_label_5, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_5, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_5, &lv_font_weather_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_5, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ===================== 第二行：今日温度（居中）=====================
    ui->screen_main_label_20 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_20, 0, 32);
    lv_obj_set_size(ui->screen_main_label_20, 290, 40);
    lv_label_set_text(ui->screen_main_label_20, "16~28");
    lv_label_set_long_mode(ui->screen_main_label_20, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_20, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_20, &lv_font_SMG_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_20, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 今日温度 ℃ 符号
    ui->screen_main_label_18 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_18, 230, 38);
    lv_obj_set_size(ui->screen_main_label_18, 20, 30);
    lv_label_set_text(ui->screen_main_label_18, "℃");
    lv_label_set_long_mode(ui->screen_main_label_18, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_18, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_18, &lv_font_weather_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ===================== 第三行：明日天气（左对齐）=====================
    ui->screen_main_label_2 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_2, 10, 75);
    lv_obj_set_size(ui->screen_main_label_2, 280, 24);
    lv_label_set_text(ui->screen_main_label_2, "明日天气");
    lv_label_set_long_mode(ui->screen_main_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_2, &lv_font_weather_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ===================== 第四行：明日温度（居中）=====================
    ui->screen_main_label_19 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_19, 0, 98);
    lv_obj_set_size(ui->screen_main_label_19, 290, 40);
    lv_label_set_text(ui->screen_main_label_19, "");
    lv_label_set_long_mode(ui->screen_main_label_19, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_19, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_19, &lv_font_SMG_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_19, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 明日温度 ℃ 符号
    ui->screen_main_label_21 = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_21, 230, 104);
    lv_obj_set_size(ui->screen_main_label_21, 20, 30);
    lv_label_set_text(ui->screen_main_label_21, "℃");
    lv_label_set_long_mode(ui->screen_main_label_21, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_21, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_21, &lv_font_weather_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_21, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ===================== 第五行：明日预报（居中）=====================
    ui->screen_main_label_weather_tmrw = lv_label_create(ui->screen_main_cont_bottom);
    lv_obj_set_pos(ui->screen_main_label_weather_tmrw, 0, 130);
    lv_obj_set_size(ui->screen_main_label_weather_tmrw, 290, 24);
    lv_label_set_text(ui->screen_main_label_weather_tmrw, "");
    lv_label_set_long_mode(ui->screen_main_label_weather_tmrw, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_weather_tmrw, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_weather_tmrw, &lv_font_weather_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_weather_tmrw, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_weather_tmrw, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_weather_tmrw, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
}