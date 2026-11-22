/*
 * Copyright Everest Semiconductor Co.,Ltd *
 *
 * This program is free software; you can redistribute it and/or modify
 *
 */




#include "hal_gpt.h"
#include "hal_platform.h"
#include "hal_i2c_master.h"
#include "hal_log.h"
#include "hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "es8388.h"
#include <stdio.h>
#include <string.h>
#define VOLUME_MAX      (10)

static hal_i2c_port_t es8388_i2c_port;
static uint8_t es8388_init = 0;
static uint8_t volume_value =7;

AUCODEC_STATUS_e aucodec_i2c_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency);
AUCODEC_STATUS_e aucodec_i2c_deinit();

uint8_t aucodec_volume_reg(uint8_t volume)
{
    uint8_t vol_reg;

    if(volume == 10)
        vol_reg = 30;
    else if(volume == 9)
        vol_reg = 27;
    else if(volume == 8)
//  vol_reg = 24;
    vol_reg = 23;

    else if(volume == 7)
        vol_reg = 21;
    else if(volume == 6)
        vol_reg = 18;
    else if(volume == 5)
        vol_reg = 15;
    else if(volume == 4)
        vol_reg = 12;
    else if(volume == 3)
        vol_reg = 9;
    else if(volume == 2)
        vol_reg = 6;
    else if(volume == 1)
        vol_reg = 3;
    else if(volume == 0)
        vol_reg = 0;

    return vol_reg;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_write
 *
 * @Routine Description:
 *      Audio codec write register
 *
 * @Parameter:
 *      u16Addr:    Register address
 *      u16Data:    Register data
 *
 * @Return:
 *      1:    Success
 *      -1:    Failure
 *
 *****************************************************************************************************************/
int8_t aucodec_i2c_write(
    uint8_t u8Addr,
    uint8_t u8Data)
{
    uint8_t u8TxDataBuf[2];

    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;

    u8TxDataBuf[0] = u8Addr;
    u8TxDataBuf[1] = u8Data;

    i2c_status = hal_i2c_master_send_polling(es8388_i2c_port, ES8388_I2C_ADDR, u8TxDataBuf, 2);
    if (i2c_status != HAL_I2C_STATUS_OK) {
        log_hal_error("[es8323]i2c write fail: RegAddress=%02x  Data=%u\n\r", u8TxDataBuf[0], u8TxDataBuf[1]);
        return -1;
    }

    return 0;
}


/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_read
 *
 * @Routine Description:
 *      Audio Codec read register
 *
 * @Parameter:
 *      (in) u16Addr:    Register address
 *      (out) u16Data:  Data
 *
 * @Return:
 *      1:    Success
 *      -1:    Failure
 *
 *****************************************************************************************************************/
uint8_t aucodec_i2c_read(uint8_t u8Addr)
{
    uint8_t u8TxDataBuf;

    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;


    i2c_status = hal_i2c_master_send_polling(es8388_i2c_port, ES8388_I2C_ADDR, &u8Addr, 1);
    if (i2c_status != HAL_I2C_STATUS_OK) {
        log_hal_error("[es8323][1]i2c read fail1: RegAddress=%02x\n\r", u8Addr);
        return -1;
    }

    i2c_status = hal_i2c_master_receive_polling(es8388_i2c_port, ES8388_I2C_ADDR, &u8TxDataBuf, 1);
    if (i2c_status != HAL_I2C_STATUS_OK) {
        log_hal_error("[es8323][2]i2c read fail2: RegAddress=%02x\n\r", u8Addr);
        return -1;
    }

    return u8TxDataBuf;
}


/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_output
 *
 * @Routine Description:
 *      Audio codec set analog output path for DACIN
 *
 * @Parameter:
 *      AUCODEC_OUTPUT_SEL_e:    eAuxOut
 *                               eHpOut
 *                               eSpkOut
 *                               eNoneOut
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_output(
    AUCODEC_OUTPUT_SEL_e eOutputMode)
{
    int8_t ret = 0;
    uint16_t u16Data;

    if ((eOutputMode == eLineOut) || (eOutputMode == eSpkOut)) {


    } else {

    }

    return AUCODEC_STATUS_OK;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_input
 *
 * @Routine Description:
 *      Audio codec set analog input path for ADCOUT
 *
 * @Parameter:
 *      AUCODEC_INPUT_SEL_e:    eAuxIn
 *                              eMicIn
 *                              eNoneIn
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_input(
    AUCODEC_INPUT_SEL_e eInputMode)
{
    int8_t ret = 0;
    uint16_t u16Data;

    if (eInputMode == eMicIn) {


    } else {

    }

    return AUCODEC_STATUS_OK;

}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_mute
 *
 * @Routine Description:
 *      Audio codec software reset
 *
 * @Parameter:
 *      AUCODEC_MUTE_e:    eMute
 *                         eUnmute
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_mute(
    AUCODEC_MUTE_e eMuteEnable)
{
    int8_t ret = 0;
    uint16_t u16Data;

    return AUCODEC_STATUS_OK;
}


/*****************************************************************************************************************
 * @Function:
 *      aucodec_set_dai_fmt
 *
 * @Routine Description:
 *      Audio codec digital audio interface format configuration
 *
 * @Parameter:
 *      AUCODEC_DAI_FORMAT_e:    eRightJustified
 *                               eLeftJustified
 *                               eI2S
 *                               ePCMA
 *                               ePCMB
 *      AUCODEC_DAI_WLEN_e:    e16Bit
 *                             e20Bit
 *                             e24Bit
 *                             e32Bit
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_dai_fmt(
    AUCODEC_DAI_FORMAT_e eFormat,
    AUCODEC_DAI_WLEN_e eWLEN,
    AUCODEC_BCLK_INV_e eBCLKP)
{
    int8_t ret = 0;
    uint16_t u16Data;

    // DAI format configuration
    switch (eFormat) {
        case eRightJustified:
            break;
        case eLeftJustified:
            break;
        case eI2S:
            break;
        case ePCMA:
            break;
        default: // case ePCMB:
            break;
    }

    // Data resolution of data stream
    switch (eWLEN) {
        case e16Bit:
            break;
        case e20Bit:
            break;
        case e24Bit:
            break;
        default:  // case e32Bit:
            break;
    }

    switch (eBCLKP) {
        case eBCLK_INV:
            break;
        default: //BCLK Polarity Normal
            break;
    }

    //ret = aucodec_i2c_write(AUCODEC_R04H_AU_INTERFACE_CTRL_REG, u16Data);
    if (ret == -1) {
        return AUCODEC_STATUS_ERROR;
    }

    return AUCODEC_STATUS_OK;
}


/*****************************************************************************************************************
* @Function:
*      aucodec_set_dai_sysclk
*
* @Routine Description:
*      Audio codec system clock configuration
*
* @Parameter:
*      AUCODEC_SAMPLERATE_SEL_e:    eSR48KHz
*                                   eSR44K1Hz
*                                   eSR32KHz
*                                   eSR16KHz
*                                   eSR8KHz
*      AUCODEC_DAI_ROLE_e:     eSLAVE
*                              eMASTER
*      AUCODEC_BCLK_RATE_e:     e256xFS
*                               e128xFS
*                               e64xFS
*                               e32xFS
*                               e16xFS
*                               e8xFS
*      u32MclkRate:    The frequency of mclk input (Hz)
*      AUCODEC_PLL_e:    ePLLDisable
*                        ePLLEnable
*
* @Return:
*      AUCODEC_STATUS_OK:    Success
*      AUCODEC_STATUS_ERROR:    Failure
*****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_set_dai_sysclk(
    AUCODEC_SAMPLERATE_SEL_e eSampleRate,
    AUCODEC_DAI_ROLE_e eDAIRole,
    AUCODEC_BCLK_RATE_e eBCLKRate,
    uint32_t u32MclkRate,
    AUCODEC_PLL_e ePLLCtrl)
{
    log_hal_info("[es8323]eSampleRate=%d\n",eSampleRate);
    return AUCODEC_STATUS_OK;
}

void aucodec_set_sampling_rate(int bitrate)
{

}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_softreset
 *
 * @Routine Description:
 *      Audio codec software reset
 *
 * @Parameter:
 *
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_softreset(void)
{
    //aucodec_i2c_write(ES8323_CONTROL1, 0x80);
    //return aucodec_i2c_write(ES8323_CONTROL1, 0x00);
    return 0;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_init
 *
 * @Routine Description:
 *      Audio codec initial
 *
 * @Parameter:
 *
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_init(void)
{
    int8_t i,ret = 0;
    uint16_t u16Data;
    uint8_t vol_reg;

    if(es8388_init > 0)
        return 0;
    aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec
    vol_reg = aucodec_volume_reg(/* volume_value */ 8);

    es8388_init = 1;
    //for reset
/*  aucodec_i2c_write(0x35,0xA0);
    aucodec_i2c_write(0x37,0x10);
    aucodec_i2c_write(0x38,0x02);//12db dif,gain 12db
    aucodec_i2c_write(0x39,0x10);
*/
//aucodec_i2c_write(0x01,0x58);
    aucodec_i2c_write(0x01,0x50);
    aucodec_i2c_write(0x08,0x40);
    aucodec_i2c_write(0x2B,0x80);
    aucodec_i2c_write(0x00,0x16);
    vTaskDelay(50 / portTICK_RATE_MS);
    aucodec_i2c_write(0x03,0x00);
    aucodec_i2c_write(0x07,0x7c);
    aucodec_i2c_write(0x19,0x02);
//aucodec_i2c_write(0x09,0x66);//调MIC的增益,PGA gain = 6 * 3db = 18db
    //0x16: 人声音变大
//aucodec_i2c_write(0x09,0x18);//调MIC的增益,PGA gain = 6 * 3db = 18db,high 4bits:Step back
    aucodec_i2c_write(0x09,0x08);//调MIC的增益,PGA gain = 6 * 3db = 18db


    //LIN1/RIN1
//aucodec_i2c_write(0x0a,0x00);
//aucodec_i2c_write(0x0b,0x02);
    //LIN2/RIN2
    aucodec_i2c_write(0x0a,0xf8);
    aucodec_i2c_write(0x0b,0x80);
// low 16bits
    aucodec_i2c_write(0x0C,0x0C);
    aucodec_i2c_write(0x0D,0x02);
    aucodec_i2c_write(0x10,0x00);
    aucodec_i2c_write(0x11,0x00);
    aucodec_i2c_write(0x12,0x22);//ALC
    aucodec_i2c_write(0x13,0xa0);
    aucodec_i2c_write(0x14,0x05);
    aucodec_i2c_write(0x15,0x06);
    aucodec_i2c_write(0x16,0x91);
    aucodec_i2c_write(0x17,0x18);
    aucodec_i2c_write(0x18,0x02);
    aucodec_i2c_write(0x1A,0x00);
    aucodec_i2c_write(0x1B,0x00);
    aucodec_i2c_write(0x27,0xB8);
    aucodec_i2c_write(0x2A,0xB8);
    aucodec_i2c_write(0x02,0x00);
    vTaskDelay(50 / portTICK_RATE_MS);
    aucodec_i2c_write(0x04,0x3c);
    aucodec_i2c_write(0x2e,vol_reg);
    aucodec_i2c_write(0x2f,vol_reg);
    aucodec_i2c_write(0x30,vol_reg);
    aucodec_i2c_write(0x31,vol_reg);
    //aucodec_i2c_debug();
    aucodec_i2c_deinit();
    log_hal_info("[es8388]aucodec_init ok, vol_reg = %d\n", vol_reg);
    return AUCODEC_STATUS_OK;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_init
 *
 * @Routine Description:
 *      i2c initial
 *
 * @Parameter:
 *                  hal_i2c_port_t i2c_port
 *                              HAL_I2C_MASTER_0 = 0
 *                              HAL_I2C_MASTER_1 = 1
 *                              HAL_I2C_MASTER_2 = 2
 *
 *                  hal_i2c_frequency_t frequency
 *                              HAL_I2C_FREQUENCY_50K  = 0,
 *                              HAL_I2C_FREQUENCY_100K = 1,
 *                              HAL_I2C_FREQUENCY_200K = 2,
 *                              HAL_I2C_FREQUENCY_400K = 3,
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/
AUCODEC_STATUS_e aucodec_i2c_init(hal_i2c_port_t i2c_port, hal_i2c_frequency_t frequency)
{
    /*I2C*/
    hal_i2c_config_t i2c_config;
    hal_i2c_status_t i2c_status;

    i2c_status = HAL_I2C_STATUS_OK;
    i2c_config.frequency = (hal_i2c_frequency_t)frequency;
    es8388_i2c_port = (hal_i2c_port_t)i2c_port;
    log_hal_info("zhanglong hal_i2c_master_init\n");
    i2c_status = hal_i2c_master_init(es8388_i2c_port, &i2c_config);
    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[es8388]i2c init ok\n");
    } else {

        log_hal_error("[es8388]i2c init error\n");
        return AUCODEC_STATUS_ERROR;
    }

    return AUCODEC_STATUS_OK;
}

