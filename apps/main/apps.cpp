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
#include "stdio.h"
#include "cmsis_os.h"
#include "list.h"
#include "string.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"
#include "hal_sleep.h"
#include "pmu.h"
#include "audioflinger.h"
#include "apps.h"
#include "app_thread.h"
#include "app_key.h"
#include "app_bt_media_manager.h"
#include "app_pwl.h"
#include "app_audio.h"
#include "app_overlay.h"
#include "app_battery.h"
#include "app_utils.h"
#include "app_status_ind.h"
#include "bt_drv_interface.h"
#include "besbt.h"
#include "norflash_api.h"
#include "nvrecord.h"
#include "nvrecord_dev.h"
#include "nvrecord_env.h"
#include "crash_dump_section.h"
#include "log_section.h"
#include "factory_section.h"
#include "a2dp_api.h"
#include "me_api.h"
#include "os_api.h"
#include "btapp.h"
#include "app_bt.h"
#include "bt_if.h"
#include "gapm_task.h"
#include "app_ble_include.h"
#include "app_bt_func.h"
#include "app_ai_if.h"
#include "app_ai_manager_api.h"
#include "audio_process.h"

#ifdef __PC_CMD_UART__
#include "app_cmd.h"
#endif

#ifdef __FACTORY_MODE_SUPPORT__
#include "app_factory.h"
#include "app_factory_bt.h"
#endif

#ifdef __INTERCONNECTION__
#include "app_interconnection.h"
#include "app_interconnection_ble.h"
#include "app_interconnection_logic_protocol.h"
#include "app_ble_mode_switch.h"
#endif

#ifdef __INTERACTION__
#include "app_interaction.h"
#endif

#ifdef BISTO_ENABLED
#include "app_ai_manager_api.h"
#include "gsound_custom_reset.h"
#include "nvrecord_gsound.h"
#include "gsound_custom_actions.h"
#include "gsound_custom_ota.h"
#endif

#ifdef BES_OTA_BASIC
#include "ota_bes.h"
#endif

#ifdef MEDIA_PLAYER_SUPPORT
#include "resources.h"
#include "app_media_player.h"
#endif

#ifdef VOICE_DATAPATH
#include "app_voicepath.h"
#endif

#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#include "usbaudio_thread.h"
#include "usb_audio_app.h"
#endif

#ifdef TILE_DATAPATH
#include "tile_target_ble.h"
#endif

#if defined(IBRT)
#include "app_ibrt_if.h"
#include "app_ibrt_customif_ui.h"
#include "app_ibrt_ui_test.h"
#include "app_ibrt_voice_report.h"
#include "app_tws_if.h"
#endif

#ifdef GFPS_ENABLED
#include "app_gfps.h"
#endif

#ifdef BTIF_BLE_APP_DATAPATH_SERVER
#include "app_ble_cmd_handler.h"
#endif

#ifdef ANC_APP
#include "app_anc.h"
#endif

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#ifdef OTA_ENABLED
#include "nvrecord_ota.h"
#include "ota_common.h"
#endif

/** add by pang **/
#include "analog.h"
#include "hal_codec.h"
#include "tgt_hardware.h"
#include "audio_prompt_sbc.h"//add by cai
#ifdef __USER_DEFINE_CTR__ 
#include "app_user.h"
#endif

#if defined(__CST816S_TOUCH__)
#include "cst_capacitive_tp_hynitron_cst0xx.h"
#include "cst_ctp_hynitron_ext.h"
#endif

#if defined(__HAYLOU_APP__)
#include "../../services/ble_app/app_datapath/haylou_ble_hop.h"
#endif

bool factory_reset_flag = 0;
static APP_STATUS_INDICATION_T prompt_status = APP_STATUS_INDICATION_NUM;

#if defined(AUDIO_LINEIN)
extern int app_play_linein_onoff(bool onoff);
#endif
/**end add **/


#ifdef AUDIO_DEBUG_V0_1_0
extern "C" int speech_tuning_init(void);
#endif

#if (defined(BTUSB_AUDIO_MODE) || defined(BT_USB_AUDIO_DUAL_MODE))
extern "C"  bool app_usbaudio_mode_on(void) ;
#endif

#ifdef BES_OTA_BASIC
extern "C" void ota_flash_init(void);
#endif

#ifdef BT_USB_AUDIO_DUAL_MODE
extern "C" int hal_usb_configured(void);
#endif

#define APP_BATTERY_LEVEL_LOWPOWERTHRESHOLD (1)
#define POWERON_PRESSMAXTIME_THRESHOLD_MS  (7000)//5000   by pang

#ifdef FPGA
uint32_t __ota_upgrade_log_start[100];
#endif

enum APP_POWERON_CASE_T {
    APP_POWERON_CASE_NORMAL = 0,
    APP_POWERON_CASE_DITHERING,
    APP_POWERON_CASE_REBOOT,
    APP_POWERON_CASE_ALARM,
    APP_POWERON_CASE_CALIB,
    APP_POWERON_CASE_BOTHSCAN,
    APP_POWERON_CASE_CHARGING,
    APP_POWERON_CASE_FACTORY,
    APP_POWERON_CASE_TEST,
    APP_POWERON_CASE_USB_AUDIO,//add by cai
    APP_POWERON_CASE_INVALID,

    APP_POWERON_CASE_NUM
};

#ifdef RB_CODEC
extern int rb_ctl_init();
extern bool rb_ctl_is_init_done(void);
extern void app_rbplay_audio_reset_pause_status(void);
#endif

#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
extern bool app_pwr_key_monitor_get_val(void);
static bool anc_single_mode_on = false;
extern "C" bool anc_single_mode_is_on(void)
{
    return anc_single_mode_on;
}
#endif

#ifdef __ANC_STICK_SWITCH_USE_GPIO__
extern bool app_anc_switch_get_val(void);
#endif

#ifdef GFPS_ENABLED
extern "C" void app_fast_pairing_timeout_timehandler(void);
#endif

uint8_t  app_poweroff_flag = 0;
static enum APP_POWERON_CASE_T g_pwron_case = APP_POWERON_CASE_INVALID;

#ifndef APP_TEST_MODE
static uint8_t app_status_indication_init(void)
{
    struct APP_PWL_CFG_T cfg;
    memset(&cfg, 0, sizeof(struct APP_PWL_CFG_T));
    app_pwl_open();
	cfg.startlevel=0;//add by pang
    app_pwl_setup(APP_PWL_ID_0, &cfg);
    app_pwl_setup(APP_PWL_ID_1, &cfg);
    return 0;
}
#endif

#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__)

void PairingTransferToConnectable(void);

typedef void (*APP_10_SECOND_TIMER_CB_T)(void);

void app_pair_timerout(void);
void app_poweroff_timerout(void);
void CloseEarphone(void);
void Earphone_auto_poweroff(void);//add by cai


typedef struct
{
    uint8_t timer_id;
    uint8_t timer_en;
    //uint8_t timer_count;
	uint16_t timer_count;
    //uint8_t timer_period;
	uint16_t timer_period;
    APP_10_SECOND_TIMER_CB_T cb;
}APP_10_SECOND_TIMER_STRUCT;

#define INIT_APP_TIMER(_id, _en, _count, _period, _cb) \
    { \
        .timer_id = _id, \
        .timer_en = _en, \
        .timer_count = _count, \
        .timer_period = _period, \
        .cb = _cb, \
    }

APP_10_SECOND_TIMER_STRUCT app_10_second_array[] =
{
    INIT_APP_TIMER(APP_PAIR_TIMER_ID, 0, 0, 6, PairingTransferToConnectable),
    INIT_APP_TIMER(APP_POWEROFF_TIMER_ID, 0, 0, 45, CloseEarphone),//modify by pang
    INIT_APP_TIMER(APP_AUTO_POWEROFF_TIMER_ID, 0, 0, 45, Earphone_auto_poweroff),//add by pang
#ifdef GFPS_ENABLED
    INIT_APP_TIMER(APP_FASTPAIR_LASTING_TIMER_ID, 0, 0, APP_FAST_PAIRING_TIMEOUT_IN_SECOND/10,
        app_fast_pairing_timeout_timehandler),
#endif
};

void app_stop_10_second_timer(uint8_t timer_id)
{
    APP_10_SECOND_TIMER_STRUCT *timer = &app_10_second_array[timer_id];
	TRACE(1,"app_stop_10_second_timer %d",timer_id);

    timer->timer_en = 0;
    timer->timer_count = 0;
}

void app_start_10_second_timer(uint8_t timer_id)
{
    APP_10_SECOND_TIMER_STRUCT *timer = &app_10_second_array[timer_id];

	TRACE(1,"app_start_10_second_timer %d",timer_id);

    timer->timer_en = 1;
    timer->timer_count = 0;
}

void app_set_10_second_timer(uint8_t timer_id, uint8_t enable, uint8_t period)
{
    APP_10_SECOND_TIMER_STRUCT *timer = &app_10_second_array[timer_id];

    timer->timer_en = enable;
    timer->timer_count = period;
}

#if 0
void app_10_second_timer_check(void)
{
    APP_10_SECOND_TIMER_STRUCT *timer = app_10_second_array;
    unsigned int i;

    for(i = 0; i < ARRAY_SIZE(app_10_second_array); i++) {
        if (timer->timer_en) {
            timer->timer_count++;
            if (timer->timer_count >= timer->timer_period) {
                timer->timer_en = 0;
                if (timer->cb)
                    timer->cb();
            }
        }
        timer++;
    }
}
#else //m by pang
void app_10_second_timer_check(void)
{
    APP_10_SECOND_TIMER_STRUCT *timer = app_10_second_array;
	unsigned int i;
	
    for(i = 0; i < ARRAY_SIZE(app_10_second_array); i++) {
        if(i == APP_POWEROFF_TIMER_ID){
			if ((timer->timer_en) && (get_sleep_time()!=SLEEP_TIME_PERM)) {
            	timer->timer_count++;
            	if (timer->timer_count >= get_sleep_time()) {
                	timer->timer_en = 0;
                	if (timer->cb)
                    	timer->cb();
           	 	}
   			}
        } else if(i == APP_AUTO_POWEROFF_TIMER_ID){
			if ((timer->timer_en) && (get_auto_pwoff_time()!=AUTO_PWOFF_TIME_PERM)) {
				timer->timer_count++;
				if (timer->timer_count >= get_auto_pwoff_time()) {
					timer->timer_en = 0;
					if (timer->cb)
						timer->cb();
				}
			}
		} else{
			if (timer->timer_en) {
				timer->timer_count++;
					if (timer->timer_count >= timer->timer_period) {
					timer->timer_en = 0;
					if (timer->cb)
						timer->cb();
				}
			}
		}
		timer++;		
    }
}
#endif

