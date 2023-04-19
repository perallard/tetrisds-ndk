#ifndef UTIL_TERM_INCLUDE_FILE
#define UTIL_TERM_INCLUDE_FILE

#include <stdarg.h>

void term_init(unsigned short *char_data, unsigned short *screen_data,
               int palette);

void term_draw();

void term_clear();

void term_set_cursor(int x, int y);

void term_printc(char c);

void term_prints(const char *s);

void term_printf(const char *fmt, ...);

#endif // UTIL_TERM_INCLUDE_FILE
