/**
 * Print formatted data to an output buffer.
 *
 * This modules main purpose is to give the terminal 'printf'-like support
 * without being dependet on libc. And also to support custom types that are
 * found in the NDS only, like the fixed point data type.
 */

#ifndef SNPRINTF_INCLUDE_FILE
#define SNPRINTF_INCLUDE_FILE

#include <stdarg.h>

int snprintf(char *out, int len, const char *fmt, ...);

int vsnprinf(char *out, int len, const char *fmt, va_list ap);

#endif // SNPRINTF_INCLUDE_FILE
