CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LD = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
AR = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-ar"

INCLUDES = -I$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/source/ti/posix/gcc \
    -I$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/source \
    "-I$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    "-I$(XDC_INSTALL_DIR)/packages" \
    "-I$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

CFLAGS += -DDeviceFamily_CC13X2 \
    $(INCLUDES) \
    -mcpu=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -ffunction-sections -fdata-sections -g -gstrict-dwarf \
    -Wall -std=c99

LFLAGS = -Wl,-T,CC1352R1_LAUNCHXL_TIRTOS.lds \
    -Wl,-Map,$@.map -Wl,-T,$(KERNEL_BUILD)/linker.cmd -nostartfiles \
    -L$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/source \
    -l:ti/display/lib/display.am4fg \
    -l:ti/drivers/lib/drivers_cc13x2_v2.am4fg \
    -L$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib \
    -l:dpl_cc13x2_v2.am4fg \
    -L$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/source/ti/devices/cc13x2_cc26x2_v2/driverlib/bin/gcc \
    -l:driverlib.lib \
    -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -static -Wl,--gc-sections "-L$(SIMPLELINK_CC13X2_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard" -lgcc -lc -lm -lnosys --specs=nano.specs