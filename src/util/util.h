#ifndef UTIL_INCLUDE_GUARD
#define UTIL_INCLUDE_GUARD

#include "nds.h"
#include "fix_math.h"

#define FIX_SCALE 12
#define F2X(f) (fix12)((f) * (1 << FIX_SCALE))

#define FIX_ANGLE_0 (0x0000)
#define FIX_ANGLE_90 (0x4000)
#define FIX_ANGLE_180 (0x8000)
#define FIX_ANGLE_270 (0xc000)

/**
 * Multiply two fix point values.
 */
inline fix12 fix_mul(fix12 a, fix12 b)
{
    return ((long long)a * b) >> FIX_SCALE;
}

/**
 * Calculate fix point cosine.
 *
 * @param alpha angle
 * @return Cosine
 */
static inline fix12 ndk_fix_cos(fix_angle alpha)
{
    return fix_trig_table[FIX_ANGLE_TO_TABLE_INDEX(alpha)].cos;
}

/**
 * Calculate fix point sine.
 *
 * @param alpha angle
 * @return Sine
 */
static inline fix12 ndk_fix_sin(fix_angle alpha)
{
    return fix_trig_table[FIX_ANGLE_TO_TABLE_INDEX(alpha)].sin;
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