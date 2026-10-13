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

struct agc_native_program;
/* RESUME runs the instruction BRUPT holds, then continues at ZRUPT. A native
 * RESUME continues at Z = ZRUPT-1, which is the same thing only when BRUPT is
 * the word at ZRUPT-1 under the banks the interrupted program gets back (fb,
 * eb, superbank). The interrupt hardware always saves that word; code that
 * writes BRUPT itself (ENDJASK, ALARM_AND_ABORT, P70) may not. False when the
 * word can be read (erasable, or fixed through the program's fixed_data) and
 * differs, so the routine refuses rather than resume at the wrong point.
 * p and s must not be NULL. */
bool agc_resume_exact(const struct agc_native_program *p, const apollo_overlay_state *s, agc_word fb, agc_word eb,
                      agc_word superbank);

/* Key of an erasable continuation (Z below 02000; 01400-01777 under EB):
 * AGC_TRANSFER_ERASABLE_BASE + the physical erasable address. A file that
 * stores a TC into erasable for later execution (RESTARTS' GOLOC words)
 * registers a routine at that key for exactly the words it stores. */
#define AGC_TRANSFER_ERASABLE_BASE UINT16_C(40960)
uint16_t agc_erasable_point(agc_word logical, agc_word eb);

typedef struct { uint16_t physical; agc_word word; } agc_site_operand;
bool agc_site_lookup(const agc_site_operand *table, uint16_t count,
                     uint16_t physical, agc_word *word);

/* CPU state a routine may need beyond the erasable image and the mission's
 * device record: output channel 11 (the Executive's idle loop turns off COMP
 * ACTY), whether an interrupt is in progress (yaAGC InIsr; Waitlist tasks
 * run inside T3RUPT, and TASKOVER's RESUME ends it), and the RCS jet
 * channels 5 and 6 and channel 13 (bit 15 enables the TIME6 counter) that
 * the LM T6RUPT programs drive, input channel 15 (the DSKY key code
 * KEYRUPT1 reads), output channel 10 (the last DSKY relay word T4RUPT
 * wrote) and input channel 32 (the PRO key T4RUPT samples), output channels
 * 12 and 14 (IMU control, CDU and gyro drive), input channels 30, 31 and 33
 * (IMU, hand controller, temperature and PIPA status) and the scaler
 * channels 3 and 4 (HISCALAR, LOSCALAR). Input channels hold what READ
 * returns, output channels the last word written; channel 13 holds what the
 * CPU can write (043777: the trap-reset bits never stay set). One layout for
 * every lane: new channels are appended here by the ABI owner. Zero-initialize
 * with `agc_hardware hw = {0};`, never a partial positional list. */
typedef struct { agc_word channel11; uint16_t in_interrupt; agc_word channel5, channel6, channel13, channel15, channel10, channel32, channel12, channel14, channel30, channel33, channel31, channel3, channel4; } agc_hardware;

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
    /* NULL when the program has no routine that needs it; such routines then
     * refuse with EXEC_OUT_OF_SCOPE. */
    agc_hardware *hardware;
} agc_native_program;

/* Pending right-hand order. A pair whose left order is RTB returns through
 * DANZIG (LM 06061, CM 06030) with the right order's code still in EDOP
 * (CCS EDOP positive: TCF OPJUMP) and LOC at the RTB's operand word, the
 * pair word +1. That point is interpretive too: when Z is DANZIG, EDOP is
 * positive and the program's first interpretive table is the mission's
 * marker table, the point is LOC under BANKSET. The file owning the pair
 * registers a routine there, in its interpretive table, that accepts exactly
 * the EDOP code its pair leaves, performs DANZIG's BBANK load and the right
 * order, and refuses anything else with EXEC_OUT_OF_SCOPE. An RTB operand
 * word is never an order-pair word, so these keys never meet NEWOPS keys.
 * Programs without the marker resolve DANZIG by Z as before.
 * On the C64 the check costs about 150 bytes in every program linking this
 * file, so it is compiled (and the marker tables defined) only with
 * -DAGC_TRANSFER_DANZIG_MARKER; a C64 program registering a marker without
 * it does not compile. Host builds always have it. */
#if !defined(__mos__) && !defined(AGC_TRANSFER_DANZIG_MARKER)
#define AGC_TRANSFER_DANZIG_MARKER 1
#endif
#define AGC_LM_DANZIG UINT16_C(3121)  /* physical 06061 */
#define AGC_CM_DANZIG UINT16_C(3096)  /* physical 06030 */
#define AGC_EDOP 023u
#ifdef AGC_TRANSFER_DANZIG_MARKER
extern const agc_routine_table agc_lm_danzig_marker, agc_cm_danzig_marker;
#endif

/* Native routine for the current Z (or LOC at the fetch point, or at a
 * pending DANZIG), or NULL. */
agc_routine agc_transfer_resolve(const agc_native_program *, const apollo_overlay_state *,
                                 agc_word superbank);
/* The inline operand at logical address q (the word after the caller's TC),
 * from the program's compiled site table or from erasable memory. */
bool agc_transfer_operand(const agc_native_program *, const apollo_overlay_state *,
                          agc_word superbank, agc_word q, agc_word *word);
/* Point the trampoline compares against stop: the physical fixed address of
 * Z, its erasable key when Z is erasable, or LOC under BANKSET when Z is the
 * interpreter fetch point or a pending DANZIG. */
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
