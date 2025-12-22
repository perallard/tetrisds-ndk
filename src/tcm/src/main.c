#include "nds.h"
#include "dtcm.h"
#include "cpu.h"
#include "gfx.h"
#include "interrupts.h"

#include "term.h"

#include "tcm.h"

static void init(void);
static void init_gfx(void);
static void vblank_handler(void);

static const char *info_text = "TCM code demo";

/*
 * These two definitions below are needed so the linker can patch the CRT0
 * code to execute our own main function.
 */
int main(void);
int (*main_ref)(void) __attribute__ ((section("text_main_fcn_ref"))) = &main;

int main()
{
  init();
  init_gfx();
  term_init((unsigned short *)0x06200000, (unsigned short *)0x06202000, 0);

  term_set_cursor(0, 0);
  term_printf("%s", info_text);
  term_set_cursor(0, 5);
  term_printf("fibonacci(10) = %i", fibonacci(10));
  term_draw();

  while (1) {
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


void vblank_handler(void)
{
  thread_irq_bits |= IS_VBLANK;
}

void init_gfx(void)
{
  VRAMCNT_A = 0x81;
  VRAMCNT_B = 0x00;
  VRAMCNT_C = 0x84;
  VRAMCNT_D = 0x00;

	//set up 256*256 16bit color bitmap on engine A
  DISPCNT = 0x00010803;
  BG3CNT = 0x4084;
	BLDCNT = 0;
  BG3HOFS = 0;
  BG3VOFS = 0;
	BG3PB = 0;
	BG3PC = 0;
	BG3X = 0;
	BG3Y = 0;
	BG3PA = 0x0100;
	BG3PD = 0x0100;

  // setup palette 0
  *(unsigned short *)0x05000400 = 0;      // Background color
  *(unsigned short *)0x05000402 = 0x7fff; // Palette 0 color 1

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
}
