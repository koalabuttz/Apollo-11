/* SPDX-License-Identifier: MIT */
/* The readable FINDCDUW--GUIDAP_INTERFACE (src/findcduw/) as routines of
 * the LM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_findcduw.c): lm_findcduw_basic (the basic code)
 * and lm_findcduw_interpretive (the order pairs), all in bank 30. The
 * tables carry every key of the faithful ones, in their order
 * (findcduw_keys.inc, which tools/p20_p25_keys.py writes from them), and
 * every key runs a readable routine.
 *
 * Plumbing only. A basic routine runs from its key until its code leaves
 * the file (ALARM, IBNKCALL, a TC through TEM2 or TEM4, a TC Q to a return
 * the file does not hold) or reaches NEWOPS through a TC INTPRET. An order
 * pair is one pass of the readable interpreter over the listing's words
 * (interpret_listing()), which ends at the pair's DANZIG, at an EXIT, or
 * where a CALL or branch leaves it; a pass that ends at an RTB of NORMUNIT
 * or QUICTRIG runs it (and QUICTRIG's return, SWRETURN) and goes on, as the
 * faithful pair does. NORMUNIT's alarm 1302 (POODOO1) the faithful pair
 * refuses, and so does this one.
 *
 * The device is the LM's io record with the input channels after it
 * (lm_channel_io): RAND of channels 30 and 31 (PNGCS control, the auto
 * mode). The states the faithful routines take, at every key: FB bank 30,
 * the superbank bit clear; at an order pair Z at NEWOPS, BANKSET the BBANK
 * and INTBIT15 bank 30's bit 15. Always ZERO +0, the bank registers
 * agreeing, the interrupt and superbank latches in range, NEWJOB +0 or a
 * core set, FIXLOC one of the VAC areas, and every erasable word an
 * ordinary one. A run the readable code refuses part way leaves the state
 * and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#include "../findcduw/findcduw.h"
#include "../powered_flight/powered_flight.h"
#include "../interpreter/interpreter.h"
#include "../interbank/interbank.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define SUPERBANK_BIT      0100u

/* NORMUNIT's and QUICTRIG's banks (FB), from their FCADRs. */
#define NORMUNIT_BANK ((word)(I_NORMUNIT & 076000u))
#define QUICTRIG_BANK ((word)(I_QUICTRIG & 076000u))
#define VECTOR_MODE   077776u /* MODE -1 */

static const agc_input_channels *inputs_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channels 30 and 31, as the run latched them. */
static word channel_in(unsigned number)
{
    switch (number) {
    case 030u: return inputs_now->channel30;
    case 031u: return inputs_now->channel31;
    default: not_modelled(); return 0;
    }
}

/* The file writes no channel. */
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

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_channel_io *io)
{
    bind_machine(program, state, &io->io);
    inputs_now = &io->in;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its sane()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* Bank 30 selected: FB, and the superbank bit clear (with it set, FB 30
 * would be bank 40). */
static bool in_bank_30(void)
{
    return FBANK == (word)(030u << 10) && !(*agc.superbank & SUPERBANK_BIT);
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    {KEY(030u, FINDCDUW + 5u), findcduw_after_exit},
    {KEY(030u, PAUTNO + 020u), thrust_filter},
    {KEY(030u, PAUTNO + 023u), thrust_filter_after_y},
    {KEY(030u, PAUTNO + 027u), thrust_filter_after_z},
    {KEY(030u, MGARET - 012u), limit_mga},
    {KEY(030u, MGARET - 6u), limit_mga_after_limitsub},
    {KEY(030u, MGARET + 043u), limit_attitude_after_z},
    {KEY(030u, MGARET + 053u), limit_attitude_after_y},
    {KEY(030u, MGARET + 070u), limit_attitude_after_x},
    {KEY(030u, CDUWXFR + 6u), cduwxfr_after_onesto2s},
    {KEY(030u, CDUWXFR + 030u), cduwxfr_after_limitsub},
    {KEY(030u, FLTRSUB + 013u), fltrsub_after_increment},
    {KEY(030u, FLTRSUB + 017u), fltrsub_after_total},
    {KEY(030u, NB2CDUSP + 010u), nb2cdusp_after_exit},
    {KEY(030u, NB2CDUSP + 023u), nb2cdusp_store_cduz},
    {KEY(030u, NB2CDUSP + 026u), nb2cdusp_cduy_second},
    {KEY(030u, NB2CDUSP + 030u), nb2cdusp_cduy_angle},
    {KEY(030u, NB2CDUSP + 032u), nb2cdusp_store_cduy},
    {KEY(030u, NB2CDUSP + 035u), nb2cdusp_cdux_second},
    {KEY(030u, NB2CDUSP + 037u), nb2cdusp_cdux_angle},
    {KEY(030u, NB2CDUSP + 041u), nb2cdusp_store_cdux},
    {KEY(030u, ARCTRGSP + 025u), arctrgsp_after_sparcsin},
    {KEY(030u, ARCTRGSP + 026u), arctrgsp_after_onesto2s},
    {KEY(030u, ONE_TO2_TCQ + 1u), one_to2_tcq_plus1},
    {KEY(030u, USECOS + 2u), usecos_after_sparcsin},
    {KEY(030u, USECOS + 016u), sparcsin_minus1}, /* SPARCSIN -1 */
    {KEY(030u, NOATTCNT), noattcnt},
    {KEY(030u, NOATTCNT + 2u), noattcnt_plus2},
    {KEY(030u, NOATTCNT + 5u), noattcnt_after_stoprate},
    {KEY(030u, ALARMMGA + 2u), alarmmga_after_alarm},
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
    &INITCDUW_CODE, &FINDCDUW_CODE, &PAUTNO_CODE,   &AFTRFLTR_CODE,
    &TCQCDUW_CODE,  &UNWCTEST_CODE, &NB2CDUSP_CODE, &NB2CDUSP_RVQ_CODE};

/* The table an address is in, in bank 30, or none. */
static const struct interpretive_code *code_at(word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (address >= codes[i]->first && address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* ---- The keys, in the faithful tables' order ---- */

static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank_30() || !sane()) return EXEC_OUT_OF_SCOPE;

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

/* One pass; after an RTB of NORMUNIT or QUICTRIG, the routine (QUICTRIG
 * ends at SWRETURN) and the DANZIG it ends at, on through the rest of the
 * pair, as the faithful pair does. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 lm_channel_io *io, const struct interpretive_code *code)
{
    for (;;) {
        enum exec_status status = interpret_listing(program, state, &io->io, code);

        if (status != EXEC_OK) return status;
        bind(program, state, io);
        if (Z == NORMUNIT && FBANK == NORMUNIT_BANK) {
            if (MODE != VECTOR_MODE || !sane())
                return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it: a vector) */
            if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
            normunit();
            if (Z == POODOO1) return EXEC_OUT_OF_SCOPE;
        } else if (Z == QUICTRIG && FBANK == QUICTRIG_BANK) {
            if (!sane()) return EXEC_OUT_OF_SCOPE;
            if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
            quictrig();
            swreturn();
        } else {
            return EXEC_OK;
        }
    }
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(LOC);
    if (Z != NEWOPS || !in_bank_30() || !code || LOC != (word)(02000u + key % 1024u) ||
        BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) || !sane())
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
bool findcduw_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at((word)(02000u + key % 1024u)) != NULL : basic_entry(key) != NULL;
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
#include "findcduw_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "findcduw_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "findcduw_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_findcduw_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_findcduw_basic = {basic_keys, COUNT(basic_keys)};

#endif
