/* SPDX-License-Identifier: MIT */
/* The readable P51-P53 of the CM (src/p51_p53_cm/) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_p51_p53.c): cm_p51_p53_basic (the basic code)
 * and cm_p51_p53_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_p51_keys.inc, which tools/p20_p25_keys.py writes from them). The
 * file is written part by part: a key a part has written runs the
 * readable routine; any other runs the faithful port's, built into the
 * program as faithful_cm_p51_p53_basic and faithful_cm_p51_p53_interpretive
 * (tests/x86_readable.py, the Makefile's tests).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, PHASCHNG, DOWNFLAG, ENDOFJOB, GOTOPOOH, ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()).
 *
 * The device is the CM's io record (cm_native_io); the file reads and
 * writes no channel. The states the faithful routines take, at every key:
 * FB the key's bank, ZERO +0, NEWJOB +0 or a core set, the bank registers
 * agreeing, the interrupt and superbank latches in range, FIXLOC one of
 * the VAC areas, and every erasable word an ordinary one; at an order pair
 * also Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. A
 * run the readable code refuses part way leaves the state and io record
 * as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p51_p53.h"
#include "../p51_p53_cm/p51_p53_cm.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

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

/* The interpreter's fixed operands, and the file's constants, are the
 * rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(1u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, cm_native_io *io)
{
    bind_machine(program, state, io);
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its sane()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
        EBANK > 03400u || (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P52 */
    {KEY(015u, PROG52), prog52},
    {KEY(015u, PROG52 + 2u), prog52_after_phaschng},
    {KEY(015u, PROG52 + 4u), prog52_after_updatflg},
    {KEY(015u, PROG52 + 6u), prog52_after_trackflg},
    {KEY(015u, PROG52 + 010u), prog52_after_r02both},
    {KEY(015u, PROG52 + 014u), prog52_no_preferred},
    {KEY(015u, P52A), p52a},
    {KEY(015u, P52B + 3u), p52b_terminate},
    {KEY(015u, P52B + 4u), p52b_proceed},
    {KEY(015u, P52B + 5u), p52b_enter},
    {KEY(015u, P52B + 6u), p52b_immediate_return},
    {KEY(015u, P52B + 010u), p52b_after_phaschng},
    {KEY(015u, P52B + 015u), p52_landing_site},
    {KEY(015u, P52B + 016u), p52_preferred},
    {KEY(015u, P52B + 017u), p52_nominal},
    {KEY(015u, P52B + 020u), p52_reference},
    {KEY(015u, P52T + 6u), p52t_terminate},
    {KEY(015u, P52T + 7u), p52t_proceed},
    {KEY(015u, P52T + 010u), p52t_enter},
    {KEY(015u, P52T + 011u), p52t_time},
    {KEY(015u, P52D + 3u), p52d_after_exit},
    {KEY(015u, P52D + 6u), p52d_terminate},
    {KEY(015u, P52D + 7u), p52d_proceed},
    {KEY(015u, P52J), p52j},
    {KEY(015u, P52J + 3u), p52j_coarse},
    {KEY(015u, P52C), p52c},
    {KEY(015u, P52C + 2u), p52c_after_phaschng},
    {KEY(015u, P52C + 5u), p52c_terminate},
    {KEY(015u, P52C + 6u), p52c_proceed},
    {KEY(015u, P52C + 7u), p52c_enter},
    {KEY(015u, P52C + 010u), p52c_stars},
    {KEY(015u, P52E), p52e},
    {KEY(015u, P52E + 2u), p52e_no_stars},
    {KEY(015u, P52F), p52f},
    {KEY(015u, ENDP50S + 1u), p52f_after_exit},
    {KEY(015u, P52I + 2u), p52i_after_alarm},
    {KEY(015u, P52I + 5u), p52i_terminate},
    {KEY(015u, P52I + 6u), p52i_proceed},
    {KEY(015u, P52I + 7u), p52i_recycle},
    {KEY(015u, COARSTYP + 3u), coarstyp_terminate},
    {KEY(015u, COARSTYP + 4u), coarstyp_normal},
    {KEY(015u, COARSTYP + 5u), coarstyp_gyro},
    {KEY(015u, GYCRS + 022u), gycrs_after_exit},
    {KEY(015u, GYCRS + 025u), gycrs_after_godspr},
    {KEY(015u, GYCRS + 030u), gycrs_after_imupulse},
    {KEY(015u, GYCRS + 032u), gycrs_torque_failed},
    {KEY(015u, GYCRS + 033u), gycrs_torqued},
    {KEY(015u, GYCRS + 035u), gycrs_after_phaschng},
    /* Part 2: P52LS and R52 */
    {KEY(016u, LSDISP), lsdisp},
    {KEY(016u, LSDISP + 3u), lsdisp_terminate},
    {KEY(016u, LSDISP + 4u), lsdisp_proceed},
    {KEY(016u, LSDISP + 5u), lsdisp_enter},
    {KEY(016u, LSDISP + 6u), lsdisp_accept},
    {KEY(014u, R52VRB + 1u), r52vrb_after_exit},
    {KEY(014u, R52A), r52a},
    {KEY(014u, R52C), r52c},
    {KEY(014u, R52D + 2u), r52d_over_90},
    {KEY(014u, R52D + 3u), r52d_over_50},
    {KEY(014u, R52D + 4u), r52d_under_50},
    {KEY(014u, R52JA), r52ja},
    {KEY(014u, R52JA + 4u), r52ja_not_lem},
    {KEY(014u, R52E), r52e},
    {KEY(014u, R52E + 7u), r52e_trunnion},
    {KEY(014u, R52F + 3u), r52f_after_delayjob},
    {KEY(014u, R52HA), r52ha},
    {KEY(014u, R52HA + 2u), r52ha_after_r61csm},
    {KEY(014u, R52SYNC), r52sync},
    {KEY(014u, R52J + 2u), r52j_after_downflag},
    {KEY(014u, R52J + 4u), r52j_after_alarm},
    {KEY(014u, R52M), r52m},
    {KEY(014u, R52M + 4u), r52m_no_r53},
    {KEY(014u, R52M + 011u), r52m_after_findvac},
    {KEY(014u, R53JOB), r53job},
    {KEY(014u, ENDPLAC + 1u), endplac_after_exit},
    {KEY(014u, R52L + 4u), r52l_not_lem},
    {KEY(014u, R52L + 7u), r52l_terminate},
    {KEY(014u, R52L + 010u), r52l_proceed},
    {KEY(014u, R52L + 011u), r52l_enter},
    {KEY(014u, R52L + 012u), r52l_immediate_return},
    {KEY(014u, TERM52), term52},
    {KEY(014u, TERM52 + 1u), term52_after_clearmrk},
    {KEY(014u, TERM52 + 3u), term52_after_mkreleas},
    {KEY(014u, TERM52 + 7u), term52_after_kleenex},
    /* Part 3: S50 and R56 */
    {KEY(014u, PICAPAR), picapar},
    {KEY(014u, PICAPAR + 1u), picapar_after_makecadr},
    {KEY(014u, PICEND + 3u), picend_after_exit},
    {KEY(014u, PICGXT + 1u), picgxt_after_exit},
    /* Part 4: R51 and R55 */
    {KEY(014u, R51 + 1u), r51_after_exit},
    {KEY(014u, R51_3 + 4u), r51_3_after_exit},
    {KEY(014u, R51_3 + 7u), r51_3_restart},
    {KEY(014u, R51DSP + 3u), r51dsp_terminate},
    {KEY(014u, R51DSP + 4u), r51dsp_proceed},
    {KEY(014u, R51DSP + 5u), r51dsp_enter},
    {KEY(014u, R51DSP + 6u), r51dsp_displayed},
    {KEY(014u, R51DSP + 010u), r51dsp_after_blanket},
    {KEY(014u, R51DSP + 012u), r51dsp_bad_code},
    {KEY(014u, R51DSP + 013u), r51dsp_after_falton},
    {KEY(014u, R51DSP + 014u), r51dsp_code_good},
    {KEY(014u, R51ST + 2u), r51st_after_exit},
    {KEY(014u, R51A + 4u), r51a_after_exit},
    {KEY(014u, R51A + 6u), r51a_after_mkreleas},
    {KEY(014u, R51A + 013u), r51a_sighted},
    {KEY(014u, R51A + 015u), r51a_both_sighted},
    {KEY(014u, R51K + 1u), r51k_after_exit},
    {KEY(014u, R51K + 4u), r51k_terminate},
    {KEY(014u, R51K + 5u), r51k_proceed},
    {KEY(014u, R51K + 6u), r51k_enter},
    {KEY(014u, R51K + 7u), r51k_again},
    {KEY(014u, R51K + 011u), r51k_done},
    {KEY(015u, R55_1), r55_1},
    {KEY(015u, R55_1 + 3u), r55_terminate},
    {KEY(015u, R55_1 + 4u), r55_proceed},
    {KEY(015u, R55_1 + 5u), r55_enter},
    {KEY(015u, R55_2), r55_2},
    {KEY(015u, R55_2 + 2u), r55_torque},
    {KEY(015u, R55_2 + 5u), r55_after_imupulse},
    {KEY(015u, R55_2 + 7u), r55_failed},
    {KEY(015u, R55_2 + 010u), r55_torqued},
    {KEY(015u, R55RET), r55ret},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    /* Part 1: P52 */
    &P52T_LANDING_SITE_CODE,
    &P52T_NOMINAL_CODE,
    &P52J_CODE,
    &P52J_COARSE_CODE,
    &P52C_STARS_CODE,
    &P52F_CODE,
    &GYCRS_CODE,
    &GYCRS_DONE_CODE,
    /* Part 2: P52LS and R52 */
    &P52LS_CODE,
    &LSDISP_CODE,
    &R52_CODE,
    &R52VRB_CODE,
    &R52A_CODE,
    &R52Q_CODE,
    &R53JOB_CODE,
    &ADVORB_CODE,
    /* Part 3: S50 and R56 */
    &S50_CODE,
    &PICAPAR_CODE,
    &PICEND_CODE,
    &PICGXT_CODE,
    /* Part 4: R51 and R55 */
    &R51_CODE,
    &R51_3_CODE,
    &R51_STAR_CODE,
    &R51_MARK_CODE,
    &R51A_PLANET_CODE,
    &R51_TORQUE_CODE,
    &R51K_DONE_CODE,
    &R51_4_CODE,
    &R51B_CODE,
    &R55_CODE,
    &R55RET_CODE,
};

