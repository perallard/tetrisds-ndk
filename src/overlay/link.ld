OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

TEXT_START = 0x02026500;
TEXT_END = 0x0205a0d0;
TEXT_SIZE = TEXT_END - TEXT_START;

DATA_START = 0x0205f11c;
DATA_END = 0x0208d748;
DATA_SIZE = DATA_END - DATA_START;

BSS_START = 0x20a9d0c;
BSS_END = 0x0213a130;
BSS_SIZE = BSS_END - BSS_START;

/* Currently there is no support for compiling code/data to ITCM and DTCM. */
/* Overlays can be used to load executable code to ITCM and data to DTCM.  */

/* All memory is allocated as stack space. Use DTCM_area_size symbol to    */
/* specify its size                                                        */
DTCM_START = 0x027c0060;

ITCM_START = 0x01ff8620;
ITCM_END = 0x02000000;
ITCM_SIZE = ITCM_END - ITCM_START;

MEMORY
{
    ram_text : ORIGIN = TEXT_START, LENGTH = TEXT_SIZE
    ram_data : ORIGIN = DATA_START, LENGTH = DATA_SIZE
    ram_bss : ORIGIN = BSS_START, LENGTH = BSS_SIZE
    /* This is just a dummy memory region to simplify managing      */
    /* this script. In particular for overlay files. All section    */
    /* definitions should set the LMA region to: AT>dummy           */
    dummy : ORIGIN = 0x04000000, LENGTH = 0x0c000000
}

SECTIONS
{
    /* Example(s) of how to binary patch the firmware               */

    /* Patch CRT0 to call our new main function. Note it needs to   */
    /* be a pointer to our new main function.                       */
    crt0_main_fcn_patch CRT0_main_fcn : {
        *(text_main_fcn_ref)
    } AT>dummy

    main_heap_patch MAIN_area_start : {
        LONG(HEAP_START);
    } AT>dummy

    /* Allocate 4Kb to DTCM memory area. The rest is left for stack   */
    /* usage by the firmware code. See nitro/heap.h for more details. */
    dtcm_area_start_patch DTCM_area_size : {
        LONG(0xfffff000)
    } AT>dummy

    /* Section definitions for the overlays must come before the    */
    /* section definitions for the main binary because of how LD    */
    /* wildcard matching works.                                     */
    INCLUDE overlay.ld

    text_patch : {
        *(.text .text*)
        . = ALIGN(4);
    } >ram_text AT>dummy

    data_patch : {
        *(.data .data*)
        *(.rodata .rodata*)
        . = ALIGN(4);
    } >ram_data AT>dummy

    .bss BSS_START : {
        *(.bss .bss* COMMON)
        . = ALIGN(4);
    } >ram_bss AT>dummy

    OVERLAY_START = BSS_END;
    HEAP_START = OVERLAY_START + OVERLAY_RAM_SIZE;

    ITCM_OVERLAY_START = ITCM_START;
}