void CloseEarphone(void)
{
    int activeCons;
    osapi_lock_stack();
    activeCons = btif_me_get_activeCons();
    osapi_unlock_stack();
#if 0
#ifdef ANC_APP
    if(app_anc_work_status()) {
        app_set_10_second_timer(APP_POWEROFF_TIMER_ID, 1, 30);
        return;
    }
#endif /* ANC_APP */
    if(activeCons == 0) {
        TRACE(0,"!!!CloseEarphone\n");
        app_shutdown();
    }
#else //m b pang

	if(app_lacal_host_bt_off())
		return;

//#if defined(__USE_3_5JACK_CTR__)
//	if(app_apps_3p5jack_plugin_flag(0))
//		return;
//#endif	
	if((activeCons == 0)||(0==app_bt_is_connected())) {
		TRACE(0,"!!!CloseEarphone\n");
		app_shutdown();
	}
	
#endif
}

void Earphone_auto_poweroff(void)
{
    int activeCons;
    osapi_lock_stack();
    activeCons = btif_me_get_activeCons();
    osapi_unlock_stack();
#if 0
#ifdef ANC_APP
    if(app_anc_work_status()) {
        app_set_10_second_timer(APP_POWEROFF_TIMER_ID, 1, 30);
        return;
    }
#endif /* ANC_APP */
    if(activeCons == 0) {
        TRACE(0,"!!!CloseEarphone\n");
        app_shutdown();
    }
#else //m b pang	
	if((activeCons > 0) || (0 < app_bt_is_connected())) {
		TRACE(0,"!!!CloseEarphone\n");
		app_shutdown();
	}
	
#endif
}

#endif /* #if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__) */

int signal_send_to_main_thread(uint32_t signals);
uint8_t stack_ready_flag = 0;
void app_notify_stack_ready(uint8_t ready_flag)
{
    TRACE(2,"app_notify_stack_ready %d %d", stack_ready_flag, ready_flag);

    stack_ready_flag |= ready_flag;

#ifdef __IAG_BLE_INCLUDE__
    if(stack_ready_flag == (STACK_READY_BT|STACK_READY_BLE))
#endif
    {
        signal_send_to_main_thread(0x3);
    }
}

bool app_is_stack_ready(void)
{
    bool ret = false;

    if (stack_ready_flag == (STACK_READY_BT
#ifdef __IAG_BLE_INCLUDE__
                             | STACK_READY_BLE
#endif
                             ))
    {
        ret = true;
    }

    return ret;
}

static void app_stack_ready_cb(void)
{
    TRACE(0,"stack init done");
#ifdef BLE_ENABLE
    app_ble_stub_user_init();
    app_ble_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
}

//#if (HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_BATTERY_REPORT) || (HF_SDK_FEATURES & HF_FEATURE_HF_INDICATORS)
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
extern void app_hfp_set_battery_level(uint8_t level);
#endif

int app_status_battery_report(uint8_t level)
{
#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
    if (anc_single_mode_on)  //anc power on,anc only mode
        return 0;
#endif
#if defined(__BTIF_EARPHONE__)
    if (app_is_stack_ready())
    {
        app_bt_state_checker();
    }
    //app_10_second_timer_check();//c by pang
#endif
    if (level <= APP_BATTERY_LEVEL_LOWPOWERTHRESHOLD)
    {
        //add something
    }

    if (app_is_stack_ready())
    {
// #if (HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_BATTERY_REPORT) || (HF_SDK_FEATURES & HF_FEATURE_HF_INDICATORS)
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
#if defined(IBRT)
        if (app_tws_ibrt_mobile_link_connected())
        {
            app_hfp_set_battery_level(level);
        }
#else
        app_hfp_set_battery_level(level);
#endif
#else
        TRACE(1,"[%s] Can not enable SUPPORT_BATTERY_REPORT", __func__);
#endif
        osapi_notify_evm();
    }
    return 0;
}

#ifdef MEDIA_PLAYER_SUPPORT

void app_status_set_num(const char* p)
{
    media_Set_IncomingNumber(p);
}

int app_voice_report_handler(APP_STATUS_INDICATION_T status, uint8_t device_id, uint8_t isMerging)
{
/** add by pang **/
#if defined(__USE_AMP_MUTE_CTR__)
    if(!hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin))
		hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);
#endif

/** end add **/
    TRACE(3,"%s %d%s",__func__, status, status2str((uint16_t)status));
    AUD_ID_ENUM id = MAX_RECORD_NUM;
#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
    if(anc_single_mode_on)
        return 0;
#endif
    if (app_poweroff_flag == 1){
        switch (status) {
            case APP_STATUS_INDICATION_POWEROFF:
                id = AUD_ID_POWER_OFF;
                break;
			case APP_STATUS_INDICATION_POWEROFF_LOWBATTERY:
                id = AUD_ID_POWEROFF_LOWBATTERY;
                break;
            default:
                return 0;
                break;
        }
    }else{
        switch (status) {
            case APP_STATUS_INDICATION_POWERON:
                id = AUD_ID_POWER_ON;
                break;
            case APP_STATUS_INDICATION_POWEROFF:
                id = AUD_ID_POWER_OFF;
                break;
            case APP_STATUS_INDICATION_CONNECTED:
                id = AUD_ID_BT_CONNECTED;
                break;
            case APP_STATUS_INDICATION_DISCONNECTED:
                id = AUD_ID_BT_DIS_CONNECT;
                break;
            case APP_STATUS_INDICATION_CALLNUMBER:
                id = AUD_ID_BT_CALL_INCOMING_NUMBER;
                break;
            case APP_STATUS_INDICATION_CHARGENEED:
                id = AUD_ID_BT_CHARGE_PLEASE;
                break;
            case APP_STATUS_INDICATION_FULLCHARGE:
                id = AUD_ID_BT_CHARGE_FINISH;
                break;
            case APP_STATUS_INDICATION_PAIRSUCCEED:
                id = AUD_ID_BT_PAIRING_SUC;
                break;
            case APP_STATUS_INDICATION_PAIRFAIL:
                id = AUD_ID_BT_PAIRING_FAIL;
                break;

            case APP_STATUS_INDICATION_HANGUPCALL:
                id = AUD_ID_BT_CALL_HUNG_UP;
                break;

            case APP_STATUS_INDICATION_REFUSECALL:
                id = AUD_ID_BT_CALL_REFUSE;
                isMerging = false;
                break;

            case APP_STATUS_INDICATION_ANSWERCALL:
                id = AUD_ID_BT_CALL_ANSWER;
                break;

            case APP_STATUS_INDICATION_CLEARSUCCEED:
                id = AUD_ID_BT_CLEAR_SUCCESS;
                break;

            case APP_STATUS_INDICATION_CLEARFAIL:
                id = AUD_ID_BT_CLEAR_FAIL;
                break;
            case APP_STATUS_INDICATION_INCOMINGCALL:
                id = AUD_ID_BT_CALL_INCOMING_CALL;
                break;
            case APP_STATUS_INDICATION_BOTHSCAN:
                id = AUD_ID_BT_PAIR_ENABLE;
                break;
            case APP_STATUS_INDICATION_WARNING:
                id = AUD_ID_BT_WARNING;
                break;
            case APP_STATUS_INDICATION_ALEXA_START:
                id = AUDIO_ID_BT_ALEXA_START;
                break;
            case APP_STATUS_INDICATION_ALEXA_STOP:
                id = AUDIO_ID_BT_ALEXA_STOP;
                break;
            case APP_STATUS_INDICATION_GSOUND_MIC_OPEN:
                id = AUDIO_ID_BT_GSOUND_MIC_OPEN;
                break;
            case APP_STATUS_INDICATION_GSOUND_MIC_CLOSE:
                id = AUDIO_ID_BT_GSOUND_MIC_CLOSE;
                break;
            case APP_STATUS_INDICATION_GSOUND_NC:
                id = AUDIO_ID_BT_GSOUND_NC;
                break;
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__
            case APP_STATUS_RING_WARNING:
                id = AUD_ID_RING_WARNING;
                break;
#endif
            case APP_STATUS_INDICATION_MUTE:
                id = AUDIO_ID_BT_MUTE;
                break;
#if 1//def __INTERACTION__
            case APP_STATUS_INDICATION_FINDME:
                id = AUD_ID_BT_FINDME;
                break;
#endif
            case APP_STATUS_INDICATION_TILE_FIND:
                id = AUDIO_ID_BT_ALEXA_START;
                break;
/** add by pang **/
			case APP_STATUS_INDICATION_ANC_ON:
				id = AUD_ID_BT_ANC_ON;			
                break;
			case APP_STATUS_INDICATION_ANC_OFF:
				id = AUD_ID_BT_ANC_OFF;			
                break;
			case APP_STATUS_INDICATION_MONITOR_ON:
				id = AUD_ID_BT_MONITOR_ON;			
                break;
			case APP_STATUS_INDICATION_SHORT_1:
				id = AUD_ID_BT_SHORT_1;	
				break;
			case APP_STATUS_INDICATION_LOST_OF_RANGE:
				id = AUD_ID_BT_LOST_OF_RANGE;                
                break;
			case APP_STATUS_INDICATION_FACTORYRESET:
				id = AUD_ID_BT_FACTORY_RESET;
                break;
			case APP_STATUS_INDICATION_GAMING_ON:
				id = AUD_ID_BT_GAMING_ON;
                break;
			case APP_STATUS_INDICATION_GAMING_OFF:
				id = AUD_ID_BT_GAMING_OFF;
                break;
			
			case APP_STATUS_INDICATION_BEEP_21:
                id = AUD_ID_BEEP_21;
                break;

			case APP_STATUS_INDICATION_BEEP_22:
                id = AUD_ID_BEEP_22;
                break;

			case APP_STATUS_INDICATION_DEMO_MODE:
                id = AUD_ID_DEMO_MODE;
                break;
				
			case APP_STATUS_INDICATION_CALLING_MUTE:
				id = AUD_ID_CALLING_MUTE;
                break;
			
			case APP_STATUS_INDICATION_CALLING_UNMUTE:
				id = AUD_ID_CALLING_UNMUTE;
				break;		
/** end add **/
            default:
                break;
        }
    }
#ifdef BT_USB_AUDIO_DUAL_MODE
    if(btusb_is_usb_mode()) {
        if(hal_usb_configured()){
			audio_prompt_stop_playing();//add by cai for prompt cutdown to play
            usb_audio_start_audio_prompt(id);
        }
    }
    else
#endif
    {
#if defined(IBRT)
        app_ibrt_if_voice_report_handler(id, isMerging);
#else
        trigger_media_play(id, device_id, isMerging);
#endif
    }

    return 0;
}

extern "C" int app_voice_report(APP_STATUS_INDICATION_T status, uint8_t device_id)
{
	prompt_status = status;//add by pang
    return app_voice_report_handler(status, device_id, true);
}

