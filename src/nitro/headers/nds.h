/**
 * IO and functions that do not yet have a module defined for them.
 *
 * See: https://problemkaputt.de/gbatek.htm#dsiomaps
 */
#ifndef NDS_INCLUDE_FILE
#define NDS_INCLUDE_FILE

#include <stdarg.h>
#include <stddef.h>

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 192

#define ARM9 0
#define ARM7 1

#define BUS_CLOCK 33513982
#define ARM9_CLOCK 67027964

/**
 * 2D graphics engine A registers
 */
#define DISPCNT     (*(volatile unsigned int *) 0x04000000)
#define DISPSTAT    (*(volatile unsigned short *) 0x04000004)
#define VCOUNT      (*(volatile unsigned short *) 0x04000006)
#define BG0CNT      (*(volatile unsigned short *) 0x04000008)
#define BG1CNT      (*(volatile unsigned short *) 0x0400000a)
#define BG2CNT      (*(volatile unsigned short *) 0x0400000c)
#define BG3CNT      (*(volatile unsigned short *) 0x0400000e)
#define BG0HOFS     (*(volatile unsigned short *) 0x04000010)
#define BG0VOFS     (*(volatile unsigned short *) 0x04000012)
#define BG1HOFS     (*(volatile unsigned short *) 0x04000014)
#define BG1VOFS     (*(volatile unsigned short *) 0x04000016)
#define BG2HOFS     (*(volatile unsigned short *) 0x04000018)
#define BG2VOFS     (*(volatile unsigned short *) 0x0400001a)
#define BG3HOFS     (*(volatile unsigned short *) 0x0400001c)
#define BG3VOFS     (*(volatile unsigned short *) 0x0400001e)
#define BG2PA       (*(volatile unsigned short *) 0x04000020)
#define BG2PB       (*(volatile unsigned short *) 0x04000022)
#define BG2PC       (*(volatile unsigned short *) 0x04000024)
#define BG2PD       (*(volatile unsigned short *) 0x04000026)
#define BG2X        (*(volatile unsigned int *) 0x04000028)
#define BG2Y        (*(volatile unsigned int *) 0x0400002c)
#define BG3PA       (*(volatile unsigned short *) 0x04000030)
#define BG3PB       (*(volatile unsigned short *) 0x04000032)
#define BG3PC       (*(volatile unsigned short *) 0x04000034)
#define BG3PD       (*(volatile unsigned short *) 0x04000036)
#define BG3X        (*(volatile unsigned int *) 0x04000038)
#define BG3Y        (*(volatile unsigned int *) 0x0400003c)
#define WIN0H       (*(volatile unsigned short *) 0x04000040)
#define WIN1H       (*(volatile unsigned short *) 0x04000042)
#define WIN0V       (*(volatile unsigned short *) 0x04000044)
#define WIN1V       (*(volatile unsigned short *) 0x04000046)
#define WININ       (*(volatile unsigned short *) 0x04000048)
#define WINOUT      (*(volatile unsigned short *) 0x0400004a)
#define MOSAIC      (*(volatile unsigned short *) 0x0400004c)
#define BLDCNT      (*(volatile unsigned short *) 0x04000050)
#define BLDALPHA    (*(volatile unsigned short *) 0x04000052)
#define BLDY        (*(volatile unsigned short *) 0x04000054)
#define DISP3DCNT   (*(volatile unsigned short *) 0x04000060)
#define DISPCAPCNT  (*(volatile unsigned int *) 0x04000064)
#define DISP_MMEM_FIFO (*(volatile unsigned int *) 0x04000068)
#define MASTER_BRIGHT (*(volatile unsigned short *) 0x0400006c)

/**
 * DMA see memory.h
 */
#define DMA0SAD (*(volatile unsigned int *) 0x040000b0)
#define DMA0DAD (*(volatile unsigned int *) 0x040000b4)
#define DMA0CNT (*(volatile unsigned int *) 0x040000b8)
#define DMA1SAD (*(volatile unsigned int *) 0x040000bc)
#define DMA1DAD (*(volatile unsigned int *) 0x040000c0)
#define DMA1CNT (*(volatile unsigned int *) 0x040000c4)
#define DMA2SAD (*(volatile unsigned int *) 0x040000c8)
#define DMA2DAD (*(volatile unsigned int *) 0x040000cc)
#define DMA2CNT (*(volatile unsigned int *) 0x040000d0)
#define DMA3SAD (*(volatile unsigned int *) 0x040000d4)
#define DMA3DAD (*(volatile unsigned int *) 0x040000d8)
#define DMA3CNT (*(volatile unsigned int *) 0x040000dc)

