/* SPDX-License-Identifier: MIT */
/* The readable sextant marks (src/sxtmark/) as routines of the CM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_sxtmark.c): cm_sxtmark_basic, with the empty
 * cm_sxtmark_interpretive beside it (SXTMARK has no interpretive code).
 *
 * Plumbing only: point the machine at the program's state; give it
 * channel 12 from the mission's io record and channel 16 (NAVKEYIN, the
 * MARK buttons and the navigation DSKY's keys) as the run latched it (the
 * device is the io record with the input channels after it,
 * cm_channel_io); and take the states the faithful routines take: in the
 * routine's bank (07 or 10), the bank registers agreeing, ZERO +0, the
 * latches in range and every erasable word an ordinary one. A step the
 * readable code does not model leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "../sxtmark/sxtmark.h"
#include "../agc/names.h"

#if COMANCHE055

#define BANK07 016000u
#define BANK10 020000u

static cm_channel_io *io_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static word channel_in(unsigned number)
{
    if (number == NAVKEYIN) return io_now->in.channel16;
    if (number != CHAN12) not_modelled();
    return io_now->io.channel12;
}

/* Only channel 12 is written (WAND, in MKRLEES). */
static void channel_out(unsigned number, word value)
{
    if (number != CHAN12) not_modelled();
    io_now->io.channel12 = value;
}

/* The routines read no fixed word. */
static word fixed_memory(unsigned physical)
{
    (void)physical;
    not_modelled();
    return 0;
}

/* ---- The states the faithful routines take ---- */