/* The table an address of a bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* PICAPAR loads stars from the catalog in fixed memory, bank 14, by X1
 * and X2 (VLOAD* CATLOG,1, DOT* CATLOG,2). The faithful port carries only
 * the words a load of CATLOG less 0 to 228 reads (14,3414 to 14,3765)
 * and refuses a load from fixed memory outside them. (At PIC4 +3, BON
 * VLOAD*, the load comes only when CULTFLAG is clear.) */
static const struct catalog_load {
    uint16_t key;
    bool by_x1, by_x2, after_bon;
} catalog_loads[] = {
    {KEY(014u, PIC2), true, false, false},        {KEY(014u, PIC4), false, true, false},
    {KEY(014u, PIC4 + 3u), true, false, true},    {KEY(014u, PIC4 + 7u), false, true, false},
    {KEY(014u, PIC4 + 017u), true, false, false}, {KEY(014u, PIC4 + 025u), false, true, false},
    {KEY(014u, STRAT), true, true, false},
};

/* CULTFLAG: flag word 065 / 15, bit 065 % 15 counted from bit 15 down. */
static bool cultflag(void)
{
    return (agc.memory[ADDRESS_OF(STATE) + I_CULTFLAG / 15u] & (040000u >> (I_CULTFLAG % 15u))) !=
           0u;
}

