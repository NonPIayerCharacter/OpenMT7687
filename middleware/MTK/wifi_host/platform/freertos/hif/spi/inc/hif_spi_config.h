/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __HIF_SPI_CONFIG_H__
#define __HIF_SPI_CONFIG_H__

/* Define SPI slave command  */
#define SPIS_CFG_RD_CMD         0x0a
#define SPIS_RD_CMD             0x81
#define SPIS_CFG_WR_CMD         0x0c
#define SPIS_WR_CMD             0x0e
#define SPIS_RS_CMD             0x06
#define SPIS_PWON_CMD           0x04
#define SPIS_PWOFF_CMD          0x02
#define SPIS_CT_CMD             0x10

/* Define Host SPI frequency */
#define SPIM_TEST_FREQUENCY     1000000//24000000

/* Define Host SPI port */
#define SPIM_TEST_PORT          HAL_SPI_MASTER_0

/* Define Host SPI relative pinmux */
#define SPIM_PIN_NUMBER_CS      HAL_GPIO_9
#define SPIM_PIN_FUNC_CS        3
#define SPIM_PIN_NUMBER_CLK     HAL_GPIO_10
#define SPIM_PIN_FUNC_CLK       3
#define SPIM_PIN_NUMBER_MOSI    HAL_GPIO_12
#define SPIM_PIN_FUNC_MOSI      2
#define SPIM_PIN_NUMBER_MISO    HAL_GPIO_13
#define SPIM_PIN_FUNC_MISO      2
#define SPIM_PIN_NUMBER_SIO2    HAL_GPIO_7
#define SPIM_PIN_FUNC_SIO2      3
#define SPIM_PIN_NUMBER_SIO3    HAL_GPIO_6
#define SPIM_PIN_FUNC_SIO3      3

/* Define slave reset pinmux */
#define SPIM_PIN_NUMBER_MT5932_RESET  HAL_GPIO_35  /* The GPIO pin to control reset pin of MT5932 */

/* Define MT5932 relative clock strap pinmux */
#define SPIM_PIN_NUMBER_MT5932_32K_CLK  HAL_GPIO_4

/* Temp macro */
//#define os_memcpy memcpy
//#define os_memset memset

#endif /* __HIF_SPI_CONFIG_H__ */

