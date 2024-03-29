CHIP		?= best2300p

DEBUG       ?= 1
PC_CMD_UART ?= 0

FPGA        ?= 0

MBED        ?= 0

RTOS        ?= 1

KERNEL      ?= RTX
#KERNEL      ?= FREERTOS

export SPEECH_SIDETONE ?= 1

#add by pang
CALIB_SLOW_TIMER ?= 1

#add by pang
SUSPEND_ANOTHER_DEV_A2DP_STREAMING_WHEN_CALL_IS_COMING ?= 1
export PLAYBACK_FORCE_48K ?= 0

LIBC_ROM    ?= 1

export USER_SECURE_BOOT	?= 0
# enable:1
# disable:0

WATCHER_DOG ?= 1

DEBUG_PORT  ?= 1
# 0: usb
# 1: uart0
# 2: uart1

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

export PLAYBACK_FORCE_48K ?= 0

export FORCE_SIGNALINGMODE ?= 0

export FORCE_NOSIGNALINGMODE ?= 0

export FORCE_SCO_MAX_RETX ?= 0

export FA_RX_GAIN_CTRL ?= 0

export CONTROLLER_DUMP_ENABLE ?= 0

export INTERSYS_DEBUG ?= 0

export PROFILE_DEBUG ?= 0

export BTDUMP_ENABLE ?= 0

export BT_DEBUG_TPORTS ?= 0

export SNIFF_MODE_CHECK ?= 0

AUDIO_OUTPUT_MONO ?= 0

AUDIO_OUTPUT_DIFF ?= 1

#### ANC DEFINE START ######
export ANC_APP		    ?= 1
export ANC_FF_ENABLED	?= 1
export ANC_FB_ENABLED	?= 1
export ANC_WNR_ENABLED	?= 0
export AUDIO_ANC_FB_MC ?= 1
export AUDIO_SECTION_SUPPT ?= 1
export AUD_SECTION_STRUCT_VERSION ?= 2
export ANC_CAP_ADC_EN_WORKAROUND ?= 0
##### ANC DEFINE END ######

HW_FIR_EQ_PROCESS ?= 0

SW_IIR_EQ_PROCESS ?= 1

HW_DAC_IIR_EQ_PROCESS ?= 0

HW_IIR_EQ_PROCESS ?= 0

HW_DC_FILTER_WITH_IIR ?= 0

AUDIO_DRC ?= 0

AUDIO_DRC2 ?= 0

#PC_CMD_UART ?= 0

AUDIO_SECTION_ENABLE ?= 0

# MC=1,A2DP_PLAYER_USE_BT_TRIGGER=0, AUDIO_RESAMPLE=0 by pang 
A2DP_PLAYER_USE_BT_TRIGGER ?= 0
AUDIO_RESAMPLE ?= 0

ANC_FB_MC_96KHZ ?= 0

RESAMPLE_ANY_SAMPLE_RATE ?= 1

OSC_26M_X4_AUD2BB ?= 1

AUDIO_OUTPUT_VOLUME_DEFAULT ?= 13
# range:1~16

AUDIO_INPUT_CAPLESSMODE ?= 0

AUDIO_INPUT_LARGEGAIN ?= 0

AUDIO_CODEC_ASYNC_CLOSE ?= 0

AUDIO_SCO_BTPCM_CHANNEL ?= 1

export A2DP_CP_ACCEL ?= 1

export SCO_CP_ACCEL ?= 1

export SCO_TRACE_CP_ACCEL ?= 0

# For TWS SCO DMA snapshot and low delay
export PCM_FAST_MODE ?= 1

export CVSD_BYPASS ?= 1

export LOW_DELAY_SCO ?= 0

SPEECH_TX_DC_FILTER ?= 1

SPEECH_TX_AEC2FLOAT ?= 1

SPEECH_TX_AEC3 ?= 0

SPEECH_TX_NS3 ?= 1

SPEECH_TX_2MIC_NS2 ?= 0

SPEECH_TX_COMPEXP ?= 1

SPEECH_TX_EQ ?= 1

SPEECH_TX_POST_GAIN ?= 0

SPEECH_RX_NS2FLOAT ?= 1

SPEECH_RX_EQ ?= 1

SPEECH_RX_POST_GAIN ?= 0

LARGE_RAM ?= 1

