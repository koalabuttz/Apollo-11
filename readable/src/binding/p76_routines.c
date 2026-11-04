/* SPDX-License-Identifier: MIT */
/* The readable P76 (src/p76/) as routines of each mission's flight
 * program, under the table names and keys of the faithful port's: the
 * compiled files (c64-port/src/lm_p76_file.c and cm_p76_file.c:
 * lm_p76_file_basic and cm_p76_file_basic, the basic code, and
 * lm_p76_file_interpretive and cm_p76_file_interpretive, the order pairs)
 * and the older modules (lm_p76_routines.c and cm_p76_routines.c), whose
 * basic tables the flight program tries first: lm_p76_basic_routines and
 * cm_p76_basic_routines here, at their keys. The older modules' order
 * pairs, which the flight program tries only after the files', are left
 * out (empty tables). All in bank 13.
 *
 * Plumbing only. Each readable routine ends where its code leaves it (at a
 * TC out of the file, a TC INTPRET or a TCF ENDP76). Both modules go on
 * through the TCF into ENDP76; a compiled file's key also follows a TC
 * INTPRET into the interpreter as far as NEWOPS, where the older module's
 * stops. An order pair is one pass of the readable interpreter over the
 * listing's words (interpret_listing()), which ends at the pair's DANZIG,
 * at the EXIT, or where a CALL leaves its table.
 *
 * The states the faithful routines take: FB bank 13 (the superbank latch
 * any value in range; the older modules' with its bit 0100 clear, Z at
 * their key and Q an ordinary word); at an order pair Z at NEWOPS,
 * BANKSET the BBANK, INTBIT15 the bank's bit 15 and FIXLOC one of the VAC
 * areas. Always ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range and every erasable word an ordinary one; at
 * an order pair NEWJOB, too, +0 or positive. The CM's compiled basic
 * routines also want a hardware record, and the CM's older module takes
 * the LM's kind of io record (p66_bias_io), as the flight program gives
 * it. A run the readable code refuses part way leaves the state as it
 * was. */
#include <setjmp.h>
#include <string.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../p76/p76.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"
#if !LUMINARY099
#include "p66_bias.h"
#endif

/* Keys: bank 13, and the address in it. */
#define P76_BANK 013u
#define KEY(z)   (uint16_t)(P76_BANK * 1024u + ((z) - 02000u))
#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif
#define SUPERBANK_BIT 0100u

