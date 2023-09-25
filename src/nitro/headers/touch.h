/**
 * Touch API.
 *
 * NOTE: This API used the math accelerator for some of its operations.
 *
 * NOTE: The touch screen controller is managed by the ARM7.
 *
 * Typically when initializing the game one would do:
 *
 * struct touch_xform xform;
 *
 * // This retrieves user touch calibration data.
 * if (! ndk_touch_get_transform_from_firmware_settings(&xform))
 *     ndk_panic();
 *
 * // Tells the touch API to use this transform to convert raw ADC data to
 * // screen space coordinates.
 * ndk_touch_set_coordinate_transform(&xform);
 *
 * // Get raw touch sample
 *
 * struct touch_pos *raw;
 * struct touch_pos coords;
 *
 * // Will apply the set transform to raw and stored in coords.
 * ndk_touch_transform_into_screen_coords(&coords, &raw);
 *
 * // Use data in coords ...
 */

#ifndef TOUCH_INCLUDE_FILE
#define TOUCH_INCLUDE_FILE

#include <stdbool.h>

#define TOUCH_SUCCESS 0
#define TOUCH_FAILURE 1

struct touch_pos {
	short x;
	short y;
	unsigned short pen_down;
	unsigned short valid;
};

struct touch_xform {
	short x_offset;
	short y_offset;
	short x_scale;
	short y_scale;
};

/**
 * Set by the init function.
 *
 * NOTE: Don't access directly. It's here only for reference.
 */
extern int touch_initialised;

/**
 * Touch state memory layout
 *
 * NOTE: Don't access these values directly. They are define here only for
 * reference.
 */
extern struct {
	void (*unk000)(int, int, void *);
	short adc_x;                     // latest touch sample
	short adc_y;
	unsigned short pen_down;
	unsigned short valid;
	unsigned short auto_count;		 // offset of the last sample
	short unk00e;
	unsigned short *auto_buffer;
	unsigned short auto_buffer_size;
	short unk016;
	int x_offset;                    // calibration transform
	int unk01c;
	int x_scale;
	int y_offset;
	int unk028;
	int y_scale;
	short have_transform;
	short unk032;
	short data_status;               // bit: 1 = ready, 0 = not ready
	short request_status;            // bit: 1 = busy, 0 = ready
} touch_state;

/**
 * Touch calibration data.
 *
 * Used to transform raw touch data into screen coordinates.
 *
 * NOTE: These values are read from the firmware by the ARM7.
 * NOTE: Don't use these values directly. They are defined here only for
 * reference.
 */
extern struct {
	short adc_x1;
	short adc_y1;
	char scr_x1;
	char scr_y1;
	short adc_x2;
	short adc_y2;
	char scr_x2;
	char scr_y2;
} touch_calibration_data;

/**
 * Touch raw input data.
 *
 * After a touch sample has been requested using direct mode the ARM7 places
 * data words into the following two memory locations.
 *
 * bits:
 *   0-11 adc x-coordinate
 *   12-23 adc y-coordinate
 *   24 pen down
 *	    1 : down
 * 	    0 : not down
 *   25-26 data validity
 *	    00 : Both the x-coordinate and y-coordinate are valid
 *	    01 : The x-coordinate is invalid
 *	    10 : The y-coordinate is invalid
 *	    11 : Both the x-coordinate and y-coordinate are invalid
 *
 * NOTE: Don't read this values directly. They are defined here only for
 * reference.
 */
extern unsigned short touch_requested_sample_raw_lo;
extern unsigned short touch_requested_sample_raw_hi;

/**
 * Inititialize the touch subsystem.
 */
void ndk_touch_init(void);

/**
 * Request a sample from the touch screen controller.
 *
 * This is referred as direct mode.
 *
 * To get the actual sample data call ndk_touch_get_sample.
 */
void ndk_touch_request_sample(void);

/**
 * Get the offset in the circular buffer of the last auto sampled touch
 * position.
 *
 * @return the offset
 */
unsigned int ndk_touch_get_last_sample_offset(void);

/**
 * Start auto sampling.
 *
 * Use this function when you want samples from the touch screen controller
 * at higher frequencies.
 *
 * NOTE: This function depends on v-blank interrupts. So if you plan to use
 * your own v-blank handler don't forget to set the correct bit in the
 * thread_irq_bits location. See: interrupts.h for more details.
 *
 * @param start at what vertical line count the first sample should be read.
 * @param count number of samples to be read within the same frame.
 * @param buffer a circular buffer. Its size must be larger than count by at
 *        least one.
 * @param size number of elements the buffer can hold.
 */
void ndk_touch_start_auto_sampling(short start, short count,
                                   struct touch_pos *buffer, int size);
/**
 * Stop auto sampling.
 */
void ndk_touch_stop_auto_sampling(void);

/**
 * Set touch raw to screen coordinate transform.
 *
 * Set the transform that will be used to convert raw touch coordinates into
 * screen coordinates when calling ndk_touch_transform_into_screen_coords.
 *
 * @param xform transform or null to allow no transformation.
 */
void ndk_touch_set_coordinate_transform(struct touch_xform *xform);

/**
 * Get touch coordinate transform form the user settings.
 *
 * Calculate the offsets and scaling factors using the touch calibration data
 * in the firmware.
 *
 * NOTE: This function can fail if the calibration data is malformed. Then the
 * xform will be all zeroes, but will return true anyway.
 *
 * @param[out] pointer to a touch_xform struct where the result will be stored.
 * @return true if the operation succeeds, false otherwise.
 */
bool ndk_touch_get_transform_from_firmware_settings(struct touch_xform *xform);

/**
 * Calculate touch coordiate transform.
 *
 * Calculate the offsets and scaling factors according to supplied calibration
 * data.
 *
 * NOTE: If this function fails the contents of xform is undefined.
 * NOTE: This function will most likely never be called directly. It's defined
 * here only for reference.
 *
 * @return TOUCH_SUCCESS on success, TOUCH_FAILURE otherwise
 */
int ndk_touch_calculate_transform(struct touch_xform *xform,
								  unsigned short adc_x1, unsigned short adc_y1,
								  unsigned short scr_x1, unsigned short scr_y1,
								  unsigned short adc_x2, unsigned short adc_y2,
								  unsigned short scr_x2, unsigned short scr_y2
								  );

/**
 * Convert raw coordinates to screen coordinates.
 *
 * This function takes raw touch data and converts it into pixel coordinates
 * using the set coordinate transform.
 *
 * NOTE: Result and raw can point to the same object. i.e. inplace conversion is
 * possible.
 *
 * @param[out] result
 * @param[in] raw
 */
void ndk_touch_transform_into_screen_coords(struct touch_pos *result,
											struct touch_pos *raw);

/**
 * Read one touch sample in direct mode.
 *
 * This call will block until a touch sample is available. First call
 * ndk_touch_request_sample to request a sample from the touch screen
 * controller.
 *
 * @param[out] data pointer to a touch_pos struct where the sample will be
 * written to.
 * @return TOUCH_SUCCESS on success, TOUCH_FAILURE otherwise
 */
int ndk_touch_get_sample(struct touch_pos *data);

#endif // TOUCH_INCLUDE_FILE
