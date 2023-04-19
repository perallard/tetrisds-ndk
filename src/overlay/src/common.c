#include <limits.h>

#include "file.h"
#include "nds.h"

#include "common.h"

static unsigned short old_gamepad_state;

unsigned short read_key_presses(void)
{
  int new_keys = ext_gamepad;

  new_keys |= KEYINPUT;

  new_keys ^= 0x2fff;
  new_keys &= 0x2fff;

  int old_keys = old_gamepad_state;

  old_gamepad_state = new_keys;

  int change_mask = new_keys ^ old_keys;

  new_keys &= change_mask;

  return new_keys;
}

int load_file(char *filename, void *dest)
{
  struct file h;

  ndk_file_init_handle(&h);

  if(ndk_file_open(&h, filename) == 0)
    return 0;

  int read = ndk_file_read(&h, dest, INT_MAX);

  ndk_file_close(&h);

  return read;
}
