/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/**
* @file wifi_wps.c
*
* WIFI WPS Feature with Enrollee and Registrar
*
*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "type_def.h"
#include "wifi_api.h"
#include "FreeRTOS.h"
#include "wifi_wps.h"
#include "syslog.h"
#include "os.h"
#include "os_util.h"
#ifdef MTK_MINISUPP_ENABLE
#include "driver_inband.h"
#endif

#include "wifi_event_id.h"
#include "wifi_default_config.h"
#include "wifi_scan.h"

#ifdef MTK_WIFI_ROM_ENABLE
#include "mlme_interface.h"

#include "wifi_event.h"
#include "wifi.h"
//#include "wifi_event_id.h"
/*In lwip task ip_ready_callback function will call 3 WiFi APIs to get driver status, but it have low rate to lead to dead lock, 
so disable the mutex protected by defalut. */
//#define MTK_WIFI_API_PROTECT_GET

void wifi_wps_wlan_evt_handler(unsigned char *body, int len)
{
#if defined(MTK_WIFI_WPS_ENABLE)
    iwreq_data_t    *wrqu = (iwreq_data_t *)body;

    if (wrqu->data.flags ==  RT_WSC_DONE_EVENT_FLAG)
    {
        unsigned char *extra = NULL;

        if (wrqu->data.length) 
        {
            extra = body + sizeof(iwreq_data_t) + sizeof(uint32_t);
        }

        wifi_event_notification(0,WIFI_EVENT_ID_IOT_WSC_DONE,extra,wrqu->data.length);
    }

    if (wrqu->data.flags ==  RT_WSC_INFO_CONNECTED_EVENT_FLAG)
    {
        unsigned char *extra = NULL;

        if (wrqu->data.length) 
        {
            extra = body + sizeof(iwreq_data_t) + sizeof(uint32_t);
        }

        wifi_event_notification(0,WIFI_EVENT_ID_IOT_WSC_INFO_CONNECTED,extra,wrqu->data.length);
    }
    if (wrqu->data.flags ==  RT_WSC_M2D_RECEIVED)
    {
        unsigned char *extra = NULL;

        if (wrqu->data.length) 
        {
            extra = body + sizeof(iwreq_data_t) + sizeof(uint32_t);
        }

        wifi_event_notification(0,WIFI_EVENT_ID_IOT_WSC_M2D_RECEIEVED,extra,wrqu->data.length);
    }
#endif
}
#endif

#if defined(MTK_WIFI_WPS_ENABLE)

#ifdef MTK_MINISUPP_ENABLE
extern struct timeval wait_time; /*Invoked WiFi API default waiting time*/

/* NULL means max delay*/
extern int32_t wpa_wifi_api_mutex_take(struct timeval *wait_time);
extern void wpa_wifi_api_mutex_give(void);
#endif
extern int32_t wifi_config_get_opmode_internel(uint8_t *mode);