/* The older modules' io record. */
typedef p66_bias_io older_io;

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, mission_io *io)
{
    bind_machine(program, state, io);
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The same for an older module's routine, whose io record has the latches
 * and nothing the routines read. */
static void bind_older(const agc_native_program *program, apollo_overlay_state *state, older_io *io)
{
    agc.memory = state->words;
    agc.superbank = &io->superbank;
    agc.interrupts = &io->interrupt_enabled;
    agc.dsalmout = NULL;
    agc.in_interrupt = program->hardware ? &program->hardware->in_interrupt : NULL;
    agc.channel_in = NULL;
    agc.channel_out = NULL;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states both kinds of routine take (the faithful basic_sane() and
 * the common part of its sane()). */
static bool sane_banks(void)
{
    unsigned i;

    if (FBANK != (word)(P76_BANK << 10) || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u ||
        (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

/* The routine, then on through a TCF ENDP76 into it, and for a compiled
 * file's key through a TC INTPRET into the interpreter. A run the readable
 * code refuses part way leaves the state as it was. */
static enum exec_status run(void (*routine)(void), bool into_interpreter,
                            apollo_overlay_state *state, void *io, size_t io_size)
{
    apollo_overlay_state before = *state;
    union {
        mission_io mission;
        older_io older;
    } kept;

    memcpy(&kept, io, io_size);
    if (setjmp(back)) {
        *state = before;
        memcpy(io, &kept, io_size);
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    if (Z == ENDP76)
        endp76();
    else if (Z == INTPRET && into_interpreter)
        intpret();
    return EXEC_OK;
}

/* A compiled file's basic routine runs from its key, whatever Z holds (the
 * trampoline calls it there). */
static enum exec_status basic(void (*routine)(void), const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    mission_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
#if !LUMINARY099
    if (!program->hardware) return EXEC_OUT_OF_SCOPE;
#endif
    bind(program, state, io);
    if (!sane_banks()) return EXEC_OUT_OF_SCOPE;
    return run(routine, true, state, io, sizeof *io);
}

/* An older module's: at Z its key's, with the superbank bit clear and Q,
 * too, an ordinary word. */
static enum exec_status older(void (*routine)(void), word label, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    older_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind_older(program, state, io);
    if (Z != label || Q > 077777u || (*agc.superbank & SUPERBANK_BIT) || !sane_banks())
        return EXEC_OUT_OF_SCOPE;
    return run(routine, false, state, io, sizeof *io);
}

#define BASIC(name)                                                                                \
    static enum exec_status basic_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return basic(name, program, state, device);                                                \
    }
#define OLDER(name, label)                                                                         \
    static enum exec_status older_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return older(name, label, program, state, device);                                         \
    }
#if LUMINARY099
#define DELTA_V_AT  (P76 + 7u)
#define TIG_AT      (P76 + 015u)
#define PHASCHNG_AT (P76 + 0107u)
#define INTWAKE_AT  (P76 + 0121u)
#else
#define DELTA_V_AT  (P76 + 2u)
#define TIG_AT      (P76 + 010u)
#define PHASCHNG_AT (P76 + 0104u)
#define INTWAKE_AT  (P76 + 0116u)
#endif
BASIC(p76)
BASIC(flash_delta_v)
BASIC(delta_v_terminate)
BASIC(delta_v_proceed)
BASIC(delta_v_recycle)
BASIC(tig_terminate)
BASIC(tig_proceed)
BASIC(tig_recycle)
BASIC(p76_phaschng)
BASIC(p76_reintflg)
BASIC(p76_reintflg_intpret)
BASIC(p76_intwake)
BASIC(endp76)
OLDER(p76, P76)
OLDER(flash_delta_v, DELTA_V_AT)
OLDER(delta_v_terminate, DELTA_V_AT + 3u)
OLDER(delta_v_proceed, DELTA_V_AT + 4u)
OLDER(delta_v_recycle, DELTA_V_AT + 5u)
OLDER(flash_tig, TIG_AT)
OLDER(tig_terminate, TIG_AT + 3u)
OLDER(tig_proceed, TIG_AT + 4u)
OLDER(tig_recycle, TIG_AT + 5u)
OLDER(tig_intpret, TIG_AT + 6u)
OLDER(p76_phaschng, PHASCHNG_AT)
OLDER(p76_reintflg, PHASCHNG_AT + 2u)
OLDER(p76_reintflg_intpret, PHASCHNG_AT + 4u)
OLDER(p76_intwake, INTWAKE_AT)
OLDER(endp76, ENDP76)
#if LUMINARY099
BASIC(p76_intpret)
OLDER(p76_intpret, P76 + 2u)
#endif
#undef BASIC
#undef OLDER

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
#if LUMINARY099
    &P76_CODE,
#endif
    &UPDATE_CODE, &ATOPOTH_CODE, &OUT_CODE, &P76SUB1_CODE};

/* The table LOC is in, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (LOC >= codes[i]->first && LOC < codes[i]->first + codes[i]->count) return codes[i];
    return NULL;
}

static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at();
    if (Z != NEWOPS || !code || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) ||
        NEWJOB > 037777u || FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u != 0u ||
        !sane_banks())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each interpretive key: the word a pass starts at. */
#define P(z) {KEY(z), pair}
static const agc_routine_entry pairs[] = {
#if LUMINARY099
    P(02212u), P(02214u), P(02215u), /* P76 +3 ... */
    P(02233u), P(02235u), P(02237u), P(02241u), P(02242u), P(02243u), P(02245u), P(02246u),
    P(02250u), P(02252u), P(02254u), P(02256u), P(02257u), P(02261u), P(02263u), P(02266u),
    P(02270u), P(02272u), P(02274u), P(02275u), P(02300u), P(02302u), P(02304u), P(02306u),
    P(02307u), P(02311u), P(02313u), P(02315u), /* DLOAD TIG ... MINIRECT's EXIT */
    P(02323u), P(02325u),                       /* CALL ATOPOTH, SSP */
    P(02332u),                                  /* OUT */
    P(02340u), P(02343u), P(02347u),            /* P76SUB1 */
#else
    P(02110u), P(02112u), P(02114u), P(02116u), P(02117u), P(02120u), P(02122u), P(02123u),
    P(02125u), P(02127u), P(02131u), P(02133u), P(02134u), P(02136u), P(02140u), P(02143u),
    P(02145u), P(02147u), P(02151u), P(02152u), P(02155u), P(02157u), P(02161u), P(02163u),
    P(02165u), P(02166u), P(02170u), P(02172u), P(02174u), /* DLOAD TIG ... MINIRECT's EXIT */
    P(02202u), P(02204u),                                  /* CALL ATOPOTH, SSP */
    P(02211u),                                             /* OUT */
    P(02222u), P(02225u), P(02230u),                       /* P76SUB1 */
#endif
};
#undef P

#define B(z, name) {KEY(z), basic_##name}
static const agc_routine_entry basic_keys[] = {
    B(P76, p76),
#if LUMINARY099
    B(P76 + 2u, p76_intpret),
#endif
    B(DELTA_V_AT, flash_delta_v),
    B(DELTA_V_AT + 3u, delta_v_terminate),
    B(DELTA_V_AT + 4u, delta_v_proceed),
    B(DELTA_V_AT + 5u, delta_v_recycle),
    B(TIG_AT + 3u, tig_terminate),
    B(TIG_AT + 4u, tig_proceed),
    B(TIG_AT + 5u, tig_recycle),
    B(PHASCHNG_AT, p76_phaschng),
    B(PHASCHNG_AT + 2u, p76_reintflg),
    B(PHASCHNG_AT + 4u, p76_reintflg_intpret),
    B(INTWAKE_AT, p76_intwake),
    B(ENDP76, endp76),
};
#undef B

#define O(z, name) {KEY(z), older_##name}
static const agc_routine_entry older_keys[] = {
    O(P76, p76),
#if LUMINARY099
    O(P76 + 2u, p76_intpret),
#endif
    O(DELTA_V_AT, flash_delta_v),
    O(DELTA_V_AT + 3u, delta_v_terminate),
    O(DELTA_V_AT + 4u, delta_v_proceed),
    O(DELTA_V_AT + 5u, delta_v_recycle),
    O(TIG_AT, flash_tig),
    O(TIG_AT + 3u, tig_terminate),
    O(TIG_AT + 4u, tig_proceed),
    O(TIG_AT + 5u, tig_recycle),
    O(TIG_AT + 6u, tig_intpret),
    O(PHASCHNG_AT, p76_phaschng),
    O(PHASCHNG_AT + 2u, p76_reintflg),
    O(PHASCHNG_AT + 4u, p76_reintflg_intpret),
    O(INTWAKE_AT, p76_intwake),
    O(ENDP76, endp76),
};
#undef O

static const agc_routine_entry none[] = {{0xFFFFu, NULL}};
#if LUMINARY099
const agc_routine_table lm_p76_file_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_p76_file_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_p76_basic_routines = {older_keys, COUNT(older_keys)};
const agc_routine_table lm_p76_interpretive_routines = {none, 0u};
#else
const agc_routine_table cm_p76_file_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_p76_file_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table cm_p76_basic_routines = {older_keys, COUNT(older_keys)};
const agc_routine_table cm_p76_interpretive_routines = {none, 0u};
#endif
