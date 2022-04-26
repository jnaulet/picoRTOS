# S32K116 test board
ELF := arduino-due.elf
SKETCH := arduino-due

CC := gcc
CROSS := arm-none-eabi-

# directories
PICORTOS_DIR := $(abspath ../..)
BUILD_DIR := build
SKETCH_DIR := sketch/$(SKETCH)

DEFINE := -DATSAM3X8E
INCLUDE := -I. -I$(PICORTOS_DIR) -I$(PICORTOS_DIR)/arch/arm/cm3

CFLAGS += -Wall -Wextra -std=c99 $(INCLUDE) -O1 -g3 $(DEFINE) -mcpu=cortex-m3
SFLAGS += $(DEFINE) $(INCLUDE) -g3 -mcpu=cortex-m3
SPLINTFLAGS += -checks

# picoRTOS
SRC_C_FILES += $(PICORTOS_DIR)/picoRTOS.c
SRC_C_FILES += $(PICORTOS_DIR)/arch/arm/cm3/picoRTOS_port.c
SRC_S_FILES += $(PICORTOS_DIR)/arch/arm/cm3/picoRTOS_portasm.S

SRC_H_FILES += $(PICORTOS_DIR)/picoRTOS.h
SRC_H_FILES += $(PICORTOS_DIR)/arch/arm/cm3/picoRTOS_types.h
SRC_H_FILES += picoRTOSConfig.h

# demo
SRC_C_FILES += blink.c

OBJ_FILES := $(SRC_C_FILES:%.c=%.o)
OBJ_FILES += $(SRC_S_FILES:%.S=%.o)

# fix paths
OBJ_FILES := $(addprefix $(BUILD_DIR)/,$(OBJ_FILES))

all: sketch
	arduino-cli compile -b arduino:sam:arduino_due_x -p /dev/ttyACM0 \
	  -l serial $(SKETCH_DIR)

sketch:
	mkdir -p $(SKETCH_DIR)
	cp $(SKETCH).ino $(SRC_C_FILES) $(SRC_S_FILES) $(SRC_H_FILES) $(SKETCH_DIR)

upload:
	arduino-cli upload -b arduino:sam:arduino_due_x -p /dev/ttyACM0 \
	  -l serial $(SKETCH_DIR)

debug:
	arduino-cli debug -b arduino:sam:arduino_due_x -p /dev/ttyACM0 \
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

.PHONY: sketch clean splint uncrustify