int32_t wifi_wps_credential_event_handler(wifi_event_t event, uint8_t *payload, uint32_t length)
{
    LOG_I(wifi, "wifi_wps_credential_event_handler==>\n");
    LOG_I(wifi, "wps cred length = %d\n", length);
    uint8_t cred_passphrase_len = 0;
    uint8_t passphrase[WIFI_LENGTH_PASSPHRASE] = {0};

    wifi_wps_credential_info_t *customer_cred = (wifi_wps_credential_info_t *)payload;

    if (event == WIFI_EVENT_IOT_WPS_COMPLETE) {
        LOG_I(wifi, "WPS: ssid %s; ssid_len %d; auth_mode %d; encr_mode %d\n",
              customer_cred->ssid,
              customer_cred->ssid_len,
              customer_cred->auth_mode,
              customer_cred->encrypt_type);
        LOG_I(wifi, "WPS: key_length:%d\n", customer_cred->key_length);

        for (uint8_t i = 0; i < customer_cred->key_length; i++) {
            LOG_I(wifi, "WPS: key[%d] %02x\n", i, customer_cred->key[i]);
        }

        switch (customer_cred->encrypt_type) {
            case WIFI_ENCRYPT_TYPE_WEP_ENABLED:
                LOG_I(wifi, "WPS: key_index %d\n", customer_cred->key_index);
                break;
            case WIFI_ENCRYPT_TYPE_TKIP_ENABLED:
            case WIFI_ENCRYPT_TYPE_AES_ENABLED:
                os_memcpy(passphrase, customer_cred->key, sizeof(customer_cred->key));
                cred_passphrase_len = customer_cred->key_length;
                LOG_I(wifi, "WPS: passphrase %s\n", customer_cred->key);
                break;
            default :
                LOG_I(wifi, "WPS encrypt type %d\n", customer_cred->encrypt_type);
                break;
        }
    }

    LOG_I(wifi, "wifi_config_set_ssid==>\n");
    wifi_config_set_ssid(WIFI_PORT_STA, customer_cred->ssid, customer_cred->ssid_len);

    LOG_I(wifi, "wifi_config_set_security_mode==>\n");
    wifi_config_set_security_mode(WIFI_PORT_STA, customer_cred->auth_mode, customer_cred->encrypt_type);

    LOG_I(wifi, "wifi_config_set_wpa_psk_key==>\n");
    if (customer_cred->encrypt_type == WIFI_ENCRYPT_TYPE_TKIP_ENABLED || customer_cred->encrypt_type == WIFI_ENCRYPT_TYPE_AES_ENABLED) {
        LOG_I(wifi, "wifi_config_set_wpa_psk_key \n");
        wifi_config_set_wpa_psk_key(WIFI_PORT_STA, customer_cred->key, cred_passphrase_len);
    }

    wifi_wps_unregister_credential_handler();

    LOG_I(wifi, "wifi_config_reload_setting==>\n");
    wifi_config_reload_setting();

    return 0;
}
int32_t wifi_wps_register_credential_handler(void)
{
    LOG_I(wifi, "wifi_wps_register_credential_handler==>\n");
    return wifi_connection_register_event_handler(WIFI_EVENT_IOT_WPS_COMPLETE,
            (wifi_event_handler) wifi_wps_credential_event_handler);
}

int32_t wifi_wps_unregister_credential_handler(void)
{
    LOG_I(wifi, "wifi_wps_unregister_credential_handler==>\n");
    return wifi_connection_unregister_event_handler(WIFI_EVENT_IOT_WPS_COMPLETE,
            (wifi_event_handler) wifi_wps_credential_event_handler);
}

int32_t wifi_wps_config_set_auto_connection(bool auto_connection)
{
    uint8_t opmode = 0;
    LOG_I(wifi, "wifi_wps_config_set_wps_auto_connection==>\n");
    int32_t status = 0;
#ifdef MTK_MINISUPP_ENABLE
    if( WIFI_SUCC != wpa_wifi_api_mutex_take( &wait_time ) ) {
        LOG_E(wifi, WPA_WIFI_API_MUTEX_TAKE_FAIL);
        return WIFI_FAIL;
    }
#endif

    if (wifi_config_get_opmode_internel(&opmode) < 0) {
        //return -1;
        status = -1;
        goto final;
    }
    if(WIFI_MODE_REPEATER == opmode) {
        LOG_E(wifi, "Don't support WPS in Repeater mode.");
        //return -1;
        status = -1;
        goto final;
    }
#ifdef MTK_MINISUPP_ENABLE
    uint8_t driver_msg = 0; 
    driver_msg = auto_connection;
    if (WIFI_SUCC != wifi_event_notification_wait(WIFI_PORT_STA, WIFI_EVENT_ID_IOT_WPS_SET_AUTO_CONN_WAIT, &driver_msg, sizeof(driver_msg)) ){
        LOG_E(wifi, "Set_wps_auto_connection fail");
        //return -1;
        status = -1;
        goto final;
    }
    //return 0;
#else
    //return fw_mlme_wps_set_auto_connection(auto_connection);
    status = fw_mlme_wps_set_auto_connection(auto_connection);
#endif

final:
#ifdef MTK_MINISUPP_ENABLE
    wpa_wifi_api_mutex_give();
#endif
    return status;
}

