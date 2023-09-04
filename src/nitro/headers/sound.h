/**
 * Sound (SDAT) API.
 *
 * TODO: Document what can and can't be done using the reversed API in its
 * current state.
 *
 * Some notes on how the API for the Sound Archive (SDAT) works.
 *
 * 1) A 'sound source' is either a sound sequence or a sound stream. The
 *    maximum number of sound sources that can be played simultaneously is 16
 *    for sequences (SEQ) and 4 for streams (STRM). Sound streams can't be
 *    played since code for it seems to be missing from the ROM.
 *
 * 2) All music and sound effects are in a special format called a Sound
 *    sequence (SEQ), think Midi.
 *
 * 3) Sound sequences (SEQ) are added to a list of audio sources for playback
 *    using the 'ndk_sound_add_source_seq' and 'ndk_sound_add_source_seqarc'
 *    functions.
 *
 * 4) Added audio sources will only start playing after 'ndk_sound_process' has
 *    been called.
 *
 * 5) As soon as a sound sequence has finished it's removed from the list of
 *    sound sources.
 *
 * 6) All sound sequences have a player assigned to them. If a sound sequence
 *    is playing and another sound source is added that uses the same player
 *    it will replace the old one.
 *
 * 7) All sequence and stream playback is done by the ARM7. Note for streams
 *    it looks like there is some decoding from IMA-ADPCM to PCM16 that is done
 *    on the ARM9 side for some reason. Making IMA-ADPCM streams somewhat less
 *    attractive to use.
 *
 * See: http://www.feshrine.net/hacking/doc/nds-sdat.php
 * See: https://gota7.github.io/NitroStudio2/specs/soundData.html
 * See: http://www.feshrine.net/hacking/doc/nds-sdat.php
 * See: https://github.com/Kermalis/VGMusicStudio
 * See: https://ndspy.readthedocs.io/en/latest/index.html
 *
 *
 * Code example on how open an SDAT archive for playback:
 *
 * // initialize the sound lib
 * ndk_sound_init();
 *
 * // initialize a heap that the sound lib will use. buffer is a reference to
 * // memory that is large enough to hold all sound data needed by the program
 * // or game at one time.
 * struct sound_heap *snd_heap = ndk_sound_sdat_heap_init(buffer, size);
 *
 * struct sound_arch *arch;
 * ndk_sound_open_sdat_archive(arch, "/my_sound_arch.sdat", snd_heap, false);
 *
 * // initialize players using player data defined in the SDAT archive.
 * ndk_sound_sdat_init_seq_players(snd_heap);
 *
 * // sound data can now be loaded from the archive.
 *
 * TODO: give an code example on how to play a sound data.
 */
#ifndef SOUND_INCLUDE_FILE
#define SOUND_INCLUDE_FILE

#include <stdbool.h>

#include "file.h"
#include "thread.h"

/*
 * Sound IO error code defines
 */

#define SOUND_SUCCESS 0
#define SOUND_INVALID_GROUP 1
#define SOUND_INVALID_SEQ 2
#define SOUND_INVALID_SEQARC 3
#define SOUND_INVALID_BANK 4
#define SOUND_INVALID_WAVEARC 5
#define SOUND_LOAD_SEQ_ERROR 6
#define SOUND_LOAD_SEQARC_ERROR 7
#define SOUND_LOAD_BANK_ERROR 8
#define SOUND_LOAD_WAVEARC_ERROR 9

/*
 * The types of file that can be contained in a group.
 */
 #define SOUND_INFO_GROUP_TYPE_SEQ 0
 #define SOUND_INFO_GROUP_TYPE_BANK 1
 #define SOUND_INFO_GROUP_TYPE_WAVEARC 2
 #define SOUND_INFO_GROUP_TYPE_SEQARC 3

/*
 * These flags are OR'd to enable or disable automatic loading of dependent
 * files in group entries.
 *
 * For example sequence files are usually loaded with 7 this will load the seq
 * file and also bank and wavearc files as well.
 *
 * NOTE: For seqarcs dependent files are *never* loaded. Only the seqarc flag is
 * considered. This is probably due to the fact that the sequences in the seqarc
 * can depend on any number of banks and wavearcs in a non-tivial way.
 */
#define SOUND_INFO_GROUP_FLAG_SEQ 1
#define SOUND_INFO_GROUP_FLAG_BANK 2
#define SOUND_INFO_GROUP_FLAG_WAVEARC 4
#define SOUND_INFO_GROUP_FLAG_SEQARC 8

struct sound_seq {
  unsigned short bnk;	// Bank
  unsigned char vol;	// Volume
  unsigned char cpr;  // channel priority
  unsigned char ppr;  // player priority
  unsigned char ply;  // player [0-31]
  unsigned char unknown2[2]; // padding
};

