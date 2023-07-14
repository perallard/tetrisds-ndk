/**
 * FAT and file functions.
 *
 * NOTE: To see how file loading is performed at a lower level see cart.h
 *
 * See: https://problemkaputt.de/gbatek.htm##dscartridgenitroromandnitroarcfilesystems
 */
#ifndef FILE_INCLUDE_FILE
#define FILE_INCLUDE_FILE

#include <stdbool.h>

#include "thread.h"

#define FILE_SEEK_SET 0
#define FILE_SEEK_CURR 1
#define FILE_SEEK_END 2

/**
 * File allocation table entry.
 *
 * See: https://problemkaputt.de/gbatek.htm#dscartridgenitroromandnitroarcfilesystems
 */
struct fat_entry
{
  int ROM_start;
  int ROM_end;
};

/**
 * File name table entry structure.
 *
 * See: https://problemkaputt.de/gbatek.htm#dscartridgenitroromandnitroarcfilesystems
 */
struct fnt_entry;

/**
 * File handle structure.
 *
 * size: 72 bytes
 */
struct file
{
  int unk_0x00;              // 0x00
  int unk_0x04;              // 0x04
  struct fat_volume *volume; // 0x08
  /*
   * bit 0 : 0 == done, 1 == ongoing ?
   * bit 1 : ?
   * bit 2 : 0 == async, 1 == blocking
   * bit 6 : 1 == busy ?
   */
  unsigned int flags;         // 0x0c
  int unk_0x10;               // 0x10
  int unk_0x14;               // 0x14
  struct thread_list waiting; // 0x18
  int FAT_id;                 // 0x20
  int start_offset;           // 0x24
  int end_offset;             // 0x28
  int current_offset;         // 0x2c
  int dest;                   // 0x30
  int count;                  // 0x34
  /**
   * Will be end_offset - current_offset or count
   */
  int real_count; // 0x38
  int unk_0x3c;   // 0x3c
  int unk_0x40;   // 0x40
  int unk_0x44;   // 0x44
  // 0x48
};

/**
 * Handle for FAT entries.
 *
 * Used as an in/out argument to ndk_fat_get_fat_id_from_filename.
 *
 * NOTE: volume should always be equal to fat_volume (see below).
 */
struct fat_handle
{
  struct fat_volume *volume;
  int id;
};

/**
 * What DMA channel will be used for file transfer.
 *
 * Values 0-3 to for DMA or -1 to use the CPU.
 *
 * NOTE: Don't set this value directly. It's defined here only for reference.
 * To set the value call ndk_fat_mount.
 */
extern unsigned int fat_dma_channel;

/**
 * This data structure *is* the ROM file system volume.
 *
 * NOTE: Don't access it directly. It's defined here for reference only.
 *
 * size: unknown
 */
extern struct fat_volume
{
  char name[4];               // 'rom\0'
  int unk0;                   // 0x4
  int unk1;                   // 0x8
  struct thread_list pending; // 0xc
  int unk4;                   // 0x14
  int unk5;                   // 0x18
  int unk6;                   // 0x1c
  int unk7;                   // 0x20
  struct file *unk8;          // 0x24
  int unk9;                   // 0x28
  struct fat_entry *fat;      // 0x2c
  int fat_table_size;         // 0x30
  struct fnt_entry *fnt;      // 0x34
  int fnt_table_size;         // 0x38
  int fat_rom_offset;         // 0x3c
  int fnt_rom_offset;         // 0x40
  void *cache;                // 0x44
  /* Set to a function that calls ndk_cart_read in ndk_fat_mount */
  int (*fn_1)(int, int, int); // 0x48
  /* Set to a function that just returns 1 in ndk_fat_mount */
  int (*fn_2)(); // 0x4c
  /* Set to the same as fn_1 in ndk_fat_mount */
  int (*fn_3)(int, int, int);      // 0x50
  int (*fn_4)(struct file *, int); // 0x54
  int unk10;                       // 0x58
  // more ?
} fat_volume;

/* An array of file IO functions. Used by fcn.0200a3f0. What is important here
 * is the fact that some of them use the fn_X function in the fat_volume
 * structure. fcn.0200a3f0 seen to be of great importance as it's called from
 * all over.
 */
extern int (*file_fn_array[9])(struct file *);

