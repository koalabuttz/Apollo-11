/* SPDX-License-Identifier: MIT */
/* The readable geometry (src/geometry/), LEM_GEOMETRY and CSM_GEOMETRY, as
 * routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_geometry.c and
 * cm_geometry.c): <m>_geometry_interpretive (the order pairs) and
 * <m>_geometry_basic (the basic code among them).
 *
 * Plumbing only. An order pair is one pass of the readable interpreter
 * over the listing's words (interpret_listing()); a pass that ends at an
 * RTB of one of the routines the faithful pair runs (CDULOGIC and
 * 2V1STO2S in the LM; CDULOGIC, SXTLOGIC, TRANSP1, TRANSP2 and 1STO2S in
 * the CM) runs that routine (src/rtb_op_codes/, src/geometry/) and the
 * DANZIG it ends at, as the faithful pair does. A basic key runs its code
 * to where the faithful routine stops: RRANGLES's and LOG's to the
 * interpreter (INTPRET) or POLY, SXTLOGIC's on through DANZIG unless an
 * order is left in EDOP.
 *
 * The states the faithful routines take: at an order pair Z at NEWOPS, FB
 * the pair's bank, BANKSET the BBANK and INTBIT15 the bank's bit 15; for
 * the basic code FB its bank. Always ZERO +0, the bank registers agreeing,
 * NEWJOB +0 or a core set, FIXLOC one of the VAC areas, the interrupt and
 * superbank latches in range and every erasable word an ordinary one. The
 * io record is the LM's p66_bias_io or the CM's cm_native_io (the
 * routines use their SUPERBNK and interrupt state). A run the readable
 * code refuses part way leaves the state as it was. */
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
#define ROPE 0u /* the LM's rope */
#else
#include "cm_geometry.h"
#define ROPE 1u /* the CM's */
#endif

/* Keys and physical addresses in banks 10, 13 and 23. */
#define B10(z)  (uint16_t)(010u * 1024u + ((z) - 02000u))
#define B13(z)  (uint16_t)(013u * 1024u + ((z) - 02000u))
#define B23(z)  (uint16_t)(023u * 1024u + ((z) - 02000u))
#define BANK(n) (word)((n) << 10)

#if !LUMINARY099
/* ENDT(X) -- CM p. 1387 (TIME_OF_FREE_FALL.agc, not yet readable), under
 * the name TCDANZIG: TC DANZIG. EARROT2's BOVB goes there only to reset
 * OVFIND; the pair goes on with its right order. */
static void tcdanzig(void)
{
    tc_at(TCDANZIG);
    continue_at(DANZIG);
}
#endif

/* The basic routines an order pair runs, at their addresses under their
 * banks (FB, from their FCADRs): RTB's, and the CM's TCDANZIG a BOVB
 * goes to. */
static const struct {
    word at, bank;
    void (*routine)(void);
} basic_routines[] = {
    {CDULOGIC, I_CDULOGIC & 076000u, cdulogic}, /* RTB_OP_CODES */
#if LUMINARY099
    {TWO_V1STO2S, I_2V1STO2S & 076000u, two_v1sto2s}, /* RTB_OP_CODES */
#else
    {SXTLOGIC, I_SXTLOGIC & 076000u, sxtlogic}, /* CSM_GEOMETRY */
    {TRANSP1, I_TRANSP1 & 076000u, transp1},    /* RT8_OP_CODES */
    {TRANSP2, I_TRANSP2 & 076000u, transp2},    /* RT8_OP_CODES */
    {ONE_STO2S, I_1STO2S & 076000u, one_sto2s}, /* RT8_OP_CODES */
    {TCDANZIG, I_TCDANZIG & 076000u, tcdanzig}, /* (BOVB) TIME_OF_FREE_FALL */
#endif
};

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* 1TO2SUB's INDEX A; CAF LIMITS and the CM's DCA CLOG2/32 read the rope. */
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

/* ---- The basic code ---- */

#if LUMINARY099
/* RRANGLES +042, after its EXIT, in bank 13. */
static bool run_basic(void)
{
    if (Z != RRANGLES + 042u || FBANK != BANK(013u)) return false;
    rrangles_after_exit();
    return true;
}
#else
/* SXTLOGIC (its own key too, besides the RTB), LOG +4 and LOG +016, in
 * bank 23. */
static bool run_basic(void)
{
    if (FBANK != BANK(023u)) return false;
    if (Z == SXTLOGIC) {
        /* (the faithful RTB routine takes Q an ordinary word too) */
        if (Q > 077777u) return false;
        sxtlogic();
        /* DANZIG: an order left in EDOP waits there for the interpreter */
        if (agc.memory[ADDRESS_OF(EDOP)] == 0u) danzig();
    } else if (Z == LOG + 4u) {
        log_after_exit();
    } else if (Z == LOG + 016u) {
        log_after_poly();
    } else {
        return false;
    }
    return true;
}
#endif

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    mission_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!sane() || !run_basic()) return EXEC_OUT_OF_SCOPE;
    return EXEC_OK;
}

/* ---- The order pairs ---- */

#if LUMINARY099
static const struct interpretive_code *const codes[] = {&SVDWN_CODE, &RRANGLES_CODE,
                                                        &RRANGLES_GOTO_CODE, &RRNB_CODE};
#else
static const struct interpretive_code *const codes[] = {&SXTNB_CODE,   &CALCSXA_CODE, &LOG_CODE,
                                                        &LOG_RVQ_CODE, &EARROT_CODE,  &SVDWN_CODE};
#endif

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

