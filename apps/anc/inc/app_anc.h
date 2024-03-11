/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __APP_ANC_H__
#define __APP_ANC_H__

#include "hal_aud.h"
#include "app_key.h"

#ifdef __cplusplus
extern "C" {
#endif

/** add by pang **/
enum ANC_STATUS
{
	anc_off = 0,
	anc_on,
	monitor,
};

enum ANC_ON_MODE
{
	anc_high = 0,
	anc_low,
	anc_wind,
};

enum MONITOR_ON_MODE
{
	monitor1 = 3,
	monitor2,
	monitor3,
	monitor4,
	monitor5,

	clearvoice1,
	clearvoice2,
	clearvoice3,
	clearvoice4,
	clearvoice5,
};

enum APP_ANC_MODE_STATUS
{
	NC_OFF = 0,
	ANC_HIGH,
	ANC_LOW,
	ANC_ADAPTIVE,
	ANC_WIND,
	MONITOR_ON,
	NC_INVALID
};

enum ANC_TOGGLE_MODE//add by cai
{
	AncOn_AncOff_Awareness = 0x00,
	AncOn_Awareness,
	AncOn_AncOff,
	Awareness_AncOff,
};
void app_anc_switch_turnled(bool on);
void app_monitor_switch_turnled(bool on);
void app_anc_Key_Pro(APP_KEY_STATUS *status, void *param);
void app_monitor_Key_Pro(APP_KEY_STATUS *status, void *param);
void set_anc_mode(uint8_t anc_new_mode,uint8_t prom_on);
void app_monitor_moment(bool on);
void app_anc_power_off(void);
void app_set_anc_on_mode(enum APP_ANC_MODE_STATUS anc_on_new_mode);
enum ANC_STATUS app_get_anc_status(void);
enum ANC_ON_MODE app_get_anc_on_mode(void);
void app_set_anc_on_mode(enum APP_ANC_MODE_STATUS anc_on_new_mode);
enum MONITOR_ON_MODE app_get_monitor_mode(void);
void app_set_monitor_mode(uint8_t monitor_new_level);
enum APP_ANC_MODE_STATUS app_get_anc_mode_status(void);
void poweron_set_anc(void);
enum ANC_TOGGLE_MODE app_get_anc_toggle_mode(void);
void app_set_anc_toggle_mode(enum ANC_TOGGLE_MODE anc_new_toggle_mode);
/** end add **/
void app_anc_set_coef(uint8_t index);
uint8_t app_anc_get_coef(void);
void app_anc_set_playback_samplerate(enum AUD_SAMPRATE_T sample_rate);
void app_anc_init(enum AUD_IO_PATH_T input_path, enum AUD_SAMPRATE_T playback_rate, enum AUD_SAMPRATE_T capture_rate);
void app_anc_close(void);
void app_anc_enable(void);
void app_anc_disable(void);
bool anc_enabled(void);
void test_anc(void);
void app_anc_resample(uint32_t res_ratio, uint32_t *in, uint32_t *out, uint32_t samples);
void app_anc_key(APP_KEY_STATUS *status, void *param);
int app_anc_open_module(void);
int app_anc_close_module(void);
enum AUD_SAMPRATE_T app_anc_get_play_rate(void);
bool app_anc_work_status(void);
void app_anc_send_howl_evt(uint32_t howl);
uint32_t app_anc_get_anc_status(void);
bool app_pwr_key_monitor_get_val(void);
bool app_anc_switch_get_val(void);
void app_anc_ios_init(void);
void app_anc_set_init_done(void);
bool app_anc_set_reboot(void);
void app_anc_status_post(uint8_t status);
bool app_anc_is_on(void);
uint32_t app_anc_get_sample_rate(void);

#ifdef __cplusplus
}
#endif

#endif
