CC=$(DEVKITARM)/bin/arm-none-eabi-gcc
LD=$(DEVKITARM)/bin/arm-none-eabi-ld

ifeq ($(DEBUG_BUILD),1)
	CFLAGS = -g -O2 -DDEBUG_BUILD
else
	CFLAGS = -O2
endif

CFLAGS += -MMD -Werror -Wall -mthumb -march=armv5te -mtune=arm946e-s \
-mfloat-abi=soft -fomit-frame-pointer

LDFLAGS = -r --use-blx -nostdlib

INCLUDES = -I$(NDK_DIR)/headers -I$(ROOT_DIR) -I$(UTIL_PATH)

LIBS =

OBJS = cart.o

.PHONY: all clean

all: $(OBJS)

-include *.d

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.s
	$(AS) $< -o $@

clean:
	rm -f *.o *.d
