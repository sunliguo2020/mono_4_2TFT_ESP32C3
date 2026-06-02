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
#include "setup_scr_screen_main_top_bar.h"

void setup_scr_screen_main_top_bar(lv_ui *ui)
{
    // ============================================================
    // 顶部栏 screen_main_cont_top_bar (y=4, 290x52)
    // 第一行 (y=0~25): WiFi图标(5,1) | 光照图标(135,2) | 人体存在(170,4) | 电池符号(221,4)
    // 第二行 (y=26~50): 光照值(135,28) | 风扇计数(170,28) | 电池百分比(221,28)
    // ============================================================
    //Write codes screen_main_cont_top_bar
    ui->screen_main_cont_top_bar = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_top_bar, 5, 4);
    lv_obj_set_size(ui->screen_main_cont_top_bar, 290, 52);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_top_bar, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_top_bar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont_top_bar, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_top_bar, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_top_bar, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_top_bar, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_top_bar, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_top_bar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_top_bar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_top_bar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_top_bar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_top_bar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_top_bar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ---- 第一行 (y=0~25) ----

    //Write codes screen_main_img_1 (WiFi图标)
    ui->screen_main_img_1 = lv_image_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_img_1, 5, 1);
    lv_obj_set_size(ui->screen_main_img_1, 33, 31);
    lv_obj_add_flag(ui->screen_main_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_1, &_wifi_down_I4_33x31);
    lv_image_set_pivot(ui->screen_main_img_1, 50,50);
    lv_image_set_rotation(ui->screen_main_img_1, 0);

    //Write style for screen_main_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_2 (光照图标)
    ui->screen_main_img_2 = lv_image_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_img_2, 135, 2);
    lv_obj_set_size(ui->screen_main_img_2, 29, 25);
    lv_obj_add_flag(ui->screen_main_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_2, &_light_val_I4_29x25);
    lv_image_set_pivot(ui->screen_main_img_2, 50,50);
    lv_image_set_rotation(ui->screen_main_img_2, 0);

    //Write style for screen_main_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_Human_Presence (人体存在图标)
    ui->screen_main_img_5 = lv_image_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_img_5, 170, 4);
    lv_obj_set_size(ui->screen_main_img_5, 36, 37);
    lv_obj_add_flag(ui->screen_main_img_5, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_5, &_Human_Presence_Sensor_RGB565A8_36x37);
    lv_image_set_pivot(ui->screen_main_img_5, 50,50);
    lv_image_set_rotation(ui->screen_main_img_5, 0);

    //Write style for screen_main_img_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_17 (电池符号)
    ui->screen_main_label_17 = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_17, 221, 4);
    lv_obj_set_size(ui->screen_main_label_17, 52, 22);
    lv_label_set_text(ui->screen_main_label_17, LV_SYMBOL_BATTERY_FULL);
    lv_label_set_long_mode(ui->screen_main_label_17, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_17, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_17, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_17, &lv_font_montserratMedium_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_17, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ---- 第二行 (y=26~50) ----

    //Write codes screen_main_label_wifi (WiFi文字)
    ui->screen_main_label_wifi = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_wifi, 5, 32);
    lv_obj_set_size(ui->screen_main_label_wifi, 33, 12);
    lv_label_set_text(ui->screen_main_label_wifi, "WiFi");
    lv_label_set_long_mode(ui->screen_main_label_wifi, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(ui->screen_main_label_wifi, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_wifi, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_light_val (光照值)
    ui->screen_main_label_light_val = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_light_val, 135, 28);
    lv_obj_set_size(ui->screen_main_label_light_val, 60, 14);
    lv_label_set_text(ui->screen_main_label_light_val, "0");
    lv_label_set_long_mode(ui->screen_main_label_light_val, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_light_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_light_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_light_val, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_light_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_light_val, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_bat_val (电池百分比)
    ui->screen_main_label_bat_val = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_bat_val, 221, 28);
    lv_obj_set_size(ui->screen_main_label_bat_val, 52, 14);
    lv_label_set_text(ui->screen_main_label_bat_val, "65535");
    lv_label_set_long_mode(ui->screen_main_label_bat_val, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_bat_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_bat_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_bat_val, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_bat_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_bat_val, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_bat_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

}
