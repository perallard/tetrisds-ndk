#include "fix_math.h"
#include "gfx.h"
#include "interrupts.h"
#include "nds.h"
#include "util.h"

#include "common.h"

static fix_angle angle = FIX_ANGLE_0;

void _overlay1_entry()
{
  load_file("c64_grafitti.bin", (void *)0x06000000);

  while(1) {
    unsigned short keypad = read_key_presses();

    if (keypad & PAD_Up)
      break;

    ndk_wait_vblank_intr();

    fix12 transform[4] = {
      F2X(1), 0,
      0, F2X(1)
    };

    int xcoord = (64 * fix_cos(angle)) >> 12;

    ndk_bg_set_affine_transform(&BG3PA, transform, 128, 96, xcoord, 0);

    angle += 256;
  }
}