extern "C" int app_voice_report_generic(APP_STATUS_INDICATION_T status, uint8_t device_id, uint8_t isMerging)
{
    return app_voice_report_handler(status, device_id, isMerging);
}

/** add by pang **/
void app_ring_merge_set(APP_STATUS_INDICATION_T status)
{
    prompt_status=status;
}

APP_STATUS_INDICATION_T app_ring_merge_get(void)
{
    return prompt_status;
}
/** end add **/
#endif

static void app_poweron_normal(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    g_pwron_case = APP_POWERON_CASE_NORMAL;
	
#ifdef ANC_APP
	poweron_set_anc();//add by cai for Pairing tone distortion
#endif

#if 1 //by pang
	app_status_indication_recover_set(APP_STATUS_INDICATION_POWERON);
#ifdef MEDIA_PLAYER_SUPPORT
	app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif

#else
    signal_send_to_main_thread(0x2);
#endif
}

#if !defined(BLE_ONLY_ENABLED)
static void app_poweron_scan(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    g_pwron_case = APP_POWERON_CASE_BOTHSCAN;

#if 1 //by pang
	lostconncection_to_pairing=1;
	app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
#ifdef MEDIA_PLAYER_SUPPORT
    app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
#endif

#else
    signal_send_to_main_thread(0x2);
#endif
}
#endif

/** add by pang **/
#if 0
static void apps_Clean_bt_PDL(APP_KEY_STATUS *status, void *param)
{	
	//TRACE(3,"%s %d,%d",__func__, status->code, status->event);
	g_pwron_case = APP_POWERON_CASE_FACTORY;

	//app_status_indication_set(APP_STATUS_INDICATION_FACTORYRESET);
	app_status_indication_set_next(APP_STATUS_INDICATION_FACTORYRESET,APP_STATUS_INDICATION_BOTHSCAN);
#ifdef MEDIA_PLAYER_SUPPORT
	app_voice_report(APP_STATUS_INDICATION_FACTORYRESET,0);
#endif

	//osSignalSet(apps_init_tid, 0x2);
}
#endif
extern bt_status_t LinkDisconnectDirectly(bool PowerOffFlag);
extern struct BT_DEVICE_T  app_bt_device;
void app_factory_reset(void)
{
	app_status_indication_recover_set(APP_STATUS_INDICATION_FACTORYRESET);

	app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
	osDelay(500);		  
	factory_reset_flag=1;
	LinkDisconnectDirectly(false);
	osDelay(800);
	
#ifdef MEDIA_PLAYER_SUPPORT
	app_voice_report(APP_STATUS_INDICATION_FACTORYRESET, 0);
#endif

#if 0
	struct nvrecord_env_t *nvrecord_env;
	nv_record_sector_clear();
	nv_record_env_init();
	nv_record_env_get(&nvrecord_env);
	if(nvrecord_env) {
		nvrecord_env->media_language.language = NVRAM_ENV_MEDIA_LANGUAGE_DEFAULT;        
        nvrecord_env->factory_tester_status.status = NVRAM_ENV_FACTORY_TESTER_STATUS_DEFAULT;
    }	
    nv_record_env_set(nvrecord_env);
#if FPGA==0	
    nv_record_flash_flush();
#endif
#else
	nv_record_rebuild();
#endif

	osDelay(500);

#if defined(__EVRCORD_USER_DEFINE__)
	app_nvrecord_para_default();
#endif

	app_bt_reconnect_idle_mode();
#ifdef  __IAG_BLE_INCLUDE__
	app_ble_force_switch_adv(BLE_SWITCH_USER_BT_CONNECT, false);
#endif
	app_bt_accessmode_set_req(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
}
/**  end add **/


#if 0// def __ENGINEER_MODE_SUPPORT__ //c by pang
#if !defined(BLE_ONLY_ENABLED)
static void app_poweron_factorymode(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    app_factorymode_enter();
}
#endif
#endif


static bool g_pwron_finished = false;
static void app_poweron_finished(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    g_pwron_finished = true;
    signal_send_to_main_thread(0x2);
}

void app_poweron_wait_finished(void)
{
    if (!g_pwron_finished){
        osSignalWait(0x2, osWaitForever);
    }
}

#if  defined(__POWERKEY_CTRL_ONOFF_ONLY__)
void app_bt_key_shutdown(APP_KEY_STATUS *status, void *param);
const  APP_KEY_HANDLE  pwron_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},           "power on: shutdown"     , app_bt_key_shutdown, NULL},
};
#elif defined(__ENGINEER_MODE_SUPPORT__)
const  APP_KEY_HANDLE  pwron_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLONGPRESS},    "power on: normal"     , app_poweron_normal, NULL},
#if !defined(BLE_ONLY_ENABLED)
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLONGLONGPRESS}, "power on: both scan"  , app_poweron_scan  , NULL},
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLONGLONGPRESS},"power on: factory mode", app_poweron_factorymode  , NULL},
#endif
	//{{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLLLONGPRESS},"power on: clear device", apps_Clean_bt_PDL, NULL},//M by pang
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITFINISHED},     "power on: finished"   , app_poweron_finished  , NULL},
};
#else
const  APP_KEY_HANDLE  pwron_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITUP},           "power on: normal"     , app_poweron_normal, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLONGPRESS},    "power on: both scan"  , app_poweron_scan  , NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITFINISHED},     "power on: finished"   , app_poweron_finished  , NULL},
};
#endif

#ifndef APP_TEST_MODE
static void app_poweron_key_init(void)
{
    uint8_t i = 0;
    TRACE(1,"%s",__func__);

    for (i=0; i<(sizeof(pwron_key_handle_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&pwron_key_handle_cfg[i]);
    }
}

static uint8_t app_poweron_wait_case(void)
{
    uint32_t stime = 0, etime = 0;

#ifdef __POWERKEY_CTRL_ONOFF_ONLY__
    g_pwron_case = APP_POWERON_CASE_NORMAL;
#else
#if 0
    TRACE(1,"poweron_wait_case enter:%d", g_pwron_case);
    if (g_pwron_case == APP_POWERON_CASE_INVALID){
        stime = hal_sys_timer_get();
        osSignalWait(0x2, POWERON_PRESSMAXTIME_THRESHOLD_MS);
        etime = hal_sys_timer_get();
    }
    TRACE(2,"powon raw case:%d time:%d", g_pwron_case, TICKS_TO_MS(etime - stime));
#else //modify by pang
	if (g_pwron_case == APP_POWERON_CASE_INVALID){
		stime = hal_sys_timer_get();
		osSignalWait(0x2, 3000);
		if((g_pwron_case != APP_POWERON_CASE_INVALID)&&(g_pwron_finished==false))
			osSignalWait(0x2, POWERON_PRESSMAXTIME_THRESHOLD_MS);
			
		etime = hal_sys_timer_get();
	}
	TRACE(2,"powon raw case:%d time:%d", g_pwron_case, TICKS_TO_MS(etime - stime));
#endif
#endif

    return g_pwron_case;
}
#endif

static void app_wait_stack_ready(void)
{
    uint32_t stime, etime;
    stime = hal_sys_timer_get();
    osSignalWait(0x3, 1000);
    etime = hal_sys_timer_get();
    TRACE(1,"app_wait_stack_ready: wait:%d ms", TICKS_TO_MS(etime - stime));

    app_stack_ready_cb();
}

extern "C" int system_shutdown(void);
int app_shutdown(void)
{
    system_shutdown();
    return 0;
}

int system_reset(void);
int app_reset(void)
{
    system_reset();
    return 0;
}

static void app_postponed_reset_timer_handler(void const *param);
osTimerDef(APP_POSTPONED_RESET_TIMER, app_postponed_reset_timer_handler);
static osTimerId app_postponed_reset_timer = NULL;
#define APP_RESET_PONTPONED_TIME_IN_MS  2000
static void app_postponed_reset_timer_handler(void const *param)
{
    pmu_reboot();
}

void app_start_postponed_reset(void)
{
    if (NULL == app_postponed_reset_timer)
    {
        app_postponed_reset_timer = osTimerCreate(osTimer(APP_POSTPONED_RESET_TIMER), osTimerOnce, NULL);
    }

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);

    osTimerStart(app_postponed_reset_timer, APP_RESET_PONTPONED_TIME_IN_MS);
}

void app_bt_key_shutdown(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
#ifdef __POWERKEY_CTRL_ONOFF_ONLY__
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    app_reset();
#else
    
#if defined(__DEFINE_DEMO_MODE__)//add by pang
	if(app_battery_is_charging() && app_nvrecord_demo_mode_get()){
		TRACE(0,"power off->charging!!!");
		hal_sw_bootmode_set(HAL_SW_BOOTMODE_CHARGING_POWEROFF);
		app_reset();
	}
	else{
		if(!app_call_status_get()) app_shutdown();//m by cai for not to power off when call is active
	}
#else
	if(!app_call_status_get()) app_shutdown();//m by cai for not to power off when call is active
#endif
#endif
}

void app_bt_key_enter_testmode(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s\n",__FUNCTION__);

    if(app_status_indication_get() == APP_STATUS_INDICATION_BOTHSCAN){
#ifdef __FACTORY_MODE_SUPPORT__
        app_factorymode_bt_signalingtest(status, param);
#endif
    }
}

void app_bt_key_enter_nosignal_mode(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s\n",__FUNCTION__);
    if(app_status_indication_get() == APP_STATUS_INDICATION_BOTHSCAN){
#ifdef __FACTORY_MODE_SUPPORT__
        app_factorymode_bt_nosignalingtest(status, param);
#endif
    }
}

void app_bt_singleclick(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
}

void app_bt_doubleclick(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
}

void app_power_off(APP_KEY_STATUS *status, void *param)
{
    TRACE(0,"app_power_off\n");
}

extern "C" void OS_NotifyEvm(void);

#define PRESS_KEY_TO_ENTER_OTA_INTERVEL    (15000)          // press key 15s enter to ota
#define PRESS_KEY_TO_ENTER_OTA_REPEAT_CNT    ((PRESS_KEY_TO_ENTER_OTA_INTERVEL - 2000) / 500)
void app_otaMode_enter(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s",__func__);

    hal_norflash_disable_protection(HAL_NORFLASH_ID_0);

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
#ifdef __KMATE106__
    app_status_indication_set(APP_STATUS_INDICATION_OTA);
    //app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
    osDelay(1200);
#endif
    pmu_reboot();
}

#ifdef __USB_COMM__
void app_usb_cdc_comm_key_handler(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d", __func__, status->code, status->event);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_CDC_COMM);
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    hal_cmu_reset_set(HAL_CMU_MOD_GLOBAL);
}
#endif

