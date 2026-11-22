/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

#ifdef BLE_THROUGHPUT
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bt_gap_le.h"
#include "bt_gatts.h"
#include "bt_uuid.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "ble_throughput.h"

/* Create the log control block as user wishes. Here we use 'BLE_TPS' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(BLE_TPS, PRINT_LEVEL_INFO);

/************************************************
*   Defines
*************************************************/
/* CCCD notification value */
#define BLE_TPS_CCCD_NOTIFICATION  0x0001

/* Attribute Vlaue Hanlde */
#define TPS_RX_CHAR_VALUE_HANDLE    (0x0063)    /* RX Characteristic. */
#define TPS_TX_CHAR_VALUE_HANDLE    (0x0065)    /* TX Characteristic. */

/* UUID (TPS Service) */
#define BT_SIG_UUID_TPS_PRIMARY_SERVICE                     \
    {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,       \
      0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E}}
/*
#define BT_SIG_UUID_TPS_PRIMARY_SERVICE                     \
    {0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93,        \
      0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}
*/

/* UUID (TPS Rx) */
#define BT_SIG_UUID128_TPS_RX                               \
    {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,       \
      0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E}}
const bt_uuid_t BT_SIG_UUID_RX_CHAR =
      {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,     \
      0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E}};


/* UUID (TPS Tx) */
#define BT_SIG_UUID128_TPS_TX                               \
    {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,       \
      0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E}}
/*
#define BT_SIG_UUID128_TPS_TX                               \
        {0x6E, 0x40, 0x00, 0x03, 0xB5, 0xA3, 0xF3, 0x93,	\
          0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}
*/
const bt_uuid_t BT_SIG_UUID_TX_CHAR =
      {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,     \
      0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E}};
/*
const bt_uuid_t BT_SIG_UUID_TX_CHAR =
      {{0x6E, 0x40, 0x00, 0x03, 0xB5, 0xA3, 0xF3, 0x93,     \
        0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}};
*/

/************************************************
*   Variables
*************************************************/
static uint16_t ble_throughput_notify_enabled = 0;
static bool g_ble_gatt_server_test_enable = 0;

/* Get the writeable data for downlink */
static uint8_t g_ble_tps_data_first = 1;
static uint32_t g_ble_tps_tx_data_begin = 0;
static uint32_t g_ble_tps_tx_data_sum = 0;
bool ble_tps_relay = false;
static uint16_t peer_mtu = 23;
/************************************************
*   Prototypes
*************************************************/
static uint32_t ble_tps_rx_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_tps_tx_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_tps_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/************************************************
*   Service
*************************************************/
/* TPS Service */
BT_GATTS_NEW_PRIMARY_SERVICE_128(ble_tps_primary_service, BT_SIG_UUID_TPS_PRIMARY_SERVICE);

/* TPS Rx */
BT_GATTS_NEW_CHARC_128(ble_tps_char4_rx,
                        BT_GATT_CHARC_PROP_WRITE_WITHOUT_RSP|BT_GATT_CHARC_PROP_WRITE,
                        TPS_RX_CHAR_VALUE_HANDLE, BT_SIG_UUID128_TPS_RX);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_tps_char_rx_value, BT_SIG_UUID_RX_CHAR,
                        BT_GATTS_REC_PERM_WRITABLE,
                        ble_tps_rx_char_callback);

/* TPS Tx */
BT_GATTS_NEW_CHARC_128(ble_tps_char4_tx,
                        BT_GATT_CHARC_PROP_READ|BT_GATT_CHARC_PROP_NOTIFY,
                        TPS_TX_CHAR_VALUE_HANDLE, BT_SIG_UUID128_TPS_TX);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_tps_char_tx_value, BT_SIG_UUID_TX_CHAR,
                        BT_GATTS_REC_PERM_READABLE,
                        ble_tps_tx_char_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_tps_client_config,
                        BT_GATTS_REC_PERM_READABLE|BT_GATTS_REC_PERM_WRITABLE,
                        ble_tps_client_config_callback);

static const bt_gatts_service_rec_t *ble_throughput_service_rec[] = {
    /* TPS Service */
    (const bt_gatts_service_rec_t*) &ble_tps_primary_service,

    /* TPS Tx */
    (const bt_gatts_service_rec_t*) &ble_tps_char4_rx,
    (const bt_gatts_service_rec_t*) &ble_tps_char_rx_value,

    /* TPS Rx */
    (const bt_gatts_service_rec_t*) &ble_tps_char4_tx,
    (const bt_gatts_service_rec_t*) &ble_tps_char_tx_value,
    (const bt_gatts_service_rec_t*) &ble_tps_client_config,
};

const bt_gatts_service_t ble_throughput_service = {
    .starting_handle = 0x0061,
    .ending_handle = 0x0066,
    .required_encryption_key_size = 0,
    .records = ble_throughput_service_rec,
};

