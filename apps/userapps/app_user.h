#ifndef __USER_APPS_H__
#define __USER_APPS_H__

#include "../../../apps/anc/inc/app_anc.h"

#if defined(__PWM_LED_CTL__)
#include "hal_cmu.h"
#include "hal_pwm.h"

#define COL1_PWM_LED 	HAL_PWM_ID_1  //P2_5
#define COL2_PWM_LED 	HAL_PWM_ID_2  //P2_6
#define RED_PWM_LED 	HAL_PWM_ID_3  //P2_7

#define RED_LED 	0
#define BLUE_LED 	1
#define GREEN_LED 	2

#define PWM_IDLE 0
#define PWM_EN 1
#define BlINK_1x 2
#define BLINK_1S 3
#define BLINK_5S 4
#define BlINK_2x 5

#define LED_ON  2
#define LED_OFF_1S 5
#define LED_OFF_3S 15
#define LED_OFF_5S 25
#endif

#if defined(__USE_MOTOR_CTL__)
#define MOTOR_POWER_ON  500
#define MOTOR_POWER_OFF 500
#define MOTOR_PAIRING   200
#define MOTOR_CALLING   200
#define MOTOR_ENDCALL   500
#define MOTOR_MUTE      200
#define MOTOR_UNMUTE    200
#endif


#if defined(__EVRCORD_USER_DEFINE__)
#include "nvrecord_env.h"
#include "../../../services/multimedia/audio/process/filters/include/iir_process.h"
#define DEMO_MODE 0xEE

#define SLEEP_TIME_3MIN  18//24
#define SLEEP_TIME_5MIN  30//39
#define SLEEP_TIME_10MIN 60//77
#define SLEEP_TIME_PERM  255

#define DEFAULT_SLEEP_TIME SLEEP_TIME_5MIN

#define AUTO_PWOFF_TIME_30MIN  180//212
#define AUTO_PWOFF_TIME_1HOUR  360//431
#define AUTO_PWOFF_TIME_2HOUR  720//862
#define AUTO_PWOFF_TIME_4HOUR  1440//1724
#define AUTO_PWOFF_TIME_6HOUR  2160//2586
#define AUTO_PWOFF_TIME_PERM   4095
#define DEFAULT_AUTO_PWOFF_TIME AUTO_PWOFF_TIME_PERM
extern IIR_CFG_T eq_custom_para_ancon;
extern IIR_CFG_T eq_custom_para_ancoff;
extern IIR_CFG_T eq_custom_para;
extern IIR_CFG_T eq_custom_para_anc_off;
#if defined(AUDIO_LINEIN)
extern IIR_CFG_T eq_custom_para_linein;
#endif

#endif

#if defined(__USE_3_5JACK_CTR__)
extern bool reconncect_null_by_user;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int app_user_event_open_module(void);
int app_user_event_open_module_for_charge(void);
void app_user_event_close_module(void);

#if defined(__USE_3_5JACK_CTR__)
bool apps_3p5_jack_get_val(void);
bool  apps_3p5jack_plugin_check(void);	
bool app_apps_3p5jack_plugin_flag(bool clearcount);
void app_jack_start_timer(void);
void app_jack_stop_timer(void);
void apps_jack_event_process(void);
#endif

#if defined(__USE_IR_CTL__)
void app_ir_start_timer(void);
void app_ir_stop_timer(void);
void apps_ir_event_process(void);
#endif

#if defined(__PWM_LED_CTL__)
void app_pwm_disable(void);
void app_pwm_start_timer(void);
void app_pwm_stop_timer(void);
void apps_pwm_event_process(void);
void app_pwm_clear(void);
void apps_pwm_set(enum HAL_PWM_ID_T id, uint8_t enable);
void apps_blink_nx_set(uint8_t id, uint8_t enable, uint8_t period_on, uint8_t period_off);
void apps_blink_1x_set(uint8_t id,uint8_t period_on);
bool app_pwm_idle(void);
#endif

#if defined(__USE_MOTOR_CTL__)
void app_motor_init_timer(void);
void app_motor_timer_set(uint8_t vibrate_num, uint32_t vibrate_ms,uint16_t delay_ms);
void app_motor_timer_close(void);
#endif

#if defined(__USE_AMP_MUTE_CTR__)
void app_amp_open_start_timer(void);
void app_amp_open_stop_timer(void);
void apps_amp_switch_event_process(void);
void play_reboot_set(void);
#endif

