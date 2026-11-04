/* SPDX-License-Identifier: MIT */
/* The readable P32-P33_P72-P73 (src/p32_p33_cm/) as routines of the CM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_p32_p33.c): cm_p32_p33_basic (the basic code, banks 4
 * and 37) and cm_p32_p33_interpretive (the order pairs, in fifteen
 * banks).
 *
 * The file is readable part by part, so the tables carry every key of the
 * faithful ones, in their order (cm_p32_keys.inc, which
 * tools/p20_p25_keys.py writes from them): a key a readable part has
 * written runs the readable routine; any other runs the faithful port's,
 * built into the program as faithful_cm_p32_p33_basic and
 * faithful_cm_p32_p33_interpretive (tests/x86_readable.py, the Makefile's
 * cm-p32 tests).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (AVFLAGA, AVFLAGP, P20FLGON, VNPOOH, BANKCALL, ALARM, VARALARM,
 * GOTOPOOH), reaches a label the faithful port keys a routine at, or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()),
 * which ends at the pair's DANZIG, at an EXIT, or where a CALL or GOTO
 * leaves its table.
 *
 * The device is the CM's io record (cm_native_io); the routines want no
 * hardware record. The states the faithful routines take: FB the key's
 * bank (in banks 30 to 37 with the superbank bit clear); at an order pair
 * Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. Always
 * ZERO +0, the bank registers agreeing, NEWJOB +0 or a core set, FIXLOC
 * one of the VAC areas, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p32_p33.h"
#include "../p32_p33_cm/p32_p33_cm.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if COMANCHE055

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p32_p33_basic, faithful_cm_p32_p33_interpretive;

/* A key: the bank, and the address in it. */
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

/* The key's bank selected: FB, and for banks 30 to 37 the superbank bit
 * clear (with it set, FB 30 to 33 would be banks 40 to 43). */