/* The basic routine at Z, under FB, or none. */
static void (*basic_routine_at(void))(void)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_routines); ++i)
        if (Z == basic_routines[i].at && FBANK == basic_routines[i].bank)
            return basic_routines[i].routine;
    return NULL;
}

/* One pass; after each RTB (a pair may hold two) or BOVB to TCDANZIG, its
 * routine and the DANZIG it ends at. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             mission_io *io, const struct interpretive_code *code)
{
    void (*routine)(void);
    unsigned rtbs;

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    for (rtbs = 0; rtbs < 2u; ++rtbs) {
        bind(program, state, io);
        if ((routine = basic_routine_at()) == NULL) return EXEC_OK;
        if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
        if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
        routine();
        if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    mission_io *io = device, kept;
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
#define K10(z) {B10(z), interpretive}
#define K13(z) {B13(z), interpretive}
#define K23(z) {B23(z), interpretive}

#if LUMINARY099
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

#else
static const agc_routine_entry pairs[] = {
    /* SVDWN1 */
    K10(K_SVDWN1),
    K10(K_SVDWN1 + 03u),
    K10(K_SVDWN1 + 06u),
    K10(K_SVDWN1 + 011u),
    K10(K_SVDWN1 + 013u),
    K10(K_SVDWN1 + 016u),
    K10(K_SVDWN1 + 020u),
    K10(K_SVDWN1 + 022u),
    K10(K_SVDWN1 + 023u),
    /* SVDWN2 */
    K10(K_SVDWN2),
    K10(K_SVDWN2 + 03u),
    K10(K_SVDWN2 + 06u),
    K10(K_SVDWN2 + 010u),
    K10(K_SVDWN2 + 013u),
    K10(K_SVDWN2 + 015u),
    K10(K_SVDWN2 + 016u),
    /* SXTNB */
    K23(K_SXTNB),
    K23(K_SXTNB + 03u),
    K23(K_SXTNB + 05u),
    K23(K_SXTNB + 06u),
    K23(K_SXTNB + 010u),
    K23(K_SXTNB + 012u),
    K23(K_SXTNB + 014u),
    K23(K_SXTNB + 015u),
    K23(K_SXTNB + 016u),
    K23(K_SXTNB + 020u),
    K23(K_SXTNB + 021u),
    K23(K_SXTNB + 023u),
    K23(K_SXTNB + 025u),
    K23(K_SXTNB + 026u),
    /* CALCSXA */
    K23(K_CALCSXA),
    K23(K_CALCSXA + 03u),
    K23(K_CALCSXA + 05u),
    K23(K_CALCSXA + 07u),
    K23(K_CALCSXA + 011u),
    K23(K_CALCSXA + 013u),
    K23(K_CALCSXA + 015u),
    /* SXTANG */
    K23(K_SXTANG),
    K23(K_SXTANG + 03u),
    K23(K_SXTANG + 06u),
    K23(K_SXTANG + 07u),
    K23(K_SXTANG + 011u),
    K23(K_SXTANG + 013u),
    K23(K_SXTANG + 015u),
    K23(K_SXTANG + 017u),
    K23(K_SXTANG + 020u),
    /* SXTANG1 */
    K23(K_SXTANG1),
    K23(K_SXTANG1 + 03u),
    K23(K_SXTANG1 + 05u),
    K23(K_SXTANG1 + 07u),
    K23(K_SXTANG1 + 010u),
    K23(K_SXTANG1 + 012u),
    K23(K_SXTANG1 + 014u),
    K23(K_SXTANG1 + 016u),
    K23(K_SXTANG1 + 020u),
    K23(K_SXTANG1 + 022u),
    K23(K_SXTANG1 + 024u),
    K23(K_SXTANG1 + 026u),
    K23(K_SXTANG1 + 030u),
    K23(K_SXTANG1 + 031u),
    K23(K_SXTANG1 + 033u),
    K23(K_SXTANG1 + 036u),
    K23(K_SXTANG1 + 040u),
    K23(K_SXTANG1 + 041u),
    /* SXTALARM */
    K23(K_SXTALARM),
    /* ZNB=S1 */
    K23(K_ZNB_EQ_S1),
    K23(K_ZNB_EQ_S1 + 02u),
    K23(K_ZNB_EQ_S1 + 04u),
    K23(K_ZNB_EQ_S1 + 05u),
    /* LOG */
    K23(K_LOG),
    K23(K_LOG + 03u),
    K23(K_LOG + 033u),
    /* EARROT1 */
    K23(K_EARROT1),
    K23(K_EARROT1 + 03u),
    K23(K_EARROT1 + 04u),
    K23(K_EARROT1 + 06u),
    /* EARROT2 */
    K23(K_EARROT2),
    K23(K_EARROT2 + 03u),
    K23(K_EARROT2 + 05u),
    K23(K_EARROT2 + 07u),
    K23(K_EARROT2 + 011u),
    K23(K_EARROT2 + 013u),
    K23(K_EARROT2 + 014u),
    K23(K_EARROT2 + 016u),
    K23(K_EARROT2 + 017u),
    /* OVERADAY */
    K23(K_OVERADAY),
    K23(K_OVERADAY + 03u),
    K23(K_OVERADAY + 05u),
    K23(K_OVERADAY + 06u),
};

#define KB23(z) {B23(z), basic}
static const agc_routine_entry basic_keys[] = {KB23(K_SXTLOGIC), KB23(K_LOG + 04u),
                                               KB23(K_LOG + 016u)};

const agc_routine_table cm_geometry_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_geometry_basic = {basic_keys, COUNT(basic_keys)};

#endif
