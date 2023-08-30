/**
 * Overlay functions.
 *
 * TODO: explain what overlays are used for.
 */
#ifndef OVERLAY_INCLUDE_FILE
#define OVERLAY_INCLUDE_FILE

#include <stdbool.h>

#include "file.h"

/**
 * Overlay file info. The overlay file table (OVT) is an array of these
 * structures.
 *
 * Note: overlay id is the index to the element in the overlay file table.
 *
 * flags:
 *  bit 0: 1 = compressed, 0 = not compressed
 *  bit 1: 1 ?
 */
struct overlay_info {
  int overlay_id;             // 0x0
  int ram_address;            // 0x4
  int ram_size;               // 0x8
  int bss_size;               // 0xc
  int init_array_start;       // 0x10
  int init_array_end;         // 0x14
  int fat_id;                 // 0x18
  int compressed_size:24;     // 0x1c
  int flags:8;                // 0x1c
  // 0x20
};

/**
 * Overlay file handle
 */
struct overlay {
  struct overlay_info info;   // 0x0
  int cpu;                    // 0x20
  int rom_start_offset;       // 0x24
  int rom_size;               // 0x28
  // 0x2c
};

/**
 * Load an overlay file into memory.
 *
 * This function calls ndk_overlay_open then ndk_overlay_read_into_ram
 * and finaly ndk_overlay_init_in_ram gets called. When this call returns
 * it's safe to call any function/entry point within the overlay.
 *
 * @param cpu 0=ARM9, ARM7 otherwise
 * @param id overlay file id
 * @return false on failure, true on success
 */
bool ndk_overlay_load(int cpu, int id);

/**
 * Unload an overlay in memory.
 * 
 * Calls ndk_overlay_fini_in_ram.
 *
 * @param cpu 0=ARM9, ARM7 otherwise
 * @param id overlay file id
 * @return false on failure, true on success
 */
bool ndk_overlay_unload(int cpu, int id);


/*========================= PRIVATE FUNCTIONS =================================
 * The heuristic for 'private' functions is that they are only called from
 * other functions in this module/logical unit. They are kept here for
 * reference so we don't end up reversing the same functions over and over
 * agin.
 */


/*
 * Open an overlay file for reading.
 * 
 * This initializes the overlay struct so it can be used by other functions.
 *
 * @param h pointer to a overlay handle struct
 * @param cpu 0=ARM9, ARM7 otherwise
 * @param id overlay file id
 *
 * return false on failure, true on success
 */
bool ndk_overlay_open(struct overlay *h, int cpu, int id);

/**
 * Executes the static initializers.
 * 
 * If the overlay is compressed it will be decompressed before they are
 * executed.
 *
 * @param h pointer to a overlay handle struct
 */
void ndk_overlay_init_in_ram(struct overlay *h);

/**
 * Executes the .fini array or dtor chain ???
 *
 * @param h pointer to a overlay handle struct
 */
void ndk_overlay_fini_in_ram(struct overlay *h);

/**
 * Read overlay data into memory.
 * 
 * After this function returns, call ndk_overlay_init_in_ram before the
 * overlay entry function is called. This function also initializes the BSS
 * section.
 *
 * @param h pointer to a overlay handle struct
 * @return false on failure, true on success
 */
bool ndk_overlay_read_into_ram(struct overlay *h);

/**
 * Initialize a fat_handle from overlay data.
 *
 * @param h pointer to a fat_handle
 * @param o pointer to a overlay
 */
void ndk_overlay_init_fat_handle(struct fat_handle *h, struct overlay *o);

/**
 * Returns the size of the overlay file in ROM.
 *
 * NOTE: If the overlay file is compressed it returns the compressed size.
 *
 * @param h pointer to a overlay handle struct
 * @return size in bytes
 */
int ndk_overlay_get_file_size(struct overlay *h);

#endif // OVERLAY_INCLUDE_FILE