static bool in_bank(unsigned bank)
{
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P32 and P72 */
    {KEY(04u, P32), p32},
    {KEY(04u, P32 + 1u), p32_after_avflaga},
    {KEY(04u, P72), p72},
    {KEY(04u, P32STRT), p32strt},
    {KEY(04u, P32STRT + 5u), p32strt_after_exit},
    {KEY(04u, ALMXIT + 4u), almxit_after_exit},
    {KEY(04u, ALMXIT + 6u), almxit_after_varalarm},
    {KEY(04u, ALMXIT + 011u), almxit_terminate},
    {KEY(04u, ALMXIT + 012u), almxit_proceed},
    {KEY(04u, P32_P72A), p32_p72a},
    {KEY(04u, P32_P72A + 1u), p32_p72a_after_p20flgon},
    {KEY(04u, P32_P72A + 6u), vn0611},
    {KEY(04u, P32_P72A + 010u), vn0655},
    {KEY(04u, P32_P72A + 013u), vn0655_terminate},
    {KEY(04u, P32_P72A + 014u), vn0655_proceed},
    {KEY(04u, P32_P72A + 015u), vn0655_recycle},
    {KEY(04u, P32_P72A + 016u), vn0637},
    {KEY(04u, P32_P72A + 020u), vn0637_after_vnpooh},
    {KEY(04u, P32_P72F + 5u), p32_p72_n75},
    {KEY(04u, P32_P72F + 7u), p32_p72_n75_after_vnpooh},
    /* Part 1: P33 and P73 */
    {KEY(04u, P33), p33},
    {KEY(04u, P33 + 1u), p33_after_avflaga},
    {KEY(04u, P73), p73},
    {KEY(04u, P33_P73A), p33_p73a},
    {KEY(04u, P33_P73A + 1u), p33_p73a_after_p20flgon},
    {KEY(04u, P33_P73A + 3u), p33_p73a_after_vnpooh},
    {KEY(04u, P33_P73B + 033u), p33_p73_alarm},
    {KEY(04u, P33_P73B + 035u), p33_p73_after_alarm},
    {KEY(04u, P33_P73B + 040u), p33_p73_terminate},
    {KEY(04u, P33_P73B + 041u), p33_p73_proceed},
    {KEY(04u, P33_P73B + 042u), p33_p73_recycle},
    {KEY(04u, P33_P73B + 043u), p33_p73_after_proceed},
    {KEY(04u, P33_P73F + 010u), p33_p73_n75},
    {KEY(04u, P33_P73F + 012u), p33_p73_n75_after_vnpooh},
    /* Part 1: DISDVLVC */
    {KEY(037u, DISDVLVC + 013u), disdvlvc_after_exit},
    {KEY(037u, DISDVLVC + 015u), disdvlvc_after_vnpooh},
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
    &P32STRT_CODE,     &ALMXITA_CODE,  &P32_P72A_CODE,      &VN0637_CODE,   &P32_P72E_CODE,
    &P32_P72_N75_CODE, &P33_P73A_CODE, &P33_P73_ALARM_CODE, &P33_P73C_CODE, &P33_P73E_CODE,
    &P33_P73_N75_CODE, &DISDVLVC_CODE, &DISDVLVC_EXIT_CODE, &CSI_A_CODE,    &CSI_B2_CODE,
    &CSI_B23_CODE,     &CSINEXT_CODE,
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

/* An indexed load's address, as INDEX2 works it out: the operand's
 * address with INTBIT15, ADS -X1, overflow corrected. */
static unsigned indexed_address(unsigned operand)
{
    word x1 = agc.memory[(FIXLOC + ADDRESS_OF(X1)) & 03777u];
    unsigned minus_x1 = ~(x1 | (x1 & 040000u) << 1) & 0177777u; /* CS X1, 16 bits */
    unsigned sum = minus_x1 + (operand | (operand & 040000u) << 1);

    if (sum > 0177777u) sum = (sum + 1u) & 0177777u; /* end around */
    return (sum & 037777u) | (sum >> 1 & 040000u);   /* overflow corrected */
}

/* The indexed load the faithful port carries only a few words for, and
 * refuses from anywhere else, so the binding works its address out before
 * the pass: ALMXIT's SLOAD* ALARM/TB -1,1 (4,3320), by X1 from LXC,1
 * CSIALRM: the seven words ALARM/TB to ALARM/TB +6 (bank 6). */
static bool outside_table(uint16_t key)
{
    unsigned address;

    if (key == KEY(04u, ALMXIT + 2u)) {
        address = indexed_address(I_ALARM_TB - 1u);
        return address < I_ALARM_TB || address > I_ALARM_TB + 6u;
    }
    return false;
}

/* Whether a pass that starts at store code w (a CHANG2 resume) would store
 * past the end of the E-bank window (01777): the AGC then writes into
 * fixed memory and reads it back into A and L, which the readable machine
 * does not model, so the binding refuses it. DOSTORE's index (X1 or X2)
 * and STORE's addressing, from w - 1 (NEWOPS's CCS); MODE says how many
 * words (DP, TP or vector). */
static bool store_spills(word w)
{
    unsigned v = (unsigned)w - 1u, kind = (v >> 11) & 7u, a = v & 03777u, n;

    if (kind == 1u || kind == 2u)
        a = (a + (~agc.memory[(FIXLOC + ADDRESS_OF(X1) + kind - 1u) & 03777u] & 077777u)) % 077777u;
    if (a <= 45u) return false; /* the work area, or word 45 itself */
    n = MODE == 0u || MODE == 077777u ? 1u : (MODE & 040000u) ? 5u : 2u;
    return (a & 0377u) + n > 0377u;
}

/* Whether a pass in a bank below 30 runs with the superbank bit set. The
 * faithful pairs there take the bit either way, and read their constants
 * where the listing has them; but the bit moves banks 30 to 37 to 40 to
 * 47, so the AGC would read the constants this file keeps in bank 35
 * (ONETHTH, NICKELDP, 1RTEB2, PMINE...) from bank 45, which the rope
 * lacks. No program runs the file so, and the binding refuses it. */
static bool superbank_set_below_30(unsigned bank)
{
    return bank < 030u && (*agc.superbank & SUPERBANK_BIT);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each key's routine: the readable one, or the faithful port's. A basic
 * routine runs from its key, whatever Z holds (the trampoline calls it
 * there). */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_cm_p32_p33_basic.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
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

/* P33/P73E's DSU BPL branches back to itself while the time stays
 * positive. The faithful pair runs its passes round that loop in one call
 * (to 4,096 of them), for as long as the state is one its pairs take, so
 * the binding does too. */
static enum exec_status loop_back(const agc_native_program *program, apollo_overlay_state *state,
                                  cm_native_io *io)
{
    unsigned loops;

    for (loops = 1u; loops < 4096u; ++loops) {
        bind(program, state, io);
        if (Z != NEWOPS || LOC != I_P33_P73E % 02000u + 02000u || !in_bank(04u) ||
            BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) || !sane())
            return EXEC_OK;
        if (interpret_listing(program, state, io, &P33_P73E_CODE) != EXEC_OK)
            return EXEC_OUT_OF_SCOPE;
    }
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
        return faithful_cm_p32_p33_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane() || outside_table(key) ||
        superbank_set_below_30(bank))
        return EXEC_OUT_OF_SCOPE;
    if (!(code->words[LOC - code->first] & 040000u) && store_spills(code->words[LOC - code->first]))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, code);
    if (status == EXEC_OK && key == KEY(04u, I_P33_P73E)) status = loop_back(program, state, io);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p32_p33_readable(uint16_t key, bool order_pair)
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
#include "cm_p32_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p32_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p32_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p32_p33_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p32_p33_basic = {basic_keys, COUNT(basic_keys)};

#endif
