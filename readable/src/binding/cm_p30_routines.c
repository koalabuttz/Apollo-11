/* SPDX-License-Identifier: MIT */
/* The readable CM P30-P37 (src/p30_cm/) as routines of the CM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_p30_p37.c): cm_p30_p37_basic (the basic code, banks
 * 35 and fixed-fixed) and cm_p30_p37_interpretive (the order pairs, banks
 * 31, 32 and 35).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, PHASCHNG, 2PHSCHNG, WAITLIST, UPFLAG, DOWNFLAG,
 * ENDOFJOB, GOTOPOOH, or a return saved in an erasable word) or reaches
 * NEWOPS through a TC INTPRET. An order pair is one pass of the readable
 * interpreter over the listing's words (interpret_listing()), which ends
 * at the pair's DANZIG, at an EXIT, or where a CALL, GOTO or RVQ leaves
 * its table.
 *
 * The device is the CM's io record (cm_native_io); the routines want no
 * hardware record. The states the faithful routines take: FB the key's
 * bank with the superbank bit clear (fixed-fixed code under any FB); at
 * an order pair Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit
 * 15. Always ZERO +0, the bank registers agreeing, NEWJOB +0 or a core
 * set, FIXLOC one of the VAC areas, the interrupt and superbank latches in
 * range and every erasable word an ordinary one. A run the readable code
 * refuses part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p30_p37.h"
#include "../p30_cm/p30_cm.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if COMANCHE055

/* Keys: the bank, and the address in it (or in fixed-fixed memory). */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define SUPERBANK_BIT      0100u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(1u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, cm_native_io *io)
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

/* The key's bank selected: FB, with the superbank bit clear (the file's
 * banks are 31, 32 and 35). */
