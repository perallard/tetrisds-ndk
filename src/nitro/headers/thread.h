/**
 * Thread API.
 *
 * The thread system seem to be based upon events. That is events like
 * interrupt requests and timeouts will preempt the current thread. But all
 * threads should yield or exit when there is no more processing to be done.
 * To let lower priority threads to resume their execution.
 *
 * This module/logical unit also includes a mutex API.
 *
 * NOTE: To have a thread suspend until a specific IRQ event see
 * ndk_thread_wait_irq in interrupts.h
 *
 * NOTE: At every context switch the thread priority list is always searched
 * from the begining for the first pending thread to run. This implementation
 * does not scale well to many threads! So design your program to use as few
 * threads as possible at one time.
 */
#ifndef THREAD_INCLUDE_FILE
#define THREAD_INCLUDE_FILE

#include <stdbool.h>

#include "cpu.h"

/**
 * These functions are not found in the ROM but compile to very similar code.
 * They are added here because they don't require an compiled object file to
 * be used.
 */
inline void ndk_thread_critical_enter(int *lock)
{
  *lock = ndk_cpu_disable_irq();
}

inline void ndk_thread_critical_leave(int *lock)
{
  ndk_cpu_write_irq_flag(*lock);
}

/**
 * CPU register and math coprocessor state
 */
struct context {
  // NOTE: This struct is poorly understod. Only its size is known.
  int program_status_register;      // 0x00
  // {r0, r1, r2, r3, r4, r5, r6, r7, r8, sb, sl, fp, ip, sp, lr}
  int registers[15];                // 0x04
   // initially set to worker function
  void *unk1;                       // 0x40
  void *unk2;                       // 0x44
  unsigned int hw_math_state[7];    // 0x48
  // 0x64
};

/*
 * arguments ?
 */
typedef void thread_exit_fn(int);

/**
 * Function type that gets executed when a thread is started. It takes a
 * pointer to an argument of any type as input.
 */
typedef void thread_worker_fn(void *);

struct thread;

struct thread_list {
  struct thread *first;
  struct thread *last;
};

struct thread_list_node {
  struct thread *next;
  struct thread *prev;
};

struct mutex;

struct mutex_list {
  struct mutex *first;
  struct mutex *last;
};

struct mutex_list_node {
  struct mutex *next;
  struct mutex *prev;
};

struct mutex {
  // List of blocked threads
  struct thread_list queue;         // 0x00
  // The thread that holds this lock
  struct thread *thread;            // 0x08
  // The number of threads that are blocked
  int lock_count;                   // 0x0c
  /*
   * The thread that holds this lock will have this node in its list of held
   * mutexes.
   */
  struct mutex_list_node elem;      // 0x10
  // 0x18
};

struct thread {
  struct context ctx;
  // 0 = stopped/paused, 1 = running/pending, 2 = removed
  int status;                       // 0x64
  /*
   * List of all threads sorted in priority order. This is the list that is
   * used by the scheduler to select what thread should run.
   */
  struct thread *priority_next;     // 0x68
  int id;                           // 0x6c
  /*
   * A value between 0-31. 0 has the highest priority and 31 the lowest.
   */
  int priority;                     // 0x70
  int unk3;                         // 0x74
  /*
   * If a thread is stopped using ndk_thread_yield with a non-null argument.
   * pending_list will point to the supplied list and the prev and next fields
   * in pending_node will reflect this threads position in the list. The list
   * is kept sorted in priority order.
   */
  struct thread_list *pending_list; // 0x78
  struct thread_list_node *pending_node;  // 0x7c
  /*
   * If non-null points to the current mutex this thread is blocked at.
   */
  struct mutex *blocked_at;         // 0x84
  /*
   * If held_by_me.first is non-null this thread holds one of more mutexes.
   */
  struct mutex_list *held_by_me;    // 0x88
  void *stack_bottom;               // 0x90
  void *stack_top;                  // 0x94
  int unk12;                        // 0x98
  struct thread_list deleted_pending_list;   // 0x9c
  int unk15[3];                     // 0xa4
  void *unk16;                      // 0xb0
  /*
   * Function that gets called when the worker function returns/exits. Default
   * set to null when the thread is created. To set a custom exit function call
   * ndk_thread_set_exit_fn.
   */
  thread_exit_fn *exit_fn;          // 0xb4
  int unk17;                        // 0xb8
  int unk18;                        // 0xbc
  // 0xc0
};

// arguments (current, pending)
typedef void thread_switch_fn(struct thread *, struct thread *);

/**
 * Counting lock to prevent ndk_thread_switch from executing.
 */
