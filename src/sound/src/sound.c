#include <stdbool.h>
#include <stddef.h>

#include "cpu.h"
#include "dtcm.h"
#include "file.h"
#include "gfx.h"
#include "interrupts.h"
#include "nds.h"
#include "sound.h"

#include "term.h"

#define SND_DATA_SIZE 512*1024

struct sdat_symbol
{
	char type[4];		              // 'SYMB'
	unsigned int size;		        // size of this Symbol Block
	unsigned int rec_offset[8];	  // offset of Records (note below)
	char reserved[24];	          // unused, 0s
};

struct sdat_symbolrec
{
	unsigned int count;		        // No of entries in this record
	unsigned int entry_offset[];	// Array of offsets of each entry
};

static void init(void);
static void init_sound(void);
static void init_displays(void);
static void read_key_presses(void);
static void vblank_handler(void);
static char *get_sdat_seq_symbol(int seq_id);


static unsigned short old_gamepad_state;
static unsigned short gamepad_btn_down;

static char snd_data[SND_DATA_SIZE];

static struct sound_handle my_sound_handle;

static struct sound_sdat_heap *snd_heap;
static struct sound_sdat_arch snd_arch;

static int group_heap_id;

static const char *instructions =
"Play songs from the Tetris DS\n"
"sound archive. Instructions:\n"
" Y to play the next song\n"
" X to toggle play/pause\n"
" A to stop (2 second fadeout)\n"
" B to set tempo to 200\n"
"\n"
"Playing: ";

/*
 * These two definitions below are needed so the linker can patch the CRT0 code
 * to execute our own main function.
 */
int main(void);
int (*main_ref)(void) __attribute__ ((section("text_main_fcn_ref"))) = &main;

int main()
{
  init();
  // The sound library uses the file API to load sound data from the cart.
  ndk_fat_mount(3);
  init_sound();
  init_displays();

  term_init((unsigned short *)0x06000000, (unsigned short *)0x06002000, 0);

  term_set_cursor(0, 1);
  term_prints(instructions);

  int seq_id = 0;
  int toggle = 1;

  while(1) {
    read_key_presses();

    if ((ext_gamepad & 0x8000) != 0)
      ndk_handle_lid_closed(0xc, 0, 0);

    if (gamepad_btn_down & PAD_Y) {
      seq_id++;
      if (seq_id == 9) seq_id = 1;

      term_set_cursor(9, 8);

      if (ndk_sound_add_source_seq(&my_sound_handle, seq_id)) {
        char *seq_name = get_sdat_seq_symbol(seq_id);
        if (seq_name != NULL) {
          term_prints(seq_name);
        } else {
          term_printf("seq %d", seq_id);
        }
      } else {
        term_printf("%s", "ndk_sound_add_source_seq error!\n");
      }
    }

    if (gamepad_btn_down & PAD_X) {
      ndk_sound_handle_pause(&my_sound_handle, toggle);
      toggle = (toggle == 1) ? 0 : 1;
    }

    if (gamepad_btn_down & PAD_A) {
      ndk_sound_seq_stop(seq_id, 120);
    }

    if (gamepad_btn_down & PAD_B) {
      ndk_sound_handle_set_tempo(&my_sound_handle, 200);
    }

    ndk_sound_process();
    ndk_wait_vblank_intr();
    term_draw();
  }
}

void init()
{
    ndk_platform_init();

    ndk_irq_set_handler(IE_VBLANK, &vblank_handler);

    ndk_gfx_init();
    ndk_irq_enable_interrupt_sources(IE_VBLANK);
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

void init_sound()
{
  ndk_sound_init();

  snd_heap = ndk_sound_init_sdat_data_heap(snd_data, SND_DATA_SIZE);

  ndk_sound_open_sdat_archive(&snd_arch, "/sound_data.sdat", snd_heap, true);

  ndk_sound_sdat_init_seq_players(snd_heap);

  // used when the group should be freed from the sound heap.
  group_heap_id = ndk_sound_sdat_create_heap_id(snd_heap);

  ndk_sound_sdat_load_group(0, snd_heap);

  ndk_sound_handle_init(&my_sound_handle);
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

char *get_sdat_seq_symbol(int seq_id)
{
  struct sound_sdat_arch *sdat = ndk_sound_get_current_sdat_archive();

  if (sdat == NULL)
    return NULL;

  struct sdat_symbol *symb = sdat->cached_symbol_block;

  if (symb == NULL)
    return NULL;

  struct sdat_symbolrec *symb_rec =
                (struct sdat_symbolrec *) ((char *)symb + symb->rec_offset[0]);

  if (seq_id < 0 || seq_id >= symb_rec->count)
    return NULL;

  return (char *)symb + symb_rec->entry_offset[seq_id];
}
