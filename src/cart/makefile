export ROOT_DIR = $(realpath .)
export UTIL_PATH = $(realpath ../util)

GCC_VERSION=$(shell ls $(DEVKITARM)/lib/gcc/arm-none-eabi)

LD=$(DEVKITARM)/bin/arm-none-eabi-ld
LDFLAGS= --use-blx -nostdlib -Map=build/smap

PATCHED_ROM_NAME=cart
PATCHED_ROM_FILE=build/$(PATCHED_ROM_NAME).nds
XDELTA_FILE=build/$(PATCHED_ROM_NAME).xdelta

MODULES=src $(NDK_DIR) $(UTIL_PATH)

OBJS=src/cart.o $(NDK_DIR)/symbols.o $(UTIL_PATH)/term.o

.PHONY: all debug patch clean $(MODULES)

all: $(PATCHED_ROM_FILE)

debug: export DEBUG_BUILD:=1
debug: $(PATCHED_ROM_FILE)

setup:
	mkdir -p build
	ndstool -9 build/arm9.bin -7 build/arm7.bin -t build/banner.bin \
	-x $(TETRIS_DS_ROM)
	touch setup

build/arm9.o: $(OBJS)
	$(LD) $(LDFLAGS) -T link.ld -o $@ $(OBJS) \
	-L$(DEVKITARM)/lib/gcc/arm-none-eabi/$(GCC_VERSION) \
	-L$(DEVKITARM)/arm-none-eabi/lib -lgcc -lc

$(OBJS) &: $(MODULES)
	@echo -n ""

$(MODULES):
	$(MAKE) -C $@

$(PATCHED_ROM_FILE): setup build/arm9.o
	cp build/arm9.bin build/arm9_patched.bin
	$(NDK_DIR)/patcher.sh build/arm9.o build/arm9_patched.bin

	ndstool -9 build/arm9_patched.bin -7 build/arm7.bin \
	-d data -e9 0x02000800 -r9 0x02000000 -e7 0x2380000 \
	-r7 0x2380000 -h 0x4000 -g XCAE 00 CART -t build/banner.bin -c $@

	ndstool -se $@
	ndstool -sd $@

patch: $(PATCHED_ROM_FILE)
	xdelta3 -s $(TETRIS_DS_ROM) $< $(XDELTA_FILE)

clean:
	rm -rf build setup
	for p in $(MODULES); do $(MAKE) -C $$p clean; done
