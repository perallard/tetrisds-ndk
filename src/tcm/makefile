export ROOT_DIR = $(realpath .)
export UTIL_PATH = $(realpath ../util)

GCC_VERSION=$(shell ls $(DEVKITARM)/lib/gcc/arm-none-eabi)

LD=$(DEVKITARM)/bin/arm-none-eabi-ld

LDFLAGS= --use-blx -nostdlib -Map=build/smap

PATCHED_ROM_NAME=tcm

PATCHED_ROM_FILE=build/$(PATCHED_ROM_NAME).nds
XDELTA_FILE=build/$(PATCHED_ROM_NAME).xdelta

# Add any projects that are to be built recursively here
MODULES=src $(NDK_DIR) $(UTIL_PATH)
# Add all the objects that go into the final linking process here
OBJS=src/main.o src/tcm.o $(NDK_DIR)/symbols.o $(UTIL_PATH)/term.o

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

# This recipe will tell make that the prerequisite(s) build *all*
# targets. So this recipe will be executed only once! See:
# https://www.gnu.org/software/make/manual/html_node/Multiple-Targets.html
#
# Strictly it will work with independent targets ie. : since a target is
# built only once per makefile invocation, the sub-modules will be recursed
# into only once.
#
# Requires GNU make >= 4.3
$(OBJS) &: $(MODULES)
	@echo -n ""

$(MODULES):
	$(MAKE) -C $@

$(PATCHED_ROM_FILE): setup build/arm9.o
	cp build/arm9.bin build/arm9_patched.bin
	$(NDK_DIR)/patch_tool build/arm9.o build/arm9_patched.bin

	ndstool -9 build/arm9_patched.bin -7 build/arm7.bin \
	-d data -e9 0x02000800 -r9 0x02000000 -e7 0x2380000 \
	-r7 0x2380000 -h 0x4000 -g XSNE 00 SOUND -t build/banner.bin -c $@

	ndstool -se $@
	ndstool -sd $@

patch: $(PATCHED_ROM_FILE)
	xdelta3 -s $(TETRIS_DS_ROM) $< $(XDELTA_FILE)

clean:
	rm -rf build setup
	for p in $(MODULES); do $(MAKE) -C $$p clean; done
