export ROOT_DIR = $(realpath .)
export UTIL_PATH = $(realpath ../util)
export CC=$(DEVKITARM)/bin/arm-none-eabi-gcc
export OBJCOPY=$(DEVKITARM)/bin/arm-none-eabi-objcopy

BUILD_DIR = build

GCC_VERSION=$(shell ls $(DEVKITARM)/lib/gcc/arm-none-eabi)

LD=$(DEVKITARM)/bin/arm-none-eabi-ld
LDFLAGS= --use-blx -nostdlib -Map=$(BUILD_DIR)/smap

PATCHED_ROM_NAME=overlay

PATCHED_ROM_FILE=$(BUILD_DIR)/$(PATCHED_ROM_NAME).nds
XDELTA_FILE=$(BUILD_DIR)/$(PATCHED_ROM_NAME).xdelta

MODULES=src $(UTIL_PATH) $(NDK_DIR)

OBJS=src/main.o src/common.o src/ovr0.o src/ovr1.o src/ovr_itcm.o \
	$(UTIL_PATH)/term.o $(UTIL_PATH)/snprintf.o $(NDK_DIR)/symbols.o

ARM9_PATCHES = $(BUILD_DIR)/arm9.o

.PHONY: all debug patch clean $(MODULES)

all: $(PATCHED_ROM_FILE)

debug: export DEBUG_BUILD:=1
debug: $(PATCHED_ROM_FILE)

patch: $(PATCHED_ROM_FILE)
	xdelta3 -s $(TETRIS_DS_ROM) $< $(XDELTA_FILE)

setup:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/overlay
	ndstool -9 $(BUILD_DIR)/arm9.bin -7 $(BUILD_DIR)/arm7.bin \
	-t $(BUILD_DIR)/banner.bin -x $(TETRIS_DS_ROM)
	touch setup

$(OBJS) &: $(MODULES)
	@echo -n ""

$(MODULES):
	$(MAKE) -C $@

$(PATCHED_ROM_FILE): setup $(ARM9_PATCHES)
	cp $(BUILD_DIR)/arm9.bin $(BUILD_DIR)/arm9_patched.bin
	$(NDK_DIR)/patcher.sh $(ARM9_PATCHES) $(BUILD_DIR)/arm9_patched.bin

	$(OBJCOPY) -O binary -j ovr1 $(ARM9_PATCHES) $(BUILD_DIR)/overlay/overlay_0001.bin
	$(OBJCOPY) -O binary -j ovr0 $(ARM9_PATCHES) $(BUILD_DIR)/overlay/overlay_0000.bin
	$(OBJCOPY) -O binary -j itcm1 $(ARM9_PATCHES) $(BUILD_DIR)/overlay/overlay_0002.bin
	$(OBJCOPY) -O binary -j ovr_tbl $(ARM9_PATCHES) $(BUILD_DIR)/overlay_table.bin

	ndstool -9 $(BUILD_DIR)/arm9_patched.bin -7 $(BUILD_DIR)/arm7.bin \
	-y9 $(BUILD_DIR)/overlay_table.bin -d data -y $(BUILD_DIR)/overlay \
	-e9 0x02000800 -r9 0x02000000 -e7 0x2380000 -r7 0x2380000 -h 0x4000 \
	-g XOVE 00 OVERLAY -t $(BUILD_DIR)/banner.bin -c $@

	ndstool -se $@
	ndstool -sd $@

$(ARM9_PATCHES): $(OBJS)
	$(LD) $(LDFLAGS) -T link.ld -o $@ $(OBJS) \
	-L$(DEVKITARM)/lib/gcc/arm-none-eabi/$(GCC_VERSION) \
	-L$(DEVKITARM)/arm-none-eabi/lib -lgcc -lc

clean:
	rm -rf $(BUILD_DIR) setup
	for p in $(MODULES); do $(MAKE) -C $$p clean; done