int32_t wifi_wps_config_get_auto_connection(bool *auto_connection)
{
    uint8_t opmode = 0;
    LOG_I(wifi, "wifi_wps_config_get_wps_auto_connection==>\n");
    int32_t status = 0;
#if defined(MTK_MINISUPP_ENABLE) && defined(MTK_WIFI_API_PROTECT_GET)
    if( WIFI_SUCC != wpa_wifi_api_mutex_take( &wait_time ) ) {
        LOG_E(wifi, WPA_WIFI_API_MUTEX_TAKE_FAIL);
        return WIFI_FAIL;
    }
#endif

    if (wifi_config_get_opmode_internel(&opmode) < 0) {
        //return -1;
        status = -1;
        goto final;
    }
    if(WIFI_MODE_REPEATER == opmode) {
        LOG_E(wifi, "Don't support WPS in Repeater mode.");
        //return -1;
        status = -1;
        goto final;
    }
#ifdef MTK_MINISUPP_ENABLE
    if( __g_wpa_supplicant_api.wps_get_auto_connection ) {
        //return __g_wpa_supplicant_api.wps_get_auto_connection(auto_connection);
        status = __g_wpa_supplicant_api.wps_get_auto_connection(auto_connection);
        goto final;
    }
    //return -1;
    status = -1;
#else
    //return fw_mlme_wps_get_auto_connection(auto_connection);
    status = fw_mlme_wps_get_auto_connection(auto_connection);
#endif

final:
#if defined(MTK_MINISUPP_ENABLE) && defined(MTK_WIFI_API_PROTECT_GET)
    wpa_wifi_api_mutex_give();
#endif
    return status;
}

int32_t wifi_wps_connection_by_pbc(uint8_t port, uint8_t *bssid)
{
    uint8_t opmode = 0;
    int32_t status = 0;
#ifdef MTK_MINISUPP_ENABLE
    if( WIFI_SUCC != wpa_wifi_api_mutex_take( &wait_time ) ) {
        LOG_E(wifi, WPA_WIFI_API_MUTEX_TAKE_FAIL);
        return WIFI_FAIL;
    }
#endif

    if (wifi_config_get_opmode_internel(&opmode) < 0) {
        status = -1;
        goto final;
        //return -1;
    }
    if(WIFI_MODE_REPEATER == opmode) {
        LOG_E(wifi, "Don't support WPS in Repeater mode.");
        status = -1;
        goto final;
        //return -1;
    }
    if(WIFI_PORT_AP < port) {
        LOG_E(wifi, "Port=%d, it must be 0/1:STA/AP.", port);
        status = -1;
        goto final;
        //return -1;
    }
#ifdef MTK_MINISUPP_ENABLE
#if 1
    if( __g_wpa_supplicant_api.wps_connect_pbc ) {
        //return __g_wpa_supplicant_api.wps_connect_pbc(port, bssid);
        status = __g_wpa_supplicant_api.wps_connect_pbc(port, bssid);
        goto final;
    }
    //return -1;
    status = -1;
#endif
#else
    status = fw_mlme_wps_connection_by_pbc(port, bssid);
#endif

final:
#ifdef MTK_MINISUPP_ENABLE
    wpa_wifi_api_mutex_give();
#endif
    return status;
}


int32_t wifi_wps_connection_by_pin(uint8_t port, uint8_t *bssid, uint8_t *pin_code)
{
    uint8_t opmode = 0;
    int32_t status = 0;
#ifdef MTK_MINISUPP_ENABLE
    if( WIFI_SUCC != wpa_wifi_api_mutex_take( &wait_time ) ) {
        LOG_E(wifi, WPA_WIFI_API_MUTEX_TAKE_FAIL);
        return WIFI_FAIL;
    }
#endif

    if (wifi_config_get_opmode_internel(&opmode) < 0) {
        status = -1;
        goto final;
        //return -1;
    }
    if(WIFI_MODE_REPEATER == opmode) {
        LOG_E(wifi, "Don't support WPS in Repeater mode.");
        status = -1;
        goto final;
        //return -1;
    }
    if(WIFI_PORT_AP < port) {
        LOG_E(wifi, "Port=%d, it must be 0/1:STA/AP.", port);
        status = -1;
        goto final;
        //return -1;
    }

    if(pin_code == NULL)
    {
        LOG_E(wifi, "Null pin_code.");
        status = -1;
        goto final;
    }

    if( pin_code && 8 != strlen((char *)pin_code) && 4 != strlen((char *)pin_code)) {
        LOG_E(wifi, "Input pin_code len error.");
        status = -1;
        goto final;
        //return -1;
    }

#ifdef MTK_MINISUPP_ENABLE
#if 1
    if( __g_wpa_supplicant_api.wps_connect_pin ) {
        //return __g_wpa_supplicant_api.wps_connect_pin(port, bssid, pin_code);
        status = __g_wpa_supplicant_api.wps_connect_pin(port, bssid, pin_code);;
        goto final;
    }
    //return -1;
    status = -1;
#endif
#else
    //return fw_mlme_wps_connection_by_pin(port, bssid, pin_code);
    status = fw_mlme_wps_connection_by_pin(port, bssid, pin_code);
#endif

final:
#ifdef MTK_MINISUPP_ENABLE
    wpa_wifi_api_mutex_give();
#endif
    return status;
}


