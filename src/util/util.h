#ifndef UTIL_INCLUDE_GUARD
#define UTIL_INCLUDE_GUARD

#include "nds.h"
#include "fix_math.h"

#define fix12_one (1 << 12)

/**
 * Multiply two fix point values.
 */
inline fix12 fix_mul(fix12 a, fix12 b)
{
    return ((long long)a * b) >> 12;
}

/**
 * Performance timer.
 * 
 * Counts bus cycles (33Mhz)
 */

inline void timer_start()
{
    *(unsigned int *)&TM3CNT_L = 0x00840000;
    *(unsigned int *)&TM2CNT_L = 0x00800000;
}

inline void timer_stop()
{
    *(unsigned int *)&TM3CNT_L = 0x00000000;
    *(unsigned int *)&TM2CNT_L = 0x00000000;
}

inline unsigned int timer_value()
{
    return TM3CNT_L << 16 | TM2CNT_L;
}

#endif // UTIL_INCLUDE_GUARD