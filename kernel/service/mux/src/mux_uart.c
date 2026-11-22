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

#include "mux.h"
#include "mux_port_device.h"
#include "hal_uart.h"

static volatile uint32_t g_uart_record_wptr_legnth[3] = {0};
static volatile bool g_uart_on_off_status[3] = {true, true, true};

#ifdef MTK_CPU_NUMBER_0

#include "hal_uart_internal.h"

static mux_irq_handler_t g_mux_irq_handler;
#define UART_PORT_INDEX_TO_MUX_PORT(port_index) (port_index+MUX_UART_0)

static void mux_uart_callback(hal_uart_callback_event_t event, void *user_data)
{
    uint8_t port_index = (uint32_t)user_data;

    PORT_MUX_UNUSED(user_data);

    if (event == HAL_UART_EVENT_READY_TO_WRITE) {
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index),MUX_EVENT_READY_TO_WRITE);
    } else if (event == HAL_UART_EVENT_READY_TO_READ) {
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index),MUX_EVENT_READY_TO_READ);
    }
    else if (event == HAL_UART_EVENT_WAKEUP_SLEEP) {
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index),MUX_EVENT_WAKEUP_FROM_SLEEP);
    }

}


mux_status_t port_mux_uart_normal_init(uint8_t port_index, mux_port_config_t *p_setting,mux_irq_handler_t irq_handler)
{
    hal_uart_config_t uart_config;
    hal_uart_dma_config_t dma_config;

    g_mux_irq_handler = irq_handler;

    uart_config.baudrate = p_setting->user_setting.dev_setting.uart.uart_config.baudrate;  //CONFIG_SYSLOG_BAUDRATE;
    uart_config.parity = p_setting->user_setting.dev_setting.uart.uart_config.parity;//HAL_UART_PARITY_NONE;
    uart_config.stop_bit = p_setting->user_setting.dev_setting.uart.uart_config.stop_bit;//HAL_UART_STOP_BIT_1;
    uart_config.word_length = p_setting->user_setting.dev_setting.uart.uart_config.word_length;//HAL_UART_WORD_LENGTH_8;

    dma_config.send_vfifo_buffer      = (uint8_t*)(uint32_t*)p_setting->tx_buf_addr;
    dma_config.send_vfifo_buffer_size = p_setting->tx_buf_size;
    dma_config.receive_vfifo_buffer   = (uint8_t*)(uint32_t*)p_setting->rx_buf_addr;
    dma_config.receive_vfifo_buffer_size = p_setting->rx_buf_size;
    dma_config.send_vfifo_threshold_size = dma_config.send_vfifo_buffer_size / 8;
    dma_config.receive_vfifo_threshold_size   = dma_config.receive_vfifo_buffer_size / 2;
    dma_config.receive_vfifo_alert_size       = 12;
    if(HAL_UART_STATUS_OK != hal_uart_init(port_index, &uart_config))
    {
        return MUX_STATUS_ERROR_INIT_FAIL;
    }
    if(HAL_UART_STATUS_OK != hal_uart_set_dma(port_index, &dma_config))
    {
        return MUX_STATUS_ERROR_INIT_FAIL;
    }

    if(p_setting->user_setting.dev_setting.uart.flowcontrol_type == MUX_UART_NONE_FLOWCONTROL)
    {
       hal_uart_disable_flowcontrol(port_index); 
    }
    else if(p_setting->user_setting.dev_setting.uart.flowcontrol_type == MUX_UART_SW_FLOWCONTROL)
    {
       hal_uart_set_software_flowcontrol(port_index,0x11,0x13,0x77);
    }
    else if(p_setting->user_setting.dev_setting.uart.flowcontrol_type == MUX_UART_HW_FLOWCONTROL)
    {
        hal_uart_set_hardware_flowcontrol(port_index);
    }
    else
    {
        assert(0);
    }

    if(HAL_UART_STATUS_OK != hal_uart_register_callback(port_index, mux_uart_callback, (void*)(intptr_t)port_index))
    {
        return MUX_STATUS_ERROR_INIT_FAIL;
    }

    return MUX_STATUS_OK;

}

