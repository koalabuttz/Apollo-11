/* SPDX-License-Identifier: MIT */
/* The readable jet failure monitor (src/rcs_failure_monitor/) as routines
 * of the LM's flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_rcsmon.c): lm_rcsmon_basic, with the
 * empty lm_rcsmon_interpretive beside it (the monitor has no interpretive
 * code).
 *
 * Plumbing only: point the machine at the program's state; give it
 * channel 32 as the run latched it (the device is the LM's io record with
 * the input channels after it, lm_channel_io) and the fixed words the
 * routine reads, BIT8 ... BIT1 and 5FAILTAB's and 6FAILTAB's eight words;
 * and take the states the faithful routines take: in bank 06, the bank
 * registers agreeing, ZERO +0, NEWJOB an offset or -0, the latches in
 * range and every erasable word an ordinary one. An index past a table
 * (L not 1 to 8, which the monitor's own PVALVEST never gives) is refused
 * as the faithful routine refuses it, leaving the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "../rcs_failure_monitor/rcs_failure_monitor.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK06 014000u

/* Keys and physical addresses in bank 06. */
#define B6(z) (uint16_t)(06u * 1024u + ((z) - 02000u))

static const agc_input_channels *inputs_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static word channel_in(unsigned number)
{
    if (number != CHAN32) not_modelled();
    return inputs_now->channel32;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled(); /* the monitor writes no channel */
}

/* BIT8, BIT7 ... BIT1 (fixed-fixed), and each table's eight words after
 * its name (bank 06). */
static word fixed_memory(unsigned physical)
{
    static const word five_failtab[8] = FIVE_FAILTAB_WORDS, six_failtab[8] = SIX_FAILTAB_WORDS;
    unsigned bit8 = FF(ADDRESS_OF(BIT8)), five = B6(FIVE_FAILTAB) + 1u, six = B6(SIX_FAILTAB) + 1u;

    if (physical >= bit8 && physical < bit8 + 8u) return (word)(BIT8 >> (physical - bit8));
    if (physical >= five && physical < five + 8u) return five_failtab[physical - five];
    if (physical >= six && physical < six + 8u) return six_failtab[physical - six];
    not_modelled();
    return 0;
}

/* ---- The states the faithful routines take ---- */

static bool as_expected(void)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (FBANK != BANK06 || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (newjob > 037777u && newjob != MINUS_ZERO) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The entries ---- */

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    lm_channel_io *io = device;
    apollo_overlay_state before;
    word entry;

    if (!state || !io || !bind_machine(program, state, &io->io)) return EXEC_OUT_OF_SCOPE;
    inputs_now = &io->in;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = fixed_memory;
    agc.leave = leave;
    entry = Z;
    if ((entry != RCSMON && entry != ONE_ACCFIX + 4u) || !as_expected()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    if (setjmp(back)) {
        *state = before;
        return EXEC_OUT_OF_SCOPE;
    }
    if (entry == RCSMON)
        rcsmon();
    else
        one_accfix_after_novac();
    return EXEC_OK;
}

static const agc_routine_entry routines[] = {{B6(K_RCSMON), run}, {B6(K_1_ACCFIX + 4u), run}};
const agc_routine_table lm_rcsmon_basic = {routines, COUNT(routines)};
const agc_routine_table lm_rcsmon_interpretive = {NULL, 0};

#endif
