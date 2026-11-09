/* SPDX-License-Identifier: MIT */
/* The readable INTEGRATION_INITIALIZATION (src/integration_init/) as
 * routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_integration_init.c and
 * cm_integration_init.c): <m>_integration_init_basic and
 * <m>_integration_init_interpretive. The keys come from the faithful
 * tables, in their order (integration_init_keys.inc and
 * cm_integration_init_keys.inc, which tools/p20_p25_keys.py writes from
 * them). The keys no part has written yet run the faithful port's
 * routines (renamed faithful_) until their parts are written.
 * Partial file: Luminary099/INTEGRATION_INITIALIZATION.agc and
 * Comanche055/INTEGRATION_INITIALIZATION.agc.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (PHASCHNG, FINDVAC, LONGCALL, DANZIG) or reaches NEWOPS through
 * a TC INTPRET. An order pair is one pass of the readable interpreter
 * over the listing's words (interpret_listing()), in the part of the code
 * LOC is in. The LM's pair runs an RTB's routine and the rest of the
 * pair, as the faithful one does: LOADTIME, SGNAGREE or SIGNMPAC
 * (src/rtb_op_codes/), or a move (moves.c). The CM's pass ends at the
 * RTB; the routine ends at DANZIG,
 * and the key there goes on with the pair's right-hand order, which waits
 * in EDOP (see pending_orders). At a move's own key the LM's routine goes
 * on from its DANZIG too, when no order waits there.
 *
 * The states the faithful routines take: FB the key's bank, ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range,
 * every erasable word an ordinary one, NEWJOB +0 or a core set and FIXLOC
 * one of the VAC areas or a NOVAC job's (but for the CM's basic code,
 * which takes any NEWJOB and FIXLOC); at an order pair also Z at NEWOPS,
 * BANKSET the BBANK and INTBIT15 the bank's bit 15 (or, at a pending key,
 * Z at DANZIG). A move takes, as the faithful one does, the latch, ZERO
 * and EBANK in range and Q an ordinary word too. A run the readable code
 * refuses part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../integration_init/integration_init.h"
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

/* The bank of STATEINT and the moves (INTINIT), and LOADTIME's (FB),
 * from their FCADRs. */
#define INTINIT_BANK  CODE_BANK(I_STATEINT)
#define MOVES_BANK    ((word)(I_MOVEACSM & 076000u))
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))

/* The basic code after A-PCHK's EXIT starts 3 words on from the LM's
 * A-PCHK, from the CM's ANDOUT. */
#if LUMINARY099
#define A_PCHK_EXIT A_PCHK
#else
#define A_PCHK_EXIT ANDOUT
#endif

/* The faithful tables, renamed. */
#if LUMINARY099
extern const agc_routine_table faithful_lm_integration_init_basic,
    faithful_lm_integration_init_interpretive;
#define faithful_basic        faithful_lm_integration_init_basic
#define faithful_interpretive faithful_lm_integration_init_interpretive
#else
extern const agc_routine_table faithful_cm_integration_init_basic,
    faithful_cm_integration_init_interpretive;
#define faithful_basic        faithful_cm_integration_init_basic
#define faithful_interpretive faithful_cm_integration_init_interpretive
#endif

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
 * basic_sane(), which leaves out NEWJOB and the work area). */
