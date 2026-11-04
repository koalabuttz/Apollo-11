/* SPDX-License-Identifier: MIT */
/* The readable P34-35_P74-75 (src/p34_p35/) as routines of each mission's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_p34_p35.c and cm_p34_p35.c): <m>_p34_p35_basic (the
 * basic code) and <m>_p34_p35_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (p34_keys.inc and cm_p34_keys.inc, which tools/p20_p25_keys.py writes
 * from them), and every key runs a readable routine: the file was written
 * part by part, and is whole. The side-by-side test (tests/p34_cases.c)
 * checks that no key is left without one.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (AVFLAGA, AVFLAGP, P20FLGON, ALARM, BANKCALL, GOTOPOOH,
 * or a return) or reaches NEWOPS through a TC INTPRET. An order
 * pair is one pass of the readable interpreter over the listing's words
 * (interpret_listing()), which ends at the pair's DANZIG, at an EXIT, or
 * where a CALL or GOTO leaves its table; a pass that ends at RTB LOADTIME
 * (P35/P75B's) runs LOADTIME (src/rtb_op_codes/) and the DANZIG it ends
 * at, as the faithful pair does.
 *
 * The device is the mission's io record (mission_io); the routines want
 * no hardware record. The states the faithful routines take: FB the key's
 * bank, with the superbank bit clear in banks 30 to 37 (any FB in fixed-
 * fixed memory, banks 2 and 3: the CM's VNPOOH); at an order pair
 * Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. Always
 * ZERO +0, the bank registers agreeing, NEWJOB +0 or a core set, FIXLOC
 * one of the VAC areas, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../p34_p35/p34_p35.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define SUPERBANK_BIT      0100u
#define LOADTIME_BANK      ((word)(I_LOADTIME & 076000u))
#define DPMODE_BANK        ((word)(I_DPMODE & 076000u))
#if LUMINARY099
#define R36_BANK 04u /* R36's display code */
#else
#define R36_BANK 041u
#endif

#if LUMINARY099
#define ROPE                  0u /* the LM's rope */
#define readable_basic        lm_p34_p35_basic
#define readable_interpretive lm_p34_p35_interpretive
#define KEYS                  "p34_keys.inc"
#else
#define ROPE                  1u /* the CM's */
#define readable_basic        cm_p34_p35_basic
#define readable_interpretive cm_p34_p35_interpretive
#define KEYS                  "cm_p34_keys.inc"
#endif

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

/* The key's bank selected: FB, and in banks 30 to 37 the superbank bit
 * clear; banks 40 to 47 are FB 30 to 37 with it set (the CM's R36). Banks
 * 2 and 3 are fixed-fixed, there whatever FB holds. */
