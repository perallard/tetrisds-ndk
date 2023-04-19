#include <stdbool.h>

#include "cpu.h"
#include "dtcm.h"
#include "gfx.h"
#include "interrupts.h"
#include "memory.h"
#include "nds.h"
#include "timers.h"
#include "touch.h"

#include "term.h"

/*
 * Thumb is used by default so we need to tell GCC to use ARM here.
 */
__attribute__((target("arm")))
static void plot(short color, int x, int y);

// clear lower screen
static void clear(void);
static void init(void);
static void read_key_presses();
static void vblank_handler(void);


static unsigned short old_gamepad_state;
static unsigned short gamepad_btn_down;

static const char *info_text =
"Demonstrate the touch API.\n"
"L-btn: auto samling\n"
"R-btn: manual sampling\n"
"X: clear screen";

/*
 * These two definitions below are needed so the linker can patch the CRT0
 * code to execute our own main function.
 */
int main(void);
int (*main_ref)(void) __attribute__ ((section("text_main_fcn_ref"))) = &main;

int main()
{
  init();

  VRAMCNT_A = 0x81;
  VRAMCNT_B = 0x00;
  VRAMCNT_C = 0x84;
  VRAMCNT_D = 0x00;

	//set up 16-color text BG on engine A
  DISPCNT = 0x00010403;
  BG2CNT = 0x0400;

  // setup palette 0
  *(unsigned short *)0x05000000 = 0;  // Background color
  *(unsigned short *)0x05000002 = 0x7fff; // Palette 0 color 1

	// set up 256*256 16bit color bitmaps on engine b
  DB_DISPCNT = 0x00010803;
  DB_BG3CNT = 0x4084;

  term_init((unsigned short *)0x06000000, (unsigned short *)0x06002000, 0);

  term_prints(info_text);

  struct touch_pos tbuf[5];
  bool auto_sample = false;

  while(1) {
    read_key_presses();

    if ((ext_gamepad & 0x8000) != 0)
      ndk_handle_lid_closed(0xc, 0, 0);

    if (gamepad_btn_down & PAD_X) {
      clear();
    }

    if (gamepad_btn_down & BTN_R) {
      auto_sample = false;
      ndk_touch_stop_auto_sampling();
    }

    if (gamepad_btn_down & BTN_L) {
      auto_sample = true;
      ndk_touch_start_auto_sampling(0, 4, tbuf, 5);
    }

    if (auto_sample) {
  	  struct touch_pos result;
      unsigned int offset = ndk_touch_get_last_sample_offset();

      for (unsigned int i = 0; i < 4; i++) {
        /*
         * offset 'points' to the last (top) element. So to move back to the
         * first element 3 is subtracted since 4 values are read per frame.
         */
        int idx = (offset - 3 + i) % 5;

        ndk_touch_transform_into_screen_coords(&result, &tbuf[idx]);

        if (result.pen_down != 0 && result.valid == 0)
          plot(0xffff, result.x, result.y);
      }
    } else {
      struct touch_pos sample;

      ndk_touch_request_sample();

      if (ndk_touch_get_sample(&sample) == TOUCH_SUCCESS) {
        ndk_touch_transform_into_screen_coords(&sample, &sample);

        if (sample.pen_down != 0 && sample.valid == 0)
          plot(0xfc1f, sample.x, sample.y);
      }
    }

    ndk_wait_vblank_intr();

    term_draw();
  }
}

void init()
{
    ndk_platform_init();
    ndk_touch_init();

    struct touch_xform xform;

    if (! ndk_touch_get_transform_from_firmware_settings(&xform))
      ndk_panic();

    ndk_touch_set_coordinate_transform(&xform);
    ndk_irq_set_handler(IE_VBLANK, &vblank_handler);
    ndk_gfx_init();
    // Enable V-Blank IRQ
    ndk_irq_enable_interrupt_sources(IE_VBLANK);
    // Master IRQ enable
    IME = 1;
    ndk_cpu_enable_irq();
    ndk_lcd_set_vblank_irq_enable(1);
}

void vblank_handler(void)
{
  thread_irq_bits |= 1;
}

void read_key_presses()
{
  int new_keys = ext_gamepad;

  new_keys |= KEYINPUT;

  new_keys ^= 0x2fff;
  new_keys &= 0x2fff;

  int old_keys = old_gamepad_state;

  old_gamepad_state = new_keys;

  int change_mask = new_keys ^ old_keys;

  new_keys &= change_mask;

  gamepad_btn_down = (unsigned short) new_keys;
}

void plot(short color, int x, int y)
{
  unsigned short *scr = (unsigned short *)0x06200000;

  scr[x + y * 256] = color;
}

void clear()
{
  ndk_memory_16bit_fill(0, (unsigned short *)0x06200000,
                        SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(unsigned short));
}