HSP_ENABLE ?= 0

HFP_1_6_ENABLE ?= 1

ifeq ($(HFP_1_6_ENABLE),1)
    MSBC_16K_SAMPLE_RATE ?= 1
else
    MSBC_16K_SAMPLE_RATE := 0
endif

SBC_FUNC_IN_ROM ?= 0

ROM_UTILS_ON ?= 0

TEST_OVER_THE_AIR ?= 1

APP_ANC_TEST ?= 1

APP_LINEIN_A2DP_SOURCE ?= 0

APP_I2S_A2DP_SOURCE ?= 0

VOICE_PROMPT ?= 1

AI_CAPTURE_CHANNEL_NUM ?= 0

AI_CAPTURE_DATA_AEC ?= 0

export AI_AEC_CP_ACCEL ?= 0

export THROUGH_PUT ?= 0
#### Google related feature ####
# the overall google service switch
# currently, google service includes BISTO and GFPS
export GOOGLE_SERVICE_ENABLE ?= 0

# BISTO is a GVA service on Bluetooth audio device
# BISTO is an isolated service relative to GFPS
export BISTO_ENABLE ?= 0

# macro switch for reduced_guesture
export REDUCED_GUESTURE_ENABLE ?= 0

# GSOUND_HOTWORD is a hotword library running on Bluetooth audio device
# GSOUND_HOTWORD is a subset of BISTO
export GSOUND_HOTWORD_ENABLE ?= 0

# this is a subset choice for gsound hotword
export GSOUND_HOTWORD_EXTERNAL ?= 0

# GFPS is google fastpair service
# GFPS is an isolated service relative to BISTO
export GFPS_ENABLE ?= 0
#### Google related feature ####

#### alexa ai voice ####
AMA_VOICE ?= 0

#### xiaodu ai voice ####
DMA_VOICE ?= 0

#### ali ai voice ####
GMA_VOICE ?= 0

#### BES ai voice ####
SMART_VOICE ?= 0

#### xiaowei ai voice ####
TENCENT_VOICE ?= 0

#### customize ai voice ####
CUSTOMIZE_VOICE ?= 0

#### use the hot word lib of amazon ####
export ALEXA_WWE ?= 0
#### a subset choice for the hot word lib of amazon -- lite mode####
export ALEXA_WWE_LITE ?= 0
ifeq ($(ALEXA_WWE),1)
KBUILD_CPPFLAGS += -D__ALEXA_WWE
export USE_THIRDPARTY := 1
TRACE_BUF_SIZE := 8*1024
FAST_XRAM_SECTION_SIZE := 0x11000

ifeq ($(ALEXA_WWE_LITE),1)
KBUILD_CPPFLAGS += -D__ALEXA_WWE_LITE
export THIRDPARTY_LIB := kws/alexa_lite
else
export THIRDPARTY_LIB := kws/alexa
export MCU_HIGH_PERFORMANCE_MODE := 1
endif
endif

export KWS_ALEXA := 0
ifeq ($(KWS_ALEXA),1)
export MCU_HIGH_PERFORMANCE_MODE :=1
export USE_THIRDPARTY := 1
export THIRDPARTY_LIB := kws/bes

FAST_XRAM_SECTION_SIZE := 0x17400
KBUILD_CPPFLAGS += -D__KWS_ALEXA__
KBUILD_CPPFLAGS += \
                           -Ithirdparty/kws/bes_lib/include/
endif

ifeq ($(MCU_HIGH_PERFORMANCE_MODE),1)
OSC_26M_X4_AUD2BB := 0
endif

BLE ?= 1

TILE_DATAPATH_ENABLED ?= 0

CUSTOM_INFORMATION_TILE_ENABLE ?= 0

TOTA ?= 0

## for SPP OTA ##
BES_OTA_BASIC ?= 1

INTERCONNECTION ?= 0

INTERACTION ?= 0

INTERACTION_FASTPAIR ?= 0

BT_ONE_BRING_TWO ?= 1

DSD_SUPPORT ?= 0

A2DP_EQ_24BIT ?= 1

A2DP_AAC_ON ?= 1

A2DP_SCALABLE_ON ?= 0

A2DP_LHDC_ON ?= 0

