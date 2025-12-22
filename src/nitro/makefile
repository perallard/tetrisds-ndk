CC=$(DEVKITARM)/bin/arm-none-eabi-gcc

ifeq ($(DEBUG_BUILD),1)
	CFLAGS = -g -O2 -DDEBUG_BUILD
else
	CFLAGS = -O2
endif

CFLAGS += -Werror -Wall -marm -march=armv5te -mtune=arm946e-s \
-mfloat-abi=soft -fomit-frame-pointer

.PHONY: all clean

all: symbols.o patch_tool

patch_tool: patch_tool.c
	gcc -O2 -Werror -Wall $^ -o $@

symbols.o: symbols.c symbols.txt headers/* add_symbols.sh
	$(CC) $(CFLAGS) -c symbols.c -o $@
	./add_symbols.sh $@ symbols.txt

clean:
	rm -f symbols.o patch_tool