static bool in_bank(unsigned bank)
{
    if (bank == 2u || bank == 3u) return true;
    if (bank >= 040u)
        return FBANK == (word)((bank - 010u) << 10) && (*agc.superbank & SUPERBANK_BIT);
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P34 and P74 */
    {KEY(035u, P34), p34},
    {KEY(035u, P34 + 1u), p34_after_avflaga},
    {KEY(035u, P74), p74},
    {KEY(035u, P34_P74A), p34_p74a},
    {KEY(035u, P34_P74A + 1u), p34_p74a_after_p20flgon},
    {KEY(035u, P34_P74A + 3u), p34_p74a_after_vnpooh},
#if LUMINARY099
    {KEY(035u, P34_P74A + 011u), p34_p74a_after_displaye},
#else
    {KEY(035u, P34_P74A + 7u), p34_p74a_after_nn},
    {KEY(035u, P34_P74A + 010u), p34_p74a_after_displaye},
#endif
    {KEY(035u, P34_P74E - 023u), p34_alarm},
    {KEY(035u, P34_P74E - 021u), p34_after_alarm},
    {KEY(035u, P34_P74E - 016u), p34_terminate},
    {KEY(035u, P34_P74E - 015u), p34_proceed},
    {KEY(035u, P34_P74E - 014u), p34_recycle},
    {KEY(035u, P34_P74E - 5u), swchclr_after_exit},
    {KEY(035u, P34_P74E - 4u), swchclr_after_displaye},
    {KEY(035u, P34_P74E - 2u), p34_p74d_after_exit},
    {KEY(035u, P34_P74E), p34_p74e},
    {KEY(035u, P34_P74E + 054u), p34_p74e_after_exit},
    {KEY(035u, P34_P74E + 056u), p34_p74e_after_v06n58},
    /* Part 2: P35 and P75 */
    {KEY(035u, P35), p35},
    {KEY(035u, P35 + 1u), p35_after_avflaga},
    {KEY(035u, P75), p75},
    {KEY(035u, P75 + 1u), p75_after_avflagp},
    {KEY(035u, P35_P75A + 2u), p35_p75a_after_p20flgon},
/* Part 3: S34/35.5, VN1645 and its clock */
#if LUMINARY099
    {KEY(035u, I_FLAGON + 5u), s34_35_5_after_exit},
    {KEY(035u, I_FLAGON + 010u), s34_35_5_terminate},
    {KEY(035u, I_FLAGON + 011u), s34_35_5_proceed},
    {KEY(035u, I_FLAGON + 012u), s34_35_5_recycle},
    {KEY(035u, I_FLAGON + 013u), s34_35_5_compare},
    {KEY(035u, I_FLAGON + 034u), s34_35_5_after_upflag},
#else
    {KEY(035u, I_FLAGON + 4u), s34_35_5_after_exit},
    {KEY(035u, I_FLAGON + 6u), s34_35_5_after_vnpooh},
#endif
    {KEY(035u, I_FLAGOFF + 3u), flagoff_after_exit},
    {KEY(035u, I_FLAGOFF + 5u), flagoff_after_vnpooh},
    {KEY(035u, I_GET45 - 010u), vn1645_after_exit},
    {KEY(035u, I_GET45 - 7u), vn1645_p3x},
    {KEY(035u, I_GET45 - 6u), vn1645_p7x},
    {KEY(035u, I_GET45 - 5u), vn1645_mga},
    {KEY(035u, I_GET45 + 1u), vn1645_clock},
    {KEY(035u, I_GET45 + 2u), vn1645_after_comptgo},
    {KEY(035u, I_GET45 + 7u), vn1645_after_delayjob},
    {KEY(035u, I_GET45 + 012u), vn1645_terminate},
    {KEY(035u, I_GET45 + 013u), vn1645_proceed},
    {KEY(035u, I_GET45 + 014u), vn1645_recycle},
    {KEY(035u, KILCLOCK), kilclock},
    {KEY(035u, N45PROC), n45proc},
    {KEY(035u, N45PROC + 6u), n45proc_after_phaschng},
    {KEY(035u, CLUPDATE), clupdate},
    {KEY(035u, CLUPDATE + 4u), clupdate_after_phaschng},
/* Part 6: R36 */
#if LUMINARY099
    {KEY(04u, R36), r36_start},
    {KEY(04u, R36 + 6u), r36_time_terminate},
    {KEY(04u, R36 + 7u), r36_time_proceed},
    {KEY(04u, R36 + 010u), r36_time_recycle},
#else
    {KEY(020u, R36), r36},
    {KEY(020u, R36 + 7u), r36_terminate},
    {KEY(020u, R36 + 010u), r36_proceed},
    {KEY(020u, R36 + 011u), r36_loaded},
    {KEY(020u, R36 + 012u), r36_to_r36a},
    {KEY(041u, R36A), r36_start},
    {KEY(041u, R36P3 + 3u), r36_time_terminate},
    {KEY(041u, R36P3 + 4u), r36_time_proceed},
    {KEY(041u, R36P3 + 5u), r36_time_recycle},
#endif
    {KEY(R36_BANK, I_R36TAG2 + 5u), r36_after_exit},
    {KEY(R36_BANK, I_R36TAG2 + 010u), r36_end_terminate},
    {KEY(R36_BANK, I_R36TAG2 + 011u), r36_end_proceed},
    {KEY(R36_BANK, I_R36TAG2 + 012u), r36_redisplay},
    {KEY(R36_BANK, LREGCHK), lregchk},
    /* Part 1: the display subroutines */
    {KEY(035u, DISPLAYE), displaye},
    {KEY(035u, DISPLAYE + 5u), displaye_terminate},
    {KEY(035u, DISPLAYE + 6u), displaye_proceed},
    {KEY(035u, DISPLAYE + 7u), displaye_recycle},
    {KEY(035u, P3XORP7X), p3xorp7x},
#if LUMINARY099
    {KEY(035u, VNPOOH), vnpooh},
    {KEY(035u, VNPOOH + 6u), vnpooh_terminate},
    {KEY(035u, VNPOOH + 7u), vnpooh_proceed},
    {KEY(035u, VNPOOH + 010u), vnpooh_recycle},
#else
    {KEY(02u, VNPOOH), vnpooh},
    {KEY(02u, VNPOOH + 011u), vnpooh_terminate},
    {KEY(02u, VNPOOH + 012u), vnpooh_proceed},
    {KEY(02u, VNPOOH + 013u), vnpooh_recycle},
    {KEY(02u, VNPOOH + 014u), vnpooh_restore},
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

static const struct interpretive_code *const codes[] = {
#if !LUMINARY099
    &P34_NN_CODE,
#endif
    &P34_TTPI_CODE, &P34_SWCHCLR_CODE, &P34_P74D_CODE, &P34_P74E_CODE, &P34_N58_CODE,
    &P35_CODE,      &S33_34_1_CODE,    &FIGTIME_CODE,  &OKMAX_CODE,    &S34_35_1_CODE,
    &S34_35_2_CODE, &S34_35_3_CODE,    &LOMAT_CODE,    &S34_35_5_CODE, &S34_35_5_LOADED_CODE,
    &FLAGOFF_CODE,  &VN1645_CODE,      &GETMGA_CODE,   &CLUPDATE_CODE, &INITVEL_CODE,
    &MIDGIM_CODE,   &SELECTMU_CODE,    &PERIAPO_CODE,  &PRECSET_CODE,  &VECSHIFT_CODE,
    &ASTROTIM_CODE, &ENTTIM2_CODE,     &R36INT_CODE,   &R36B_CODE,     &R36TAG2_CODE,
};

/* The table an address is in, in a bank, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* One pass; after RTB LOADTIME or RTB DPMODE (R36's), the routine and
 * its DANZIG, which goes on with the pair's other order. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    void (*routine)(void);

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z == LOADTIME && FBANK == LOADTIME_BANK)
        routine = loadtime;
    else if (Z == DPMODE && FBANK == DPMODE_BANK)
        routine = dpmode;
    else
        return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    routine();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each key's routine. A basic routine runs from its key, whatever Z
 * holds (the trampoline calls it there). */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(key / 1024u) || !sane()) return EXEC_OUT_OF_SCOPE;

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

/* MUTABLE, read by index: the faithful port carries only the words an
 * index of -2 (the Earth's) or -10D (the Moon's) reads, and refuses a load
 * from anywhere else; so does the binding. SETMUER's DLOAD* and STODL*
 * (SETMUER +3) take X1 as the pass finds it; the LM's INITVELX loads X1
 * from RTX1 in the same pass. */
static bool outside_mutable(uint16_t key)
{
    word x1;

    if (key == KEY(I_SETMUER / 02000u, I_SETMUER) || key == KEY(I_SETMUER / 02000u, I_SETMUER + 3u))
        x1 = agc.memory[(FIXLOC + I_X1) & 03777u];
#if LUMINARY099
    else if (key == KEY(011u, I_INITVELX))
        x1 = agc.memory[I_RTX1];
#endif
    else
        return false;
    return x1 != 077775u && x1 != 077765u;
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane() || outside_mutable(key))
        return EXEC_OUT_OF_SCOPE;

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
bool p34_p35_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
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
#include KEYS
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include KEYS
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include KEYS
};
#undef PAIR
#undef BASIC

const agc_routine_table readable_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table readable_basic = {basic_keys, COUNT(basic_keys)};
