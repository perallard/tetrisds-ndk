#include "term.h"

#include "nds.h"
#include "cpu.h"
#include "memory.h"

#define TERM_ROWS 24
#define TERM_COLS 32
#define TERM_FIRST_ROW 0
#define TERM_LAST_ROW (TERM_ROWS - 1)
#define TERM_FIRST_COL 0
#define TERM_LAST_COL (TERM_COLS - 1)
#define TERM_MAP_SIZE (TERM_ROWS * TERM_COLS)
#define TERM_CONV_BUF_SIZE 1024

struct terminal {
  unsigned short *char_data_loc;
  unsigned short *screen_data_loc;
  int palette;
  int x;
  int y;
  int dma_channel;
  unsigned short screen_buf[TERM_MAP_SIZE];
  char conv_buf[TERM_CONV_BUF_SIZE];
};

static void term_line_feed();
static void term_clear_line(int y);
static void term_scroll_up();
static void term_draw_character(char c);
static void term_set_font(char *font);


static struct terminal term_base;

// The font is a char array so it goes here.
#include "font8x8_basic.h"


static inline unsigned short *term_get_screen_buf_location(int x, int y)
{
  return &term_base.screen_buf[x + y * TERM_COLS];
}

void term_init(unsigned short *char_data_loc, unsigned short *screen_data_loc,
               int palette)
{
  term_base.char_data_loc = char_data_loc;
  term_base.screen_data_loc = screen_data_loc;
  term_base.palette = palette;
  term_base.x = 0;
  term_base.y = 0;
  term_base.dma_channel = 3;

  term_clear();
  term_set_font(&font8x8_basic[0][0]);
}

void term_set_font(char *font)
{
  unsigned int *vram = (unsigned int *)term_base.char_data_loc;

  for (int c = 0; c < 127; c++) {
    char *bitmap = &font[c * 8];
    unsigned int conv = 0;

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if (bitmap[i] & (1 << (7 - j))) {
          conv |= 1;
        }
        conv <<= 4;
      }
      *vram++ = conv;
    }
  }
}

void term_clear()
{
  unsigned int tile = term_base.palette << 12 | ' ';

  tile |= tile << 16;

  ndk_memory_fast_32bit_fill(tile, term_base.screen_buf, TERM_MAP_SIZE * 2);

  term_base.x = 0;
  term_base.y = 0;
}

void term_draw()
{
  ndk_cpu_clean_dcache_lines(term_base.screen_buf, TERM_MAP_SIZE * 2);

  ndk_memory_dma_16bit_copy(term_base.dma_channel, term_base.screen_buf,
                            term_base.screen_data_loc, TERM_MAP_SIZE * 2);
}

void term_printf(const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  ndk_vsnprintf(term_base.conv_buf, sizeof(term_base.conv_buf), fmt, ap);
  va_end (ap);

  term_prints(term_base.conv_buf);
}

void term_set_cursor(int x, int y)
{
  if (x < TERM_FIRST_ROW) x = TERM_FIRST_ROW;
  if (x > TERM_LAST_ROW) x = TERM_LAST_ROW;

  term_base.x = x;

  if (y < TERM_FIRST_COL) y = TERM_FIRST_COL;
  if (y > TERM_LAST_COL) y = TERM_LAST_COL;

  term_base.y = y;
}

void term_prints(const char *s)
{
  char c = *s++;

  while (c != '\0') {
    term_printc(c);
    c = *s++;
  }
}

void term_printc(char c)
{
  if (c == '\n') {
    term_line_feed();
  } else {
    if (term_base.x == TERM_COLS) {
      term_line_feed();
    }

    term_draw_character(c);
    term_base.x++;
  }
}

void term_line_feed()
{
  term_base.x = 0;

  int curr_y = term_base.y;

  if (curr_y < TERM_LAST_ROW) {
    term_base.y = curr_y + 1;
  } else {
    term_scroll_up();
  }
}

void term_clear_line(int y)
{
  if (y < TERM_FIRST_ROW || y > TERM_LAST_ROW)
    return;

  unsigned short *last = term_get_screen_buf_location(0, y);
  unsigned int tile = term_base.palette << 12 | ' ';

  tile |= tile << 16;

  ndk_memory_fast_32bit_fill(tile, last, TERM_COLS * 2);
}

void term_scroll_up()
{
  unsigned short *src = term_get_screen_buf_location(0, 1);
  unsigned short *dst = term_get_screen_buf_location(0, 0);

  ndk_memory_fast_32bit_copy(src, dst, TERM_COLS * (TERM_ROWS - 1) * 2);

  term_clear_line(TERM_LAST_ROW);
}

void term_draw_character(char c)
{
  char cc = c;

  if (c < 32 || c >= 127)
    cc = ' ';

  unsigned short *p = term_get_screen_buf_location(term_base.x, term_base.y);

  *p = term_base.palette << 12 | cc;
}
