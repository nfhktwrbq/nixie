TARGET = nixie
BUILD_DIR = ./
STM32_STD_PERIPH_LIB = path_to_std_periph_lib
SIDE_TARGETS = $(TARGET).size
TARGET_PATH = $(BUILD_DIR)/$(TARGET)

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

PREPROCESSOR_DEFINES += -DBME280_32BIT_ENABLE
PREPROCESSOR_DEFINES += -DPRINTF_INCLUDE_CONFIG_H

WARNINGS += -Wall   
WARNINGS += -Wextra -Waggregate-return -Wcast-align
WARNINGS += -Wcast-qual  -Wchar-subscripts  -Wcomment 
WARNINGS += -Wdisabled-optimization 
WARNINGS += -Werror -Wformat  -Wformat=2 
WARNINGS += -Wformat-nonliteral -Wformat-security  
WARNINGS += -Wformat-y2k 
WARNINGS += -Wimplicit  -Wimport  -Winit-self  -Winline 
WARNINGS += -Winvalid-pch   
WARNINGS += -Wunsafe-loop-optimizations  -Wmissing-braces 
#WARNINGS += -Wlong-long -Wfloat-equal -Wconversion #printf 
WARNINGS += -Wmissing-field-initializers -Wmissing-format-attribute   
WARNINGS += -Wmissing-include-dirs
#WARNINGS += -Wpadded  
WARNINGS += -Wlogical-op -Wundef -Wsizeof-pointer-div -Wint-conversion
WARNINGS += -Wincompatible-pointer-types -Wimplicit-function-declaration
WARNINGS += -Warray-bounds 
#WARNINGS += -Wstrict-prototypes
WARNINGS += -Wpacked  -Wparentheses  -Wpointer-arith 
WARNINGS += -Wredundant-decls -Wreturn-type 
WARNINGS += -Wsequence-point  -Wshadow -Wsign-compare  -Wstack-protector 
#WARNINGS += -Wstrict-aliasing -Wstrict-aliasing=2  #not work with FreeRtos Os optimizations
WARNINGS += -Wswitch  -Wswitch-default 
WARNINGS += -Wswitch-enum -Wtrigraphs  -Wuninitialized 
WARNINGS += -Wunknown-pragmas  -Wunreachable-code -Wunused 
WARNINGS += -Wunused-function  -Wunused-label  -Wunused-parameter 
WARNINGS += -Wunused-value  -Wunused-variable  -Wvariadic-macros 
WARNINGS += -Wvolatile-register-var  -Wwrite-strings

OPTIMIZATION ?= -O3

CFLAGS += -g3 -std=gnu11 -mcpu=cortex-m3 -mno-thumb-interwork -mfix-cortex-m3-ldrd -mfloat-abi=soft -mthumb 
CFLAGS += $(OPTIMIZATION) $(WARNINGS)
LDFLAGS = -T$(LINKER_SCRIPT) --static -Wl,--gc-sections -nostartfiles --specs=nano.specs --specs=nosys.specs
# LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group

HDRS += -I../
HDRS += -I../src
HDRS += -I../src/app 
HDRS += -I../src/configs
HDRS += -I../src/drivers
HDRS += -I../src/hardware
HDRS += -I../src/utils
HDRS += -I../src/thirdparty/
HDRS += -I../src/thirdparty/free_rtos/
HDRS += -I../src/thirdparty/free_rtos/include
HDRS += -I../src/thirdparty/free_rtos/portable/GCC/ARM_CM3

SRCS += ../src/main.c 
SRCS += ../src/drivers/bkp.c 
SRCS += ../src/drivers/i2c.c 
SRCS += ../src/drivers/rtc.c 
SRCS += ../src/drivers/uart.c 
SRCS += ../src/hardware/isr_handle.c 
SRCS += ../src/hardware/startup.c 
SRCS += ../src/hardware/sys_init.c 
SRCS += ../src/thirdparty/bme280/bme280.c 
SRCS += ../src/thirdparty/free_rtos/portable/GCC/ARM_CM3/port.c 
SRCS += ../src/thirdparty/free_rtos/portable/MemMang/heap_4.c 
SRCS += ../src/thirdparty/free_rtos/list.c 
SRCS += ../src/thirdparty/free_rtos/queue.c 
SRCS += ../src/thirdparty/free_rtos/tasks.c 
SRCS += ../src/thirdparty/free_rtos/timers.c
SRCS += ../src/thirdparty/printf/printf.c 

# ASMS = ../src/startup.s

OBJS := $(patsubst %.c,%.o,$(SRCS))
ASMOBJS := $(patsubst %.s,%.o,$(ASMS))

CURRENT_DIR = $(shell pwd)

$(info CWD is $(CURRENT_DIR))
$(info OBJS is $(OBJS))
$(info PATH is $(PATH))


all: $(TARGET_PATH).bin $(TARGET_PATH).lst $(SIDE_TARGETS)

$(TARGET_PATH).bin: $(TARGET_PATH).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET_PATH).lst: $(TARGET_PATH).elf
	$(OBJDUMP) -D $< > $@

$(TARGET_PATH).elf: $(OBJS) $(ASMOBJS)
	$(CC) $(CFLAGS) $(foreach obj, $^,$(BUILD_DIR)/$(notdir $(obj))) -o $@  $(LDFLAGS)

%.o: %.c
	@echo [CC] $@
	@$(CC) $(CFLAGS) $(HDRS) -c $< -o $(BUILD_DIR)/$(notdir $@) $(PREPROCESSOR_DEFINES)

%.o: %.s
	@echo [CS] $@
	@$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $@) $(PREPROCESSOR_DEFINES)

%.size: $(TARGET_PATH).elf
	$(SIZE) -x $< 

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.lst

.PHONY: all clean