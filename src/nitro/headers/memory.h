/**
 * Memory copy and fill functions
 *
 * For asynchronous DMA copy and fill functions. If there are any ongoing
 * transfers on the choosen DMA channel this function will block until it's
 * finished before it starts the requested one.
 *
 * NOTE: Master interrupt must enabled for these functions to work. See IME
 * register. DMA interrupts must also be enabled.
 *
 * NOTE: When the function blocks to wait for an ongoing transfer IRQs (CPSR)
 * are disabled. So all threads and IRQ handlers are blocked from executing.
 *
 * NOTE: Parts of this module also resides in ITCM.
 */
#ifndef MEMORY_INCLUDE_FILE
#define MEMORY_INCLUDE_FILE

/**
 * Copy memory DMA async.
 *
 * @param channel 0-3 See DMA0-3
 * @param source
 * @param dest
 * @param size number of bytes to fill/copy. Must be less than 2^21.
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
 * @param channel 0-3 See DMA0-3
 * @param dest
 * @param pattern memory will be filled this 32bit wide pattern.
 * @param size number of bytes to fill/copy. Must be less than 2^21.
 * @param data parameter passed to the callback function
 * @param callback function that will be called when the transfer is complete.
 *        NULL is a valid argument, if no callback is needed.
 */
void ndk_memory_dma_32bit_fill_async(int channel, void *dest,
                                     unsigned int pattern, int size,
                                     void *data, void (*callback)(void *));

/**
 * DMA memory copy and fill functions.
 *
 * All functions are CPU polled. The functions will wait/block for any ongoing
 * transfers to finish before starting the requested one.
 */

/**
 * 16bit DMA memory copy.
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
 * @param channel 0-3 See DMA0-3
 * @param dest
 * @param pattern memory will be filled this 32bit wide pattern.
 * @param size number of bytes to fill. Must be less than 2^21.
 */
void ndk_memory_dma_32bit_fill(int channel, void *dest,
                               unsigned int pattern, int size);

/**
 * CPU memory copy and fill functions.
 *
 * @param size size of the memory region in bytes
 */
void ndk_memory_16bit_fill(unsigned short pattern, void *dest, int size);

void ndk_memory_16bit_copy(void *source, void *dest, int size);

void ndk_memory_32bit_fill(unsigned int pattern, void *dest, int size);

void ndk_memory_32bit_copy(void *source, void *dest, int size);

/**
 * Fast CPU memory copy and fill functions.
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
 * @param c
 * @param size
 */
void ndk_memory_fill(void *dest, unsigned char c, int size);

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
#define COMPRESSION_LZ77 1
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
 * Decompress to a buffer using LZ77
 *
 * @param comp pointer to the compressed data
 * @param dest pointer to the output buffer
 */
void ndk_memory_decompress_LZ77(unsigned char *comp, unsigned char *dest);

#endif // MEMORY_INCLUDE_FILE
