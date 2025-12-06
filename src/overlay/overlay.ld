
ovr0 OVERLAY_START : {
    _ovr0_start = .;
    src/ovr0.o(.text .text*)
    src/ovr0.o(.data)
    src/ovr0.o(.rodata .rodata*)
    . = ALIGN(4);
    _ovr0_end = .;
    _ovr0_bss_start = .;
    src/ovr0.o(.bss .bss* COMMON)
    . = ALIGN(4);
    _ovr0_bss_end = .;
} AT>dummy

ovr1 OVERLAY_START : {
    _ovr1_start = .;
    src/ovr1.o(.text .text*)
    src/ovr1.o(.data)
    src/ovr1.o(.rodata .rodata*)
    . = ALIGN(4);
    _ovr1_end = .;
    _ovr1_bss_start = .;
    src/ovr1.o(.bss .bss* COMMON)
    . = ALIGN(4);
    _ovr1_bss_end = .;
} AT>dummy

itcm1 ITCM_OVERLAY_START : {
    _itcm1_start = .;
    src/ovr_tcm.o(.text .text*)
    . = ALIGN(4);
    _itcm1_end = .;
    _itcm1_bss_start = .;
    . = ALIGN(4);
    _itcm1_bss_end = .;
} AT>dummy

dtcm1 DTCM_OVERLAY_START : {
    _dtcm1_start = .;
    src/ovr_tcm.o(.data)
    src/ovr_tcm.o(.rodata .rodata*)
    . = ALIGN(4);
    _dtcm1_end = .;
    _dtcm1_bss_start = .;
    src/ovr_tcm.o(.bss .bss* COMMON)
    . = ALIGN(4);
    _dtcm1_bss_end = .;
} AT>dummy

ovr_tbl 0x0 : {
    LONG(0);
    LONG(_ovr0_start);
    LONG(_ovr0_end - _ovr0_start);
    LONG(_ovr0_bss_end - _ovr0_bss_start);
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);    /* flags */

    LONG(1);
    LONG(_ovr1_start);
    LONG(_ovr1_end - _ovr1_start);
    LONG(_ovr1_bss_end - _ovr1_bss_start);
    LONG(0);
    LONG(0);
    LONG(1);
    LONG(0);    /* flags */

    LONG(2);
    LONG(_itcm1_start);
    LONG(_itcm1_end - _itcm1_start);
    LONG(_itcm1_bss_end - _itcm1_bss_start);
    LONG(0);
    LONG(0);
    LONG(2);
    LONG(0);    /* flags */

    LONG(3);
    LONG(_dtcm1_start);
    LONG(_dtcm1_end - _dtcm1_start);
    LONG(_dtcm1_bss_end - _dtcm1_bss_start);
    LONG(0);
    LONG(0);
    LONG(3);
    LONG(0);    /* flags */

} AT>dummy

OVERLAY_RAM_SIZE = MAX(SIZEOF(ovr0), SIZEOF(ovr1));

ITCM_OVERLAY_SIZE = SIZEOF(itcm1);

DTCM_OVERLAY_SIZE = SIZEOF(dtcm1);