/************************************************
*   Functions
*************************************************/
void ble_tps_dump_write_data(void *data, uint16_t size){
    uint16_t i;
    printf("Downlink data\r\n");
    for (i = 0; i < size; i++) {
        printf("%x,", *(uint8_t *)((uint8_t *)data + i));
        if (i%10 == 0) {
            printf("\r\n");
        }
    }
    printf("\r\n");
}

#if 0
#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_ble_throughput_write_callback =_default_ble_throughput_write_callback")
#pragma comment(linker, "/alternatename:_ble_throughput_read_callback =_default_ble_throughput_read_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__CC_ARM)
#pragma weak ble_throughput_write_callback = default_ble_throughput_write_callback
#pragma weak ble_throughput_read_callback = default_ble_throughput_read_callback
#else
#error "Unsupported Platform"
#endif

uint8_t default_ble_throughput_read_callback(ble_throughput_event_t event, bt_handle_t conn_handle)
{
    return 0;
}

void default_ble_throughput_write_callback(ble_throughput_event_t event, bt_handle_t conn_handle, void *data)
{
    return;
}
#endif

static void ble_tps_calculate_throughput(uint32_t start_time, uint32_t end_time, uint32_t sum)
{
    uint32_t time_s = (end_time - start_time) * portTICK_PERIOD_MS / 1000;
    uint32_t throughput = sum/time_s*8;
    uint32_t tp_1, tp_2;

    if (throughput >= 1024) {
        tp_1 = throughput/1000*1000/1024;
        tp_2 = throughput%1000*1000/1024;
    } else {
        tp_1 = 0;
        tp_2 = throughput*1000/1024;
    }
    /* printf not enable print float function */
    printf("TX throughput equal %ld.%ldkbps\n", tp_1, tp_2);

}
static uint32_t ble_tps_rx_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    uint8_t *pData = (uint8_t*)data;

    if (handle > 0) {
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            if (0 != size) {
                if (g_ble_tps_data_first) {
                    g_ble_tps_data_first = 0;
                    g_ble_tps_tx_data_sum = 0;
                    g_ble_tps_tx_data_begin = xTaskGetTickCount();
                }

                if (ble_tps_relay) {
                    /* relay to send indication */
                    if (size > 260) {
                        LOG_W(BLE_TPS, "Rx char receive large data (size:%d)", size);
                        size = 260;
                    }

                    if (BT_STATUS_SUCCESS != ble_throughput_notify(handle, pData, size)) {
                        LOG_E(BLE_TPS, "Rx char relay data fail!");
                    }
                }

                g_ble_tps_tx_data_sum += size;

                if (size == 3) {
                    uint8_t end_tag[3] = {'E','N','D'};
                    if (memcmp(pData, end_tag, size) == 0) {
                        uint32_t end_time = xTaskGetTickCount();
                        ble_tps_calculate_throughput(g_ble_tps_tx_data_begin, end_time, g_ble_tps_tx_data_sum);
                        g_ble_tps_data_first = 1;
                        LOG_W(BLE_TPS, "Rx char receive data done.");
                        return size;
                    }
                }
#if 0
                ble_tps_dump_write_data(data, size);
#endif
                LOG_I(BLE_TPS, "Rx char receive data:%x (size:%d)", pData[0], size);
            }
            return size;

        }else {
            LOG_I(BLE_TPS, "Rx char callback, opcode:%d, size:%d", rw, size);
        }
    }

    return 0;
}

/*need to do, how to get the current TX Char Value*/
static uint32_t ble_tps_tx_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle > 0) {
        if (rw == BT_GATTS_CALLBACK_READ) {
            if (0 != size) {
                uint8_t *read_rsp_value = (uint8_t*)data;
                //*read_rsp_value = ble_throughput_read_callback(BLE_THROUGHPUT_EVENT_RX_READ, handle);
                *read_rsp_value = 0;
                LOG_I(BLE_TPS, "Tx char read data:%x (size:%d)", *read_rsp_value, size);
            }
            return sizeof(uint8_t);

        } else {
            LOG_I(BLE_TPS, "Tx char callback, opcode:%d, size:%d", rw, size);
        }
    }
    return 0;
}

/** Client Characteristic Configuration Descriptor. */
static uint32_t ble_tps_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle > 0) {
        /** record for each connection. */
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            if (size != sizeof(uint16_t)) { //Size check
                return 0;
            }
            //ble_throughput_write_callback(BLE_THROUGHPUT_EVENT_CCCD_WRITE, handle, data);
            ble_throughput_notify_enabled = *(uint16_t *)data;
            LOG_I(BLE_TPS, "Cccd read value:%d", ble_throughput_notify_enabled);

        } else if (rw == BT_GATTS_CALLBACK_READ) {
            if (size != 0) {
                uint16_t *buf = (uint16_t*) data;
                *buf = (uint16_t)ble_throughput_notify_enabled;
                LOG_I(BLE_TPS, "Cccd write value:%d", *buf);
            }
        } else {
            LOG_I(BLE_TPS, "Cccd callback opcode:%d, size:%d", rw, size);
        }
        return sizeof(uint16_t);
    }
    return 0;
}

