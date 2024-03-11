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
#include "cmsis_os.h"
#include "tgt_hardware.h"
#include "pmu.h"
#include "hal_timer.h"
#include "hal_gpadc.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_chipid.h"
#include "app_thread.h"
#include "app_battery.h"
#include "apps.h"
#include "app_status_ind.h"
#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif
#include <stdlib.h>

#ifdef __INTERCONNECTION__
#include "app_ble_mode_switch.h"
#endif

/** add by pang **/
#include "app_user.h"
#include "philips_ble_api.h"
#include "hal_bootmode.h"
#include "analog.h"//add by cai
#include "hal_usb.h"//add by cai

bool battery_pd_poweroff=0;


/** end add **/
#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
extern "C" bool app_usbaudio_mode_on(void);
#endif

#define APP_BATTERY_TRACE(s,...) TRACE(s, ##__VA_ARGS__)

static APP_BATTERY_MV_T batterylevel[]={4030,3920,3820,3740,3670,3620,3570,3510,3430};//add by pang
static bool charge_full_flag=0;//add by pang

#ifndef APP_BATTERY_MIN_MV
#define APP_BATTERY_MIN_MV (3430)
#endif

#ifndef APP_BATTERY_MAX_MV
#define APP_BATTERY_MAX_MV (4200)
#endif

#ifndef APP_BATTERY_PD_MV
#define APP_BATTERY_PD_MV   (3400)
#endif

#ifndef APP_BATTERY_CHARGE_TIMEOUT_MIN
#define APP_BATTERY_CHARGE_TIMEOUT_MIN (125)//(210)  m by cai
#endif

#ifndef APP_BATTERY_CHARGE_OFFSET_MV
#define APP_BATTERY_CHARGE_OFFSET_MV (50)//20
#endif

#ifndef CHARGER_PLUGINOUT_RESET
#define CHARGER_PLUGINOUT_RESET (1)
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_MS
#define CHARGER_PLUGINOUT_DEBOUNCE_MS (50)
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_CNT
#define CHARGER_PLUGINOUT_DEBOUNCE_CNT (3)
#endif

#define APP_BATTERY_CHARGING_PLUGOUT_DEDOUNCE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<500?3:1)

#define APP_BATTERY_CHARGING_EXTPIN_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<2*1000?2*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_EXTPIN_DEDOUNCE_CNT (6)

#define APP_BATTERY_CHARGING_OVERVOLT_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<2*1000?2*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_OVERVOLT_DEDOUNCE_CNT (3)

#define APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<20*1000?20*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT (6)


#define APP_BATTERY_REPORT_INTERVAL (5)

#define APP_BATTERY_MV_BASE ((APP_BATTERY_MAX_MV-APP_BATTERY_PD_MV)/(APP_BATTERY_LEVEL_NUM))

#define APP_BATTERY_STABLE_COUNT (5)
#define APP_BATTERY_MEASURE_PERIODIC_FAST_MS (200)
#ifdef BLE_ONLY_ENABLED
#define APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS (25000)
#else
#define APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS (10000)
#endif
#define APP_BATTERY_CHARGING_PERIODIC_MS (APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS)

#define APP_BATTERY_SET_MESSAGE(appevt, status, volt) (appevt = (((uint32_t)status&0xffff)<<16)|(volt&0xffff))
#define APP_BATTERY_GET_STATUS(appevt, status) (status = (appevt>>16)&0xffff)
#define APP_BATTERY_GET_VOLT(appevt, volt) (volt = appevt&0xffff)
#define APP_BATTERY_GET_PRAMS(appevt, prams) ((prams) = appevt&0xffff)

enum APP_BATTERY_MEASURE_PERIODIC_T
{
    APP_BATTERY_MEASURE_PERIODIC_FAST = 0,
    APP_BATTERY_MEASURE_PERIODIC_NORMAL,
    APP_BATTERY_MEASURE_PERIODIC_CHARGING,

    APP_BATTERY_MEASURE_PERIODIC_QTY,
};

struct APP_BATTERY_MEASURE_CHARGER_STATUS_T
{
    HAL_GPADC_MV_T prevolt;
    int32_t slope_1000[APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT];
    int slope_1000_index;
    int cnt;
};


typedef void (*APP_BATTERY_EVENT_CB_T)(enum APP_BATTERY_STATUS_T, APP_BATTERY_MV_T volt);

struct APP_BATTERY_MEASURE_T
{
    uint32_t start_time;
    enum APP_BATTERY_STATUS_T status;
#ifdef __INTERCONNECTION__
    uint8_t currentBatteryInfo;
    uint8_t lastBatteryInfo;
    uint8_t isMobileSupportSelfDefinedCommand;
#else
    uint8_t currlevel;
#endif
    APP_BATTERY_MV_T currvolt;
    APP_BATTERY_MV_T lowvolt;
    APP_BATTERY_MV_T highvolt;
    APP_BATTERY_MV_T pdvolt;
    uint32_t chargetimeout;
    enum APP_BATTERY_MEASURE_PERIODIC_T periodic;
    HAL_GPADC_MV_T voltage[APP_BATTERY_STABLE_COUNT];
    uint16_t index;
    struct APP_BATTERY_MEASURE_CHARGER_STATUS_T charger_status;
    APP_BATTERY_EVENT_CB_T cb;
    APP_BATTERY_CB_T user_cb;
};

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void);

static void app_battery_pluginout_debounce_start(void);
static void app_battery_pluginout_debounce_handler(void const *param);
osTimerDef (APP_BATTERY_PLUGINOUT_DEBOUNCE, app_battery_pluginout_debounce_handler);
static osTimerId app_battery_pluginout_debounce_timer = NULL;
static uint32_t app_battery_pluginout_debounce_ctx = 0;
static uint32_t app_battery_pluginout_debounce_cnt = 0;

