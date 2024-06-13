/**
 * CRT0 and entry
 */
#ifndef CRT0_INCLUDE_FILE
#define CRT0_INCLUDE_FILE

/**
 * For compressed firmware binaries (this includes overlays) this structure
 * is appended to the end of the compressed binary data so that it can be
 * decompressed by the ndk_decompress_firmware_lz function.
 */
struct fw_lz_footer {
  /**
   * Offset to where the first uncompressed byte is to be written. It's relative
   * to the end of this structure.
   */
  unsigned int top_offset;
  unsigned int compressed_size: 24;
  /**
   * Offset to the start of the compressed data
   */
  unsigned int start_offset: 8;
};

/**
 * These two memory locations hold the addresses that define
 * the BSS section of the main ARM9 binary. They are used
 * by the crt0 to initialize the section with zeroes.
 *
 * NOTE: The overlay region immediately follows the BSS
 * section so bss_end equals overlay_start.
 *
 * NOTE: To alter the size of the BSS section just patch
 * the binary at bss_end with a new address.
 */
extern void *bss_start;
extern void *bss_end;

/**
 * This address holds the address of the games main function.
 * To have it point to a custom function just patch the binary
 * with the new address. The crt0 code uses an bx instruction
 * to transfer control to the main function so it supports thumb
 * code as well.
 */
void (*CRT0_main_fcn)(void);

/**
 * This memory location holds the address to the static initializer array.
 *
 * NOTE: The linker script currently does not support static initializer
 * functions.
 *
 * NOTE: See: https://stackoverflow.com/questions/15265295/understanding-the-libc-init-array
 * https://stackoverflow.com/questions/32700494/executing-init-and-fini
 */
extern void (*(*static_initializer_array)[])(void);

/**
 * See: https://en.wikipedia.org/wiki/Crt0 for a brief explanation.
 */
void crt0_entry(void);

/**
 * Decompress LZ compressed data inplace in memory. Used to decompress the
 * ARM9 binary and overlays.
 *
 * @param comp pointer to the end of the compressed buffer.
 */
void ndk_decompress_firmware_lz(unsigned char *comp);

#endif // CRT0_INCLUDE_FILE