#define DMA0FILL (*(volatile unsigned int *) 0x040000e0)
#define DMA1FILL (*(volatile unsigned int *) 0x040000e4)
#define DMA2FILL (*(volatile unsigned int *) 0x040000e8)
#define DMA3FILL (*(volatile unsigned int *) 0x040000ec)

/**
 * Timers see timers.h
 */
#define TMOCNT_L (*(volatile unsigned short *) 0x04000100)
#define TMOCNT_H (*(volatile unsigned short *) 0x04000102)
#define TM1CNT_L (*(volatile unsigned short *) 0x04000104)
#define TM1CNT_H (*(volatile unsigned short *) 0x04000106)
#define TM2CNT_L (*(volatile unsigned short *) 0x04000108)
#define TM2CNT_H (*(volatile unsigned short *) 0x0400010a)
#define TM3CNT_L (*(volatile unsigned short *) 0x0400010c)
#define TM3CNT_H (*(volatile unsigned short *) 0x0400010e)

/**
 * Values for the buttons are 0=Pressed, 1=Released
 */
#define PAD_A       (1 << 0)
#define PAD_B       (1 << 1)
#define PAD_Select  (1 << 2)
#define PAD_Start   (1 << 3)
#define PAD_Right   (1 << 4)
#define PAD_Left    (1 << 5)
#define PAD_Up      (1 << 6)
#define PAD_Down    (1 << 7)
#define BTN_R       (1 << 8)
#define BTN_L       (1 << 9)

#define KEYINPUT      (*(volatile unsigned short *) 0x4000130)
#define KEYCNT        (*(volatile unsigned short *) 0x4000132)

#define EXMEMCNT      (*(volatile unsigned short *) 0x4000204)
#define IME           (*(volatile unsigned short *) 0x4000208)

#define IE_VBLANK                   (1 << 0)
#define IE_HBLANK                   (1 << 1)
#define IE_VCOUNT_MATCH             (1 << 2)
#define IE_TIMER_0                  (1 << 3)
#define IE_TIMER_1                  (1 << 4)
#define IE_TIMER_2                  (1 << 5)
#define IE_TIMER_3                  (1 << 6)
#define IE_DMA_0                    (1 << 8)
#define IE_DMA_1                    (1 << 9)
#define IE_DMA_2                    (1 << 10)
#define IE_DMA_3                    (1 << 11)
#define IE_KEYPAD                   (1 << 12)
#define IE_SLOT_2                   (1 << 13)
#define IE_IPC_SYNC                 (1 << 16)
#define IE_IPC_SEND_FIFO_EMPTY      (1 << 17)
#define IE_IPC_RECV_FIFO_NOT_EMPTY  (1 << 18)
#define IE_SLOT_1_TRANSFER_COMPLETE (1 << 19)
#define IE_SLOT_1_IREQ_MC           (1 << 20)
#define IE_GEOMETRY_COMMAND_FIFO    (1 << 21)

#define IE            (*(volatile unsigned int *) 0x4000210)
#define IF            (*(volatile unsigned int *) 0x4000214)

#define VRAMCNT_A     (*(volatile unsigned char *) 0x4000240)
#define VRAMCNT_B     (*(volatile unsigned char *) 0x4000241)
#define VRAMCNT_C     (*(volatile unsigned char *) 0x4000242)
#define VRAMCNT_D     (*(volatile unsigned char *) 0x4000243)
#define VRAMCNT_E     (*(volatile unsigned char *) 0x4000244)
#define VRAMCNT_F     (*(volatile unsigned char *) 0x4000245)
#define VRAMCNT_G     (*(volatile unsigned char *) 0x4000246)
#define WRAMCNT       (*(volatile unsigned char *) 0x4000247)
#define VRAMCNT_H     (*(volatile unsigned char *) 0x4000248)
#define VRAMCNT_I     (*(volatile unsigned char *) 0x4000249)

