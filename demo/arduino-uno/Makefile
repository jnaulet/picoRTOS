# S32K116 test board
ELF := arduino-uno.elf
SKETCH := arduino-uno

CC := gcc
CROSS := avr-
ARCH := arch/avr/avr5

# directories
PICORTOS_DIR := $(abspath ../..)
BUILD_DIR := build
SKETCH_DIR := sketch/$(SKETCH)

DEFINE := -D__AVR_ATmega328P__
INCLUDE := -I. -I$(PICORTOS_DIR) -I$(PICORTOS_DIR)/$(ARCH)
INCLUDE += -I$(PICORTOS_DIR)/drivers/hal
INCLUDE += -I$(PICORTOS_DIR)/drivers/clock
INCLUDE += -I$(PICORTOS_DIR)/drivers/gpio
INCLUDE += -I$(PICORTOS_DIR)/drivers/uart

CFLAGS += -Wall -Wextra -std=c99 $(INCLUDE) -O1 -g3 $(DEFINE) -mmcu=avr5
SFLAGS += $(DEFINE) $(INCLUDE) -g3 -mmcu=avr5
SPLINTFLAGS += -checks

# picoRTOS
SRC_C_FILES += $(PICORTOS_DIR)/picoRTOS.c
SRC_C_FILES += $(PICORTOS_DIR)/$(ARCH)/picoRTOS_port.c
SRC_S_FILES += $(PICORTOS_DIR)/$(ARCH)/picoRTOS_portasm.S

SRC_H_FILES += $(PICORTOS_DIR)/picoRTOS.h
SRC_H_FILES += $(PICORTOS_DIR)/arch/include/picoRTOS_port.h
SRC_H_FILES += $(PICORTOS_DIR)/$(ARCH)/picoRTOS_types.h
SRC_H_FILES += picoRTOSConfig.h

# drivers
SRC_C_FILES += $(PICORTOS_DIR)/drivers/clock/clock-atmega328p.c
SRC_C_FILES += $(PICORTOS_DIR)/drivers/gpio/gpio-avr.c
SRC_C_FILES += $(PICORTOS_DIR)/drivers/uart/uartcore.c
SRC_C_FILES += $(PICORTOS_DIR)/drivers/uart/uart-avr.c
SRC_H_FILES += $(PICORTOS_DIR)/drivers/hal/clock.h
SRC_H_FILES += $(PICORTOS_DIR)/drivers/clock/clock-atmega328p.h
SRC_H_FILES += $(PICORTOS_DIR)/drivers/hal/gpio.h
SRC_H_FILES += $(PICORTOS_DIR)/drivers/gpio/gpio-avr.h
SRC_H_FILES += $(PICORTOS_DIR)/drivers/hal/uart.h
SRC_H_FILES += $(PICORTOS_DIR)/drivers/uart/uart-avr.h

# demo
SRC_C_FILES += blink.c

OBJ_FILES := $(SRC_C_FILES:%.c=%.o)
OBJ_FILES += $(SRC_S_FILES:%.S=%.o)

# fix paths
OBJ_FILES := $(addprefix $(BUILD_DIR)/,$(OBJ_FILES))

all:
	mkdir -p $(SKETCH_DIR)
	cp $(SKETCH).ino $(SRC_C_FILES) $(SRC_S_FILES) $(SRC_H_FILES) $(SKETCH_DIR)
	arduino-cli compile -b arduino:avr:uno -p /dev/ttyACM0 \
	  -l serial $(SKETCH_DIR)

upload:
	arduino-cli upload -b arduino:avr:uno -p /dev/ttyACM0 \
	  -l serial $(SKETCH_DIR)

build: $(ELF)

asm: CFLAGS += -S
asm: build

$(ELF): $(OBJ_FILES)
	-mkdir -p $(@D)
	$(CROSS)$(CC) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.c
	-mkdir -p $(@D)
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	-mkdir -p $(@D)
	$(CROSS)$(CC) $(SFLAGS) -c $< -o $@

splint:
	splint $(SPLINTFLAGS) $(DEFINE) $(INCLUDE) $(SRC_C_FILES)

clean:
	-rm -rf $(BUILD_DIR)
	-rm -rf sketch
	-rm -f $(ELF)

.PHONY: clean splint uncrustify