struct sound_info_seq {
  union {
    unsigned int fat_id;	  // for accessing this file
    /*
     * For a seq files in a seqarc. Offset Where the seq is within the seqarc
     * file.
     */
    unsigned int nOffset;
  } head;
  struct sound_seq data;
};

struct sound_info_seqarc {
  unsigned int fat_id;
};

struct sound_info_bank {
  unsigned int fat_id;
  // Associated WAVEARC(s). 0xffff if not in use.
  unsigned short wavearc[4];
};

struct sound_info_wavearc {
  unsigned int fat_id : 24;
  /**
   * Bit 0: Cache archive header data only
   *  1 : Load only archive header data. All SWAV files that you intend to use
   *      for playback needs to be manually loaded into RAM. Note that there
   *      seem to be no function present in the ROM that does this. So to use
   *      this feature you will have to implement it yourself.
   *
   *      Implementation notes: The SWARs 'DATA' chunk contains an array of
   *      offsets to all embedded SWAV files. When caching header only the
   *      'SWAR' and 'DATA' chunks are loaded into RAM. *But* the array of
   *      offsets is doubled in size. The lower half will be zeroed out,
   *      probably to signal that no SWAVs have been loaded yet. The upper half
   *      contains a copy of the original array. This upper half can be used to
   *      accuire the file offset of the SWAV file you want to load. Once
   *      loaded write the RAM address back to the lower part.
   *
   *      Header caching is useful if you have very large wave archives that
   *      don't fit in your sound heap and you know you only will need a
   *      subset of the sound samples at a given time.
   *
   *      See: http://www.feshrine.net/hacking/doc/nds-sdat.php#swar
   *
   *      Note: Checkout the sound heap functions in this module if you feel
   *      the need to add heap management support to you SWAV load function.
   *
   *  0 : Load entire archive (all sound data) into RAM.
   */
  unsigned int flag : 8;
};

struct sound_info_player {
  // max seq that can be played at once by this player.
  unsigned char max_sequences;
  /**
   * Bit array of allowed hardware channels to use. If set to zero channel
   * allocation will be dynamic.
   */
  unsigned short channels;
  /**
   * Max amount of memory the player is allowed to use to load sound data. If
   * set to zero all sound data has to be preloaded manually before starting
   * playback of any sound sequences that use this player. Use
   * ndk_sound_load_group for instance to preload sound data. If not zero sound
   * data will be loaded dynamically when calling any of the
   * ndk_sound_add_source_xxx functions. Note however in this case there might
   * some delay before the sound is played due to having to get the sound data
   * from the ROM first.
   */
  unsigned int heap_size;
};

struct sound_info_group {
  unsigned int nCount;		// number of sub-records
  struct {
    unsigned char type;   // convert to enum?
    unsigned char flags;  // convert to enum?
    unsigned int nEntry;  // index
  } Group[1];
};

struct sound_info_player2 {
  unsigned char nCount;       // number of elements in v
  unsigned char v[16];		    // 0xff if not in use
  unsigned char unk0[7];	    // padding, 0s
};

struct sound_info_strm {
  unsigned int fat_id;		  // for accessing the file
  unsigned char vol;		    // volume
  unsigned char pri;        // priority
  unsigned char ply;        // player [0-7] ?
  unsigned char unk0[5];
};

struct sound_fat_rec {
  unsigned int nOffset;		      // offset of the sound file
  unsigned int nSize;		        // size of the Sound file
  unsigned int rt_ram_address;  // run-time RAM address, always 0 in file.
  unsigned int unk0[1];	        // always 0s, for storing data runtime or pad.
};

struct sound_fat_table {
  char type[4];		              // 'FAT '
  unsigned int nSize;		        // size of the FAT block
  unsigned int nCount;		      // Number of FAT records
  struct sound_fat_rec Rec[1];	// Arrays of FAT records
};

/**
 * Sound library run-time structures
 */

struct sound_list;
struct sound_list_node;

/**
 * Double linked list. Used to support lists of various types for internal
 * book keeping.
 *
 * A list element must include a sound_list_node structure. When a list is
 * initialized the offset to the sound_list_node structure must be supplied so
 * it can be found by the sound list functions.
 */

struct sound_list {
  void *first;                // 0x0
  void *last;                 // 0x4
  unsigned short count;       // 0x8
  unsigned short node_offset; // 0xa
  // 0x0c
};

struct sound_list_node {
    void *prev;               // 0x0
    void *next;               // 0x4
    // 0x8
};

// List struct definition end.

struct sound_sdat_arch;
struct sound_sdat_heap_node;

// Notify function that gets called then sound heap memory is free'd.
typedef void (*sdat_heap_free_fn)(void *mem, int size,
                                  struct sound_sdat_arch *arch, int fat_id);

struct sound_sdat_heap_node {
  struct sound_list_node node;          // 0x00
  int size;                             // 0x08
  sdat_heap_free_fn free_notify;        // 0x0c
  // this value can be NULL
  struct sound_sdat_arch *arch;         // 0x10
  int fat_id;                           // 0x14
  char pad[8];                          // 0x18
  // 0x20
  // memory block starts here
};