#define DIVCNT_MODE_32B_DIV_32B 0
#define DIVCNT_MODE_64B_DIV_32B 1
#define DIVCNT_MODE_64B_DIV_64B 2

#define DIVCNT        (*(volatile unsigned short *) 0x4000280)
#define DIV_NUMER     (*(volatile long long int *) 0x4000290)
#define DIV_DENOM     (*(volatile long long int *) 0x4000298)
#define DIV_RESULT    (*(volatile long long int *) 0x40002a0)
#define DIVREM_RESULT (*(volatile long long int *) 0x40002a8)
#define SQRTCNT       (*(volatile unsigned short *) 0x40002b0)
#define SQRT_RESULT   (*(volatile unsigned int *) 0x40002b4)
#define SQRT_PARAM   (*(volatile unsigned long long int *) 0x40002b8)

#define POWCNT        (*(volatile unsigned int *) 0x4000304)

/**
 * 3D engine registers
 */
#define RDLINES_COUNT (*(volatile unsigned short *) 0x4000320)
/**
 * Edge color table. A list of 8 colors. 2 colors per word at
 * consecutive addresses.
 */
#define EDGE_COLOR ((volatile unsigned int *) 0x4000330)

#define APLHA_TEST_REF (*(volatile unsigned short *) 0x4000340)

#define CLEAR_COLOR (*(volatile unsigned int *) 0x4000350)
#define CLEAR_DEPTH (*(volatile unsigned short *) 0x4000354)
#define CLEARIMAGE_OFFSET (*(volatile unsigned short *) 0x4000356)
#define FOG_COLOR (*(volatile unsigned int *) 0x4000358)
#define FOG_OFFSET (*(volatile unsigned short *) 0x400035c)
/**
 * Fog Density Table. A list of 32 density value. 4 values per word
 * at consecutive addresses.
 */
#define FOG_TABLE ((volatile unsigned short *) 0x4000360)
/**
 * Toon color table. A list of 32 colors. 2 values per word
 * at consecutive addresses.
 */
#define TOON_TABLE ((volatile unsigned int *) 0x4000380)

#define GXFIFO (*(volatile unsigned int *) 0x4000400)

/**
 * 3D engine command ports.
 */
#define MTX_MODE (*(volatile unsigned int *) 0x4000440)
#define MTX_PUSH (*(volatile unsigned int *) 0x4000444)
#define MTX_POP  (*(volatile unsigned int *) 0x4000448)
#define MTX_STORE (*(volatile unsigned int *) 0x400044c)
#define MTX_RESTORE (*(volatile unsigned int *) 0x4000450)
#define MTX_IDENTITY (*(volatile unsigned int *) 0x4000454)
#define MTX_LOAD_4x4 (*(volatile unsigned int *) 0x4000458)
#define MTX_LOAD_4x3 (*(volatile unsigned int *) 0x400045c)
#define MTX_MULT_4x4 (*(volatile unsigned int *) 0x4000460)
#define MTX_MULT_4x3 (*(volatile unsigned int *) 0x4000464)
#define MTX_MULT_3x3 (*(volatile unsigned int *) 0x4000468)
#define MTX_SCALE (*(volatile unsigned int *) 0x400046c)
#define MTX_TRANS (*(volatile unsigned int *) 0x4000470)

#define COLOR (*(volatile unsigned int *) 0x4000480)
#define NORMAL (*(volatile unsigned int *) 0x4000484)
#define TEXCOORD (*(volatile unsigned int *) 0x4000488)
#define VTX_16 (*(volatile unsigned int *) 0x400048c)
#define VTX_10 (*(volatile unsigned int *) 0x4000490)
#define VTX_XY (*(volatile unsigned int *) 0x4000494)
#define VTX_XZ (*(volatile unsigned int *) 0x4000498)
#define VTX_YZ (*(volatile unsigned int *) 0x400049c)
#define VTX_DIFF (*(volatile unsigned int *) 0x40004a0)

#define POLYGON_ATTR (*(volatile unsigned int *) 0x40004a4)
#define TEXIMAGE_PARAM (*(volatile unsigned int *) 0x40004a8)
#define TEXPLTT_BASE (*(volatile unsigned int *) 0x40004ac)