#if 0
void app_dfu_key_handler(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s ",__func__);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_FORCE_USB_DLD);
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    hal_cmu_reset_set(HAL_CMU_MOD_GLOBAL);
}
#else
void app_dfu_key_handler(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s ",__func__);
    hal_sw_bootmode_clear(0xffffffff);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_FORCE_USB_DLD | HAL_SW_BOOTMODE_SKIP_FLASH_BOOT);
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    hal_cmu_reset_set(HAL_CMU_MOD_GLOBAL);
}
#endif

void app_ota_key_handler(APP_KEY_STATUS *status, void *param)
{
    static uint32_t time = hal_sys_timer_get();
    static uint16_t cnt = 0;

    TRACE(3,"%s %d,%d",__func__, status->code, status->event);

    if (TICKS_TO_MS(hal_sys_timer_get() - time) > 600) // 600 = (repeat key intervel)500 + (margin)100
        cnt = 0;
    else
        cnt++;

    if (cnt == PRESS_KEY_TO_ENTER_OTA_REPEAT_CNT) {
        app_otaMode_enter(NULL, NULL);
    }

    time = hal_sys_timer_get();
}
extern "C" void app_bt_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
#define DEBUG_CODE_USE 0
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
            TRACE(0,"first blood!");
#if DEBUG_CODE_USE
            if (status->code == APP_KEY_CODE_PWR)
            {
#ifdef __INTERCONNECTION__
                // add favorite music
                // app_interconnection_handle_favorite_music_through_ccmp(1);

                // ask for ota update
                ota_update_request();
                return;
#else
                static int m = 0;
                if (m == 0) {
                    m = 1;
                    hal_iomux_set_analog_i2c();
                }
                else {
                    m = 0;
                    hal_iomux_set_uart0();
                }
#endif
            }
#endif
            break;
        case APP_KEY_EVENT_DOUBLECLICK:
            TRACE(0,"double kill");
#if DEBUG_CODE_USE
            if (status->code == APP_KEY_CODE_PWR)
            {
#ifdef __INTERCONNECTION__
                // play favorite music
                app_interconnection_handle_favorite_music_through_ccmp(2);
#else
                app_otaMode_enter(NULL, NULL);
#endif
                return;
            }
#endif
			break;
        case APP_KEY_EVENT_TRIPLECLICK:
            TRACE(0,"triple kill");
/* //c by pang
            if (status->code == APP_KEY_CODE_PWR)
            {
            
#ifndef __BT_ONE_BRING_TWO__ 
				if(btif_me_get_activeCons() < 1){
#else
	            if(btif_me_get_activeCons() < 2){
#endif
	                app_bt_accessmode_set(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
#ifdef __INTERCONNECTION__
	                app_interceonnection_start_discoverable_adv(INTERCONNECTION_BLE_FAST_ADVERTISING_INTERVAL,
	                                                            APP_INTERCONNECTION_FAST_ADV_TIMEOUT_IN_MS);
	                return;
#endif
#ifdef GFPS_ENABLED
	                app_enter_fastpairing_mode();
#endif
					app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
            	}
                return;
            }
*/
            break;
        case APP_KEY_EVENT_ULTRACLICK:
            TRACE(0,"ultra kill");
            break;
        case APP_KEY_EVENT_RAMPAGECLICK:
            TRACE(0,"rampage kill!you are crazy!");
            break;

        case APP_KEY_EVENT_UP:
            break;
    }
#ifdef __FACTORY_MODE_SUPPORT__
	if(0){//m by cai
    //if (app_status_indication_get() == APP_STATUS_INDICATION_BOTHSCAN && (status->event == APP_KEY_EVENT_DOUBLECLICK)){
        app_factorymode_languageswitch_proc();
    }else
#endif
    {
#ifdef __SUPPORT_ANC_SINGLE_MODE_WITHOUT_BT__
        if(!anc_single_mode_on)
#endif
        bt_key_send(status);
    }
}

#ifdef RB_CODEC
extern bool app_rbcodec_check_hfp_active(void );
void app_switch_player_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);

    if(!rb_ctl_is_init_done()) {
        TRACE(0,"rb ctl not init done");
        return ;
    }

    if( app_rbcodec_check_hfp_active() ) {
        app_bt_key(status,param);
        return;
    }

    app_rbplay_audio_reset_pause_status();

    if(app_rbplay_mode_switch()) {
        app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
        app_rbcodec_ctr_play_onoff(true);
    } else {
        app_rbcodec_ctr_play_onoff(false);
        app_voice_report(APP_STATUS_INDICATION_POWEROFF, 0);
    }
    return ;

}
#endif

void app_voice_assistant_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(2,"%s event %d", __func__, status->event);
    if (app_ai_manager_is_in_multi_ai_mode())
    {
        if (app_ai_manager_spec_get_status_is_in_invalid()) {
            TRACE(0,"AI feature has been diabled");
            return;
        }

#ifdef MAI_TYPE_REBOOT_WITHOUT_OEM_APP
        if (app_ai_manager_get_spec_update_flag()) {
            TRACE(0,"device reboot is ongoing...");
            return;
        }
#endif

        if(app_ai_manager_voicekey_is_enable()) {
            if (AI_SPEC_GSOUND == app_ai_manager_get_current_spec()) {
#ifdef BISTO_ENABLED
                gsound_custom_actions_handle_key(status, param);
#endif
            } else if(AI_SPEC_INIT != app_ai_manager_get_current_spec()) {
                app_ai_key_event_handle(status, 0);
            }
        }
    }
    else
    {
        app_ai_key_event_handle(status, 0);
#if defined(BISTO_ENABLED)
        gsound_custom_actions_handle_key(status, param);
#endif
    }
}

#ifdef IS_MULTI_AI_ENABLED
void app_voice_gva_onoff_key(APP_KEY_STATUS *status, void *param)
{
    uint8_t current_ai_spec = app_ai_manager_get_current_spec();

    TRACE(2,"%s current_ai_spec %d", __func__, current_ai_spec);
    if (current_ai_spec == AI_SPEC_INIT)
    {
        app_ai_manager_enable(true, AI_SPEC_GSOUND);
    }
    else if(current_ai_spec == AI_SPEC_GSOUND)
    {
        app_ai_manager_enable(false, AI_SPEC_GSOUND);
    }
    else if(current_ai_spec == AI_SPEC_AMA)
    {
        app_ai_manager_switch_spec(AI_SPEC_GSOUND);
    }
    app_ble_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
}

void app_voice_ama_onoff_key(APP_KEY_STATUS *status, void *param)
{
    uint8_t current_ai_spec = app_ai_manager_get_current_spec();

    TRACE(2,"%s current_ai_spec %d", __func__, current_ai_spec);
    if (current_ai_spec == AI_SPEC_INIT)
    {
        app_ai_manager_enable(true, AI_SPEC_AMA);
    }
    else if(current_ai_spec == AI_SPEC_AMA)
    {
        app_ai_manager_enable(false, AI_SPEC_AMA);
    }
    else if(current_ai_spec == AI_SPEC_GSOUND)
    {
        app_ai_manager_switch_spec(AI_SPEC_AMA);
    }
    app_ble_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
}
#endif

#if defined(BT_USB_AUDIO_DUAL_MODE_TEST) && defined(BT_USB_AUDIO_DUAL_MODE)
extern "C" void test_btusb_switch(void);
void app_btusb_audio_dual_mode_test(APP_KEY_STATUS *status, void *param)
{
    TRACE(0,"test_btusb_switch");
    test_btusb_switch();
}
#endif

extern void switch_dualmic_status(void);

void app_switch_dualmic_key(APP_KEY_STATUS *status, void *param)
{
    switch_dualmic_status();
}

#ifdef POWERKEY_I2C_SWITCH
extern void app_factorymode_i2c_switch(APP_KEY_STATUS *status, void *param);
#endif

#if defined(TILE_DATAPATH)
extern "C" void app_tile_key_handler(APP_KEY_STATUS *status, void *param);
#endif

#ifdef __POWERKEY_CTRL_ONOFF_ONLY__
#if defined(__APP_KEY_FN_STYLE_A__)
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"bt volume up key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_LONGPRESS},"bt play backward key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_UP},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_LONGPRESS},"bt play forward key",app_bt_key, NULL},
#ifdef SUPPORT_SIRI
    {{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif

};
#else //#elif defined(__APP_KEY_FN_STYLE_B__)
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_REPEAT},"bt volume up key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"bt play backward key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_REPEAT},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_UP},"bt play forward key",app_bt_key, NULL},
#ifdef SUPPORT_SIRI
    {{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif

};
#endif
#else
#if defined(__APP_KEY_FN_STYLE_A__)
//--
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGLONGPRESS},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGPRESS},"bt function key",app_bt_key, NULL},
	{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"play forward",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"play backward",app_bt_key, NULL},
#if defined(BT_USB_AUDIO_DUAL_MODE_TEST) && defined(BT_USB_AUDIO_DUAL_MODE)
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_bt_key, NULL},
#ifdef RB_CODEC
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_switch_player_key, NULL},
#else
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"btusb mode switch key.",app_btusb_audio_dual_mode_test, NULL},
#endif
#endif

#if RAMPAGECLICK_TEST_MODE
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_ULTRACLICK},"bt function key",app_bt_key_enter_nosignal_mode, NULL},
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"bt function key",app_bt_key_enter_testmode, NULL},
#endif
#ifdef POWERKEY_I2C_SWITCH
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"bt i2c key",app_factorymode_i2c_switch, NULL},
#endif
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt volume up key",app_bt_key, NULL},
    //{{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"bt play backward key",app_bt_key, NULL},
#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
    //{{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"bt mode src snk key",app_bt_key, NULL},
#endif
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_REPEAT},"bt volume down key",app_bt_key, NULL},
    //{{APP_KEY_CODE_FN2,APP_KEY_EVENT_LONGPRESS},"bt play forward key",app_bt_key, NULL},
    //{{APP_KEY_CODE_FN15,APP_KEY_EVENT_UP},"bt volume down key",app_bt_key, NULL},
    
	{{HAL_KEY_CODE_FN5,APP_KEY_EVENT_CLICK},"bt anc key",bt_key_handle_ANC_key, NULL},
	{{HAL_KEY_CODE_FN5,APP_KEY_EVENT_DOUBLECLICK},"bt anc key",bt_key_handle_ANC_key, NULL},
	{{HAL_KEY_CODE_FN6,APP_KEY_EVENT_LONGPRESS},"bt quick monitor",app_bt_key, NULL},
	{{HAL_KEY_CODE_FN6,APP_KEY_EVENT_UP_AFTER_LONGPRESS},"bt quick monitor",app_bt_key, NULL},
	//{{HAL_KEY_CODE_FN6,APP_KEY_EVENT_DOUBLECLICK},"game mode",app_bt_key, NULL},
	//{{HAL_KEY_CODE_FN6,APP_KEY_EVENT_TRIPLECLICK},"siri",app_bt_key, NULL},
	//{{HAL_KEY_CODE_FN6,APP_KEY_EVENT_ULTRACLICK},"siri",app_bt_key, NULL},

	{{HAL_KEY_CODE_FN5|APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGLONGLONGPRESS},"factory reset",app_bt_key, NULL},
	//{{APP_KEY_CODE_FN1|APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGLONGLONGPRESS},"demo mode",app_bt_key, NULL},
