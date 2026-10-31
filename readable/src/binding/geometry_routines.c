/* SPDX-License-Identifier: MIT */
/* The readable LEM_GEOMETRY (src/geometry/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_geometry.c): lm_geometry_interpretive (the order pairs,
 * in banks 13 and 23) and lm_geometry_basic (RRANGLES's basic code).
 *
 * Plumbing only. An order pair is one pass of the readable interpreter
 * over the listing's words (interpret_listing()); a pass that ends at an
 * RTB of CDULOGIC or 2V1STO2S runs that routine (src/rtb_op_codes/) and
 * the DANZIG it ends at, as the faithful pair does. The basic routine runs
 * from its key to the interpreter's INTPRET.
 *
 * The states the faithful routines take: at an order pair Z at NEWOPS, FB
 * the pair's bank, BANKSET the BBANK and INTBIT15 the bank's bit 15; for
 * the basic code FB bank 13. Always ZERO +0, the bank registers agreeing,
 * NEWJOB +0 or a core set, FIXLOC one of the VAC areas, the interrupt and
 * superbank latches in range and every erasable word an ordinary one. The
 * io record is the LM's p66_bias_io (SUPERBNK and the interrupt state). A
 * run the readable code refuses part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../geometry/geometry.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099
#include "lm_geometry.h"

/* Keys and physical addresses in banks 13 and 23. */
#define B13(z)  (uint16_t)(013u * 1024u + ((z) - 02000u))
#define B23(z)  (uint16_t)(023u * 1024u + ((z) - 02000u))
#define BANK(n) (word)((n) << 10)

/* The RTB routines' bank (FB), from their FCADRs. */
#define CDULOGIC_BANK    ((word)(I_CDULOGIC & 076000u))
#define TWO_V1STO2S_BANK ((word)(I_2V1STO2S & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* 1TO2SUB's INDEX A; CAF LIMITS reads the rope. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, p66_bias_io *io)
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

/* ---- The basic code ---- */

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    p66_bias_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != RRANGLES + 042u || FBANK != BANK(013u) || !sane()) return EXEC_OUT_OF_SCOPE;
    rrangles_after_exit();
    return EXEC_OK;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {&SVDWN_CODE, &RRANGLES_CODE,
                                                        &RRANGLES_GOTO_CODE, &RRNB_CODE};

/* The table LOC is in, under FB, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (FBANK == BANK(codes[i]->bank) && LOC >= codes[i]->first &&
            LOC < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* One pass; after an RTB, its routine and the DANZIG it ends at. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             p66_bias_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z == CDULOGIC && FBANK == CDULOGIC_BANK) {
        if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
        if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
        cdulogic();
    } else if (Z == TWO_V1STO2S && FBANK == TWO_V1STO2S_BANK) {
        if (!sane()) return EXEC_OUT_OF_SCOPE;
        if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
        two_v1sto2s();
    } else {
        return EXEC_OK;
    }
    return interpret_listing(program, state, io, code);
}

static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    p66_bias_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at();
    if (Z != NEWOPS || !code || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) || !sane())
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
#define K13(z) {B13(z), interpretive}
#define K23(z) {B23(z), interpretive}
static const agc_routine_entry pairs[] = {
    /* SVDWN2 */
    K13(K_SVDWN2),
    K13(K_SVDWN2 + 03u),
    K13(K_SVDWN2 + 06u),
    K13(K_SVDWN2 + 011u),
    K13(K_SVDWN2 + 013u),
    K13(K_SVDWN2 + 016u),
    K13(K_SVDWN2 + 020u),
    K13(K_SVDWN2 + 022u),
    K13(K_SVDWN2 + 023u),
    /* SVDWN1 */
    K13(K_SVDWN1),
    K13(K_SVDWN1 + 03u),
    K13(K_SVDWN1 + 06u),
    K13(K_SVDWN1 + 010u),
    K13(K_SVDWN1 + 013u),
    K13(K_SVDWN1 + 015u),
    K13(K_SVDWN1 + 016u),
    /* RRANGLES */
    K13(K_RRANGLES),
    K13(K_RRANGLES + 01u),
    K13(K_RRANGLES + 03u),
    K13(K_RRANGLES + 05u),
    K13(K_RRANGLES + 07u),
    K13(K_RRANGLES + 011u),
    K13(K_RRANGLES + 013u),
    K13(K_RRANGLES + 015u),
    K13(K_RRANGLES + 017u),
    K13(K_RRANGLES + 021u),
    K13(K_RRANGLES + 023u),
    K13(K_RRANGLES + 024u),
    K13(K_RRANGLES + 026u),
    K13(K_RRANGLES + 030u),
    K13(K_RRANGLES + 032u),
    K13(K_RRANGLES + 034u),
    K13(K_RRANGLES + 036u),
    K13(K_RRANGLES + 040u),
    K13(K_RRANGLES + 041u),
    K13(K_RRANGLES + 052u),
    /* RRNB, RRNB1 and RRNBMPAC */
    K23(K_RRNB),
    K23(K_RRNB + 03u),
    K23(K_RRNB + 05u),
    K23(K_RRNB + 06u),
    K23(K_RRNB + 07u),
    K23(K_RRNB + 010u),
    K23(K_RRNB1),
    K23(K_RRNB1 + 01u),
    K23(K_RRNB1 + 03u),
    K23(K_RRNB1 + 04u),
    K23(K_RRNB1 + 05u),
    K23(K_RRNB1 + 06u),
    K23(K_RRNB1 + 010u),
    K23(K_RRNBMPAC),
    K23(K_RRNBMPAC + 02u),
    K23(K_RRNBMPAC + 05u),
    K23(K_RRNBMPAC + 06u),
    K23(K_RRNBMPAC + 07u),
    K23(K_RRNBMPAC + 010u),
    K23(K_RRNBMPAC + 011u),
    K23(K_RRNBMPAC + 014u),
};

static const agc_routine_entry basic_keys[] = {{B13(K_RRANGLES + 042u), basic}};

const agc_routine_table lm_geometry_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_geometry_basic = {basic_keys, COUNT(basic_keys)};

#endif