#define DIF_AMB (*(volatile unsigned int *) 0x40004c0)
#define SPE_EMI (*(volatile unsigned int *) 0x40004c4)
#define LIGHT_VECTOR (*(volatile unsigned int *) 0x40004c8)
#define LIGHT_COLOR (*(volatile unsigned int *) 0x40004cc)
#define SHININESS (*(volatile unsigned int *) 0x40004d0)

#define BEGIN_VTXS (*(volatile unsigned int *) 0x4000500)
#define END_VTXS (*(volatile unsigned int *) 0x4000504)
#define SWAP_BUFFERS (*(volatile unsigned int *) 0x4000540)
#define VIEWPORT (*(volatile unsigned int *) 0x4000580)

#define BOX_TEST (*(volatile unsigned int *) 0x40005c0)
#define POS_TEST (*(volatile unsigned int *) 0x40005c4)
#define VEC_TEST (*(volatile unsigned int *) 0x40005c8)

#define GXSTAT (*(volatile unsigned int *) 0x4000600)
#define LISTRAM_COUNT (*(volatile unsigned short *) 0x4000604)
#define VTXRAM_COUNT (*(volatile unsigned short *) 0x4000606)

#define DISP_1DOT_DEPTH (*(volatile unsigned short *) 0x4000610)

/**
 * Clip coordinates values (x, y, z, w) at consecutive addresses
 */
#define POS_RESULT ((volatile unsigned int *) 0x04000620)
/**
 * Directional vector values (x, y, z) of the view coordinate space
 * at consecutive addressses
 */
#define VEC_RESULT ((volatile unsigned short *) 0x04000630)

/**
 * 4x4 matrix of 32 bit values at consecutive addresses
 */
#define CLIPMTX_RESULT ((volatile unsigned int *) 0x04000640)

/**
 * 3x3 matrix of 32 bit values at consecutive addresses
 */
#define VECMTX_RESULT ((volatile unsigned int *) 0x04000680)

/**
 * 2D graphics engine B registers
 */
#define DB_DISPCNT     (*(volatile unsigned int *) 0x04001000)
#define DB_BG0CNT      (*(volatile unsigned short *) 0x04001008)
#define DB_BG1CNT      (*(volatile unsigned short *) 0x0400100a)
#define DB_BG2CNT      (*(volatile unsigned short *) 0x0400100c)
#define DB_BG3CNT      (*(volatile unsigned short *) 0x0400100e)
#define DB_BG0HOFS     (*(volatile unsigned short *) 0x04001010)
#define DB_BG0VOFS     (*(volatile unsigned short *) 0x04001012)
#define DB_BG1HOFS     (*(volatile unsigned short *) 0x04001014)
#define DB_BG1VOFS     (*(volatile unsigned short *) 0x04001016)
#define DB_BG2HOFS     (*(volatile unsigned short *) 0x04001018)
#define DB_BG2VOFS     (*(volatile unsigned short *) 0x0400101a)
#define DB_BG3HOFS     (*(volatile unsigned short *) 0x0400101c)
#define DB_BG3VOFS     (*(volatile unsigned short *) 0x0400101e)
#define DB_BG2PA       (*(volatile unsigned short *) 0x04001020)
#define DB_BG2PB       (*(volatile unsigned short *) 0x04001022)
#define DB_BG2PC       (*(volatile unsigned short *) 0x04001024)
#define DB_BG2PD       (*(volatile unsigned short *) 0x04001026)
#define DB_BG2X        (*(volatile unsigned int *) 0x04001028)
#define DB_BG2Y        (*(volatile unsigned int *) 0x0400102c)
#define DB_BG3PA       (*(volatile unsigned short *) 0x04001030)
#define DB_BG3PB       (*(volatile unsigned short *) 0x04001032)
#define DB_BG3PC       (*(volatile unsigned short *) 0x04001034)
#define DB_BG3PD       (*(volatile unsigned short *) 0x04001036)
#define DB_BG3X        (*(volatile unsigned int *) 0x04001038)
#define DB_BG3Y        (*(volatile unsigned int *) 0x0400103c)
#define DB_WIN0H       (*(volatile unsigned short *) 0x04001040)
#define DB_WIN1H       (*(volatile unsigned short *) 0x04001042)
#define DB_WIN0V       (*(volatile unsigned short *) 0x04001044)
#define DB_WIN1V       (*(volatile unsigned short *) 0x04001046)
#define DB_WININ       (*(volatile unsigned short *) 0x04001048)
#define DB_WINOUT      (*(volatile unsigned short *) 0x0400104a)
#define DB_MOSAIC      (*(volatile unsigned short *) 0x0400104c)
#define DB_BLDCNT      (*(volatile unsigned short *) 0x04001050)
#define DB_BLDALPHA    (*(volatile unsigned short *) 0x04001052)
#define DB_BLDY        (*(volatile unsigned short *) 0x04001054)
#define DB_MASTER_BRIGHT (*(volatile unsigned short *) 0x0400106c)