/* Whether CATLOG less x is a fixed word outside the faithful port's copy:
 * the address word, less one, less x (ones' complement: end around, or
 * where x is negative, overflow corrected as ADS leaves ADDRWD), in fixed
 * memory where any of its top four bits is set (INDEX2's MASK HIGH4); its
 * top five bits are the bank (TS FBANK), so a load past bank 14's words
 * is outside too. */
static bool outside_star_catalog(word x)
{
    unsigned first = (I_CATLOG & 037777u) - 38u * 6u, sum;

    sum = (I_CATLOG & 037777u) + (~x & 077777u);
    if (sum > 077777u)
        sum = (sum + 1u) & 077777u; /* end around */
    else if ((x & 040000u) && (sum & 040000u))
        sum &= 037777u; /* overflow corrected */
    if (!(sum & 074000u)) return false;
    return sum < first || sum + 6u > first + 234u;
}

static bool loads_outside_star_catalog(uint16_t key)
{
    word x1 = agc.memory[(FIXLOC + ADDRESS_OF(X1)) & 03777u];
    word x2 = agc.memory[(FIXLOC + ADDRESS_OF(X2)) & 03777u];
    unsigned i;

    for (i = 0; i < COUNT(catalog_loads); ++i)
        if (catalog_loads[i].key == key && !(catalog_loads[i].after_bon && cultflag()))
            return (catalog_loads[i].by_x1 && outside_star_catalog(x1)) ||
                   (catalog_loads[i].by_x2 && outside_star_catalog(x2));
    return false;
}

/* The RTB routines the pairs run, at their addresses under their banks
 * (FB, from their FCADRs). */
static const struct {
    word at, bank;
    void (*routine)(void);
} rtb_routines[] = {
    {LOADTIME, I_LOADTIME & 076000u, loadtime}, /* P52C, R51DSP */
    {SET1_PDT, I_SET1_PDT & 076000u, set1_pdt}, /* GYCRS (p51.c) */
};

/* One pass; after an RTB, the routine (src/rtb_op_codes/, or SET1/PDT in
 * p51.c) and the pass on from its DANZIG, as the faithful pair runs them. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 cm_native_io *io, const struct interpretive_code *code)
{
    unsigned i;

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    for (i = 0; i < COUNT(rtb_routines); ++i)
        if (Z == rtb_routines[i].at && FBANK == rtb_routines[i].bank) break;
    if (i == COUNT(rtb_routines)) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    rtb_routines[i].routine();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p51_p53_basic, faithful_cm_p51_p53_interpretive;

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_cm_p51_p53_basic.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)((key / 1024u) << 10) || !sane()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return EXEC_OK;
}

static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)))
        return faithful_cm_p51_p53_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || !code ||
        LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) ||
        !sane())
        return EXEC_OUT_OF_SCOPE;
    if (loads_outside_star_catalog(key)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = one_pass(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: the tables of readable words, by index (none
 * past the last), which it checks against the rope. */
const struct interpretive_code *cm_p51_code(unsigned i)
{
    return i < COUNT(codes) ? codes[i] : NULL;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p51_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
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
#include "cm_p51_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p51_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p51_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p51_p53_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p51_p53_basic = {basic_keys, COUNT(basic_keys)};

#endif
