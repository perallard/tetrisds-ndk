#include <stdbool.h>
#include <stddef.h>

#include "nds.h"
#include "gfx.h"
#include "cart.h"
#include "interrupts.h"
#include "timers.h"
#include "cpu.h"
#include "dtcm.h"

#include "term.h"

struct FAT_entry {
  unsigned int start;
  unsigned int end;
};

static void init(void);
static void init_displays(void);
static void read_key_presses(void);
static void vblank_handler(void);

unsigned short old_gamepad_state;
unsigned short gamepad_btn_down;

const char *instructions =
"Perfom cart operations\n"
"Instructions:\n"
" Y get ROM chip id\n"
" X to read data from first.txt\n"
" A to read data from second.txt\n";

struct FAT_entry FAT[2];
char data[256];

void print_file_data(unsigned int id)
{
  unsigned int start = FAT[id].start;
  int length = FAT[id].end - FAT[id].start;

  ndk_cart_read(3, start, data, length, NULL, 0, false);
  term_set_cursor(0, 19);
  term_printf("%s", data);
}

/*
 * The definition below is needed so the linker can patch the original
 * main function with our own main function.
 */
__attribute__((section("text_main_fcn"), target("arm")))
int main()
{
  init();
  ndk_cart_init();
  init_displays();

  term_init((unsigned short *)0x06000000, (unsigned short *)0x06002000, 0);

  term_set_cursor(0, 1);
  term_printf("%s", instructions);

  unsigned int cart_id = ndk_cart_get_rom_id();

  // cache FAT table in RAM
  ndk_cart_read(3, 0xD2600, &FAT, sizeof(FAT), NULL, 0, false);

  while(1) {
    read_key_presses();

    if ((ext_gamepad & 0x8000) != 0) {
      ndk_handle_lid_closed(0xc, 0, 0);
    }

    if (gamepad_btn_down & PAD_Y) {
      term_set_cursor(0, 15);
      term_printf("Cart ROM id: %x", cart_id);
    }

    if (gamepad_btn_down & PAD_X) {
      print_file_data(0);
    }

    if (gamepad_btn_down & PAD_A) {
      print_file_data(1);
    }

    ndk_wait_vblank_intr();
    term_draw();
  }
}

void init()
{
  ndk_platform_init();

  ndk_irq_set_handler(IS_VBLANK, &vblank_handler);

  ndk_gfx_init();
  ndk_irq_enable_interrupt_sources(IS_VBLANK);
  // Master IRQ enable
  IME = 1;
  ndk_cpu_enable_irq();
  ndk_lcd_set_vblank_irq_enable(1);
}

void init_displays()
{
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

  //set up 256*256 16bit color bitmap on engines B
  DB_DISPCNT = 0x00010803;
  DB_BG3CNT = 0x4084;
}

void vblank_handler(void)
{
  thread_irq_bits |= IS_VBLANK;
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
