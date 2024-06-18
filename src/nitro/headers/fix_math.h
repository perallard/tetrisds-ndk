/**
 * This module contains a mix of HW accelerated and SW implemented math
 * functions.
 *
 * NOTE: Unless you plan to use the functions in the gfx subsystem there is no
 * real reason to use this module at all. Just roll your own.
 */
#ifndef FIX_MATH_INCLUDE_FILE
#define FIX_MATH_INCLUDE_FILE

#define fix12_one (1 << 12)

/**
 * This is the main fixed point type. It's 32 bits wide with a 20 bit signed
 * integer part and a 12 bit fractional part.
 */
typedef int fix12;

#define fix_angle_0 (0x0000)
#define fix_angle_90 (0x4000)
#define fix_angle_180 (0x8000)
#define fix_angle_270 (0xc000)

/**
 * This type represents an angle.
 *
 * It is a 16 bit unsigned value that lets one divide a complete angular period
 * into 2^16 steps. To convince your self that you can perform arithmetic on
 * this type read up on modular arithmetic.
 *
 * Conversion to degrees or radians is also quite trivial. As is the reverse.
 *
 * It's the return type of the ndk_fix_atan2 function and the input values for
 * the ndk_fix_sin and ndk_fix_cos functions.
 */
typedef unsigned short fix_angle;

/**
 * Macro to convert from fix_angle to sine, cosine table index.
 */
#define FIX_ANGLE_TO_TABLE_INDEX(a) (((a) >> 4) & 4095)

/**
 * This type represents a value of sine or cosine in the table below.
 * It's 16 bits wide with a 4 bit signed integer part and a 12 bit fractional
 * part.
 */
typedef short fix12_short;

/**
 * Sine and cosine table.
 *
 * See ndk_fix_cos and ndk_fix_sin for example usage.
 *
 * Since it has 4096 entries using this table will give a precision of
 * 360/4096 degrees.
 *
 * The tables is actually two interleaved tables. All even entries are
 * sines and all odd entries are cosines.
 */
extern struct fx_trig_entry_t {
    fix12_short sin;
    fix12_short cos;
} fix_trig_table[4096];

/**
 * Lookup table used by ndk_fix_atan2
 */
extern fix_angle fix_angle_table[128];

/**
 * Perform integer modulus using the HW accelerator.
 *
 * @param number
 * @param denom
 * @return the modulus
 */
int ndk_mod(int number, int denom);

/**
 * Perform integer division using the HW accelerator.
 *
 * @param number
 * @param denom
 * @return the quotient
 */
int ndk_div(int number, int denom);

/**
 * Perfrom division on two fx values using the HW accelerator.
 *
 * NOTE: This function does not wait for the hardware divider to finish before
 * returning. Call ndk_fix_quotient to to get the result.
 *
 * @param number
 * @param denom
 */
void ndk_fix_div_async(fix12 number, fix12 denom);

/**
 * Compute the inverse of a fx value using the HW accelerator.
 *
 * NOTE: This function does not wait for the hardware divider to finish before
 * returning. Call ndk_fix_quotient to to get the result.
 *
 * @param value
 */
void ndk_fix_inverse_async(fix12 value);

/**
 * Reads the DIV_RESULT HW register and returns it as a fx value.
 *
 * NOTE: If the hardware divider is busy this function will block until the
 * it's done.
 *
 * @return
 */
fix12 ndk_fix_quotient(void);

/**
 * Reads the DIV_RESULT HW register and returns it as an 64bit signed value
 *
 * NOTE: If the hardware divider is busy this function will block until the
 * it's done.
 *
 * @return the quotient
 */
long long ndk_64bit_quotient();

/**
 * Compute the inverse of a fx value using the HW accelerator.
 *
 * @param value
 * @return the inverse
 */
fix12 ndk_fix_inverse(fix12 value);

/**
 * Perfrom division on two fx values using the HW accelerator.
 *
 * @param number
 * @param denom
 * @return the quotient
 */
fix12 ndk_fix_div(fix12 number, fix12 denom);

/**
 * Normalize a 3D vector.
 *
 * @param[in] v array to a three dimensional verctor.
 * @param[out] res where to put the normalized vector.
 */
void ndk_fix_normalize_vector(fix12 *v, fix12 *res);

/**
 * Calculate the 3D cross product of two fx vectors. Storing the result in res.
 *
 * @param u
 * @param v
 * @param[out] res
 */
void ndk_fix_cross_product(fix12 *u, fix12 *v, fix12 *res);

/**
 * Calculate the 3D dot product of two
 * fx vectors.
 *
 * @param u
 * @param v
 * @return The dot product of u and v
 */
fix12 ndk_fix_dot_product(fix12 *u, fix12 *v);

/**
 * Calculate the two argument arctangent. See:
 * https://en.wikipedia.org/wiki/Atan2
 *
 * NOTE: This function uses a small lookup table to calculate the angle. Where
 * the precisiton is 360/1024 of a degree.
 *
 * @param y Y coordinate in the plane
 * @param x X coordinate in the plane
 * @return Angle between the positive x-axis and the line from the origin to
 * the point at (x, y).
 */
fix_angle ndk_fix_atan2(fix12 y, fix12 x);

/**
 * Calculate fix point cosine.
 *
 * @param alpha angle
 * @return Cosine
 */
inline fix12 ndk_fix_cos(fix_angle alpha)
{
    return fix_trig_table[FIX_ANGLE_TO_TABLE_INDEX(alpha)].cos;
}

/**
 * Calculate fix point sine.
 *
 * @param alpha angle
 * @return Sine
 */
inline fix12 ndk_fix_sin(fix_angle alpha)
{
    return fix_trig_table[FIX_ANGLE_TO_TABLE_INDEX(alpha)].sin;
}

/**
 * Multiply two fix point values.
 *
 * This function should perhaps be placed in a fix_math_extend module.
 */
inline fix12 fix_mul(fix12 a, fix12 b)
{
    return ((long long)a * b) >> 12;
}

#endif // FIX_MATH_INCLUDE_FILE
