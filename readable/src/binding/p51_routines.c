/* SPDX-License-Identifier: MIT */
/* The readable P51-P53 (src/p51_p53/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_p51_p53.c): lm_p51_p53_basic (the basic code) and
 * lm_p51_p53_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (p51_keys.inc, which tools/p20_p25_keys.py writes from them). The file
 * is written part by part: a key a part has written runs the readable
 * routine; any other runs the faithful port's, built into the program as
 * faithful_lm_p51_p53_basic and faithful_lm_p51_p53_interpretive
 * (tests/x86_readable.py, the Makefile's tests).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, PHASCHNG, ENDOFJOB, GOTOPOOH, ...) or reaches NEWOPS
 * through a TC INTPRET. An order pair is one pass of the readable
 * interpreter over the listing's words (interpret_listing()).
 *
 * The states the faithful routines take, at every key: FB the key's bank;
 * at an order pair Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's
 * bit 15. Always ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range, and every erasable word an ordinary one. At
 * an order pair and at the basic keys the faithful port marks so (its
 * at_basic), NEWJOB +0 or a core set and FIXLOC one of the VAC areas; at
 * the other basic keys (its at_basic_nv) NEWJOB may be -0 too, and FIXLOC
 * anything. A run the readable code refuses part way leaves the state and
 * io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "lm_p51_p53.h"
#include "../p51_p53/p51_p53.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The banks (FB) of the RTB routines the pairs run, from their FCADRs. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))
#define SGNAGREE_BANK ((word)(I_SGNAGREE & 076000u))
#define RDCDUS_BANK   ((word)(I_RDCDUS & 076000u))

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

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, p66_bias_io *io)
{
    bind_machine(program, state, io);
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its sane(); without a work area,
 * its sane_nv()). */