/**
 * NOTE: The heap is a LIFO structure that at every entry holds groups of
 * allocated blocks. The top entry in the LIFO structure is considered to be the
 * current heap. All blocks created with a call to ndk_sound_sdat_heap_alloc
 * will be added to the group of the current heap. ndk_sound_sdat_heap_free will
 * pop/destroy all groups of allocated blocks to the requested LIFO level. To
 * create/push a new group use ndk_sound_sdat_heap_new_group
 *
 * NOTE: LIFO level zero (0) is reserved for players and data allocated by
 * the open sdat archive function.
 *
 * NOTE: size unknown
 */
struct sound_sdat_heap {
  void *unk0;                 // 0x00
  /**
   * This list implemets the LIFO structure that holds groups of allocated
   * blocks.
   *
   * sound_list.count is used by by ndk_sound_sdat_heap_new_group
   */
  struct sound_list allocator_stack;  // 0x04
};

// in memory sdat archive structure
struct sound_sdat_arch {
  char type[4];               // 0x00 'SDAT'
  unsigned int magic;	        // 0x04 always 0x0100feff
  unsigned int file_size;     // 0x08 total file size
  unsigned short size;        // 0x0c header size
  unsigned short blocks;      // 0x0e
  unsigned int symbol_offset; // 0x10
	unsigned int symbol_size;   // 0x14
	unsigned int info_offset; 	// 0x18
	unsigned int info_size;    	// 0x1c
	unsigned int fat_offset;   	// 0x20
	unsigned int fat_size;     	// 0x24
	unsigned int file_offset; 	// 0x28
	unsigned int file_block_size; // 0x2c
  int unk1;                   // 0x30
  struct file file_handle;    // 0x34
  struct fat_handle fat_handle;  // 0x7c
  struct sound_fat_table *cached_fat_table;     // 0x84
  void *cached_symbol_block;  // 0x88
  void *cached_info_block;    // 0x8c
  // 0x90
};

struct sound_seq_source;

struct sound_player;

struct sound_handle;

struct sound_stream_source;

struct sound_player2;

struct sound_handle {
  struct sound_seq_source *elem;
};

struct sound_player {
  void *unk0;                  // 0x00
  void *unk1;                  // 0x04
  char unk2[28];               // 0x08
  // 0x24
};

struct sound_player2 {
  char unk0[48];               // 0x00
  // 0x30
};

struct sound_fade_info {
  int curr_volume;             // 0x00
  int volume;                  // 0x04
  int curr_duration;           // 0x08
  int duration;                // 0x0c
  // 0x10
};

struct sound_seq_source {
  struct sound_handle *handle;  // 0x00
  struct sound_player *player;  // 0x04
  int unk1;                     // 0x08
  int unk2;                     // 0x0c
  int unk3;                     // 0x10
  struct sound_list_node list;  // 0x14
  struct sound_fade_info fade;  // 0x1c
  // 0 : not playing
  // 1 : playing
  // 2 : pending to be stopped ?
  unsigned char status;         // 0x2c
  char unk17;                   // 0x2d
  // bool
  char paused;                  // 0x2e
  // bool
  char unk15;                   // 0x2f
  int unk9;                     // 0x30
  // 0: none, 1:seq, 2:seqarc seq
  unsigned short seq_type;      // 0x34
  short unk10;                  // 0x36
  union {
    unsigned short seq_id;      // 0x38
    struct {
      unsigned short arc_id;    // 0x38
      unsigned short seq_id;    // 0x3a
    } seqarc;
  };
  char player_var_id;           // 0x3c
  char unk12[3];                // 0x3d
  unsigned char volume;         // 0x40
  char unk16;                   // 0x41
  short unk13;                  // 0x42
  // 0x44
};

struct sound_stream_source {
  char unk0[0x5c];              // 0x00
  struct file handle;           // 0x5c
  char *file_offset;            // 0xa4
  // read buffer
  char buffer[0x40];            // 0xa8
  struct sound_fade_info fade_info;       // 0xe8
  char unk1[0x18];              // 0xf8
  /**
   * bit array of booleans
   */
  unsigned int flags;           // 0x110
  unsigned int unk2;            // 0x114
  unsigned int unk3;            // 0x118
  unsigned int unk4;            // 0x11c
  unsigned int unk5;            // 0x120
  char unk6[0x3c];              // 0x124
  void *open_file;              // 0x160
  void *close_file;             // 0x164
  void *read_file;              // 0x168
  void *unk1_fn_ptr;            // 0x16c
  // 0x170
};

