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

WARNINGS += -Wall   
WARNINGS += -Wextra -Waggregate-return -Wcast-align
WARNINGS += -Wcast-qual  -Wchar-subscripts  -Wcomment -Wconversion 
WARNINGS += -Wdisabled-optimization 
WARNINGS += -Werror -Wfloat-equal  -Wformat  -Wformat=2 
WARNINGS += -Wformat-nonliteral -Wformat-security  
WARNINGS += -Wformat-y2k 
WARNINGS += -Wimplicit  -Wimport  -Winit-self  -Winline 
WARNINGS += -Winvalid-pch   
WARNINGS += -Wunsafe-loop-optimizations  -Wlong-long -Wmissing-braces 
WARNINGS += -Wmissing-field-initializers -Wmissing-format-attribute   
WARNINGS += -Wmissing-include-dirs
#WARNINGS += -Wpadded  
WARNINGS += -Wpacked  -Wparentheses  -Wpointer-arith 
WARNINGS += -Wredundant-decls -Wreturn-type 
WARNINGS += -Wsequence-point  -Wshadow -Wsign-compare  -Wstack-protector 
WARNINGS += -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch  -Wswitch-default 
WARNINGS += -Wswitch-enum -Wtrigraphs  -Wuninitialized 
WARNINGS += -Wunknown-pragmas  -Wunreachable-code -Wunused 
WARNINGS += -Wunused-function  -Wunused-label  -Wunused-parameter 
WARNINGS += -Wunused-value  -Wunused-variable  -Wvariadic-macros 
WARNINGS += -Wvolatile-register-var  -Wwrite-strings

CFLAGS = $(WARNINGS) -fno-short-enums -g -std=gnu11 -mcpu=cortex-m3 -mno-thumb-interwork -mfix-cortex-m3-ldrd -mfloat-abi=soft -mthumb 
LDFLAGS = -T$(LINKER_SCRIPT) --static -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs
# LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group

HDRS += -I../src 
HDRS += -I../
HDRS += -I../src/thirdparty/free_rtos/
HDRS += -I../src/thirdparty/free_rtos/include
HDRS += -I../src/thirdparty/free_rtos/portable/GCC/ARM_CM3

SRCS += ../src/startup.c 
SRCS += ../src/main.c 
SRCS += ../src/test.c 
SRCS += ../src/isr_handle.c 
SRCS += ../src/sys_init.c 
SRCS += ../src/i2c_stm.c 
SRCS += ../src/thirdparty/free_rtos/portable/GCC/ARM_CM3/port.c 
SRCS += ../src/thirdparty/free_rtos/portable/MemMang/heap_4.c 
SRCS += ../src/thirdparty/free_rtos/list.c 
SRCS += ../src/thirdparty/free_rtos/queue.c 
SRCS += ../src/thirdparty/free_rtos/tasks.c 
SRCS += ../src/thirdparty/free_rtos/timers.c

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
	$(CC) $(CFLAGS) $(HDRS) -c $< -o $(BUILD_DIR)/$(notdir $@) $(PREPROCESSOR_DEFINES)

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $@) $(PREPROCESSOR_DEFINES)

%.size: $(TARGET_PATH).elf
	$(SIZE) -x $< 

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.lst

.PHONY: all clean