static bool as_expected(word bank)
{
    unsigned i;

    if (FBANK != bank || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The entries ---- */

static const struct {
    word bank, at;
    void (*routine)(void);
} entries[] = {
    {BANK07, SXTMARK, sxtmark},
    {BANK07, SXTMARK + 2u, sxtmark_plus2},
    {BANK07, MARKOK, markok},
    {BANK07, MKVACFND, mkvacfnd},
    {BANK07, MKVACFND + 011u, mkvacfnd_after_checkmm53_no},
    {BANK07, MKVACFND + 012u, mkvacfnd_after_checkmm53_yes},
    {BANK07, MKVACFND + 015u, mkvacfnd_after_checkmm54_no},
    {BANK07, MKVACFND + 016u, mkvacfnd_after_checkmm54_yes},
    {BANK07, MKVACFND + 030u, mkvacfnd_after_novac},
    {BANK07, MKRELEAS, mkreleas},
    {BANK07, MKRLEES, mkrlees},
    {BANK07, MARKRUPT, markrupt},
    {BANK07, KEYCALL + 011u, keycall_after_alarm},
    {BANK07, MARKCONT, markcont},
    {BANK07, MARKIT1 + 4u, markit1_after_gentran_mrkbuf2},
    {BANK07, MARKIT1 + 010u, markit1_after_gentran_mrkbuf1},
    {BANK07, MARKET + 016u, market_after_novac},
    {BANK07, MARKET + 022u, market_after_gentran_markdown},
    {BANK07, MARKET + 026u, market_after_gentran_mrkbuf1},
    {BANK07, MARKET3 + 2u, market3_after_alarm},
    {BANK07, HUNDRED14_ALM + 2u, hundred14_alm_after_alarm},
    {BANK07, VACSTOR + 034u, vacstor_after_novac},
    {BANK07, MRKREJCT + 4u, mrkrejct_after_alarm},
    {BANK07, REJECT2 + 010u, reject2_after_alarm},
    {BANK07, REJECT3 + 020u, reject3_after_novac},
    {BANK07, MKVB51, mkvb51},
    {BANK07, MKVB51 + 2u, mkvb51_after_kleenex},
    {BANK07, MKVB51 + 5u, mkvb51_after_terminate},
    {BANK07, MKVB51 + 6u, mkvb51_after_proceed},
    {BANK07, MKVB51 + 7u, mkvb51_after_enter},
    {BANK07, TERMSXT + 1u, termsxt_after_clearmrk},
    {BANK07, TERMSXT + 3u, termsxt_after_checkmm03_no},
    {BANK07, TERMSXT + 4u, termsxt_after_checkmm03_yes},
    {BANK07, ENTANSWR, entanswr},
    {BANK07, JAMIT + 7u, jamit_after_waitlist},
    {BANK07, ENDMARKS, endmarks},
    {BANK07, MKVB5X, mkvb5x},
    {BANK07, MKVB50, mkvb50},
    {BANK07, MARKIT + 010u, markit_after_waitlist},
    {BANK10, MARKDIF, markdif},
    {BANK10, DIFCHK, difchk},
    {BANK10, DIFCHK + 016u, difchk_after_alarm},
};

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    apollo_overlay_state before;
    cm_channel_io io_before;
    word entry;
    unsigned i;

    if (!state || !device || !bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    io_now = device;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = fixed_memory;
    agc.leave = leave;
    entry = Z;
    for (i = 0; i < COUNT(entries); ++i) {
        if (entries[i].at != entry || entries[i].bank != FBANK) continue;
        if (!as_expected(entries[i].bank)) return EXEC_OUT_OF_SCOPE;
        before = *state;
        io_before = *io_now;
        if (setjmp(back)) {
            *state = before;
            *io_now = io_before;
            return EXEC_OUT_OF_SCOPE;
        }
        entries[i].routine();
        return EXEC_OK;
    }
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 07 and bank 10, in the faithful table's order. */
#define B7(z)  (uint16_t)(07u * 1024u + ((z) - 02000u))
#define B10(z) (uint16_t)(010u * 1024u + ((z) - 02000u))

static const agc_routine_entry routines[] = {
    {B7(K_SXTMARK), run},         {B7(K_SXTMARK + 2u), run},    {B7(K_MARKOK), run},
    {B7(K_MKVACFND), run},        {B7(K_MKVACFND + 011u), run}, {B7(K_MKVACFND + 012u), run},
    {B7(K_MKVACFND + 015u), run}, {B7(K_MKVACFND + 016u), run}, {B7(K_MKVACFND + 030u), run},
    {B7(K_MKRELEAS), run},        {B7(K_MKRLEES), run},         {B7(K_MARKRUPT), run},
    {B7(K_KEYCALL + 011u), run},  {B7(K_MARKCONT), run},        {B7(K_MARKIT1 + 4u), run},
    {B7(K_MARKIT1 + 010u), run},  {B7(K_MARKET + 016u), run},   {B7(K_MARKET + 022u), run},
    {B7(K_MARKET + 026u), run},   {B7(K_MARKET3 + 2u), run},    {B7(K_114ALM + 2u), run},
    {B7(K_VACSTOR + 034u), run},  {B7(K_MRKREJCT + 4u), run},   {B7(K_REJECT2 + 010u), run},
    {B7(K_REJECT3 + 020u), run},  {B7(K_MKVB51), run},          {B7(K_MKVB51 + 2u), run},
    {B7(K_MKVB51 + 5u), run},     {B7(K_MKVB51 + 6u), run},     {B7(K_MKVB51 + 7u), run},
    {B7(K_TERMSXT + 1u), run},    {B7(K_TERMSXT + 3u), run},    {B7(K_TERMSXT + 4u), run},
    {B7(K_ENTANSWR), run},        {B7(K_JAMIT + 7u), run},      {B7(K_ENDMARKS), run},
    {B7(K_MKVB5X), run},          {B7(K_MKVB50), run},          {B7(K_MARKIT + 010u), run},
    {B10(K_MARKDIF), run},        {B10(K_DIFCHK), run},         {B10(K_DIFCHK + 016u), run},
};
const agc_routine_table cm_sxtmark_basic = {routines, COUNT(routines)};
const agc_routine_table cm_sxtmark_interpretive = {NULL, 0};

#endif