static bool in_bank(unsigned bank)
{
    return FBANK == (word)(bank << 10) && !(*agc.superbank & SUPERBANK_BIT);
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* AVFLAGA, AVFLAGP and P20FLGON (fixed-fixed) */
    {KEY(02u, AVFLAGA), avflaga},
    {KEY(02u, AVFLAGA + 4u), avflaga_after_avflag},
    {KEY(02u, AVFLAGP), avflagp},
    {KEY(02u, AVFLAGP + 4u), avflagp_after_avflag},
    {KEY(02u, P20FLGON), p20flgon},
    {KEY(02u, P20FLGON + 4u), p20flgon_after_trackflg},
    {KEY(02u, P20FLGON + 6u), p20flgon_after_updatflg},
    /* DISPMGA, DISP45 and COMPTGO (bank 35) */
    {KEY(035u, DISPMGA + 2u), dispmga_after_exit},
    {KEY(035u, DISP45), disp45},
    {KEY(035u, DISP45 + 3u), disp45_terminate},
    {KEY(035u, DISP45 + 4u), disp45_proceed},
    {KEY(035u, DISP45 + 5u), disp45_recycle},
    {KEY(035u, P30PHSI), p30phsi},
    {KEY(035u, P30PHSI + 2u), p30phsi_after_phaschng},
    {KEY(035u, COMPTGO), comptgo},
    {KEY(035u, COMPTGO + 4u), comptgo_after_timrflag},
    {KEY(035u, COMPTGO + 012u), comptgo_after_waitlist},
    {KEY(035u, COMPTGO + 016u), comptgo_after_2phschng},
    /* P30 */
    {KEY(035u, P30), p30},
    {KEY(035u, P30 + 1u), p30_after_p30_p31},
    {KEY(035u, P30 + 2u), p30_after_cntnup30},
    {KEY(035u, P30 + 4u), p30_after_updatflg},
    {KEY(035u, P30 + 010u), p30_after_exit},
    {KEY(035u, P30 + 011u), p30_after_param30},
    {KEY(035u, P30 + 013u), p30_after_xdelvflg},
    /* P31 */
    {KEY(035u, P31), p31},
    {KEY(035u, P31 + 1u), p31_after_p30_p31},
    {KEY(035u, P31 + 3u), p31_after_updatflg},
    {KEY(035u, P31 + 5u), p31_after_normsw},
    {KEY(035u, P31 + 011u), p31_after_exit},
    {KEY(035u, P31 + 012u), p31_after_cntnup30},
    {KEY(035u, P31 + 013u), p31_after_param30},
    {KEY(035u, P31 + 015u), p31_after_xdelvflg},
    /* P30/P31, CNTNUP30 and PARAM30 */
    {KEY(035u, P30_P31 + 4u), p30_p31_after_updatflg},
    {KEY(035u, P30_P31 + 6u), p30_p31_after_trackflg},
    {KEY(035u, P30_P31 + 011u), p30_p31_terminate},
    {KEY(035u, P30_P31 + 012u), p30_p31_proceed},
    {KEY(035u, P30_P31 + 013u), p30_p31_recycle},
    {KEY(035u, P30_P31 + 014u), p30_p31_immediate_return},
    {KEY(035u, P30_P31 + 016u), p30_p31_after_phaschng},
    {KEY(035u, CNTNUP30 + 5u), cntnup30_terminate},
    {KEY(035u, CNTNUP30 + 6u), cntnup30_proceed},
    {KEY(035u, CNTNUP30 + 7u), cntnup30_recycle},
    {KEY(035u, PARAM30 + 5u), param30_terminate},
    {KEY(035u, PARAM30 + 6u), param30_proceed},
    {KEY(035u, PARAM30 + 7u), param30_recycle},
    {KEY(035u, FLASHMGA + 3u), flashmga_after_exit},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* A basic routine runs from its key, whatever Z holds (the trampoline
 * calls it there). */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    /* (fixed-fixed code, bank 02, runs under any FB) */
    if ((key / 1024u != 02u && !in_bank(key / 1024u)) || !sane()) return EXEC_OUT_OF_SCOPE;

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

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    &DISPMGA_CODE,  &END45_CODE, &P30_CODE,   &P31_CODE,     &REFTEST_CODE,
    &FLASHMGA_CODE, &S30_1_CODE, &S31_1_CODE, &DELRSPL_CODE,
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

/* K1K2LOOP's loads by X1 from AUGEKUGL's tables in bank 32 (DSU* V(32K)
 * +1,1; DMP* YK1K2 +1,1 and DAD* CK1K2 +1,1, a pass later): the faithful
 * port carries only the 13 words of each that a load less X1 0 to 12
 * reads, and refuses a load from anywhere else. The binding works the
 * address out before the pass, as INDEX2 does (the operand's address
 * with INTBIT15, ADS -X1, overflow corrected); one X1 serves both of the
 * second pass's loads. */
static bool outside_k1k2_tables(uint16_t key)
{
    word x1;
    unsigned operand, minus_x1, sum;

    if (key == KEY(032u, I_K1K2LOOP))
        operand = I_V_32K + 1u;
    else if (key == KEY(032u, I_K1K2LOOP + 3u))
        operand = I_YK1K2 + 1u;
    else
        return false;
    x1 = agc.memory[(FIXLOC + ADDRESS_OF(X1)) & 03777u];
    minus_x1 = ~(x1 | (x1 & 040000u) << 1) & 0177777u; /* CS X1, 16 bits */
    sum = minus_x1 + (operand | (operand & 040000u) << 1);
    if (sum > 0177777u) sum = (sum + 1u) & 0177777u; /* end around */
    sum = (sum & 037777u) | (sum >> 1 & 040000u);    /* overflow corrected */
    return sum > operand || sum + 12u < operand;
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane() || outside_k1k2_tables(key))
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

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p30_p37_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
}

/* ---- The keys, in the faithful tables' order (cm_p30_keys.inc, which
 * tools/p20_p25_keys.py writes from them) ---- */

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
#include "cm_p30_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p30_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p30_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p30_p37_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p30_p37_basic = {basic_keys, COUNT(basic_keys)};

#endif
