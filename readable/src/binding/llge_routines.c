/* SPDX-License-Identifier: MIT */
/* The readable LUNAR_LANDING_GUIDANCE_EQUATIONS (src/landing_guidance/) as
 * routines of the LM's flight program, under the table names and keys of
 * the faithful port's (c64-port/src/lm_llge.c): lm_llge_basic (the basic
 * code) and lm_llge_interpretive (the order pairs).
 *
 * The file is written part by part, so the tables carry every key of the
 * faithful ones, in their order (llge_keys.inc, which tools/p20_p25_keys.py
 * writes from them): a key the parts written so far cover runs a readable
 * routine; any other runs the faithful port's, built into the program as
 * faithful_lm_llge_basic and faithful_lm_llge_interpretive
 * (tests/x86_readable.py renames them).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (CHECKMM, TWIDDLE, FINDVAC, POWRSERS, USPRCADR, ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()),
 * which ends at the pair's DANZIG, at an EXIT, or where a CALL or branch
 * leaves it. Three keys are the words after ROOTPSRS's TC USPRCADRs,
 * reached at DANZIG with the EXIT that USPRCADR left in EDOP still to run
 * (its LOC the CADR's word): there the pass runs the EXIT.
 *
 * The device is the LM's io record with the input channels after it
 * (lm_channel_io): READ and RAND of channel 31 (the hand controller, the
 * attitude mode), and channel 30. WOR of channel 13 is the program's
 * hardware record's, whose bits 12-14 (the hand controller's traps) read
 * back +0. The states the faithful routines take, at every key: FB the
 * key's bank, with the superbank bit clear in banks 30 to 37; at an order
 * pair Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15; at
 * the words after the TC USPRCADRs Z at DANZIG, BANKSET bank 31's and EDOP
 * the EXIT. Always ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range, and every erasable word an ordinary one.
 * At an order pair and at the basic keys the faithful port marks so (its
 * at_basic: those that can reach a TC INTPRET), NEWJOB +0 or a core set
 * and FIXLOC one of the VAC areas; at the other basic keys (its
 * at_basic_nv) NEWJOB may be -0 too, and FIXLOC anything. A run the
 * readable code refuses part way leaves the state, io record and channels
 * as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#include "../landing_guidance/landing_guidance.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define SUPERBANK_BIT      0100u

/* NORMUNIT's bank (FB), from its FCADR. */
#define NORMUNIT_BANK ((word)(I_NORMUNIT & 076000u))
#define VECTOR_MODE   077776u /* MODE -1 */

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_lm_llge_basic, faithful_lm_llge_interpretive;

static const agc_input_channels *inputs_now;
static agc_hardware *hardware_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channels 30-33, as the run latched them, and 13, the hardware's latch.
 * Channel 13's bits 12-14 (the hand controller traps' resets) read back
 * +0, and a write leaves them as they were or sets them. */
static word channel_in(unsigned number)
{
    switch (number) {
    case 030u: return inputs_now->channel30;
    case 031u: return inputs_now->channel31;
    case 032u: return inputs_now->channel32;
    case 033u: return inputs_now->channel33;
    case 013u:
        if (!hardware_now) not_modelled();
        return (word)(hardware_now->channel13 & 043777u);
    default: not_modelled(); return 0;
    }
}

/* Channel 13 only (WOR). */
static void channel_out(unsigned number, word value)
{
    if (number != 013u || !hardware_now) not_modelled();
    hardware_now->channel13 = (uint16_t)((hardware_now->channel13 & 034000u) | value);
}

/* The interpreter's fixed operands are the rope's. */
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
    hardware_now = program->hardware;
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

/* The key's bank selected: FB, and in banks 30 to 37 the superbank bit
 * clear (with it set, FB 30 to 37 would be banks 40 to 47). */
