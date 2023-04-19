#include "snprintf.h"

#include <limits.h>

#include "fix_math.h"

typedef struct buffer buffer;

struct buffer {
  char *base;
  char *top;
  char *ptr;
};

static void buffer_init(buffer *b, char *out, int len)
{
  b->base = out;
  b->top = out + len - 1;
  b->ptr = out;
}

static int buffer_length(buffer *b)
{
  return b->ptr - b->base;
}

static void put_terminator(buffer *b)
{
  *b->ptr = '\0';
}

static void put_char(buffer *b, char c)
{
  if (b->ptr < b->top)
    *b->ptr++ = c;
}

static void put_string(buffer *b, const char *s)
{
  char c = *s++;

  while (c != '\0') {
    put_char(b, c);
    c = *s++;
  }
}

static void put_uint(buffer *b, unsigned int u)
{
  char buf[11];
  int cnt = 0;

  while(1) {
    buf[cnt] = (u % 10) + '0';

    u /= 10;

    if (u == 0)
      break;

    cnt++;
  }

  for (int i = cnt; i >= 0; i--)
    put_char(b, buf[i]);
}

static void put_int(buffer *b, int i)
{
  if (i < 0) {
    if (i > INT_MIN) {
      i = -i;
    }
    put_char(b, '-');
  }

  put_uint(b, (unsigned int)i);
}

static void put_hex(buffer *b, unsigned int u)
{
  static char hex[16] =
  {'0', '1', '2', '3', '4', '5', '6', '7',
   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  for (int i = 7; i >= 0; i--)
    put_char(b, hex[ (u >> (4*i)) & 15 ]);
}

static void put_pointer(buffer *b, void *p)
{
  put_hex(b, (unsigned int)p);
}

static void put_fix12(buffer *b, fix12 v)
{
  int integer = fix12_integer(v);

  put_int(b, integer);

  put_char(b, '.');

  // Put fraction
  unsigned int frac = fix12_frac(v);
  frac *= 10000;
  // Round half up. See:
  // https://en.wikipedia.org/wiki/Rounding#Rounding_to_the_nearest_integer
  frac += fix12_scale >> 1;
  frac /= fix12_scale;

  char buf[4];

  for (int i = 0; i < 4; i++) {
    buf[i] = (frac % 10) + '0';
    frac /= 10;
  }

  // Print decimal digts, skipping trailing zeroes
  int last_digit = 0;

  for (int i = 0; i < 4; i++) {
    last_digit = i;

    if (buf[i] != '0')
      break;
  }

  for (int i = 3; i >= last_digit; i--)
    put_char(b, buf[i]);
}

int snprintf(char *out, int len, const char *fmt, ...)
{
    int count;

    va_list ap;
    va_start(ap, fmt);
    count = vsnprinf(out, len, fmt, ap);
    va_end(ap);

    return count;
}

int vsnprinf(char *out, int len, const char *fmt, va_list ap)
{
  union data {
    char c;
    int i;
    unsigned int u;
    char *s;
    fix12 q;
    void *p;
  } d;

  char f = *fmt++;
  buffer b;

  buffer_init(&b, out, len);

  while (f != '\0') {
    if (f == '%') {
      f = *fmt++;
      if (f != '\0') {
        switch (f) {
        case 'c':
          d.c = va_arg(ap, int);
          put_char(&b, d.c);
          break;
        case 's':
          d.s = va_arg(ap, char *);
          put_string(&b, d.s);
          break;
        case 'u':
          d.u = va_arg(ap, unsigned int);
          put_uint(&b, d.u);
          break;
        case 'i':
          d.i = va_arg(ap, int);
          put_int(&b, d.i);
          break;
        case 'x':
          d.u = va_arg(ap, unsigned int);
          put_hex(&b, d.u);
          break;
        case 'p':
          d.p = va_arg(ap, void *);
          put_pointer(&b, d.p);
          break;
        case 'q':
          d.q = va_arg(ap, fix12);
          put_fix12(&b, d.q);
          break;
        default:
          put_char(&b, '%');
          put_char(&b, f);
          break;
        }
      }
    } else {
      put_char(&b, f);
    }

    f = *fmt++;
  }

  put_terminator(&b);

  return buffer_length(&b);
}