struct sound_stream_handler_thread {
  struct thread worker_thread;    // 0x00
  char unk0[8];                   // 0xb8 -- pad ?
  char worker_thread_stack[1024]; // 0xc0
  struct thread_list *pending_list; // 0x4c0
  char unk3[16];                  // 0x4c8 -- struct of 4 int
  int unk4;                       // 0x4d8
  int unk5;                       // 0x4dc
  struct sound_list list;         // 0x4e0 -- not confirmed
};

/**
 * This structure contains the local and global player variables. Every player
 * has 16 local variables of 16 bits each. And there are an additional 16
 * global variables of 16 bits each. The global variables are shared by all
 * players.
 *
 * NOTE: All state in this structure is read/written to by the ARM7 which
 * handles all sound playback.
 *
 * NOTE: This is probably IPC data state. Consider moving this definition to
 * ipc.h.
 *
 * NOTE: Size is estimated!!
 */
extern char sound_players_variable_states[640];

/**
 * This is the current SDAT archive the library will use for playback.
 */
extern struct sound_sdat_arch *sound_current_sdat_archive;

extern char sound_temp_data_buffer[60];

extern struct sound_list sound_seq_queue;

extern struct sound_seq_source sound_seq_sources[16];

extern struct sound_player sound_seq_players[32];


extern int sound_stream_initialized;

extern void *sound_stream_buffer;

extern struct sound_list sound_stream_queue;

extern struct sound_stream_source sound_stream_sources[4];

extern struct sound_player2 sound_stream_players[8];

extern int sound_library_initialized;

/**
 * This is the worker thread for the sound subsystem.
 *
 * NOTE: It seems to be used for STRM playback.
 * NOTE: This location might be a larger struct that has a thread struct as its
 * first member.
 *
 * NOTE: Don't access this struct directly, it's defined only for reference.
 */
extern struct thread sound_thread;

/**
 * Used do decode IMA-ADPCM stream data on the fly.
 *
 * Used by a function in ROM at 0x02025348 that is called from
 * ndk_sound_worker_thread_fn.
 *
 * This shouldn't be needed as the sound hardware supports IMA-ADPCM playback.
 * See: https://problemkaputt.de/gbatek.htm#dssoundnotes it notes a potential
 * hardware bug.
 */
extern signed char ima_adpcm_index_table[16];
extern short ima_adpcm_step_tabble[89];

/*====================== PRIVATE DOUBLE LINKED LIST ===========================
 * Double linked list. Used to support various lists of various types for
 * internal book keeping.
 */


/**
 * Initialize a sound list
 *
 * @param q pointer to the list element.
 * @param node_offset offset to the sound_list_node in the list element
 *        struct. Use the 'offsetof' macro to accuire the offset.
 */
void ndk_sound_list_init(struct sound_list *q, unsigned int node_offset);

/**
 * Remove an element from a list.
 *
 * @param q
 * @param e
 */
void ndk_sound_list_remove(struct sound_list *q, void *e);

/**
 * Insert an element in a list.
 *
 * @param q
 * @param at if null the e is appended to the list else e is prepended to at.
 * @param e the element to be added.
 */
void ndk_sound_list_insert(struct sound_list *q, void *at, void *e);

/**
 * Add an element to the start of a list.
 *
 * @param q
 * @param e
 */
void ndk_sound_list_prepend(struct sound_list *q, void *e);

/**
 * Add an element to the end of a list.
 *
 * @param q
 * @param e
 */
void ndk_sound_list_append(struct sound_list *q, void *e);

/**
 * Add an element to an empty list.
 *
 * @param q
 * @param e
 */
void ndk_sound_list_append_to_empty_list(struct sound_list *q, void *e);

/**
 * Get the previous element from a sound element list.
 *
 * @param q the sound element list
 * @param e if null then the last element of the list will be returned else
 *        the previous element to e will be returned.
 * @return the sound list element
 */
void *ndk_sound_list_get_prev(struct sound_list *q, void *e);

/**
 * Get the next element from a sound element list.
 *
 * @param q the sound element list
 * @param e if null then the first element of the list will be returned else
 *        the next element to e will be returned.
 * @return the sound list element
 */
void *ndk_sound_list_get_next(struct sound_list *q, void *e);


/*======================== END DOUBLE LINKED LIST ===========================*/


/**
 * Suspend playback of all sequences and streams immediately.
 *
 * NOTE: This function does a lot more that just stop all sound playback. This
 * function needs more reversing.
 */
void ndk_sound_stop_all(void);

/**
 * Must be called to start playback of sound sources. It also manages updates
 * of volume fades so it must be called at regular intervals.
 */
void ndk_sound_process(void);

/**
 * Initialize sound library.
 *
 * Must be called once before any subsequent calls to sound functions are made.
 */
void ndk_sound_init(void);

/**
 * Used when the player allocates memory from the SDAT heap. It passed as an
 * argument to ndk_sound_sdat_heap_alloc.
 *
 * NOTE: There are several of these functions in the library. They are used for
 * different types of sound data.
 *
 * NOTE: Don't call this function directly it's here only for reference.
 *
 * @param mem
 * @param size
 * @param arch
 * @param fat_id
 */
