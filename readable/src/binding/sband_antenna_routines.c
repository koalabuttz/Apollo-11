/* SPDX-License-Identifier: MIT */
/* The readable S-band antenna routine, R05 (src/sband_antenna/), as
 * routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_sband_antenna.c and
 * cm_sband_antenna.c): <m>_sband_antenna_basic (the basic code) and
 * <m>_sband_antenna_interpretive (the order pairs), all in bank 42: FB 32
 * with the superbank bit.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, PRIOCHNG, BLANKET, B5OFF, ENDOFJOB, ENDEXT) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()); a
 * pass that ends at RTB LOADTIME runs LOADTIME (src/rtb_op_codes/) and
 * the DANZIG it ends at, as the faithful pair does. A pass that ends at an
 * EXIT or a CALL out of the file stops there, as the faithful pair does.
 *
 * The device is the mission's io record (mission_io): the routine reads
 * and writes no channel.
 *
 * The states the faithful routines take, at every key: FB 32 with the
 * superbank bit (channel 7's 0100) set; at an order pair Z at NEWOPS,
 * BANKSET the BBANK and INTBIT15 the bank's bit 15. Always ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range,
 * every erasable word an ordinary one, NEWJOB +0 or a core set and FIXLOC
 * one of the VAC areas. A run the readable code refuses part way leaves
 * the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../sband_antenna/sband_antenna.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif

/* Bank 42 is FB 32 with the superbank bit; a word's key in it. */
#define FB_32         064000u
#define SUPERBANK_BIT 0100u
#define B42(z)        (uint16_t)(042u * 1024u + ((z) - 02000u))

/* LOADTIME's bank (FB), from its FCADR. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, mission_io *io)
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

/* Bank 42 selected (its sb_ok() and FB check). */
static bool in_bank_42(void)
{
    return FBANK == FB_32 && (*agc.superbank & SUPERBANK_BIT);
}

/* ---- The basic code ---- */

static const struct {
    word at;
    void (*routine)(void);
} basic_entries[] = {
#if LUMINARY099
    {SBANDANT, sbandant},
    {SBANDANT + 2u, sbandant_after_r02both},
    {SBANDEX + 1u, sbandant_after_exit},
    {SBANDEX + 7u, sbandant_after_prio5},
    {SBANDEX + 012u, sbandant_terminate},
    {SBANDEX + 013u, sbandant_proceed},
    {SBANDEX + 014u, sbandant_recycle},
    {SBANDEX + 015u, sbandant_immediate_return},
    {SBANDEX + 017u, sbandant_after_blanket},
    {SBANDEX + 021u, sbandant_after_prio4},
#else
    {SBANDANT, sbandant},
    {SBANDANT + 2u, sbandant_after_r02both},
    {SBANDANT + 077u, sbandant_after_exit},
    {SBANDANT + 0106u, sbandant_terminate},
    {SBANDANT + 0107u, sbandant_proceed},
    {SBANDANT + 0110u, sbandant_recycle},
    {SBANDANT + 0111u, sbandant_immediate_return},
    {SBANDANT + 0113u, sbandant_after_blanket},
    {SBANDANT + 0116u, sbandant_after_delayjob},
#endif
};

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    unsigned i;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == Z) break;
    if (i == COUNT(basic_entries) || !in_bank_42() || !sane()) return EXEC_OUT_OF_SCOPE;

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

/* One pass; after RTB LOADTIME, LOADTIME and its DANZIG. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             mission_io *io)
{
    if (interpret_listing(program, state, io, &SBAND_CODE) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, io, &SBAND_CODE);
}

static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != NEWOPS || !in_bank_42() || LOC < SBAND_CODE.first ||
        LOC >= SBAND_CODE.first + SBAND_CODE.count || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = pair(program, state, io);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* ---- The keys, in the faithful tables' order ---- */

#define PAIR(z) {B42(z), interpretive}
#define B(z)    {B42(z), basic}

#if LUMINARY099