ifeq ($(A2DP_LHDC_ON),1)
A2DP_LHDC_V3 ?= 0
endif
A2DP_LDAC_ON ?= 0

export TX_RX_PCM_MASK ?= 0

A2DP_SCALABLE_ON ?= 0

FACTORY_MODE ?= 1

ENGINEER_MODE ?= 1

ULTRA_LOW_POWER	?= 1

DAC_CLASSG_ENABLE ?= 1

NO_SLEEP ?= 0

CORE_DUMP ?= 1

CORE_DUMP_TO_FLASH ?= 0

ENHANCED_STACK ?= 1

export A2DP_DECODER_VER := 2

export POWER_MODE   ?= DIG_DCDC

export SPEECH_CODEC ?= 1

export MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED ?= 1

export FLASH_SIZE ?= 0x400000
export FLASH_SUSPEND ?= 1

BTUSB_DUAL_MODE_SUPPORT ?= 1

ifeq ($(BTUSB_DUAL_MODE_SUPPORT),1)
export USB_AUDIO_SEND_MONO ?= 1
export AUDIO_INPUT_MONO ?= 1
export USB_AUDIO_SEND_CHAN ?= 1
export BT_USB_AUDIO_DUAL_MODE = 1
#export BTUSB_AUDIO_MODE ?= 1
export DELAY_STREAM_OPEN ?= 1
export USB_ISO ?= 1
ifeq ($(AUDIO_DRC),1)
export USB_AUDIO_DYN_CFG ?= 0
else ifeq ($(AUDIO_DRC2),1)
export USB_AUDIO_DYN_CFG ?= 0
else
export USB_AUDIO_DYN_CFG ?= 1
endif
#export USB_AUDIO_UAC2 ?= 1
#export USB_HIGH_SPEED ?= 1
#export USB_AUDIO_384K ?= 1
#export USB_AUDIO_32BIT	?= 1
#export USB_AUDIO_SPEECH ?= 1
endif

ifeq ($(DSD_SUPPORT),1)
export BTUSB_AUDIO_MODE     ?= 1
export AUDIO_INPUT_MONO     ?= 1
export USB_ISO              ?= 1
export USB_AUDIO_DYN_CFG    ?= 1
export DELAY_STREAM_OPEN    ?= 0
export KEEP_SAME_LATENCY    ?= 1
export HW_FIR_DSD_PROCESS   ?= 1
ifeq ($(HW_FIR_DSD_PROCESS),1)
ifeq ($(CHIP),best2300)
export HW_FIR_DSD_BUF_MID_ADDR  ?= 0x200A0000
export DATA_BUF_START           ?= 0x20040000
endif
endif
export USB_AUDIO_UAC2 ?= 1
export USB_HIGH_SPEED ?= 1
KBUILD_CPPFLAGS += \
    -DHW_FIR_DSD_BUF_MID_ADDR=$(HW_FIR_DSD_BUF_MID_ADDR) \
    -DDATA_BUF_START=$(DATA_BUF_START)
endif

USE_THIRDPARTY ?= 0

export USE_KNOWLES ?= 0
ifeq ($(CURRENT_TEST),1)
export VCODEC_VOLT ?= 1.95V
export VANA_VOLT ?= 1.35V
endif

export LAURENT_ALGORITHM ?= 0

export TX_IQ_CAL ?= 0

export BT_XTAL_SYNC ?= 1

export BTADDR_FOR_DEBUG ?= 1

export POWERKEY_I2C_SWITCH ?=0

AUTO_TEST ?= 0

export DUMP_NORMAL_LOG ?= 0

SUPPORT_BATTERY_REPORT ?= 1

SUPPORT_HF_INDICATORS ?= 0

SUPPORT_SIRI ?= 1

ifeq ($(A2DP_LHDC_ON),1)
ifeq ($(AUDIO_ANC_FB_MC),1)
AUDIO_BUFFER_SIZE ?= 140*1024+130*1024
else
AUDIO_BUFFER_SIZE ?= 140*1024
endif
else
ifeq ($(AUDIO_ANC_FB_MC),1)
AUDIO_BUFFER_SIZE ?=120*1024+130*1024
else
AUDIO_BUFFER_SIZE ?= 120*1024
endif
endif

export TRACE_BUF_SIZE ?= 10*1024
export TRACE_BAUD_RATE := 921600

