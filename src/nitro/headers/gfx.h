/**
 * Graphics functions.
 *
 * None of the other SDK subsystems seem to be dependent on this one. So it's
 * probably safe to roll your own graphics functions.
 *
 * TODO: prefix all functions with 'ndk_gfx_'
 * TODO: make function names more readable
 */
#ifndef GFX_INCLUDE_FILE
#define GFX_INCLUDE_FILE

#include "stdbool.h"

#include "nds.h"
#include "fix_math.h"

/**
 * Display Modes for engine A
 */
#define ENGINE_A_DISPLAY_MODE_OFF 0
#define ENGINE_A_DISPLAY_MODE_GRAPHICS 1

#define ENGINE_A_DISPLAY_MODE_BITMAP_VRAM_A 2
#define ENGINE_A_DISPLAY_MODE_BITMAP_VRAM_B 6
#define ENGINE_A_DISPLAY_MODE_BITMAP_VRAM_C 10
#define ENGINE_A_DISPLAY_MODE_BITMAP_VRAM_D 14

#define ENGINE_A_DISPLAY_MODE_BITMAP_MAIN_RAM 3

/**
 * 3D engine commands. Used when writing commands to the command fifo.
 */
#define CMD_NOOP 0x00
#define CMD_MTX_MODE 0x10
#define CMD_MTX_PUSH 0x11
#define CMD_MTX_POP  0x12
#define CMD_MTX_STORE 0x13
#define CMD_MTX_RESTORE 0x14
#define CMD_MTX_IDENTITY 0x15
#define CMD_MTX_LOAD_4x4 0x16
#define CMD_MTX_LOAD_3x3 0x17
#define CMD_MTX_MULT_4x4 0x18
#define CMD_MTX_MULT_4x3 0x19
#define CMD_MTX_MULT_3x3 0x1a
#define CMD_MTX_SCALE 0x1b
#define CMD_MTX_TRANS 0x1c

#define CMD_COLOR 0x20
#define CMD_NORMAL 0x21
#define CMD_TEXCOORD 0x22
#define CMD_VTX_16 0x23
#define CMD_VTX_10 0x24
#define CMD_VTX_XY 0x25
#define CMD_VTX_XZ 0x26
#define CMD_VTX_YZ 0x27
#define CMD_VTX_DIFF 0x28
#define CMD_POLYGON_ATTR 0x29
#define CMD_TEXIMAGE_PARAM 0x2a
#define CMD_TEXPLTT_BASE 0x2b

#define CMD_DIF_AMB 0x30
#define CMD_SPE_EMI 0x31
#define CMD_LIGHT_VECTOR 0x32
#define CMD_LIGHT_COLOR 0x33
#define CMD_SHININESS 0x34

#define CMD_BEGIN_VTXS 0x40
#define CMD_END_VTXS 0x41

#define CMD_SWAP_BUFFERS 0x50

#define CMD_VIEWPORT 0x60

#define CMD_BOX_TEST 0x70
#define CMD_POS_TEST 0x71
#define CMD_VEC_TEST 0x72

/**
 * State variable used by the functions:
 *
 * ndk_set_display_mode_engine_a
 * ndk_set_display_mode_on_engine_a
 * ndk_set_display_mode_off_engine_a
 *
 * 1 = display on, 0 = display off
 */
extern unsigned short gfx_engine_a_is_display_on;

/**
 * Holds the current display mode.
 */
extern unsigned short gfx_display_mode_engine_a;

/**
 * This address contains the DMA channel number that is used by the load
 * functions (gfx lib?) when copying data to the graphics memory. If it
 * contains -1 then the CPU will do the copying.
 *
 * NOTE: For transfer sizes <= 0x30 the CPU will be used to load the data.
 *
 * NOTE: This value is here only for reference don't use it directly.
 *
 * NOTE: To set this value patch it from the linker script.
 */
extern unsigned int gfx_dma_number;

/**
 * Set modes for 3D/2D engine A
 *
 * BG Modes
 *
 * Mode  BG0      BG1      BG2      BG3
 * 0     Text/3D  Text     Text     Text
 * 1     Text/3D  Text     Text     Affine
 * 2     Text/3D  Text     Affine   Affine
 * 3     Text/3D  Text     Text     Extended
 * 4     Text/3D  Text     Affine   Extended
 * 5     Text/3D  Text     Extended Extended
 * 6     3D       -        Large    -
 *
 * @param bg_mode 0-6
 * @param disp_mode the display mode
 * @param if bg0_3d true = use 3D engine for BG0
 */
void ndk_set_display_mode_engine_a(int disp_mode, int bg_mode, bool bg0_3d);