static void app_battery_timer_handler(void const *param);
osTimerDef (APP_BATTERY, app_battery_timer_handler);
static osTimerId app_battery_timer = NULL;
static struct APP_BATTERY_MEASURE_T app_battery_measure;

static int app_battery_charger_handle_process(void);

#ifdef __INTERCONNECTION__
uint8_t* app_battery_get_mobile_support_self_defined_command_p(void)
{
    return &app_battery_measure.isMobileSupportSelfDefinedCommand;
}
#endif


void app_battery_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    uint8_t i;
    uint32_t meanBattVolt = 0;
    HAL_GPADC_MV_T vbat = volt;
	static uint8_t battery_level_init = 0;//add by cai
    APP_BATTERY_TRACE(2,"%s %d",__func__, vbat);
    if (vbat == HAL_GPADC_BAD_VALUE)
    {
        app_battery_measure.cb(APP_BATTERY_STATUS_INVALID, vbat);
        return;
    }

#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
    if(app_usbaudio_mode_on()) return ;
#endif
    app_battery_measure.voltage[app_battery_measure.index++%APP_BATTERY_STABLE_COUNT] = vbat<<2;

    if (app_battery_measure.index > APP_BATTERY_STABLE_COUNT)
    {
        for (i=0; i<APP_BATTERY_STABLE_COUNT; i++)
        {
            meanBattVolt += app_battery_measure.voltage[i];
        }
        meanBattVolt /= APP_BATTERY_STABLE_COUNT;
        if (app_battery_measure.cb)
        {
            if (meanBattVolt>app_battery_measure.highvolt)
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_OVERVOLT, meanBattVolt);
            }
            else if((meanBattVolt>app_battery_measure.pdvolt) && (meanBattVolt<app_battery_measure.lowvolt))
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_UNDERVOLT, meanBattVolt);
            }
            else if(meanBattVolt<app_battery_measure.pdvolt)
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_PDVOLT, meanBattVolt);
            }
            else
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_NORMAL, meanBattVolt);
            }
        }
    }
    else
    {
        int8_t level = 0;
        meanBattVolt = vbat<<2;
#if 0 
        level = (meanBattVolt-APP_BATTERY_PD_MV)/APP_BATTERY_MV_BASE;
#else   //m by cai
		for(i=0;i<9;i++){
			if(meanBattVolt>=batterylevel[i])
				break;
		}
		level=9-i;
#endif

        if (level<APP_BATTERY_LEVEL_MIN)
            level = APP_BATTERY_LEVEL_MIN;
        if (level>APP_BATTERY_LEVEL_MAX)
            level = APP_BATTERY_LEVEL_MAX;

        app_battery_measure.currvolt = meanBattVolt;
#ifdef __INTERCONNECTION__
        APP_BATTERY_INFO_T* pBatteryInfo = (APP_BATTERY_INFO_T*)&app_battery_measure.currentBatteryInfo;
        pBatteryInfo->batteryLevel = level;
#else
#if 0//m by cai
        app_battery_measure.currlevel = level+1;
#else
		if(battery_level_init==0)
		{
			app_battery_measure.currlevel = level+1;
			TRACE(2,"***%s: app_battery_measure.currlevel=%d",__func__,app_battery_measure.currlevel);
			battery_level_init=1;
		}
#endif
#endif
    }
}

static void app_battery_timer_start(enum APP_BATTERY_MEASURE_PERIODIC_T periodic)
{
    uint32_t periodic_millisec = 0;

    if (app_battery_measure.periodic != periodic){
        app_battery_measure.periodic = periodic;
        switch (periodic)
        {
            case APP_BATTERY_MEASURE_PERIODIC_FAST:
                periodic_millisec = APP_BATTERY_MEASURE_PERIODIC_FAST_MS;
                break;
            case APP_BATTERY_MEASURE_PERIODIC_CHARGING:
                periodic_millisec = APP_BATTERY_CHARGING_PERIODIC_MS;
                break;
            case APP_BATTERY_MEASURE_PERIODIC_NORMAL:
                periodic_millisec = APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS;
            default:
                break;
        }
        osTimerStop(app_battery_timer);
        osTimerStart(app_battery_timer, periodic_millisec);
    }
}

static void app_battery_timer_handler(void const *param)
{
    hal_gpadc_open(HAL_GPADC_CHAN_BATTERY, HAL_GPADC_ATP_ONESHOT, app_battery_irqhandler);
}

static void app_battery_event_process(enum APP_BATTERY_STATUS_T status, APP_BATTERY_MV_T volt)
{
    uint32_t app_battevt;
    APP_MESSAGE_BLOCK msg;

    APP_BATTERY_TRACE(3,"%s %d,%d",__func__, status, volt);
    msg.mod_id = APP_MODUAL_BATTERY;
    APP_BATTERY_SET_MESSAGE(app_battevt, status, volt);
    msg.msg_body.message_id = app_battevt;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_mailbox_put(&msg);

}

