/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *screen_main;
	bool screen_main_del;
	lv_obj_t *screen_main_cont_time;
	lv_obj_t *screen_main_label_date;
	lv_obj_t *screen_main_cont_2;
	lv_obj_t *screen_main_label_time_hour;
	lv_obj_t *screen_main_label_time_min;
	lv_obj_t *screen_main_label_1;
	lv_obj_t *screen_main_label_time_update;
	lv_obj_t *screen_main_cont_pc_time;
	lv_obj_t *screen_main_label_5;
	lv_obj_t *screen_main_cont_4;
	lv_obj_t *screen_main_label_4;
	lv_obj_t *screen_main_label_3;
	lv_obj_t *screen_main_label_2;
	lv_obj_t *screen_main_cont_top_bar;
	lv_obj_t *screen_main_img_1;
	lv_obj_t *screen_main_img_PC_statee;
	lv_obj_t *screen_main_img_2;
	lv_obj_t *screen_main_label_bat_val;
	lv_obj_t *screen_main_img_bat;
	lv_obj_t *screen_main_label_6;
	lv_obj_t *screen_main_label_light_val;
	lv_obj_t *screen_main_img_5;
	lv_obj_t *screen_main_label_15;
	lv_obj_t *screen_main_img_keep_stu;
	lv_obj_t *screen_main_label_17;
	lv_obj_t *screen_main_cont_temp_humi;
	lv_obj_t *screen_main_label_8;
	lv_obj_t *screen_main_label_10;
	lv_obj_t *screen_main_label_temp_val;
	lv_obj_t *screen_main_label_humi_val;
	lv_obj_t *screen_main_cont_5;
	lv_obj_t *screen_main_label_14;
	lv_obj_t *screen_main_img_4;
	lv_obj_t *screen_main_label_11;
	lv_obj_t *screen_main_label_16;
	lv_obj_t *screen_main_label_fan_update;
	lv_obj_t *screen_sleep;
	bool screen_sleep_del;
	lv_obj_t *screen_sleep_img_1;
	lv_obj_t *screen_sleep_label_1;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_screen_main(lv_ui *ui);
void setup_scr_screen_sleep(lv_ui *ui);
LV_IMAGE_DECLARE(_wifi_down_I4_33x31);
LV_IMAGE_DECLARE(_out_line_I4_39x43);
LV_IMAGE_DECLARE(_light_val_I4_29x25);
LV_IMAGE_DECLARE(_battery_charge_line_I4_39x28);
LV_IMAGE_DECLARE(_Human_Presence_Sensor_RGB565A8_36x37);
LV_IMAGE_DECLARE(_wifi_fill_I4_33x31);
LV_IMAGE_DECLARE(_keep_stu_I4_32x34);
LV_IMAGE_DECLARE(_tiktok_RGB565A8_47x45);
LV_IMAGE_DECLARE(_sleep2_RGB565_301x399);

LV_FONT_DECLARE(lv_font_SMG_32)
LV_FONT_DECLARE(lv_font_SMG_90)
LV_FONT_DECLARE(lv_font_SourceHanSansSCBold_24)
LV_FONT_DECLARE(lv_font_SMG_53)
LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_montserratMedium_15)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_montserratMedium_29)
LV_FONT_DECLARE(lv_font_SourceHanSansSCBold_20)
LV_FONT_DECLARE(lv_font_SMG_55)
LV_FONT_DECLARE(lv_font_SourceHanSansSCBold_17)
LV_FONT_DECLARE(lv_font_SMG_23)


#ifdef __cplusplus
}
#endif
#endif