extern unsigned int thread_switch_lock;

/**
 * Used by ndk_thread_create_id to generate unique id for every thread.
 */
extern int thread_id_count;

/**
 * Gets executed after the current thread has been suspended.
 */
extern thread_switch_fn *thread_post_stop_fn;

extern struct thread **current_thread;

extern bool thread_subsystem_initialized;

extern struct {
  /*
   * bit 0 = has waiting threads in pending_irq_thread_list
   */
  short flags;                      // 0x00
  short flags2;                     // 0x02
  struct thread *current;           // 0x04
  // linked list of threads sorted in ascending priority order
  struct thread *priority_list;     // 0x08
  // called before a pending thread is resumed
  thread_switch_fn *pre_resume_fn;  // 0x0c
  // more ?
} thread_base;

/**
 * This is the thread context for the main program. It has priority 16.
 */
extern struct thread main_thread;

/**
 * This thread will resume execution when all other threads have suspended
 * their execution. It will halt the CPU and wait for an IRQ in an endless
 * loop. It is probably a good idea to let the idle thread run when no
 * processing needs to be done as it will save some power. It has priority
 * 31.
 */
extern struct thread idle_thread;

/**
 * Query if the thread subsystem has been initialised.
 *
 * @return true if thread subsystem has been initialized otherwize false.
 */
bool ndk_thread_is_subsystem_initialized();

/**
 * Set a function that will be called if the threads worker function returns.
 *
 * @param t
 * @param fn
 */
void ndk_thread_set_exit_fn(struct thread *t, thread_exit_fn *exit);

/**
 * This is the worker function for the idle_thread.
 *
 * All it does is enable IRQs and then halts the CPU and wait for an interrupt
 * event to occur in an endless loop.
 *
 * Don't use it directly, it's defined here only for reference.
 */
void ndk_thread_idle_cpu_fn(void *);

/**
 * ???
 */
thread_switch_fn *ndk_thread_set_pre_resume_fn(thread_switch_fn *fn);

/**
 * Yield the current thread for n miliseconds.
 *
 * NOTE: After n miliseconds the currently executing thread will be preempted
 * by a switch back to this thread.
 *
 * NOTE: ndk_thread_init_timers must have been called before this function can
 * be used. See timers.h for more details.
 *
 * @param milis
 */
void ndk_thread_sleep(int milis);

/**
 * Get a threads priority.
 *
 * @param t
 * @return [0-31]
 */
int ndk_thread_get_priority(struct thread *t);

/**
 * Change the priority of a thread. This call might cause a context switch.
 *
 * @param t
 * @param priority
 * @return true on success, false otherwize
 */
bool ndk_thread_set_priority(struct thread *t, int priority);

/**
 * Move the current thread to be the last of its priority group. This call
 * might cause a context switch.
 */
void ndk_thread_push_back(void);

/**
 * Suspend the current thread (this) and resume the next pending thread in
 * priority order. Used by the other thread functions when a context switch is
 * required.
 *
 * NOTE: This function is here only for reference Don't call it directly.
 */
void ndk_thread_switch(void);

/**
 * Get the next thread to be resumed according to the priority list.
 *
 * NOTE: Helper function. Don't call directly.
 *
 * @return the thread
 */
struct thread *ndk_thread_get_pending(void);

/**
 * Set a thread as pending and resume the next thread according to the priority
 * list.
 *
 * @param t
 */
void ndk_thread_run(struct thread *t);

/**
 * Set a list of threads as pending and resume the next thread according to the
 * priority list.
 *
 * Every thread in the list is popped from the list and set as pending in the
 * priority list. So when this function returns the list will be empty.
 *
 * @param list
 */
void ndk_thread_run_list(struct thread_list *list);

/**
 * Stop/pause the current thread and resume the next thread in the priority
 * list.
 * 
 * NOTE: To start the thread(s) again call ndk_thread_run or
 * ndk_thread_run_list.
 *
 * @param pending_list if it's non-null this thread will be inserted in the
 * list in priority order.
 */
void ndk_thread_yield(struct thread_list *pending_list);

/**
 * Check if thread has been marked as removed from the priority queue
 *
 * @param t
 * @return true if it has been removed, false otherwise.
 */
bool ndk_thread_has_been_removed(struct thread *t);

/**
 * Remove a thread from the priority list. It's also removed from the pending
 * list, if it's assigned to one. If the thread is the current one switch to
 * the next pending thread in the priority list.
 *
 * @param t
 */
void ndk_thread_delete(struct thread *t);

/**
 * Called from ndk_thread_delete if the thread is the current thread.
 *
 * NOTE: Don't call this function directly it's here only for reference.
 */