void ndk_sound_player_free_notify(void *mem, int size,
                                  struct sound_sdat_arch *arch,
                                  int fat_id);

/**
 * Initialize state for all players.
 *
 * NOTE: Called from ndk_sound_init so there is no need to call it directly.
 * It's defined here only for reference.
 */
void ndk_sound_init_players(void);

/**
 * Write to a variable to the player that's assigned to a handle.
 *
 * @param handle
 * @param var variable index [0-15]
 * @param value value to put in the variable.
 * @return true if success, false otherwise.
 */
bool ndk_sound_handle_write_var(struct sound_handle *handle, int var,
                                unsigned short value);

/**
 * Read a variable from the player that's assigned to a handle.
 *
 * @param handle
 * @param var variable index [0-15]
 * @param dst location where to put the contents of the variable.
 * @return true if success, false otherwise.
 */
bool ndk_sound_handle_read_var(struct sound_handle *handle, int var,
                               unsigned short *dst);

/**
 * Get the SEQ id assigned to a specific handle.
 *
 * @param handle
 * @return the id or -1 if it's not a SEQ element.
 */
unsigned short ndk_sound_handle_get_seq_id(struct sound_handle *handle);

/**
 * Assign a SEQARC SEQ to a specific handle.
 *
 * NOTE: Probably a helper function don't call it directly. It's defiend here
 * only for reference.
 *
 * @param handle
 * @param seqarc_id
 * @param seq_id
 */
void ndk_sound_handle_set_seqarc_id(struct sound_handle *handle,
                                 unsigned short arc_id, unsigned short seq_id);

/**
 * Assign a SEQ to a specific handle.
 *
 * NOTE: Probably a helper function don't call it directly. It's defiend here
 * only for reference.
 *
 * @param handle
 * @param seq_id
 */
void ndk_sound_handle_set_seq_id(struct sound_handle *handle,
                                 unsigned short seq_id);

/**
 * Set the tempo of a SEQ assigned to a specific handle.
 *
 * @param handle
 * @param tempo
 */
void ndk_sound_handle_set_tempo(struct sound_handle *handle,
                                unsigned int tempo);

/**
 * Set the pitch of induvidual tracks of a SEQ assigned to a specific handle.
 *
 * @param handle
 * @param tracks bit array of all 16 tracks
 * @param pitch
*/
void ndk_sound_handle_set_pitch(struct sound_handle *handle,
                                unsigned short tracks, unsigned int pitch);

/**
 * Mute induvidual tracks in a SEQ assigned to a specific handle.
 *
 * @param handle
 * @param tracks bit array of all 16 tracks
 * @param mute false:no, true:yes
 */
void ndk_sound_handle_mute_tracks(struct sound_handle *handle,
                                  unsigned short tracks, bool mute);

/**
 * Set the channel priority of a SEQ assigned to a specific handle.
 *
 * @param handle
 * @param priority
 */
void ndk_sound_handle_set_channel_priority(struct sound_handle *handle,
                                           unsigned char cpr);

/**
 * Fade to a new volume of a SEQ assigned to a specific handle.
 *
 * This function can be used to create fade-in and fade-out effects.
 *
 * @param handle
 * @param volume
 * @param duration
 */
void ndk_sound_handle_fade_volume(struct sound_handle *handle,
                                  unsigned char volume, int duration);

/**
 * Set the volume of a SEQ assigned to a specific handle.
 *
 * @param handle
 * @param volume
 */
void ndk_sound_handle_set_volume(struct sound_handle *handle,
                                 unsigned char volume);

/**
 * Query if a SEQ is active.
 *
 * It is considered active as soon as it's added using the ndk_sound_add_source_seq
 * function. Note here that at this stage playback hasn't started yet.
 *
 * It is considered inactive as soon as the end of the sequence has been
 * reached or playback has been stopped using one of the stop functions.
 *
 * @param seq_id
 * @return true if seq is active, false otherwise.
 */
bool ndk_sound_seq_is_active(int seq_id);

/**
 * Query if a player is used for playing sounds.
 *
 * The player will be considered active as soon it has been assigned a sound
 * sequence for playback, this happens after a call to one of the
 * ndk_sound_add_source_xxx functions.
 *
 * It will be considered inactive as soon as the end of the sound sequence has
 * been reached or playback has been stopped.
 *
 * @param player
 * @return true if player is active, false otherwise.
 */
bool ndk_sound_player_is_active(int player);

/**
 * Unreference this handle.
 *
 * NOTE: Used by the ndk_sound_add_source_xxx probably a helper function.
 * Don't call this function directly. It's defined here only for reference.
 *
 * @param handle
 */
void ndk_sound_handle_clear(struct sound_handle *handle);

