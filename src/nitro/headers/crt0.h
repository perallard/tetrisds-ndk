/**
 * CRT0 and entry
 */
#ifndef CRT0_INCLUDE_FILE
#define CRT0_INCLUDE_FILE

/**
 * For compressed firmware binaries (this includes overlays) this structure
 * is appended to the end of the binary so that it can be decompressed by
 * ndk_decompress_firmware_LZ77.
 */
struct fw_lz77_footer {
  unsigned int top_offset;
  unsigned int compressed_size: 24;
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
 * See: https://en.wikipedia.org/wiki/Crt0 for a brief explanation.
 */
void crt0_entry(void);

/**
 * Decompress LZ77 compressed data inplace in memory. Used to decompress the
 * ARM9 binary and overlays.
 *
 * @param comp pointer to the end of the compressed buffer.
 */
void ndk_decompress_firmware_LZ77(unsigned char *comp);

#endif // CRT0_INCLUDE_FILE