int app_battery_handle_process_normal(uint32_t status,  union APP_BATTERY_MSG_PRAMS prams)
{
    int8_t level = 0;
/** add by pang **/
	int8_t i = 0;
	static uint8_t lowbat_warning_time=0;
	static int8_t level_count=0;
/** end add **/
    switch (status)
    {
        case APP_BATTERY_STATUS_UNDERVOLT:
            TRACE(1,"UNDERVOLT:%d", prams.volt);
			app_status_indication_set(APP_STATUS_INDICATION_CHARGENEED);
#if 1 //m by cai
			//低电提示音每10分钟播报一次
			TRACE(2,"***batterylow: %d", lowbat_warning_time);
			if((lowbat_warning_time == 0) || (lowbat_warning_time == 60)){
#ifdef MEDIA_PLAYER_SUPPORT
				app_voice_report(APP_STATUS_INDICATION_CHARGENEED, 0);
#endif
				lowbat_warning_time = 0;
			}
			lowbat_warning_time++;
#else			
            app_status_indication_set(APP_STATUS_INDICATION_CHARGENEED);
#ifdef MEDIA_PLAYER_SUPPORT
#if defined(IBRT)

#else
            app_voice_report(APP_STATUS_INDICATION_CHARGENEED, 0);
#endif
#endif
#endif
        case APP_BATTERY_STATUS_NORMAL:
        case APP_BATTERY_STATUS_OVERVOLT:
            app_battery_measure.currvolt = prams.volt;
		#if 0
            level = (prams.volt-APP_BATTERY_PD_MV)/APP_BATTERY_MV_BASE;
		#else //m by pang
		    for(i = 0; i < 9; i++){
			if(app_battery_measure.currvolt >= batterylevel[i])
				break;
		    }
			level=9-i;	
		#endif

            if (level<APP_BATTERY_LEVEL_MIN)
                level = APP_BATTERY_LEVEL_MIN;
            if (level>APP_BATTERY_LEVEL_MAX)
                level = APP_BATTERY_LEVEL_MAX;
			
#ifdef __INTERCONNECTION__
            APP_BATTERY_INFO_T* pBatteryInfo;
            pBatteryInfo = (APP_BATTERY_INFO_T*)&app_battery_measure.currentBatteryInfo;
            pBatteryInfo->batteryLevel = level;
            if(level == APP_BATTERY_LEVEL_MAX)
            {
                level = 9;
            }
            else
            {
                level /= 10;
            }
#else
			TRACE(3,"***%s: volt=%d, level=%d", __func__, prams.volt, level);
			TRACE(1,"***app_battery_measure.currlevel=%d", app_battery_measure.currlevel);//add by cai
			//app_battery_measure.currlevel = level;//m by cai
#endif
			#if 0
            app_status_battery_report(level);
			#else //m by cai
			if(app_battery_measure.currlevel-1 != level)
				level_count++;
			else
				level_count = 0;

			if(level_count > 3){
				level_count = 0;
				app_battery_measure.currlevel = level+1;
				//#if defined(__HAYLOU_APP__)
				Notification_Battery_Level_Change();//notify app while battery level change
				//#endif
			}
			app_status_battery_report(app_battery_measure.currlevel-1);//m by cai	
			#endif
			app_10_second_timer_check();
            break;
        case APP_BATTERY_STATUS_PDVOLT:
//#ifndef BT_USB_AUDIO_DUAL_MODE //m by cai for open usb audio
            TRACE(1,"PDVOLT-->POWEROFF:%d", prams.volt);
			battery_pd_poweroff=1;//add by pang
            osTimerStop(app_battery_timer);
            app_shutdown();
//#endif
            break;
        case APP_BATTERY_STATUS_CHARGING:
            TRACE(1,"CHARGING-->APP_BATTERY_CHARGER :%d", prams.charger);
            if (prams.charger == APP_BATTERY_CHARGER_PLUGIN)
            {
#ifdef BT_USB_AUDIO_DUAL_MODE
                TRACE(1,"%s:PLUGIN.", __func__);
                btusb_switch(BTUSB_MODE_USB);
#else
#if CHARGER_PLUGINOUT_RESET
                app_reset();
#else
                app_battery_measure.status = APP_BATTERY_STATUS_CHARGING;
#endif
#endif
            }
            break;
        case APP_BATTERY_STATUS_INVALID:
        default:
            break;
    }
    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_NORMAL);
    return 0;
}

/** add by cai **/
uint8_t usb_plugout_to_proff_flag = 0;

uint8_t usb_plugout_to_proff_status_get(void)
{
	TRACE(2, "***%s: usb_plugout_to_proff_flag=%d", __func__,usb_plugout_to_proff_flag);

	return usb_plugout_to_proff_flag;
}

void usb_plugout_to_proff_status_set(uint8_t flag)
{
	TRACE(2, "***%s: usb_plugout_to_proff_flag=%d", __func__,flag);

	usb_plugout_to_proff_flag = flag;
}

osTimerId usb_plugout_timer = NULL;
static void app_usb_plugout_timehandler(void const *param);
osTimerDef(USB_PLUGOUT_SW_TIMER, app_usb_plugout_timehandler);// define timers
#define USB_PLUGOUT_TIMER_MS (1000)

void app_usb_plugout_start_timer(void)
{
	if(usb_plugout_timer == NULL)
			usb_plugout_timer = osTimerCreate(osTimer(USB_PLUGOUT_SW_TIMER), osTimerOnce, NULL);
	
	osTimerStart(usb_plugout_timer,USB_PLUGOUT_TIMER_MS);
}

void app_usb_plugout_stop_timer(void)
{	
	osTimerStop(usb_plugout_timer);
}

static void app_usb_plugout_timehandler(void const *param)
{
	if(app_battery_is_charging())
	{
		if(usb_plugout_to_proff_status_get())
		{
			if(app_poweroff_flag == 0)
			{
				if(get_usb_configured_status() || hal_usb_configured()) ;
				else {
					TRACE(0,"CHARGING-->RESET");
	                app_shutdown();
				}
				app_usb_plugout_start_timer();
			} else{
				app_usb_plugout_stop_timer();
			}
		}
	}
}
/** end add **/

