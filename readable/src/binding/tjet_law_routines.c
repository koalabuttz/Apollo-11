/* SPDX-License-Identifier: MIT */
/* The readable TJET_LAW (src/tjet_law/), the LM's, as a routine of its
 * flight program, under the table name and key of the faithful port's
 * (c64-port/src/lm_tjet_law.c): lm_tjet_law_routines, its one key,
 * TJETLAW, in bank 17.
 *
 * Plumbing only: point the machine at the program's state (TJETLAW reads
 * no channel and no io record, so a call without one gets an empty
 * record), give it the rope's words the file reads by address (AXISDIFF -1
 * to +1, and ZON1,2,3's word, which a TS FIREFCT that overflows adds), and
 * take the states the faithful routine takes: FB bank 17, EBANK 6, a
 * return in Q in fixed memory and AXISCTR -1, +0 or +1. A run that stops
 * at not_modelled() leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include <string.h>
#include "binding.h"
#include "rope_image.h"
#include "lm_tjet_law.h"
#include "../tjet_law/tjet_law.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK17          017u
#define EBANK6_REGISTER 03000u
#define MINUS_ONE_WORD  077776u

#define B17(z) (uint16_t)(BANK17 * 1024u + ((z) - 02000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The rope's words at AXISDIFF -1 to +1 and at ZON1,2,3. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if ((physical + 1u < B17(AXISDIFF) || physical > B17(AXISDIFF) + 1u) &&
        physical != B17(ZON1_2_3))
        not_modelled();
    if (!apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    mission_io none;
    apollo_overlay_state before;
    word axisctr;

    if (!device) {
        memset(&none, 0, sizeof none);
        device = &none;
    }
    if (!state || !program || !bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    axisctr = fetch(ADDRESS_OF(AXISCTR));
    if ((FBANK & 076000u) != (word)(BANK17 << 10) || EBANK != EBANK6_REGISTER || Q < 02000u ||
        Q > 07777u || (axisctr > 1u && axisctr != MINUS_ONE_WORD))
        return EXEC_OUT_OF_SCOPE;
    agc.fixed_memory = rope_word;
    agc.leave = leave;

    before = *state;
    if (setjmp(back)) {
        *state = before;
        return EXEC_OUT_OF_SCOPE;
    }
    tjetlaw();
    return EXEC_OK;
}

static const agc_routine_entry keys[] = {{B17(TJETLAW), run}};

const agc_routine_table lm_tjet_law_routines = {keys, COUNT(keys)};

#endif
