/* SPDX-License-Identifier: MIT */
/* The readable AOTMARK (src/aotmark/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_aotmark.c): lm_aotmark_basic (the basic code, bank 07)
 * and lm_aotmark_interpretive (the order pairs, in banks 04, 07 and 15).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, ALARM, NOVAC, RESUME...) or reaches NEWOPS through a
 * TC INTPRET. An order pair is one pass of the readable interpreter over
 * the listing's words (interpret_listing()); a pass that ends at an RTB
 * of CDULOGIC runs CDULOGIC (src/rtb_op_codes/) and the DANZIG it ends at,
 * as the faithful pair does.
 *
 * The machine gets channel 16 (NAVKEYIN, the mark buttons) as the run
 * latched it (the device is the LM's io record with the input channels
 * after it, lm_channel_io), and the four MKVB words CHANGEVB indexes.
 *
 * The states the faithful routines take: FB bank 07 for the basic code; at
 * an order pair FB its bank, BANKSET the BBANK, INTBIT15 the bank's bit
 * 15. Always ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range and every erasable word an ordinary one, and
 * NEWJOB +0 or an offset; where the routine uses the job's work area (the
 * order pairs, and the basic entries that index FIXLOC), FIXLOC one of
 * the VAC areas, and elsewhere NEWJOB may be -0 too. A run the readable
 * code refuses part way (not_modelled()) leaves the state and io record
 * as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "../aotmark/aotmark.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* Keys and physical addresses in banks 04, 07 and 15. */
#define BANK07  016000u
#define B4(z)   (uint16_t)(04u * 1024u + ((z) - 02000u))
#define B7(z)   (uint16_t)(07u * 1024u + ((z) - 02000u))
#define B15(z)  (uint16_t)(015u * 1024u + ((z) - 02000u))
#define BANK(n) (word)((n) << 10)

/* CDULOGIC's bank (FB), from its FCADR. */
#define CDULOGIC_BANK ((word)(I_CDULOGIC & 076000u))

static const agc_input_channels *inputs_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static word channel_in(unsigned number)
{
    if (number != NAVKEYIN) not_modelled();
    return inputs_now->channel16;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled(); /* AOTMARK writes no channel */
}

/* The four MKVB words (bank 07), CHANGEVB's only fixed read. */
static word fixed_memory(unsigned physical)
{
    static const word mkvb[4] = MKVB54_WORDS;
    unsigned first = B7(ADDRESS_OF(MKVB54));

    if (physical >= first && physical < first + 4u) return mkvb[physical - first];
    not_modelled();
    return 0;
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_channel_io *io)
{
    bind_machine(program, state, &io->io);
    inputs_now = &io->in;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = fixed_memory;
    agc.leave = leave;
}

/* ---- The states the faithful routines take ---- */

static bool ordinary(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* With a work area: NEWJOB +0 or an offset, FIXLOC a VAC area. */
static bool with_work_area(void)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];

    return ordinary() && newjob <= 037777u && FIXLOC >= 0401u && FIXLOC <= 0661u &&
           (FIXLOC - 0401u) % 054u == 0u;
}

/* Without: NEWJOB may be -0 too, any FIXLOC. */
static bool without_work_area(void)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];

    return ordinary() && (newjob <= 037777u || newjob == MINUS_ZERO);
}

/* ---- The basic entries ---- */