init-y      :=
core-y      := platform/ services/ apps/ utils/cqueue/ utils/list/ services/multimedia/ utils/intersyshci/

KBUILD_CPPFLAGS +=  -Iplatform/cmsis/inc \
                    -Iservices/audioflinger \
                    -Iplatform/hal \
                    -Iservices/fs/ \
                    -Iservices/fs/sd \
                    -Iservices/fs/fat \
                    -Iservices/fs/fat/ChaN

KBUILD_CPPFLAGS += \
    -DAPP_AUDIO_BUFFER_SIZE=$(AUDIO_BUFFER_SIZE) \
    -D__APP_KEY_FN_STYLE_A__ \
    -D__USER_DEFINE_CTR__ \
    -D__LDO_3V3_CTR__ \
    -D__NTC_DETECT__ \
    -D__USE_3_5JACK_CTR__ \
    -D__EVRCORD_USER_DEFINE__ \
    -DCUSTOM_BIN_CONFIG \
    -D__DEFINE_DEMO_MODE__ \
    -D__CHARGE_CURRRENT__ \
    -D__AUDIO_FADEIN__
#   -D__EARPHONE_STAY_BOTH_SCAN__
# 	-D__PWM_LED_CTL__
#    -D__HAYLOU_APP__
#    -D__USE_AMP_MUTE_CTR__

#    -DANC_LED_PIN   
#   -D__CST816S_TOUCH__

#    -D__PC_CMD_UART__

#KBUILD_CPPFLAGS += -D__3M_PACK__

#-D_AUTO_SWITCH_POWER_MODE__
#-D__APP_KEY_FN_STYLE_A__
#-D__APP_KEY_FN_STYLE_B__
#-D__EARPHONE_STAY_BOTH_SCAN__
#-D__POWERKEY_CTRL_ONOFF_ONLY__
#-DAUDIO_LINEIN

export RSA_SHA ?= 0
ifeq ($(RSA_SHA),1)
KBUILD_CPPFLAGS += -D__RSA_SHA__
endif
export AES256_FOR_BIN ?= 0
ifeq ($(AES256_FOR_BIN),1)
KBUILD_CPPFLAGS += -D__AES256_FOR_BIN__
endif

ifeq ($(CURRENT_TEST),1)
INTSRAM_RUN ?= 1
endif
ifeq ($(INTSRAM_RUN),1)
LDS_FILE    := best1000_intsram.lds
else
LDS_FILE    := best1000.lds
endif

ifneq ($(FAST_XRAM_SECTION_SIZE),)
CPPFLAGS_${LDS_FILE} +=-DFAST_XRAM_SECTION_SIZE=$(FAST_XRAM_SECTION_SIZE)
endif

export OTA_SUPPORT_SLAVE_BIN ?= 0
ifeq ($(OTA_SUPPORT_SLAVE_BIN),1)
export SLAVE_BIN_FLASH_OFFSET ?= 0x100000
export SLAVE_BIN_TARGET_NAME ?= anc_usb
endif

ifeq ($(BES_OTA_BASIC),1)
export OTA_BASIC ?= 1
export BES_OTA_BASIC ?= 1
KBUILD_CPPFLAGS += -DBES_OTA_BASIC=1
endif

ifeq ($(INTERACTION),1)
export OTA_BASIC := 1
endif

ifeq ($(INTERCONNECTION),1)
export BLE := 1
export OTA_BASIC := 1
endif

ifeq ($(TOTA),1)
ifeq ($(BLE),1)
KBUILD_CPPFLAGS += -DBLE_TOTA_ENABLED
endif
KBUILD_CPPFLAGS += -DTEST_OVER_THE_AIR_ENANBLED
export TEST_OVER_THE_AIR ?= 1
endif

ifeq ($(A2DP_CP_ACCEL),1)
export A2DP_DECODER_VER := 2
endif

ifneq ($(A2DP_DECODER_VER), )
KBUILD_CPPFLAGS += -DA2DP_DECODER_VER=$(A2DP_DECODER_VER)
endif

KBUILD_CPPFLAGS +=

KBUILD_CFLAGS +=

LIB_LDFLAGS += -lstdc++ -lsupc++

#CFLAGS_IMAGE += -u _printf_float -u _scanf_float

#LDFLAGS_IMAGE += --wrap main
