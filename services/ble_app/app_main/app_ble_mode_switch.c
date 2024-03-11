/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

/*****************************header include********************************/
#include "string.h"
#include "co_math.h" // Common Maths Definition
#include "cmsis_os.h"
#include "ble_app_dbg.h"
#include "stdbool.h"
#include "app_thread.h"
#include "app_utils.h"
#include "apps.h"
#include "app.h"
#include "app_sec.h"
#include "app_ble_include.h"
#include "nvrecord.h"
#include "app_bt_func.h"
#include "hal_timer.h"
#include "app_bt.h"
#include "app_hfp.h"
#include "rwprf_config.h"
#include "nvrecord_ble.h"
#include "app_sec.h"

#include "app_battery.h"//add by pang


/************************private macro defination***************************/
#define DEBUG_BLE_STATE_MACHINE true

#if DEBUG_BLE_STATE_MACHINE
#define SET_BLE_STATE(newState)                                                                                \
    do                                                                                                         \
    {                                                                                                          \
        LOG_I("[STATE]%s->%s at line %d", ble_state2str(bleModeEnv.state), ble_state2str(newState), __LINE__); \
        bleModeEnv.state = (newState);                                                                         \
    } while (0);

#define SET_BLE_OP(newOp)                                                                       \
    do                                                                                          \
    {                                                                                           \
        LOG_I("[OP]%s->%s at line %d", ble_op2str(bleModeEnv.op), ble_op2str(newOp), __LINE__); \
        bleModeEnv.op = (newOp);                                                                \
    } while (0);
#else
#define SET_BLE_STATE(newState)        \
    do                                 \
    {                                  \
        bleModeEnv.state = (newState); \
    } while (0);

#define SET_BLE_OP(newOp)        \
    do                           \
    {                            \
        bleModeEnv.op = (newOp); \
    } while (0);
#endif

#ifdef CHIP_BEST2000
extern void bt_drv_reg_set_rssi_seed(uint32_t seed);
#endif

/************************private type defination****************************/

/************************extern function declearation***********************/
extern uint8_t bt_addr[6];

/**********************private function declearation************************/
/*---------------------------------------------------------------------------
 *            ble_state2str
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the string of the ble state
 *
 * Parameters:
 *    state - state to get string
 *
 * Return:
 *    the string of the BLE state
 */
static char *ble_state2str(uint8_t state);

/*---------------------------------------------------------------------------
 *            ble_op2str
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the string of the ble operation
 *
 * Parameters:
 *    op - operation to get string
 *
 * Return:
 *    the string of the BLE operation
 */
static char *ble_op2str(uint8_t op);

/*---------------------------------------------------------------------------
 *            ble_adv_user2str
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the string of the ble_adv_user
 *
 * Parameters:
 *    op - operation to get string
 *
 * Return:
 *    the string of the ble_adv_user
 */
static char *ble_adv_user2str(enum BLE_ADV_USER_E user);

/*---------------------------------------------------------------------------
 *            ble_adv_config_param
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    configure BLE adv related parameter in @advParam for further use
 *
 * Parameters:
 *    advType - advertisment mode, see @ for more info
 *    advInterval - advertisement interval in MS
 *
 * Return:
 *    void
 */
static void ble_adv_config_param(uint8_t advType, uint16_t advInterval);

/*---------------------------------------------------------------------------
 *            ble_adv_is_allowed
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    check if BLE advertisment is allowed or not
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - if advertisement is allowed
 *    flase -  if adverstisement is not allowed
 */
static bool ble_adv_is_allowed(void);

/*---------------------------------------------------------------------------
 *            ble_start_adv
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    start BLE advertisement
 *
 * Parameters:
 *    param - see @BLE_ADV_PARAM_T to get more info
 *
 * Return:
 *    void
 */
static void ble_start_adv(void *param);

/*---------------------------------------------------------------------------
 *            ble_start_adv_failed_cb
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    callback function of start BLE advertisement failed
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
static void ble_start_adv_failed_cb(void);

/*---------------------------------------------------------------------------
 *            ble_start_scan
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    start BLE scan api
 *
 * Parameters:
 *    param - see @BLE_SCAN_PARAM_T to get more info
 *
 * Return:
 *    void
 */
static void ble_start_scan(void *param);

/*---------------------------------------------------------------------------
 *            ble_start_connect
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    start BLE connection
 *
 * Parameters:
 *    bleBdAddr - address of BLE device to connect
 *
 * Return:
 *    void
 */
static void ble_start_connect(uint8_t *bleBdAddr);