static bool sane(bool with_work_area)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (newjob > 037777u && (with_work_area || newjob != MINUS_ZERO)) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (with_work_area && (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
    bool with_work_area; /* the faithful routine's at_basic (else at_basic_nv) */
} basic_entries[] = {
    /* Part 1: P52 */
    {KEY(015u, PROG52), prog52, false},
    {KEY(015u, PROG52 + 2u), prog52_after_r02both, false},
    {KEY(015u, PROG52 + 6u), prog52_no_preferred, false},
    {KEY(015u, P52A), p52a, false},
    {KEY(015u, P52B + 3u), p52b_terminate, false},
    {KEY(015u, P52B + 4u), p52b_proceed, true},
    {KEY(015u, P52B + 5u), p52b_enter, false},
    {KEY(015u, P52B + 6u), p52b_immediate_return, false},
    {KEY(015u, P52B + 010u), p52b_after_phaschng, false},
    {KEY(015u, P52B + 015u), p52_option4, false},
    {KEY(015u, P52T + 6u), p52t_terminate, false},
    {KEY(015u, P52T + 7u), p52t_proceed, true},
    {KEY(015u, P52T + 010u), p52t_enter, true},
    {KEY(015u, P52T + 011u), p52t_time, true},
    {KEY(015u, P52V + 4u), p52v_option4, true},
    {KEY(015u, P52D + 3u), p52d_after_exit, false},
    {KEY(015u, P52D + 6u), p52d_terminate, false},
    {KEY(015u, P52D + 7u), p52d_proceed, false},
    {KEY(015u, REGCOARS), regcoars, true},
    {KEY(015u, P52OUT + 1u), p52out_after_exit, false},
    {KEY(015u, COARSTYP + 3u), coarstyp_terminate, false},
    {KEY(015u, COARSTYP + 4u), coarstyp_normal, true},
    {KEY(015u, COARSTYP + 5u), coarstyp_gyro_torque, true},
    {KEY(014u, LSDISP), lsdisp, false},
    {KEY(014u, LSDISP + 3u), lsdisp_terminate, false},
    {KEY(014u, LSDISP + 4u), lsdisp_proceed, true},
    {KEY(014u, LSDISP + 5u), lsdisp_enter, false},
    /* Part 2: S50 and R56 */
    {KEY(014u, PICAPAR), picapar, false},
    {KEY(014u, PICAPAR + 1u), picapar_after_makecadr, true},
    {KEY(014u, PICEND + 3u), picend_after_exit, false},
    {KEY(014u, PICNSWP + 1u), picnswp_after_exit, false},
    /* Part 3: R51 and GYCOARS */
    {KEY(014u, R51_1 + 1u), r51_after_exit, false},
    {KEY(014u, R51C), r51c, false},
    {KEY(014u, R51C + 3u), r51c_terminate, false},
    {KEY(014u, R51C + 4u), r51c_proceed, true},
    {KEY(014u, R51C + 5u), r51c_enter, true},
    {KEY(014u, R51C + 6u), r51c_pick_stars, true},
    {KEY(014u, R51C + 015u), r51c_after_exit, false},
    {KEY(014u, R51F - 1u), r51c_no_stars, true},
    {KEY(014u, R51F), r51f, true},
    {KEY(014u, R51I), r51i, false},
    {KEY(014u, R51I + 2u), r51i_after_alarm, false},
    {KEY(014u, R51I + 5u), r51i_terminate, false},
    {KEY(014u, R51I + 6u), r51i_proceed, true},
    {KEY(014u, R51I + 7u), r51i_enter, true},
    {KEY(014u, R51E), r51e, true},
    {KEY(014u, R51_3 + 1u), r51_3_after_exit, false},
    {KEY(014u, R51_3 + 3u), r51_3_restart, true},
    {KEY(014u, R51_3 + 7u), r51_mark_after_exit, false},
    {KEY(014u, R51_3 + 011u), r51_mark_after_aotmark, false},
    {KEY(014u, R51_3 + 013u), r51_mark_failed, false},
    {KEY(014u, R51_3 + 014u), r51_marked, true},
    {KEY(014u, R51_3 + 017u), r51_both_marked, true},
    {KEY(014u, R51_3 + 024u), r51_second_after_exit, false},
    {KEY(014u, R51_3 + 026u), r51_second_restart, true},
    {KEY(014u, R51P63), r51p63, false},
    {KEY(014u, R51P63 + 3u), r51p63_terminate, false},
    {KEY(014u, R51P63 + 4u), r51p63_enter, true},
    {KEY(014u, R51P63 + 5u), r51p63_proceed, true},
    {KEY(014u, GYCOARS + 7u), gycoars_after_exit, false},
    {KEY(014u, GYCOARS + 012u), gycoars_torque, false},
    {KEY(014u, GYCOARS + 015u), gycoars_after_imupulse, false},
    {KEY(014u, GYCOARS + 017u), gycoars_failed, false},
    {KEY(014u, GYCOARS + 020u), gycoars_torqued, false},
    {KEY(014u, GYCOARS + 022u), gycoars_restart, true},
    /* Part 4: R55, R54 (CHKSDATA) and CAL53A */
    {KEY(014u, R55_1), r55_1, false},
    {KEY(014u, R55_1 + 3u), r55_terminate, false},
    {KEY(014u, R55_1 + 4u), r55_proceed, true},
    {KEY(014u, R55_1 + 5u), r55_enter, true},
    {KEY(014u, R55_2), r55_2, false},
    {KEY(014u, R55_2 + 2u), r55_torque, false},
    {KEY(014u, R55_2 + 5u), r55_after_imupulse, false},
    {KEY(014u, R55_2 + 7u), r55_failed, false},
    {KEY(014u, R55_2 + 010u), r55_torqued, false},
    {KEY(014u, R55RET), r55ret, true},
    {KEY(014u, CHKSDATA + 034u), chksdata_after_exit, false},
    {KEY(014u, CHKSDATA + 037u), chksdata_terminate, false},
    {KEY(014u, CHKSDATA + 040u), chksdata_proceed, true},
    {KEY(014u, CHKSDATA + 041u), chksdata_enter, true},
    {KEY(014u, CAL53A + 026u), cal53a_after_exit, false},
    {KEY(014u, CAL53A + 030u), coarfine, true},
    /* Part 5: P51, COARSE and NCOARSE */
    {KEY(014u, P51), p51, false},
    {KEY(014u, P51 + 2u), p51_plus2, false},
    {KEY(014u, P51 + 5u), p51_terminate, false},
    {KEY(014u, P51 + 6u), p51_proceed, false},
    {KEY(014u, P51 + 7u), p51_enter, false},
    {KEY(014u, P51 + 011u), p51_zero_gimbals, false},
    {KEY(014u, P51 + 020u), p51_display_v41, false},
    {KEY(014u, P51 + 023u), p51_coarse, true},
    {KEY(014u, P51 + 027u), p51_after_coarse, false},
    {KEY(014u, P51 + 031u), p51_again, false},
    {KEY(014u, P51B + 2u), p51b_ncoarse, true},
    {KEY(014u, P51C + 1u), p51c_plus1, false},
    {KEY(014u, P51C + 3u), p51c_aotmark, false},
    {KEY(014u, P51C + 5u), p51c_after_aotmark, false},
    {KEY(014u, P51C + 7u), p51c_failed, false},
    {KEY(014u, P51C + 010u), p51c_star, true},
    {KEY(014u, P51D + 1u), p51d_plus1, false},
    {KEY(014u, P51D + 3u), p51d_star, true},
    {KEY(014u, P51D + 7u), p51d_planet, true},
    {KEY(014u, P51D + 015u), p51d_second_star, false},
    {KEY(014u, P51E + 2u), p51e_planet, true},
    {KEY(014u, P51E + 021u), p51e_again, false},
    {KEY(014u, P51G + 011u), p51g_finis, false},
    {KEY(014u, COARSE + 1u), coarse_imucoars, false},
    {KEY(014u, COARSE + 3u), coarse_after_imucoars, false},
    {KEY(014u, COARSE + 5u), coarse_failed, false},
    {KEY(014u, COARSE + 6u), coarse_imufine, false},
    {KEY(014u, COARSE + 010u), coarse_after_imufine, false},
    {KEY(014u, COARSE + 012u), coarse_fine_failed, false},
    {KEY(014u, COARSE + 013u), coarse_return, true},
    {KEY(014u, NCOARSE + 1u), ncoarse_plus1, true},
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
    &P52E_CODE,
    &P52V_CODE,
    &P52W_CODE,
    &P52H_CODE,
    &REGCOARS_CODE,
    &COARSTYP_CODE,
    &P52LS_CODE,
    &N89DISP_CODE,
    &LSDISP_CODE,
    /* Part 2: S50 and R56 */
    &LOCSAM_CODE,
    &PICAPAR_CODE,
    &PICEND_CODE,
    &PICGXT_CODE,
    /* Part 3: R51 and GYCOARS */
    &R51_CODE,
    &R51C_CODE,
    &R51_3_CODE,
    &R51_MARK_CODE,
    &R51_SECOND_CODE,
    &R51_TORQUE_CODE,
    &R51P63_CODE,
    &R51_4_CODE,
    &GYCOARS_CODE,
    &GYCOARS_DONE_CODE,
    /* Part 4: R55, R54 (CHKSDATA), CAL53A and MATMOVE */
    &R55_CODE,
    &R55RET_CODE,
    &CHKSDATA_CODE,
    &CHKSDATA_ENTER_CODE,
    &CHKSDA_CODE,
    &CAL53A_CODE,
    &COARFINE_CODE,
    &MATMOVE_CODE,
    /* Part 5: P51, COARSE, NCOARSE, S52.2 and S52.3 */
    &P51_CODE,
    &P51B_CODE,
    &P51C_CODE,
    &P51D_CODE,
    &P51E_CODE,
    &P51G_CODE,
    &COARSE_CODE,
    &COARSE_RETURN_CODE,
    &NCOARSE_CODE,
    &NCOARSE_GCOMP_CODE,
    &S52_2_CODE,
    &S52_3_CODE,
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

/* R56 loads stars from the catalog in fixed memory, bank 14, by X1 and
 * X2 (VLOAD* CATLOG,1, DOT* CATLOG,2). The faithful port carries only the
 * words a load of CATLOG less 0 to 228 reads (14,2002 to 14,2353) and
 * refuses a load from fixed memory outside them. */
static const struct catalog_load {
    uint16_t key;
    bool by_x1, by_x2;
} catalog_loads[] = {
    {KEY(014u, PIC2), true, false},        {KEY(014u, PIC4), false, true},
    {KEY(014u, PIC4 + 6u), true, true},    {KEY(014u, PIC4 + 014u), true, false},
    {KEY(014u, PIC4 + 022u), false, true}, {KEY(014u, STRAT), true, true},
    {KEY(014u, PICGXT + 3u), true, false}, {KEY(014u, PICGXT + 6u), false, true},
};

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
        if (catalog_loads[i].key == key)
            return (catalog_loads[i].by_x1 && outside_star_catalog(x1)) ||
                   (catalog_loads[i].by_x2 && outside_star_catalog(x2));
    return false;
}

/* One pass; after an RTB (R51C's LOADTIME, CHKSD's SGNAGREE, CAL53A's
 * RDCDUS), the routine (src/rtb_op_codes/, or RDCDUS in r55.c) and the
 * pass on from its DANZIG, as the faithful pair runs them. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 p66_bias_io *io, const struct interpretive_code *code)
{
    void (*routine)(void);

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z == LOADTIME && FBANK == LOADTIME_BANK)
        routine = loadtime;
    else if (Z == SGNAGREE && FBANK == SGNAGREE_BANK)
        routine = sgnagree;
    else if (Z == RDCDUS && FBANK == RDCDUS_BANK)
        routine = rdcdus;
    else
        return EXEC_OK;
    if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    routine();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_lm_p51_p53_basic, faithful_lm_p51_p53_interpretive;

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    p66_bias_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_lm_p51_p53_basic.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)((key / 1024u) << 10) || !sane(entry->with_work_area))
        return EXEC_OUT_OF_SCOPE;

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
    p66_bias_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)))
        return faithful_lm_p51_p53_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || !code ||
        LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) ||
        !sane(true))
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

/* For the side-by-side test: whether a key runs readable code. */
bool p51_readable(uint16_t key, bool order_pair)
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
#include "p51_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "p51_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "p51_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_p51_p53_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_p51_p53_basic = {basic_keys, COUNT(basic_keys)};

#endif
