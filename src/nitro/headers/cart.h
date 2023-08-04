/**
 * Low level cart functions.
 *
 * *Don't* use this API to read data from the ROM! This file is here to serve
 * as documentation only. The functions below don't actually work on real
 * hardware with some flash carts! Here's my matrix:
 *
 * cart                 status
 * M3DS Real              no
 * M3i Zero               no
 * 500in1 (Ace3DS+)       no
 * R4i Revolution 3DS     yes
 *
 * *But* note however that the file API works just fine on all the flash carts
 * in the matrix above.
 *
 * NOTE: Cart ROM data reads are handled on the ARM9 side i.e. no IPC
 * communication involved.
 */
#ifndef CART_INCLUDE_FILE
#define CART_INCLUDE_FILE

#include <stdbool.h>

#include "thread.h"

#define CART_BLOCK_SIZE 0x200
#define CART_THREAD_STACK_SIZE 0x400

struct cart;
struct backup;

typedef void (*cart_thread_fn)(struct cart *);

struct cart_read_buf;

struct cart_read_buf {
  // set to ndk_cart_cpu_read_data during cart initialization
  void (*read)(struct cart_read_buf *);  // 0x00
  unsigned int ROMCTRL;             // 0x04
  unsigned int something;           // 0x08
  char unk0[0x14];                  // 0x0c
  char data[CART_BLOCK_SIZE];       // 0x20
  // more?
};

/*
 * A reference to this object is sent to the ARM7 via IPC when a backup
 * command is performed.
 */
extern struct backup {
    // 0 : success
    int status;     // 0x00
    int unk1;       // 0x04
    int unk2;       // 0x08
    unsigned int src; // 0x0c
    // can be both src and dest?
    char *io_buffer; // 0x10
    // number of bytes to read/write in the next command.
    int count;      // 0x14
    // size of the backup memory
    int size;       // 0x18
    // all values below initialized to zero

    // max number of bytes that can be written in write command.
    int max_write_count; // 0x1c
    int page_size;  // 0x20
    int unk9;       // 0x24
    int unk10;      // 0x28
    int unk11;      // 0x2c
    int unk12;      // 0x30
    int unk13;      // 0x34
    int unk14;      // 0x38
    int unk15;      // 0x3c
    char unk16;     // 0x40
    int unk17;      // 0x44
    // 0x60
} cart_backup_state;

extern void *cart_thread_stack_top;

extern struct cart_read_buf cart_read_buffer;

/**
 * Used by low level cart routines.
 *
 * Used when transfering cart ROM and backup data. The worker thread used has
 * priority 4.
 *
 * size unknown
 */
extern struct cart
{
  struct backup *bak;    // 0x00
  int unk1;              // 0x04
  int unk2;              // 0x08
  int unk3;              // 0x0c
  struct thread_list unk4;  // 0x10
  int unk6;              // 0x18
  unsigned int src;      // 0x1c
  unsigned int dst;      // 0x20
  unsigned int count;    // 0x24
  int dma_channel;       // 0x28
  // IPC message: 8 : write to backup memory, 6 : read from backup memory
  int unk15;             // 0x2c
  // 10 for writes, 1 for reads
  int retries;           // 0x30
  // 0 : read, 2 : write
  int mode;              // 0x34
  void (*complete_cb)(int);  // 0x38
  int complete_cb_arg;       // 0x3c
  // Is set by ndk_cart_thread_set_handler_and_start
  cart_thread_fn handler;  // 0x40
  // This is the worker thread for the cart IO subsystem
  struct thread worker_thread;  // 0x44
  int unk16;             // 0xfc
  int unk17;             // 0x100
  struct thread *unk20;  // 0x104
  // Set to 4 by ndk_cart_init
  int worker_thread_priority;   // 0x108
  // if a transfer is alreay ongoing the threads gets parked here
  struct thread_list waiting; // 0x10c
  // bit 0: 1 == cart subsystem initialized
  // bit 1: ?
  // bit 2: 1 == transfer ongoing | busy
  // bit 3: 1 == handler set
  // bit 5:
  unsigned int flags;    // 0x114
  int unk18;             // 0x118
  int unk19;             // 0x11c
  char buffer[0x100];    // 0x120
  // more ?
} cart_state;

/**
 * Sets a handler function that is to be executed in ndk_cart_thread_fcn.
 * It also wakes the cart worker thread.
 *
 * NOTE: Most likely a helper function.
 */
void ndk_cart_thread_set_handler_and_start(cart_thread_fn handler);

/**
 * Initialize the cart subsystem.
 *
 * NOTE: Called from ndk_platform_init so there is no need to call it directly.
 */
void ndk_cart_init(void);

/**
 * Low level cart data read.
 *
 * NOTE: This function uses cart_state for its internal operation
 *
 * NOTE: For async reads all threads that have a higher priority than 4 will be
 * executed before the read thread is started.
 *
 * NOTE: This function is set as the cart read hook in fat_volume by the
 * ndk_fat_mount function.
 *
 * NOTE: This function will try use DMA to transfer all data to its destination
 * but it will fall back to CPU if a number of alignment requirements are not
 * met. See ndk_cart_dma_read_data for more details. Even when falling back to
 * CPU there will be alignment requirements see ndk_cart_cpu_read_data. Long
 * story short in the worst case data might be read by CPU using a double
 * buffer scheme.
 *
 * @param dma_channel 0-3 to use DMA or -1 to use CPU
 * @param src address in cart ROM
 * @param dst destination address in RAM
 * @param count number of bytes to read
 * @param cb function to called when the read is complete
 * @param cb_arg argument passed to cb
 * @param async if true perform a non-blocking read
 */
