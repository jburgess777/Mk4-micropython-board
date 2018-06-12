/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== MSP_EXP432E401Y.c ========
 *  This file is responsible for setting up the board specific items for the
 *  MSP_EXP432E401Y board.
 */
#include <stdint.h>
#include <stdlib.h>

#ifndef __MSP432E401Y__
#define __MSP432E401Y__
#endif

#include <ti/devices/msp432e4/inc/msp432.h>

#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/pwm.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/udma.h>

#include <ti/drivers/Power.h>

#include "MSP_EXP432E401Y.h"

/*
 *  =============================== ADC ===============================
 */
#include <ti/drivers/ADC.h>
#include <ti/drivers/adc/ADCMSP432E4.h>

/* ADC objects */
ADCMSP432E4_Object adcMSP432E4Objects[MSP_EXP432E401Y_ADCCOUNT];

/* ADC configuration structure */
const ADCMSP432E4_HWAttrsV1 adcMSP432E4HWAttrs[MSP_EXP432E401Y_ADCCOUNT] = {
    {
        .adcPin = ADCMSP432E4_PE_3_A0,
        .refVoltage = ADCMSP432E4_VREF_INTERNAL,
        .adcModule = ADCMSP432E4_MOD0,
        .adcSeq = ADCMSP432E4_SEQ0
    },
    {
        .adcPin = ADCMSP432E4_PE_2_A1,
        .refVoltage = ADCMSP432E4_VREF_INTERNAL,
        .adcModule = ADCMSP432E4_MOD1,
        .adcSeq = ADCMSP432E4_SEQ0
    }
};

const ADC_Config ADC_config[MSP_EXP432E401Y_ADCCOUNT] = {
    {
        .fxnTablePtr = &ADCMSP432E4_fxnTable,
        .object = &adcMSP432E4Objects[MSP_EXP432E401Y_ADC0],
        .hwAttrs = &adcMSP432E4HWAttrs[MSP_EXP432E401Y_ADC0]
    },
    {
        .fxnTablePtr = &ADCMSP432E4_fxnTable,
        .object = &adcMSP432E4Objects[MSP_EXP432E401Y_ADC1],
        .hwAttrs = &adcMSP432E4HWAttrs[MSP_EXP432E401Y_ADC1]
    }
};

const uint_least8_t ADC_count = MSP_EXP432E401Y_ADCCOUNT;

/*
 *  =============================== DMA ===============================
 */
#include <ti/drivers/dma/UDMAMSP432E4.h>

#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(dmaControlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (1024)))
#endif
static tDMAControlTable dmaControlTable[32];

/*
 *  ======== dmaErrorFxn ========
 *  This is the handler for the uDMA error interrupt.
 */
static void dmaErrorFxn(uintptr_t arg)
{
    int status = uDMAErrorStatusGet();
    uDMAErrorStatusClear();

    /* Suppress unused variable warning */
    (void)status;

    while (1);
}

UDMAMSP432E4_Object udmaMSP432E4Object;

const UDMAMSP432E4_HWAttrs udmaMSP432E4HWAttrs = {
    .controlBaseAddr = (void *)dmaControlTable,
    .dmaErrorFxn = (UDMAMSP432E4_ErrorFxn)dmaErrorFxn,
    .intNum = INT_UDMAERR,
    .intPriority = (~0)
};

const UDMAMSP432E4_Config UDMAMSP432E4_config = {
    .object = &udmaMSP432E4Object,
    .hwAttrs = &udmaMSP432E4HWAttrs
};

/*
 *  =============================== General ===============================
 */
/*
 *  ======== MSP_EXP432E401Y_initGeneral ========
 */
void MSP_EXP432E401Y_initGeneral(void)
{
    Power_init();

    /* Grant the DMA access to all FLASH memory */
    FLASH_CTRL->PP |= FLASH_PP_DFA;

    /* Region start address - match FLASH start address */
    FLASH_CTRL->DMAST = 0x00000000;

    /*
     * Access to FLASH is granted to the DMA in 2KB regions.  The value
     * assigned to DMASZ is the amount of 2KB regions to which the DMA will
     * have access.  The value can be determined via the following:
     *     2 * (num_regions + 1) KB
     *
     * To grant full access to entire 1MB of FLASH:
     *     2 * (511 + 1) KB = 1024 KB (1 MB)
     */
    FLASH_CTRL->DMASZ = 511;
}