/*---------------------------------------------------------------------------
 *            ble_stop_all_activities
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    stop all BLE pending operations, stop ongoing adv and scan
 *    NOTE: will not disconnect BLE connections
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
static void ble_stop_all_activities(void);

/*---------------------------------------------------------------------------
 *            ble_execute_pending_op
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    execute pended BLE op
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
static void ble_execute_pending_op(void);

/*---------------------------------------------------------------------------
 *            ble_switch_activities
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    switch BLE activities after last state complete
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
static void ble_switch_activities(void);

/************************private variable defination************************/
static BLE_MODE_ENV_T bleModeEnv;
static BLE_ADV_PARAM_T advParam;
static BLE_SCAN_PARAM_T scanParam;

/****************************function defination****************************/
// common used function
#ifdef USE_LOG_I_ID
static inline char *ble_state2str(uint8_t state)
{
    return (char *)(uint32_t)state;
}
static inline char *ble_op2str(uint8_t op)
{
    return (char *)(uint32_t)op;
}
static inline char *ble_adv_user2str(enum BLE_ADV_USER_E user)
{
    return (char *)(uint32_t)user;
}

#else
static char *ble_state2str(uint8_t state)
{
    char *str = NULL;

#define CASES(state)          \
    case state:               \
        str = "[" #state "]"; \
        break

    switch (state)
    {
        CASES(STATE_IDLE);
        CASES(ADVERTISING);
        CASES(STARTING_ADV);
        CASES(STOPPING_ADV);
        CASES(SCANNING);
        CASES(STARTING_SCAN);
        CASES(STOPPING_SCAN);
        CASES(CONNECTING);
        CASES(STARTING_CONNECT);
        CASES(STOPPING_CONNECT);

    default:
        str = "[INVALID]";
        break;
    }

    return str;
}

static char *ble_op2str(uint8_t op)
{
    char *str = NULL;

#define CASEO(op)          \
    case op:               \
        str = "[" #op "]"; \
        break

    switch (op)
    {
        CASEO(OP_IDLE);
        CASEO(START_ADV);
        CASEO(START_SCAN);
        CASEO(START_CONNECT);
        CASEO(STOP_ADV);
        CASEO(STOP_SCAN);
        CASEO(STOP_CONNECT);

    default:
        str = "[INVALID]";
        break;
    }

    return str;
}

static char *ble_adv_user2str(enum BLE_ADV_USER_E user)
{
#define CASE_S(s) \
    case s:       \
        return "[" #s "]";
#define CASE_D() \
    default:     \
        return "[INVALID]";

    switch (user)
    {
        CASE_S(USER_STUB)
        CASE_S(USER_GFPS)
        CASE_S(USER_GSOUND)
        CASE_S(USER_AI)
        CASE_S(USER_INTERCONNECTION)
        CASE_S(USER_TILE)
        CASE_S(USER_OTA)
        CASE_D()
    }
}
#endif


void app_ble_mode_init(void)
{
    LOG_I("%s", __func__);

    memset(&bleModeEnv, 0, sizeof(bleModeEnv));
    SET_BLE_STATE(STATE_IDLE);
    SET_BLE_OP(OP_IDLE);

    bleModeEnv.bleEnv = &app_env;
}