bt_status_t ble_throughput_notify_test(bt_handle_t conn_handle, uint8_t data)
{
    uint8_t buf[260] = {0};
    bt_gattc_charc_value_notification_indication_t *tps_noti_rsp;
    bt_status_t result = BT_STATUS_FAIL;

    tps_noti_rsp = (bt_gattc_charc_value_notification_indication_t*)buf;

    if ((conn_handle > 0) && (BLE_TPS_CCCD_NOTIFICATION == ble_throughput_notify_enabled)) {
        tps_noti_rsp->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;
        tps_noti_rsp->att_req.handle = TPS_TX_CHAR_VALUE_HANDLE;
        memset((void*)(tps_noti_rsp->att_req.attribute_value), data, (peer_mtu-3));

        tps_noti_rsp->attribute_value_length = peer_mtu;

        result = bt_gatts_send_charc_value_notification_indication(conn_handle, tps_noti_rsp);
    }

    if (result == BT_STATUS_SUCCESS) {
        //LOG_I(BLE_TPS, "Tx notify data:%x (size:%d) conn_handle:%x", data, length, conn_handle);
    } else {
        //LOG_I(BLE_TPS, "Tx notify data fail!(%x)", result);
    }

    return result;
}

bt_status_t ble_throughput_notify(bt_handle_t conn_handle, uint8_t *data, uint32_t length)
{
    uint8_t buf[260] = {0};
    bt_gattc_charc_value_notification_indication_t *tps_noti_rsp;
    bt_status_t result = BT_STATUS_FAIL;

    tps_noti_rsp = (bt_gattc_charc_value_notification_indication_t*)buf;

    if ((conn_handle > 0) && (BLE_TPS_CCCD_NOTIFICATION == ble_throughput_notify_enabled)) {
        tps_noti_rsp->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;
        tps_noti_rsp->att_req.handle = TPS_TX_CHAR_VALUE_HANDLE;
        memcpy((void*)(tps_noti_rsp->att_req.attribute_value), data, length);

        tps_noti_rsp->attribute_value_length = 3 + length;

        result = bt_gatts_send_charc_value_notification_indication(conn_handle, tps_noti_rsp);
    }

    if (result == BT_STATUS_SUCCESS) {
        LOG_I(BLE_TPS, "Tx notify data:%x (size:%d) conn_handle:%x", data[0], length, conn_handle);
    } else {
        LOG_I(BLE_TPS, "Tx notify data fail!");
    }

    return result;
}

bt_status_t ble_throughput_notify_indication(bt_handle_t conn_handle, uint8_t *data, uint32_t length)
{
    uint8_t buf[260]={0};
    bt_gattc_charc_value_notification_indication_t *tps_noti_rsp;
    bt_status_t result = BT_STATUS_FAIL;

    if (length > 257) {
        length = 257;
    }
    tps_noti_rsp = (bt_gattc_charc_value_notification_indication_t*) buf;

    if ((conn_handle > 0) && (BLE_TPS_CCCD_NOTIFICATION == ble_throughput_notify_enabled)) {
        tps_noti_rsp->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_INDICATION;
        tps_noti_rsp->att_req.handle = TPS_TX_CHAR_VALUE_HANDLE;
        memcpy((void*)(tps_noti_rsp->att_req.attribute_value), data, length);

        tps_noti_rsp->attribute_value_length = 3 + length;

        result = bt_gatts_send_charc_value_notification_indication(conn_handle, tps_noti_rsp);
    }

    if (result == BT_STATUS_SUCCESS) {
        //LOG_I(BLE_TPS, "Tx ind data:%x (size:%d) conn_handle:%x", data[0], length, conn_handle);
    } else {
        LOG_I(BLE_TPS, "Tx ind data fail!");
    }

    return result;
}

void ble_throughput_gatt_server_test(bool enable)
{
    g_ble_gatt_server_test_enable = enable;
    if (!enable) {
        g_ble_tps_data_first = 1;
    }
}

bool ble_throughput_is_server_testing(void)
{
    return g_ble_gatt_server_test_enable;
}

void ble_throughput_set_peer_mtu(uint16_t mtu)
{
    if (mtu > 260) {
        mtu = 260;
    }

    peer_mtu = mtu;
}

#endif  /* BLE_THROUGHPUT */