/**
 * Turn on display for engine A.
 *
 * If no previous display mode has been set this function will default to
 * 'Graphics Display' otherwise the last set display mode is set.
 */
void ndk_set_display_mode_on_engine_a(void);

/**
 * Turn off display for engine A.
 *
 * The display will become white.
 *
 * The current display mode is saved before the display mode is set to off. It
 * is restored by a call to ndk_set_display_mode_on_engine_a
 */
void ndk_set_display_mode_off_engine_a(void);

inline void ndk_set_display_mode_on_engine_b(void)
{
    DB_DISPCNT |= 0x10000;
}

inline void ndk_set_display_mode_off_engine_b(void)
{
    DB_DISPCNT ^= 0x10000;
}

/**
 * Enable or disable LCD v-blank IRQs.
 *
 * @param enable 0 : disable, non-zero : enable
 * @return old value (DISPSTAT & 8)
 */
int ndk_lcd_set_vblank_irq_enable(int enable);

/**
 * Set BG mode for 2D engine B
 *
 * Mode  BG0      BG1      BG2      BG3
 * 0     Text/3D  Text     Text     Text
 * 1     Text/3D  Text     Text     Affine
 * 2     Text/3D  Text     Affine   Affine
 * 3     Text/3D  Text     Text     Extended
 * 4     Text/3D  Text     Affine   Extended
 * 5     Text/3D  Text     Extended Extended
 *
 * @param mode 0-5
 */
void ndk_set_bg_mode_engine_b(int mode);

/**
 * Initialize the graphics subsystem.
 *
 * Does the following:
 *
 * send display A to upper screen (display B to bottom screen)
 * enable 2D graphics engine A
 * enable 2D graphics engine B
 * enable 3D rendering engine
 * enable 3D geometry engine
 * disable both LCDs
 * disable VRAM for banks A-I
 * disable all display interrupts.
 * reset all BG layer registers (to zero) for both engines
 * disable master brightness for both engines
 * set rotation to 0 deg and scale to 1 for BG2 and BG3 on both engines
 * 
 * NOTE: This function an only be called once per boot! The second time this
 * function is called the platform locks up.
 */
void ndk_gfx_init(void);

/**
 * Init VRAM
 *
 * Disables VRAM for all banks (A-I). By default all banks are allocated to
 * LCDC (plain ARM9 CPU access).
 */
void ndk_gfx_init_vram_control(void);

void *ndk_get_bg2_screen_base_addr_engine_b(void);

/**
 * Set the affine transform parameters.
 * 
 * See: http://problemkaputt.de/gbatek-lcd-i-o-bg-rotation-scaling.htm
 * and: http://problemkaputt.de/gbatek-ds-video-bg-modes-control.htm
 * 
 * @param BGxPA address of the BG PA register
 * @param transform a 2x2 matrix [A, B, C, D]
 * @param x0 x coordinate of the center of rotation
 * @param y0 y coordinate of the center of rotation
 * @param x1 x coordinate of the reference point
 * @param y1 y coordinate of the reference point
 */
void ndk_bg_set_affine_transform(volatile unsigned short *BGxPA,
                                 fix12 *transform, int x0, int y0, int x1,
                                 int y1);

/**
 * Geometric engine commands.
 */
void ndk_clear_edge_for_toon_and_shininess_tables();

void ndk_reset_all_matrix_stacks();

void ndk_matrix_z_axis_rotation(fix12 sin, fix12 cos);

void ndk_matrix_y_axis_rotation(fix12 sin, fix12 cos);

void ndk_matrix_x_axis_rotation(fix12 sin, fix12 cos);

/**
 * Same as OpenGL glLookAt function.
 *
 * NOTE: Parameter order is unconfirmed!!
 *
 * @param eye vector of 3 FX values
 * @param up vector of 3 FX values
 * @param target vector of 3 FX values
 */
void ndk_look_at(fix12 *eye, fix12 *up, fix12 *target);

/*
 * TODO: document me!
 */
void ndk_set_clear_plane_params(short color, int alpha, int depth,
                                int polygon_id, bool fog);

void ndk_load_oam_data_to_engine_a(void *src, int offset, int size);

void ndk_load_oam_data_to_engine_b(void *src, int offset, int size);

void ndk_load_obj_palette_data_to_engine_b(void *src, int offset, int size);

void ndk_load_obj_palette_data_to_engine_a(void *src, int offset, int size);

void ndk_load_bg_palette_data_to_engine_b(void *src, int offset, int size);

void ndk_load_bg_palette_data_to_engine_a(void *src, int offset, int size);

#endif // GFX_INCLUDE_FILE
