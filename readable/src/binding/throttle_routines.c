/* SPDX-License-Identifier: MIT */
/* The readable throttle (src/throttle/) and FASTCHNG
 * (src/landing_guidance/) as routines of the LM's flight program, under
 * the table name and keys of the faithful port's (c64-port/src/
 * lm_throttle.c): lm_throttle_routines.
 *
 * Plumbing only: point the machine at the program's state, give it
 * channel 14 from the program's hardware record and the rope's fixed
 * words (MASSMULT's DMPs read their ADRES and SCALEFAC there), and take
 * the states the faithful routines take: in bank 31 under E-bank 7, as
 * every caller runs them. Each call runs one entry, as the faithful port's
 * do. Where the readable code calls not_modelled() (a return word that is
 * not a TC to fixed memory), the call is refused and the state and
 * channel 14 are left as they were. The faithful table takes FASTCHNG's
 * return (Q) only as a TC to fixed memory, 02000 to 07777, and so does
 * this one (FASTCHNG itself, as LUNAR_LANDING_GUIDANCE_EQUATIONS' table
 * runs it, takes any TC). */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../throttle/throttle.h"
#include "../landing_guidance/landing_guidance.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK31 062000u

static agc_hardware *hardware_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channel 14 (DOIT's WOR); any other is not modelled. */
static agc_word *latch(unsigned number)
{
    if (number != CHAN14) not_modelled();
    return &hardware_now->channel14;
}

static word channel_in(unsigned number)
{
    return (word)*latch(number);
}

static void channel_out(unsigned number, word value)
{
    *latch(number) = value;
}

static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static const struct {
    word at;
    void (*routine)(void);
} entries[] = {
    {THROTTLE, throttle},
    {THROTTLE + 3u, throttle_plus3},
    {AFDUMP + 1u, afdump_after_massmult},
    {AFDUMP + 5u, afdump_after_second_massmult},
    {DOPIF + 1u, dopif_after_fastchng},
    {FLATOUT, flatout},
    {WHATOUT, whatout},
    {MASSMULT, massmult},
    {FASTCHNG, fastchng},
};

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    apollo_overlay_state before;
    agc_word channel14;
    unsigned i;

    if (!state || !program || !program->hardware || !bind_machine(program, state, device))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
    if ((FBANK & 076000u) != BANK31 || EBANK != 03400u) return EXEC_OUT_OF_SCOPE;
    if (Z == FASTCHNG && (Q < 02000u || Q > 07777u)) return EXEC_OUT_OF_SCOPE;
    for (i = 0; i < COUNT(entries); ++i) {
        if (entries[i].at != Z) continue;
        before = *state;
        channel14 = hardware_now->channel14;
        if (setjmp(back)) {
            *state = before;
            hardware_now->channel14 = channel14;
            return EXEC_OUT_OF_SCOPE;
        }
        entries[i].routine();
        return EXEC_OK;
    }
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 31, in the faithful table's order. */
#define B31(z) (uint16_t)(031u * 1024u + ((z) - 02000u))

static const agc_routine_entry routines[] = {
    {B31(K_THROTTLE), run},    {B31(K_THROTTLE + 3u), run}, {B31(K_AFDUMP + 1u), run},
    {B31(K_AFDUMP + 5u), run}, {B31(K_DOPIF + 1u), run},    {B31(K_FLATOUT), run},
    {B31(K_WHATOUT), run},     {B31(K_MASSMULT), run},      {B31(K_FASTCHNG), run}};
const agc_routine_table lm_throttle_routines = {routines, COUNT(routines)};

#endif
