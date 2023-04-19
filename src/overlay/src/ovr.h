#ifndef OVERLAY_INCLUDE_GUARD
#define OVERLAY_INCLUDE_GUARD

#define OVERLAY0_FAT_ID 0
#define OVERLAY1_FAT_ID 1
#define ITCM_OVERLAY_FAT_ID 2


// Definitions for ovr0.c
void _overlay0_entry(void);

// Definitions for ovr1.c
void _overlay1_entry(void);

// Definitions for ovr_itcm.c
int itcm1_fibonacci(int n);

#endif // OVERLAY_INCLUDE_GUARD
