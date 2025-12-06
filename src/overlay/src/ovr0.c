#include "fix_math.h"
#include "gfx.h"
#include "interrupts.h"
#include "nds.h"
#include "util.h"
#include "ovr.h"
#include "common.h"

static fix12 angle = 0;

void _overlay0_entry()
{
  load_file("spy_vs_spy.bin", (void *)0x06000000);

  while(1) {
    unsigned short keypad = read_key_presses();

    if (keypad & PAD_Up)
      break;

    ndk_wait_vblank_intr();

    // Create a 2d clockwise rotation matrix
    fix12 inv = ndk_fix_inverse(F2X(1));

    fix12 transform[4] = {
      fix_mul(inv, tcm1_cos(angle)),
      fix_mul(inv, tcm1_sin(angle)),
      fix_mul(inv, -tcm1_sin(angle)),
      fix_mul(inv, tcm1_cos(angle))
    };

    ndk_bg_set_affine_transform(&BG3PA, transform, 128, 96, 0, 0);

    angle += 4;
  }
}
