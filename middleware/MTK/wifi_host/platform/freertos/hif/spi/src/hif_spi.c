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

#include "memory_attribute.h"
#include "hal_spi_master.h"
#include "hal_gpio.h"
#include "hal_gpt.h"
#include "hif_spi.h"
#include "hif_spi_config.h"
#include "assert.h"

#define HIF_SPI_LOG(fmt, ...) printf("[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#if 1
#define __ASSERT__(x) assert(x)
#else
#define __ASSERT__(x)
#endif

/* MT5932 relative macros define  */
#define SPIS_CFG_4BYTE_ADDR     (0x01 << 2)
#define SPIS_CFG_SINGLE_MODE    (0x00 << 0)
#define SPIS_CFG_DUAL_MODE      (0x01 << 0)
#define SPIS_CFG_QUAD_MODE      (0x02 << 0)

#define MT5932_STA_TXRX_FIFO_RDY_OFFSET    (2)
#define MT5932_STA_TXRX_FIFO_RDY_MASK      (0x1 << MT5932_STA_TXRX_FIFO_RDY_OFFSET)
#define MT5932_STA_RDWR_FINISH_OFFSET      (5)
#define MT5932_STA_RDWR_FINISH_MASK        (0x1 << MT5932_STA_RDWR_FINISH_OFFSET)

#define MAX_TX_BUFFER_SIZE      1024    /* Tx buffer size decided by user's request */
#define MAX_RX_BUFFER_SIZE      1800    /* Rx buffer size decided by user's request */
uint8_t g_transfer_mode = HAL_SPI_MASTER_SINGLE_MODE;
static volatile bool g_spi_transaction_finish;
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t g_transfer_tx_buffer[MAX_TX_BUFFER_SIZE];
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t g_transfer_rx_buffer[MAX_RX_BUFFER_SIZE];

static void spi_master_dma_callback(hal_spi_master_callback_event_t event, void *user_data)
{
    g_spi_transaction_finish = true;
}

static void spi_query_slave_status(uint8_t *status)
{
    uint8_t status_cmd = SPIS_RS_CMD;
    uint8_t status_receive[2];
    hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;

    status_receive[1] = 0;
    spi_send_and_receive_config.receive_length = 2;
    spi_send_and_receive_config.send_length = 1;
    spi_send_and_receive_config.send_data = &status_cmd;
    spi_send_and_receive_config.receive_buffer = status_receive;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_and_receive_polling(SPIM_TEST_PORT, &spi_send_and_receive_config)) {
        return;
    }
    HIF_SPI_LOG("receive_status:0x%x\r\n", status_receive[1]);
    *status = status_receive[1];
}

bool spi_check_slave_status(uint8_t status, bool iseq)
{
    uint32_t retry_times = 2000;
    uint8_t status_recv = 0;
    bool is_status_ok = false;

    for(;;) {
        spi_query_slave_status(&status_recv);
        if (iseq) {
            is_status_ok = ((status_recv & (uint8_t)status) == (uint8_t)status);
        } else {
            is_status_ok = ((status_recv & (uint8_t)status) != (uint8_t)status);
        }
        if ((--retry_times == 0) || (is_status_ok)) {
            break;
        } else {
            hal_gpt_delay_ms(5);
        }
    }

    return (retry_times == 0) ? (false) : (true);
}