int app_battery_handle_process_charging(uint32_t status,  union APP_BATTERY_MSG_PRAMS prams)
{
	/** add by cai **/
	int8_t level = 0;
	int8_t i = 0;
	static int8_t level_count=0;
	/** end add **/
	
    switch (status)
    {
        case APP_BATTERY_STATUS_OVERVOLT:
        case APP_BATTERY_STATUS_NORMAL:
        case APP_BATTERY_STATUS_UNDERVOLT:
            app_battery_measure.currvolt = prams.volt;
#if 0//m by cai
            app_status_battery_report(prams.volt);
#else
			for(i = 0;i < 9;i++){
				if(app_battery_measure.currvolt>=batterylevel[i])
					break;
			}
			level=9-i;
			
			if (level<APP_BATTERY_LEVEL_MIN)
				level = APP_BATTERY_LEVEL_MIN;
			if (level>APP_BATTERY_LEVEL_MAX)
				level = APP_BATTERY_LEVEL_MAX;

			TRACE(3,"***%s: volt=%d, level=%d", __func__, prams.volt, level);
			TRACE(1,"***app_battery_measure.currlevel=%d", app_battery_measure.currlevel);//add by cai
			
			if(app_battery_measure.currlevel-1 != level)
				level_count++;
			else
				level_count=0;

			if(level_count>3){
				level_count=0;
				app_battery_measure.currlevel = level+1;
				//#if defined(__HAYLOU_APP__)
				Notification_Battery_Level_Change();//notify app while battery level change
				//#endif
			}
			app_status_battery_report(app_battery_measure.currlevel-1);
#endif
            break;
        case APP_BATTERY_STATUS_CHARGING:
            TRACE(1,"CHARGING:%d", prams.charger);
            if (prams.charger == APP_BATTERY_CHARGER_PLUGOUT)
            {
#ifdef BT_USB_AUDIO_DUAL_MODE
                TRACE(1,"%s:PlUGOUT.", __func__);
#if 0 //m by cai
                btusb_switch(BTUSB_MODE_BT);
#else
				if(get_usb_configured_status() || hal_usb_configured()) {
					btusb_switch(BTUSB_MODE_BT);
					app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
				} else{
					TRACE(0,"CHARGING-->RESET");
					usb_plugout_to_proff_status_set(true);//add by cai
#if defined(__LDO_3V3_CTR__) 
					//if(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter.pin)) hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_pio_3_3v_control.pin);//add by cai for 3.5 line in pop noise
#endif
	                osTimerStop(app_battery_timer);
	                app_shutdown();
					app_usb_plugout_start_timer();//add by cai
				}
#endif

#else
#if CHARGER_PLUGINOUT_RESET
                TRACE(0,"CHARGING-->RESET");
                osTimerStop(app_battery_timer);
                app_shutdown();
#else
                app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
#endif
#endif
            }
            else if (prams.charger == APP_BATTERY_CHARGER_PLUGIN)
            {
#ifdef BT_USB_AUDIO_DUAL_MODE
                TRACE(1,"%s:PLUGIN.", __func__);
                //btusb_switch(BTUSB_MODE_USB); //m by cai for pop noise when insert usb
#endif
            }
            break;
        case APP_BATTERY_STATUS_INVALID:
        default:
            break;
    }

    if (app_battery_charger_handle_process()<=0)
    {
        if (app_status_indication_get() != APP_STATUS_INDICATION_FULLCHARGE)
        {
            TRACE(1,"FULL_CHARGING:%d", app_battery_measure.currvolt);
#ifdef __PWM_LED_CTL__
			app_pwm_clear();//disable pwm
#endif
            app_status_indication_set(APP_STATUS_INDICATION_FULLCHARGE);
#ifdef MEDIA_PLAYER_SUPPORT
#if defined(BT_USB_AUDIO_DUAL_MODE) || defined(IBRT)
#else
            //app_voice_report(APP_STATUS_INDICATION_FULLCHARGE, 0);
#endif
#endif
			/** add by cai **/
			charge_full_flag=1;

			if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
			{
				hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin);
			}
			/** end add **/
        }
		else {   //add by cai for current recharge
			if(app_battery_measure.currvolt < 4060 && !charge_protection_status_get())
			{
				if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
				{
					hal_gpio_pin_set((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin);
				}
				app_battery_measure.start_time = hal_sys_timer_get();
				app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
#ifdef __PWM_LED_CTL__
				apps_pwm_set(RED_PWM_LED, 1); //enable pwm	
#endif	
			}
		}
    }

    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_CHARGING);

    return 0;
}

static int app_battery_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint8_t status;
    union APP_BATTERY_MSG_PRAMS msg_prams;

    APP_BATTERY_GET_STATUS(msg_body->message_id, status);
    APP_BATTERY_GET_PRAMS(msg_body->message_id, msg_prams.prams);

    uint32_t generatedSeed = hal_sys_timer_get();
    for (uint8_t index = 0; index < sizeof(bt_addr); index++)
    {
        generatedSeed ^= (((uint32_t)(bt_addr[index])) << (hal_sys_timer_get()&0xF));
    }
    srand(generatedSeed);

    if (status == APP_BATTERY_STATUS_PLUGINOUT){
        app_battery_pluginout_debounce_start();
    }
    else
    {
        switch (app_battery_measure.status)
        {
            case APP_BATTERY_STATUS_NORMAL:
                app_battery_handle_process_normal((uint32_t)status, msg_prams);
                break;

            case APP_BATTERY_STATUS_CHARGING:
                app_battery_handle_process_charging((uint32_t)status, msg_prams);
                break;

            default:
                break;
        }
    }
    if (NULL != app_battery_measure.user_cb)
    {
        uint8_t batteryLevel;
#ifdef __INTERCONNECTION__
        APP_BATTERY_INFO_T* pBatteryInfo;
        pBatteryInfo = (APP_BATTERY_INFO_T*)&app_battery_measure.currentBatteryInfo;
        pBatteryInfo->chargingStatus = ((app_battery_measure.status == APP_BATTERY_STATUS_CHARGING)? 1:0);
        batteryLevel = pBatteryInfo->batteryLevel;

#else
        batteryLevel = app_battery_measure.currlevel;
#endif
        app_battery_measure.user_cb(app_battery_measure.currvolt,
                                    batteryLevel, app_battery_measure.status,status,msg_prams);
    }

    return 0;
}

