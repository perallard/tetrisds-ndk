/**
 * FAT and file functions.
 *
 * Some notes about the file API
 *
 * 1. The API can block the current thread for a number of reasons. Usually it
 * boils to two cases: file objects can only have one active operation on it at
 * one time. Second the cart layer only supports one read/write operation at one
 * time.
 *
 * 2. Using blocking CPU transfer is always interrupable. Higher priority
 * threads can preempt the read thread. Mount the FS with dma number -1 and
 * only use the blocking file read function to read data. This could be useful
 * in implementing file accesses that should be done in a background thread. But
 * you also always have the option to split accesses into many smaller parts to
 * get concurrency. I guess the only reason for CPU only cart IO is that you
 * need to allocate DMA channel for more important work.
 *
 * 3. Using DMA for file transfers are *not* quarantied to use DMA unless a
 * bunch of alignment constraints are met. See cart.h
 *
 * 4. Asynchronicity is only really possible using DMA transfers and the CPU
 * doing work from cache and TCM memory only, since DMA work blocks the CPU from
 * the memory bus. But thread switching will most likely block if main memory
 * can't be accessed, so getting that to work is probably tricky. And probably
 * of limited use.
 *
 * 5. The async option only make sense in conjunction with DMA transfers.
 *
 * Conclusion: Async file operation is almost useless as a feature IMHO.
 * Blocking DMA transfers might be useful, from a performance perspective, but
 * require some work and planning to get max performance. Since Flash carts have
 * custom (cart) IO implementations it will be basically imposible to tell in
 * the general case if DMA and async operation is even used at all. Add to this
 * the fact that SD cards also differ wildly in IO performance. Designing your
 * program to be very conservative regarding IO performance is probably a good
 * idea.
 *
 * NOTE: To see how file loading is performed by the SDK at a lower level see
 * cart.h
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
  // list of some sort
  int unk_0x00;              // 0x00
  int unk_0x04;              // 0x04
  struct fat_volume *volume; // 0x08
  /**
   * bit 0 : 0 == done, 1 == ongoing
   * bit 1 : ?
   * bit 2 : 0 == async, 1 == blocking
   * bit 6 : 0 == file handle owned/used by another thread
   */
  unsigned int flags;         // 0x0c
  /**
   * NOTE: Internal state information, no need to set it directly.
   *
   * What type of IO operation will be performed when using this file handle
   * 0 == read using ndk_cart_read (via file_fn_array)
   * 6 == open by FAT id
   * 7 == open by ROM range
   * 8 == close
   * 14 == no operation. Set by ndk_file_init_handle and ndk_file_close
   */
  int operation;              // 0x10
  /**
   * Error state
   *
   * NOTE: set by ndk_file_central_dispatch and by a common subfunction to file
   * open, read and close functions. This member variable can be used to check
   * if a read operation succeded or not in the async case.
   *
   * 0 == OK / SUCCESS
   * 2 == ?
   * 6 == ?
   */
  int error;                  // 0x14
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
  /* Set to a function that calls ndk_cart_read by ndk_fat_mount */
  int (*fn_1)(int, int, int); // 0x48
  /* Set to a function that just returns 1 in ndk_fat_mount */
  int (*fn_2)(); // 0x4c
  /* Set to the same as fn_1 in ndk_fat_mount */
  int (*fn_3)(int, int, int);      // 0x50
  int (*fn_4)(struct file *, int); // 0x54
  int unk10;                       // 0x58
  // more ?
} fat_volume;

/**
 * An array of file IO functions. Used by ndk_file_central_dispatch. What is
 * important here is the fact that some of them use the fn_X functions in the
 * fat_volume structure. ndk_file_central_dispatch seem to be of great
 * importance as it's called from all over.
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
 * NOTE: This function is blocks until all bytes have been read (or on failure).
 * For more instances where this function blocks see: ndk_file_read_impl.
 *
 * NOTE: To read the entire file without knowing the file size set count to
 * MAX_INT or call ndk_file_size
 *
 * NOTE: Calls ndk_file_read_impl with async = false
 *
 * @param h the file handle
 * @param dest the memory location the read data is to be stored
 * @param count the number of bytes to read
 * @return number of bytes read or -1 if the read operation failed.
 */
int ndk_file_read(struct file *h, void *dest, int count);

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

/*========================= PRIVATE FUNCTIONS =================================
 * The heuristic for 'private' functions is that they are only called from
 * other functions in this module/logical unit. They are kept here for
 * reference so we don't end up reversing the same function over and over agin.
 */

/**
 * Used to perform different types of IO operations.
 *
 * NOTE: There might be no need to call this function directly. It's defined
 * here only for reference.
 *
 * @param h
 * @param operation
 * @return status see file.error
 */
int ndk_file_central_dispatch(struct file *h, int operation);

/**
 * Internal implementation of file read.
 *
 * There are instances where this function will block even though the async
 * option is set to true. If the file object is currently used by another
 * thread, it will block until the file object becomes available again. Or if
 * the file object is owned by us but there is an ongoing file operation. This
 * case can happen if we try to start a read before a previous read has
 * finished.
 *
 * Example of checking if a file operation has finished in async mode:
 *
 *       struct file *h;
 *       // initialize file object, open file, ...
 *       // store old state
 *       int old_offset = h.current_offset;
 *
 *       int count = 68;
 *       // perform read
 *
 *       // check
 *       if (h.error == SUCCESS) {
 *          // check that we got all requested bytes
 *          if ((h.current_offset - old_offset) == count) {
 *              // all ok
 *          } else {
 *              // error
 *          }
 *       } else {
 *          // we need to wait some more, or error?
 *       }
 *
 * @param h the file handle
 * @param dest the memory location the read data is to be stored
 * @param count the number of bytes to read
 * @param async non-blocking operation or not. Non-blocking operation only makes
 * sense if the file system was initialized to use DMA.
 * @return number of bytes read or -1 if the read operation failed. In the async
 * case count is always returned.
 */
int ndk_file_read_impl(struct file *h, void *dest, int count, bool async);

#endif // FILE_INCLUDE_FILE
