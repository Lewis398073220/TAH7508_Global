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

#ifdef MEDIA_PLAYER_SUPPORT

const uint8_t EN_POWER_ON [] = {
//#include "res/en/SOUND_POWER_ON.txt"
#include "res/en/EN_PowerOn_16000.txt"
};

const uint8_t EN_POWER_OFF [] = {
//#include "res/en/SOUND_POWER_OFF.txt"
#include "res/en/EN_PowerOFF_16000.txt"
};

const uint8_t EN_SOUND_ZERO[] = {
//#include "res/en/SOUND_ZERO.txt"
};

const uint8_t EN_SOUND_ONE[] = {
//#include "res/en/SOUND_ONE.txt"
};

const uint8_t EN_SOUND_TWO[] = {
//#include "res/en/SOUND_TWO.txt"
};

const uint8_t EN_SOUND_THREE[] = {
//#include "res/en/SOUND_THREE.txt"
};

const uint8_t EN_SOUND_FOUR[] = {
//#include "res/en/SOUND_FOUR.txt"
};

const uint8_t EN_SOUND_FIVE[] = {
//#include "res/en/SOUND_FIVE.txt"
};

const uint8_t EN_SOUND_SIX[] = {
//#include "res/en/SOUND_SIX.txt"
};

const uint8_t EN_SOUND_SEVEN [] = {
//#include "res/en/SOUND_SEVEN.txt"
};

const uint8_t EN_SOUND_EIGHT [] = {
//#include "res/en/SOUND_EIGHT.txt"
};

const uint8_t EN_SOUND_NINE [] = {
//#include "res/en/SOUND_NINE.txt"
};

const uint8_t EN_BT_PAIR_ENABLE[] = {
//#include "res/en/SOUND_PAIR_ENABLE.txt"
#include "res/en/EN_Pairing_16000.txt"
};

const uint8_t EN_BT_PAIRING[] = {
//#include "res/en/SOUND_PAIRING.txt"
};

const uint8_t EN_BT_PAIRING_FAIL[] = {
//#include "res/en/SOUND_PAIRING_FAIL.txt"
};

const uint8_t EN_BT_PAIRING_SUCCESS[] = {
//#include "res/en/SOUND_PAIRING_SUCCESS.txt"
};

const uint8_t EN_BT_REFUSE[] = {
//#include "res/en/SOUND_REFUSE.txt"
};

const uint8_t EN_BT_OVER[] = {
//#include "res/en/SOUND_OVER.txt"
};

const uint8_t EN_BT_ANSWER[] = {
//#include "res/en/SOUND_ANSWER.txt"
};

const uint8_t EN_BT_HUNG_UP[] = {
//#include "res/en/SOUND_HUNG_UP.txt"
};

const uint8_t EN_BT_CONNECTED [] = {
//#include "res/en/SOUND_CONNECTED.txt"
#include "res/en/EN_Connected_16000.txt"
};

const uint8_t EN_BT_DIS_CONNECT [] = {
//#include "res/en/SOUND_DIS_CONNECT.txt"
#include "res/en/EN_Disconnected_16000.txt"
};

const uint8_t EN_BT_INCOMING_CALL [] = {
//#include "res/en/SOUND_INCOMING_CALL.txt"
#include "res/en/EN_Ring_Digital_16000.txt"
};

const uint8_t EN_CHARGE_PLEASE[] = {
//#include "res/en/SOUND_CHARGE_PLEASE.txt"
#include "res/en/EN_Battery_low_16000.txt"
};

const uint8_t EN_CHARGE_FINISH[] = {
//#include "res/en/SOUND_CHARGE_FINISH.txt"
};

const uint8_t EN_LANGUAGE_SWITCH[] = {
//#include "res/en/SOUND_LANGUAGE_SWITCH.txt"
};

const uint8_t EN_BT_WARNING[] = {
//#include "res/en/SOUND_WARNING.txt"
#include "res/en/EN_Min_max3_16000.txt"
};

const uint8_t EN_BT_ALEXA_START[] = {
//#include "res/en/SOUND_ALEXA_START.txt"
};

const uint8_t EN_BT_ALEXA_STOP[] = {
//#include "res/en/SOUND_ALEXA_STOP.txt"
};

const uint8_t EN_BT_GSOUND_MIC_OPEN[] = {
//#include "res/en/SOUND_GSOUND_MIC_OPEN.txt"
};

const uint8_t EN_BT_GSOUND_MIC_CLOSE[] = {
//#include "res/en/SOUND_GSOUND_MIC_CLOSE.txt"
};

const uint8_t EN_BT_GSOUND_NC[] = {
//#include "res/en/SOUND_GSOUND_NC.txt"
};
#ifdef __INTERACTION__
const uint8_t EN_BT_FINDME[] = {
//#include "res/en/SOUND_FINDME.txt"
};
#endif
/** add by pang  **/
const uint8_t EN_BT_FINDME[] = {
#include "res/en/SOUND_FINDME.txt"
};

const uint8_t EN_BT_BEEP_21[] = {
#include "res/en/EN_beep21_16000.txt"
};

const uint8_t EN_BT_BEEP_22[] = {
#include "res/en/EN_beep22_16000.txt"
};

const uint8_t EN_POWEROFF_LOWBATTERY[] = {
#include "res/en/EN_PowerOFF_16000.txt"
};

const uint8_t EN_DEMO_MODE[] = {
#if defined(__DEFINE_DEMO_MODE__)
#include "res/en/EN_DEMO_MODE_ENABLED_16000.txt"
#endif
};

const uint8_t EN_CALLING_MUTE[] = {
#include "res/en/EN_Mute_16000.txt"
};

const uint8_t EN_CALLING_UNMUTE[] = {
#include "res/en/EN_Unmute_16000.txt"
};

const uint8_t EN_BT_SHORT_1[] = {
//#include "res/en/Short_1_8k.txt"
};

const uint8_t EN_BT_VOL_MAX[] = {
//#include "res/en/Middle9_8000_mono_16Bits_By_wav2sbc.txt"
};

const uint8_t EN_BT_FACTORY_RESET[] = {
//#include "res/en/Tone_16000.txt"
#include "res/en/EN_beep22_16000.txt"
};

const uint8_t EN_BT_ANC_ON[] = {
#include "res/en/EN_ANC_ON_16000.txt"
};

const uint8_t EN_BT_MONITOR_ON[] = {
#include "res/en/EN_Awareness_16000.txt"
};

const uint8_t EN_BT_ANC_OFF[] = {
#include "res/en/EN_ANC_OFF_16000.txt"
};

const uint8_t EN_BT_GAMING_ON[] = {
#include "res/en/Gaming_on_16000.txt"
};

const uint8_t EN_BT_GAMING_OFF[] = {
#include "res/en/Gaming_off_16000.txt"
};

const uint8_t EN_BT_LINK_LOST_OF_RANGE[] = {
//#include "res/en/5th_Up_Short_Short_16000.txt"
//#include "res/en/Short_Short_16000.txt"
//#include "res/en/beep_24s_16km_16000.txt"
};
/** end add  **/
#endif