int app_battery_register(APP_BATTERY_CB_T user_cb)
{
    if(NULL == app_battery_measure.user_cb)
    {
        app_battery_measure.user_cb = user_cb;
        return 0;
    }
    return 1;
}

int app_battery_get_info(APP_BATTERY_MV_T *currvolt, uint8_t *currlevel, enum APP_BATTERY_STATUS_T *status)
{
    if (currvolt)
    {
        *currvolt = app_battery_measure.currvolt;
    }

    if (currlevel)
    {
#ifdef __INTERCONNECTION__
        *currlevel = app_battery_measure.currentBatteryInfo;
#else
        *currlevel = app_battery_measure.currlevel;
#endif
		TRACE(2,"***%s: *currlevel=%d",__func__,*currlevel);
    }

    if (status)
    {
        *status = app_battery_measure.status;
    }

    return 0;
}

int app_battery_open(void)
{
    APP_BATTERY_TRACE(3,"%s batt range:%d~%d",__func__, APP_BATTERY_MIN_MV, APP_BATTERY_MAX_MV);
    int nRet = APP_BATTERY_OPEN_MODE_INVALID;

    if (app_battery_timer == NULL)
        app_battery_timer = osTimerCreate (osTimer(APP_BATTERY), osTimerPeriodic, NULL);

    if (app_battery_pluginout_debounce_timer == NULL)
        app_battery_pluginout_debounce_timer = osTimerCreate (osTimer(APP_BATTERY_PLUGINOUT_DEBOUNCE), osTimerOnce, &app_battery_pluginout_debounce_ctx);

    app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
#ifdef __INTERCONNECTION__
    app_battery_measure.currentBatteryInfo = APP_BATTERY_DEFAULT_INFO;
    app_battery_measure.lastBatteryInfo = APP_BATTERY_DEFAULT_INFO;
    app_battery_measure.isMobileSupportSelfDefinedCommand = 0;
#else
    app_battery_measure.currlevel = APP_BATTERY_LEVEL_MAX;
#endif
    app_battery_measure.currvolt = APP_BATTERY_MAX_MV;
    app_battery_measure.lowvolt = APP_BATTERY_MIN_MV;
    app_battery_measure.highvolt = APP_BATTERY_MAX_MV;
    app_battery_measure.pdvolt = APP_BATTERY_PD_MV;
    app_battery_measure.chargetimeout = APP_BATTERY_CHARGE_TIMEOUT_MIN;

    app_battery_measure.periodic = APP_BATTERY_MEASURE_PERIODIC_QTY;
    app_battery_measure.cb = app_battery_event_process;
    app_battery_measure.user_cb = NULL;

    app_battery_measure.charger_status.prevolt = 0;
    app_battery_measure.charger_status.slope_1000_index = 0;
    app_battery_measure.charger_status.cnt = 0;

    app_set_threadhandle(APP_MODUAL_BATTERY, app_battery_handle_process);

    if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_detecter_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_IN, 1);
    }

    if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_enable_cfg, 1);//m by pang
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 0);
    }

    if (app_battery_charger_indication_open() == APP_BATTERY_CHARGER_PLUGIN)
    {
        app_battery_measure.status = APP_BATTERY_STATUS_CHARGING;
        app_battery_measure.start_time = hal_sys_timer_get();
        //pmu_charger_plugin_config();
        if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
        {
            hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 1);//m by pang
        }

#if (CHARGER_PLUGINOUT_RESET == 0)
        nRet = APP_BATTERY_OPEN_MODE_CHARGING_PWRON;
#else
	#if defined(__DEFINE_DEMO_MODE__)
		if(app_nvrecord_demo_mode_get()){ //add by pang
			if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_CHARGING_POWEROFF || hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)cfg_hw_pio_3p5_jack_detecter.pin)){//m by cai
				nRet = APP_BATTERY_OPEN_MODE_CHARGING;
			}
			else if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_CHARGING_POWERON){
				nRet = APP_BATTERY_OPEN_MODE_NORMAL;
			}
			else {
				nRet = APP_BATTERY_OPEN_MODE_CHARGING_PWRON;
			}
		}
		else
	#endif
           nRet = APP_BATTERY_OPEN_MODE_CHARGING;
#endif
    }
    else
    {
        app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
        //pmu_charger_plugout_config();
        nRet = APP_BATTERY_OPEN_MODE_NORMAL;
    }

#if defined(__NTC_DETECT__)
	ntc_capture_open();//add by pang
#endif

#if defined(__DEFINE_DEMO_MODE__)
	hal_sw_bootmode_clear(HAL_SW_BOOTMODE_CHARGING_POWEROFF);//add by pang
	hal_sw_bootmode_clear(HAL_SW_BOOTMODE_CHARGING_POWERON);
#endif
    return nRet;
}

int app_battery_start(void)
{
    APP_BATTERY_TRACE(2,"%s %d",__func__, APP_BATTERY_MEASURE_PERIODIC_FAST_MS);

    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_FAST);

    return 0;
}

int app_battery_stop(void)
{
    osTimerStop(app_battery_timer);

    return 0;
}