/**
 * Initialize a sound handle.
 *
 * Call this function before using it for sound playback tasks.
 *
 * @param handle
 */
void ndk_sound_handle_init(struct sound_handle *handle);

/**
 * Pause playback for a specific player
 *
 * @param player
 * @param pause pause or play
 */
void ndk_sound_player_pause(int player, bool pause);

/**
 * Pause playback for a specific handle
 *
 * @param handle
 * @param pause pause or play
 */
void ndk_sound_handle_pause(struct sound_handle *handle, bool pause);

/**
 * Stop playback of all SEQs.
 *
 * @param duration fade-out duration.
 */
void ndk_sound_seq_stop_all(int duration);

/**
 * Stop playback of a SEQ.
 *
 * @param seq_id
 * @param duration fade-out duration
 */
void ndk_sound_seq_stop(int seq_id, int duration);

/**
 * Stop a playback for a specific player.
 *
 * @param id
 * @param duration fade-out duration
 */
void ndk_sound_player_stop(int id, int duration);

/**
 * Stop playback for a specific handle.
 *
 * @param handle
 * @param duration fade-out duration
 */
void ndk_sound_handle_stop(struct sound_handle *handle, int duration);

/**
 * Open a SDAT archive.
 *
 * Open a SDAT archive and optionally read the FAT, Info and Symbol
 * blocks.
 *
 * @param arch
 * @param filename
 * @param heap where the FAT, Info and Symbol blocks are stored. If heap is
 *        null only the header is read.
 * @param load_symbols if true load the symbol block as well.
 * @return true on success, false otherwise
 */
bool ndk_sound_open_sdat_archive(struct sound_sdat_arch *arch, char *filename,
                                 struct sound_sdat_heap *heap,
                                 bool load_symbols);

/**
 * Load the SDAT header.
 *
 * Read the SDAT header and optionally read the FAT, Info and Symbol blocks.
 * The blocks are stored in memory allocated from the supplied heap using an
 * hardcoded heap id of 0.
 *
 * NOTE: Helper function for ndk_sound_open_sdat_archive.
 *
 * @param arch
 * @param heap where the FAT, Info and Symbol blocks are stored. If heap is
 *        null only the header is read.
 * @param load_symbols if true load the symbol block as well.
 * @return true on success, false otherwise
 */
bool ndk_sound_load_sdat_header(struct sound_sdat_arch *arch,
                                struct sound_sdat_heap *heap,
                                bool load_symbols);

/**
 * Get the FAT handle from the current SDAT archive.
 *
 * NOTE: Probably a helper function.
 *
 * @param[out] out
 */
void ndk_sound_get_fat_handle_from_sdat_archive(struct fat_handle *out);

/**
 * Read data from a file in the SDAT archive. This function is used to cache
 * sound data in RAM for (repeated) playback.
 *
 * NOTE: Probably a helper function.
 *
 * @param fat_id
 * @param dest destination buffer.
 * @param count number of bytes to read.
 * @param offset offset within the file to read from in bytes.
 * @return number of bytes read or 0 if operation failed.
 */
int ndk_sound_read_data_from_sdat_archive(int fat_id, void *dest,
                                          int count, int offset);

/**
 * Initialize a memory heap for sdat archive sound data.
 *
 * The SDAT library has an internal heap to manage all data that is needed for
 * sound playback. A region of memory must be supplied.
 *
 * @param data_buffer
 * @param size
 * @return pointer to the heap structure or null if operation failed
 */
struct sound_sdat_heap *ndk_sound_sdat_heap_init(void *data_buffer, int size);

/**
 * Free used memory.
 *
 * Frees memory by destroying all groups of allocated blocks from the top of the
 * LIFO structure to and including a requested level.
 *
 * @param heap
 * @param level A level in the LIFO structure.
 */
void ndk_sound_sdat_heap_free(struct sound_sdat_heap *heap, int level);

/**
 * Create a group to group allocated blocks with.
 *
 * All subsequent allocations will use this group until another call to this
 * function is made. Use this function to group allocations. Which can then
 * later be freed using a single call to ndk_sound_sdat_heap_free.
 *
 * @param heap
 * @return level of the newly create group.
 */
int ndk_sound_sdat_heap_new_group(struct sound_sdat_heap *heap);

/**
 * Allocate memory for SDAT data.
 *
 * NOTE: Helper function. There will probably never be a need to call this
 * function directly. It's defined here only for reference.
 *
 * @param heap
 * @param size
 * @param free_notify
 * @param arch
 * @param id
 * @return pointer to allocated memory or null i operation failed
 */
void *ndk_sound_sdat_heap_alloc(struct sound_sdat_heap *heap, int size,
                                sdat_heap_free_fn free_notify,
                                struct sound_sdat_arch *arch, int id);

// TODO: rename to ndk_sound_fat_set_ram_location
/**
 * Set the RAM location of a SDAT archive file.
 *
 * NOTE: Helper function
 *
 * @param fat_id
 * @param address Location of the file data in RAM.
 */
