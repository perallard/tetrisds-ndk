/**
 * Heap functions.
 *
 * NOTE: None of the other SDK subsystems seem to be dependent on this one.
 * So there is no need to use this heap implementation or any heap(s) at all.
 * Just use the ndk_area_get_memory_start and ndk_area_get_memory_end to get
 * the useable memory at every area and then use it as you will.
 *
 * NOTE: Only the main area seems to be initialized by the game. To see how the
 * heaps are set up see: game_init_heaps.
 *
 * NOTE: The heap algorithm seems to be a free list with first fit and
 * coalescing of free'd blocks.
 *
 * Here is an example that was reversed out from the game on how to set up
 * a heap for later use by the program. It allocates all available memory in
 * MAIN ram to be used in one great heap. Note: ndk_area_init must have been
 * called before any of the below code is executed.
 *
 * // Get the usable bounds of MAIN ram.
 * void *start = ndk_sys_get_memory_start(AREA_MAIN);
 * void *end = ndk_sys_get_memory_end(AREA_MAIN);
 *
 * // Create a pool with one (1) heap using all available memory.
 * void *heap_mem_start = ndk_area_create_heap_pool(AREA_MAIN, start, end, 1);
 *
 * // Create the heap.
 * int id = ndk_area_create_heap(AREA_MAIN, heap_mem_start, end);
 *
 * // if id is less than zero then no heaps where available for use.
 * if (id < 0)
 *   ndk_panic();
 *
 * // This is a convenience function so we don't have to pass around the heap
 * // id all over the place.
 * ndk_area_set_current_heap(AREA_MAIN, id);
 *
 * // Initialization is now done! Allocate memory like this:
 *  void *m = ndk_area_alloc_mem(AREA_MAIN, HEAP_CURRENT, 1000);
 */
#ifndef HEAP_INCLUDE_FILE
#define HEAP_INCLUDE_FILE

#define HEAP_CURRENT -1

#define AREA_MAIN 0
#define AREA_UNDEFINED 1
#define AREA_DEBUG 2
#define AREA_ITCM 3
#define AREA_DTCM 4
#define AREA_SHARED 5
#define AREA_WRAM 6

/**
 * MAIN_area_start symbol. Defined only in the symbols object file. Tells where
 * in RAM the main memory heap should start. Note that the heap directly
 * follows the overlay area. So moving this value forwards or backwards in
 * memory will also affect the size of the overlay area.
 *
 * This value should not be altered in SW instead patch the ARM9 binary to the
 * desired value.
 */
extern void *MAIN_area_start;

/**
 * DTCM_area_size symbol. Defined only in the symbols object file. Tells how
 * big the area in the DTCM memory should be.
 *
 * If DTCM_area_size == 0 then no memory area is defined for DTCM.
 * If DTCM_area_size > 0 then you specify how much memory the user/supervisor
 * stack should be allowed to use. The rest is given to the area.
 *
 * If DTCM_area_size < 0 then you specify how much memory the area is allowed
 * to use and the rest is allocated to user/supervisor stack.
 *
 * The default is zero i.e. all remaining memory is allocated to the
 * user/supervisor stack. Giving a default stack size of about 15Kb.
 *
 * This value should not be altered in SW instead patch the ARM9 binary to the
 * desired value.
 */
extern int DTCM_area_size;

struct heap_block;

struct heap_block {
  struct heap_block *prev;  // 0x00
  struct heap_block *next;  // 0x04
  int size;                 // 0x08
  // 0x0c
};

struct heap {
  int size;                 // 0x00
  struct heap_block *free;  // 0x04
  struct heap_block *alloc; // 0x08
  // 0x0c
};

struct area {
  int current_heap;        // 0x00
  int heap_count;          // 0x04
  void *start;             // 0x08
  void *end;               // 0x0c
  struct heap *heap_list;  // 0x10
  struct heap heaps[];     // 0x14
};

extern int memory_areas_initialized;
extern struct area *memory_areas[6];

/**
 * Call these functions to get or set the runtime usable bounds of every area.
 *
 * Typically this functions will be called to get the bounds for
 * ndk_area_init_heap_pool.
 *
 * NOTE: These function are probably used for memory debugging purposes.
 *
 * NOTE: In ndk_area_init these value are set to the corresponding values from
 * ndk_area_get_memory_start and ndk_area_get_memory_end.
 *
 * @param area 0-6
 * @param address
 */
void ndk_sys_set_memory_start(int area, void *address);

void ndk_sys_set_memory_end(int area, void *address);

void *ndk_sys_get_memory_start(int area);

void *ndk_sys_get_memory_end(int area);

/**
 * Call these functions to get the bounds of every memory area. These values
 * where determined at compile time and can not be altered at runtime.
 *
 * @param area 0-6
 *
 * @return address or null if it's not defined
 */
void *ndk_area_get_memory_start(int area);

void *ndk_area_get_memory_end(int area);

/**
 * ?
 */
void ndk_area_debug_init(void);

/**
 * Initialize the area sub system.
 *
 * NOTE: Called from ndk_platform_init. So there is no need to call this
 * function directly. It's defined here for reference only.
 */
void ndk_area_init(void);

/**
 * Initialize an area for heap usage.
 *
 * @param area
 * @param start
 * @param end
 * @param count number of heaps in the pool
 * @return the address of the first allocatable byte
 */
void *ndk_area_create_heap_pool(int area, void *start, void *end, int count);

/**
 * There is no ndk_area_destroy_heap_pool function but implementing one is
 * quite easy. An example (with no error checking):
 *
 * void ndk_area_destroy_heap_pool(int area)
 * {
 *    int lock;
 *
 *    ndk_thread_critical_enter(&lock);
 *
 *    memory_areas[area] = NULL;
 *
 *    ndk_thread_critical_leave(&lock);
 * }
 *
 * This example was derived by reversing the ndk_area_alloc_mem function.
 */

/**
 * Setup a heap in an area.
 *
 * Call ndk_area_create_heap_pool before you call this function to allocate a
 * heap.
 *
 * @param area
 * @param start
 * @param end
 *
 * return heap or -1 if no heaps are available
 */
int ndk_area_create_heap(int area, void *start, void *end);

/**
 * There is no ndk_area_destroy_heap function but implementing one is quite
 * easy. An example follows (no error checking):
 *
 * void ndk_area_destroy_heap(int area, int heap)
 * {
 *    int lock;
 *
 *    ndk_thread_critical_enter(&lock);
 *
 *    struct area *a = memory_areas[area];
 *    struct heap *h = a->heap_list[heap];
 *
 *    h->size = -1; // this will signal that this heap slot is unused
 *    h->free = NULL;
 *    h->alloc = NULL;
 *
 *    ndk_thread_critical_leave(&lock);
 * }
 *
 * This example was derived by reversing the ndk_area_create_heap function.
 */

/**
 * Set the current working heap in a memory area.
 *
 * @param area
 * @param heap
 * @return the old heap
 */
int ndk_area_set_current_heap(int area, int heap);

/**
 * Allocate memory from an area heap.
 *
 * @param area
 * @param heap -1 is the current heap else it's the heap index
 * @param size
 * @return pointer to the allocated memory block
 */
void *ndk_area_alloc_mem(int area, int heap, int size);

/**
 * Free memory allocated from a area heap
 *
 * @param area
 * @param heap -1 is the current heap else it's the heap index
 * @param mem
 */
void ndk_area_free_mem(int area, int heap, void *mem);

#endif // HEAP_INCLUDE_FILE