int app_battery_close(void)
{
    hal_gpadc_close(HAL_GPADC_CHAN_BATTERY);

    return 0;
}

#if 0
static int32_t app_battery_charger_slope_calc(int32_t t1, int32_t v1, int32_t t2, int32_t v2)
{
    int32_t slope_1000;
    slope_1000 = (v2-v1)*1000/(t2-t1);
    return slope_1000;
}
#endif
static int app_battery_charger_handle_process(void)
{
    int nRet = 1;
    //int8_t i=0,cnt=0;
    //uint32_t slope_1000 = 0;
    uint32_t charging_min;
    static uint8_t overvolt_full_charge_cnt = 0;
    static uint8_t ext_pin_full_charge_cnt = 0;
	static uint8_t usb_audio_full_charge_cnt = 0;//add by cai

    charging_min = hal_sys_timer_get() - app_battery_measure.start_time;
    charging_min = TICKS_TO_MS(charging_min)/1000/60;
    if (charging_min >= app_battery_measure.chargetimeout)
    {
        TRACE(0,"TIMEROUT-->FULL_CHARGING");
        nRet = -1;
        goto exit;
    }

    if ((app_battery_measure.charger_status.cnt++%APP_BATTERY_CHARGING_OVERVOLT_MEASURE_CNT) == 0)
    {
        if (app_battery_measure.currvolt>=(app_battery_measure.highvolt+APP_BATTERY_CHARGE_OFFSET_MV))
        {
            overvolt_full_charge_cnt++;
        }
        else
        {
            overvolt_full_charge_cnt = 0;
        }
        if (overvolt_full_charge_cnt>=APP_BATTERY_CHARGING_OVERVOLT_DEDOUNCE_CNT)
        {
            TRACE(0,"OVERVOLT-->FULL_CHARGING");
            nRet = -1;
            goto exit;
        }
    }
	/** add by cai **/
#if defined(BT_USB_AUDIO_DUAL_MODE) || defined(BTUSB_AUDIO_MODE)
	if(get_usb_configured_status() || hal_usb_configured()) 
	{
		if(app_battery_measure.currvolt>=4190)
		{
			usb_audio_full_charge_cnt++;		
		}
		else
		{
			usb_audio_full_charge_cnt=0;
		}
		if(usb_audio_full_charge_cnt>=6)
		{
			TRACE(0,"USB AUDIO-->FULL_CHARGING");
			nRet = -1;
			goto exit;
		}
	}
#endif
	/** end add **/

    if ((app_battery_measure.charger_status.cnt%APP_BATTERY_CHARGING_EXTPIN_MEASURE_CNT) == 0)
    {
        if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
        {
            if (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin))
            {
				ext_pin_full_charge_cnt = 0;
				//ext_pin_full_charge_cnt++;
            }
            else
            {
				ext_pin_full_charge_cnt++;
				//ext_pin_full_charge_cnt = 0;
            }
            if (ext_pin_full_charge_cnt>=APP_BATTERY_CHARGING_EXTPIN_DEDOUNCE_CNT)
            {
                TRACE(0,"EXT PIN-->FULL_CHARGING");
                nRet = -1;
                goto exit;
            }
        }
    }
#if 0
    if ((app_battery_measure.charger_status.cnt%APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT) == 0)
    {
        if (!app_battery_measure.charger_status.prevolt)
        {
            app_battery_measure.charger_status.slope_1000[app_battery_measure.charger_status.slope_1000_index%APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT] = slope_1000;
            app_battery_measure.charger_status.prevolt = app_battery_measure.currvolt;
            for (i=0; i<APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT; i++)
            {
                app_battery_measure.charger_status.slope_1000[i]=100;
            }
        }
        else
        {
            slope_1000 = app_battery_charger_slope_calc(0, app_battery_measure.charger_status.prevolt,
                         APP_BATTERY_CHARGING_PERIODIC_MS*APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT/1000, app_battery_measure.currvolt);
            app_battery_measure.charger_status.slope_1000[app_battery_measure.charger_status.slope_1000_index%APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT] = slope_1000;
            app_battery_measure.charger_status.prevolt = app_battery_measure.currvolt;
            for (i=0; i<APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT; i++)
            {
                if (app_battery_measure.charger_status.slope_1000[i]>0)
                    cnt++;
                else
                    cnt--;
                TRACE(3,"slope_1000[%d]=%d cnt:%d", i,app_battery_measure.charger_status.slope_1000[i], cnt);
            }
            TRACE(3,"app_battery_charger_slope_proc slope*1000=%d cnt:%d nRet:%d", slope_1000, cnt, nRet);
            if (cnt>1)
            {
                nRet = 1;
            }/*else (3>=cnt && cnt>=-3){
                nRet = 0;
            }*/else
            {
                if (app_battery_measure.currvolt>=(app_battery_measure.highvolt-APP_BATTERY_CHARGE_OFFSET_MV))
                {
                    TRACE(0,"SLOPE-->FULL_CHARGING");
                    nRet = 1;//-1 by pang
                }
            }
        }
        app_battery_measure.charger_status.slope_1000_index++;
    }
#endif

exit:
    return nRet;
}

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    enum PMU_CHARGER_STATUS_T charger;

    charger = pmu_charger_get_status();

    if (charger == PMU_CHARGER_PLUGIN)
    {
        status = APP_BATTERY_CHARGER_PLUGIN;
        // TRACE(0,"force APP_BATTERY_CHARGER_PLUGIN");
    }
    else
    {
        status = APP_BATTERY_CHARGER_PLUGOUT;
        // TRACE(0,"force APP_BATTERY_CHARGER_PLUGOUT");
    }

    return status;
}

