AS=$(DEVKITARM)/bin/arm-none-eabi-as
CC=$(DEVKITARM)/bin/arm-none-eabi-gcc
LD=$(DEVKITARM)/bin/arm-none-eabi-ld

ifeq ($(DEBUG_BUILD),1)
	CFLAGS = -g -O2 -DDEBUG_BUILD
else
	CFLAGS = -O2
endif

CFLAGS += -MMD -Werror -Wall -marm -march=armv5te -mtune=arm946e-s \
-mfloat-abi=soft -fomit-frame-pointer -I$(NDK_DIR)/headers

CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions

LDFLAGS = -r --use-blx

OBJS = term.o snprintf.o

.PHONY: all setup clean

all: $(OBJS)

-include *.d

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $< -o $@

clean:
	rm -f *.o *.d