static const struct {
    word at;
    void (*routine)(void);
    bool work_area; /* the routine indexes FIXLOC */
} basic_entries[] = {
    {AOTMARK, aotmark, false},
    {AOTMARK + 3u, aotmark_plus3, false},
    {AOTMARK + 4u, aotmark_plus4, false},
    {EXTVBCHK, extvbchk, false},
    {MKVACFND, mkvacfnd, false},
    {MKVACFND + 011u, mkvacfnd_after_findvac, false},
    {MKRELEAS, mkreleas, false},
    {KILLAOT, killaot, false},
    {GETDAT, getdat, false},
    {GETDAT + 6u, getdat_after_terminate, false},
    {GETDAT + 7u, getdat_after_proceed, true},
    {ENTERDAT, enterdat, false},
    {CODE7 + 3u, code7_after_terminate, false},
    {CODE7 + 4u, code7_after_proceed, true},
    {CODE7 + 5u, code7_after_enter, false},
    {OPTAXIS + 026u, optaxis_after_exit, false},
    {PASTIT + 2u, pastit_after_terminate, false},
    {PASTIT + 3u, pastit_after_proceed, true},
    {PASTIT + 4u, pastit_after_enter, false},
    {AVEIT + 017u, aveit_after_exit, true},
    {ENDMARKS + 5u, endmarks_after_waitlist, false},
    {MKALARM, mkalarm, false},
    {MKALARM + 2u, mkalarm_after_alarm, false},
    {MARKRUPT, markrupt, false},
    {MARKRUPT + 026u, markrupt_after_resume, false},
    {MARKRUPT + 032u, markrupt_after_alarm, false},
    {FINDKEY, findkey, false},
    {FINDKEY + 010u, findkey_plus10, false},
    {FINDKEY + 015u, findkey_plus15, false},
    {SOMEKEY + 3u, somekey_plus3, false},
    {SOMEKEY + 7u, somekey_after_postjump, false},
    {SOMEKEY + 011u, somekey_after_alarm, false},
    {XMKRUPT, xmkrupt, false},
    {YMKRUPT + 5u, ymkrupt_after_marktype, false},
    {VERIFYMK + 7u, verifymk_after_alarm, false},
    {FIVE_MKALARM, five_mkalarm, false},
    {FIVE_MKALARM + 2u, five_mkalarm_after_alarm, false},
    {FIVE_MKALARM + 3u, five_mkalarm_after_marktype, false},
    {MKREJ, mkrej, false},
    {MKREJ + 1u, mkrej_after_marktype, false},
    {REJALM + 2u, rejalm_after_alarm, false},
    {REJECT, reject, false},
    {MARKTYPE, marktype, false},
    {VACSTOR + 027u, vacstor_after_marktype, false},
    {SURFJOB + 4u, surfjob_after_novac, false},
    {CHANGEVB, changevb, false},
    {CHANGEVB + 1u, changevb_after_marktype, false},
    {DSPV6N79 + 3u, dspv6n79_after_terminate, false},
    {DSPV6N79 + 4u, dspv6n79_after_proceed, true},
    {DSPV6N79 + 5u, dspv6n79_after_enter, true},
};

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    unsigned i;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == Z) break;
    if (i == COUNT(basic_entries) || FBANK != BANK07 ||
        !(basic_entries[i].work_area ? with_work_area() : without_work_area()))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    basic_entries[i].routine();
    return EXEC_OK;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {&OANB_CODE, &OPTAXIS_CODE, &YZCHK_CODE,
                                                        &AVESTAR_CODE, &SURFSTAR_CODE};

/* The table LOC is in, under BANKSET, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (BANKSET == BBANK && FBANK == BANK(codes[i]->bank) && LOC >= codes[i]->first &&
            LOC < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* One pass; after an RTB, CDULOGIC and its DANZIG. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             lm_channel_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, &io->io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != CDULOGIC || FBANK != CDULOGIC_BANK) return EXEC_OK;
    if (!ordinary()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    cdulogic();
    return interpret_listing(program, state, &io->io, code);
}

static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at();
    if (Z != NEWOPS || !code || INTBIT15 != (FBANK & 040000u) || !with_work_area())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = pair(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* ---- The keys, in the faithful tables' order ---- */