static void app_battery_charger_handler(enum PMU_CHARGER_STATUS_T status)
{
    TRACE(2,"%s: status=%d", __func__, status);
    pmu_charger_set_irq_handler(NULL);
    app_battery_event_process(APP_BATTERY_STATUS_PLUGINOUT,
                              (status == PMU_CHARGER_PLUGIN) ? APP_BATTERY_CHARGER_PLUGIN : APP_BATTERY_CHARGER_PLUGOUT);
}

static void app_battery_pluginout_debounce_start(void)
{
    TRACE(1,"%s", __func__);
    app_battery_pluginout_debounce_ctx = (uint32_t)app_battery_charger_forcegetstatus();
    app_battery_pluginout_debounce_cnt = 1;
    osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
}

static void app_battery_pluginout_debounce_handler(void const *param)
{
    enum APP_BATTERY_CHARGER_T status_charger = app_battery_charger_forcegetstatus();

    if(app_battery_pluginout_debounce_ctx == (uint32_t) status_charger){
        app_battery_pluginout_debounce_cnt++;
    }
    else
    {
        TRACE(2,"%s dithering cnt %u", __func__, app_battery_pluginout_debounce_cnt);
        app_battery_pluginout_debounce_cnt = 0;
        app_battery_pluginout_debounce_ctx = (uint32_t)status_charger;
    }

    if (app_battery_pluginout_debounce_cnt >= CHARGER_PLUGINOUT_DEBOUNCE_CNT){
        TRACE(2,"%s %s", __func__, status_charger == APP_BATTERY_CHARGER_PLUGOUT ? "PLUGOUT" : "PLUGIN");
        if (status_charger == APP_BATTERY_CHARGER_PLUGIN)
        {
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 1);//m by pang
            }
            app_battery_measure.start_time = hal_sys_timer_get();
        }
        else
        {
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 0);//m by pang
            }
        }
        app_battery_event_process(APP_BATTERY_STATUS_CHARGING, status_charger);
        pmu_charger_set_irq_handler(app_battery_charger_handler);
        osTimerStop(app_battery_pluginout_debounce_timer);
    }else{
        osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
    }
}

int app_battery_charger_indication_open(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    uint8_t cnt = 0;

    APP_BATTERY_TRACE(1,"%s",__func__);

    pmu_charger_init();

    do
    {
        status = app_battery_charger_forcegetstatus();
        if (status == APP_BATTERY_CHARGER_PLUGIN)
            break;
        osDelay(20);
    }
    while(cnt++<5);
/* //close by pang
    if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        if (!hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin))
        {
            status = APP_BATTERY_CHARGER_PLUGIN;
        }
    }
*/
    pmu_charger_set_irq_handler(app_battery_charger_handler);

    return status;
}

int8_t app_battery_current_level(void)
{
#ifdef __INTERCONNECTION__
    return app_battery_measure.currentBatteryInfo & 0x7f;
#else
    return app_battery_measure.currlevel;
#endif
}

int8_t app_battery_is_charging(void)
{
	TRACE(2,"***%s %d",__func__, app_battery_measure.status);

    return (APP_BATTERY_STATUS_CHARGING == app_battery_measure.status);
}

/** add by pang **/
bool app_battery_is_pdvolt(void)
{
    return (battery_pd_poweroff);
}

#if defined(__NTC_DETECT__)
void ntc_capture_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt);
static void app_ntc_timer_handler(void const *param);
osTimerDef (APP_NTC, app_ntc_timer_handler);
static osTimerId app_ntc_timer = NULL;