static bool in_bank(unsigned bank)
{
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
    bool with_work_area; /* the faithful routine's at_basic (else at_basic_nv) */
} basic_entries[] = {
    /* Part 1: the redesignator and rate of descent traps */
    {KEY(011u, PITFALL), pitfall, false},
    {KEY(011u, PITFALL + 5u), pitfall_not_p64, false},
    {KEY(011u, PITFALL + 6u), pitfall_in_p64, false},
    {KEY(011u, PITFALL + 020u), pitfall_after_twiddle, false},
    {KEY(011u, REDESMON), redesmon, false},
    {KEY(011u, RESETRPT + 3u), resetrpt_taskover, false},
    {KEY(020u, DESCBITS), descbits, false},
    /* Part 1: ROOTPSRS, INTPRETX and FASTCHNG */
    {KEY(031u, ROOTPSRS), rootpsrs, false},
    {KEY(031u, ROOTPSRS + 021u), rootpsrs_after_shortmp, false},
    {KEY(031u, ROOTPSRS + 023u), rootpsrs_after_abs, false},
    {KEY(031u, DERCLOOP + 3u), dercloop_after_dmpnsub, false},
    {KEY(031u, ROOTLOOP + 6u), rootloop_after_derivative, false},
    {KEY(031u, ROOTLOOP + 015u), rootloop_after_residual, false},
    {KEY(031u, ROOTLOOP + 017u), rootloop_after_ddv, false},
    {KEY(031u, ROOTLOOP + 024u), rootloop_after_abs, true},
    {KEY(031u, INTPRETX), intpretx, true},
    {KEY(031u, FASTCHNG), fastchng, false},
    /* Part 2: QUADGUID to EXTLOGIC */
    {KEY(031u, QUADGUID), quadguid, true},
    {KEY(031u, AFCCLEND + 1u), afcclend_after_exit, true},
    {KEY(031u, AFCCLEND + 2u), afcclend_after_fastchng, true},
    {KEY(031u, EXTLOGIC), extlogic, true},
    {KEY(031u, EXSPOT1 + 4u), exspot1_after_fastchng, true},
    /* Part 2: the exits */
    {KEY(031u, EXGSUB + 011u), exgsub_after_exit, true},
    {KEY(031u, EXGSUB + 017u), exgsub_postjump, false},
    {KEY(031u, EXBRAK + 5u), steer, true},
    {KEY(031u, EXNORM + 013u), exnorm_after_exit, true},
    {KEY(031u, RATESTOP), ratestop, true},
    {KEY(031u, RATESTOP + 7u), dispexit, true},
    {KEY(031u, GDUMP1 + 1u), gdump1_after_throttle, true},
    /* Part 2: the displays */
    {KEY(031u, DISPEXIT), dispexit, true},
    {KEY(031u, DISPEXIT + 3u), dispexit_plus3, true},
    {KEY(031u, P63DISPS - 2u), p63disps_phaschng, false},
    {KEY(031u, P63DISPS), p63disps, false},
    {KEY(031u, P63DISPS + 3u), endlljob, false},
    {KEY(031u, P64DISPS + 012u), p64disps_terminate, false},
    {KEY(031u, P64DISPS + 013u), p64ceed, false},
    {KEY(031u, P64DISPS + 014u), p64disps, false},
    {KEY(031u, P64DISPS + 015u), endlljob, false},
    {KEY(031u, P64CEED + 5u), p64ceed_after_upflag, false},
    {KEY(031u, REDES_OK), redes_ok, false},
    /* Part 2: P67 */
    {KEY(031u, P67VERT + 2u), p67vert_after_phaschng, true},
    /* Part 2: 1406ALM's return */
    {KEY(031u, ALM1406 + 2u), alm1406_after_alarm, true},
    /* Part 1: P66 */
    {KEY(032u, RODTASK), rodtask, false},
    {KEY(032u, RODTASK + 4u), rodtask_after_findvac, false},
    {KEY(032u, P66VERTA), p66verta, false},
    {KEY(032u, P66VERTA + 2u), p66verta_after_phaschng, false},
    {KEY(032u, RODCOMP), rodcomp, true},
    {KEY(032u, ITRPNT2 + 1u), itrpnt2_after_exit, false},
    {KEY(032u, ITRPNT2 + 4u), itrpnt2_after_throttle, true},
    {KEY(032u, VHORCOMP + 012u), vhorcomp_after_exit, false},
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
    &QUADGUID_CODE, &CGCALC_CODE,  &EXGSUB_CODE,  &EXBRAK_CODE,  &EXNORM_CODE,
    &GDUMP1_CODE,   &P65VERT_CODE, &P67VERT_CODE, &ITRPNT1_CODE, &VHORCOMP_CODE};

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

/* The words after ROOTPSRS's TC USPRCADRs: CADR ABS, CADR DDV/BDDV and
 * CADR ABS. */
static bool after_usprcadr(uint16_t key)
{
    return key == KEY(031u, ROOTPSRS + 022u) || key == KEY(031u, ROOTLOOP + 016u) ||
           key == KEY(031u, ROOTLOOP + 023u);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each key's routine: the readable one, or the faithful port's. A basic
 * routine runs from its key, whatever Z holds (the trampoline calls it
 * there). */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    agc_word channel13;

    if (!entry) return faithful_lm_llge_basic.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(key / 1024u) || !sane(entry->with_work_area)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    channel13 = hardware_now ? hardware_now->channel13 : 0u;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        if (hardware_now) hardware_now->channel13 = channel13;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return EXEC_OK;
}

/* One pass; after RTB NORMUNIT, NORMUNIT and the DANZIG it ends at, on
 * through the rest of the pair, as the faithful pair does. Its alarm 1302
 * (POODOO1) ends the pair. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 lm_channel_io *io, const struct interpretive_code *code)
{
    enum exec_status status = interpret_listing(program, state, &io->io, code);

    if (status != EXEC_OK) return status;
    bind(program, state, io);
    if (Z != NORMUNIT || FBANK != NORMUNIT_BANK) return EXEC_OK;
    if (MODE != VECTOR_MODE || !sane(true))
        return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it: a vector) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    normunit();
    if (Z == POODOO1) return EXEC_OK;
    return interpret_listing(program, state, &io->io, code);
}

static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = NULL;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)) && !after_usprcadr(key))
        return faithful_lm_llge_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (after_usprcadr(key)) {
        if (Z != DANZIG || (BANKSET & 076000u) != (word)(bank << 10) || (BANKSET & 01770u) ||
            (*agc.superbank & SUPERBANK_BIT) || EDOP != 1u || !sane(true))
            return EXEC_OUT_OF_SCOPE;
    } else {
        code = code_at(bank, LOC);
        if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
            INTBIT15 != (FBANK & 040000u) || !sane(true))
            return EXEC_OUT_OF_SCOPE;
    }

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
bool llge_readable(uint16_t key, bool order_pair)
{
    return order_pair
               ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL || after_usprcadr(key)
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
#include "llge_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "llge_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "llge_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_llge_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_llge_basic = {basic_keys, COUNT(basic_keys)};

#endif
