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

DTCM_START = 0x027c0060;
/* CAUTION!! This value is actually the top of the user stack. So it should */
/* be set to an even lower value.                                           */
DTCM_END = 0x27C3B80;
DTCM_SIZE = DTCM_END - DTCM_START;

ITCM_START = 0x01ff8620;
ITCM_END = 0x02000000;
ITCM_SIZE = ITCM_END - ITCM_START;


MEMORY
{
    ram_text : ORIGIN = TEXT_START, LENGTH = TEXT_SIZE
    ram_data : ORIGIN = DATA_START, LENGTH = DATA_SIZE
    ram_bss : ORIGIN = BSS_START, LENGTH = BSS_SIZE
    itcm_mem : ORIGIN = ITCM_START, LENGTH = ITCM_SIZE
    dtcm_mem : ORIGIN = DTCM_START, LENGTH = DTCM_SIZE
    /* This is just a dummy memory region to simplify managing      */
    /* this script. In particular for overlay files. All section    */
    /* definitions should set the LMA region to: AT>dummy           */
    dummy : ORIGIN = 0x04000000, LENGTH = 0x0c000000
}

SECTIONS
{
    /* Example(s) of how to binary patch the firmware               */

    /* Patch CRT0 to call our new main function                     */
    crt0_main_fcn_patch CRT0_main_fcn : {
        *(text_main_fcn_ref)
    } AT>dummy

    main_heap_patch MAIN_area_start : {
        LONG(HEAP_START);
    } AT>dummy

    /* TCM patches                                                    */
    /* Allocate 4Kb to DTCM memory area. The rest is left for stack   */
    /* usage by the firmware code. See nitro/heap.h for more details. */

    dtcm_area_size_patch DTCM_area_size : {
        LONG(0xfffff000)
    } AT>dummy
    dtcm_area_size_2_patch DTCM_area_size_2 : {
        LONG(0xfffff000)
    } AT>dummy

    dtcm_area_start_patch DTCM_area_start : {
        LONG(DTCM_HEAP_START)
    } AT>dummy

    dtcm_area_start_2_patch DTCM_area_start_2 : {
        LONG(DTCM_HEAP_START)
    } AT>dummy

    dtcm_area_start_3_patch DTCM_area_start_3 : {
        LONG(DTCM_HEAP_START)
    } AT>dummy

    text_tcm_extend : {
        src/tcm.o(.text .text*)
        . = ALIGN(4);
    } >itcm_mem AT>dummy

    data_tcm_extend : {
        src/tcm.o(.data)
        src/tcm.o(.rodata .rodata*)
        . = ALIGN(4);
    } >dtcm_mem AT>dummy

    bss_tcm_extend : {
        src/tcm.o(.bss .bss* COMMON)
        . = ALIGN(4);
    } >dtcm_mem AT>dummy

    /* All other code and data go to MAIN RAM                      */

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

    HEAP_START = BSS_END;
    DTCM_HEAP_START = DTCM_START + SIZEOF(data_tcm_extend) + SIZEOF(bss_tcm_extend);
}
