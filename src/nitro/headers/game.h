/*
 * This header is used for defining functions and data that is a part of the
 * Tetris DS game itself. If you want to experiment with the game logic and
 * not the SDK, include this header.
 */
#ifndef GAME_INCLUDE_FILE
#define GAME_INCLUDE_FILE

#include <stdbool.h>

#include "file.h"

/**
 * Main. This is the games entry point.
 */
void game_main(void);

/**
 * Game init function.
 */
void game_init(void);


// --- Tetris game heap functions below ----

void game_init_heaps(void);

/**
 * Allocate memory from the main memory ie. area 0
 *
 * NOTE: Calls ndk_area_alloc_mem(0, -1, size)
 *
 * @param size
 * @return the allocated memory block
 */
void *game_alloc(int size);

/**
 * Free memory from the main memory ie. area 0
 *
 * NOTE: Calls ndk_area_free_mem(0, -1, mem)
 *
 * @param mem
 */
void game_free(void *mem);

// --- Tetris game file functions and data below ----

void game_init_filesystem(void);

/*
 * This is where FAT and FNT tables are cached.
 *
 * The game calls ndk_fat_cache_file_tables with a reference to this
 * memory.
 *
 * Size was estimated using the info in the ROM header and the size calculation
 * code in the ndk_fat_cache_file_tables function.
 */
extern char fs_fat_volume_cache[0x2dc0];

/**
 * Load all data from a file.
 *
 * NOTE: This function allocates memory for the file data from the main heap.
 *
 * NOTE: This function probably expects a certain type of file format.
 *
 * @param h file handle of an opened file
 * @param size will contain the size of the data buffer
 * @return pointer to the data buffer
 */
void *ndk_load_file(struct file *h, int *size);

/**
 * Load a LZ77 compressed file into main memory.
 *
 * NOTE: This function allocates memory for the file data from the main heap.
 *
 * @param filename
 * @param[out] size pointer where the file size in memory is written.
 * @return pointer to the file data
 */
void *ndk_load_LZ77_compressed_file(char *filename, int *size);


// --- Tetris game sound functions and data below ----

/**
 * Sound queues below are used by the Tetris game.
 */

/** Used for music playback */
extern struct sound_handle sound_handle_1;
/** Used for SFX like button press sounds etc ... */
extern struct sound_handle sound_handle_2;

extern struct sound_handle sound_handle_3;

extern struct sound_sdat_arch sound_sdat_state;

/**
 * This memory area is used for the SDAT sound heap Section BSS
 */
extern char sound_sdat_data_buffer[0x80000];

/**
 * This memory location holds a refrence to the sound heap after it has been
 * initialized.
 *
 * Section BSS
 */
extern struct sound_sdat_heap *sound_sdat_data_heap;

/**
 * This function calls ndk_sound_init and opens the file '/sound_data.sdat'.
 */
void game_sound_init_and_open_sdat_archive(void);

/**
 * NOTE: Don't call this function directly it's defined here only for
 * reference.
 */
void game_commit_sounds(void);

/*
 * Load group 0 from the SDAT archive
 */
void game_load_sound_sfx(void);

/**
 * Called with unk0 := 0x7f
 */
void ndk_sound_something_2(int unk0);

void ndk_sound_something_3(int unk0);

// --- End Tetris game sound functions and data ----

// ---- Tetris game keypad functions below ----

/**
 * Both variables have the format below.
 *
 * bit 0     Button A        (1=Pressed, 0=Released)
 * bit 1     Button B        (etc.)
 * bit 2     Select          (etc.)
 * bit 3     Start           (etc.)
 * bit 4     Right           (etc.)
 * bit 5     Left            (etc.)
 * bit 6     Up              (etc.)
 * bit 7     Down            (etc.)
 * bit 8     Button R        (etc.)
 * bit 9     Button L        (etc.)
 * bit 10    Button X        (etc.)
 * bit 11    Button Y        (etc.)
 * bit 13    PEN             (etc.)
 */
extern volatile unsigned short button_state_old;

/**
 * This memory location has the same bit layout
 * as button_state_old but their value have the
 * following meaning:
 *    1 = Button down action detected
 *    0 = No button down action detected
 */
extern volatile unsigned short buttons_pressed;

/**
 * This function will read the keypad input registers/data
 * and leave the result in the button_state_old and
 * buttons_pressed memory locations.
 *
 * NOTE: This function must be called before the memory
 * locations are read.
 */
void game_read_key_presses(void);

// ---- End Tetris game keypad functions ----

#endif // GAME_INCLUDE_FILE