// ----------------------------------------------------------------------------
//  variables in memory
// ----------------------------------------------------------------------------

/**
 * Frame counter
 * The value is updated by the ARM7.
 */
extern volatile unsigned int frame_counter;

// ----------------------------------------------------------------------------
//  Gamepad
// ----------------------------------------------------------------------------

#define PAD_X (1 << 10)
#define PAD_Y (1 << 11)
#define PEN_STATUS (1 << 13)
#define HINGE (1 << 15)

/**
 * X, Y buttons, Pen and the hinge state.
 * This value is updated by the ARM7.
 *
 * bit 10 X button 0=pressed, 1=released
 * bit 11 Y button 0=pressed, 1=released
 * bit 13 PEN 0=Pressed, 1=Released/Disabled
 * bit 15 hinge status 1=closed, 0=open
 */
extern volatile unsigned short ext_gamepad;


// ----------------------------------------------------------------------------
//  secure area
// ----------------------------------------------------------------------------

/**
 * NOTE: All BIOS calls are routed via trampoline functions located
 * in the secure area. They all use the thumb instruction set.
 */

__attribute__((target("thumb"))) void ndk_svc_wait_by_loop(int delay);

// ----------------------------------------------------------------------------
//  Init functions
// ----------------------------------------------------------------------------

/**
 * Called from ndk_platform_init, unknown purpose.
 *
 * TODO: reverse these functions
 */
void ndk_02007f10();
void ndk_0200e8f4();
void ndk_0200ff78(int (*fn)(void));

// ?
void ndk_set_irq_stack_canaries();

/**
 * Init platform. Must be the first thing called
 * in main otherwise the DS locks up.
 */
void ndk_platform_init();

// ----------------------------------------------------------------------------
//  Misc
// ----------------------------------------------------------------------------

struct owner_info {
  char language;
  char favorite_color;
  char birthday_month;
  char birthday_day;
  short nickname[10];
  short nickname_length;
  short message[26];
  short message_length;
};

/**
 * Get owner info.
 *
 * NOTE: reads owner data from 0x027ffc80 and not from the firmware.
 * See: http://problemkaputt.de/gbatek-ds-firmware-user-settings.htm
 */
void ndk_get_owner_info(struct owner_info *info);

/**
 * Sleep platform and wait for lid to open.
 *
 * Call this function to sleep the platform when a lid closed event is
 * detected. One way to detect it is to poll the ext_gamepad location to see if
 * bit 15 is set.
 *
 * NOTE: The game always the uses params (12, 0, 0).
 *
 * @param unk0
 * @param unk1
 * @param unk2
 */
void ndk_handle_lid_closed(unsigned int unk0, unsigned int unk1,
                           unsigned int unk2);

/**
 * Halt the CPU forever. There is no way exit this function.
 *
 * NOTE: Perhaps it should be possible to patch this function during debug
 * builds.
 */
void ndk_panic(void);

// ----------------------------------------------------------------------------
//  External memory contol. Slot-1 and slot-2
// ----------------------------------------------------------------------------

void ndk_slot1_give_arm9_access(void);

void ndk_slot1_give_arm7_access(void);

void ndk_slot2_give_arm9_access(void);

void ndk_slot2_give_arm7_access(void);


// ----------------------------------------------------------------------------
//  printf functions
// ----------------------------------------------------------------------------

/**
 * All these functions have been built without support for float and double
 * data types.
 */

int ndk_vsnprintf(char *s, size_t size, const char *fmt, va_list arg);

int ndk_snprintf(char *s, size_t n, const char *fmt, ...);

int ndk_vsprintf(char *s, const char *fmt, va_list arg);

int ndk_sprintf(char *s, const char *fmt, ...);

#endif