#if defined(__AUDIO_FADEIN__)
uint8_t get_allow_to_fadein(void);
void set_allow_to_fadein(uint8_t on);
void app_audio_fadein_start(uint8_t audio_type);
void apps_audio_fadein_event_process(void);
#endif

#if defined(__EVRCORD_USER_DEFINE__)
uint8_t app_get_fota_flag(void);
void app_nvrecord_fotaflag_set(uint8_t on);
enum APP_ANC_MODE_STATUS app_nvrecord_anc_status_get(void);
uint8_t app_nvrecord_anc_table_get(void);
void app_nvrecord_anc_set(enum APP_ANC_MODE_STATUS nc);
uint8_t app_get_monitor_level(void);
void app_nvrecord_monitor_level_set(uint8_t level);
uint8_t app_get_focus(void);
void app_focus_set_no_save(uint8_t focus);
void app_nvrecord_focus_set(uint8_t focus);
uint8_t app_eq_index_get(void);
void app_eq_index_set_nosave(uint8_t eq_index);
void app_nvrecord_eq_set(uint8_t eq_index);
//void app_eq_para_get(uint8_t *p);
void app_eq_custom_para_get(uint8_t customization_eq_value[6]);
void app_nvrecord_eq_param_set(uint8_t customization_eq_value[6]);
void app_nvrecord_eq_param2_set(void);

uint8_t app_nvrecord_anc_get(void);
void app_nvrecord_demo_mode_set(uint8_t mod);
uint8_t app_nvrecord_demo_mode_get(void);
uint8_t app_get_sleep_time(void);
uint8_t get_sleep_time(void);
void app_nvrecord_sleep_time_set(uint8_t sltime);
uint8_t app_get_sleep_time(void);
uint8_t app_get_auto_poweroff(void);
uint16_t get_auto_pwoff_time(void);
void app_auto_poweroff_set(uint16_t pftime);
uint8_t app_get_vibrate_mode(void);
void app_nvrecord_vibrate_mode_set(uint8_t mod);
uint8_t app_get_monitor_level(void);
uint8_t app_get_focus(void);
void app_nvrecord_monitor_level_set(uint8_t level);
void app_nvrecord_focus_set(uint8_t focus);
uint8_t app_get_focus(void);
void app_nvrecord_sensor_set(uint8_t on);
uint8_t app_get_touchlock(void);
void app_nvrecord_touchlock_set(uint8_t on);
uint8_t app_get_sidetone(void);
void app_nvrecord_sidetone_set(uint8_t on);
uint8_t app_get_low_latency_status(void);
void app_low_latency_set(uint8_t on);
uint8_t app_get_fota_flag(void);
void app_nvrecord_fotaflag_set(uint8_t on);
uint8_t app_get_new_multipoint_flag(void);
uint8_t app_get_multipoint_flag(void);
void app_nvrecord_multipoint_set(uint8_t on);
enum ANC_TOGGLE_MODE app_nvrecord_anc_toggle_mode_get(void);
void app_nvrecord_anc_toggle_mode_set(enum ANC_TOGGLE_MODE nc_toggle);
void app_nvrecord_language_set(uint8_t lang);
void app_nvrecord_demo_mode_set(uint8_t mod);
uint8_t app_nvrecord_demo_mode_get(void);
void app_demo_mode_poweron_flag_set(uint8_t powron);
uint8_t app_demo_mode_poweron_flag_get(void);
uint8_t app_color_change_flag_get(void);
void app_nvrecord_color_change_flag_set(uint8_t flag);
uint8_t app_color_value_get(void);
void app_nvrecord_color_value_set(uint8_t color_val);
void app_nvrecord_para_get(void);
uint8_t app_get_TalkMicLed_flag(void);
void app_nvrecord_TalkMicLed_set(uint8_t on);
void app_nvrecord_para_default(void);
#endif

#if defined(CUSTOM_BIN_CONFIG)
void app_get_custom_bin_config(void);
uint8_t get_custom_bin_config(uint8_t config_num);
#endif

#if defined(__HAYLOU_APP__)
void app_api_start(void);
void app_api_stop(void);
void apps_api_event_process(void);
#endif


#ifdef __cplusplus
	}//extern "C" {
#endif

#endif
