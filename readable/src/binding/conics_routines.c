/* SPDX-License-Identifier: MIT */
/* The readable CONIC_SUBROUTINES (src/conics/) as routines of each
 * mission's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_conic_compiled.c and cm_conics.c):
 * lm_conic_compiled_basic and _interpretive for the LM, cm_conics_basic
 * and _interpretive for the CM. The keys come from the faithful tables, in
 * their order (conics_keys.inc and cm_conics_keys.inc, which
 * tools/p20_p25_keys.py writes from them).
 *
 * Every key runs a readable routine.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (POLY, DANZIG) or reaches NEWOPS through a TC INTPRET. An order
 * pair is one pass of the readable interpreter over the listing's words
 * (interpret_listing()), in the part of the code LOC is in. The LM's pair
 * runs an RTB's routine and the rest of the pair, as the faithful one does:
 * TPMODE (src/rtb_op_codes/) or CHECKCTR. The CM's pass ends at the RTB;
 * the routine (CHECKCTR is a basic key here) ends at DANZIG, and the key
 * there goes on with the pair's right-hand order, which waits in EDOP (see
 * pending_orders).
 *
 * The states the faithful routines take: FB the key's bank, ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range
 * and every erasable word an ordinary one; but for the CM's basic code,
 * NEWJOB +0 or a core set and FIXLOC one of the VAC areas too; at an order
 * pair also Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15
 * (or, at a pending key, Z at DANZIG). A run the readable code refuses
 * part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../conics/conics.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE            0u   /* the LM's rope */
#define BASIC_WORK_AREA true /* the LM's basic keys check the work area too */
#else
#define ROPE            1u /* the CM's */
#define BASIC_WORK_AREA false
#endif

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The bank of KEPLER, DELTIME and CHECKCTR, and TPMODE's (FB), from their
 * FCADRs. */
#define CONICS_BANK ((unsigned)(I_DELTIME >> 10))
#define TPMODE_BANK ((word)(I_TPMODE & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The file reads and writes no channel. */
static word channel_in(unsigned number)
{
    (void)number;
    not_modelled();
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled();
}

/* The file's constants are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static bool bind(const agc_native_program *program, apollo_overlay_state *state, mission_io *io)
{
    if (!bind_machine(program, state, io)) return false;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
    return true;
}

/* The states the faithful routines take (their sane(), and the CM's
 * basic_sane(), which leaves out the work area). */
