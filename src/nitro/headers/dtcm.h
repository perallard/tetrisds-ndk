/**
 * DTCM
 *
 * TODO: Explain how this data is copied from the firmware in main RAM to
 * DTCM.
 *
 * NOTE: Except for the data defined below all DTCM memory is allocated for
 * stack usage. It's possible to alter this configuration, see DTCM_area_size
 * in heap.h
 */
#ifndef DTCM_INCLUDE_FILE
#define DTCM_INCLUDE_FILE

#include "interrupts.h"
#include "thread.h"

/**
 * If a thread is stopped using ndk_thread_wait_irq it will be added to this
 * list. All threads in this list will be scheduled to be executed after the
 * next IRQ. See itcm.h for more details.
 */
extern struct thread_list waiting_irq_thread_list;

/**
 * A vector containing handlers for all IRQ sources. To alter it call
 * ndk_irq_set_handler. They will be set to default values by the CRT0.
 */
extern ndk_irq_handler_fn *irq_handlers[16];

/**
 * This memory location should mirror the state of 4000214h - NDS9/NDS7 - IF -
 * 32bit - Interrupt Request Flags (R/W)
 *
 * NOTE: see ndk_irq_set_handler for more details in interrupts.h
 */
extern volatile unsigned int thread_irq_bits;

#endif // DTCM_INCLUDE_FILE