// ble advertisement used functions
#if 0
static void ble_adv_config_param(uint8_t advType, uint16_t advInterval)
{
    int8_t avail_space;
    memset(&advParam, 0, sizeof(advParam));

    advParam.advType = advType;
    advParam.advInterval = advInterval;
    advParam.withFlag = true;

    // connectable adv is not allowed if max connection reaches
    if (app_is_arrive_at_max_ble_connections() && (GAPM_ADV_UNDIRECT == advType))
    {
        LOG_W("will change adv type to none-connectable because max ble connection reaches");
        advParam.advType = GAPM_ADV_NON_CONN;
    }

    for (uint8_t user = 0; user < BLE_ADV_USER_NUM; user++)
    {
        if (bleModeEnv.bleDataFillFunc[user])
        {
            bleModeEnv.bleDataFillFunc[user]((void *)&advParam);

            // check if the adv/scan_rsp data length is legal
            if(advParam.withFlag)
            {
                ASSERT(BLE_ADV_DATA_WITH_FLAG_LEN >= advParam.advDataLen, "[BLE][ADV]adv data exceed");
            }
            else
            {
                ASSERT(BLE_ADV_DATA_WITHOUT_FLAG_LEN >= advParam.advDataLen, "[BLE][ADV]adv data exceed");
            }
            ASSERT(SCAN_RSP_DATA_LEN >= advParam.scanRspDataLen, "[BLE][ADV]scan response data exceed");
        }
    }

    if(advParam.withFlag)
    {
        avail_space = BLE_ADV_DATA_WITH_FLAG_LEN - advParam.advDataLen - BLE_ADV_DATA_STRUCT_HEADER_LEN;
    }
    else
    {
        avail_space = BLE_ADV_DATA_WITHOUT_FLAG_LEN - advParam.advDataLen - BLE_ADV_DATA_STRUCT_HEADER_LEN;
    }

    // Check if data can be added to the adv Data
    if (avail_space > 2)
    {
        avail_space = co_min(avail_space, bleModeEnv.bleEnv->dev_name_len);
        advParam.advData[advParam.advDataLen++] = avail_space + 1;
        // Fill Device Name Flag
        advParam.advData[advParam.advDataLen++] =
            (avail_space == bleModeEnv.bleEnv->dev_name_len) ? '\x08' : '\x09';
        // Copy device name
        memcpy(&advParam.advData[advParam.advDataLen], bleModeEnv.bleEnv->dev_name, avail_space);
        // Update adv Data Length
        advParam.advDataLen += avail_space;
    }
}
#else //m by cai for TPV ble ADV
static const uint32_t crc32tab[] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
    0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
    0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
    0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
    0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
    0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
    0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
    0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
    0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
    0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
    0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
    0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
    0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
    0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
    0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
    0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
    0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
    0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
    0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
    0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
    0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
    0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
    0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
    0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
    0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
    0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
    0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
    0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
    0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
    0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
    0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
    0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
    0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
    0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
    0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
    0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
    0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
    0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
    0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
    0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
    0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
    0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
    0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
    0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
    0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
    0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
    0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
    0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
    0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
    0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
    0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
    0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

//crc32校验码
uint32_t CRC32(const unsigned char *buf, uint32_t size)
{
    uint32_t i, crc;
    crc = 0xFFFFFFFF;
    for (i = 0; i < size; i++)
        crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    return crc^0xFFFFFFFF;
}

static void ble_adv_config_param(uint8_t advType, uint16_t advInterval)
{
	uint8_t MacAddr[6] = {0};
	uint8_t device_name[100] = {0};
	uint8_t namelen=0;
	uint8_t crc32_device_name[4] = {0};
	uint8_t adv_data[31]={0};
	
    memcpy(MacAddr, bt_addr, 6);
	app_dev_name_get(device_name);
	namelen = strlen((const char *)device_name);

	//Manufacturer Specific Data adv type
	adv_data[1] = 0xFF;
	
	if(namelen > 0 && (btif_me_get_activeCons() || app_bt_is_connected())){
		adv_data[0] = 0x05;
		if(namelen>0)
		{
			uint8_t dev_na[namelen];
			memcpy(dev_na, device_name, namelen);
			*((uint32_t *)crc32_device_name) = CRC32((const unsigned char *)dev_na, namelen);
			LOG_I("***crc32_device_name: 0x%x",*((uint32_t *)crc32_device_name));
			adv_data[2] = crc32_device_name[3];
			adv_data[3] = crc32_device_name[2];
			adv_data[4] = crc32_device_name[1];
			adv_data[5] = crc32_device_name[0];
		}
		
		//Shortened Local Name adv type
		adv_data[6] = bleModeEnv.bleEnv->dev_name_len+1;
		adv_data[7] = 0x08;
		memcpy(&adv_data[8], bleModeEnv.bleEnv->dev_name, bleModeEnv.bleEnv->dev_name_len);
	} else{
		LOG_I("***pdevice_name is NULL");
		adv_data[0] = sizeof(MacAddr)+1;
		adv_data[2] = MacAddr[5];
		adv_data[3] = MacAddr[4];
		adv_data[4] = MacAddr[3];
		adv_data[5] = MacAddr[2];
		adv_data[6] = MacAddr[1];
		adv_data[7] = MacAddr[0];

		//Shortened Local Name adv type
		adv_data[9] = 0x08;
		if(bleModeEnv.bleEnv->dev_name_len > 30-9){
			adv_data[8] = 30-9+1;
			memcpy(&adv_data[10], bleModeEnv.bleEnv->dev_name, 30-9);
		} else{
			adv_data[8] = bleModeEnv.bleEnv->dev_name_len+1;
			memcpy(&adv_data[10], bleModeEnv.bleEnv->dev_name, bleModeEnv.bleEnv->dev_name_len);
		}
	}

    memset(&advParam, 0, sizeof(advParam));

    advParam.advType = advType;
    advParam.advInterval = advInterval;
    advParam.withFlag = false;

    // connectable adv is not allowed if max connection reaches
    if (app_is_arrive_at_max_ble_connections() && (GAPM_ADV_UNDIRECT == advType))
    {
        LOG_W("will change adv type to none-connectable because max ble connection reaches");
        advParam.advType = GAPM_ADV_NON_CONN;
    }

    for (uint8_t user = 0; user < BLE_ADV_USER_NUM; user++)
    {
        if (bleModeEnv.bleDataFillFunc[user])
        {
            bleModeEnv.bleDataFillFunc[user]((void *)&advParam);

            // check if the adv/scan_rsp data length is legal
            if(advParam.withFlag)
            {
                ASSERT(BLE_ADV_DATA_WITH_FLAG_LEN >= advParam.advDataLen, "[BLE][ADV]adv data exceed");
            }
            else
            {
                ASSERT(BLE_ADV_DATA_WITHOUT_FLAG_LEN >= advParam.advDataLen, "[BLE][ADV]adv data exceed");
            }
            ASSERT(SCAN_RSP_DATA_LEN >= advParam.scanRspDataLen, "[BLE][ADV]scan response data exceed");
        }
    }

    memcpy(&advParam.advData[advParam.advDataLen], adv_data, 31);
    // Update adv Data Length
    advParam.advDataLen = 31;
}
#endif

