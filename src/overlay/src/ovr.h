#ifndef OVERLAY_INCLUDE_GUARD
#define OVERLAY_INCLUDE_GUARD

#define OVERLAY0_FAT_ID 0
#define OVERLAY1_FAT_ID 1
#define ITCM_OVERLAY_FAT_ID 2
#define DTCM_OVERLAY_FAT_ID 3

typedef int fix12;

// Definitions for ovr0.c
void _overlay0_entry(void);

// Definitions for ovr1.c
void _overlay1_entry(void);

// Definitions for ovr_tcm.c
int tcm1_fibonacci(int n);
fix12 tcm1_sin(fix12 a);
fix12 tcm1_cos(fix12 a);

#endif // OVERLAY_INCLUDE_GUARD
