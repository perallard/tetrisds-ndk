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

MEMORY
{
    /* This is the memory for the Tetris games main function.       */
    /* Since my own main function(s) probably will be small. 644    */
    /* bytes is plenty of space so why not place mine there as      */
    /* well.                                                        */
    ram_main_fcn : ORIGIN = game_main, LENGTH = 644
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

    /* Example of how to patch the original main function and       */
    /* save a couple of hundred bytes of memory ;).                 */
    /*                                                              */
    /* Our main function must be defined with the attribute:        */
    /* __attribute__((section("text_main_fcn"), target("arm")))     */
    /* the original games main function location is defined in      */
    /* the MEMORY definition at the beginning of this file. The     */
    /* reason to have it defined there is that if our replacement   */
    /* function has a larger memory footprint that the original     */
    /* the linker will stop with a failure message.                 */
    main_fcn_patch : {
        *(text_main_fcn)
        . = ALIGN(4);
    } >ram_main_fcn AT>dummy

    main_heap_patch MAIN_area_start : {
        LONG(HEAP_START);
    } AT>dummy

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
}