#ifdef SUPPORT_SIRI
    //{{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif
#if defined( __BT_ANC_KEY__)&&defined(ANC_APP)
#if defined(IBRT)|| defined(__POWERKEY_CTRL_BT_ANC__)
     //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt anc key",app_anc_key, NULL},
#else
	 //{{APP_KEY_CODE_FN2,APP_KEY_EVENT_CLICK},"bt anc key",app_anc_key, NULL},
#endif
#endif
#if defined(VOICE_DATAPATH) || defined(__AI_VOICE__)
    //{{APP_KEY_CODE_AI, APP_KEY_EVENT_FIRST_DOWN}, "AI key", app_voice_assistant_key, NULL},
#if defined(IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED) || defined(PUSH_AND_HOLD_ENABLED) || defined(__TENCENT_VOICE__)
    //{{APP_KEY_CODE_AI, APP_KEY_EVENT_UP}, "AI key", app_voice_assistant_key, NULL},
#endif
    //{{APP_KEY_CODE_AI, APP_KEY_EVENT_UP_AFTER_LONGPRESS}, "AI key", app_voice_assistant_key, NULL},
    //{{APP_KEY_CODE_AI, APP_KEY_EVENT_LONGPRESS}, "AI key", app_voice_assistant_key, NULL},
    //{{APP_KEY_CODE_AI, APP_KEY_EVENT_CLICK}, "AI key", app_voice_assistant_key, NULL},
    //{{APP_KEY_CODE_AI, APP_KEY_EVENT_DOUBLECLICK}, "AI key", app_voice_assistant_key, NULL},
#endif
#ifdef IS_MULTI_AI_ENABLED
    //{{APP_KEY_CODE_FN13, APP_KEY_EVENT_CLICK}, "gva on-off key", app_voice_gva_onoff_key, NULL},
    //{{APP_KEY_CODE_FN14, APP_KEY_EVENT_CLICK}, "ama on-off key", app_voice_ama_onoff_key, NULL},
#endif
#if defined(TILE_DATAPATH)
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"bt function key",app_tile_key_handler, NULL},
    //{{APP_KEY_CODE_TILE, APP_KEY_EVENT_DOWN}, "tile function key", app_tile_key_handler, NULL},
    //{{APP_KEY_CODE_TILE, APP_KEY_EVENT_UP}, "tile function key", app_tile_key_handler, NULL},
#endif

#if defined(BT_USB_AUDIO_DUAL_MODE_TEST) && defined(BT_USB_AUDIO_DUAL_MODE)
    //{{APP_KEY_CODE_FN15, APP_KEY_EVENT_CLICK}, "btusb mode switch key.", app_btusb_audio_dual_mode_test, NULL},
#endif
};
#else //#elif defined(__APP_KEY_FN_STYLE_B__)
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGPRESS},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_ULTRACLICK},"bt function key",app_bt_key_enter_nosignal_mode, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"bt function key",app_bt_key_enter_testmode, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_REPEAT},"bt volume up key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt play backward key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_REPEAT},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"bt play forward key",app_bt_key, NULL},
#ifdef SUPPORT_SIRI
    {{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif

    {{APP_KEY_CODE_AI, APP_KEY_EVENT_FIRST_DOWN}, "AI key", app_voice_assistant_key, NULL},
#if defined(IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED) || defined(PUSH_AND_HOLD_ENABLED)
    {{APP_KEY_CODE_AI, APP_KEY_EVENT_UP}, "AI key", app_voice_assistant_key, NULL},
#endif
    {{APP_KEY_CODE_AI, APP_KEY_EVENT_UP_AFTER_LONGPRESS}, "AI key", app_voice_assistant_key, NULL},
    {{APP_KEY_CODE_AI, APP_KEY_EVENT_LONGPRESS}, "AI key", app_voice_assistant_key, NULL},
    {{APP_KEY_CODE_AI, APP_KEY_EVENT_CLICK}, "AI key", app_voice_assistant_key, NULL},
    {{APP_KEY_CODE_AI, APP_KEY_EVENT_DOUBLECLICK}, "AI key", app_voice_assistant_key, NULL},
};
#endif
#endif

void app_key_init(void)
{
#if defined(IBRT)
    app_ibrt_ui_test_key_init();
#else
    uint8_t i = 0;
    TRACE(1,"%s",__func__);

    app_key_handle_clear();
    for (i=0; i<(sizeof(app_key_handle_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&app_key_handle_cfg[i]);
    }
#endif
}
/** add by pang **/
#if defined(__DEFINE_DEMO_MODE__)
void app_charging_poweron_key_handler(APP_KEY_STATUS *status, void *param)
{
	TRACE(0,"%s ",__func__);
	if(app_nvrecord_demo_mode_get() && !hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter.pin))//m by cai
	{
		//hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);//add by cai
		hal_sw_bootmode_set(HAL_SW_BOOTMODE_CHARGING_POWERON);
		hal_cmu_sys_reboot();
	}
}
#endif
/** end add **/

void app_key_init_on_charging(void)
{
    uint8_t i = 0;
    const APP_KEY_HANDLE  key_cfg[] = {
        //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_REPEAT},"ota function key",app_ota_key_handler, NULL},
        //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_dfu_key_handler, NULL},
#ifdef __USB_COMM__
        //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"usb cdc key",app_usb_cdc_comm_key_handler, NULL},
#endif
#if defined(__DEFINE_DEMO_MODE__)
		{{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGLONGPRESS},"power on key",app_charging_poweron_key_handler, NULL},//m by cai
#endif
    };

    TRACE(1,"%s",__func__);
    for (i=0; i<(sizeof(key_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&key_cfg[i]);
    }
}

extern bt_status_t LinkDisconnectDirectly(bool PowerOffFlag);
void a2dp_suspend_music_force(void);

bool app_is_power_off_in_progress(void)
{
    return app_poweroff_flag?TRUE:FALSE;
}

#if GFPS_ENABLED
#define APP_GFPS_BATTERY_TIMEROUT_VALUE             (10000)
static void app_gfps_battery_show_timeout_timer_cb(void const *n);
osTimerDef (GFPS_BATTERY_SHOW_TIMEOUT_TIMER, app_gfps_battery_show_timeout_timer_cb);
static osTimerId app_gfps_battery_show_timer_id = NULL;
#include "app_gfps.h"
static void app_gfps_battery_show_timeout_timer_cb(void const *n)
{
    TRACE(1,"%s", __func__);
    app_gfps_set_battery_datatype(HIDE_UI_INDICATION);
}

void app_gfps_battery_show_timer_start()
{
    if (app_gfps_battery_show_timer_id == NULL)
        app_gfps_battery_show_timer_id = osTimerCreate(osTimer(GFPS_BATTERY_SHOW_TIMEOUT_TIMER), osTimerOnce, NULL);
    osTimerStart(app_gfps_battery_show_timer_id, APP_GFPS_BATTERY_TIMEROUT_VALUE);
}

void app_gfps_battery_show_timer_stop()
{
    if (app_gfps_battery_show_timer_id)
        osTimerStop(app_gfps_battery_show_timer_id);
}

void app_voice_gfps_find(void)
{
    app_voice_report(APP_STATUS_INDICATION_TILE_FIND, 0);
}

#endif

int app_deinit(int deinit_case)
{
    int nRet = 0;
    TRACE(2,"%s case:%d",__func__, deinit_case);

#ifdef __PC_CMD_UART__
    app_cmd_close();
#endif
#if (defined(BTUSB_AUDIO_MODE) || defined(BT_USB_AUDIO_DUAL_MODE))
    if(app_usbaudio_mode_on()) return 0;
#endif
    if (!deinit_case){
        app_poweroff_flag = 1;
		/** add by pang **/	
		app_user_event_close_module();
		app_anc_power_off();
		/** end add **/
#if defined(APP_LINEIN_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE,0);
#endif
#if defined(APP_I2S_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_I2S_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE,0);
#endif
        app_status_indication_filter_set(APP_STATUS_INDICATION_BOTHSCAN);
        app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
        osDelay(500);
        LinkDisconnectDirectly(true);
        osDelay(500);
		
		if(!app_battery_is_charging()){
			//app_status_indication_set(APP_STATUS_INDICATION_POWEROFF);
			app_status_indication_recover_set(APP_STATUS_INDICATION_POWEROFF);
		}
		
		if((!app_battery_is_charging())&&(!app_apps_3p5jack_plugin_flag(0))){
#ifdef MEDIA_PLAYER_SUPPORT
	        if(app_battery_is_pdvolt()){
				app_voice_report(APP_STATUS_INDICATION_POWEROFF_LOWBATTERY, 0);//add by pang
				osDelay(2000);//m by cai
			}
			else{
	        	app_voice_report(APP_STATUS_INDICATION_POWEROFF, 0);
				osDelay(2000);//m by cai
			}
#endif
		}
		
#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_DEINIT);
#endif
#if FPGA==0
        nv_record_flash_flush();
        norflash_flush_all_pending_op();
#endif
        osDelay(1000);

/** add by pang **/
		//hal_codec_dac_mute(1);
#if defined(__USE_AMP_MUTE_CTR__)
		hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_pio_AMP_mute_control.pin);
#endif
		
#if defined(__LDO_3V3_CTR__)	
		//osDelay(300);
		//if (cfg_hw_pio_3_3v_control.pin != HAL_IOMUX_PIN_NUM){
			//hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_pio_3_3v_control.pin);
		//}
#endif
/** end add **/
        af_close();
    }

    return nRet;
}

#ifdef APP_TEST_MODE
extern void app_test_init(void);
int app_init(void)
{
    int nRet = 0;
    //uint8_t pwron_case = APP_POWERON_CASE_INVALID;
    TRACE(1,"%s",__func__);
    app_poweroff_flag = 0;

    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_52M);
    list_init();
    af_open();
    app_os_init();
    app_pwl_open();
    app_audio_open();
    app_audio_manager_open();
    app_overlay_open();
    if (app_key_open(true))
    {
        nRet = -1;
        goto exit;
    }

    app_test_init();
exit:
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);
    return nRet;
}
#else /* !defined(APP_TEST_MODE) */