static bool ble_adv_is_allowed(void)
{
    bool allowed_adv = true;
    if (!app_is_stack_ready())
    {
        LOG_I("reason: stack not ready");
        allowed_adv = false;
    }

    if (app_is_power_off_in_progress())
    {
        LOG_I("reason: in power off mode");
        allowed_adv = false;
    }

    if (bleModeEnv.advSwitch)
    {
        LOG_I("adv switched off:%d", bleModeEnv.advSwitch);
        allowed_adv = false;
    }

    if (btapp_hfp_is_sco_active())
    {
        LOG_I("SCO ongoing");
        allowed_adv = false;
    }

    if (false == allowed_adv)
    {
        app_ble_stop_activities();
    }

    return allowed_adv;
}

static void ble_start_adv(void *param)
{
    switch (bleModeEnv.state)
    {
    case ADVERTISING:
        SET_BLE_STATE(STOPPING_ADV);
        SET_BLE_OP(START_ADV);
        appm_stop_advertising();
        break;

    case SCANNING:
        SET_BLE_STATE(STOPPING_SCAN);
        SET_BLE_OP(START_ADV);
        appm_stop_scanning();
        break;

    case CONNECTING:
        SET_BLE_STATE(STOPPING_CONNECT);
        SET_BLE_OP(START_ADV);
        appm_stop_connecting();
        break;

    case STARTING_ADV:
    case STARTING_SCAN:
    case STARTING_CONNECT:
    case STOPPING_ADV:
    case STOPPING_SCAN:
    case STOPPING_CONNECT:
        SET_BLE_OP(START_ADV);
        break;

    case STATE_IDLE:
        if (!ble_adv_is_allowed())
        {
            LOG_I("[ADV] not allowed.");
            if (START_ADV == bleModeEnv.op)
            {
                SET_BLE_OP(OP_IDLE);
            }
            break;
        }

        memcpy(&bleModeEnv.advInfo, param, sizeof(bleModeEnv.advInfo));
        appm_start_advertising(&bleModeEnv.advInfo);

        SET_BLE_STATE(STARTING_ADV);
        break;

    default:
        break;
    }
}

static void ble_start_adv_failed_cb(void)
{
    if (STARTING_ADV == bleModeEnv.state)
    {
        SET_BLE_STATE(STATE_IDLE);
    }

    // start pending op(start adv again)
}

static void ble_start_scan(void *param)
{
    switch (bleModeEnv.state)
    {
    case ADVERTISING:
        SET_BLE_STATE(STOPPING_ADV);
        SET_BLE_OP(START_SCAN);
        appm_stop_advertising();
        break;

    case SCANNING:
        SET_BLE_STATE(STOPPING_SCAN);
        SET_BLE_OP(START_SCAN);
        appm_stop_scanning();
        break;

    case CONNECTING:
        SET_BLE_STATE(STOPPING_CONNECT);
        SET_BLE_OP(START_SCAN);
        appm_stop_connecting();
        break;

    case STARTING_ADV:
    case STARTING_SCAN:
    case STARTING_CONNECT:
    case STOPPING_ADV:
    case STOPPING_SCAN:
    case STOPPING_CONNECT:
        SET_BLE_OP(START_SCAN);
        break;

    case STATE_IDLE:
    
        SET_BLE_STATE(STARTING_SCAN);
        memcpy(&bleModeEnv.scanInfo, param, sizeof(BLE_SCAN_PARAM_T));
        appm_start_scanning(bleModeEnv.scanInfo.scanInterval,
                            bleModeEnv.scanInfo.scanWindow,
                            bleModeEnv.scanInfo.scanType);
        break;

    default:
        break;
    }
}

