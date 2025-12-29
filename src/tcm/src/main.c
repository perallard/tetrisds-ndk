#include "nds.h"
#include "dtcm.h"
#include "cpu.h"
#include "gfx.h"
#include "interrupts.h"

#include "term.h"

#include "tcm.h"

static void init(void);
static void init_obj(void);
static void init_gfx(void);
static void vblank_handler(void);

static const char *info_text = "TCM code demo";

static inline fix12 fix12_mul(fix12 a, fix12 b)
{
  long long int result = (long long int)a * b;
  result >>= 12;

  return result;
}

/*
 * These two definitions below are needed so the linker can patch the CRT0
 * code to execute our own main function.
 */
int main(void);
int (*main_ref)(void) __attribute__ ((section("text_main_fcn_ref"))) = &main;

int main(void)
{
  init();
  init_gfx();
  init_obj();
  term_init((unsigned short *)0x06200000, (unsigned short *)0x06202000, 0);

  term_set_cursor(0, 0);
  term_printf("%s", info_text);
  term_set_cursor(0, 5);
  term_printf("fibonacci(10) = %i", fibonacci(10));
  term_draw();

  fix12 a = 0;
  fix12 b = 0;

  while (1) {
    // main loop
    ndk_wait_vblank_intr();

    fix12 x = fix12_sin(a);
    x = fix12_mul(x, (124 << 12));
    x += (124 << 12);
    x >>= 12;
    a += 16;

    fix12 y = fix12_cos(b);
    y = fix12_mul(y, (92 << 12));
    y += (92 << 12);
    y >>= 12;
    b += 8;

    unsigned short *oam_ptr = (unsigned short* )0x07000000;

    oam_ptr[0] = y;
    oam_ptr[1] = x;
  }
}

void init(void)
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

void init_obj(void)
{
  /* setup OBJ 0 */

  unsigned int *vram_obj_ptr = (unsigned int *)0x06400000;

  for (int i = 0; i < 8; i++) {
    *vram_obj_ptr++ = 0x11111111;
  }

  unsigned short *oam_ptr = (unsigned short* )0x07000000;

  oam_ptr[0] = 0x005C;
  oam_ptr[1] = 0x0000;
  oam_ptr[2] = 0x0000;

  // setup palette 0
  *(unsigned short *)0x05000200 = 0;      // Background color
  *(unsigned short *)0x05000202 = 0x7fff; // Palette 0 color 1
}

void vblank_handler(void)
{
  thread_irq_bits |= IS_VBLANK;
}

void init_gfx(void)
{
	//set up OBJ 16 color 1D mapping 32k on engine A
  VRAMCNT_A = 0x82;
  VRAMCNT_B = 0x00;
  VRAMCNT_C = 0x84;
  VRAMCNT_D = 0x00;

  DISPCNT = 0x00811010;

	//set up 16-color text BG on engine B
  DB_DISPCNT = 0x00010403;
  DB_BG2CNT = 0x0400;
	DB_BLDCNT = 0;
  DB_BG2HOFS = 0;
  DB_BG2VOFS = 0;
	DB_BG2PB = 0;
	DB_BG2PC = 0;
	DB_BG2X = 0;
	DB_BG2Y = 0;
	DB_BG2PA = 0x0100;
	DB_BG2PD = 0x0100;

  // setup palette 0
  *(unsigned short *)0x05000400 = 0;      // Background color
  *(unsigned short *)0x05000402 = 0x7fff; // Palette 0 color 1
}