/*****************************************************************************************************************
 * @Function:
 *      aucodec_i2c_deinit
 *
 * @Routine Description:
 *      i2c deinitial
 *
 * @Parameter:
 *
 * @Return:
 *      AUCODEC_STATUS_OK:    Success
 *      AUCODEC_STATUS_ERROR:    Failure
 *****************************************************************************************************************/

AUCODEC_STATUS_e aucodec_i2c_deinit()
{
    hal_i2c_status_t i2c_status;
    i2c_status = HAL_I2C_STATUS_OK;

    i2c_status = hal_i2c_master_deinit(es8388_i2c_port);

    if (i2c_status == HAL_I2C_STATUS_OK) {
        log_hal_info("[es8323]i2c deinit ok\n");
    } else {
        log_hal_error("[es8323]i2c deinit error\n");
        return AUCODEC_STATUS_ERROR;
    }

    return AUCODEC_STATUS_OK;
}

void aucodec_i2c_debug(void)
{
    log_hal_info("\n\n");
    for(uint8_t i=0; i<=0x32; i++){
        log_hal_info("%d=0x%x,\n", i,aucodec_i2c_read(i));
    }
    log_hal_info("\n\n");
}

void aucodec_pa_close(int pinnum)
{
    log_hal_info("pin=%d\n\n",pinnum);
    hal_gpio_init(pinnum);
//hal_pinmux_set_function(HAL_GPIO_10, HAL_GPIO_10_GPIO10);
    hal_gpio_set_direction(pinnum, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(pinnum, HAL_GPIO_DATA_HIGH);
    hal_gpio_deinit(pinnum);
/*
        hal_gpio_init(HAL_GPIO_10);
    //hal_pinmux_set_function(HAL_GPIO_10, HAL_GPIO_10_GPIO10);
        hal_gpio_set_direction(HAL_GPIO_10, HAL_GPIO_DIRECTION_OUTPUT);
        hal_gpio_set_output(HAL_GPIO_10, HAL_GPIO_DATA_LOW);
        hal_gpio_deinit(HAL_GPIO_10);
*/
}

void aucodec_pa_open(int pinnum)
{
    log_hal_info("pin=%d\n\n",pinnum);
    hal_gpio_init(pinnum);
//hal_pinmux_set_function(HAL_GPIO_10, HAL_GPIO_10_GPIO10);
    hal_gpio_set_direction(pinnum, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(pinnum,HAL_GPIO_DATA_LOW);
    hal_gpio_deinit(pinnum);
/*
    hal_gpio_init(HAL_GPIO_10);
//hal_pinmux_set_function(HAL_GPIO_10, HAL_GPIO_10_GPIO10);
    hal_gpio_set_direction(HAL_GPIO_10, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(HAL_GPIO_10,HAL_GPIO_DATA_HIGH );
    hal_gpio_deinit(HAL_GPIO_10);
    */
}
void aucodec_set_volume(uint8_t volume)
{
    uint8_t vol_reg;
    vol_reg = aucodec_volume_reg(volume);
    aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec
    log_hal_info("aucodec_set_volume vol_reg=%d,\n", vol_reg);
    aucodec_i2c_write(0x2e,vol_reg);//1.5db: 0x1e = 0db, 00: = -45db
    aucodec_i2c_write(0x2f,vol_reg);
    aucodec_i2c_write(0x30,vol_reg);//1.5db: 0x1e = 0db, 00: = -45db
    aucodec_i2c_write(0x31,vol_reg);
    aucodec_i2c_deinit();
}
uint8_t aucodec_get_volume()
{
    return volume_value;
}

void aucodec_set_mic_gain(uint8_t gain){
    aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec
    aucodec_i2c_write(0x09,gain);
    aucodec_i2c_deinit();
}

void aucodec_volume_up(void)
{
    if(volume_value >= VOLUME_MAX)
        return;

    volume_value++;
    aucodec_set_volume(volume_value);
}

void aucodec_volume_down(void)
{
    if(volume_value <= 0)
        return;

    volume_value--;
    aucodec_set_volume(volume_value);
}

void aucodec_volume_init(void)
{
    nvram_volume_get(&volume_value);
}

void aucodec_volume_save(void)
{
    nvram_volume_set(volume_value);
}

void aucodec_deinit(void)
{

    aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec
    #if 0
    aucodec_i2c_write(0x19, 0x06);
    aucodec_i2c_write(0x04, 0x00);
    aucodec_i2c_write(0x04, 0xC0);
    aucodec_i2c_write(0x03, 0xFF);
    aucodec_i2c_write(0x02, 0xF3);
    #endif
    aucodec_i2c_deinit();

}

void aucodec_es8388_resume(void)
{

    aucodec_i2c_init(HAL_I2C_MASTER_1, HAL_I2C_FREQUENCY_200K); //init codec
    #if 0
    aucodec_i2c_write(0x02, 0x00);
    aucodec_i2c_write(0x03, 0x00);
    vTaskDelay(100 / portTICK_RATE_MS);
    aucodec_i2c_write(0x04, 0x00);
    aucodec_i2c_write(0x04, 0x3C);
    aucodec_i2c_write(0x19, 0x02);
    #endif
    aucodec_i2c_deinit();
}
void aucodec_set_linX(uint8_t mic_state){

}

