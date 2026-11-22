/* Copyright Statement:
 *
 * (C) 2005-2017 MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its
 * licensors. Without the prior written permission of MediaTek and/or its
 * licensors, any reproduction, modification, use or disclosure of MediaTek
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute
 * (as applicable) MediaTek Software if you have agreed to and been bound by
 * the applicable license agreement with MediaTek ("License Agreement") and
 * been granted explicit permission to do so within the License Agreement
 * ("Permitted User"). If you are not a Permitted User, please cease any
 * access or use of MediaTek Software immediately.
 */

/**
 * @addtogroup mt7686_hdk mt7686_hdk
 * @{
 * @addtogroup mt7686_hdk_hal_examples hal_examples
 * @{
 * @addtogroup mt7686_hdk_hal_examples_fatfs_nor_read_write_file fatfs_nor_read_write_file
 * @{

@par Overview
  - Application description
    - This application demonstrates how to use FatFS to access a file stored on nor flash.
    - The application does not require FreeRTOS.
  - Results
    - Log will show whether the file is read and written successfully.

@par Hardware and software environment
  - Supported platform
    - LinkIt 7686 HDK.
  - Environment configuration
    - The output logs are communicated through a type-A to micro-B USB cable
      to the PC from USB (CON5) connector on the HDK.
    - Install the mbed serial driver according to the instructions at
      https://developer.mbed.org/handbook/Windows-serial-configuration. For
      more information, please refer to section "Installing the LinkIt 7686
      HDK drivers on Microsoft Windows" on the "LinkIt 7686 HDK User Guide"
      in [sdk_root]/doc folder.
    - Use a type-A to micro-B USB cable to connect type-A USB of the PC and
      CON5 micro-B USB connector on the LinkIt 7686 HDK. For more information
      about the connector cable, please refer to
      https://en.wikipedia.org/wiki/USB#Mini_and_micro_connectors.
    - Launch a terminal emulator program, such as Tera terminal on your PC
      for data logging through UART. For the installation details, please
      refer to section "Installing Tera terminal on Microsoft Windows" on the
      "Airoha IoT SDK Get Started Guide" in [sdk_root]/doc folder.
    - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1, parity:
      none and flow control: off.

@par Directory contents
  - Source and header files
    - \b src/main.c:            Main program.
    - \b src/system_mt7686.c:   MT7686 clock configuration file.
    - \b inc/hal_feature_config.h:
                                MT7686 feature configuration file.
    - \b inc/memory_map.h:      MT7686 memory layout symbol file.
    - \b GCC/startup_mt7686.s:  MT7686 startup file for GCC.
    - \b GCC/syscalls.c:        MT7686 syscalls for GCC.
  - Project configuration files using GCC
    - \b GCC/feature.mk:        Feature configuration.
    - \b GCC/Makefile.:         Makefile.
    - \b GCC/mt7686_flash.ld:   Linker script.

@par Run the example
  - Build the example project with the command "./build.sh mt7686_hdk
    fatfs_nor_read_write_file" from the SDK root folder and download the binary
    file to LinkIt 7686 HDK.
  - Connect the HDK to the PC with a type-A to micro-B USB cable, and open
    the port with the name "mbed Serial Port". Connect all the switches
    described in "HDK switches and pin configuration" section and insert an
    SD card.
  - Run the application. The transaction result is displayed in the log and
    "The file is written and read successfully!" indicates a successful
    access to a file.
*/
/**
 * @}
 * @}
 * @}
 */