static bool sane(bool work_area)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (work_area && (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u || FIXLOC < 0401u ||
                      FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    {KEY(CONICS_BANK, DELTIME + 1u), deltime_after_exit},
    {KEY(CONICS_BANK, DELTIME + 027u), deltime_after_poly},
    {KEY(CONICS_BANK, DELTIME + 033u), deltime_after_second_exit},
    {KEY(CONICS_BANK, DELTIME + 061u), deltime_after_second_poly},
    {KEY(CONICS_BANK, POLYCOEF + 010u), getx_after_exit},
    {KEY(CONICS_BANK, POLYCOEF + 030u), getx_after_poly},
    {KEY(CONICS_BANK, ABTCONIC + 1u), abtconic_after_exit},
#if COMANCHE055
    {KEY(CONICS_BANK, CHECKCTR), checkctr},
#endif
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

/* The parts of the code written so far. */
static const struct interpretive_code *const codes[] = {
    &KEPLER_CODE,   &DELTIME_CODE,  &DELTIME_S_CODE, &DELTIME_C_CODE, &ITERATOR_CODE,
    &NEWSTATE_CODE, &TIMETHET_CODE, &PARAM_CODE,     &GETX_CODE,      &GETX_SERIES_CODE,
    &LAMBERT_CODE,  &TIMERAD_CODE,  &APSIDES_CODE,
};

/* The part of the code an address of a bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

#if COMANCHE055
/* The CM's keys a pair's right-hand order waits at in EDOP while its
 * left, an RTB, runs a routine: KEPLER's RTB BHIZ CHECKCTR, DELTIME's
 * two RTB PDDL TPMODEs and GETX's at 1/WLOOP. The routine ends at DANZIG, which finds the order
 * in EDOP and resumes the pass at LOC: EDOP holds the order's code plus
 * one. */
static const struct pending_order {
    uint16_t key;
    word edop;
} pending_orders[] = {
    {KEY(CONICS_BANK, KEPLERN + 0244u), ORDER_BHIZ + 1u}, /* BRNCHCTR +1 */
    {KEY(CONICS_BANK, DELTIME + 071u), ORDER_PDDL + 1u},
    {KEY(CONICS_BANK, DELTIME + 0100u), ORDER_PDDL + 1u},
    {KEY(CONICS_BANK, K_1_WLOOP + 2u), ORDER_PDDL + 1u},
};

static const struct pending_order *pending_order(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(pending_orders); ++i)
        if (pending_orders[i].key == key) return &pending_orders[i];
    return NULL;
}

/* The states the faithful port resumes such a pair from: Z at DANZIG, the
 * order in EDOP, BANKSET the pair's bank (FB may be any) with no E-bank
 * bits, and INTBIT15 the bank's bit 15. */
static bool at_danzig(unsigned bank, const struct pending_order *pending)
{
    return Z == DANZIG && EDOP == pending->edop && ((BANKSET >> 10) & 037u) == bank &&
           !(BANKSET & 01770u) && INTBIT15 == ((bank << 10) & 040000u) && sane(true);
}
#endif

/* The states the faithful CHECKCTR takes: ITERCTR, in the job's work area
 * (INDEX FIXLOC), an erasable word; the LM's, after the RTB, past the
 * central registers, with ZERO +0 and every word from Q on an ordinary
 * one, and the CM's not FB, Z or BBANK. */
static bool checkctr_sane(void)
{
    unsigned iterctr = indexed(ADDRESS_OF(ITERCTR), FIXLOC);

#if LUMINARY099
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || iterctr < 010u || iterctr >= 01400u) return false;
    for (i = Q_ADDRESS; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
#else
    return iterctr < 02000u && (iterctr < FBANK_ADDRESS || iterctr > BBANK_ADDRESS);
#endif
}

/* One pass; in the LM, after RTB TPMODE or RTB CHECKCTR, the routine and
 * its DANZIG, which goes on with the pair's other order. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
#if LUMINARY099
    void (*routine)(void);

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z == TPMODE && FBANK == TPMODE_BANK) {
        routine = tpmode;
    } else if (Z == CHECKCTR && FBANK == (word)(CONICS_BANK << 10)) {
        if (!checkctr_sane()) return EXEC_OUT_OF_SCOPE;
        routine = checkctr;
    } else {
        return EXEC_OK;
    }
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    routine();
#endif
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* A readable basic routine; a run it refuses part way leaves the state
 * as it was. */
static enum exec_status run_basic(void (*routine)(void), apollo_overlay_state *state,
                                  mission_io *io)
{
    mission_io kept = *io;
    apollo_overlay_state before = *state;

    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

/* Each basic key's routine. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    mission_io *io = device;

    if (!entry || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (FBANK != (word)((key / 1024u) << 10) || !sane(BASIC_WORK_AREA)) return EXEC_OUT_OF_SCOPE;
    if (entry->routine == checkctr && !checkctr_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* Each order pair's routine: one pass of the readable interpreter. */
static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    word at = (word)(02000u + key % 1024u);
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, at);
    enum exec_status status;
    bool started;

    if (!code || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
#if COMANCHE055
    if (pending_order(key))
        started = LOC == at && at_danzig(bank, pending_order(key));
    else
#endif
        started = Z == NEWOPS && FBANK == (word)(bank << 10) && LOC == at && BANKSET == BBANK &&
                  INTBIT15 == (FBANK & 040000u) && sane(true);
    if (!started) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = one_pass(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool conics_readable(uint16_t key, bool order_pair)
{
    if (order_pair) return code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL;
    return basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(KEY(bank, address), program, state, device);                                   \
    }
#if LUMINARY099
#include "conics_keys.inc"
#else
#include "cm_conics_keys.inc"
#endif
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#if LUMINARY099
#include "conics_keys.inc"
#else
#include "cm_conics_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
#include "conics_keys.inc"
#else
#include "cm_conics_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#if LUMINARY099
const agc_routine_table lm_conic_compiled_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_conic_compiled_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_conics_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_conics_basic = {basic_keys, COUNT(basic_keys)};
#endif
