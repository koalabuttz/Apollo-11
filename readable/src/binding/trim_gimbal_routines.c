/* SPDX-License-Identifier: MIT */
/* The readable TRIM_GIMBAL_CONTROL_SYSTEM (src/trim_gimbal/), the LM's,
 * as routines of its flight program, under the table name and keys of the
 * faithful port's (c64-port/src/lm_trim_gimbal.c): lm_trim_gimbal_routines,
 * its four keys, ACDT+C12 in bank 16 and GTS, TIMEGMBL and DONEYET +6 (where
 * TIMEGMBL's IBNKCALL returns) in bank 21.
 *
 * Plumbing only: point the machine at the program's state, give it channel
 * 12 from the program's hardware record, and the rope's words the file
 * reads by address (bank 21 and fixed-fixed: LIMITS, BIT15 on, GMBLBITA's
 * neighbours, and the word DOSHIFT's INDEX SHFTFLAG makes its CA read), and
 * take the states the faithful routines take: FB bank 21 (bank 16 for
 * ACDT+C12), EBANK 6, the io record present; for ACDT+C12 a return in Q in
 * fixed memory, for DONEYET +6 one in RUPTREG2. A run that stops at
 * not_modelled() leaves the state, the io record and the hardware record as
 * they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "lm_trim_gimbal.h"
#include "../trim_gimbal/trim_gimbal.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK16          016u
#define BANK21          021u
#define EBANK6_REGISTER 03000u

#define B16(z) (uint16_t)(BANK16 * 1024u + ((z) - 02000u))
#define B21(z) (uint16_t)(BANK21 * 1024u + ((z) - 02000u))

static jmp_buf back;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

static word channel_in(unsigned number)
{
    if (number != CHAN12) not_modelled();
    return hardware_now->channel12;
}

static void channel_out(unsigned number, word value)
{
    if (number != CHAN12) not_modelled();
    hardware_now->channel12 = value;
}

/* The rope's words in bank 21 and fixed-fixed (banks 2 and 3). */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if ((physical < BANK21 * 02000u || physical >= (BANK21 + 1u) * 02000u) &&
        (physical < 2u * 02000u || physical >= 4u * 02000u))
        not_modelled();
    if (!apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* A TC's word into fixed memory. */
static bool return_word(word w)
{
    return w >= 02000u && w <= 07777u;
}

/* What a key asks of the state beyond its bank and EBANK. */
enum entry { ANY, RETURN_IN_Q, RETURN_IN_RUPTREG2 };

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, unsigned bank, enum entry entry, void (*routine)(void))
{
    mission_io *io = device, kept_io;
    agc_hardware kept_hardware;
    apollo_overlay_state before;

    if (!state || !program || !io || !program->hardware || !bind_machine(program, state, io))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    if ((FBANK & 076000u) != (word)(bank << 10) || EBANK != EBANK6_REGISTER)
        return EXEC_OUT_OF_SCOPE;
    if (entry == RETURN_IN_Q && !return_word(Q)) return EXEC_OUT_OF_SCOPE;
    if (entry == RETURN_IN_RUPTREG2 && !return_word(fetch(ADDRESS_OF(RUPTREG2))))
        return EXEC_OUT_OF_SCOPE;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;

    before = *state;
    kept_io = *io;
    kept_hardware = *hardware_now;
    if (setjmp(back)) {
        *state = before;
        *io = kept_io;
        *hardware_now = kept_hardware;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

#define KEY(name, bank, entry)                                                                     \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, bank, entry, name);                                     \
    }
KEY(acdt_c12, BANK16, RETURN_IN_Q)
KEY(gts, BANK21, ANY)
KEY(timegmbl, BANK21, ANY)
KEY(timegmbl_return, BANK21, RETURN_IN_RUPTREG2)

static const agc_routine_entry keys[] = {
    {B16(ACDTPC12), key_acdt_c12},
    {B21(GTS), key_gts},
    {B21(TIMEGMBL), key_timegmbl},
    {B21(DONEYET + 6u), key_timegmbl_return},
};

const agc_routine_table lm_trim_gimbal_routines = {keys, COUNT(keys)};

#endif