static void ble_start_connect(uint8_t *bleBdAddr)
{
    SET_BLE_OP(START_CONNECT);

    if ((CONNECTING != bleModeEnv.state) &&
        (STARTING_CONNECT != bleModeEnv.state))
    {
        switch (bleModeEnv.state)
        {
        case ADVERTISING:
            SET_BLE_STATE(STOPPING_ADV);
            appm_stop_advertising();
            break;

        case SCANNING:
            SET_BLE_STATE(STOPPING_SCAN);
            appm_stop_scanning();
            break;

        case STATE_IDLE:
            SET_BLE_STATE(STARTING_CONNECT);
            struct gap_bdaddr bdAddr;
            memcpy(bdAddr.addr.addr, bleBdAddr, BTIF_BD_ADDR_SIZE);
            bdAddr.addr_type = 0;
            LOG_I("Master paired with mobile dev is scanned, connect it via BLE.");
            appm_start_connecting(&bdAddr);
            break;

        default:
            break;
        }
    }
}

static void ble_stop_all_activities(void)
{
    switch (bleModeEnv.state)
    {
    case ADVERTISING:
        SET_BLE_OP(OP_IDLE);
        SET_BLE_STATE(STOPPING_ADV);
        appm_stop_advertising();
        break;

    case SCANNING:
        SET_BLE_OP(OP_IDLE);
        SET_BLE_STATE(STOPPING_SCAN);
        appm_stop_scanning();
        break;

    case CONNECTING:
        SET_BLE_OP(OP_IDLE);
        SET_BLE_STATE(STOPPING_CONNECT);
        appm_stop_connecting();
        break;

    case STARTING_ADV:
        SET_BLE_OP(STOP_ADV);
        break;

    case STARTING_SCAN:
        SET_BLE_OP(STOP_SCAN);
        break;

    case STARTING_CONNECT:
        SET_BLE_OP(STOP_CONNECT);
        break;

    case STOPPING_ADV:
    case STOPPING_SCAN:
    case STOPPING_CONNECT:
        SET_BLE_OP(OP_IDLE);
        break;

    default:
        break;
    }
}

static void ble_execute_pending_op(void)
{
    LOG_I("%s", __func__);
    uint8_t op = bleModeEnv.op;
    SET_BLE_OP(OP_IDLE);

    switch (op)
    {
    case START_ADV:
        ble_start_adv(&advParam);
        break;

    case START_SCAN:
        ble_start_scan(&scanParam);
        break;

    case START_CONNECT:
        ble_start_connect(bleModeEnv.bleAddrToConnect);
        break;

    case STOP_ADV:
    case STOP_SCAN:
    case STOP_CONNECT:
        ble_stop_all_activities();
        break;

    default:
        break;
    }
}

static void ble_switch_activities(void)
{
    switch (bleModeEnv.state)
    {
    case STARTING_ADV:
        SET_BLE_STATE(ADVERTISING);
        break;

    case STARTING_SCAN:
        SET_BLE_STATE(SCANNING);
        break;

    case STARTING_CONNECT:
        SET_BLE_STATE(CONNECTING);
        break;

    case STOPPING_ADV:
        SET_BLE_STATE(STATE_IDLE);
        break;

    case STOPPING_SCAN:
        SET_BLE_STATE(STATE_IDLE);
        break;

    case STOPPING_CONNECT:
        SET_BLE_STATE(STATE_IDLE);
        break;

    default:
        break;
    }

    ble_execute_pending_op();
}

// BLE advertisement event callbacks
void app_advertising_started(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)ble_switch_activities);
}

void app_advertising_starting_failed(void)
{
    memset(&bleModeEnv.advInfo, 0, sizeof(bleModeEnv.advInfo));
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)ble_start_adv_failed_cb);
}

void app_advertising_stopped(void)
{
    memset(&bleModeEnv.advInfo, 0, sizeof(bleModeEnv.advInfo));
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              ( uint32_t )ble_switch_activities);
}

// BLE adv data updated event callback
void app_adv_data_updated(void) 
{
    app_bt_start_custom_function_in_bt_thread(0,
            0,
            (uint32_t)ble_switch_activities);
}

