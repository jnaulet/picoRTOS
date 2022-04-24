# S32K116 test board
ELF := arduino-mega2560.elf
SKETCH := arduino-mega2560

CC := gcc
CROSS := avr-
ARCH := arch/avr/avr6

# directories
PICORTOS_DIR := $(abspath ../..)
BUILD_DIR := build
SKETCH_DIR := sketch/$(SKETCH)

DEFINE := -D__AVR_ATmega2560__
INCLUDE := -I. -I$(PICORTOS_DIR) -I$(PICORTOS_DIR)/$(ARCH)

CFLAGS += -Wall -Wextra -std=c99 $(INCLUDE) -O1 -g3 $(DEFINE) -mmcu=avr6
SFLAGS += $(DEFINE) $(INCLUDE) -g3 -mmcu=avr6
SPLINTFLAGS += -checks

# picoRTOS
SRC_C_FILES += $(PICORTOS_DIR)/picoRTOS.c
SRC_C_FILES += $(PICORTOS_DIR)/$(ARCH)/picoRTOS_port.c
SRC_S_FILES += $(PICORTOS_DIR)/$(ARCH)/picoRTOS_portasm.S

SRC_H_FILES += $(PICORTOS_DIR)/picoRTOS.h
SRC_H_FILES += $(PICORTOS_DIR)/$(ARCH)/picoRTOS_types.h
SRC_H_FILES += picoRTOSConfig.h

# demo
SRC_C_FILES += blink.c

OBJ_FILES := $(SRC_C_FILES:%.c=%.o)
OBJ_FILES += $(SRC_S_FILES:%.S=%.o)

# fix paths
OBJ_FILES := $(addprefix $(BUILD_DIR)/,$(OBJ_FILES))

all:
	mkdir -p $(SKETCH_DIR)
	cp $(SKETCH).ino $(SRC_C_FILES) $(SRC_S_FILES) $(SRC_H_FILES) $(SKETCH_DIR)
	arduino-cli compile -b arduino:avr:mega -p /dev/ttyACM0 \
	  -l serial $(SKETCH_DIR)

upload:
	arduino-cli upload -b arduino:avr:mega -p /dev/ttyACM0 \
	  -l serial $(SKETCH_DIR)

debug:
	arduino-cli debug -b arduino:avr:mega -p /dev/ttyACM0 \
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