/**
 * Init file system.
 *
 * Must be called before any other file operations are performed. Sets FS as
 * mounted and initializes the fat_volume structure.
 *
 * @param dma_number 0-3 to use DMA or -1 to use the CPU to transfer the data
 */
void ndk_fat_mount(unsigned int dma_number);

/**
 * Cache files tables in RAM for faster file accesses.
 *
 * NOTE: If size is smaller than the required cache size nothing will be
 * cached.
 *
 * NOTE: To get the total size needed to cache the file tables call this
 * function with size = 0.
 *
 * @param cache address where the file tables are to be stored
 * @param size size of the cache
 * @return total size of the cached tables.
 */
int ndk_fat_cache_file_tables(void *cache, int size);

/**
 * Is file system mounted
 */
bool ndk_fat_mounted();

/**
 * Init file handle.
 *
 * Must be called before it's passed as an argument to the other functions.
 *
 * @param h file handle
 */
void ndk_file_init_handle(struct file *h);

/**
 * Seek file.
 *
 * @param h file handle
 * @param offset can be a negative number
 * @param whence is one of
 *  FILE_SEEK_SET = 0
 *  FILE_SEEK_CUR = 1
 *  FILE_SEEK_END = 2
 *
 * @return false if whence is not one of the above else true if seek succeeds
 */
bool ndk_file_seek(struct file *h, int offset, int whence);

/**
 * Read data from file.
 *
 * NOTE: Calls ndk_file_read_impl with async = false
 *
 * NOTE: To read the entire file without knowing the file size set count to
 * MAX_INT.
 *
 * NOTE: This function is blocking and will only return when all bytes have
 * been read (or on failure). But it is interruptable so threads that wait for
 * events like IRQs of timeouts etc. can execute provided they have a priority
 * that is higher than 4.
 *
 * @param h the file handle
 * @param dest the memory location the read data is to be stored
 * @param count the number of bytes to read
 * @return number of bytes read or -1 if the read operation failed.
 */
int ndk_file_read(struct file *h, void *dest, int count);

/**
 * Internal implementation of file read.
 *
 * @param h the file handle
 * @param dest the memory location the read data is to be stored
 * @param count the number of bytes to read
 * @param async Continue to execute or block until all bytes are read.
 * @return number of bytes read or -1 if the read operation failed. In the async
 * case count is always returned.
 */
int ndk_file_read_impl(struct file *h, void *dest, int count, bool async);

/**
 * Open file.
 *
 * @param h file
 * @param filename
 * @return false if operation failed or true otherwise
 */
bool ndk_file_open(struct file *h, char *filename);

/**
 * Close file.
 *
 * @param h file
 */
void ndk_file_close(struct file *h);

/**
 * Get file FAT id from filename.
 *
 * Scans the FNT (File name table) for its corresponding FAT index.
 *
 * NOTE: The fat_handle must have its volume member set to a valid fat_volume
 * structure. Typically it will be fat_volume.
 *
 * @param[in] filename
 * @param[out] f if the operation succeeds it will contain the id
 * @return false if operation failed, true otherwise
 */
bool ndk_fat_get_fat_id_from_filename(struct fat_handle *f, char *filename);

/**
 * Open file by FAT id.
 *
 * @param h file handle
 * @param fs file system volume. Use a reference to fat_volume.
 * @param FAT id
 * @return false if operation failed or true otherwise
 */
bool ndk_file_open_by_fat_id(struct file *h, struct fat_volume *fs, int FAT_id);

/**
 * Open a range in cart ROM as a file.
 *
 * @param h file handle
 * @param fs file system volume. Use a reference to fat_volume.
 * @param start lower address in ROM
 * @param end upper address in ROM
 * @param unk unknown always -1
 * @return false if operation failed or true otherwise
 */
bool ndk_file_open_by_rom_range(struct file *h, struct fat_volume *fs, int start,
                                int end, int unk);

/**
 * Get file size.
 *
 * The file must have be opened prior to this call otherwise the result is
 * unpredictable.
 *
 * NOTE: This function was added here for convenience. It's not pressent in
 * this form anywhere in the ROM.
 *
 * @param h file handle
 * @return fiĺe size in bytes
 */
inline int ndk_file_size(struct file *h)
{
  return h->end_offset - h->start_offset;
}

#endif // FILE_INCLUDE_FILE
