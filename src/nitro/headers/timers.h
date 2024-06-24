/**
 * Timer API.
 *
 * The reason for reversing this module was to resolve how the thread API was
 * dependet on timers.
 *
 * NOTE: ndk_thread_sleep is implemented using timer 0 and 1. So use only
 * timers 2 and 3 to implement your own timing logic.
 */
#ifndef TIMERS_INCLUDE_GUARD
#define TIMERS_INCLUDE_GUARD

#include <stdbool.h>

/**
 * Used by ndk_timers_is_initialized and ndk_init_timers
 *
 * Don't used it directly. It's defined here only for reference.
 */
extern unsigned short thread_timers_initialized;

/**
 * Query if the timers for the thread subsystem has been initialized.
 * 
 * @return status
 */
bool ndk_thread_timers_is_initialized(void);

/**
 * Read the current counter value for timer 0.
 *
 * Don't call it directly. It's defined here only for reference.
 *
 * @return the current value of timer 0
 */
unsigned int ndk_timer_read_timer0(void);

/**
 * Initialise timers for use by the thread API.
 *
 * If you plan to use the ndk_thread_sleep function in your game logic then
 * this function must be called as part of you startup initialization.
 */
void ndk_thread_init_timers(void);

#endif // TIMERS_INCLUDE_GUARD