/* Order pair keys: label + offset, in each bank. */
#define K4(z)  {B4(z), interpretive}
#define K7(z)  {B7(z), interpretive}
#define K15(z) {B15(z), interpretive}
static const agc_routine_entry pairs[] = {
    /* OANB */
    K4(K_OANB),
    K4(K_OANB + 03u),
    K4(K_OANB + 06u),
    K4(K_OANB + 07u),
    K4(K_OANB + 010u),
    K4(K_OANB + 013u),
    K4(K_OANB + 015u),
    K4(K_OANB + 016u),
    K4(K_OANB + 017u),
    K4(K_OANB + 020u),
    K4(K_OANB + 021u),
    K4(K_OANB + 023u),
    K4(K_OANB + 024u),
    K4(K_OANB + 026u),
    K4(K_OANB + 027u),
    K4(K_OANB + 030u),
    K4(K_OANB + 033u),
    K4(K_OANB + 035u),
    K4(K_OANB + 036u),
    /* OPTAXIS, YZCHK, AVESTAR and AVEIT */
    K7(K_OPTAXIS),
    K7(K_OPTAXIS + 02u),
    K7(K_OPTAXIS + 04u),
    K7(K_OPTAXIS + 05u),
    K7(K_OPTAXIS + 06u),
    K7(K_OPTAXIS + 010u),
    K7(K_OPTAXIS + 013u),
    K7(K_OPTAXIS + 015u),
    K7(K_OPTAXIS + 017u),
    K7(K_OPTAXIS + 021u),
    K7(K_OPTAXIS + 024u),
    K7(K_OPTAXIS + 025u),
    K7(K_YZCHK),
    K7(K_YZCHK + 03u),
    K7(K_YZCHK + 06u),
    K7(K_AVESTAR + 014u),
    K7(K_AVESTAR + 020u),
    K7(K_AVESTAR + 022u),
    K7(K_AVESTAR + 024u),
    K7(K_AVESTAR + 026u),
    K7(K_AVESTAR + 030u),
    K7(K_AVESTAR + 032u),
    K7(K_AVESTAR + 033u),
    K7(K_AVEIT),
    K7(K_AVEIT + 03u),
    K7(K_AVEIT + 05u),
    K7(K_AVEIT + 07u),
    K7(K_AVEIT + 011u),
    K7(K_AVEIT + 014u),
    K7(K_AVEIT + 015u),
    K7(K_AVEIT + 016u),
    /* SURFSTAR */
    K15(K_SURFSTAR),
    K15(K_SURFSTAR + 02u),
    K15(K_SURFSTAR + 03u),
    K15(K_SURFSTAR + 06u),
    K15(K_SURFSTAR + 07u),
    K15(K_SURFSTAR + 011u),
    K15(K_SURFSTAR + 012u),
    K15(K_SURFSTAR + 013u),
    K15(K_SURFSTAR + 015u),
    K15(K_SURFSTAR + 017u),
    K15(K_SURFSTAR + 021u),
    K15(K_SURFSTAR + 022u),
    K15(K_SURFSTAR + 025u),
    K15(K_SURFSTAR + 026u),
    K15(K_SURFSTAR + 031u),
    K15(K_SURFSTAR + 033u),
    K15(K_SURFSTAR + 034u),
    K15(K_SURFSTAR + 035u),
    K15(K_SURFSTAR + 037u),
    K15(K_SURFSTAR + 041u),
    K15(K_SURFSTAR + 042u),
    K15(K_SURFSTAR + 044u),
};

static const agc_routine_entry basic_keys[] = {
    {B7(K_AOTMARK), basic},         {B7(K_AOTMARK + 3u), basic},    {B7(K_AOTMARK + 4u), basic},
    {B7(K_EXTVBCHK), basic},        {B7(K_MKVACFND), basic},        {B7(K_MKVACFND + 011u), basic},
    {B7(K_MKRELEAS), basic},        {B7(K_KILLAOT), basic},         {B7(K_GETDAT), basic},
    {B7(K_GETDAT + 6u), basic},     {B7(K_GETDAT + 7u), basic},     {B7(K_ENTERDAT), basic},
    {B7(K_CODE7 + 3u), basic},      {B7(K_CODE7 + 4u), basic},      {B7(K_CODE7 + 5u), basic},
    {B7(K_OPTAXIS + 026u), basic},  {B7(K_PASTIT + 2u), basic},     {B7(K_PASTIT + 3u), basic},
    {B7(K_PASTIT + 4u), basic},     {B7(K_AVEIT + 017u), basic},    {B7(K_ENDMARKS + 5u), basic},
    {B7(K_MKALARM), basic},         {B7(K_MKALARM + 2u), basic},    {B7(K_MARKRUPT), basic},
    {B7(K_MARKRUPT + 026u), basic}, {B7(K_MARKRUPT + 032u), basic}, {B7(K_FINDKEY), basic},
    {B7(K_FINDKEY + 010u), basic},  {B7(K_FINDKEY + 015u), basic},  {B7(K_SOMEKEY + 3u), basic},
    {B7(K_SOMEKEY + 7u), basic},    {B7(K_SOMEKEY + 011u), basic},  {B7(K_XMKRUPT), basic},
    {B7(K_YMKRUPT + 5u), basic},    {B7(K_VERIFYMK + 7u), basic},   {B7(K_5MKALARM), basic},
    {B7(K_5MKALARM + 2u), basic},   {B7(K_5MKALARM + 3u), basic},   {B7(K_MKREJ), basic},
    {B7(K_MKREJ + 1u), basic},      {B7(K_REJALM + 2u), basic},     {B7(K_REJECT), basic},
    {B7(K_MARKTYPE), basic},        {B7(K_VACSTOR + 027u), basic},  {B7(K_SURFJOB + 4u), basic},
    {B7(K_CHANGEVB), basic},        {B7(K_CHANGEVB + 1u), basic},   {B7(K_DSPV6N79 + 3u), basic},
    {B7(K_DSPV6N79 + 4u), basic},   {B7(K_DSPV6N79 + 5u), basic},
};

const agc_routine_table lm_aotmark_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_aotmark_basic = {basic_keys, COUNT(basic_keys)};

#endif
