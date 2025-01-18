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

CFLAGS = -Wall -g -std=gnu11 -mcpu=cortex-m3 -mno-thumb-interwork -mfix-cortex-m3-ldrd -mfloat-abi=soft -mthumb 
LDFLAGS = -T$(LINKER_SCRIPT) --static -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs
# LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group

HDRS += -I../src -I../

SRCS += ../src/startup.c 
SRCS += ../src/main.c 
SRCS += ../src/test.c 
SRCS += ../src/isr_handle.c 
SRCS += ../src/sys_init.c 

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