bool wifi_bus_init(void)
{
    HIF_SPI_LOG("\r\n");
    uint8_t poweron_cmd;
    uint8_t status_receive;
    uint32_t result;
    hal_spi_master_config_t spi_config;
    hal_spi_master_chip_select_timing_t chip_select_timing;
    hal_spi_master_advanced_config_t advanced_config;
    hal_spi_master_running_status_t running_status;
#if 0
    /* Reset MT5932, XTAL Select traping share with SPI CS pin, so we set to 0 before reset MT5932 */
    hal_gpio_init(SPIM_PIN_NUMBER_CS);
    hal_pinmux_set_function(SPIM_PIN_NUMBER_CS, 0);
    hal_gpio_set_direction(SPIM_PIN_NUMBER_CS, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(SPIM_PIN_NUMBER_CS, HAL_GPIO_DATA_LOW);
    hal_gpio_init(SPIM_PIN_NUMBER_MT5932_RESET);
    hal_pinmux_set_function(SPIM_PIN_NUMBER_MT5932_RESET, 0);
    hal_gpio_set_direction(SPIM_PIN_NUMBER_MT5932_RESET, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(SPIM_PIN_NUMBER_MT5932_RESET, HAL_GPIO_DATA_LOW);
    hal_gpt_delay_ms(200);
    hal_gpio_set_output(SPIM_PIN_NUMBER_MT5932_RESET, HAL_GPIO_DATA_HIGH);

    /* wait MT5932 finish straping */
    hal_gpt_delay_ms(500);
#endif
#if 1
    /* Step1: GPIO configuration for SPI master */
    hal_gpio_init(SPIM_PIN_NUMBER_CS);
    hal_gpio_init(SPIM_PIN_NUMBER_CLK);
    hal_gpio_init(SPIM_PIN_NUMBER_MOSI);
    hal_gpio_init(SPIM_PIN_NUMBER_MISO);
    // hal_gpio_init(SPIM_PIN_NUMBER_SIO2);
    // hal_gpio_init(SPIM_PIN_NUMBER_SIO3);
    hal_pinmux_set_function(SPIM_PIN_NUMBER_CS, SPIM_PIN_FUNC_CS);
    hal_pinmux_set_function(SPIM_PIN_NUMBER_CLK, SPIM_PIN_FUNC_CLK);
    hal_pinmux_set_function(SPIM_PIN_NUMBER_MOSI, SPIM_PIN_FUNC_MOSI);
    hal_pinmux_set_function(SPIM_PIN_NUMBER_MISO, SPIM_PIN_FUNC_MISO);
    // hal_pinmux_set_function(SPIM_PIN_NUMBER_SIO2, SPIM_PIN_FUNC_SIO2);
    // hal_pinmux_set_function(SPIM_PIN_NUMBER_SIO3, SPIM_PIN_FUNC_SIO3);

    hal_gpio_set_driving_current(SPIM_PIN_NUMBER_CS  , HAL_GPIO_DRIVING_CURRENT_2MA);
    hal_gpio_set_driving_current(SPIM_PIN_NUMBER_CLK , HAL_GPIO_DRIVING_CURRENT_2MA);
    hal_gpio_set_driving_current(SPIM_PIN_NUMBER_MOSI, HAL_GPIO_DRIVING_CURRENT_2MA);
    hal_gpio_set_driving_current(SPIM_PIN_NUMBER_MISO, HAL_GPIO_DRIVING_CURRENT_2MA);
    // hal_gpio_set_driving_current(SPIM_PIN_NUMBER_SIO2, HAL_GPIO_DRIVING_CURRENT_2MA);
    // hal_gpio_set_driving_current(SPIM_PIN_NUMBER_SIO3, HAL_GPIO_DRIVING_CURRENT_2MA);

    hal_gpt_delay_ms(1000);


    /* Step2: Initializes SPI master with normal mode */
    spi_config.bit_order = HAL_SPI_MASTER_LSB_FIRST;
    spi_config.clock_frequency = SPIM_TEST_FREQUENCY;
    spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
    spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
    spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_init(SPIM_TEST_PORT, &spi_config)) {
        return false;
    }
    if (HAL_SPI_MASTER_STATUS_OK != (result = hal_spi_master_get_running_status(SPIM_TEST_PORT, &running_status))) {
        HIF_SPI_LOG("spi master running_status:%d result:%d\r\n", running_status, result);
        return false;
    }

    advanced_config.byte_order = HAL_SPI_MASTER_LITTLE_ENDIAN;
    advanced_config.chip_polarity = HAL_SPI_MASTER_CHIP_SELECT_LOW;
    advanced_config.get_tick = HAL_SPI_MASTER_NO_GET_TICK_MODE;
    advanced_config.sample_select = HAL_SPI_MASTER_SAMPLE_POSITIVE;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_advanced_config(SPIM_TEST_PORT, &advanced_config)) {
        return false;
    }

    chip_select_timing.chip_select_setup_count = 0;
    chip_select_timing.chip_select_hold_count = 0;
    chip_select_timing.chip_select_idle_count = 0;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_chip_select_timing(SPIM_TEST_PORT, chip_select_timing)) {
        return false;
    }

    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_dummy_bits(SPIM_TEST_PORT, 0)) {
        return false;
    }

    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_deassert(SPIM_TEST_PORT, HAL_SPI_MASTER_DEASSERT_DISABLE)) {
        return false;
    }
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_register_callback(SPIM_TEST_PORT, spi_master_dma_callback, NULL)) {
        return false;
    }