static void app_ntc_timer_handler(void const *param)
{
    hal_gpadc_open(HAL_GPADC_CHAN_2, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
}
#endif

#if 1 //1.97v reference volt ntcç”µé˜»10k
#define CHARGE_HIGH_TEMPERATURE         590     // 45C
#define CHARGE_LOW_TEMPERATURE     		1320    // 0C 
#define CHARGE_HIGH_TEMPERATURE_RECOVER	530		// 43C
#define CHARGE_LOW_TEMPERATURE_RECOVER  1270    // 2C

#define DISCHARGE_HIGH_TEMPERATURE 		380//400  	// 60C//55C
#define DISCHARGE_LOW_TEMPERATURE  		1587//1525//1730 	//-15C//-25C
#else //ntc 30k ok
//#define CHARGE_HIGH_TEMPERATURE         380     // 45C
//#define CHARGE_LOW_TEMPERATURE     		1150    // 0C 
//#define CHARGE_HIGH_TEMPERATURE_RECOVER	420		// 41C
//#define CHARGE_LOW_TEMPERATURE_RECOVER  1050    // 4C

//#define DISCHARGE_HIGH_TEMPERATURE 		255   	// 50C
//#define DISCHARGE_LOW_TEMPERATURE  		1185 	//-10C

//3v reference volt
#define CHARGE_HIGH_TEMPERATURE         340     // 45C 43+/-2
#define CHARGE_LOW_TEMPERATURE     		1000    // 0C  2+/-2
#define CHARGE_HIGH_TEMPERATURE_RECOVER	350		// 41C
#define CHARGE_LOW_TEMPERATURE_RECOVER  800    //  4C

#define DISCHARGE_HIGH_TEMPERATURE 		160   	// 60C 58+/-2
#define DISCHARGE_LOW_TEMPERATURE  		1350 	//-10C -8+/-2
#endif
#define TEMPERATURE_ERROT_COUNT 5

int8_t charge_temperature_error_num=0;
int8_t charge_temperature_valid_num=0;
bool charge_protection_flag=0;
int8_t discharge_temperature_error_num=0;

bool charge_protection_status_get(void)
{
	return charge_protection_flag;
}
/** end add **/


typedef uint16_t NTP_VOLTAGE_MV_T;
typedef uint16_t NTP_TEMPERATURE_C_T;

#define NTC_CAPTURE_STABLE_COUNT (5)
#define NTC_CAPTURE_TEMPERATURE_STEP (4)
#define NTC_CAPTURE_TEMPERATURE_REF (15)
#define NTC_CAPTURE_VOLTAGE_REF (1100)

typedef void (*NTC_CAPTURE_MEASURE_CB_T)(NTP_TEMPERATURE_C_T);

struct NTC_CAPTURE_MEASURE_T
{
    NTP_TEMPERATURE_C_T temperature;
    NTP_VOLTAGE_MV_T currvolt;
    NTP_VOLTAGE_MV_T voltage[NTC_CAPTURE_STABLE_COUNT];
    uint16_t index;
    NTC_CAPTURE_MEASURE_CB_T cb;
};

static struct NTC_CAPTURE_MEASURE_T ntc_capture_measure;

void ntc_capture_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    uint32_t meanVolt = 0;
    //TRACE(3,"%s %d irq:0x%04x",__func__, volt, irq_val);

    if (volt == HAL_GPADC_BAD_VALUE)
    {
        return;
    }

    ntc_capture_measure.voltage[ntc_capture_measure.index++%NTC_CAPTURE_STABLE_COUNT] = volt;

    if (ntc_capture_measure.index > NTC_CAPTURE_STABLE_COUNT)
    {
        for (uint8_t i=0; i<NTC_CAPTURE_STABLE_COUNT; i++)
        {
            meanVolt += ntc_capture_measure.voltage[i];
        }
        meanVolt /= NTC_CAPTURE_STABLE_COUNT;
        ntc_capture_measure.currvolt = meanVolt;
    }
    else if (!ntc_capture_measure.currvolt)
    {
        ntc_capture_measure.currvolt = volt;
    }
    ntc_capture_measure.temperature = ((int32_t)ntc_capture_measure.currvolt - NTC_CAPTURE_VOLTAGE_REF)/NTC_CAPTURE_TEMPERATURE_STEP + NTC_CAPTURE_TEMPERATURE_REF;
    pmu_ntc_capture_disable();
    TRACE(2,"%s ntc volt:%d",__func__, ntc_capture_measure.currvolt);

	/** add by pang **/
#if defined(__NTC_DETECT__)
	if(app_battery_is_charging()){
		discharge_temperature_error_num=0;
		if((ntc_capture_measure.currvolt<CHARGE_HIGH_TEMPERATURE)||(ntc_capture_measure.currvolt>CHARGE_LOW_TEMPERATURE)){
			charge_temperature_error_num++;	
			//charge_temperature_valid_num=0;
		}
		else{
			charge_temperature_error_num=0;
			//charge_temperature_valid_num++;
		}
		
        //charge over-temperature protection
		if(charge_temperature_error_num>TEMPERATURE_ERROT_COUNT){
			charge_temperature_error_num=TEMPERATURE_ERROT_COUNT+1;
			if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
			{
				hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin);
				charge_protection_flag=1;
				//app_pwm_clear();//disable pwm
				//app_status_indication_set(APP_STATUS_INDICATION_FULLCHARGE);
			}			
		}

		//charge recover
		if(charge_protection_flag){
			if((ntc_capture_measure.currvolt<CHARGE_HIGH_TEMPERATURE_RECOVER)||(ntc_capture_measure.currvolt>CHARGE_LOW_TEMPERATURE_RECOVER))
				charge_temperature_valid_num=0;
			else
			    charge_temperature_valid_num++;
		}
		else{
			charge_temperature_valid_num=0;
		}
		
		if(charge_temperature_valid_num>30){
			charge_temperature_valid_num=30+1;
			if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
			{
			    //if(0==charge_full_flag)
			    {
					hal_gpio_pin_set((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin);
					charge_protection_flag=0;
					app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
#ifdef __PWM_LED_CTL__
					apps_pwm_set(RED_PWM_LED, 1); //enable pwm	
#endif
			    }
			}
		}
	}
	else{
		charge_temperature_error_num=0;
		charge_temperature_valid_num=0;
		if((ntc_capture_measure.currvolt<DISCHARGE_HIGH_TEMPERATURE)||(ntc_capture_measure.currvolt>DISCHARGE_LOW_TEMPERATURE))
			discharge_temperature_error_num++;	
		else
			discharge_temperature_error_num=0;

		if(discharge_temperature_error_num>TEMPERATURE_ERROT_COUNT){
			discharge_temperature_error_num=TEMPERATURE_ERROT_COUNT+1;
			
			osTimerStop(app_battery_timer);
			osTimerStop(app_ntc_timer);
            app_shutdown();
		}
	}
#endif
	/** end add **/
}

int ntc_capture_open(void)
{
    ntc_capture_measure.currvolt = 0;
    ntc_capture_measure.index = 0;
    ntc_capture_measure.temperature = 0;
    ntc_capture_measure.cb = NULL;

    //pmu_ntc_capture_enable();
    //hal_gpadc_open(HAL_GPADC_CHAN_2, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);

/** add by pang **/
#if defined(__NTC_DETECT__)
    if (app_ntc_timer == NULL)
        app_ntc_timer = osTimerCreate (osTimer(APP_NTC), osTimerPeriodic, NULL);

	 osTimerStop(app_ntc_timer);
     osTimerStart(app_ntc_timer, 2000);

	 hal_gpio_pin_set((enum HAL_GPIO_PIN_T)Cfg_ntc_volt_ctr.pin);
#endif
/** end add */
    return 0;
}

int ntc_capture_start(void)
{
    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_2, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
    return 0;
}
