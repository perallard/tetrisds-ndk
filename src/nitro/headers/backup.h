/**
 * Backup memory functions.
 *
 * This module is used to load and store data to persistent backup memory.
 *
 * NOTE: The Backup API seem to be a sub-system to the cart API as both operate
 * on the same cart_state structure.
 *
 * NOTE: Backup read and writes are handled by the ARM7 ie. IPC communication
 * is used.
 *
 * NOTE: The functions below depend on the cart_backup_state struct defined
 * in cart.h
 *
 * NOTE: From what i can tell there seem to be no alignment restrictions
 * for addesses and read/write lengths that one must adhered to then calling
 * the ndk_backup_command function. It's all handled internally by the backup
 * API.
 *
 * For more information about backup memory see:
 * https://problemkaputt.de/gbatek.htm#dscartridgebackup
 */
#ifndef BACKUP_INCLUDE_FILE
#define BACKUP_INCLUDE_FILE

#include <stdbool.h>

#include "cart.h"

/**
 * @brief Defines for all supported backup types
 */

#define BACKUP_SPEC_TYPE_NONE 0

#define BACKUP_SPEC_EEPROM_512 0x0901
#define BACKUP_SPEC_EEPROM_8K 0x0d01
#define BACKUP_SPEC_EEPROM_64K 0x1001

#define BACKUP_SPEC_FLASH_256K 0x1202
#define BACKUP_SPEC_FLASH_512K 0x1302
#define BACKUP_SPEC_FLASH_1024K 0x1402

#define BACKUP_SPEC_FRAM_8K 0x0d03
#define BACKUP_SPEC_FRAM_32K 0x0f03

/**
 * @brief Backup error code defines
 */
#define BACKUP_SUCCESS 0
// errors are non zero values


/**
 * @brief Convenience macros for backup operations.
 */

#define ndk_backup_read(src, dest, count) \
    ndk_backup_command((unsigned int)(src), (unsigned int)(dest), (count), \
                       NULL, 0, false, 6, 1, 0)

#define ndk_backup_read_async(src, dest, count, cb, arg) \
    ndk_backup_command((unsigned int)(src), (unsigned int)(dest), (count), \
                       (cb), (arg), true, 6, 1, 0)

#define ndk_backup_write(src, dest, count) \
    ndk_backup_command((unsigned int)(src), (unsigned int)(dest), (count), \
                       NULL, 0, false, 8, 10, 2)

#define ndk_backup_write_async(src, dest, count, cb, arg) \
    ndk_backup_command((unsigned int)(src), (unsigned int)(dest), (count), \
                       (cb), (arg), true, 8, 10, 2)

/**
 * @brief Initialize the backup sub-system.
 *
 * A note on backup type and sizes. The spec variable has the following format:
 *
 * bits 0-7: type
 * 0: none, 1: EEPROM, 2: FLASH, 3: FRAM
 *
 * bits 8-15: size
 * Size is encoded as 2^size bytes.
 *
 * This SDK version supports the following backup configurations:
 *
 * EEPROM 0.5K bytes
 * EEPROM   8K bytes
 * EEPROM  64K bytes
 *
 * FLASH  256K bytes
 * FLASH  512K bytes
 * FLASH 1024K bytes
 *
 * FRAM     8K bytes
 * FRAM    32K bytes
 *
 * The configurations where reversed out from the ndk_backup_set_type function.
 *
 * The game uses EEPROM 8K (spec = 0x0d01). The spec value seem to be hardcoded
 * in the game. Well when running in an emulator or from a flash card just use
 * the spec that best suits your needs.
 *
 * @param spec Backup memory specification
 */
void ndk_backup_init(unsigned short spec);

/**
 * @brief Execute a backup memory command.
 *
 * This function is used to load and store data to backup memory.  Calls to
 * this function with the following arguments where found in various places in
 * the game code:
 *
 * write data (
 *  RAM src addr, BACKUP dest addr, count, NULL, 0, false, 8, 10, 2
 * )
 *
 * write data async, no callback set (
 *  RAM src addr, BACKUP dest addr, count, NULL, 0, true, 8, 10, 2
 * )
 *
 * read data (
 *  BACKUP src addr, RAM dest addr, count, NULL, 0, false, 6, 1, 0
 * )
 *
 * NOTE: src and dest parameters are defined as 'unsigned int' because that
 * (unsigned) integer type is wide enough to also hold a pointer.
 *
 * NOTE: This function will block until any ongoing backup operations have
 * finished regardless if the operation was requested to be async or not.
 *
 * @param src if it's a read operation it will be a address in the backup
 *        memory and if it's a write operation it will be an address in RAM.
 * @param dest if it's a read operation it will be a address in RAM and if it's
 *        a write operation it will be a address in backup memory.
 * @param count number of bytes to read/write
 * @param cb callback called when the command is complete. NULL is a valid
 *        argument if no callback is needed
 * @param arg argument given to the callback function
 * @param async perform the operation asynchronously
 * @param unk7 IPC message ?
 * @param unk8 retries ?
 * @param unk9 mode 0:read, 2:write ?
 *
 * @return True if operation was successful, false otherwise. If async mode
 * was requested true is always returned. The get error information after the
 * command completes call ndk_backup_get_status or read it directly from
 * backup_state. See cart.h.
 */
bool ndk_backup_command(unsigned int src, unsigned int dest,
                        unsigned int count, void (*cb)(int), int arg,
                        bool async, int unk7, int unk8, int unk9);

/**
 * @brief Get the status of the last backup operation.
 *
 * @return BACKUP_SUCCESS on success a non-zero value otherwise.
 */
unsigned int ndk_backup_get_status(void);


/*========================= PRIVATE FUNCTIONS =================================
 * The heuristic for 'private' functions is that they are only called from
 * other functions in this module/logical unit. They are kept here for
 * reference so we don't end up reversing the same function over and over
 * again.
 */


/**
 * @brief Set backup memory type.
 *
 * Helper function, called from ndk_backup_init no need to call it directly.
 *
 * @param spec See ndk_backup_init
 */
void ndk_backup_set_type(unsigned short spec);

/**
 * @brief Worker function called by ndk_backup_command.
 *
 * Don't call it directly it's defined here only for reference.
 *
 * @param crt
 */
void ndk_backup_command_worker_fn(struct cart *crt);

#endif // BACKUP_INCLUDE_FILE
