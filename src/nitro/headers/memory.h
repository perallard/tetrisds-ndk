/**
 * Memory copy and fill functions.
 *
 * For all functions below it's the callers responsability that all memory
 * alignment requirements are met for source, destination and size parameters.
 *
 * To be compatible with the SDK when using your own DMA routines follow these
 * rules:
 *
 * 1. Always wait for the DMA enable control bit to become zero before a
 *    transfer is started.
 *
 * 2. Always put DMA register updates in an critical section.
 *
 * Example:
 *    while (DMA0CNT & 0x80000000)
 *        ;
 *
 *    int lock;
 *    ndk_thread_critical_enter(&lock);
 *
 *    DMA0SAD = src_addr;
 *    DMA0DAD = dst_addr;
 *    DMA0CNT = 0x80000000 | count;
 *
 *    ndk_thread_critical_leave(&lock);
 *
 * NOTE: Parts of this module also resides in ITCM.
 */
#ifndef MEMORY_INCLUDE_FILE
#define MEMORY_INCLUDE_FILE

/**
 * Copy memory DMA async.
 *
 * NOTE: This function resides in MAIN RAM so it's not guarantued to be
 * asynchronous unless all code/data is cached already.
 *
 * NOTE: This function will block if there are any ongoing transfers on the
 * choosen DMA channel. When blocked IRQs (CPSR) are disabled. So all threads
 * and IRQ handlers are blocked from executing.
 *
 * NOTE: Master interrupt must enabled for this function to work. See IME
 * register. DMA interrupts must also be enabled.
 *
 * @param channel 0-3 See DMA0-3
 * @param source
 * @param dest
 * @param size number of bytes to copy. Must be less than 2^21.
 * @param data parameter passed to the callback function
 * @param callback function that will be called when the transfer is complete.
 *        NULL is a valid argument, if no callback is needed.
 */
void ndk_memory_dma_32bit_copy_async(int channel, void *source,
                                     void *dest, int size, void *data,
                                     void (*callback)(void *));

/**
 * Fill memory DMA async.
 *
 * NOTE: This function resides in MAIN RAM so it's not guarantued to be
 * asynchronous unless all code/data is cached already.
 *
 * NOTE: This function will block if there are any ongoing transfers on the
 * choosen DMA channel. When blocked IRQs (CPSR) are disabled. So all threads
 * and IRQ handlers are blocked from executing.
 *
 * NOTE: Master interrupt must enabled for this function to work. See IME
 * register. DMA interrupts must also be enabled.
 *
 * @param channel 0-3 See DMA0-3
 * @param dest
 * @param pattern memory will be filled this 32bit wide pattern.
 * @param size number of bytes to fill. Must be less than 2^21.
 * @param data parameter passed to the callback function
 * @param callback function that will be called when the fill is complete.
 *        NULL is a valid argument, if no callback is needed.
 */
void ndk_memory_dma_32bit_fill_async(int channel, void *dest,
                                     unsigned int pattern, int size,
                                     void *data, void (*callback)(void *));

/**
 * 16bit DMA memory copy.
 *
 * NOTE: This function will wait for any ongoing transfers to finish
 * before starting the requested one.
 *
 * @param channel 0-3 See DMA0-3
 * @param source
 * @param dest
 * @param size number of bytes to copy. Must be less than 2^21.
 */
void ndk_memory_dma_16bit_copy(int channel, void *source, void *dest,
                               int size);

/**
 * 16bit DMA memory fill.
 *
 * NOTE: This function will wait for any ongoing transfers to finish
 * before starting the requested one.
 *
 * @param channel 0-3 See DMA0-3
 * @param dest
 * @param pattern memory will be filled this 16bit wide pattern.
 * @param size number of bytes to fill. Must be less than 2^21.
 */
void ndk_memory_dma_16bit_fill(int channel, void *dest,
                               unsigned short pattern, int size);

/**
 * 32bit DMA memory copy.
 *
 * NOTE: This function will wait for any ongoing transfers to finish
 * before starting the requested one.
 *
 * @param channel 0-3 See DMA0-3
 * @param source
 * @param dest
 * @param size number of bytes to copy. Must be less than 2^21.
 */
void ndk_memory_dma_32bit_copy(int channel, void *source, void *dest,
                               int size);

/**
 * 32bit DMA memory fill.
 *
 * NOTE: This function will wait for any ongoing transfers to finish
 * before starting the requested one.
 *
 * @param channel 0-3 See DMA0-3
 * @param dest
 * @param pattern memory will be filled this 32bit wide pattern.
 * @param size number of bytes to fill. Must be less than 2^21.
 */
void ndk_memory_dma_32bit_fill(int channel, void *dest,
                               unsigned int pattern, int size);

/**
 * Use CPU to perform memory copy and fill operations.
 *
 * @param size size of the memory region in bytes
 */
void ndk_memory_16bit_fill(unsigned short pattern, void *dest, int size);

void ndk_memory_16bit_copy(void *source, void *dest, int size);

void ndk_memory_32bit_fill(unsigned int pattern, void *dest, int size);

void ndk_memory_32bit_copy(void *source, void *dest, int size);

/**
 * Fast memory copy and fill functions.
 *
 * These functions use LDM and STM instuctions to reduce none sequential access
 * penalties. They read/write 8 words at at time. Any remaining words are moved
 * one by one.
 */
void ndk_memory_fast_32bit_fill(unsigned int pattern, void *dest, int size);

void ndk_memory_fast_32bit_copy(void *source, void *dest, int size);

/**
 * Fill memory with an 8bit pattern.
 *
 * NOTE: This function is alignment safe so it can be used to fill any memory
 * at any offset.
 *
 * @param dest
 * @param pattern
 * @param size
 */
void ndk_memory_fill(void *dest, unsigned char pattern, int size);

/**
 * Copy memory as an array of 8bit values.
 *
 * NOTE: This function is alignment safe so it can be used to copy from and to
 * any memory at any offset.
 *
 * @param source
 * @param dest
 * @param size
 */
void ndk_memory_copy(void *source, void *dest, int size);

// ----------------------------------------------------------------------------
// Decompressors
// ----------------------------------------------------------------------------

/**
 * NOTE: Decompressors for huffman and RLE does not seem to be present in the
 * firmware image.
 */
#define COMPRESSION_LZ 1
#define COMPRESSION_HUFFMAN 2
#define COMPRESSION_RLUNCOMP 3

/**
 * Confirmed by function at 0x020497b0
 *
 * Struct extracted from info at:
 * http://www.romhacking.net/documents/%5B469%5Dnds_formats.htm
 */
struct compressed_file_header
{
  unsigned int reserved : 4;
  unsigned int compression_type : 4;
  unsigned int decompressed_size : 24;
};

/**
 * Decompress to a buffer using LZ.
 *
 * The LZ compression algorithm is a version of LZSS. See here:
 * https://www.nesdev.org/wiki/Tile_compression
 *
 * @param comp pointer to the compressed data
 * @param dest pointer to the output buffer
 */
void ndk_memory_decompress_lz(unsigned char *comp, unsigned char *dest);

#endif // MEMORY_INCLUDE_FILE
