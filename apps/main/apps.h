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
#ifndef __APPS_H__
#define __APPS_H__

#include "app_status_ind.h"

#define STACK_READY_BT  0x01
#define STACK_READY_BLE 0x02

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

int app_init(void);

int app_deinit(int deinit_case);

int app_shutdown(void);

int app_reset(void);

int app_status_battery_report(uint8_t level);
void app_10_second_timer_check(void);//by pang


int app_voice_report( APP_STATUS_INDICATION_T status,uint8_t device_id);
int app_voice_report_generic(APP_STATUS_INDICATION_T status, uint8_t device_id, uint8_t isMerging);


/*FixME*/
void app_status_set_num(const char* p);

////////////10 second tiemr///////////////
#define APP_FAST_PAIRING_TIMEOUT_IN_SECOND  120

#define APP_PAIR_TIMER_ID       0
#define APP_POWEROFF_TIMER_ID   1
#define APP_AUTO_POWEROFF_TIMER_ID   2//add by pang
#define APP_FASTPAIR_LASTING_TIMER_ID   3

void app_stop_10_second_timer(uint8_t timer_id);
void app_start_10_second_timer(uint8_t timer_id);

void app_notify_stack_ready(uint8_t ready_flag);

void app_start_postponed_reset(void);

bool app_is_power_off_in_progress(void);

#define CHIP_ID_C     1
#define CHIP_ID_D     2

void app_disconnect_all_bt_connections(void);
bool app_is_stack_ready(void);

/** add by pang **/
extern uint8_t app_poweroff_flag;
void app_factory_reset(void);
void app_ring_merge_set(APP_STATUS_INDICATION_T status);
APP_STATUS_INDICATION_T app_ring_merge_get(void);
void app_usb_key_init(void);//add by cai
void app_key_init(void);//add by cai
/** end add **/

////////////////////


#ifdef __cplusplus
}
#endif
#endif//__FMDEC_H__
