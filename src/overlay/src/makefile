
ifeq ($(DEBUG_BUILD),1)
	CFLAGS = -g -O2 -DDEBUG_BUILD
else
	CFLAGS = -O2
endif

CFLAGS += -MMD -Werror -Wall -marm -march=armv5te -mtune=arm946e-s \
-mfloat-abi=soft -fomit-frame-pointer

INCLUDES = -I$(NDK_DIR)/headers -I$(UTIL_PATH)

OBJS = main.o common.o ovr0.o ovr1.o ovr_itcm.o

.PHONY: all clean

all: $(OBJS)

-include *.d

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.s
	$(AS) $< -o $@

clean:
	rm -f *.o *.d