int32_t wifi_wps_config_get_pin_code(uint8_t port, uint8_t *pin_code)
{
    int32_t status = 0;
    int32_t buflen = 9;
    uint8_t opmode = 0;
#ifdef MTK_MINISUPP_ENABLE
    if( WIFI_SUCC != wpa_wifi_api_mutex_take( &wait_time ) ) {
        LOG_E(wifi, WPA_WIFI_API_MUTEX_TAKE_FAIL);
        return WIFI_FAIL;
    }
#endif

    if(NULL == pin_code) {
        LOG_E(wifi, "AP PIN NULL\n");
        status = -1;
        goto final;
        //return -1 ;
    }
    if (wifi_config_get_opmode_internel(&opmode) < 0) {
        status = -1;
        goto final;
        //return -1;
    }
    if(WIFI_MODE_REPEATER == opmode) {
        LOG_E(wifi, "Don't support WPS in Repeater mode.");
        status = -1;
        goto final;
        //return -1;
    }
#ifdef MTK_MINISUPP_ENABLE
    if( __g_wpa_supplicant_api.wps_get_pin ){
        status = __g_wpa_supplicant_api.wps_get_pin(port, pin_code);
    }
#else
    status = fw_mlme_wps_config_get_pin_code(port, pin_code);
#endif
    if (status >= 0) {
        if (strlen((const char *)pin_code) >= buflen) {
            LOG_E(wifi, "Lenth ERROR");
            status = -1;
            goto final;
            //return -1;
        }
        //return ret;
        goto final;
    }
    //return ret;

final:
#ifdef MTK_MINISUPP_ENABLE
    wpa_wifi_api_mutex_give();
#endif
    return status;
}

int32_t wifi_wps_get_device_info(wifi_wps_device_info_t *device_info)
{

    if (NULL == device_info) {
        return -1;
    }

    os_memset(device_info, 0, sizeof(wifi_wps_device_info_t));

	os_memcpy(device_info->device_name,   WIFI_DEFAULT_DEVICE_NAME,   sizeof(WIFI_DEFAULT_DEVICE_NAME));
	os_memcpy(device_info->manufacturer,  WIFI_DEFAULT_MANUFACTURER, sizeof(WIFI_DEFAULT_MANUFACTURER));
	os_memcpy(device_info->model_name,    WIFI_DEFAULT_MODEL_NAME, sizeof(WIFI_DEFAULT_MODEL_NAME));
	os_memcpy(device_info->model_number,  WIFI_DEFAULT_MODEL_NUMBER, sizeof(WIFI_DEFAULT_MODEL_NUMBER));
	os_memcpy(device_info->serial_number, WIFI_DEFAULT_SERIAL_NUMBER, sizeof(WIFI_DEFAULT_SERIAL_NUMBER));

    return 0;
}

