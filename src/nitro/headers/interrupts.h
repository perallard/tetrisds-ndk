/**
 * Interrupt handling, IME, IE and IF
 *
 * A note on interrupt priorities. The lower the bit position the higher the
 * priority.
 *
 * See https://problemkaputt.de/gbatek.htm#dsiomaps IE 4000210h and
 * IF 4000214h registers.
 */
#ifndef INTERRUPTS_INCLUDE_FILE
#define INTERRUPTS_INCLUDE_FILE

#include <stdbool.h>

/**
 * Type for custom interrupt request handlers.
 */
typedef void ndk_irq_handler_fn(void);

/**
 * Used by ndk_irq_callback_handler_impl to get the correct IRQ bit for a set
 * callback.
 *
 * Don't use this value directly. It's defined here only for reference.
 */
extern unsigned short irq_callback_bit_pow[8];

/**
 * This structure holds callbacks set by the ndk_irq_set_timer_callback and
 * ndk_irq_set_dma_callback functions.
 *
 * Don't use these values directly. It's defined here only for reference.
 */
struct irq_callback
{
  void (*callback)(void *);
  /*
   * if 0 clear the IRQ enable bit for this callback after it has been called.
   */
  int keep_irq;
  void *data;
};

/*
 * Entries 0-3 hold DMA callbacks and entries 4-7 hold timer callbacks.
 */
extern struct irq_callback irq_callbacks[8];

/**
 * Initialize the interrupt sub-system.
 *
 * NOTE: It's called from ndk_platform_init so there is no need to call it
 * directly.
 */
void ndk_irq_init(void);

/**
 * Get the handler function for a IRQ source.
 *
 * @param irq_mask see IE register (4000210h)
 * @return the handler
 */
ndk_irq_handler_fn *ndk_irq_get_handler(unsigned int irq_mask);

/**
 * Set IRQ handler
 *
 * Set a function to handle one or more interrupt sources. Which sources is
 * defined by the set bits in the irq_mask, see IE register (4000210h).
 *
 * NOTE: If the sources are DMA0-3 or Timer0-3 then the handler will be
 * converted to a irq_callback entry with data = 0 and keep_irq = 1. Also note
 * that IRQs for the DMA0-3 and Timer0-3 are *not* automatically enabled by this
 * function.
 *
 * The default DMA and Timer IRQ handlers seem to be critical for the operation
 * of the SDK so it can't be overridden normally.
 *
 * NOTE: For all other IRQ sources to be compatible with the
 * ndk_thread_wait_irq function i.e. sleep and awake on IRQ events the handler
 * function *must* set the corresponding bit in the thread_irq_bits memory
 * location to one. See dtcm.h. Here follows a minimal example of a V-Blank
 * interrupt handler:
 *
 * void vblank_handler(void)
 * {
 *   // set the mirrored bit. Will be read by ndk_thread_wait_irq
 *   thread_irq_bits |= 1;
 * }
 *
 * Before calling this function IRQs should be disabled by first calling
 * ndk_cpu_diable_irq.
 *
 * @param irq_mask
 * @param handler
 */
void ndk_irq_set_handler(unsigned int irq_mask, ndk_irq_handler_fn *handler);

/**
 * Acknowledge IF irq request
 *
 * Acknowledge one or more interrupt sources. Which sources is defined by the
 * set bits in ack_mask, See IF register (4000214h)
 *
 * return value of IF before the acknowledge
 */
unsigned int ndk_irq_acknowledge_request(unsigned int ack_mask);

/**
 * Disable IE IRQs
 *
 * Disable one or more IRQ sources. Which source is defined by the set bits in
 * irq_mask, See IE register (4000210h)
 *
 * @param irq_mask
 * @return old IE
 */
unsigned int ndk_irq_disable_interrupt_sources(unsigned int irq_mask);

/**
 * Enable IE IRQs
 *
 * Enable one of more IRQ sources. Which source is defined by the set bits in
 * irq_mask, See IE register (4000210h)
 *
 * @param irq_mask
 * @return old IE
 */
unsigned int ndk_irq_enable_interrupt_sources(unsigned int irq_mask);

/**
 * Set IE IRQs
 *
 * Write IE register (4000210h)
 *
 * @param irq_mask
 * @return old IE
 */
unsigned int ndk_irq_write_interrupt_enable_register(unsigned int irq_mask);

/**
 * Yield the current thread until an IRQ is triggered.
 *
 * Same as ndk_cpu_halt_and_wake_on_irq but it's thread safe. The current
 * thread will yield until the IRQ happens. Meanwhile other threads continue to
 * run.
 *
 * Any IRQ sources that should work with this function must set the correct bit
 * in the 'thread_irq_bits' memory location in its handler.
 *
 * NOTE: This function supports listening to multiple interrupt sources. Just
 * set the wanted bits in the irq_mask argument.
 *
 * NOTE: When a interrupt happens the current thread will be preempted by the
 * next pending thread in the priority list.
 *
 * @param clear
 *        true: Clear the current IRQ state i.e. wait for the next event.
 *        false: Don't clear the current IRQ state.
 * @param irq_mask See the Interrupt Request Flags register - 4000214h
 */
void ndk_thread_wait_irq(bool clear, unsigned int irq_mask);

/**
 * Handler for DMA and Timer callbacks Only here for reference. Don't call it
 * directly.
 */
void ndk_irq_callback_handler_impl(void);

/**
 * Set callback for timer overflow.
 *
 * This function will enable IRQ for the specified timer. Once the callback has
 * been invoked it's removed from the list of callbacks. Note however that the
 * IRQ is *not* disabled again after the invocation.
 *
 * @param timer 0-3
 * @param cb callback function
 * @param data cb will be called with this as argument.
 */
void ndk_irq_set_timer_callback(int timer, void (*cb)(void *), void *data);

/**
 * Set callback for DMA done.
 *
 * This function will enable IRQ for the specificed DMA channel. Once the
 * callback has been invoked it's removed from the list of callbacks. Note
 * that the IRQ will only be disabled if it was disabled prior to this call.
 *
 * @param channel 0-3
 * @param cb callback function
 * @param data cb will be called with this id.
 */
void ndk_irq_set_dma_callback(int channel, void (*cb)(void *), void *data);

/**
 * Sleep the current thread until the next vblank IRQ
 *
 * NOTE: This function calls ndk_thread_wait_irq(true, IS_VBLANK)
 */
void ndk_wait_vblank_intr(void);

#endif // INTERRUPTS_INCLUDE_FILE