/* Order pairs, by their offset from SBANDANT. */
static const agc_routine_entry pairs[] = {
    PAIR(K_SBANDANT + 003u),  PAIR(K_SBANDANT + 006u),  PAIR(K_SBANDANT + 010u),
    PAIR(K_SBANDANT + 013u),  PAIR(K_SBANDANT + 015u),  PAIR(K_SBANDANT + 017u),
    PAIR(K_SBANDANT + 021u),  PAIR(K_SBANDANT + 024u),  PAIR(K_SBANDANT + 026u),
    PAIR(K_SBANDANT + 030u),  PAIR(K_SBANDANT + 032u),  PAIR(K_SBANDANT + 034u),
    PAIR(K_SBANDANT + 036u),  PAIR(K_SBANDANT + 040u),  PAIR(K_SBANDANT + 042u),
    PAIR(K_SBANDANT + 043u),  PAIR(K_SBANDANT + 044u),  PAIR(K_SBANDANT + 046u),
    PAIR(K_SBANDANT + 050u),  PAIR(K_SBANDANT + 052u),  PAIR(K_SBANDANT + 054u),
    PAIR(K_SBANDANT + 055u),  PAIR(K_SBANDANT + 060u),  PAIR(K_SBANDANT + 062u),
    PAIR(K_SBANDANT + 064u),  PAIR(K_SBANDANT + 066u),  PAIR(K_SBANDANT + 071u),
    PAIR(K_SBANDANT + 073u),  PAIR(K_SBANDANT + 075u),  PAIR(K_SBANDANT + 076u),
    PAIR(K_SBANDANT + 077u),  PAIR(K_SBANDANT + 0102u), PAIR(K_SBANDANT + 0103u),
    PAIR(K_SBANDANT + 0104u), PAIR(K_SBANDANT + 0106u), PAIR(K_SBANDANT + 0111u),
    PAIR(K_SBANDANT + 0114u), PAIR(K_SBANDANT + 0115u), PAIR(K_SBANDANT + 0120u),
    PAIR(K_SBANDANT + 0121u), PAIR(K_SBANDANT + 0123u), PAIR(K_SBANDANT + 0125u),
    PAIR(K_SBANDANT + 0127u), PAIR(K_SBANDANT + 0131u), PAIR(K_SBANDANT + 0134u),
    PAIR(K_SBANDANT + 0135u), PAIR(K_SBANDANT + 0136u), PAIR(K_SBANDEX),
};

static const agc_routine_entry basic_keys[] = {
    B(K_SBANDANT),       B(K_SBANDANT + 2u),  B(K_SBANDEX + 1u),   B(K_SBANDEX + 7u),
    B(K_SBANDEX + 012u), B(K_SBANDEX + 013u), B(K_SBANDEX + 014u), B(K_SBANDEX + 015u),
    B(K_SBANDEX + 017u), B(K_SBANDEX + 021u),
};

const agc_routine_table lm_sband_antenna_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_sband_antenna_basic = {basic_keys, COUNT(basic_keys)};

#else

/* Order pairs, by their offset from SBANDANT. */
static const agc_routine_entry pairs[] = {
    PAIR(K_SBANDANT + 003u), PAIR(K_SBANDANT + 006u), PAIR(K_SBANDANT + 010u),
    PAIR(K_SBANDANT + 013u), PAIR(K_SBANDANT + 015u), PAIR(K_SBANDANT + 016u),
    PAIR(K_SBANDANT + 021u), PAIR(K_SBANDANT + 023u), PAIR(K_SBANDANT + 025u),
    PAIR(K_SBANDANT + 027u), PAIR(K_SBANDANT + 032u), PAIR(K_SBANDANT + 034u),
    PAIR(K_SBANDANT + 036u), PAIR(K_SBANDANT + 040u), PAIR(K_SBANDANT + 041u),
    PAIR(K_SBANDANT + 043u), PAIR(K_SBANDANT + 045u), PAIR(K_SBANDANT + 050u),
    PAIR(K_SBANDANT + 052u), PAIR(K_SBANDANT + 054u), PAIR(K_SBANDANT + 055u),
    PAIR(K_SBANDANT + 060u), PAIR(K_SBANDANT + 062u), PAIR(K_SBANDANT + 064u),
    PAIR(K_SBANDANT + 065u), PAIR(K_SBANDANT + 070u), PAIR(K_SBANDANT + 071u),
    PAIR(K_SBANDANT + 073u), PAIR(K_SBANDANT + 075u), PAIR(K_SBANDANT + 076u),
};

static const agc_routine_entry basic_keys[] = {
    B(K_SBANDANT),         B(K_SBANDANT + 2u),    B(K_SBANDANT + 077u),
    B(K_SBANDANT + 0106u), B(K_SBANDANT + 0107u), B(K_SBANDANT + 0110u),
    B(K_SBANDANT + 0111u), B(K_SBANDANT + 0113u), B(K_SBANDANT + 0116u),
};

const agc_routine_table cm_sband_antenna_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_sband_antenna_basic = {basic_keys, COUNT(basic_keys)};

#endif
