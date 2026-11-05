/* SPDX-License-Identifier: MIT */
/* The readable landing analog displays (src/landing_displays/) as routines
 * of the LM's flight program, under the table name and keys of the
 * faithful port's (c64-port/src/lm_landing_displays.c):
 * lm_landing_displays_routines, and its site operand table,
 * lm_landing_displays_sites (TC TWIDDLE's ADRES INTLZE).
 *
 * Plumbing only: point the machine at the program's state, give it
 * channels 12, 14 and 30 from the program's hardware record, and take the
 * states the faithful routines take: in bank 21 under E-bank 7 (EBANK=
 * UNIT/R/), as R10's task runs. Each call runs one entry, as the faithful
 * port's do. Where the readable code calls not_modelled() (a step the
 * listing's flow never takes, or a channel the file does not use), the
 * call is refused and the state and channels are left as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "lm_landing_displays.h"
#include "../landing_displays/landing_displays.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK21 042000u

static agc_hardware *hardware_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channels 12 and 14 (WOR, WAND) and 30 (RAND); any other is not
 * modelled. */
static agc_word *latch(unsigned number)
{
    switch (number) {
    case CHAN12: return &hardware_now->channel12;
    case CHAN14: return &hardware_now->channel14;
    case CHAN30: return &hardware_now->channel30;
    default: not_modelled();
    }
    return NULL;
}

static word channel_in(unsigned number)
{
    return (word)*latch(number);
}

/* The file writes channels 12 and 14 only. */
static void channel_out(unsigned number, word value)
{
    if (number == CHAN30) not_modelled();
    *latch(number) = value;
}

static const struct {
    word at;
    void (*routine)(void);
} entries[] = {
    {LANDISP, landisp},
    {INTLZE - 1u, intlze_minus_one},
    {INTLZE, intlze},
    {SPEEDRUN + 0102u, speedrun_after_vardelay},
};

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    apollo_overlay_state before;
    agc_hardware channels;
    unsigned i;

    if (!state || !program || !program->hardware || !bind_machine(program, state, device))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.leave = leave;
    if ((FBANK & 076000u) != BANK21 || EBANK != 03400u) return EXEC_OUT_OF_SCOPE;
    for (i = 0; i < COUNT(entries); ++i) {
        if (entries[i].at != Z) continue;
        before = *state;
        channels = *hardware_now;
        if (setjmp(back)) {
            *state = before;
            *hardware_now = channels;
            return EXEC_OUT_OF_SCOPE;
        }
        entries[i].routine();
        return EXEC_OK;
    }
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 21, in the faithful table's order. */
#define B21(z) (uint16_t)(021u * 1024u + ((z) - 02000u))

static const agc_routine_entry routines[] = {
    {B21(K_LANDISP), run},
    {B21(K_INTLZE - 1u), run},
    {B21(K_INTLZE), run},
    {B21(K_SPEEDRUN + 0102u), run},
};
const agc_routine_table lm_landing_displays_routines = {routines, COUNT(routines)};

/* TC TWIDDLE (INTLZE -3) is followed by ADRES INTLZE: TWIDDLE reads it
 * through Q. As the faithful table has it: the physical word (bank 21,
 * INTLZE -2) and the word. */
const agc_site_operand lm_landing_displays_sites[LM_LAD_SITES] = {
    {(uint16_t)(021u * 1024u + (K_INTLZE - 2u - 02000u)), (agc_word)K_INTLZE},
};

#endif