mux_status_t port_mux_uart_deinit(uint8_t port_index)
{
    if(HAL_UART_STATUS_OK != hal_uart_deinit(port_index))
        return MUX_STATUS_ERROR_DEINIT_FAIL;
    else
        return MUX_STATUS_OK;
}

void port_mux_uart_exception_init(uint8_t port_index)
{
    hal_uart_config_t uart_config;

    hal_uart_deinit(port_index);
    uart_config.baudrate = CONFIG_SYSLOG_BAUDRATE;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(port_index, &uart_config);
#ifndef MTK_DEBUG_PLAIN_LOG_ENABLE
    hal_uart_set_software_flowcontrol(port_index, 0x11, 0x13, 0x77);
#endif
}

void port_mux_uart_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    hal_uart_send_polling(port_index, buffer, size);
}
#endif
bool port_mux_uart_buf_is_full(uint8_t port_index, bool is_rx)
{
    return uart_get_buf_full_status(port_index, is_rx);
}

uint32_t port_mux_uart_get_hw_rptr(uint8_t port_index, bool is_rx)
{
    return uart_get_hw_rptr(port_index, is_rx);
}

uint32_t port_mux_uart_get_hw_wptr(uint8_t port_index, bool is_rx)
{
    return uart_get_hw_wptr(port_index, is_rx);
}

/**When Receive data, MUX will call this API to move Rx Ring buffer Read point.
    for MUX, firstly ,should copy data out. Then call this API to move Rx Ring buffer Read point. **/
void port_mux_uart_set_rx_hw_rptr(uint8_t port_index, uint32_t move_bytes)
{
    uart_set_sw_move_byte(port_index, 1, move_bytes);
}

/**When Send data, MUX will call this API to move Tx Ring buffer Write point.
    for MUX, firstly ,should copy data in. Then call this API to move Rx Ring buffer Write point. **/
void port_mux_uart_set_tx_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    // if(port_index == 1){
    //     debug_printf("111 wrptr=%d rptr=%d status=%d move=%d \r\n",*(volatile uint32_t*)0xA0130930,*(volatile uint32_t*)0xA0130934,g_uart_on_off_status[port_index],move_bytes);
    // }
    //printf("port_index:%d, port_mux_uart_set_hw_wptr:%d #######",port_index,move_bytes);
    if (g_uart_on_off_status[port_index] == true){
        uart_set_sw_move_byte(port_index, 0, move_bytes);
        g_uart_record_wptr_legnth[port_index] = 0;
        //printf("connect status:### port_index:%d, port_mux_uart_set_hw_wptr:%d ###",port_index,move_bytes);

    } else {
        g_uart_record_wptr_legnth[port_index] += move_bytes;
        //printf("disconnect status: ### g_uart_record_wptr_legnth = %d\r\n ###", g_uart_record_wptr_legnth);
    }
    // if(port_index == 1){
    //     debug_printf("222 enduart1 wrptr=%d rptr=%d status=%d\r\n",*(volatile uint32_t*)0xA0130930,*(volatile uint32_t*)0xA0130934,g_uart_on_off_status[port_index]);
    // }
}

