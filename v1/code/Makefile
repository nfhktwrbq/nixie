TARGET = nixie
BUILD_DIR = ./build
STM32_STD_PERIPH_LIB = path_to_std_periph_lib

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

CFLAGS = -Wall -g -std=gnu11 -mcpu=cortex-m3 -mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float -mfix-cortex-m3-ldrd
LDFLAGS = -TSTM32F103C8TX_FLASH.ld

HDRS += ./src

SRCS += ./src/startup.c 
SRCS += ./src/main.c 
SRCS += ./src/test.c 
SRCS += ./src/isr_handle.c 
SRCS += ./src/sys_init.c 


# ASMS = ./src/startup.s

OBJS := $(patsubst %.c,%.o,$(SRCS))
ASMOBJS := $(patsubst %.s,%.o,$(ASMS))

$(info OBJS is $(OBJS))

all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).lst

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).lst: $(BUILD_DIR)/$(TARGET).elf
	$(OBJDUMP) -D $< > $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(ASMOBJS)
	$(LD) $(LDFLAGS) -o $@ $(foreach obj, $^,$(BUILD_DIR)/$(notdir $(obj)))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $@)

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $@)

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.lst

.PHONY: all clean