int app_bt_connect2tester_init(void)
{
    btif_device_record_t rec;
    bt_bdaddr_t tester_addr;
    uint8_t i;
    bool find_tester = false;
    struct nvrecord_env_t *nvrecord_env;
    btdevice_profile *btdevice_plf_p;
    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env->factory_tester_status.status != NVRAM_ENV_FACTORY_TESTER_STATUS_DEFAULT)
        return 0;

    if (!nvrec_dev_get_dongleaddr(&tester_addr)){
        nv_record_open(section_usrdata_ddbrecord);
        for (i = 0; nv_record_enum_dev_records(i, &rec) == BT_STS_SUCCESS; i++) {
            if (!memcmp(rec.bdAddr.address, tester_addr.address, BTIF_BD_ADDR_SIZE)){
                find_tester = true;
            }
        }
        if(i==0 && !find_tester){
            memset(&rec, 0, sizeof(btif_device_record_t));
            memcpy(rec.bdAddr.address, tester_addr.address, BTIF_BD_ADDR_SIZE);
            nv_record_add(section_usrdata_ddbrecord, &rec);
            btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(rec.bdAddr.address);
            nv_record_btdevicerecord_set_hfp_profile_active_state(btdevice_plf_p, true);
            nv_record_btdevicerecord_set_a2dp_profile_active_state(btdevice_plf_p, true);
        }
        if (find_tester && i>2){
            nv_record_ddbrec_delete(&tester_addr);
            nvrecord_env->factory_tester_status.status = NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS;
            nv_record_env_set(nvrecord_env);
        }
    }

    return 0;
}

int app_nvrecord_rebuild(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    nv_record_sector_clear();
    nv_record_env_init();
    nv_record_update_factory_tester_status(NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS);
    nv_record_env_set(nvrecord_env);
    nv_record_flash_flush();

    return 0;
}

#if (defined(BTUSB_AUDIO_MODE) || defined(BT_USB_AUDIO_DUAL_MODE))
#include "app_audio.h"
#include "usb_audio_frm_defs.h"
#include "usb_audio_app.h"

static bool app_usbaudio_mode = false;

extern "C" void btusbaudio_entry(void);
void app_usbaudio_entry(void)
{
    btusbaudio_entry();
    app_usbaudio_mode = true ;
}

bool app_usbaudio_mode_on(void)
{
    return app_usbaudio_mode;
}

void app_usb_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
	if(get_usb_configured_status() || hal_usb_configured()) usb_audio_app_key((HAL_KEY_CODE_T)status->code, (HAL_KEY_EVENT_T)status->event);//add by cai
}

/** add by cai **/
void app_usb_ANC_key(APP_KEY_STATUS *status, void *param)
{
	if(get_usb_configured_status() || hal_usb_configured()) 
	{
		app_anc_Key_Pro(NULL, NULL);
		usb_audio_eq_loop();//add by cai
	}	
}

osTimerId usb_quick_awareness_sw_timer = NULL;
static void usb_quick_awareness_swtimer_handler(void const *param);
osTimerDef(USB_QUICK_AWARENESS_TIMER, usb_quick_awareness_swtimer_handler);// define timers
#define USB_QUICK_AWARENESS_SWTIMER_IN_MS	(15000)

void usb_app_quick_awareness_swtimer_start(void)
{
	if(usb_quick_awareness_sw_timer == NULL)
		usb_quick_awareness_sw_timer = osTimerCreate(osTimer(USB_QUICK_AWARENESS_TIMER), osTimerOnce, NULL);
	
	osTimerStart(usb_quick_awareness_sw_timer,USB_QUICK_AWARENESS_SWTIMER_IN_MS);
}

void usb_app_quick_awareness_swtimer_stop(void)
{
	if(usb_quick_awareness_sw_timer == NULL)
		return;
	
	osTimerStop(usb_quick_awareness_sw_timer);
}

static void usb_quick_awareness_swtimer_handler(void const *param)
{
	usb_audio_set_volume_for_quick_awareness(false, 3 + 17);//m by cai for volume indepent
	app_monitor_moment(false);
}

void app_usb_Cover_key(APP_KEY_STATUS *status, void *param)
{
	if(get_usb_configured_status() || hal_usb_configured())
	{
		switch(status->event)
		{
			case APP_KEY_EVENT_LONGPRESS:
				usb_audio_set_volume_for_quick_awareness(true, 3 + 17);//m by cai for volume indepent
				app_monitor_moment(true);
				usb_app_quick_awareness_swtimer_start();//add by cai for exit quick Awareness after 15s	
			break;

			case  APP_KEY_EVENT_UP_AFTER_LONGPRESS:
				usb_app_quick_awareness_swtimer_stop();//add by cai for exit quick Awareness after 15s
				app_monitor_moment(false);
				usb_audio_set_volume_for_quick_awareness(false, 3 + 17);//m by cai for volume indepent
			break;
			
			default:
				TRACE(2,"%s: unregister down key event=%x",__func__,status->event);
			break;
		}
	}
}
/** end add **/

#if 1
const APP_KEY_HANDLE  app_usb_handle_cfg[] = {//m by cai
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"USB HID VOLUMEUP key",app_usb_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"USB HID VOLUMEDOWN key",app_usb_key, NULL},
	{{APP_KEY_CODE_FN1,APP_KEY_EVENT_REPEAT},"USB HID VOLUMEDOWN key",app_usb_key, NULL},
	{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"USB HID PWR CLICK key",app_usb_key, NULL},
	{{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"USB HID PWR DOUBLECLICK key",app_usb_key, NULL},
	{{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"USB HID PWR TRIPLECLICK key",app_usb_key, NULL},
	{{HAL_KEY_CODE_FN5,APP_KEY_EVENT_CLICK},"bt anc key",app_usb_ANC_key, NULL},
	{{HAL_KEY_CODE_FN6,APP_KEY_EVENT_LONGPRESS},"bt quick monitor",app_usb_Cover_key, NULL},
	{{HAL_KEY_CODE_FN6,APP_KEY_EVENT_UP_AFTER_LONGPRESS},"bt quick monitor",app_usb_Cover_key, NULL},
};
#else//for debug
const APP_KEY_HANDLE  app_usb_handle_cfg[] = {
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"USB HID FN1 UP key",app_usb_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"USB HID FN2 UP key",app_usb_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},"USB HID PWR UP key",app_usb_key, NULL},
};
#endif

void app_usb_key_init(void)
{
    uint8_t i = 0;
    TRACE(1,"%s",__func__);
	app_key_handle_clear();//add by cai
    for (i=0; i<(sizeof(app_usb_handle_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&app_usb_handle_cfg[i]);
    }
}
#endif /* (defined(BTUSB_AUDIO_MODE) || defined(BT_USB_AUDIO_DUAL_MODE)) */

//#define OS_HAS_CPU_STAT 1
#if OS_HAS_CPU_STAT
extern "C" void rtx_show_all_threads_usage(void);
#define _CPU_STATISTICS_PEROID_ 6000
#define CPU_USAGE_TIMER_TMO_VALUE (_CPU_STATISTICS_PEROID_/3)
static void cpu_usage_timer_handler(void const *param);
osTimerDef(cpu_usage_timer, cpu_usage_timer_handler);
static osTimerId cpu_usage_timer_id = NULL;
static void cpu_usage_timer_handler(void const *param)
{
    rtx_show_all_threads_usage();
}
#endif

#ifdef USER_REBOOT_PLAY_MUSIC_AUTO
bool a2dp_need_to_play = false;
#endif
extern void  btif_me_write_bt_sleep_enable(uint8_t sleep_en);

int btdrv_tportopen(void);


void app_ibrt_init(void)
{
        app_bt_global_handle_init();
#if defined(IBRT)
        ibrt_config_t config;
        app_tws_ibrt_init();
        app_ibrt_ui_init();
        app_ibrt_ui_test_init();
        app_ibrt_if_config_load(&config);
        app_ibrt_customif_ui_start();
#ifdef IBRT_SEARCH_UI
        app_tws_ibrt_start(&config, true);
        app_ibrt_search_ui_init(false,IBRT_NONE_EVENT);
#else
        app_tws_ibrt_start(&config, false);
#endif

#endif
}

#ifdef GFPS_ENABLED
static void app_tell_battery_info_handler(uint8_t *batteryValueCount,
                                          uint8_t *batteryValue)
{
    GFPS_BATTERY_STATUS_E status;
    if (app_battery_is_charging())
    {
        status = BATTERY_CHARGING;
    }
    else
    {
        status = BATTERY_NOT_CHARGING;
    }

    // TODO: add the charger case's battery level
#ifdef IBRT   
    if (app_tws_ibrt_tws_link_connected())
    {
        *batteryValueCount = 2;
    }
    else
    {
        *batteryValueCount = 1;
    }
#else
    *batteryValueCount = 1;
#endif

    if (1 == *batteryValueCount)
    {
        batteryValue[0] = (app_battery_current_level() * 10) | (status << 7);
    }
    else
    {
        // if (app_tws_is_left_side())
        // {
        //     batteryValue[0] = (app_battery_current_level() * 10) | (status << 7);
        //     batteryValue[1] = (app_tws_get_peer_device_battery_level() * 10) | (status << 7);
        // }
        // else
        {
            // batteryValue[0] = (app_tws_get_peer_device_battery_level() * 10) | (status << 7);
            batteryValue[0] = (app_battery_current_level() * 10) | (status << 7);
            batteryValue[1] = (app_battery_current_level() * 10) | (status << 7);
        }
    }
}
#endif
extern uint32_t __coredump_section_start[];
extern uint32_t __ota_upgrade_log_start[];
extern uint32_t __log_dump_start[];
extern uint32_t __crash_dump_start[];
extern uint32_t __custom_parameter_start[];
extern uint32_t __aud_start[];
extern uint32_t __userdata_start[];
extern uint32_t __factory_start[];
#if defined(CUSTOM_BIN_CONFIG)
extern uint32_t __custom_bin1_start[];//add by pang
extern uint32_t __custom_bin2_start[];//add by pang
#endif

int app_init(void)
{
    int nRet = 0;
    struct nvrecord_env_t *nvrecord_env;
    bool need_check_key = true;
    uint8_t pwron_case = APP_POWERON_CASE_INVALID;
#ifdef BT_USB_AUDIO_DUAL_MODE
    uint8_t usb_plugin = 0;
#endif
#ifdef IBRT_SEARCH_UI
    bool is_charging_poweron=false;
#endif
    TRACE(0,"please check all sections sizes and heads is correct ........");
#if defined(CUSTOM_BIN_CONFIG)
	TRACE(2,"__custom_bin1_start: %p length: 0x%x", __custom_bin1_start, CUSTOM_BIN_CONFIG_SIZE); //add by pang
	TRACE(2,"__custom_bin2_start: %p length: 0x%x", __custom_bin2_start, CUSTOM_BIN_CONFIG_SIZE); //add by pang
#endif
    TRACE(2,"__coredump_section_start: %p length: 0x%x", __coredump_section_start, CORE_DUMP_SECTION_SIZE);
    TRACE(2,"__ota_upgrade_log_start: %p length: 0x%x", __ota_upgrade_log_start, OTA_UPGRADE_LOG_SIZE);
    TRACE(2,"__log_dump_start: %p length: 0x%x", __log_dump_start, LOG_DUMP_SECTION_SIZE);
    TRACE(2,"__crash_dump_start: %p length: 0x%x", __crash_dump_start, CRASH_DUMP_SECTION_SIZE);
    TRACE(2,"__custom_parameter_start: %p length: 0x%x", __custom_parameter_start, CUSTOM_PARAMETER_SECTION_SIZE);
    TRACE(2,"__userdata_start: %p length: 0x%x", __userdata_start, USERDATA_SECTION_SIZE*2);
    TRACE(2,"__aud_start: %p length: 0x%x", __aud_start, AUD_SECTION_SIZE);
    TRACE(2,"__factory_start: %p length: 0x%x", __factory_start, FACTORY_SECTION_SIZE);

    TRACE(0,"app_init\n");

#ifdef __RPC_ENABLE__
extern int rpc_service_setup(void);
    rpc_service_setup();
#endif

#ifdef IBRT
    // init tws interface
    app_tws_if_init();
#endif // #ifdef IBRT

    nv_record_init();
    factory_section_init();

#if defined(__EVRCORD_USER_DEFINE__)
	app_nvrecord_para_get();//add by pang
#endif

#ifdef ANC_APP
    app_anc_ios_init();
#endif
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_104M);
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    TRACE(1,"sys freq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
#endif
    list_init();
    nRet = app_os_init();
    if (nRet) {
        goto exit;
    }
#if OS_HAS_CPU_STAT
        cpu_usage_timer_id = osTimerCreate(osTimer(cpu_usage_timer), osTimerPeriodic, NULL);
        if (cpu_usage_timer_id != NULL) {
            osTimerStart(cpu_usage_timer_id, CPU_USAGE_TIMER_TMO_VALUE);
        }
#endif

    app_status_indication_init();

#ifdef BTADDR_FOR_DEBUG
    gen_bt_addr_for_debug();
#endif

#ifdef FORCE_SIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
#elif defined FORCE_NOSIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
#endif

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT_FROM_CRASH){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT_FROM_CRASH);
        TRACE(0,"Crash happened!!!");
    #ifdef VOICE_DATAPATH
        gsound_dump_set_flag(true);
    #endif
    }

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pwron_case = APP_POWERON_CASE_REBOOT;
        need_check_key = false;
        TRACE(0,"Initiative REBOOT happens!!!");