void ndk_cart_read(unsigned dma_channel, unsigned int src, void *dst,
                   unsigned int count, void (*cb)(int), int cb_arg,
                   bool async);

/**
 * This is the worker function that run in the cart thread.
 * 
 * It checks if a handler function has be set, see
 * ndk_cart_thread_set_handler_and_start. If it has been set executes it else
 * it yields, in an endless loop. Once it has yelded it is only awoken again
 * by a call to ndk_cart_thread_set_handler_and_start.
 */
void ndk_cart_thread_fcn(void);

/**
 * This function is set as IPC handler for channel 11.
 *
 * NOTE: Backup related?
 */
void ndk_cart_ipc_handler_fn(int unk0, int unk1, int unk2);

/**
 * Get the ROM chip ID.
 *
 * See: https://problemkaputt.de/gbatek.htm#dscartridgeheader for more details.
 *
 * @return cart ROM id
 */
unsigned int ndk_cart_get_rom_id(void);

/**
 * Read data from cart using CPU only.
 *
 * The following values in cart_state must be setup before a call to this
 * function:
 *   cart_state.src == ROM start address
 *   cart_state.dst == destination addres
 *   cart_state.count == number of bytes to read
 *
 * NOTE: If the src address is not aligned with 512 or dst is not aligned
 * with 4 or if count is less than 512 then double buffering will occur.
 * Causing a performace penalty.
 *
 * NOTE: If count is larger than 512 and src and dst meet the alignment
 * restrictions then only the last count % 512 bytes will be read using
 * double buffering.
 *
 * NOTE: This function is used by ndk_cart_read when any of the conditions
 * above are not met.
 *
 * NOTE: This is probably only a helper function to ndk_cart_read.
 *
 * @param tmp
 */
void ndk_cart_cpu_read_data(struct cart_read_buf *tmp);

/**
 * Read data from cart using DMA.
 *
 * This function will set up a cart transfer complete IRQ handler and start
 * a DMA read by calling ndk_cart_start_dma_read. The IRQ handler will in trun
 * check if more data needs to be transfered, if so start a new DMA read by
 * calling ndk_cart_start_dma_read again and again until there is no more bytes
 * to be transfered.
 *
 * NOTE: For the IRQ handler see ndk_cart_dma_read_complete_irq_handler.
 *
 * NOTE: Data is transfered in 512 byte blocks. While a block transfer is
 * ongoing the CPU is blocked from the memory bus and can't perform any
 * processing. But any interrupts that have higher priority than the card data
 * transfer completion interrupt will be executed in between data blocks. Thus
 * has a chance to run, but with a slight delay.
 *
 * NOTE: This is most likely a helper function to ndk_cart_read.
 *
 * The following values in cart_state must be setup before a call to this
 * function:
 *   cart_state.src == ROM start address
 *   cart_state.dst == destination addres
 *   cart_state.count == number of bytes to read
 *   cart_state.dma_channel == DMA channel to use
 *
 * NOTE: If dma_channel is a value higher than 3 then no DMA read will be
 * started.
 *
 * NOTE: If dst is not aligned with 4 or src is not aligned with 512 or count
 * is not a multiple of 512 or zero no DMA read will be started.
 *
 * NOTE: Used by ndk_cart_read.
 *
 * @param tmp pointer to a card_read_buf structure.
 * @return true if DMA data read was started, false otherwise.
 */
bool ndk_cart_dma_read_data(struct cart_read_buf *tmp);

/**
 * NOTE: Don't call this function directly it's defined here only for
 * reference.
 */
void ndk_cart_dma_read_complete_irq_handler(void);

/**
 * Read data from cart using DMA.
 *
 * Sets up a DMA channel to receieve data from the cart, sends a data read
 * command using ndk_cart_send_command finally it sets some other CARTROM
 * control bits. Once the cart is ready to send data the DMA transfer is
 * automatically started.
 *
 * The following values in cart_state must be setup before a call to this
 * function:
 *    cart_state.dma_channel == DMA channel to use
 *    cart_state.dst == destination addres
 *    cart_state.src = ROM start address
 *
 * NOTE: This is most likely a helper function to ndk_cart_dma_read_data.
 *
 * NOTE: The function will always read 512 bytes.
 *
 * NOTE: There are strict alignmet rules to follow when settings the dst and
 * src variables. The dst address must align to 4 and the src address must
 * align to 512.
 *
 * NOTE: Used by ndk_cart_read.
 */
void ndk_cart_start_dma_read(void);

/**
 * Send 8 byte command
 *
 * NOTE: see here for more details: https://problemkaputt.de/gbatek.htm#dscartridgeheader
 *
 * @param command the command
 */
void ndk_cart_send_command(unsigned long long command);

/**
 * Copy bufferd cart data to its destination
 *
 * NOTE: called from ndk_cart_cpu_read_data
 * NOTE: updates cart_state
 *
 * @param tmp pointer to a cart_read_buf structure.
 * @return true if there is more data to read from the cart, false otherwise.
 */
bool ndk_cart_copy_data_from_buffer_to_dst(struct cart_read_buf *tmp);

#endif // CART_INCLUDE_FILE