#else
    spi_config_t config;
    spi_set_default_config(&config);
    config.mst_cs_driving  = HAL_GPIO_DRIVING_CURRENT_8MA;
    config.mst_sck_driving = HAL_GPIO_DRIVING_CURRENT_8MA;
    config.mst_sio_driving = HAL_GPIO_DRIVING_CURRENT_8MA;
    config.slv_sio_driving = HAL_GPIO_DRIVING_CURRENT_8MA;

    config.master_pin_group = 0;
    config.slave_pin_group = 0xFFFFFFFF;
    spi_test_init(spi_get_config(&config));

    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_register_callback(SPIM_TEST_PORT, spi_master_dma_callback, NULL)) {
        return false;
    }
#endif

#if 0
    /* Step3: SPI Master starts to send power_on command to SPI slave */
    do {
        poweron_cmd = SPIS_PWON_CMD;
        if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(SPIM_TEST_PORT, &poweron_cmd, 1)) {
            return false;
        }
        hal_gpt_delay_ms(1);
        spi_query_slave_status(&status_receive);
    } while ((status_receive & (uint8_t)SPIS_STA_SLV_ON_MASK) != (uint8_t)SPIS_STA_SLV_ON_MASK);

    /* Step4: Config SPI slave to use quad mode */
    cfg_mode[0] = SPIS_CT_CMD;
    cfg_mode[1] = SPIS_CFG_4BYTE_ADDR | SPIS_CFG_SINGLE_MODE;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(SPIM_TEST_PORT, cfg_mode, 2)) {
        return false;
    }
#endif

    HIF_SPI_LOG("wifi_bus_init done. [%d]\r\n",spi_config.clock_frequency);

    return true;
}

bool wifi_bus_deinit(void)
{
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_deinit(SPIM_TEST_PORT)) {
        return false;
    }

    hal_gpio_deinit(SPIM_PIN_NUMBER_CS);
    hal_gpio_deinit(SPIM_PIN_NUMBER_CLK);
    hal_gpio_deinit(SPIM_PIN_NUMBER_MOSI);
    hal_gpio_deinit(SPIM_PIN_NUMBER_MISO);
    hal_gpio_deinit(SPIM_PIN_NUMBER_SIO2);
    hal_gpio_deinit(SPIM_PIN_NUMBER_SIO3);

    return true;
}

bool wifi_read_bus(uint32_t offset, uint16_t length, uint8_t *read_buffer)
{
    uint8_t cfg_rd_cmd[9];
    uint8_t status_receive = 0;
    hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;
    HIF_SPI_LOG("start offset=%d length=%d \r\n", offset, length);

    cfg_rd_cmd[0] = SPIS_CFG_RD_CMD;
    cfg_rd_cmd[1] = offset & 0xff;
    cfg_rd_cmd[2] = (offset >> 8) & 0xff;
    cfg_rd_cmd[3] = (offset >> 16) & 0xff;
    cfg_rd_cmd[4] = (offset >> 24) & 0xff;
    cfg_rd_cmd[5] = (length - 1) & 0xff;
    cfg_rd_cmd[6] = ((length - 1) >> 8) & 0xff;
    cfg_rd_cmd[7] = ((length - 1) >> 16) & 0xff;
    cfg_rd_cmd[8] = ((length - 1) >> 24) & 0xff;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(SPIM_TEST_PORT, cfg_rd_cmd, 9)) {
        return false;
    }