#ifdef USER_REBOOT_PLAY_MUSIC_AUTO
        if(hal_sw_bootmode_get() & HAL_SW_BOOTMODE_LOCAL_PLAYER)
        {
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_LOCAL_PLAYER);
            a2dp_need_to_play = true;
            TRACE(0,"a2dp_need_to_play = true");
        }
#endif
    }

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_MODE){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MODE);
        pwron_case = APP_POWERON_CASE_TEST;
        need_check_key = false;
        TRACE(0,"To enter test mode!!!");
    }

/**add by pang **/
	if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_RESERVED_BIT24){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_RESERVED_BIT24);
        pwron_case = APP_POWERON_CASE_NORMAL;
        need_check_key = false;
		nv_record_rebuild();
        TRACE(0,"******reset");
   }
/** end add **/

#ifdef BT_USB_AUDIO_DUAL_MODE
    usb_os_init();
#endif
    nRet = app_battery_open();
    TRACE(1,"BATTERY %d",nRet);
    if (pwron_case != APP_POWERON_CASE_TEST){
#ifdef USER_REBOOT_PLAY_MUSIC_AUTO
        TRACE(0,"hal_sw_bootmode_clear HAL_SW_BOOTMODE_LOCAL_PLAYER!!!!!!");
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_LOCAL_PLAYER);
#endif
        switch (nRet) {
            case APP_BATTERY_OPEN_MODE_NORMAL:
                nRet = 0;
#if defined(__DEFINE_DEMO_MODE__)
				if(app_nvrecord_demo_mode_get()) 
				{
					app_demo_mode_poweron_flag_set(true);//add by cai
#if defined(__CHARGE_CURRRENT__)
					hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_charge_current_control.pin);//add by cai
#endif
				}
#endif
                break;
            case APP_BATTERY_OPEN_MODE_CHARGING:
                app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
                TRACE(0,"CHARGING!");
                app_battery_start();

				while(app_nvrecord_demo_mode_get() && hal_pwrkey_startup_pressed());//add by cai for demo mode
                app_key_open(false);
                app_key_init_on_charging();
				/** add by pang **/
				app_user_event_open_module_for_charge();
#ifdef __PWM_LED_CTL__
				apps_pwm_set(RED_PWM_LED, 1);//enable pwm
#endif
				/** end add **/
                nRet = 0;
#if defined(__USE_3_5JACK_CTR__)
				if(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter.pin) || app_nvrecord_demo_mode_get()) 
				{
#if defined(__LDO_3V3_CTR__) 
					hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_3_3v_control.pin);//add by cai for usb audio
#endif

#if defined(__CHARGE_CURRRENT__)
					hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_charge_current_control.pin);//add by cai for enter nomal charging mode when usb is not configed.
#endif				
					goto exit;//add by cai
				}	
#endif
			
#if defined(__LDO_3V3_CTR__) 
				hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_3_3v_control.pin);//add by cai for usb audio
#endif
#if defined(BT_USB_AUDIO_DUAL_MODE)
                usb_plugin = 1;
				need_check_key = false;//add by cai	for open usb audio
#elif defined(BTUSB_AUDIO_MODE)
                goto exit;
#else
                goto exit;
#endif
                break;
            case APP_BATTERY_OPEN_MODE_CHARGING_PWRON:
                TRACE(0,"CHARGING PWRON!");
#ifdef IBRT_SEARCH_UI
                is_charging_poweron=true;
#endif
                need_check_key = false;
                nRet = 0;
#if defined(__DEFINE_DEMO_MODE__)
				app_demo_mode_poweron_flag_set(true);//add by pang
#if defined(__CHARGE_CURRRENT__)
				hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_charge_current_control.pin);//add by cai
#endif
#endif
#if defined(BT_USB_AUDIO_DUAL_MODE)
				usb_plugin = 1;//add by cai
#endif
                break;
            case APP_BATTERY_OPEN_MODE_INVALID:
            default:
                nRet = -1;
                goto exit;
                break;
        }
    }

    if (app_key_open(need_check_key)){
        TRACE(0,"PWR KEY DITHER!");
        nRet = -1;
        goto exit;
    }

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    app_poweron_key_init();
#if defined(_AUTO_TEST_)
    AUTO_TEST_SEND("Power on.");
#endif
    app_bt_init();
    af_open();
    app_audio_open();
    app_audio_manager_open();
    app_overlay_open();

    nv_record_env_init();
    nvrec_dev_data_open();
    factory_section_open();
//    app_bt_connect2tester_init();
    nv_record_env_get(&nvrecord_env);

#ifdef BISTO_ENABLED
    nv_record_gsound_rec_init();
#endif

#ifdef BLE_ENABLE
    app_ble_mode_init();
    app_ble_customif_init();
#ifdef IBRT
    app_ble_force_switch_adv(BLE_SWITCH_USER_IBRT, false);
#endif // #ifdef IBRT
#endif

    audio_process_init();
#ifdef __PC_CMD_UART__
    app_cmd_open();
#endif
#ifdef AUDIO_DEBUG_V0_1_0
    speech_tuning_init();
#endif
#ifdef ANC_APP
    app_anc_open_module();
#endif

#ifdef MEDIA_PLAYER_SUPPORT
    app_play_audio_set_lang(nvrecord_env->media_language.language);
#endif
    app_bt_stream_volume_ptr_update(NULL);

#ifdef __THIRDPARTY
    app_thirdparty_init();
    app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO2,THIRDPARTY_INIT);
#endif

    // TODO: freddie->unify all of the OTA modules
#if defined(BES_OTA_BASIC)
    ota_flash_init();
#endif

#ifdef OTA_ENABLED
    /// init OTA common module
    ota_common_init_handler();
#endif // OTA_ENABLED

#ifdef IBRT
    // for TWS side decision, the last bit is 1:right, 0:left
    if (app_tws_is_unknown_side())
    {
        app_tws_set_side_from_addr(factory_section_get_bt_address());
    }
#endif


    btdrv_start_bt();
#if defined (__GMA_VOICE__) && defined(IBRT_SEARCH_UI)
	app_ibrt_reconfig_btAddr_from_nv();
#endif

    if (pwron_case != APP_POWERON_CASE_TEST) {
        BesbtInit();
        app_wait_stack_ready();
        bt_drv_extra_config_after_init();
        bt_generate_ecdh_key_pair();
        app_bt_start_custom_function_in_bt_thread((uint32_t)0,
            0, (uint32_t)app_ibrt_init);
    }
#if defined(BLE_ENABLE) && defined(IBRT)
    app_ble_force_switch_adv(BLE_SWITCH_USER_IBRT, true);
#endif
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_52M);
    TRACE(1,"\n\n\nbt_stack_init_done:%d\n\n\n", pwron_case);

/** add by pang **/
#if defined(__LDO_3V3_CTR__) 
	hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_pio_3_3v_control.pin);
#endif
/** end add **/

    if (pwron_case == APP_POWERON_CASE_REBOOT){
#if !defined(__DEFINE_DEMO_MODE__)//m by cai
        app_status_indication_set(APP_STATUS_INDICATION_POWERON);
#ifdef MEDIA_PLAYER_SUPPORT
        app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif
#else
/** add by pang **/
		if(app_demo_mode_poweron_flag_get() || !app_battery_is_charging()){
			app_status_indication_recover_set(APP_STATUS_INDICATION_POWERON);//m by cai
#ifdef ANC_APP
			//poweron_set_anc();//add by cai for Pairing tone distortion
#endif

#ifdef MEDIA_PLAYER_SUPPORT
			app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif
		}
/** end add **/

#endif
        app_bt_start_custom_function_in_bt_thread((uint32_t)1,
                    0, (uint32_t)btif_me_write_bt_sleep_enable);
        btdrv_set_lpo_times();

#if defined(BES_OTA_BASIC)
        bes_ota_init();
#endif
        //app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);
#if defined(IBRT)
#ifdef IBRT_SEARCH_UI
        if(is_charging_poweron==false)
        {
            if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
            {
                TRACE(0,"ibrt_ui_log:power on unknow mode");
                app_ibrt_enter_limited_mode();
            }
            else
            {
                TRACE(1,"ibrt_ui_log:power on %d fetch out", nvrecord_env->ibrt_mode.mode);
                app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);
            }
        }