// BLE scan event callbacks
void app_scanning_started(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)ble_switch_activities);
}

void app_scanning_stopped(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)ble_switch_activities);
}

// BLE connect event callbacks
void app_connecting_started(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)ble_switch_activities);
}

void app_connecting_stopped(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)ble_switch_activities);
}

/**
 * @brief : callback function of BLE connect failed
 *
 */
static void app_ble_connecting_failed_handler(void)
{
    if ((CONNECTING == bleModeEnv.state) || 
        (STOPPING_CONNECT == bleModeEnv.state) ||
        (STARTING_CONNECT == bleModeEnv.state) )
    {
        SET_BLE_STATE(STATE_IDLE);
    }
}

void app_connecting_failed(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)app_ble_connecting_failed_handler);
}
void app_ble_connected_evt_handler(uint8_t conidx, const uint8_t *pPeerBdAddress)
{
    ble_evnet_t event;

    if ((ADVERTISING == bleModeEnv.state) ||
        (STARTING_ADV == bleModeEnv.state) ||
        (CONNECTING == bleModeEnv.state))
    {
        SET_BLE_STATE(STATE_IDLE);
    }

    if (START_CONNECT == bleModeEnv.op)
    {
        SET_BLE_OP(OP_IDLE);
    }

    event.evt_type = BLE_CONNECT_EVENT;
    event.p.connect_handled.conidx = conidx;
    event.p.connect_handled.peer_bdaddr = pPeerBdAddress;
    app_ble_core_global_handle(&event, NULL);


    app_ble_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
    app_stop_fast_connectable_ble_adv_timer();
}

void app_ble_disconnected_evt_handler(uint8_t conidx)
{
    ble_evnet_t event;

    event.evt_type = BLE_DISCONNECT_EVENT;
    event.p.disconnect_handled.conidx = conidx;
    app_ble_core_global_handle(&event, NULL);

    app_ble_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
}

// BLE APIs for external use
void app_ble_data_fill_enable(enum BLE_ADV_USER_E user, bool enable)
{
    LOG_I("%s user %d%s enable %d", __func__, user, ble_adv_user2str(user), enable);

    ASSERT(user < BLE_ADV_USER_NUM, "%s user %d", __func__, user);
    if (enable)
    {
        bleModeEnv.adv_user_enable |= (1 << user);
    }
    else
    {
        bleModeEnv.adv_user_enable &= ~(1 << user);
    }
}

bool app_ble_get_data_fill_enable(enum BLE_ADV_USER_E user)
{
    bool enable = bleModeEnv.adv_user_enable & (1 << user);
    LOG_I("%s user %d enable %d", __func__, user, enable);
    return enable;
}


/**
 * @brief : callback function of BLE scan starting failed
 *
 */
static void app_ble_scanning_starting_failed_handler(void)
{
    if (STARTING_SCAN == bleModeEnv.state)
    {
        SET_BLE_STATE(STATE_IDLE);
    }
} 

void app_scanning_starting_failed(void)
{
    app_bt_start_custom_function_in_bt_thread(0,
                                              0,
                                              (uint32_t)app_ble_scanning_starting_failed_handler);
}

// BLE APIs for external use
void app_ble_register_data_fill_handle(enum BLE_ADV_USER_E user, BLE_DATA_FILL_FUNC_T func, bool enable)
{
    bool needUpdateAdv = false;

    if (BLE_ADV_USER_NUM <= user)
    {
        LOG_W("invalid user");
    }
    else
    {
        if (func != bleModeEnv.bleDataFillFunc[user] &&
            NULL != func)
        {
            needUpdateAdv = true;
            bleModeEnv.bleDataFillFunc[user] = func;
        }
    }

    bleModeEnv.adv_user_register |= (1 << user);
    if (needUpdateAdv)
    {
        app_ble_data_fill_enable(user, enable);
    }
}

void app_ble_system_ready(void)
{
#ifdef CHIP_BEST2000
    uint32_t generatedSeed = 0;
    uint32_t currentTicks;
    for (uint8_t index = 0; index < sizeof(bt_addr); index++)
    {
        currentTicks = hal_sys_timer_get();
        
        osDelay((currentTicks%3) + 1);
        
        generatedSeed += (((uint32_t)(bt_addr[index])) << (currentTicks & 0xF));
    }

    bt_drv_reg_set_rssi_seed(generatedSeed);
#endif

#if defined(ENHANCED_STACK)
    app_notify_stack_ready(STACK_READY_BLE);
#else
    app_notify_stack_ready(STACK_READY_BLE | STACK_READY_BT);
#endif
}

