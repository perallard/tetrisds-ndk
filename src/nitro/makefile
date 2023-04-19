CC=$(DEVKITARM)/bin/arm-none-eabi-gcc

ifeq ($(DEBUG_BUILD),1)
	CFLAGS = -g -O2 -DDEBUG_BUILD
else
	CFLAGS = -O2
endif

CFLAGS += -Werror -Wall -marm -march=armv5te -mtune=arm946e-s \
-mfloat-abi=soft -fomit-frame-pointer

.PHONY: all clean

all: symbols.o

symbols.o: symbols.c symbols.txt headers/*
	$(CC) $(CFLAGS) -c symbols.c -o $@
	./add_symbols.sh $@ symbols.txt

clean:
	rm -f symbols.o