#elif defined(IS_MULTI_AI_ENABLED)
        //when ama and bisto switch, earphone need reconnect with peer, master need reconnect with phone
        app_ibrt_ui_event_entry(IBRT_OPEN_BOX_EVENT);
        TRACE(1,"ibrt_ui_log:power on %d fetch out", nvrecord_env->ibrt_mode.mode);
        app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);
#endif
#else
        app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);
#endif

        app_key_init();
        app_battery_start();
#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__)
        if(app_demo_mode_poweron_flag_get() || !app_battery_is_charging()) app_start_10_second_timer(APP_POWEROFF_TIMER_ID);//m by cai for usb audio
#endif

#if defined(__IAG_BLE_INCLUDE__) && defined(BTIF_BLE_APP_DATAPATH_SERVER)
        BLE_custom_command_init();
#endif
#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_INIT);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_START);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO2,THIRDPARTY_BT_CONNECTABLE);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO3,THIRDPARTY_START);
#endif
#if defined( __BTIF_EARPHONE__) && defined(__BTIF_BT_RECONNECT__)
#if !defined(IBRT)
#if !defined(__DEFINE_DEMO_MODE__)//m by cai
		power_on_open_reconnect_flag=1;//add by pang
        app_bt_profile_connect_manager_opening_reconnect();
#else
		if(app_demo_mode_poweron_flag_get() || !app_battery_is_charging()) {
		power_on_open_reconnect_flag=1;//add by pang
        app_bt_profile_connect_manager_opening_reconnect();
		}
#endif
#endif
#endif
/** add by pang **/
#if defined(__CST816S_TOUCH__)
		cst816s_open_module();
#endif
		app_user_event_open_module();
#ifdef ANC_APP
		poweron_set_anc();
		//app_anc_Key_Pro(NULL, NULL);
		//app_anc_switch_turnled(true);
		//app_monitor_switch_turnled(false);
#endif
#if defined(__USE_AMP_MUTE_CTR__)
		play_reboot_set();
#endif
/** end add **/
    }
#ifdef __ENGINEER_MODE_SUPPORT__
    else if(pwron_case == APP_POWERON_CASE_TEST){
        app_factorymode_set(true);
        app_status_indication_set(APP_STATUS_INDICATION_POWERON);
#ifdef MEDIA_PLAYER_SUPPORT
        app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif
#ifdef __WATCHER_DOG_RESET__
        app_wdt_close();
#endif
        TRACE(0,"!!!!!ENGINEER_MODE!!!!!\n");
        nRet = 0;
        app_nvrecord_rebuild();
        app_factorymode_key_init();
        if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_SIGNALINGMODE){
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
            app_factorymode_bt_signalingtest(NULL, NULL);
        }
        if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE){
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
            app_factorymode_bt_nosignalingtest(NULL, NULL);
        }
    }
#endif
    else{
/* //c by pang
        app_status_indication_set(APP_STATUS_INDICATION_POWERON);
#ifdef MEDIA_PLAYER_SUPPORT
        app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif
*/
        if (need_check_key){
            pwron_case = app_poweron_wait_case();
        }
        else
        {
#if !defined(__DEFINE_DEMO_MODE__)//m by cai
            pwron_case = APP_POWERON_CASE_NORMAL;
#else
        	if(app_battery_is_charging() && !app_demo_mode_poweron_flag_get()) {
				pwron_case = APP_POWERON_CASE_USB_AUDIO;
			} else{
				pwron_case = APP_POWERON_CASE_NORMAL;
/** add by pang **/
#if defined(__DEFINE_DEMO_MODE__)
				if(app_demo_mode_poweron_flag_get()){
					app_status_indication_recover_set(APP_STATUS_INDICATION_POWERON);//m by cai
#ifdef ANC_APP
					//poweron_set_anc();//add by cai for Pairing tone distortion
#endif

#ifdef MEDIA_PLAYER_SUPPORT
					app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif
				}
#endif
/** end add **/
			} 
#endif
        }
        if (pwron_case != APP_POWERON_CASE_INVALID && pwron_case != APP_POWERON_CASE_DITHERING){
            AUTO_TEST_TRACE(1,"power on case:%d\n", pwron_case);
            nRet = 0;
/* //c by pang
#ifndef __POWERKEY_CTRL_ONOFF_ONLY__
            app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
#endif
*/
            app_bt_start_custom_function_in_bt_thread((uint32_t)1,
                        0, (uint32_t)btif_me_write_bt_sleep_enable);
            btdrv_set_lpo_times();

#ifdef BES_OTA_BASIC
            bes_ota_init();
#endif

#ifdef __INTERCONNECTION__
            app_interconnection_init();
#endif

#ifdef __INTERACTION__
            app_interaction_init();
#endif

#if defined(__IAG_BLE_INCLUDE__) && defined(BTIF_BLE_APP_DATAPATH_SERVER)
            BLE_custom_command_init();
#endif
#ifdef GFPS_ENABLED
             app_gfps_set_battery_info_acquire_handler(app_tell_battery_info_handler);
             app_gfps_set_battery_datatype(SHOW_UI_INDICATION);
#endif
            switch (pwron_case) {
                case APP_POWERON_CASE_CALIB:
                    break;
				case APP_POWERON_CASE_FACTORY: //add by pang
						nv_record_sector_clear();
						nv_record_env_init();
						nvrec_dev_data_open();
						nv_record_env_get(&nvrecord_env);
					    power_on_open_reconnect_flag=1;//add by pang
						app_bt_accessmode_set(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
					break;
                case APP_POWERON_CASE_BOTHSCAN:
/* //c by pang
                    app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
#ifdef MEDIA_PLAYER_SUPPORT
                    app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
#endif
*/
#if defined( __BTIF_EARPHONE__)
#if defined(IBRT)
#ifdef IBRT_SEARCH_UI
                    if(false==is_charging_poweron)
                        app_ibrt_enter_limited_mode();
#endif
#else
					power_on_open_reconnect_flag=1;//add by pang
                    app_bt_accessmode_set(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
#endif
#ifdef GFPS_ENABLED
                    app_enter_fastpairing_mode();
#endif
#if defined(__BTIF_AUTOPOWEROFF__)
                    //app_start_10_second_timer(APP_PAIR_TIMER_ID);
#endif
#endif
#ifdef __THIRDPARTY
                    app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO2,THIRDPARTY_BT_DISCOVERABLE);
#endif
                    break;
				/** add by cai **/
				case APP_POWERON_CASE_USB_AUDIO:
					app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);
#ifdef __THIRDPARTY
					app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO2,THIRDPARTY_BT_DISCOVERABLE);
#endif
					break;
				/** end add **/
                case APP_POWERON_CASE_NORMAL:
#if defined( __BTIF_EARPHONE__ ) && !defined(__EARPHONE_STAY_BOTH_SCAN__)
#if defined(IBRT)
#ifdef IBRT_SEARCH_UI
                    if(is_charging_poweron==false)
                    {
                        if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
                        {
                            TRACE(0,"ibrt_ui_log:power on unknow mode");
                            app_ibrt_enter_limited_mode();
                        }
                        else
                        {
                            TRACE(1,"ibrt_ui_log:power on %d fetch out", nvrecord_env->ibrt_mode.mode);
                            app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);
                        }
                    }
#elif defined(IS_MULTI_AI_ENABLED)
                    //when ama and bisto switch, earphone need reconnect with peer, master need reconnect with phone
                    //app_ibrt_ui_event_entry(IBRT_OPEN_BOX_EVENT);
                    //TRACE(1,"ibrt_ui_log:power on %d fetch out", nvrecord_env->ibrt_mode.mode);
                    //app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);
#endif
#else
                    app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);
#endif
#endif
                case APP_POWERON_CASE_REBOOT:
                case APP_POWERON_CASE_ALARM:
                default:
                    //app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
#if defined( __BTIF_EARPHONE__) && defined(__BTIF_BT_RECONNECT__) && !defined(IBRT)
					power_on_open_reconnect_flag=0;//add by pang
                    app_bt_profile_connect_manager_opening_reconnect();
#endif
#ifdef __THIRDPARTY
                    app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO2,THIRDPARTY_BT_CONNECTABLE);
#endif

                    break;
            }
            if (need_check_key)
            {
#ifndef __POWERKEY_CTRL_ONOFF_ONLY__
                app_poweron_wait_finished();
#endif
            }
            app_key_init();
            app_battery_start();
#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__)
            if(pwron_case != APP_POWERON_CASE_USB_AUDIO) app_start_10_second_timer(APP_POWEROFF_TIMER_ID);//m by cai for usb audio
#endif
#ifdef __THIRDPARTY
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_INIT);
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_START);
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO3,THIRDPARTY_START);
#endif

#ifdef RB_CODEC
            rb_ctl_init();
#endif

/** add by pang **/
#if defined(__CST816S_TOUCH__)
		ctp_hynitron_update();
		cst816s_open_module();
#endif
		
#ifdef ANC_APP
		//poweron_set_anc();
		//app_anc_Key_Pro(NULL, NULL);
		//app_anc_switch_turnled(true);
		//app_monitor_switch_turnled(false);
#endif
		app_user_event_open_module();
/** end add **/
        }else{
            af_close();
            app_key_close();
            nRet = -1;
        }
    }
exit:

#ifdef __BT_DEBUG_TPORTS__
    {
       extern void bt_enable_tports(void);
       bt_enable_tports();
       //hal_iomux_tportopen();
    }
#endif

#ifdef ANC_APP
    app_anc_set_init_done();
#endif
#ifdef BT_USB_AUDIO_DUAL_MODE
#if 0 //m by cai
    if(usb_plugin)
#else
	if((!app_demo_mode_poweron_flag_get() && app_nvrecord_demo_mode_get()) || hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter.pin))  ;
    else if(app_battery_is_charging() && !app_demo_mode_poweron_flag_get())//m by cai
#endif
	{
    	if(usb_plugin) usb_plugin = 1;
        btusb_switch(BTUSB_MODE_USB);
#ifdef ANC_APP
		poweron_set_anc();//add by cai for Pairing tone distortion
#endif
    }
    else
    {
        btusb_switch(BTUSB_MODE_BT);
    }
#else //BT_USB_AUDIO_DUAL_MODE
#if defined(BTUSB_AUDIO_MODE)
    if(pwron_case == APP_POWERON_CASE_CHARGING) {
#ifdef __WATCHER_DOG_RESET__
        app_wdt_close();
#endif
        af_open();
        app_key_handle_clear();
        app_usb_key_init();
        app_usbaudio_entry();
    }

#endif // BTUSB_AUDIO_MODE
#endif // BT_USB_AUDIO_DUAL_MODE
    #ifndef __PWM_LED_CTL__
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);
	#else //m by pang for pwm led
    if(!app_pwm_idle())
		app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_26M);
	else
		app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);
    #endif
	
    return nRet;
}

#endif /* APP_TEST_MODE */