int32_t wifi_wps_config_set_device_info(wifi_wps_device_info_t *device_info)
{
    uint8_t opmode = 0;
    int32_t status = 0;
    LOG_I(wifi, "wifi_wps_config_set_device_info==>\n");
#ifdef MTK_MINISUPP_ENABLE
    if( WIFI_SUCC != wpa_wifi_api_mutex_take( &wait_time ) ) {
        LOG_E(wifi, WPA_WIFI_API_MUTEX_TAKE_FAIL);
        return WIFI_FAIL;
    }
#endif

    if ( (wifi_config_get_opmode_internel(&opmode) < 0) || (NULL==device_info) ) {
        //return -1;
        status = -1;
        goto final;
    }
    if(WIFI_MODE_REPEATER == opmode) {
        LOG_E(wifi, "Don't support WPS in Repeater mode.");
        //return -1;
        status = -1;
        goto final;
    }
#ifdef MTK_MINISUPP_ENABLE
    uint8_t driver_msg[sizeof(wifi_wps_device_info_t)] = {0}; 
    os_memcpy(driver_msg, device_info, sizeof(wifi_wps_device_info_t));
    if (WIFI_SUCC != wifi_event_notification_wait(WIFI_PORT_STA, WIFI_EVENT_ID_IOT_WPS_SET_DEVICE_INFO_WAIT, (unsigned char *)&driver_msg, sizeof(driver_msg)) ){
        LOG_E(wifi, "set_device_info fail");
        //return -1;
        status = -1;
        goto final;
    }
    //return 0;
#else
    //return fw_mlme_wps_config_set_device_info(device_info);
    status = fw_mlme_wps_config_set_device_info(device_info);
#endif

final:
#ifdef MTK_MINISUPP_ENABLE
    wpa_wifi_api_mutex_give();
#endif
    return status;
}

int32_t wifi_wps_config_get_device_info(wifi_wps_device_info_t *device_info)
{
    uint8_t opmode = 0;
    int32_t status = 0;
    LOG_I(wifi, "wifi_wps_config_get_device_info==>\n");
#if defined(MTK_MINISUPP_ENABLE) && defined(MTK_WIFI_API_PROTECT_GET)
    if( WIFI_SUCC != wpa_wifi_api_mutex_take( &wait_time ) ) {
        LOG_E(wifi, WPA_WIFI_API_MUTEX_TAKE_FAIL);
        return WIFI_FAIL;
    }
#endif

    if (wifi_config_get_opmode_internel(&opmode) < 0) {
        //return -1;
        status = -1;
        goto final;
    }
    if(WIFI_MODE_REPEATER == opmode) {
        LOG_E(wifi, "Don't support WPS in Repeater mode.");
        //return -1;
        status = -1;
        goto final;
    }
#ifdef MTK_MINISUPP_ENABLE
    if( __g_wpa_supplicant_api.wps_get_device_info ) {
        //return __g_wpa_supplicant_api.wps_get_device_info(device_info);
        status = __g_wpa_supplicant_api.wps_get_device_info(device_info);
        goto final;
    }
    //return -1;
    status = -1;
#else
    //return fw_mlme_wps_config_get_device_info(device_info);
    status = fw_mlme_wps_config_get_device_info(device_info);
#endif

final:
#if defined(MTK_MINISUPP_ENABLE) && defined(MTK_WIFI_API_PROTECT_GET)
    wpa_wifi_api_mutex_give();
#endif
    return status;
}


int32_t wifi_wps_terminate_connect(int32_t port)
{
    int32_t ret = 0;


#ifdef MTK_WIFI_ROM_ENABLE
    ret = fw_mlme_wps_terminate_connect(port);
#endif

    return ret;
}

int wifi_wps_reg_pin(char *cmd)
{
#ifdef MTK_MINISUPP_ENABLE
    int32_t ret = 0;
    if( NULL==cmd ){
        LOG_E(wifi, "Cmd is NULL.");
        return -1;
    }
    
    ret = wifi_event_notification(WIFI_PORT_STA, WIFI_EVENT_ID_IOT_WPS_REG_PIN, (unsigned char *)cmd, os_strlen(cmd));
    if ( ret < 0 ) {
        LOG_E(wifi, "reg pin fail.");
        return -1;
    } else if( 1 == ret ){
        LOG_E(wifi, "NOT support wps_reg.");
        return -1;
    }
#endif
    return 0;
}

int wifi_config_set_wps_crdential_handler(char *cmd)
{
#ifdef MTK_MINISUPP_ENABLE
    int32_t ret = 0;
    if( NULL==cmd ){
        LOG_E(wifi, "Cmd is NULL.");
        return -1;
    }
    ret = wifi_event_notification_wait(WIFI_PORT_STA, WIFI_EVENT_ID_IOT_WPS_SET_CRDENTIAL_HDL_WAIT, (unsigned char *)cmd, os_strlen(cmd));
    if ( ret < 0 ) {
        LOG_E(wifi, "set crdential fail.");
        return -1;
    } else if( 1 == ret ){
        LOG_E(wifi, "NOT support .");
        return -1;
    }
#endif
    return 0;
}

#endif /* MTK_WIFI_WPS_ENABLE */


