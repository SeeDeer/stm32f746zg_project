# ------------------------------------------------
# Makefile (based on gcc)
#
# ChangeLog :
#   2023-03-25 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = STM32F746ZG_APP


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O0
# OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = Build/Obj

include Middlewares/RTOS/local.mk

######################################
# source
######################################
# C sources
C_SOURCES = $(wildcard App/Src/*.c)
C_SOURCES += Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_gpio.c
C_SOURCES += Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_rcc.c
C_SOURCES += Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_utils.c
C_SOURCES += Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_exti.c
C_SOURCES += Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_usart.c
C_SOURCES += $(foreach dir, $(LOCAL_SOURCES_DIRS), $(wildcard $(dir)/*.c))
C_SOURCES += $(LOCAL_C_SOURCES)

# C sources directory
C_SOURCES_DIRS =

# C includes
C_INCLUDES = App/Include
C_INCLUDES += Drivers/STM32F7xx_HAL_Driver/Inc Drivers/CMSIS/Device/ST/STM32F7xx/Include Drivers/CMSIS/Include
C_INCLUDES += $(LOCAL_INCLUDES)

# ASM sources
ASM_SOURCES = StartCode/gcc/startup_stm32f746xx.s
ASM_SOURCES += $(LOCAL_ASM_SOURCES)

# AS includes
AS_INCLUDES =

# 添加组件编译参数
include Middlewares/SEGGER_RTT/local.mk
include Middlewares/LwIP/local.mk

C_SOURCES += $(foreach dir, $(C_SOURCES_DIRS), $(wildcard $(dir)/*.c))

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS = -DHSE_VALUE=8000000 \
-DHSE_STARTUP_TIMEOUT=100 \
-DLSE_STARTUP_TIMEOUT=5000 \
-DLSE_VALUE=32768 \
-DEXTERNAL_CLOCK_VALUE=12288000 \
-DHSI_VALUE=16000000 \
-DLSI_VALUE=32000 \
-DVDD_VALUE=3300 \
-DPREFETCH_ENABLE=0 \
-DART_ACCLERATOR_ENABLE=0 \
-DSTM32F746xx

C_DEFS += -DUSE_FULL_ASSERT
# 选择LL库还是HAL库
C_DEFS += -DUSE_FULL_LL_DRIVER
# C_DEFS += -DUSE_HAL_DRIVER

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS = $(MCU) $(C_DEFS) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS += $(addprefix -I, $(C_INCLUDES))

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = Build/Linker/stm32f746zg_flash.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	cp $(BUILD_DIR)/$(TARGET).elf Build/
	mv $(BUILD_DIR)/$(TARGET).hex Build/
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	mv $(BUILD_DIR)/$(TARGET).bin Build/
	
$(BUILD_DIR):
	mkdir -p $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
# -include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***