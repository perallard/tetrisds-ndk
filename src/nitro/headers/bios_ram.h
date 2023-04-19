#ifndef BIOS_RAM_INCLUDE_FILE
#define BIOS_RAM_INCLUDE_FILE

/**
 * Memory initialized by BIOS/firmware a boot time.
 *
 * See: https://problemkaputt.de/gbatek.htm#biosramusage
 */

extern void *sys_memory_area_start_addresses[9];

extern void *sys_memory_area_end_addresses[9];

#define BOOT_MODE_NORMAL 0
#define BOOT_MODE_DOWNLOAD_PLAY 2

extern unsigned short boot_indicator;

#endif // BIOS_RAM_INCLUDE_FILE