extern VDMA_REGISTER_T  *vdma[6];
void port_mux_uart_clear_fifo(uint8_t port_index)
{
    uint32_t W_Point,R_Point,p_length;  // TODO TEST
    vdma_channel_t tx_dma_channel, rx_dma_channel;

    tx_dma_channel = uart_internal_port_to_dma_channel(port_index, 0);
    rx_dma_channel = uart_internal_port_to_dma_channel(port_index, 1);

    if(port_index == MUX_UART_1)
    {
        W_Point=vdma[2]->VDMA_WRPTR;
        R_Point=vdma[2]->VDMA_RDPTR;
        p_length=W_Point-R_Point;
        printf("[Before haonan clean <TX>]VDMA_WRPTR = 0x%08x VDMA_RDPTR = 0x%08x legnth = %d \r\n", W_Point,R_Point,p_length);
    }

    if(port_index == MUX_UART_1)
    {
        W_Point=vdma[3]->VDMA_WRPTR;
        R_Point=vdma[3]->VDMA_RDPTR;
        p_length=W_Point-R_Point;
        printf("[Before haonan clean <RX>]VDMA_WRPTR = 0x%08x VDMA_RDPTR = 0x%08x legnth = %d\r\n", W_Point,R_Point,p_length);
    }

    if(VDMA_OK != vdma_stop(tx_dma_channel)) {
        printf("VDMA STOP  TX != OK\r\n");
       //assert(0);
    }

    if(VDMA_OK != vdma_stop(rx_dma_channel)){
        printf("VDMA STOP  RX != OK\r\n");
       //assert(0);
    }

    if(VDMA_OK != vdma_start(tx_dma_channel)) {
        printf("VDMA START TX != OK\r\n");
       //assert(0);
    }

    if(port_index == MUX_UART_1)
    {
        W_Point=vdma[2]->VDMA_WRPTR;
        R_Point=vdma[2]->VDMA_RDPTR;
        p_length=W_Point-R_Point;
        printf("[After haonan clean TX] VDMA_WRPTR = 0x%08x VDMA_RDPTR = 0x%08x legnth = %d\r\n", W_Point,R_Point,p_length);
    }

    if(VDMA_OK != vdma_start(rx_dma_channel)) {
        printf("VDMA START RX != OK\r\n");
       //assert(0);
    }

    if(port_index == MUX_UART_1)
    {
        W_Point=vdma[3]->VDMA_WRPTR;
        R_Point=vdma[3]->VDMA_RDPTR;
        p_length=W_Point-R_Point;
        printf("[After haonan clean RX] VDMA_WRPTR = 0x%08x VDMA_RDPTR = 0x%08x legnth = %d\r\n", W_Point,R_Point,p_length);
    }

    uart_internal_clear_fifo(port_index,true,true);  /*clear tx rx hardware fifo*/
}

mux_status_t port_mux_uart_control(uint8_t port_index, mux_ctrl_cmd_t command, mux_ctrl_para_t *para)
{
        uint32_t mask;
  switch (command) {
      case MUX_CMD_CONNECT: {
// TODO:: @Jerry and Haonan, as we discuss with Snow when 20200107:3335 UART need virtual connect and disconnect
        hal_nvic_save_and_set_interrupt_mask(&mask);
        g_uart_on_off_status[port_index] = true;
        if(g_uart_record_wptr_legnth[port_index] != 0)
            port_mux_uart_set_tx_hw_wptr(port_index,g_uart_record_wptr_legnth[port_index]);
        g_uart_record_wptr_legnth[port_index] = 0;
        hal_nvic_restore_interrupt_mask(mask);
        printf("port_mux_uart_control()--> CONNECT !\r\n");
        return MUX_STATUS_OK;
      }
      break;
      case MUX_CMD_DISCONNECT: {
// TODO:: @Jerry and Haonan, as we discuss with Snow when 20200107:3335 UART need virtual connect and disconnect
        //g_uart_record_hw_wptr = uart_get_hw_wptr(port_index, 0);
        g_uart_on_off_status[port_index] = false;
        printf("port_mux_uart_control()--> DISCONNECT !\r\n");
        return MUX_STATUS_OK;
      }
      break;

      case MUX_CMD_CLEAN: {

        port_mux_uart_clear_fifo(port_index);
        printf("port_mux_uart_control()--> CLEAN !\r\n");

        return MUX_STATUS_OK;
      }
      break;
      default:
        printf("MUX UART error control cmd:0x%x\r\n",command);
          break;
      }
  return MUX_STATUS_ERROR;
}

port_mux_device_ops_t g_port_mux_uart_ops = {
#ifdef MTK_CPU_NUMBER_0
    port_mux_uart_normal_init,
    port_mux_uart_deinit,
    port_mux_uart_exception_init,
    port_mux_uart_exception_send,
#endif
    port_mux_uart_buf_is_full,
    port_mux_uart_get_hw_rptr,
    port_mux_uart_set_rx_hw_rptr,
    port_mux_uart_get_hw_wptr,
    port_mux_uart_set_tx_hw_wptr,
    port_mux_uart_control,
};