/*
 *  =============================== EMAC ===============================
 */
#include <ti/drivers/emac/EMACMSP432E4.h>

/*
 *  Required by the Networking Stack (NDK). This array must be NULL terminated.
 *  This can be removed if NDK is not used.
 *  Double curly braces are needed to avoid GCC bug #944572
 *  https://bugs.launchpad.net/gcc-linaro/+bug/944572
 */
NIMU_DEVICE_TABLE_ENTRY NIMUDeviceTable[2] = {
    {
        /* Default: use Ethernet driver */
        .init = EMACMSP432E4_NIMUInit
    },
    {NULL}
};

/*
 *  Ethernet MAC address
 *  NOTE: By default (i.e. when each octet is 0xff), the driver reads the MAC
 *        address that's stored in flash. To override this behavior, manually
 *        set the octets of the MAC address you wish to use into the array here:
 */
unsigned char macAddress[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


/* EMAC configuration structure */
const EMACMSP432E4_HWAttrs EMACMSP432E4_hwAttrs = {
    .baseAddr = EMAC0_BASE,
    .intNum = INT_EMAC0,
    .intPriority = (~0),
    .led0Pin = EMACMSP432E4_PF0_EN0LED0,
    .led1Pin = EMACMSP432E4_PF4_EN0LED1,
    .macAddress = macAddress
};

/*
 *  =============================== GPIO ===============================
 */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOMSP432E4.h>

/*
 * Array of Pin configurations
 * NOTE: The order of the pin configurations must coincide with what was
 *       defined in MSP_EXP432E401Y.h
 * NOTE: Pins not used for interrupts should be placed at the end of the
 *       array.  Callback entries can be omitted from callbacks array to
 *       reduce memory usage.
 */
GPIO_PinConfig gpioPinConfigs[] = {
	
// Inputs
    //MSP_EXP432E401Y_GPIO_JOYC = 0,
    GPIOMSP432E4_PM6 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_FALLING,

    //MSP_EXP432E401Y_GPIO_JOYU,
    GPIOMSP432E4_PM7 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_FALLING,

    //MSP_EXP432E401Y_GPIO_JOYD,
    GPIOMSP432E4_PM4 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_FALLING,

    //MSP_EXP432E401Y_GPIO_JOYL,
    GPIOMSP432E4_PM5 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_FALLING,
    
    //MSP_EXP432E401Y_GPIO_JOYR,
    GPIOMSP432E4_PB2 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_FALLING,
    
    //MSP_EXP432E401Y_GPIO_BNT_MENU,
    GPIOMSP432E4_PK7 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,

    //MSP_EXP432E401Y_GPIO_SIM_STATUS,
    GPIOMSP432E4_PN0 | GPIO_CFG_IN_NOPULL | GPIO_CFG_IN_INT_RISING,

    //MSP_EXP432E401Y_GPIO_SIM_NETLIGHT,
    GPIOMSP432E4_PN1 | GPIO_CFG_IN_NOPULL | GPIO_CFG_IN_INT_RISING,
    
    //MSP_EXP432E401Y_GPIO_BQ_INT,
    GPIOMSP432E4_PE1 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,
    
    //MSP_EXP432E401YG_GPIO_TCA_INT,
    GPIOMSP432E4_PE0 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,

    //MSP_EXP432E401Y_GPIO_LCD_TEAR,
    GPIOMSP432E4_PK3 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,

    /* CC3120 WIFI BP */
    //MSP_EXP432E401Y_CC_HOST_IRQ,
    GPIOMSP432E4_PE4 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,

    //MSP_EXP432E401Y_GPIO_VBUS_DET,
    //// Rising for USB connected, Falling for USB disconenct
    GPIOMSP432E4_PE4 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_BOTH_EDGES,

    // MSP_EXP432E401Y_ADC_A_X,
    GPIOMSP432E4_PE2 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,

    // MSP_EXP432E401Y_ADC_A_Y,
    GPIOMSP432E4_PE3 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,

    // MSP_EXP432E401Y_ADC_CH3,
    GPIOMSP432E4_PD6 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,

    //MSP_EXP432E401Y_ADC_SPK,
    GPIOMSP432E4_PD7 | GPIO_CFG_IN_PD | GPIO_CFG_IN_INT_RISING,    

// Outputs
    //MSP_EXP432E401Y_GPIO_ETHLED0,
    GPIOMSP432E4_PK4 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    //MSP_EXP432E401Y_GPIO_ETHLED1,
    GPIOMSP432E4_PK6 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,

    //MSP_EXP432E401Y_GPIO_LED1,
    GPIOMSP432E4_PL5 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    //MSP_EXP432E401Y_GPIO_LED1,
    GPIOMSP432E4_PL4 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,

    //MSP_EXP432E401Y_GPIO_SIM_PWR_KEY,
    GPIOMSP432E4_PN2 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    // MSP_EXP432E401Y_PWM_MIC,
    GPIOMSP432E4_PF3 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,

    //MSP_EXP432E401Y_PWM_LCD_BLIGHT,
    GPIOMSP432E4_PF0 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,

    //MSP_EXP432E401Y_GPIO_LCD_DCX,
    GPIOMSP432E4_PK2 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    //MSP_EXP432E401Y_GPIO_LCD_RST,
    GPIOMSP432E4_PN4 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    //MSP_EXP432E401Y_LCD_CS,
    GPIOMSP432E4_PA3 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_HIGH,

    //MSP_EXP432E401Y_GPIO_MUX_A,
    GPIOMSP432E4_PP4 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    //MSP_EXP432E401Y_GPIO_MUX_B,
    GPIOMSP432E4_PP5 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,

    // MSP_EXP432E401Y_GPIO_T_X,
    GPIOMSP432E4_PF1 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    // MSP_EXP432E401Y_GPIO_T_Y,
    GPIOMSP432E4_PF2 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    // MSP_EXP432E401Y_GPIO_FET,
    GPIOMSP432E4_PN3 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,

    // MSP_EXP432E401Y_GPIO_CH4,
    GPIOMSP432E4_PD5 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    //MSP_EXP432E401Y_FLASH_CS,
    GPIOMSP432E4_PQ1 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_HIGH,

    /* CC3120 WIFI BP */
    //MSP_EXP432E401Y_CC_RST,
    GPIOMSP432E4_PP0 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_HIGH,
   
    //MSP_EXP432E401Y_CC_nHIB_pin,
    GPIOMSP432E4_PE5 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    
    //MSP_EXP432E401Y_CC_CS_pin,
    GPIOMSP432E4_PD2 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_HIGH
	
};

/*
 * Array of callback functioxn pointers
 * NOTE: The order of the pin configurations must coincide with what was
 *       defined in MSP_EXP432E401Y.h
 * NOTE: Pins not used for interrupts can be omitted from callbacks array to
 *       reduce memory usage (if placed at end of gpioPinConfigs array).
 */
GPIO_CallbackFxn gpioCallbackFunctions[] = {
//    NULL,  /* MSP_EXP432E401Y_USR_SW1 */
//    NULL   /* MSP_EXP432E401Y_USR_SW2 */
};

/* The device-specific GPIO_config structure */
const GPIOMSP432E4_Config GPIOMSP432E4_config = {
    .pinConfigs = (GPIO_PinConfig *)gpioPinConfigs,
    .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
    .numberOfPinConfigs = sizeof(gpioPinConfigs)/sizeof(GPIO_PinConfig),
    .numberOfCallbacks = sizeof(gpioCallbackFunctions)/sizeof(GPIO_CallbackFxn),
    .intPriority = (~0)
};

/*
 *  =============================== I2C ===============================
 */
#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CMSP432E4.h>

I2CMSP432E4_Object i2cMSP432E4Objects[MSP_EXP432E401Y_I2CCOUNT];

const I2CMSP432E4_HWAttrs i2cMSP432E4HWAttrs[MSP_EXP432E401Y_I2CCOUNT] = {
    {
        .baseAddr = I2C9_BASE,
        .intNum = INT_I2C9,
        .intPriority = (~0),
        .sclPin = I2CMSP432E4_PA0_I2C9SCL,
        .sdaPin = I2CMSP432E4_PA1_I2C9SDA
    },
    {
        .baseAddr = I2C5_BASE,
        .intNum = INT_I2C5,
        .intPriority = (~0),
        .sclPin = I2CMSP432E4_PB4_I2C5SCL,
        .sdaPin = I2CMSP432E4_PB5_I2C5SDA
    }
};

const I2C_Config I2C_config[MSP_EXP432E401Y_I2CCOUNT] = {
    {
        .fxnTablePtr = &I2CMSP432E4_fxnTable,
        .object = &i2cMSP432E4Objects[MSP_EXP432E401Y_I2C9],
        .hwAttrs = &i2cMSP432E4HWAttrs[MSP_EXP432E401Y_I2C9]
    },
    {
        .fxnTablePtr = &I2CMSP432E4_fxnTable,
        .object = &i2cMSP432E4Objects[MSP_EXP432E401Y_I2C5],
        .hwAttrs = &i2cMSP432E4HWAttrs[MSP_EXP432E401Y_I2C5]
    },
};

const uint_least8_t I2C_count = MSP_EXP432E401Y_I2CCOUNT;

/*
 *  =============================== NVS ===============================
 */
#include <ti/drivers/NVS.h>
#include <ti/drivers/nvs/NVSMSP432E4.h>

#define SECTORSIZE       (0x4000)
#define NVS_REGIONS_BASE (0xF8000)
#define REGIONSIZE       (SECTORSIZE * 2)

/*
 * Reserve flash sectors for NVS driver use
 * by placing an uninitialized byte array
 * at the desired flash address.
 */
#if defined(__TI_COMPILER_VERSION__)

/*
 * Place uninitialized array at NVS_REGIONS_BASE
 */
#pragma LOCATION(flashBuf, NVS_REGIONS_BASE);
#pragma NOINIT(flashBuf);
static char flashBuf[REGIONSIZE];

#elif defined(__IAR_SYSTEMS_ICC__)

/*
 * Place uninitialized array at NVS_REGIONS_BASE
 */
__no_init static char flashBuf[REGIONSIZE] @ NVS_REGIONS_BASE;

#elif defined(__GNUC__)

/*
 * Place the flash buffers in the .nvs section created in the gcc linker file.
 * The .nvs section enforces alignment on a sector boundary but may
 * be placed anywhere in flash memory.  If desired the .nvs section can be set
 * to a fixed address by changing the following in the gcc linker file:
 *
 * .nvs (FIXED_FLASH_ADDR) (NOLOAD) : AT (FIXED_FLASH_ADDR) {
 *      *(.nvs)
 * } > REGION_TEXT
 */
__attribute__ ((section (".nvs")))
static char flashBuf[REGIONSIZE];

#endif

NVSMSP432E4_Object nvsMSP432E4Objects[MSP_EXP432E401Y_NVSCOUNT];

const NVSMSP432E4_HWAttrs nvsMSP432E4HWAttrs[MSP_EXP432E401Y_NVSCOUNT] = {
    {
        .regionBase = (void *) flashBuf,
        .regionSize = REGIONSIZE,
    },
};

const NVS_Config NVS_config[MSP_EXP432E401Y_NVSCOUNT] = {
    {
        .fxnTablePtr = &NVSMSP432E4_fxnTable,
        .object = &nvsMSP432E4Objects[MSP_EXP432E401Y_NVSMSP432E40],
        .hwAttrs = &nvsMSP432E4HWAttrs[MSP_EXP432E401Y_NVSMSP432E40],
    },
};

const uint_least8_t NVS_count = MSP_EXP432E401Y_NVSCOUNT;

/*
 *  =============================== Power ===============================
 */
#include <ti/drivers/power/PowerMSP432E4.h>
const PowerMSP432E4_Config PowerMSP432E4_config = {
    .policyFxn = &PowerMSP432E4_sleepPolicy,
    .enablePolicy = true
};

/*
 *  =============================== PWM ===============================
 */
#include <ti/drivers/PWM.h>
#include <ti/drivers/pwm/PWMMSP432E4.h>

PWMMSP432E4_Object pwmMSP432E4Objects[MSP_EXP432E401Y_PWMCOUNT];

const PWMMSP432E4_HWAttrs pwmMSP432E4HWAttrs[MSP_EXP432E401Y_PWMCOUNT] = {
    {
        .pwmBaseAddr = PWM0_BASE,
        .pwmOutput = PWM_OUT_1,
        .pwmGenOpts = PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DBG_RUN,
        .pinConfig = PWMMSP432E4_PF1_M0PWM1
    }
};

const PWM_Config PWM_config[MSP_EXP432E401Y_PWMCOUNT] = {
    {
        .fxnTablePtr = &PWMMSP432E4_fxnTable,
        .object = &pwmMSP432E4Objects[MSP_EXP432E401Y_PWM0],
        .hwAttrs = &pwmMSP432E4HWAttrs[MSP_EXP432E401Y_PWM0]
    },
};

const uint_least8_t PWM_count = MSP_EXP432E401Y_PWMCOUNT;

/*
 *  =============================== SDFatFS ===============================
 */
#include <ti/drivers/SD.h>
#include <ti/drivers/SDFatFS.h>

/*
 * Note: The SDFatFS driver provides interface functions to enable FatFs
 * but relies on the SD driver to communicate with SD cards.  Opening a
 * SDFatFs driver instance will internally try to open a SD driver instance
 * reusing the same index number (opening SDFatFs driver at index 0 will try to
 * open SD driver at index 0).  This requires that all SDFatFs driver instances
 * have an accompanying SD driver instance defined with the same index.  It is
 * acceptable to have more SD driver instances than SDFatFs driver instances
 * but the opposite is not supported & the SDFatFs will fail to open.
 */
SDFatFS_Object sdfatfsObjects[MSP_EXP432E401Y_SDFatFSCOUNT];

const SDFatFS_Config SDFatFS_config[MSP_EXP432E401Y_SDFatFSCOUNT] = {
    {
        .object = &sdfatfsObjects[MSP_EXP432E401Y_SDFatFS0]
    }
};

const uint_least8_t SDFatFS_count = MSP_EXP432E401Y_SDFatFSCOUNT;

#if 0
/*
 *  =============================== SD ===============================
 */
#include <ti/drivers/SD.h>
#include <ti/drivers/sd/SDSPI.h>

SDSPI_Object sdspiObjects[MSP_EXP432E401Y_SDCOUNT];

const SDSPI_HWAttrs sdspiHWAttrs[MSP_EXP432E401Y_SDCOUNT] = {
    {
        .spiIndex = MSP_EXP432E401Y_SPI2,
        .spiCsGpioIndex = MSP_EXP432E401Y_SDSPI_CS
    }
};

const SD_Config SD_config[MSP_EXP432E401Y_SDCOUNT] = {
    {
        .fxnTablePtr = &SDSPI_fxnTable,
        .object = &sdspiObjects[MSP_EXP432E401Y_SDSPI0],
        .hwAttrs = &sdspiHWAttrs[MSP_EXP432E401Y_SDSPI0]
    },
};

const uint_least8_t SD_count = MSP_EXP432E401Y_SDCOUNT;
#endif

/*
 *  =============================== SPI ===============================
 */
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPIMSP432E4DMA.h>

SPIMSP432E4DMA_Object spiMSP432E4DMAObjects[MSP_EXP432E401Y_SPICOUNT];

#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(spiMSP432E4DMAscratchBuf, 32)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=32
#elif defined(__GNUC__)
__attribute__ ((aligned (32)))
#endif
uint16_t spiMSP432E4DMAscratchBuf[MSP_EXP432E401Y_SPICOUNT];

/*
 * NOTE: The SPI instances below can be used by the SD driver to communicate
 * with a SD card via SPI.  The 'defaultTxBufValue' fields below are set to 0xFF
 * to satisfy the SDSPI driver requirement.
 */
const SPIMSP432E4DMA_HWAttrs spiMSP432E4DMAHWAttrs[MSP_EXP432E401Y_SPICOUNT] = {
    {
        .baseAddr = SSI2_BASE,
        .intNum = INT_SSI2,
        .intPriority = (~0),
        .scratchBufPtr = &spiMSP432E4DMAscratchBuf[MSP_EXP432E401Y_SPI2],
        .defaultTxBufValue = 0xFF,
        .rxDmaChannel = UDMA_CH12_SSI2RX,
        .txDmaChannel = UDMA_CH13_SSI2TX,
        .minDmaTransferSize = 10,
        .clkPinMask = SPIMSP432E4_PD3_SSI2CLK,
        .fssPinMask = SPIMSP432E4_PD2_SSI2FSS,
        .xdat0PinMask = SPIMSP432E4_PD1_SSI2XDAT0,
        .xdat1PinMask = SPIMSP432E4_PD0_SSI2XDAT1
    },
    {
        .baseAddr = SSI3_BASE,
        .intNum = INT_SSI3,
        .intPriority = (~0),
        .scratchBufPtr = &spiMSP432E4DMAscratchBuf[MSP_EXP432E401Y_SPI3],
        .defaultTxBufValue = 0xFF,
        .minDmaTransferSize = 10,
        .rxDmaChannel = UDMA_CH14_SSI3RX,
        .txDmaChannel = UDMA_CH15_SSI3TX,
        .clkPinMask = SPIMSP432E4_PQ0_SSI3CLK,
        .fssPinMask = SPIMSP432E4_PQ1_SSI3FSS,
        .xdat0PinMask = SPIMSP432E4_PQ2_SSI3XDAT0,
        .xdat1PinMask = SPIMSP432E4_PQ3_SSI3XDAT1
    }
};

const SPI_Config SPI_config[MSP_EXP432E401Y_SPICOUNT] = {
    {
        .fxnTablePtr = &SPIMSP432E4DMA_fxnTable,
        .object = &spiMSP432E4DMAObjects[MSP_EXP432E401Y_SPI2],
        .hwAttrs = &spiMSP432E4DMAHWAttrs[MSP_EXP432E401Y_SPI2]
    },
    {
        .fxnTablePtr = &SPIMSP432E4DMA_fxnTable,
        .object = &spiMSP432E4DMAObjects[MSP_EXP432E401Y_SPI3],
        .hwAttrs = &spiMSP432E4DMAHWAttrs[MSP_EXP432E401Y_SPI3]
    },
};

const uint_least8_t SPI_count = MSP_EXP432E401Y_SPICOUNT;

/*
 *  =============================== UART ===============================
 */
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTMSP432E4.h>

UARTMSP432E4_Object uartMSP432E4Objects[MSP_EXP432E401Y_UARTCOUNT];
unsigned char uartMSP432E4RingBuffer[MSP_EXP432E401Y_UARTCOUNT][32];

/* UART configuration structure */
const UARTMSP432E4_HWAttrs uartMSP432E4HWAttrs[MSP_EXP432E401Y_UARTCOUNT] = {
    {
        .baseAddr = UART4_BASE,
        .intNum = INT_UART4,
        .intPriority = (~0),
        .flowControl = UARTMSP432E4_FLOWCTRL_NONE,
        .ringBufPtr  = uartMSP432E4RingBuffer[MSP_EXP432E401Y_UART4],
        .ringBufSize = sizeof(uartMSP432E4RingBuffer[MSP_EXP432E401Y_UART4]),
        .rxPin = UARTMSP432E4_PK0_U4RX,
        .txPin = UARTMSP432E4_PK1_U4TX,
        .ctsPin = UARTMSP432E4_PIN_UNASSIGNED,
        .rtsPin = UARTMSP432E4_PIN_UNASSIGNED,
        .errorFxn = NULL
    },
    {
        .baseAddr = UART1_BASE,
        .intNum = INT_UART1,
        .intPriority = (~0),
        .flowControl = UARTMSP432E4_FLOWCTRL_NONE,
        .ringBufPtr  = uartMSP432E4RingBuffer[MSP_EXP432E401Y_UART1],
        .ringBufSize = sizeof(uartMSP432E4RingBuffer[MSP_EXP432E401Y_UART1]),
        .rxPin = UARTMSP432E4_PB0_U1RX,
        .txPin = UARTMSP432E4_PB1_U1TX,
        .ctsPin = UARTMSP432E4_PIN_UNASSIGNED,
        .rtsPin = UARTMSP432E4_PIN_UNASSIGNED,
        .errorFxn = NULL
    },
    {
        .baseAddr = UART3_BASE,
        .intNum = INT_UART3,
        .intPriority = (~0),
        .flowControl = UARTMSP432E4_FLOWCTRL_NONE,
        .ringBufPtr  = uartMSP432E4RingBuffer[MSP_EXP432E401Y_UART3],
        .ringBufSize = sizeof(uartMSP432E4RingBuffer[MSP_EXP432E401Y_UART3]),
        .rxPin = UARTMSP432E4_PJ0_U3RX,
        .txPin = UARTMSP432E4_PJ1_U3TX,
        .ctsPin = UARTMSP432E4_PIN_UNASSIGNED,
        .rtsPin = UARTMSP432E4_PIN_UNASSIGNED,
        .errorFxn = NULL
    }
};

const UART_Config UART_config[MSP_EXP432E401Y_UARTCOUNT] = {
    {
        .fxnTablePtr = &UARTMSP432E4_fxnTable,
        .object = &uartMSP432E4Objects[MSP_EXP432E401Y_UART4],
        .hwAttrs = &uartMSP432E4HWAttrs[MSP_EXP432E401Y_UART4]
    },
    {
        .fxnTablePtr = &UARTMSP432E4_fxnTable,
        .object = &uartMSP432E4Objects[MSP_EXP432E401Y_UART1],
        .hwAttrs = &uartMSP432E4HWAttrs[MSP_EXP432E401Y_UART1]
    },
    {
        .fxnTablePtr = &UARTMSP432E4_fxnTable,
        .object = &uartMSP432E4Objects[MSP_EXP432E401Y_UART3],
        .hwAttrs = &uartMSP432E4HWAttrs[MSP_EXP432E401Y_UART3]
    }
};

const uint_least8_t UART_count = MSP_EXP432E401Y_UARTCOUNT;

/*
 *  =============================== Watchdog ===============================
 */
#include <ti/drivers/Watchdog.h>
#include <ti/drivers/watchdog/WatchdogMSP432E4.h>

WatchdogMSP432E4_Object watchdogMSP432E4Objects[MSP_EXP432E401Y_WATCHDOGCOUNT];

const WatchdogMSP432E4_HWAttrs watchdogMSP432E4HWAttrs[MSP_EXP432E401Y_WATCHDOGCOUNT] = {
    {
        .baseAddr = WATCHDOG0_BASE,
        .intNum = INT_WATCHDOG,
        .intPriority = (~0),
        .reloadValue = 80000000 /* 1 second period at default CPU clock freq */
    },
};

const Watchdog_Config Watchdog_config[MSP_EXP432E401Y_WATCHDOGCOUNT] = {
    {
        .fxnTablePtr = &WatchdogMSP432E4_fxnTable,
        .object = &watchdogMSP432E4Objects[MSP_EXP432E401Y_WATCHDOG0],
        .hwAttrs = &watchdogMSP432E4HWAttrs[MSP_EXP432E401Y_WATCHDOG0]
    },
};

const uint_least8_t Watchdog_count = MSP_EXP432E401Y_WATCHDOGCOUNT;

/*
 *  =============================== WiFi ===============================
 *
 * This is the configuration structure for the WiFi module that will be used
 * as part of the SimpleLink SDK WiFi plugin. These are configured for SPI mode.
 * Any changes here will need to be configured on the CC31xx device as well
 */
#include <ti/drivers/net/wifi/porting/MSP432WIFI.h>

const WIFIMSP432_HWAttrsV1 wifiMSP432HWAttrs =
{
    .spiIndex = MSP_EXP432E401Y_SPI2,
    .hostIRQPin = MSP_EXP432E401Y_CC_HOST_IRQ,
    .nHIBPin = MSP_EXP432E401Y_CC_nHIB_pin,
    .csPin = MSP_EXP432E401Y_CC_CS_pin,
    .maxDMASize = 1024,
    .spiBitRate = 1000000
};

const uint_least8_t WiFi_count = 1;

const WiFi_Config WiFi_config[1] =
{
    {
        .hwAttrs = &wifiMSP432HWAttrs,
    }
};
