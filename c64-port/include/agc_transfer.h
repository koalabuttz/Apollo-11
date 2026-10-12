/* SPDX-License-Identifier: MIT */
#ifndef AGC_TRANSFER_H
#define AGC_TRANSFER_H
/* Native control-transfer ABI (lane A). See docs/NATIVE_TRANSFER_ABI.md.
 *
 * The original program moves control through addresses held as data: a CADR
 * after `TC BANKCALL`, a return address in Q or BUF2, a 2CADR in a restart
 * table. A native module that computes such a destination does not stop at an
 * AGC address. It hands the destination to the application's registry, which
 * names the native C routine compiled for it, and the trampoline calls it.
 *
 * Inline operands (the word after a `TC`, read through Q) are the caller's
 * compiled constants: agc_site_operand tables generated from the original
 * source by tools/agc_sites_generate.py. Nothing here reads the original rope.
 *
 * Registries are static, sorted by physical address, unique and resident. */
#include <stdbool.h>
#include "overlay_runtime.h"
#include "executive.h"
#include "agc_word.h"

#define AGC_TRANSFER_NONE UINT16_C(0xffff)

/* Physical fixed word bank*1024+offset (0..40959) for a logical address in
 * fixed memory (02000..07777) under FB and the channel-7 SUPERBNK bits, or
 * AGC_TRANSFER_NONE for an erasable address. */
uint16_t agc_fixed_physical(agc_word logical, agc_word fb, agc_word superbank);

/* Key of an erasable continuation (Z below 02000; 01400-01777 under EB):
 * AGC_TRANSFER_ERASABLE_BASE + the physical erasable address. A file that
 * stores a TC into erasable for later execution (RESTARTS' GOLOC words)
 * registers a routine at that key for exactly the words it stores. */
#define AGC_TRANSFER_ERASABLE_BASE UINT16_C(40960)
uint16_t agc_erasable_point(agc_word logical, agc_word eb);

typedef struct { uint16_t physical; agc_word word; } agc_site_operand;
bool agc_site_lookup(const agc_site_operand *table, uint16_t count,
                     uint16_t physical, agc_word *word);

/* A native routine entered at its original address. It runs with the flight
 * image in its entry state, performs its translated body, and leaves Z (and FB,
 * Q, LOC, ...) where the original would continue. io is the mission's device
 * record (p66_bias_io or cm_native_io); program gives access to the compiled
 * operand tables and the application's user data. */
struct agc_native_program;
typedef enum exec_status (*agc_routine)(const struct agc_native_program *, apollo_overlay_state *, void *io);
typedef struct { uint16_t physical; agc_routine call; } agc_routine_entry;
typedef struct { const agc_routine_entry *entries; uint16_t count; } agc_routine_table;

/* Basic routines are keyed by the physical address of Z (or its erasable
 * key). When Z reaches the program's interpreter fetch point, the
 * continuation is the interpretive routine keyed by the physical address of
 * LOC under BANKSET instead. */
typedef struct agc_native_program {
    const agc_routine_table *basic; uint8_t basic_count;
    const agc_routine_table *interpretive; uint8_t interpretive_count;
    const agc_site_operand *operands; uint16_t operand_count;
    uint16_t interpreter_fetch;   /* physical Z, or AGC_TRANSFER_NONE */
    uint16_t loc, bankset;        /* erasable addresses of LOC and BANKSET */
    /* Passive constant reads (SUPDACAL, display of fixed memory); may be NULL. */
    bool (*fixed_data)(const struct agc_native_program *, uint16_t physical, agc_word *word);
    void *user;
} agc_native_program;

/* Native routine for the current Z (or LOC at the fetch point), or NULL. */
agc_routine agc_transfer_resolve(const agc_native_program *, const apollo_overlay_state *,
                                 agc_word superbank);
/* The inline operand at logical address q (the word after the caller's TC),
 * from the program's compiled site table or from erasable memory. */
bool agc_transfer_operand(const agc_native_program *, const apollo_overlay_state *,
                          agc_word superbank, agc_word q, agc_word *word);
/* Point the trampoline compares against stop: the physical fixed address of
 * Z, its erasable key when Z is erasable, or LOC under BANKSET when Z is the
 * interpreter fetch point. */
uint16_t agc_transfer_point(const agc_native_program *, const apollo_overlay_state *, agc_word superbank);
/* Trampoline: call the routine at the current point, then the routine at its
 * continuation, and so on, until the point equals stop (the caller's own
 * continuation) or a routine reports anything but EXEC_OK. Each routine returns
 * before the next is entered, so C stack depth stays constant. An unmapped
 * continuation is EXEC_UNMAPPED with the image left at it. */
enum exec_status agc_transfer_run(const agc_native_program *, apollo_overlay_state *, void *io,
                                  const agc_word *superbank, uint16_t stop_physical,
                                  uint16_t max_steps, uint16_t *steps);
#endif