static void ble_adv_refreshing(void *param)
{
    BLE_ADV_PARAM_T *pAdvParam = (BLE_ADV_PARAM_T *)param;
    // four conditions that we just need to update the ble adv data instead of restarting ble adv
    // 1. BLE advertising is on
    // 2. No on-going BLE operation
    // 3. BLE adv type is the same
    // 4. BLE adv interval is the same
    if ((ADVERTISING == bleModeEnv.state) && \
        (OP_IDLE == bleModeEnv.op) && \
        bleModeEnv.advInfo.advType == pAdvParam->advType && \
        bleModeEnv.advInfo.advInterval == pAdvParam->advInterval)
    {
        memcpy(&bleModeEnv.advInfo, param, sizeof(bleModeEnv.advInfo));
        SET_BLE_STATE(STARTING_ADV);
        appm_update_adv_data(pAdvParam->advData,
                             pAdvParam->advDataLen,
                             pAdvParam->scanRspData,
                             pAdvParam->scanRspDataLen);
    }
    else
    {
        // otherwise, restart ble adv
        ble_start_adv(param);
    }
}

static bool app_ble_start_adv(uint8_t advType, uint16_t advInterval)
{
    uint32_t adv_user_enable = bleModeEnv.adv_user_enable;
    LOG_I("[ADV]type:%d, interval:%d ca:%p", advType, advInterval, __builtin_return_address(0));

    if (!ble_adv_is_allowed())
    {
        LOG_I("[ADV] not allowed.");
        return false;
    }

    ble_adv_config_param(advType, advInterval);

    LOG_I("%s old_user_enable 0x%x new 0x%x", __func__, adv_user_enable, bleModeEnv.adv_user_enable);
    if (!bleModeEnv.adv_user_enable)
    {
        LOG_I("no adv user enable");
        LOG_I("[ADV] not allowed.");
        app_ble_stop_activities();
        return false;
    }

    // param of adv request is exactly same as current adv
    if (ADVERTISING == bleModeEnv.state &&
        !memcmp(&bleModeEnv.advInfo, &advParam, sizeof(advParam)))
    {
        LOG_I("reason: adv param not changed");
        LOG_I("[ADV] not allowed.");
        return false;
    }

    LOG_I("[ADV_LEN] %d [DATA]:", advParam.advDataLen);
    DUMP8("%02x ", advParam.advData, advParam.advDataLen);
    LOG_I("[SCAN_RSP_LEN] %d [DATA]:", advParam.scanRspDataLen);
    DUMP8("%02x ", advParam.scanRspData, advParam.scanRspDataLen);

    ble_adv_refreshing(&advParam);

    return true;
}

void app_ble_start_connectable_adv(uint16_t advInterval)
{
    LOG_D("%s", __func__);
    app_bt_start_custom_function_in_bt_thread(
        (uint32_t)GAPM_ADV_UNDIRECT, (uint32_t)advInterval,
        (uint32_t)app_ble_start_adv);
}

void app_ble_refresh_adv_state(uint16_t advInterval)
{
    LOG_D("%s", __func__);
    app_bt_start_custom_function_in_bt_thread(
        (uint32_t)GAPM_ADV_UNDIRECT, (uint32_t)advInterval,
        (uint32_t)app_ble_start_adv);
}

void app_ble_start_scan(enum BLE_SCAN_FILTER_POLICY scanFilterPolicy, uint16_t scanWindow, uint16_t scanInterval)
{
    scanParam.scanWindow = scanWindow;
    scanParam.scanInterval = scanInterval;
    scanParam.scanType = scanFilterPolicy; //BLE_SCAN_ALLOW_ADV_WLST

    app_bt_start_custom_function_in_bt_thread((uint32_t)(&scanParam),
                                              0,
                                              (uint32_t)ble_start_scan);
}

void app_ble_start_connect(uint8_t *bdAddrToConnect)
{
    memcpy(bleModeEnv.bleAddrToConnect, bdAddrToConnect, BTIF_BD_ADDR_SIZE);
    app_bt_start_custom_function_in_bt_thread((uint32_t)bleModeEnv.bleAddrToConnect,
                                              0,
                                              (uint32_t)ble_start_connect);
}

bool app_ble_is_any_connection_exist(void)
{
    bool ret = false;
    for (uint8_t i = 0; i < BLE_CONNECTION_MAX; i++)
    {
	    if(bleModeEnv.bleEnv == NULL)
		{
			break;
    	}
        if (BLE_CONNECTED == bleModeEnv.bleEnv->context[i].connectStatus)
        {
            ret = true;
        }
    }

    return ret;
}

