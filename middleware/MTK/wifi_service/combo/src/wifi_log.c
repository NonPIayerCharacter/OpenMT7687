/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

extern uint8_t wifi_get_debug_level(void);

const char *mem_err_ptr = "memrory allocate fail.";
const char *addr_err_ptr = "null pointer error.";
const char *rxfilter_err_ptr = "recover rxfitler fail";
const char *port_err_ptr = "port is invalid";

static void hex_dump_data_array(const unsigned char *data, int length)
{
    int index = 0;
    for (index = 0; index < length; index++) {
        printf("%02X", (int)(data[index]));
        if ((index + 1) % 16 == 0) {
            printf("\r\n");
            continue;
        }
        if (index + 1 != length) {
            printf(" ");
        }
    }
    if (0 != index && 0 != index % 16) {
        printf("\r\n");//add one more blank line
    }
}

void wifi_msg_output(int module, const char *func, int line, int level, const char *message, ...)
{
    uint8_t wifi_dbg_level = wifi_get_debug_level();

    va_list ap;
    va_start(ap, message);
    if (level <= wifi_dbg_level) 
    {
        vprintf(message, ap);
        printf("\r\n");
    }
    va_end(ap);
}

void wifi_hex_dump(int moudle, const char *func, int line, int level, const void *data, int length, const char *message, ...)
{
    uint8_t wifi_dbg_level = wifi_get_debug_level();
    
    va_list ap;
    va_start(ap, message);
    if (level <= wifi_dbg_level)  
    {
        vprintf(message, ap);
        printf("\r\n");
        hex_dump_data_array(data, length);
    }
    va_end(ap);
}

int wifi_msg_output_rom(const char *message, ...)
{
    va_list ap;
    va_start(ap, message);

    vprintf(message, ap);
    printf("\r\n");

    va_end(ap);

    return 0;
}

void wifi_hex_dump_rom(char *message, unsigned char *data, unsigned int length)
{
    printf("%s\r\n", message);
    hex_dump_data_array(data, length);
}