void ndk_sound_set_sdat_fat_rec_ram_address(int fat_id, void *address);

/**
 * Get the RAM location of a SDAT archive file.
 *
 * NOTE: Helper function
 *
 * @param fat_id
 * @return RAM address or null if fat_id is invalid or if the file hasn't been
 * loaded to RAM.
 */
void *ndk_sound_get_sdat_fat_rec_ram_address(int fat_id);

/**
 * Get the size of a file in the SDAT archive.
 *
 * NOTE: Helper function
 *
 * @param fat_id
 * @return size or 0 if fat_id is invalid
 */
int ndk_sound_get_file_size_from_sdat_archive(int fat_id);

/**
 * Get the offset of a file the within the SDAT archive.
 *
 * NOTE: Helper function
 *
 * @param fat_id
 * @return offset or 0 if fat_id is invalid
 */
int ndk_sound_get_file_offset_from_sdat_archive(int fat_id);

/**
 * Get file info from a cached SDAT archive header.
 *
 * NOTE: Helper functions
 *
 * @param id
 * @return reference to the info structure or NULL if the id is invalid.
 */
struct sound_info_group *ndk_sound_get_info_group_from_sdat_archive(int id);
struct sound_info_player2 *ndk_sound_get_info_player2_from_sdat_archive(int id);
struct sound_info_player *ndk_sound_get_info_player_from_sdat_archive(int id);
struct sound_info_strm *ndk_sound_get_info_strm_from_sdat_archive(int id);
struct sound_info_wavearc *ndk_sound_get_info_wavearc_from_sdat_archive(int id);
struct sound_info_bank *ndk_sound_get_info_bank_from_sdat_archive(int id);
struct sound_info_seqarc *ndk_sound_get_info_seqarc_from_sdat_archive(int id);
struct sound_info_seq *ndk_sound_get_info_seq_from_sdat_archive(int id);

/**
 * Get a SEQ record from the current SDAT archive
 *
 * @param id
 * @return SEQ record or NULL if operation failed.
 */
struct sound_seq *ndk_sound_get_seq_from_sdat_archive(int id);

/**
 * Get the current sdat archive.
 *
 * @return the sdat archive.
 */
struct sound_sdat_arch *ndk_sound_get_current_sdat_archive(void);

/**
 * Set the current sdat archive.
 *
 * @param arch
 * @return the previously set sdat archive
 */
struct sound_sdat_arch *ndk_sound_set_current_sdat_archive(
                                                 struct sound_sdat_arch *arch);

/**
 * Load a file from a SDAT archive.
 *
 * NOTE: Most likely a helper function
 *
 * @param fat_id
 * @param free_notify
 * @param arch
 * @param id usually set to fat_id
 * @param heap
 * @return address of the loaded file or NULL if the operation failed.
 */
void *ndk_sound_load_file_from_sdat_archive(int fat_id,
                                            sdat_heap_free_fn free_notify,
                                            struct sound_sdat_arch *arch,
                                            int id,
                                            struct sound_sdat_heap *heap);

/**
 * Load sound resources from the SDAT archive.
 *
 * NOTE: Due to the wierd API of these functions I suspect that they are all
 * internal helper functions to ndk_sound_load_group_from_sdat_archive and
 * ndk_sound_add_source_XXX functions.
 *
 * NOTE: ndk_sound_load_group_from_sdat_archive calls these functions with
 * mode == true and out_loc == NULL. ndk_sound_add_source_XXX calls these
 * functions with mode == false and out_loc != NULL.
 *
 * @param id resource id
 * @param flags see sound_info_group.Group[n].type entry
 * @param heap heap to use when loading sound data
 * @param mode if the heap is managed by sdat heap or a player sub-heap
 * @param out_loc if non-null the loaded entrys address will be written to
 * this location.
 * @return SOUND_SUCCESS on success, failure code otherwise.
 */
int ndk_sound_load_wavearc_from_sdat_archive(int id, unsigned char flags,
                                             struct sound_sdat_heap *heap,
                                             bool mode, void *out_loc);

int ndk_sound_load_bank_from_sdat_archive(int id, unsigned char flags,
                                          struct sound_sdat_heap *heap,
                                          bool mode, void *out_loc);

int ndk_sound_load_seqarc_from_sdat_archive(int id, unsigned char flags,
                                            struct sound_sdat_heap *heap,
                                            bool mode, void *out_loc);

int ndk_sound_load_seq_from_sdat_archive(int id, unsigned char flags,
                                         struct sound_sdat_heap *heap,
                                         bool mode, void *out_loc);

/**
 * Load a Group entry from a SDAT archive
 *
 * Used to preload sound data for later playback. All sub-dependencies to
 * resources are automatically loaded So there is no need to track these for
 * your self. See SOUND_INFO_GROUP_FLAG_XXX definitions for how sub-dependencies
 * are defined when creating the SDAT archives.
 *
 * @param id
 * @param heap
 * @return SOUND_SUCCESS on success, failure code otherwise.
 */