void ndk_thread_delete_self(void);

/**
 * This function gets called when the threads routine returns.
 *
 * NOTE: Don't call this function directly it's here only for reference.
 */
void ndk_thread_exit(void);

/**
 * Create a new thread and add it to the list of prioritized threads. To start
 * the thread call one of the run functions.
 *
 * @param t
 * @param worker for the thread
 * @param arg argument passed to the worker when it gets executed.
 * @param stack_top pointer to the top of stack
 * @param size stack size
 * @param priority 0-31
 */
void ndk_thread_create(struct thread *t, thread_worker_fn *fn, void *arg,
                       void *stack_top, int size, int priority);

/**
 * Initialize the thread subsystem
 *
 * NOTE: Called from ndk_platform_init so there is no need to call it directly.
 */
void ndk_thread_init(void);

/**
 * Remove a thread from the list of prioritized threads.
 *
 * NOTE: Helper function. Don't call directly.
 *
 * @param t
 */
void ndk_thread_remove_from_priority_list(struct thread *t);

/**
 * Add a thread to the list of prioritized threads.
 *
 * NOTE: Helper function. Don't call directly.
 *
 * @param t
 */
void ndk_thread_add_to_priority_list(struct thread *t);

/**
 * Remove the first element from a mutex list.
 *
 * NOTE: Helper function. Don't call directly.
 *
 * @param list the mutex list.
 * @return the removed mutex or null if the list was empty.
 */
struct mutex *ndk_mutex_list_pop(struct mutex_list *list);

/**
 * Remove a thread from a pending list.
 *
 * NOTE: Helper function. Don't call directly.
 *
 * @param list
 * @param t
 * @return null if the thread wasn't found
 */
struct thread *ndk_thread_remove_from_pending_list(struct thread_list *list,
                                                   struct thread *t);

/**
 * Remove the first element from a pending list.
 *
 * @param list
 * @return a thread or null if the list was empty
 */
struct thread *ndk_thread_pop_from_pending_list(struct thread_list *list);

/**
 * Add a thread to the pending list. All threads are added by priority order.
 *
 * @param list
 * @param t
 */
void ndk_thread_add_to_pending_list(struct thread_list *list,
                                    struct thread *t);

/**
 * Create an id to assign to new threads.
 *
 * NOTE: Helper function used by ndk_create_thread. Don't call directly.
 *
 * @return id
 */
int ndk_thread_create_id(void);

// CPU and math coprocessor state handling functions

/**
 * NOTE: Helper function. Don't call directly.
 * 
 * @param ctx
 * @param fn
 * @param stack
 */
void ndk_context_init(struct context *ctx, thread_worker_fn *fn, void *stack);

/**
 * NOTE: Helper function. Don't call directly.
 *
 * @param ctx
 * @return always 0 ??
 */
int ndk_context_save(struct context *ctx);

/**
 * NOTE: Helper function. Don't call directly.
 *
 * return 1 if success ??
 */
int ndk_context_restore(struct context *ctx);

// Mutex API

/**
 * Remove a mutex from a threads list of held mutexes.
 *
 * NOTE: Helper function. Don't call it directly.
 *
 * @param holder the thread the holds this mutex.
 * @param m the mutex
 */
void ndk_thread_remove_mutex_from_held_by_me(struct thread *holder,
                                             struct mutex *m);

/**
 * Add a mutex to a threads list of held mutexes.
 *
 * NOTE: Helper function. Don't call it directly.
 *
 * @param holder thread that is to be a holder of the mutex.
 * @param m the mutex.
 */
void ndk_thread_add_mutex_to_held_by_me(struct thread *holder,
                                        struct mutex *m);

/**
 * Try to accuire a mutex lock.
 *
 * @param m the mutex object.
 * @return true if the lock was accuired, false otherwise.
 */
bool ndk_mutex_trylock(struct mutex *m);

/**
 * Release all mutex locks held by a thread.
 *
 * NOTE: Helper function. Don't call it directly.
 *
 * @param t the thread.
 */
void ndk_thread_unlock_all_mutexes(struct thread *t);

/**
 * Release a locked mutex.
 *
 * @param m the mutex object.
 */
void ndk_mutex_unlock(struct mutex *m);

/**
 * Accuire a mutex lock.
 *
 * @param m the mutex object.
 */
void ndk_mutex_lock(struct mutex *m);

/**
 * Initialize a mutex.
 *
 * Must be called before a mutext is used for locking.
 *
 * @param m the mutex object.
 */
void ndk_mutex_init(struct mutex *m);

#endif // THREAD_INCLUDE_FILE