static bool sane(bool work_area)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (work_area &&
        (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
         (FIXLOC != 0110u && (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* The key's bank as the faithful routines take it: banks 30-37 (the
 * CM's AVETOMID is in bank 35) with the superbank bit off. */
static bool superbank_off(unsigned bank)
{
    return bank < 030u || !(*agc.superbank & 0100u);
}

#if LUMINARY099
/* The states the faithful moves take: the interrupt latch, ZERO and EBANK
 * in range, and every word from Q on an ordinary one. */
static bool move_sane(void)
{
    unsigned i;

    if (*agc.interrupts > 1u || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u))
        return false;
    for (i = Q_ADDRESS; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}
#endif

/* ---- The basic code ---- */

/* A move at its own key. In the LM the routine goes on from its DANZIG
 * when no right-hand order waits in EDOP, as the faithful one does. */
static void after_move(void)
{
#if LUMINARY099
    if (EDOP == 0u) danzig();
#endif
}

static void moveacsm_key(void)
{
    moveacsm();
    after_move();
}

static void movepcsm_key(void)
{
    movepcsm();
    after_move();
}

static void movealem_key(void)
{
    movealem();
    after_move();
}

static void moveplem_key(void)
{
    moveplem();
    after_move();
}

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
    bool move;
} basic_entries[] = {
    /* Part 1 */
    {KEY(INTINIT_BANK, STATEINT), stateint, false},
    {KEY(INTINIT_BANK, STATEINT + 2u), stateint_after_phaschng, false},
    {KEY(INTINIT_BANK, STATEINT + 6u), stateint_after_findvac, false},
    {KEY(INTINIT_BANK, STATINT1), statint1, false},
    {KEY(INTINIT_BANK, ENDINT + 2u), endint_after_exit, false},
    {KEY(INTINIT_BANK, ENDINT + 4u), endint_after_phaschng, false},
    {KEY(INTINIT_BANK, ENDINT + 011u), endint_after_longcall, false},
    {KEY(INTINIT_BANK, NOINT + 1u), noint_after_exit, false},
    {KEY(INTINIT_BANK, NOINT + 3u), noint_after_phaschng, false},
    {KEY(INTINIT_BANK, NOINT + 5u), noint_after_downflag, false},
    {KEY(INTINIT_BANK, MOVEACSM), moveacsm_key, true},
    {KEY(INTINIT_BANK, MOVEPCSM), movepcsm_key, true},
    {KEY(INTINIT_BANK, MOVEALEM), movealem_key, true},
    {KEY(INTINIT_BANK, MOVEPLEM), moveplem_key, true},
    /* Part 2 */
    {KEY(INTINIT_BANK, A_PCHK_EXIT + 3u), a_pchk_after_exit, false},
    {KEY(INTINIT_BANK, A_PCHK_EXIT + 5u), a_pchk_after_phaschng, false},
    {KEY(INTINIT_BANK, A_PCHK_EXIT + 7u), a_pchk_after_upflag, false},
    /* Part 3 */
    {KEY(INTINIT_BANK, INTEXIT + 012u), intexit_after_exit, false},
    {KEY(INTINIT_BANK, INTSTALL + 1u), intstall_after_exit, false},
    {KEY(INTINIT_BANK, INTWAKE0 + 1u), intwake1, false},
    {KEY(INTINIT_BANK, INTWAKE), intwake, false},
    {KEY(INTINIT_BANK, INTWAKE + 011u), intwake_after_phaschng, false},
    {KEY(INTINIT_BANK, INTWAKE1), intwake1, false},
#if LUMINARY099
    {KEY(INTINIT_BANK, INTWAKE + 4u), intwake_after_intwake1, false},
    {KEY(INTINIT_BANK, INTWAKE1 + 3u), intwake1_after_jobwake, false},
#else
    {KEY(INTINIT_BANK, WAKE1 + 4u), intwake1_after_jobwake, false},
    {KEY(INTINIT_BANK, ERASTAL1 + 1u), erastal1_after_exit, false},
    {KEY(INTINIT_BANK, ERASTAL2 + 1u), erastal2_after_exit, false},
    {KEY(INTINIT_BANK, ERASWAK1), eraswak1, false},
    {KEY(INTINIT_BANK, ERASWAK2), eraswak2, false},
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
    &STATINT1_CODE, &SETIFLGS_CODE, &ATOPCSM_CODE,  &PTOACSM_CODE,  &ATOPLEM_CODE,
    &PTOALEM_CODE,  &USEPIOS_CODE,  &INTEGRV_CODE,  &ALOADED_CODE,  &A_PCHK_CODE,
    &PHEXIT_CODE,   &TESTLOOP_CODE, &INTSTALL_CODE, &INTWAKE0_CODE, &GOBAC_RVQ_CODE,
#if COMANCHE055
    &ERASTAL1_CODE, &ERASTAL2_CODE,
#endif
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
 * left, an RTB, runs a routine: PTOACSM's RTB BON, and DT/2COMP's RTB SL
 * (SGNAGREE). The routine ends at
 * DANZIG, which finds the order in EDOP and resumes the pass at LOC: EDOP
 * holds the order's code plus one. */
static const struct pending_order {
    uint16_t key;
    word edop;
} pending_orders[] = {
    {KEY(INTINIT_BANK, CODE_Z(I_PTOACSM) + 1u), ORDER_BON + 1u},
    {KEY(INTINIT_BANK, CODE_Z(I_DT_2COMP) + 4u), ORDER_SL + 1u},
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

#if LUMINARY099
/* The routines of RTB_OP_CODES the LM's pairs run, at their addresses
 * under their banks (FB, from their FCADRs). */
static const struct {
    word at, bank;
    void (*routine)(void);
} rtb_routines[] = {
    {LOADTIME, LOADTIME_BANK, loadtime},                /* STATINT1 */
    {SGNAGREE, (word)(I_SGNAGREE & 076000u), sgnagree}, /* DT/2COMP */
    {SIGNMPAC, (word)(I_SIGNMPAC & 076000u), signmpac}, /* GETMAXDT */
};

/* The RTB_OP_CODES routine an RTB has gone to, or none. */
static void (*rtb_routine_at(void))(void)
{
    unsigned i;

    for (i = 0; i < COUNT(rtb_routines); ++i)
        if (Z == rtb_routines[i].at && FBANK == rtb_routines[i].bank)
            return rtb_routines[i].routine;
    return NULL;
}

/* The move an RTB has gone to, or none. */
static void (*move_at(void))(void)
{
    if (FBANK != MOVES_BANK) return NULL;
    if (Z == MOVEACSM) return moveacsm;
    if (Z == MOVEPCSM) return movepcsm;
    if (Z == MOVEALEM) return movealem;
    if (Z == MOVEPLEM) return moveplem;
    return NULL;
}
#endif

/* One pass; in the LM, after an RTB of LOADTIME, SGNAGREE, SIGNMPAC or a
 * move, the routine and its DANZIG, which goes on with the pair's other
 * order. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
#if LUMINARY099
    void (*routine)(void);

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    routine = rtb_routine_at();
    if (routine) {
        if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    } else {
        routine = move_at();
        if (!routine) return EXEC_OK;
        if (!move_sane()) return EXEC_OUT_OF_SCOPE;
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

/* Each basic key's routine: the readable one, or the faithful port's until
 * its part is written. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    mission_io *io = device;

    if (!entry) return faithful_basic.entries[index].call(program, state, device);
    if (!program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (FBANK != (word)((key / 1024u) << 10) || !superbank_off(key / 1024u) ||
        !sane(BASIC_WORK_AREA))
        return EXEC_OUT_OF_SCOPE;
#if LUMINARY099
    if (entry->move && !move_sane()) return EXEC_OUT_OF_SCOPE;
#endif
    return run_basic(entry->routine, state, io);
}

/* Each order pair's routine: one pass of the readable interpreter, or the
 * faithful port's until its part is written. */
static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    word at = (word)(02000u + key % 1024u);
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, at);
    enum exec_status status;
    bool started;

    if (!code) return faithful_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
#if COMANCHE055
    if (pending_order(key))
        started = LOC == at && at_danzig(bank, pending_order(key));
    else
#endif
        started = Z == NEWOPS && FBANK == (word)(bank << 10) && LOC == at && BANKSET == BBANK &&
                  INTBIT15 == (FBANK & 040000u) && sane(true);
    if (!started || !superbank_off(bank)) return EXEC_OUT_OF_SCOPE;

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
bool integration_init_readable(uint16_t key, bool order_pair)
{
    if (order_pair) return code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL;
    return basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(i, KEY(bank, address), program, state, device);                               \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(i, KEY(bank, address), program, state, device);                                \
    }
#if LUMINARY099
#include "integration_init_keys.inc"
#else
#include "cm_integration_init_keys.inc"
#endif
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#if LUMINARY099
#include "integration_init_keys.inc"
#else
#include "cm_integration_init_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
#include "integration_init_keys.inc"
#else
#include "cm_integration_init_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#if LUMINARY099
const agc_routine_table lm_integration_init_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_integration_init_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_integration_init_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_integration_init_basic = {basic_keys, COUNT(basic_keys)};
#endif