int ndk_sound_load_group_from_sdat_archive(int id,
                                           struct sound_sdat_heap *heap);

/**
 * Load a group from an SDAT archive.
 *
 * Seems to be an convenience version of
 * ndk_sound_load_group_from_sdat_archive.
 * 
 * @param id
 * @param heap
 * 
 * return true on success, false otherwise.
 */
bool ndk_sound_sdat_load_group(int id, struct sound_sdat_heap *heap);

/**
 * Queue a SEQ from a sequence archive for playback
 *
 * If the assigned player has its own heap, sound resources will be loaded
 * dynamically. If not all resource must be loaded using any of
 * the ndk_sound_load_XXX functions before a call to this function.
 *
 * NOTE: If the handle is already referencing another sound source it will be
 * unreferenced i.e. a handler can only reference one sound source at a time.
 *
 * @param handle
 * @param seqarc_id
 * @param seq_id
 * @return true on success, false otherwise.
 */
bool ndk_sound_add_source_seqarc(struct sound_handle *handle, int seqarc_id,
                                 int seq_id);

/**
 * Queue a SEQ for playback
 *
 * If the assigned player has its own heap, sound resources will be loaded
 * dynamically. If not all resource must be loaded using any of the
 * ndk_sound_load_XXX functions before a call to this function.
 *
 * NOTE: If the handle is already referencing another sound source it will be
 * unreferenced i.e. a handler can only reference one sound source at a time.
 *
 * @param handle
 * @param seq_id
 * @return true on success, false otherwise.
 */
bool ndk_sound_add_source_seq(struct sound_handle *handle, int seq_id);

/**
 * Initialize players from current SDAT archive player entries.
 *
 * Should called be after ndk_sound_open_sdat_archive to setup all players for
 * subsequent playback.
 *
 * @param heap
 * @return true on success, false otherwise.
 */
bool ndk_sound_sdat_init_seq_players(struct sound_sdat_heap *heap);

/**
 * The worker function for the sound thread.
 *
 * NOTE: It's arguments is a pointer sound_thread structure.
 * NOTE: Don't call this function directly, it's defined only for reference.
 */
void ndk_sound_worker_thread_fn(void *);

/**
 * This function will create and start the worker thread for the sound library.
 * It's used for stream playback.
 *
 * NOTE: Don't call this function directly, it's defined only for reference.
 *
 * @param unk0 pointer to a huge structure that has a thread struct as its
 * first member.
 * @param priority
 */
void ndk_sound_create_sdat_worker_thread(void *unk0, int priority);

/**
 * Initialize all stream players.
 *
 * Should called be after ndk_sound_open_sdat_archive to setup all stream
 * players for subsequent playback.
 * 
 * NOTE: The game calls this function with a priority value of 10. Which is a
 * bit odd since no streams are present in the SDAT archive for the game.
 *
 * @param priority priority of the stream worker thread
 * @param heap
 */
void ndk_sound_sdat_init_stream_players(int priority,
                                        struct sound_sdat_heap *heap);

/**
 * Get a SEQ from a SEQARC
 *
 * NOTE: Don't call this function directly, it's defined only for reference.
 *
 * @param seqarc
 * @param seq
 * @return the SEQ data structure
 */
struct sound_info_seq *ndk_sound_get_sdat_seq_from_seqarc(
                                struct sound_info_seqarc *seqarc, int seq_id);

/**
 * Check if a volume fade has reached its set duration.
 *
 * @param info
 * @return true if the current duration is greater or equal to the set duration.
 */
bool ndk_sound_fade_info_duration_reached(struct sound_fade_info *info);

/**
 * Increase the current duration by one.
 *
 * NOTE: Call this function then ndk_sound_fade_info_calc_volume to get the
 * current fade volume level.
 *
 * @param info
 */
void ndk_sound_fade_info_advance_one(struct sound_fade_info *info);

/**
 * Calculate the current fade volume level.
 *
 * @param info
 * @return the current volume level
 */
int ndk_sound_fade_info_calc_volume(struct sound_fade_info *info);

/**
 * Initialize a fade info object.
 *
 * NOTE: Helper function to the ndk_sound_handle_set_volume and
 * ndk_sound_sdat_stop_xxx functions. Don't call this function directly it's
 * defined here only for reference.
 *
 * @param info
 * @param new_volume
 * @param duration
 */
void ndk_sound_fade_info_init(struct sound_fade_info *info, int new_volume,
                              int duration);

/**
 * Set all members to zero.
 *
 * NOTE: Helper function.
 *
 * @param info
 */
void ndk_sound_fade_info_clear(struct sound_fade_info *info);

#endif // SOUND_INCLUDE_FILE