#if 0
    do {
        //hal_gpt_delay_ms(5);
        spi_query_slave_status(&status_receive);
        HIF_SPI_LOG("status_receive = 0x%02x\r\n", status_receive);
    } while ((status_receive & (uint8_t)MT5932_STA_TXRX_FIFO_RDY_MASK) != (uint8_t)MT5932_STA_TXRX_FIFO_RDY_MASK);
#else
    if (!spi_check_slave_status(MT5932_STA_TXRX_FIFO_RDY_MASK, 1)) {
        HIF_SPI_LOG("check MT5932_STA_TXRX_FIFO_RDY_MASK status fail\r\n");
        __ASSERT__(0);
		return false;
    }
#endif
	if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_mode(SPIM_TEST_PORT, g_transfer_mode)) {
		return false;
	}
    if ((g_transfer_mode !=HAL_SPI_MASTER_SINGLE_MODE) && (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_command_bytes(SPIM_TEST_PORT, 1))) {
        return false;
    }
    /* if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_dummy_bits(SPIM_TEST_PORT, 0)) {
        return false;
    }*/
    g_spi_transaction_finish = false;
    g_transfer_tx_buffer[0] = SPIS_RD_CMD;
    os_memset(read_buffer, 0, (length + 1));
    spi_send_and_receive_config.receive_length = length + 1;
    spi_send_and_receive_config.send_length = 1;
    spi_send_and_receive_config.send_data = g_transfer_tx_buffer;
    spi_send_and_receive_config.receive_buffer = g_transfer_rx_buffer;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_and_receive_dma(SPIM_TEST_PORT, &spi_send_and_receive_config)) {
        return false;
    }
    while (g_spi_transaction_finish == false);
	if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_mode(SPIM_TEST_PORT, HAL_SPI_MASTER_SINGLE_MODE)) {
		return false;
	}
    /* if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_dummy_bits(SPIM_TEST_PORT, 0)) {
        return false;
    }*/
#if 0
    do {
        spi_query_slave_status(&status_receive);
        HIF_SPI_LOG("status_receive = 0x%02x\r\n", status_receive);
    } while ((status_receive & (uint8_t)MT5932_STA_RDWR_FINISH_MASK) != (uint8_t)MT5932_STA_RDWR_FINISH_MASK);
#else
    if (!spi_check_slave_status(MT5932_STA_RDWR_FINISH_MASK, 1)) {
        HIF_SPI_LOG("check MT5932_STA_RDWR_FINISH_MASK status fail\r\n");
        __ASSERT__(0);
		return false;
    }
#endif
    os_memcpy(read_buffer, &(g_transfer_rx_buffer[1]), length);

    return true;
}

bool wifi_write_bus(uint32_t offset, uint16_t length, uint8_t *write_buffer)
{
    uint8_t cfg_wr_cmd[9];
    uint8_t status_receive = 0;
    HIF_SPI_LOG("start offset=%d length=%d \r\n", offset, length);

    cfg_wr_cmd[0] = SPIS_CFG_WR_CMD;
    cfg_wr_cmd[1] = offset & 0xff;
    cfg_wr_cmd[2] = (offset >> 8) & 0xff;
    cfg_wr_cmd[3] = (offset >> 16) & 0xff;
    cfg_wr_cmd[4] = (offset >> 24) & 0xff;
    cfg_wr_cmd[5] = (length - 1) & 0xff;
    cfg_wr_cmd[6] = ((length - 1) >> 8) & 0xff;
    cfg_wr_cmd[7] = ((length - 1) >> 16) & 0xff;
    cfg_wr_cmd[8] = ((length - 1) >> 24) & 0xff;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(SPIM_TEST_PORT, cfg_wr_cmd, 9)) {
        return false;
    }
        //hal_gpt_delay_ms(5);
#if 0
    while ((status_receive & (uint8_t)MT5932_STA_TXRX_FIFO_RDY_MASK) != (uint8_t)MT5932_STA_TXRX_FIFO_RDY_MASK) {
        spi_query_slave_status(&status_receive);
        HIF_SPI_LOG("  status_receive = 0x%02x\r\n", status_receive);
    }
