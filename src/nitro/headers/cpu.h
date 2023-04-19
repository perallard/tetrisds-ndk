/**
 * CPU management functions.
 *
 * Some notes on CPU cache operations.
 *
 * 'Clean' applies to write-back data caches, and means that if the cache line
 * contains stored data that has not yet been written out to main memory, it is
 * written to main memory now, and the line is marked as clean.
 *
 * 'Invalidate' means that the cache line (or all the lines in the cache) is
 * marked as invalid. No cache hits can occur for that line until it is
 * re-allocated to an address. For write-back data caches, this does not
 * include cleaning the cache line unless that is also stated.
 *
 * 'Write buffer' is a block of high-speed memory whose purpose is to optimize
 * stores to main memory. When a store occurs, its data, address and other
 * details, for example data size, are written to the write buffer at high
 * speed. The write buffer then completes the store at main memory speed. This
 * is typically much slower than the speed of the ARM processor. In the
 * meantime, the ARM processor can proceed to execute further instructions at
 * full speed.
 *
 * See the ARM Architecture Reference Manual for more details.
 */
#ifndef CPU_INCLUDE_FILE
#define CPU_INCLUDE_FILE

/**
 * Enable IRQ.
 *
 * @return old value of CPSR & 0x80 i.e. old value of the I bit.
 */
int ndk_cpu_enable_irq(void);

/**
 * Disable IRQ.
 *
 * @return old value of CPSR & 0x80 i.e. old value of the I bit.
 */
int ndk_cpu_disable_irq(void);


/**
 * Get CPSR mode bits.
 *
 * @return CPSR mode bits
 */
int ndk_cpu_get_current_mode(void);

/**
 * Set CPSR I bit.
 *
 * NOTE: Only use as input to this function the return value from the
 * functions ndk_cpu_enable_irq, ndk_cpu_disable_irq or this function.
 *
 * @param value 0 or 0x80
 * @return old value of the I bit.
 */
int ndk_cpu_write_irq_flag(int value);

/**
 * Halt cpu and wake on interrupt.
 *
 * The CPU is paused as long as (IE AND IF)=0
 */
void ndk_cpu_halt_and_wake_on_irq(void);

/**
 * Wait for N clock cycles. Clock speed is 66MHz
 *
 * @param delay in clock cycles
 */
void ndk_cpu_wait_by_loop(unsigned int ticks);

/**
 * @brief Drain the CPU write buffer.
 *
 * Will make sure that all the entries in the processor write buffer are
 * written out to memory before the next instructin is executed.
 */
void ndk_cpu_drain_write_buffer(void);

/**
 * Invalidate the entire instruction cache.
 */
void ndk_cpu_invalidate_icache(void);

/**
 * Invalidate a region in the instruction cache.
 *
 * @param address the start of the region
 * @param size the size of the region
 */
void ndk_cpu_invalidate_icache_lines(void *address, int size);

/**
 * Invalidate the entire data cache.
 *
 * NOTE: This function does not clean the data cache.
 */
void ndk_cpu_invalidate_dcache(void);

/**
 * Invalidate a region in the data cache.
 *
 * @param address the start of the region
 * @param size the size of the region
 */
void ndk_cpu_invalidate_dcache_lines(void *address, int size);

/**
 * Clean a region in the data cache.
 *
 * @param address the start of the region
 * @param size the size of the region
 */
void ndk_cpu_clean_dcache_lines(void *address, int size);

/**
 * Clean and invalidate a region in the data cache.
 *
 * Drains the write buffer, cleans and invalidates data cache lines.
 *
 * @param address the start of the region
 * @param size the size of the region
 */
void ndk_cpu_clean_and_invalidate_dcache_lines(void *address, int size);

#endif // CPU_INCLUDE_FILE