void app_ble_start_disconnect(uint8_t conIdx)
{
    if (BLE_CONNECTED == bleModeEnv.bleEnv->context[conIdx].connectStatus)
    {
        LOG_I("will disconnect connection:%d", conIdx);
        bleModeEnv.bleEnv->context[conIdx].connectStatus = BLE_DISCONNECTING;
        app_bt_start_custom_function_in_bt_thread((uint32_t)conIdx,
                                                  0,
                                                  (uint32_t)appm_disconnect);
    }
    else
    {
        LOG_I("will not execute disconnect since state is:%d",
              bleModeEnv.bleEnv->context[conIdx].connectStatus);
    }
}

void app_ble_disconnect_all(void)
{
    for (uint8_t i = 0; i < BLE_CONNECTION_MAX; i++)
    {
        app_ble_start_disconnect(i);
    }
}

void app_ble_stop_activities(void)
{
    LOG_I("%s %p", __func__, __builtin_return_address(0));

    app_stop_fast_connectable_ble_adv_timer();
    if (bleModeEnv.state != OP_IDLE)
    {
        app_bt_start_custom_function_in_bt_thread(0,
                                                  0,
                                                  (uint32_t)ble_stop_all_activities);
    }
}

void app_ble_force_switch_adv(uint8_t user, bool onOff)
{
    ASSERT(user < BLE_SWITCH_USER_NUM, "ble switch user exceed");

    if (onOff)
    {
        bleModeEnv.advSwitch &= ~(1 << user);
        app_ble_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
    }
    else if ((bleModeEnv.advSwitch & (1 << user)) == 0)
    {
        bleModeEnv.advSwitch |= (1 << user);
        app_ble_stop_activities();

        // disconnect all of the BLE connections if box is closed
        if (BLE_SWITCH_USER_BOX == user)
        {
            app_ble_disconnect_all();
        }
    }

    LOG_I("%s user %d onoff %d switch 0x%x", __func__, user, onOff, bleModeEnv.advSwitch);
}

bool app_ble_is_in_advertising_state(void)
{
    return (ADVERTISING == bleModeEnv.state) ||
           (STARTING_ADV == bleModeEnv.state) ||
           (STOPPING_ADV == bleModeEnv.state);
}

static uint32_t POSSIBLY_UNUSED ble_get_manufacture_data_ptr(uint8_t *advData,
                                                             uint32_t dataLength,
                                                             uint8_t *manufactureData)
{
    uint8_t followingDataLengthOfSection;
    uint8_t rawContentDataLengthOfSection;
    uint8_t flag;
    while (dataLength > 0)
    {
        followingDataLengthOfSection = *advData++;
        dataLength--;
        if (dataLength < followingDataLengthOfSection)
        {
            return 0; // wrong adv data format
        }

        if (followingDataLengthOfSection > 0)
        {
            flag = *advData++;
            dataLength--;

            rawContentDataLengthOfSection = followingDataLengthOfSection - 1;
            if (BLE_ADV_MANU_FLAG == flag)
            {
                uint32_t lengthToCopy;
                if (dataLength < rawContentDataLengthOfSection)
                {
                    lengthToCopy = dataLength;
                }
                else
                {
                    lengthToCopy = rawContentDataLengthOfSection;
                }

                memcpy(manufactureData, advData - 2, lengthToCopy + 2);
                return lengthToCopy + 2;
            }
            else
            {
                advData += rawContentDataLengthOfSection;
                dataLength -= rawContentDataLengthOfSection;
            }
        }
    }

    return 0;
}

//received adv data
void app_adv_reported_scanned(struct gapm_adv_report_ind *ptInd)
{
    /*
    LOG_I("Scanned RSSI %d BD addr:", (int8_t)ptInd->report.rssi);
    DUMP8("0x%02x ", ptInd->report.adv_addr.addr, BTIF_BD_ADDR_SIZE);
    LOG_I("Scanned adv data:");
    DUMP8("0x%02x ", ptInd->report.data, ptInd->report.data_len);
    */

    ble_adv_data_parse(ptInd->report.adv_addr.addr,
                       (int8_t)ptInd->report.rssi,
                       ptInd->report.data,
                       (unsigned char)ptInd->report.data_len);
}

void app_ibrt_ui_disconnect_ble(void)
{
    app_ble_disconnect_all();
}

uint32_t app_ble_get_user_register(void)
{
    return bleModeEnv.adv_user_register;
}

