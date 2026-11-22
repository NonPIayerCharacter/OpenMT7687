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
 
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* hal includes */
#include "hal.h"
#include "memory_attribute.h"
#include "ff.h"

/*hal pinmux define*/
#include "hal_pinmux_define.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
    FIL fdst;                        /* file target which must be a global variable name if it would be accessed with global scope. */
    FATFS fatfs;                     /* fs target which must be a global variable name if it would be accessed with global scope. */
	
/* Private functions ---------------------------------------------------------*/

#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the HAL_UART_0 one at a time */
    hal_uart_put_char(HAL_UART_0, ch);
    return ch;
}

/**
*@brief Configure and initialize UART hardware initialization for logging.
*@param None.
*@return None.
*/
static void plain_log_uart_init(void)
{
    hal_uart_config_t uart_config;
	
    /* gpio config for uart0 */
    hal_gpio_init(HAL_GPIO_19);
    hal_gpio_init(HAL_GPIO_20);
    hal_pinmux_set_function(HAL_GPIO_19, HAL_GPIO_19_URXD0);
    hal_pinmux_set_function(HAL_GPIO_20, HAL_GPIO_20_UTXD0);

    /* COM port settings */
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.parity = HAL_UART_PARITY_NONE;
    hal_uart_init(HAL_UART_0, &uart_config);
}

/**
*@brief Configure and initialize the systerm clock. In this example, we invoke hal_clock_init to initialize clock driver and clock gates.
*@param None.
*@return None.
*/
static void SystemClock_Config(void)
{
    hal_clock_init();
}

/**
*@brief  Initialize the periperal driver in this function. In this example, we initialize UART, Flash, and NVIC drivers.
*@param None.
*@return None.
*/
static void prvSetupHardware(void)
{
    /* Peripherals initialization */
    plain_log_uart_init();
    hal_flash_init();
    hal_nvic_init();
}

/**
* @brief       write one file and then read the file to verify whether the file is written successfully.
* @param[in]   None.
* @return      None.
*/
BYTE buffer_src[1024];
BYTE buffer_dst[1024];
BYTE test_wrok[1024];

static void fatfs_read_write_file(void)
{
    FRESULT res;                     /* fs status infor*/
    UINT temp_count, length_written, length_read;

    for (temp_count = 0; temp_count < 1000; temp_count++) {
        buffer_src[temp_count] = temp_count;
    }

    /* Mount the partition before it is accessed, and need only do it once. */
    res = f_mount(&fatfs, _T("0:"), 1);
    res = f_mkfs(_T("0:"), FM_ANY, 0 , test_wrok, 1024);
 
    if(res == FR_OK) {
        printf("1-make FAT32 FS ok !!!!!!\r\n");
    }
	
    res = f_mount(&fatfs, _T("0:"), 1);
    if (FR_OK != res) {
        printf( "2-Failed to mount the partition,res=%d\r\n",res);
        return;
    }

    /* Step0: Create a new file for writing and reading. */
    res = f_open(&fdst, _T("0:/Test_W.txt"), FA_CREATE_ALWAYS);
    if (FR_OK != res) {
        printf( "3-Failed to create the file,res=%d\r\n",res);
        return;
    }

    res = f_open(&fdst, _T("0:/Test_W.txt"), FA_OPEN_EXISTING | FA_WRITE | FA_READ);
    if (FR_OK != res) {;
        printf( "4-Failed to open the file,res=%d\r\n",res);
        return;
    }

    /* Step2: Write the file */
    res = f_write(&fdst, buffer_src, 1000, &length_written);
    if (FR_OK != res) {
        printf( "5-Failed to write the file,res=%d\r\n",res);
        res = f_close(&fdst);
        return;
    }

    /* Close the open file object, flush the cached information of the file back to the volume.*/
    res = f_close(&fdst);
    if (FR_OK != res) {
        printf( "6-Failed to close the file,res=%d\r\n",res);
        res = f_close(&fdst);
        return;
    }       
    
    res = f_open(&fdst, _T("0:/Test_W.txt"), FA_OPEN_EXISTING | FA_READ);
    if (FR_OK != res) {
        printf( "7-Failed to open the file,res=%d\r\n",res);
        res = f_close(&fdst);
        return;
    }

    /* Step3: Read the file and verify whether the data is the same as written. */
    res = f_read(&fdst, buffer_dst, 1000, &length_read);
    if (FR_OK != res) {
        printf( "8-Failed to read the file,res=%d\r\n",res);
        res = f_close(&fdst);
        return;
    }

    for (temp_count = 0; temp_count < 1000; temp_count++) {
        if (buffer_dst[temp_count] != buffer_src[temp_count]) {
            printf( "9-Failed: data read is different from the data written,temp_count=%d\r\n",temp_count);
            res = f_close(&fdst);
            return;
        }
    }
    res = f_close(&fdst);

    printf( "The file is written and read successfully!\r\n");
}

int main(void)
{
    /* Configure system clock */
    SystemClock_Config();

    SystemCoreClockUpdate();

    /* Configure the hardware */
    prvSetupHardware();

    /* Enable I,F bits */
    __enable_irq();
    __enable_fault_irq();

    /* Add your application code here */
    printf("\r\n\r\n");/* The output UART used by printf is set by plain_log_uart_init() */
    printf("welcome to main()\r\n");
    printf("pls add your own code from here\r\n");
    printf("\r\n\r\n");

    fatfs_read_write_file();

    while (1);
}

