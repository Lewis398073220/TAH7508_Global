/***************************************************************************
 *
 * Copyright TPV.
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
#ifndef __PHILIPS_BLE_API_H__
#define __PHILIPS_BLE_API_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef enum PHILIPS_BASE_COMMAND
{
    GET_CONNECT_PHONE_MAC = 0x8000,			//0x8000
	GET_DEVICE_FEATURE_CONFIG,				//0x8001
	GET_API_VERSION,				        //0x8002
	GET_SOUND_QUALITY,						//0x8003
	GET_RIGHT_EAR_MAC_ADDRESS,				//0x8004
	GET_LEFT_EAR_MAC_ADDRESS,				//0x8005
	NOTIFICATION_SOUND_QUALITY_CHANGE,		//0x8006
	GET_BLE_MAC_ADDRESS,					//0x8007
	GET_SALES_REGION,						//0x8008
}PHILIPS_BASE_COMMAND;

typedef enum PHILIPS_FOTA_COMMAND
{
	GET_CHIPSET_VENDOR = 0x8010,	        //0x8010
	GET_CHIPSET_SOLUTION,					//0x8011
	GET_FW_VERSION,							//0x8012
	GET_DEVICE_TYPE,						//0x8013
	GET_FOTA_FINISH_FLAG,                   //0X8014
	SET_FOTA_FINISH_FLAG,                   //0X8015	
	GET_PCBA_VERSION,                       //0X8016
}PHILIPS_FOTA_COMMAND;

typedef enum PHILIPS_ANC_MODE_COMMAND
{
	GET_ANC_MODE_SUPPORT_LIST = 0x8020,	    //0x8020
	GET_ANC_MODE_STATUS = 0x8021,	        //0x8021
	SET_ANC_MODE_VALUE = 0x8022,	        //0x8022
	GET_AWARENESS_VALUE = 0x8023,	        //0x8023
	SET_AWARENESS_VALUE = 0x8024,	        //0x8024
	GET_ENHANCE_VOICE_STATUS = 0x8025,      //0x8025
	SET_ENHANCE_VOICE_STATUS = 0x8026,      //0x8026
	GET_ANC_TABLE_VALUE = 0x8027,           //0x8027
}PHILIPS_ANC_MODE_COMMAND;
	
typedef enum PHILIPS_AMBIENT_COMMAND
{
	GET_AMBIENT_SOUND_ITEM_STATUS = 0x8030,	       	//0x8030
	SET_AMBIENT_SOUND_ITEM_VALUE,					//0x8031
	GET_AMBIENT_RANGE_SUPPORT,                      //0x8032
	GET_AMBIENT_RANGE_TOTAL,                        //0x8033
	GET_AMBIENT_FOCUS_ON_VOICE_SUPPORT,             //0x8034
	GET_ADAPTIVE_STAND_D4_VALUE,                    //0x8035
	GET_ADAPTIVE_WALKING_D4_VALUE,                  //0x8036
	GET_ADAPTIVE_RUNNING_D4_VALUE,                  //0x8037
	GET_ADAPTIVE_TRAFFIC_D4_VALUE,                  //0x8038
	GET_AMBIENT_D4_VALUE,                           //0x8039
	GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_D4_VALUE,     //0x803a
	GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_D4_VALUE,   //0x803b
	GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_D4_VALUE,   //0x803c
	GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_D4_VALUE,   //0x803d
	GET_AMBIENT_FOCUS_ON_VOICE_D4_VALUE,            //0x803e
	GET_ADAPTIVE_STAND_CURRENT_VALUE,               //0x803f
	SET_ADAPTIVE_STAND_CURRENT_VALUE,               //0x8040
	GET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE,        //0x8041
	SET_ADAPTIVE_STAND_FOCUS_ON_VOICE_VALUE,        //0x8042
	GET_ADAPTIVE_WALKING_CURRENT_VALUE,             //0x8043
	SET_ADAPTIVE_WALKING_CURRENT_VALUE,             //0x8044
	GET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE,      //0x8045
    SET_ADAPTIVE_WALKING_FOCUS_ON_VOICE_VALUE,      //0x8046
	GET_ADAPTIVE_RUNNING_CURRENT_VALUE,             //0x8047
	SET_ADAPTIVE_RUNNING_CURRENT_VALUE,             //0x8048
	GET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE,      //0x8049
    SET_ADAPTIVE_RUNNING_FOCUS_ON_VOICE_VALUE,		//0x804a
    GET_ADAPTIVE_TRAFFIC_CURRENT_VALUE,             //0x804b
	SET_ADAPTIVE_TRAFFIC_CURRENT_VALUE,             //0x804c
	GET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE,      //0x804d
    SET_ADAPTIVE_TRAFFIC_FOCUS_ON_VOICE_VALUE,		//0x804e	
	GET_AMBIENT_CURRENT_VALUE,                      //0x804f
	SET_AMBIENT_CURRENT_VALUE,                      //0x8050
	GET_AMBIENT_FOCUS_ON_VOICE_STATUS,              //0x8051
	SET_AMBIENT_FOCUS_ON_VOICE_VALUE,               //0x8052
	GET_HW_NOSIE_CANCELLING_STATUS,                 //0x8053
    SET_HW_ANC_VALUE,                               //0x8054
    GET_HW_AMBIENT_VALUE,                           //0x8055
    SET_HW_AMBIENT_VALUE,                           //0x8056
    GET_HW_FOCUS_ON_VOICE_STATUS,                   //0x8057
	SET_HW_FOCUS_ON_VOICE_STATUS,                   //0x8058		
	NOTIFICATIION_NOSIE_CANCELLING_CHANGE= 0x8059,  //0x8059
}PHILIPS_AMBIENT_COMMAND;

typedef enum PHILIPS_EQUALIZER_COMMAND
{
	GET_EQUALIZER_SUPPORT_LIST = 0x8090,	        //0x8090
	GET_EQUALIZER_STATUS,					        //0x8091
	SET_EQUALIZER_STATUS_VALUE,					    //0x8092
	GET_EQUALIZER_STATUS_D4_VALUE,				    //0x8093
	GET_LAST_EQ_STATUS,							    //0x8094
	NOTIFICATIION_EQ_CHANGE,				        //0x8095
}PHILIPS_EQUALIZER_COMMAND;
	
typedef enum PHILIPS_CUSTOMIZATION_EQ_COMMAND
{
	GET_CUSTOMIZATION_EQ_SUPPORT_LIST = 0x80a0,	    //0x80a0
	GET_CUSTOMIZATION_EQ_BAND_VALUE,				//0x80a1
	SET_CUSTOMIZATION_EQ_BAND_VALUE,				//0x80a2
	GET_CUSTOMIZATION_EQ_BAND_RANGE_VALUE,			//0x80a3
	GET_CUSTOMIZATION_EQ_WITH_LIB,					//0x80a4
}PHILIPS_CUSTOMIZATION_EQ_COMMAND;

typedef enum PHILIPS_NOWPLAYING_COMMAND
{
	GET_NOWPLAYING_PLAYBACK_STATUS = 0x80b0,	    //0x80B0
	SET_NOWPLAYING_PLAYBACK_STATUS_VALUE,			//0x80B1
	SET_NOWPLAYING_PLAYBACK_NEXT,					//0x80B2
	SET_NOWPLAYING_PLAYBACK_PREVIOUS,				//0x80B3
	NOTIFICATION_MEDIA_CHANGE,						//0x80B4
	GET_MEDIA_TITLE,								//0x80B5
	GET_MEDIA_ARTIST,								//0x80B6
	GET_MEDIA_ALBUM,								//0x80B7
	GET_MEDIA_FUNTION_SUPPORT,						//0x80B8
}PHILIPS_NOWPLAYING_COMMAND;
	
typedef enum PHILIPS_HEART_RATE_COMMAND
{
	GET_HEART_RATE_STATUS = 0x80c0,	                //0x80C0
	SET_HEART_RATE_STATUS_VALUE,			        //0x80C1
	GET_HEART_RATE_CURRENT_VALUE,
	GET_HEART_RATE_AUTO_NODIFY_STATUS,
	SET_HEART_RATE_AUTO_NODIFY_VALUE,
}PHILIPS_HEART_RATE_COMMAND;
	
typedef enum PHILIPS_EAR_DETECTION_COMMAND
{
	GET_EAR_DETECTION_STATUS = 0x80d0,	            //0x80d0
	SET_EAR_DETECTION_STATUS_VALUE,			        //0x80d1
	GET_EAR_DETECTION_CURRENT_VALUE,				//0x80d2
	NOTIFICATION_EAR_DETECTION_CHANGE,				//0x80d3
	P_SENSOR_CALIBRATION,                           //0x80d4
	P_SENSOR_RESET,                                 //0x80d5
    SET_PAUSES_MUSIC_WHEN_TAKEOFF,                  //0x80d6
    GET_PAUSES_MUSIC_WHEN_TAKEOFF,                  //0x80d7
}PHILIPS_EAR_DETECTION_COMMAND;
	
typedef enum PHILIPS_BATTERY_COMMAND
{
	GET_BATTERY_CHARGE_STATUS = 0x80e0,	            //0x80e0
	GET_RIGHT_EAR_BATTERY_LEVEL_VALUE,				//0x80e1
	GET_LEFT_EAR_BATTERY_LEVEL_VALUE,				//0x80e2
	GET_CHARGE_BOX_BATTERY_LEVEL_VALUE,				//0x80e3	
	NOTIFICATION_BATTERY_LEVEL_CHANGE,				//0x80e4
	GET_BATTERY_NOTIFY_STATUS,						//0x80e5
	SET_BATTERY_NOTIFY_STATUS,						//0x80e6	
	NOTIFICATION_BATTERY_CHARGE_CHANGE,				//0x80e7
	SET_SLEEP_MODE_TIMER,							//0x80e8
	GET_SLEEP_MODE_TIMER,							//0x80e9
	GET_SMART_CHARGEBOX_SUPPORT,					//0x80ea
	GET_AUTO_POWER_OFF_SUPPORT,						//0x80eb
	GET_AUTO_POWER_OFF_TIMER,						//0x80ec
	SET_AUTO_POWER_OFF_TIMER,						//0x80eb
}PHILIPS_BATTERY_COMMAND;
	
typedef enum PHILIPS_LANGUAGE_VOICE_COMMAND
{
	GET_MULTI_LANGUAGE_VOICE_PROMPT_SUPPORT_LIST = 0x80f0,	                           //0x80f0
	GET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS,			//0x80F1
	SET_MULTI_LANGUAGE_VOICE_PROMPT_STATUS_VALUE,
	GET_MULTI_LANGUAGE_VOICE_PROMPT_D4_VALUE,
}PHILIPS_LANGUAGE_VOICE_COMMAND;

typedef enum PHILIPS_VOICE_ASSISTANT_COMMAND
{
	GET_VOICE_ASSISTANT_SUPPORT_LIST = 0x8100,	    //0x8100
	GET_VOICE_WAKEUP_SUPPORT_LIST,	                //0x8101
	GET_VOICE_ASSISTANT_STATUS,	                    //0x8102
	SET_VOICE_ASSISTANT_STATUS,	                    //0x8103
	GET_PUSH_AND_TALK_STATUS,	                    //0x8104
	SET_PUSH_AND_TALK_STATUS,	                    //0x8105
	GET_VOICE_WAKEUP_STATUS,	                    //0x8106
	SET_VOICE_WAKEUP_STATUS,	                    //0x8107				
}PHILIPS_VOICE_ASSISTANT_COMMAND;
	
typedef enum PHILIPS_SPECIAL_FUNCTION1_COMMAND
{
	GET_SPECIAL_FUNCTION1_SUPPORT_LIST = 0x8110,	//0x8110
	GET_TOUCH_STATUS,                               //0x8111
	SET_TOUCH_STATUS,	                            //0x8112		
	GET_RUNNING_LIGHT_STATUS,                       //0x8113
	SET_RUNNING_LIGHT_STATUS,	                    //0x8114	
	GET_SIDE_TONE_CONTROL_STATUS,                   //0x8115
	SET_SIDE_TONE_CONTROL_STATUS,	                //0x8116		
	GET_LOW_LATENCY_STATUS,                         //0x8117
	SET_LOW_LATENCY_STATUS,	                        //0x8118	
	GET_VIBRATION_STATUS,                           //0x8119
	SET_VIBRATION_STATUS,	                        //0x811a
	GET_DEVICE_COLOUR_STATUS,						//0x811b
	SET_DEVICE_COLOUR_STATUS,						//0x811c  RD using only
}PHILIPS_SPECIAL_FUNCTION1_COMMAND;

typedef enum PHILIPS_SPECIAL_FUNCTION2_COMMAND
{
	GET_SPECIAL_FUNCTION2_SUPPORT_LIST=0x8160,      //0x8160
	GET_SPECIAL_FUNCTION2_reserved1,
	SET_SPECIAL_FUNCTION2_reserved1,
	GET_TALK_MIC_LED_STATUS,                                                           
	SET_TALK_MIC_LED_STATUS,
	NOTIFICATIION_TALK_MIC_LED_CHANGE,
}PHILIPS_SPECIAL_FUNCTION2_COMMAND;

typedef enum PHILIPS_MULTIPOINT_COMMAND
{
	GET_MULTIPOINT_STATUS=0x8170,                   //0x8170
	SET_MULTIPOINT_STATUS,	                        //0x8171						
}PHILIPS_MULTIPOINT_COMMAND;

typedef enum PHILIPS_NOWPLAYING2_COMMAND
{
	GET_NOWPLAYING2_SUPPORT_LIST = 0x8180,			//0x8180
	GET_NOWPLAYING2_PLAYBACK_STATUS,	        	//0x8181
	SET_NOWPLAYING2_PLAYBACK_STATUS,			    //0x8182
	SET_NOWPLAYING2_PLAYBACK_NEXT,					//0x8183
	SET_NOWPLAYING2_PLAYBACK_PREVIOUS,				//0x8184
	GET_NOWPLAYING2_MEDIA_TITLE,					//0x8185
	GET_NOWPLAYING2_MEDIA_ARTIST,	   		     	//0x8186
	GET_NOWPLAYING2_MEDIA_ALBUM,					//0x8187
	NOTIFICATION_MEDIA_INFO_CHANGE,   				//0x8188
	NOTIFICATION_PLAYBACK_STATUS_CHANGE,			//0x8189
}PHILIPS_NOWPLAYING2_COMMAND;

typedef enum PHILIPS_KEY_DEFINE_CHANGE_COMMAND
{
	GET_KEY_DEFINE_SUPPORT_LIST=0x81B0,             //0x81B0
	GET_UX_ANC_TOGGLE_STATUS,	                    //0x81B1
	SET_UX_ANC_TOGGLE_STATUS,	                    //0x81B2
}PHILIPS_KEY_DEFINE_CHANGE_COMMAND;

//Byte 0
#define FOTA_SUPPORT 				  0x01
#define ANC_MODE_SUPPORT 			  0x02
#define SMART_ANC_SUPPORT 	          0x04
#define EQUALIZER_SUPPORT 			  0x08
#define CUSTOMIZATION_EQ_SUPPORT 	  0x10
#define NOWPLAYING_SUPPORT 		 	  0x20
#define HEART_RATE_SUPPORT 			  0x40
#define EAR_DETECTION_SUPPORT 		  0x80
//Byte 1
#define BATTERY_STATUS_SUPPORT 				     0x01
#define MULTI_LANGAUAGE_VOICE_PROMPT_SUPPORT 	 0x02
#define AI_VOICE_ASSISTANT_SUPPORT 	             0x04
#define SPECIAL_FUNCTION1_SUPPORT 			     0x08
#define INTERNAL_RTC_SUPPORT 	        		 0x10
#define KIDS_MODE_SUPPORT 		        		 0x20
#define SPECIAL_FUNCTION2_SUPPORT 		  		 0x40
#define MULTIPOINT_SUPPORT                		 0x80
//Byte 2
#define NOWPLAYING2_0_SUPPORT 				     0x01
#define KEY_DEFINE_CHANGE_SUPPORT 	 			 0x02

//Key Define
#define UX_ANC_TOGGLE_SUPPORT 				     0x01
#define UX_VOLUME_SUPPORT 	 				  	 0x02

//ANC
#define ANC_OFF_SUPPORT 					     0x01
#define ANC_MODE1_SUPPORT 	       			     0x02
#define ANC_MODE2_SUPPORT 	       				 0x04
#define ANC_MODE3_SUPPORT 	       				 0x08
#define ANC_MODE4_SUPPORT 	       				 0x10
#define AWARENESS_SUPPORT 	      		         0x20
#define ENHANCE_VOICE_SUPPORT 	 				 0x40
#define AWARENESS_SLIDE_SUPPORT 			     0x80

//Ambient
#define NOT_SUPPORT 			  0x00
#define SUPPORT 			      0x01
//Ambient Total Level
#define AMBIENT_TOTAL_5 		  0x05
#define AMBIENT_TOTAL_10 		  0x0a
#define AMBIENT_TOTAL_15 		  0x0f
#define AMBIENT_TOTAL_20 		  0x14

//Equalizer support list
#define EQUALIZER_DISABLE		  0x00
#define EQUALIZER_BASS_BOOSTER 	  0x01
#define EQUALIZER_CLASSICAL 	  0x02
#define EQUALIZER_JAZZ   		  0x04
#define EQUALIZER_HIP_HOP		  0x08
#define EQUALIZER_DANCE 	      0x10
#define EQUALIZER_POP 	          0x20
#define EQUALIZER_ELECTRONIC      0x40
#define EQUALIZER_TREBLE_BOOSTER  0x80

#define EQUALIZER_ACOUSTIC 		  0x01
#define EQUALIZER_BASS_REDUCER	  0x02
#define EQUALIZER_LATIN        	  0x04
#define EQUALIZER_LOUDNESS        0x08
#define EQUALIZER_LOUNGE	      0x10
#define EQUALIZER_PIANO	          0x20
#define EQUALIZER_DEEP            0x40
#define EQUALIZER_R_B             0x80

#define EQUALIZER_SMALL_SPEAKERS  0x01
#define EQUALIZER_SPOKEN_WORD     0x02
#define EQUALIZER_FLAT        	  0x04
#define EQUALIZER_TREBLE_REDUCER  0x08
#define EQUALIZER_BOCAL_BOOSTER   0x10

//Equalizer value
#define DISABLE					  0x00
#define BASS_BOOSTER 			  0x01
#define CLASSICAL 		 		  0x02
#define JAZZ   			 		  0x03
#define HIP_HOP					  0x04
#define DANCE 	                  0x05
#define POP 	             	  0x06
#define ELECTRONIC       		  0x07
#define TREBLE_BOOSTER    		  0x08
#define ACOUSTIC 				  0x09
#define BASS_REDUCER			  0x0a
#define LATIN        			  0x0b
#define LOUDNESS       	  		  0x0c
#define LOUNGE	                  0x0d
#define PIANO	               	  0x0e
#define DEEP               	 	  0x0f
#define R_B                       0x10
#define SMALL_SPEAKERS   		  0x11
#define SPOKEN_WORD       		  0x12
#define FLAT                      0x13
#define TREBLE_REDUCER    		  0x14
#define BOCAL_BOOSTER     		  0x15

//Custom EQ support list
#define NOT_SUPPORT			      0x00
#define BRAND0_SUPPORT            0x01
#define BRAND1_SUPPORT            0x02
#define BRAND2_SUPPORT            0x04
#define BRAND3_SUPPORT            0x08
#define BRAND4_SUPPORT            0x10
#define BRAND5_SUPPORT            0x20

//Language voice prompt support
#define NOT_SUPPORT			      0x00
#define ENGLISH           		  0x01
#define CHINESE                   0x02
#define RUSSIAN                   0x04
#define INDONESIAN          	  0x08
#define TURKISH               	  0x10
#define GEMAN            	      0x20
#define ITALIAN                   0x40
#define JAPANESE                  0x80

#define FRENCH                    0x01
#define THAI                      0x02
#define PROTUGUESE                0x04
#define SPANISH                   0x08
#define VIETNAMESE         	      0x10
#define ARABIC          	      0x20
#define KOREAN                    0x40
#define MALAY                     0x80

#define HINDI                     0x01
#define URDU                      0x02
#define BENGALI                   0x04

//Language voice prompt select
#define TONE_SELECT			      0x00
#define ENGLISH_SELECT            0x01
#define CHINESE_SELECT            0x02
#define RUSSIAN_SELECT            0x03
#define INDONESIAN_SELECT         0x04
#define TURKISH_SELECT            0x05
#define GEMAN_SELECT              0x06
#define ITALIAN_SELECT            0x07
#define JAPANESE_SELECT           0x08

#define FRENCH_SELECT             0x09
#define THAI_SELECT               0x0a
#define PROTUGUESE_SELECT         0x0b
#define SPANISH_SELECT            0x0c
#define VIETNAMESE_SELECT         0x0d
#define ARABIC_SELECT          	  0x0e
#define KOREAN_SELECT             0x0f
#define MALAY_SELECT              0x10

#define HINDI_SELECT              0x11
#define URDU_SELECT               0x12
#define BENGALI_SELECT            0x13

//Voice Assistant support list
#define PUSH_AND_TALK_SUPPORT 	  0x00
#define Google_SUPPORT 			  0x01
#define Alexa_SUPPORT 	  		  0x02
#define Tencent_SUPPORT 	      0x04

//Special function 1 support list
#define SIRIi_BISTO_SPECIAL_KEY_SUPPORT 	0x01
#define TOUCH_LOCK_SUPPORT 			 	  	0x02
#define RUNNING_LIGHT_SUPPORT 	  			0x04
#define SIDE_TONE_SUPPORT 	                0x08
#define LOW_LATENCY_SUPPORT 	            0x10
#define VIBRATION_SUPPORT 	                0x20
#define DEVICE_COLOUR_SUPPORT 	   			0x40

//Special function 2 support list
#define LDAC_CONTROL_SUPPORT 				0x01
#define TALK_MIC_LED_CONTROL_SUPPORT 		0x02

extern uint8_t title[150], title_len;
extern uint8_t artist[150], artist_len;
extern uint8_t album[150], album_len;

void Philips_Api_protocol_port(uint8_t port);
bool Philips_Headphone_Api_Entry(uint8_t *data, uint32_t size);
bool CheckCommandID(uint8_t *data);
bool Command_CheckSum(uint8_t *data, uint8_t size);
uint8_t Do_CheckSum(uint8_t *data, uint8_t size);
bool Philips_Functions_Call(uint8_t *data, uint8_t size);
void Philips_Send_Notify(uint8_t *data, uint32_t size);
void Notification_Battery_Level_Change(void);
void Notification_Playback_Status_Change(uint8_t playstatus);
void Notification_Media_Info_Change(uint8_t media_info);
void Notification_Sound_Quality_Change(void);
void Notification_Nosie_Cancelling_Change(void);
#ifdef __cplusplus
}
#endif

#endif // #ifndef __PHILIPS_BLE_API_H__