#else
    if (!spi_check_slave_status(MT5932_STA_TXRX_FIFO_RDY_MASK, 1)) {
        HIF_SPI_LOG("check MT5932_STA_TXRX_FIFO_RDY_MASK status fail\r\n");
        __ASSERT__(0);
		return false;
    }
#endif
	if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_mode(SPIM_TEST_PORT, g_transfer_mode)) {
		return false;
	}
    if ((g_transfer_mode !=HAL_SPI_MASTER_SINGLE_MODE) && (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_command_bytes(SPIM_TEST_PORT, 1))) {
        return false;
    }

/*    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_dummy_bits(SPIM_TEST_PORT, 0)) {
        return false;
    }*/
    g_spi_transaction_finish = false;
    g_transfer_tx_buffer[0] = SPIS_WR_CMD;
    os_memcpy(&(g_transfer_tx_buffer[1]), write_buffer, length);
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_dma(SPIM_TEST_PORT, g_transfer_tx_buffer, (length + 1))) {
        return false;
    }
    while (g_spi_transaction_finish == false);
	if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_mode(SPIM_TEST_PORT, HAL_SPI_MASTER_SINGLE_MODE)) {
		return false;
	}
   /* if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_dummy_bits(SPIM_TEST_PORT, 0)) {
        return false;
    }*/
#if 0
    do {
        spi_query_slave_status(&status_receive);
        HIF_SPI_LOG("  status_receive = 0x%02x\r\n", status_receive);
    } while ((status_receive & (uint8_t)MT5932_STA_RDWR_FINISH_MASK) != (uint8_t)MT5932_STA_RDWR_FINISH_MASK);
#else
    if (!spi_check_slave_status(MT5932_STA_RDWR_FINISH_MASK, 1)) {
        HIF_SPI_LOG("check MT5932_STA_RDWR_FINISH_MASK status fail\r\n");
        __ASSERT__(0);
		return false;
    }
#endif

    return true;
}

bool wifi_lock_bus(void)
{
    uint8_t poweron_cmd;
    uint8_t status_receive = 0;
    uint8_t cfg_mode[2];
    HIF_SPI_LOG("start \r\n");

    poweron_cmd = SPIS_PWON_CMD;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(SPIM_TEST_PORT, &poweron_cmd, 1)) {
        return false;
    }
#if 0
    do {
        //hal_gpt_delay_ms(1);
        spi_query_slave_status(&status_receive);
    } while ((status_receive & (uint8_t)SPIS_STA_SLV_ON_MASK) != (uint8_t)SPIS_STA_SLV_ON_MASK);
#else
    if (!spi_check_slave_status(SPIS_STA_SLV_ON_MASK, 1)) {
        HIF_SPI_LOG("check SPIS_STA_SLV_ON_MASK status fail\r\n");
        __ASSERT__(0);
		return false;
    }
#endif
    /* Config SPI slave's transfer mode align with SPI master*/
    cfg_mode[0] = SPIS_CT_CMD;
    cfg_mode[1] = SPIS_CFG_4BYTE_ADDR | g_transfer_mode;
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(SPIM_TEST_PORT, cfg_mode, 2)) {
        return false;
    }

    return true;
}

bool wifi_unlock_bus(void)
{
    uint8_t poweroff_cmd;
    uint8_t status_receive = 0;
    poweroff_cmd = SPIS_PWOFF_CMD;
    HIF_SPI_LOG("start \r\n");

    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(SPIM_TEST_PORT, &poweroff_cmd, 1)) {
        return false;
    }
#if 0
    do {
        //hal_gpt_delay_ms(1);
        spi_query_slave_status(&status_receive);
    } while ((status_receive & (uint8_t)SPIS_STA_SLV_ON_MASK) == (uint8_t)SPIS_STA_SLV_ON_MASK);
#else
    if (!spi_check_slave_status(SPIS_STA_SLV_ON_MASK, 0)) {
        HIF_SPI_LOG("check SPIS_STA_SLV_ON_MASK status fail\r\n");
        __ASSERT__(0);
		return false;
    }
#endif

    return true;
}

