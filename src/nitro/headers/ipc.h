/**
 * IPC/FIFO subsystem.
 *
 * The IPC API is only here for reference. It is used by the other APIs to
 * communicate with the ARM7 blob. Note the reversing of the API is far from
 * complete.
 */
#ifndef IPC_INCLUDE_FILE
#define IPC_INCLUDE_FILE

#include <stdbool.h>

/**
 * FIFO message format. Messages are one word.
 *
 * bit 31-6 message
 * bit 5    flag, unknown purpose
 * bit 4-0  FIFO channel 0-31
 *
 * channel 0 is not used?
 */
struct ipc_command {
  unsigned int channel : 5;
  unsigned int flag : 1;
  unsigned int message : 26;
};

/**
 * Default false (0)
 *
 * NOTE: used only by ndk_init_ipc should not be use directly
 */
extern short ipc_initialised;

/**
 * Bitmaps of allocated FIFO channels.
 *
 * NOTE: Don't use it directly instead use ndk_ipc_allocate_fifo_channel.
 *
 * index 0: allocated by ARM9
 * index 1: allocated by ARM7
 */
extern unsigned int ipc_allocated_channels[2];

/**
 * Vector of IPC recv FIFO not empty handler functions.
 *
 * NOTE: Don't set the handlers directly instead use
 * ndk_ipc_allocate_fifo_channel.
 *
 * Function signature is:
 *
 * @param channel 0-31
 * @param message, 26 bit value
 * @param flag, 1 or 0. unknown purpose
 */
extern void (*ipc_fifo_handlers[32])(int, int, int);

/**
 * Init IPC subsystem.
 *
 * NOTE: This function is called from ndk_platform_init. Don't call it
 * directly.
 */
void ndk_ipc_init();

/**
 * This is the receive message from the ARM7 function. It's an IRQ handler for
 * receive FIFO not empty signals.
 */
void ndk_ipc_irq_handler_recv_fifo_not_empty(void);

/**
 * Send a command to the ARM7.
 *
 * @param channel
 * @param message
 * @param flag
 *
 * return 0 if operation succeeded
 *        -1 if there is an FIFO error
 *        -2 if the send buffer is full
 */
int ndk_ipc_send_command(int channel, int message, int flag);

/**
 * Allocate FIFO channel.
 *
 * Register a function that will handle incomming data in the FIFO buffer on
 * the ARM9 side.
 *
 * @param channel 0-31
 * @param pointer to handler function or null to free the FIFO channel
 */
void ndk_ipc_allocate_fifo_channel(int channel,
                                   void (*handler)(int, int, int));

/**
 * Query if a FIFO channel is allocated.
 *
 * @channel 0-31
 * @param cpu  ARM9 | ARM7
 *
 * return 1 if true 0 otherwise
 */
bool ndk_ipc_is_fifo_channel_allocated(int channel, int cpu);

/**
 * Called from ndk_ipc_init.
 *
 * NOTE: Don't call this function directly, it's here only for reference.
 */
void ndk_ipc_init_impl(void);

#endif // IPC_INCLUDE_FILE
