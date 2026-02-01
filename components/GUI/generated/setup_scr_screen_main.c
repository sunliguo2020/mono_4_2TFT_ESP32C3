/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen_main(lv_ui *ui)
{
    //Write codes screen_main
    ui->screen_main = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_main, 300, 400);
    lv_obj_set_scrollbar_mode(ui->screen_main, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_cont_time
    ui->screen_main_cont_time = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_time, 5, 59);
    lv_obj_set_size(ui->screen_main_cont_time, 291, 129);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_time, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_time, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont_time, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_time, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_time, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_time, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_time, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_time, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_time, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_time, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_date
    ui->screen_main_label_date = lv_label_create(ui->screen_main_cont_time);
    lv_obj_set_pos(ui->screen_main_label_date, 0, 1);
    lv_obj_set_size(ui->screen_main_label_date, 233, 28);
    lv_label_set_text(ui->screen_main_label_date, "XXXX-XX-XX");
    lv_label_set_long_mode(ui->screen_main_label_date, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_date, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_date, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_date, &lv_font_SMG_32, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_date, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_date, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_cont_2
    ui->screen_main_cont_2 = lv_obj_create(ui->screen_main_cont_time);
    lv_obj_set_pos(ui->screen_main_cont_2, 3, 35);
    lv_obj_set_size(ui->screen_main_cont_2, 281, 88);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_2, LV_BORDER_SIDE_TOP, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_time_hour
    ui->screen_main_label_time_hour = lv_label_create(ui->screen_main_cont_2);
    lv_obj_set_pos(ui->screen_main_label_time_hour, -2, -1);
    lv_obj_set_size(ui->screen_main_label_time_hour, 134, 91);
    lv_label_set_text(ui->screen_main_label_time_hour, "--");
    lv_label_set_long_mode(ui->screen_main_label_time_hour, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_time_hour, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_time_hour, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_time_hour, &lv_font_SMG_90, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_time_hour, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_time_hour, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_time_hour, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_time_min
    ui->screen_main_label_time_min = lv_label_create(ui->screen_main_cont_2);
    lv_obj_set_pos(ui->screen_main_label_time_min, 148, -1);
    lv_obj_set_size(ui->screen_main_label_time_min, 139, 90);
    lv_label_set_text(ui->screen_main_label_time_min, "--");
    lv_label_set_long_mode(ui->screen_main_label_time_min, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_time_min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_time_min, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_time_min, &lv_font_SMG_90, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_time_min, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_time_min, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_time_min, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_1
    ui->screen_main_label_1 = lv_label_create(ui->screen_main_cont_2);
    lv_obj_set_pos(ui->screen_main_label_1, 125, -13);
    lv_obj_set_size(ui->screen_main_label_1, 22, 82);
    lv_label_set_text(ui->screen_main_label_1, ":");
    lv_label_set_long_mode(ui->screen_main_label_1, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_1, &lv_font_SMG_90, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_time_update
    ui->screen_main_label_time_update = lv_label_create(ui->screen_main_cont_time);
    lv_obj_set_pos(ui->screen_main_label_time_update, 241, 5);
    lv_obj_set_size(ui->screen_main_label_time_update, 30, 23);
    lv_label_set_text(ui->screen_main_label_time_update, "" LV_SYMBOL_LOOP " ");
    lv_label_set_long_mode(ui->screen_main_label_time_update, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_time_update, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_time_update, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_time_update, &lv_font_SourceHanSansSCBold_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_time_update, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_time_update, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_time_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_cont_pc_time
    ui->screen_main_cont_pc_time = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_pc_time, 5, 317);
    lv_obj_set_size(ui->screen_main_cont_pc_time, 292, 79);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_pc_time, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_pc_time, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont_pc_time, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_pc_time, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_pc_time, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_pc_time, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_pc_time, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_pc_time, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_pc_time, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_pc_time, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_pc_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_pc_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_pc_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_pc_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_pc_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_5
    ui->screen_main_label_5 = lv_label_create(ui->screen_main_cont_pc_time);
    lv_obj_set_pos(ui->screen_main_label_5, 11, 10);
    lv_obj_set_size(ui->screen_main_label_5, 54, 54);
    lv_label_set_text(ui->screen_main_label_5, "开机\n时间");
    lv_label_set_long_mode(ui->screen_main_label_5, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_5, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_5, &lv_font_SourceHanSansSCBold_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_5, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_cont_4
    ui->screen_main_cont_4 = lv_obj_create(ui->screen_main_cont_pc_time);
    lv_obj_set_pos(ui->screen_main_cont_4, 73, 5);
    lv_obj_set_size(ui->screen_main_cont_4, 208, 65);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_4, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont_4, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_4, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_4, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_4, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_4, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_4
    ui->screen_main_label_4 = lv_label_create(ui->screen_main_cont_4);
    lv_obj_set_pos(ui->screen_main_label_4, 0, 3);
    lv_obj_set_size(ui->screen_main_label_4, 89, 55);
    lv_label_set_text(ui->screen_main_label_4, "00");
    lv_label_set_long_mode(ui->screen_main_label_4, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_4, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_4, &lv_font_SMG_53, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_3
    ui->screen_main_label_3 = lv_label_create(ui->screen_main_cont_4);
    lv_obj_set_pos(ui->screen_main_label_3, 111, 0);
    lv_obj_set_size(ui->screen_main_label_3, 89, 55);
    lv_label_set_text(ui->screen_main_label_3, "00");
    lv_label_set_long_mode(ui->screen_main_label_3, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_3, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_3, &lv_font_SMG_53, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_2
    ui->screen_main_label_2 = lv_label_create(ui->screen_main_cont_4);
    lv_obj_set_pos(ui->screen_main_label_2, 93, -4);
    lv_obj_set_size(ui->screen_main_label_2, 16, 60);
    lv_label_set_text(ui->screen_main_label_2, ":");
    lv_label_set_long_mode(ui->screen_main_label_2, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_2, &lv_font_SMG_53, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_2, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

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

    //Write codes screen_main_img_1
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

    //Write codes screen_main_img_PC_statee
    ui->screen_main_img_PC_statee = lv_image_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_img_PC_statee, 86, 1);
    lv_obj_set_size(ui->screen_main_img_PC_statee, 39, 43);
    lv_obj_add_flag(ui->screen_main_img_PC_statee, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_PC_statee, &_out_line_I4_39x43);
    lv_image_set_pivot(ui->screen_main_img_PC_statee, 50,50);
    lv_image_set_rotation(ui->screen_main_img_PC_statee, 0);

    //Write style for screen_main_img_PC_statee, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_PC_statee, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_PC_statee, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_2
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

    //Write codes screen_main_label_bat_val
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

    //Write codes screen_main_img_bat
    ui->screen_main_img_bat = lv_image_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_img_bat, 233, 0);
    lv_obj_set_size(ui->screen_main_img_bat, 39, 28);
    lv_obj_add_flag(ui->screen_main_img_bat, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_main_img_bat, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_bat, &_battery_charge_line_I4_39x28);
    lv_image_set_pivot(ui->screen_main_img_bat, 50,50);
    lv_image_set_rotation(ui->screen_main_img_bat, 0);

    //Write style for screen_main_img_bat, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_bat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_bat, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_6
    ui->screen_main_label_6 = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_6, 4, 32);
    lv_obj_set_size(ui->screen_main_label_6, 39, 13);
    lv_label_set_text(ui->screen_main_label_6, "WiFi");
    lv_label_set_long_mode(ui->screen_main_label_6, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_6, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_6, &lv_font_montserratMedium_15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_light_val
    ui->screen_main_label_light_val = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_light_val, 121, 28);
    lv_obj_set_size(ui->screen_main_label_light_val, 61, 14);
    lv_label_set_text(ui->screen_main_label_light_val, "65535");
    lv_label_set_long_mode(ui->screen_main_label_light_val, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_light_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_light_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_light_val, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_light_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_light_val, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_light_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_5
    ui->screen_main_img_5 = lv_image_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_img_5, 178, 5);
    lv_obj_set_size(ui->screen_main_img_5, 36, 37);
    lv_obj_add_flag(ui->screen_main_img_5, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_5, &_Human_Presence_Sensor_RGB565A8_36x37);
    lv_image_set_pivot(ui->screen_main_img_5, 50,50);
    lv_image_set_rotation(ui->screen_main_img_5, 0);

    //Write style for screen_main_img_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_15
    ui->screen_main_label_15 = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_15, 193, 3);
    lv_obj_set_size(ui->screen_main_label_15, 39, 12);
    lv_label_set_text(ui->screen_main_label_15, "" LV_SYMBOL_CLOSE " ");
    lv_label_set_long_mode(ui->screen_main_label_15, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_15, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_15, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_15, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_15, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_15, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_keep_stu
    ui->screen_main_img_keep_stu = lv_image_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_img_keep_stu, 90, 7);
    lv_obj_set_size(ui->screen_main_img_keep_stu, 32, 34);
    lv_obj_add_flag(ui->screen_main_img_keep_stu, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_main_img_keep_stu, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_keep_stu, &_keep_stu_I4_32x34);
    lv_image_set_pivot(ui->screen_main_img_keep_stu, 50,50);
    lv_image_set_rotation(ui->screen_main_img_keep_stu, 0);

    //Write style for screen_main_img_keep_stu, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_keep_stu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_keep_stu, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_17
    ui->screen_main_label_17 = lv_label_create(ui->screen_main_cont_top_bar);
    lv_obj_set_pos(ui->screen_main_label_17, 231, 1);
    lv_obj_set_size(ui->screen_main_label_17, 42, 26);
    lv_label_set_text(ui->screen_main_label_17, "" LV_SYMBOL_BATTERY_1 " ");
    lv_label_set_long_mode(ui->screen_main_label_17, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_17, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_17, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_17, &lv_font_montserratMedium_29, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_17, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_cont_temp_humi
    ui->screen_main_cont_temp_humi = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_temp_humi, 5, 192);
    lv_obj_set_size(ui->screen_main_cont_temp_humi, 177, 118);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_temp_humi, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_temp_humi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
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

    //Write codes screen_main_label_8
    ui->screen_main_label_8 = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_8, 4, 59);
    lv_obj_set_size(ui->screen_main_label_8, 29, 49);
    lv_label_set_text(ui->screen_main_label_8, "湿度");
    lv_label_set_long_mode(ui->screen_main_label_8, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_8, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_8, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_label_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_label_8, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_label_8, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_8, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_8, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_8, &lv_font_SourceHanSansSCBold_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_8, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_8, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_10
    ui->screen_main_label_10 = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_10, 4, 4);
    lv_obj_set_size(ui->screen_main_label_10, 30, 49);
    lv_label_set_text(ui->screen_main_label_10, "温度");
    lv_label_set_long_mode(ui->screen_main_label_10, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_10, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_10, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_label_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_label_10, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_label_10, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_10, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_10, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_10, &lv_font_SourceHanSansSCBold_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_10, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_10, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_temp_val
    ui->screen_main_label_temp_val = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_temp_val, 35, 0);
    lv_obj_set_size(ui->screen_main_label_temp_val, 138, 52);
    lv_label_set_text(ui->screen_main_label_temp_val, "88.8");
    lv_label_set_long_mode(ui->screen_main_label_temp_val, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_temp_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_temp_val, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_temp_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_temp_val, &lv_font_SMG_55, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_temp_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_temp_val, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_temp_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_humi_val
    ui->screen_main_label_humi_val = lv_label_create(ui->screen_main_cont_temp_humi);
    lv_obj_set_pos(ui->screen_main_label_humi_val, 35, 54);
    lv_obj_set_size(ui->screen_main_label_humi_val, 139, 59);
    lv_label_set_text(ui->screen_main_label_humi_val, "88.8");
    lv_label_set_long_mode(ui->screen_main_label_humi_val, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_humi_val, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_humi_val, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_humi_val, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_humi_val, &lv_font_SMG_55, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_humi_val, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_humi_val, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_humi_val, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_cont_5
    ui->screen_main_cont_5 = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont_5, 188, 192);
    lv_obj_set_size(ui->screen_main_cont_5, 107, 118);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont_5, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont_5, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_cont_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_cont_5, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_cont_5, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont_5, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_cont_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_cont_5, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_14
    ui->screen_main_label_14 = lv_label_create(ui->screen_main_cont_5);
    lv_obj_set_pos(ui->screen_main_label_14, 64, 35);
    lv_obj_set_size(ui->screen_main_label_14, 30, 46);
    lv_label_set_text(ui->screen_main_label_14, "豆油");
    lv_label_set_long_mode(ui->screen_main_label_14, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_14, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_14, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_label_14, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_label_14, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_label_14, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_14, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_14, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_14, &lv_font_SourceHanSansSCBold_17, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_14, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_14, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_14, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_4
    ui->screen_main_img_4 = lv_image_create(ui->screen_main_cont_5);
    lv_obj_set_pos(ui->screen_main_img_4, 7, 35);
    lv_obj_set_size(ui->screen_main_img_4, 47, 45);
    lv_obj_add_flag(ui->screen_main_img_4, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_4, &_tiktok_RGB565A8_47x45);
    lv_image_set_pivot(ui->screen_main_img_4, 50,50);
    lv_image_set_rotation(ui->screen_main_img_4, 0);

    //Write style for screen_main_img_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_11
    ui->screen_main_label_11 = lv_label_create(ui->screen_main_cont_5);
    lv_obj_set_pos(ui->screen_main_label_11, 4, 84);
    lv_obj_set_size(ui->screen_main_label_11, 96, 27);
    lv_label_set_text(ui->screen_main_label_11, "65535");
    lv_label_set_long_mode(ui->screen_main_label_11, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_11, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_11, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_label_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_label_11, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_label_11, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_11, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_11, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_11, &lv_font_SMG_23, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_11, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_16
    ui->screen_main_label_16 = lv_label_create(ui->screen_main_cont_5);
    lv_obj_set_pos(ui->screen_main_label_16, 6, 2);
    lv_obj_set_size(ui->screen_main_label_16, 93, 29);
    lv_label_set_text(ui->screen_main_label_16, "粉丝数");
    lv_label_set_long_mode(ui->screen_main_label_16, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_16, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_16, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_main_label_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_main_label_16, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_main_label_16, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_16, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_16, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_16, &lv_font_SourceHanSansSCBold_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_16, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_16, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_16, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_fan_update
    ui->screen_main_label_fan_update = lv_label_create(ui->screen_main_cont_5);
    lv_obj_set_pos(ui->screen_main_label_fan_update, 74, 9);
    lv_obj_set_size(ui->screen_main_label_fan_update, 25, 16);
    lv_label_set_text(ui->screen_main_label_fan_update, "" LV_SYMBOL_LOOP " ");
    lv_label_set_long_mode(ui->screen_main_label_fan_update, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_fan_update, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_fan_update, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_fan_update, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_fan_update, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_fan_update, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_fan_update, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_main.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_main);

}
