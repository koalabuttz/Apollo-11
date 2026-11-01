/* SPDX-License-Identifier: MIT */
/* The readable IMU performance test (src/imu_performance/) as routines of
 * the LM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_imu_performance.c): lm_imu_performance_routines
 * (the basic keys), lm_imu_performance_interpretive (every order-pair and
 * store word) and ipt_routine_keys.
 *
 * Plumbing only. A routine runs the file's code from its key until control
 * leaves the file: one basic function after another (each leaves Z where
 * the listing goes next), and at NEWOPS with LOC in the file one pass of
 * the readable interpreter over the listing's words (interpret_listing(),
 * one order pair or store word, as the faithful port steps them). Control
 * leaves at a basic address of another file (BANKCALL, TWIDDLE, CHANG1,
 * ATIGINC ...), an RTB's routine, NEWOPS with LOC outside the file (CALL
 * CALCGA, an RVQ to a caller elsewhere), or CHANG2. It also ends where the
 * AGC would wait for the hardware: CHECKG1, the PIPA not yet moved, and
 * FINETIME +1, the scaler at 37777 (the faithful port's idle points).
 *
 * Built with AGC_FLIGHT_INTERPRETER, as the faithful port's flight program
 * builds its file, a run stops instead where it reaches NEWOPS: there the
 * flight program's interpreter takes the file's interpretive code, and its
 * EXITs come back to seven more basic keys.
 *
 * The states the faithful routines take, which this checks at each step
 * (a run refused part way leaves the state and the io record as they
 * were, from the caller's undo image):
 *   - Z one of the file's words a function starts at;
 *   - at an order pair: FB and BANKSET the same bank, EDOP +0, FIXLOC one
 *     of the five VAC areas and PUSHLOC in it; X1 and X2 within +-0377 at
 *     an indexed (starred) order or a STORE,1 or STORE,2; at SIN or COS
 *     double precision (MODE +0) and no 16-bit word but A and L; VXSC
 *     GEORGEJ with a vector in MPAC; ALSK +1,1 on the constants after
 *     DEC585; an RVQ with its return (QPRET) in fixed memory; and AINGOTN's
 *     DDV with a divisor that has a major part;
 *   - at most 4000 steps.
 * Two differences, outside any state the program reaches: the readable
 * interpreter takes no pass with NEWJOB negative, where the faithful one
 * refuses it only at DANZIG; and a return word or INDEX NDXCTR that lands
 * inside another routine's code is refused here, where the faithful port
 * runs any instruction word it lands on.
 * The io record is lm_ipt_io: the scaler words FINETIME reads (channels 3
 * and 4), and the undo image. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "flight_undo.h"
#include "lm_imu_performance.h"
#include "rope_image.h"
#include "../imu_performance/imu_performance.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK37    037u
#define RUN_LIMIT 4000u
#define B37(z)    (uint16_t)(BANK37 * 1024u + ((z) - 02000u))

static const lm_ipt_io *io_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The scaler: LOSCALAR and HISCALAR as the run found them. */
static word channel_in(unsigned number)
{
    if (number == LOSCALAR) return io_now->loscalar;
    if (number == HISCALAR) return io_now->hiscalar;
    not_modelled();
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled(); /* the test writes no channel */
}

static word rope_word(unsigned physical)
{
    uint16_t value;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_ipt_io *io)
{
    bind_machine(program, state, &io->control);
    io_now = io;
    agc.fixed_memory = rope_word;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.leave = leave;
}

static uint16_t physical(word address, word bank_register)
{
    return agc_fixed_physical(address, bank_register, *agc.superbank);
}

/* ---- The basic code ---- */

/* The words a function starts at: the keys, the returns from other files
 * and from the interpreter's EXITs, the words this file's own TCs and
 * returns go to, and those INDEX NDXCTR; TC +1 lands on. */
static const struct {
    word at;
    void (*routine)(void);
} points[] = {
    {REDO, redo},
    {GEOIMUTT, geoimutt},
    {IMUBACK, imuback},
    {NBPOSPL + 4u, nbpospl_after_zeroing},
    {LATAZCHK + 6u, latazchk_after_exit},
    {LATAZCHK + 010u, latazchk_display},
    {LATAZCHK + 013u, latazchk_after_terminate},
    {LATAZCHK + 014u, latazchk_after_proceed},
    {LATAZCHK + 015u, latazchk_after_enter},
    {LATAZCHK + 016u, latazchk_accepted},
    {POSGMBL + 3u, posgmbl_after_exit},
    {POSGMBL + 5u, posgmbl_after_imucoars},
    {POSGMBL + 014u, posgmbl_after_downflag},
    {POSGMBL + 015u, posgmbl_after_imuslllg},
    {POSGMBL + 020u, posgmbl_after_finimudd},
    {POSGMBL + 025u, posgmbl_after_longcall},
    {GOESTIMS, goestims},
    {GOESTIMS + 2u, goestims_after_jobwake},
    {TORQUE, torque},
    {PIPACHK, pipachk},
    {PIPACHK + 2u, pipachk_landing},
    {PIPACHK + 3u, pipachk_landing},
    {PIPACHK + 4u, pipachk_landing},
    {PIPACHK + 5u, pipachk_landing},
    {PIPACHK + 6u, pipachk_landing},
    {PIPACHK + 7u, pipachk_landing},
    {PIPACHK + 010u, pipachk_landing},
    {PIPACHK + 011u, pipachk_landing},
    {PIPACHK + 012u, pipachk_landing},
    {PIPACHK + 014u, pipachk_landing},
    {PIPACHK + 015u, pipachk_landing},
    {PIPACHK + 016u, pipachk_after_checkg},
    {PIPACHK + 022u, pipachk_after_twiddle},
    {PIPATASK, pipatask},
    {STARTPIP, startpip},
    {STARTPIP + 4u, startpip_after_findvac},
    {PIPJOBB, pipjobb},
    {PIPJOBB + 2u, pipjobb_landing},
    {PIPJOBB + 3u, pipjobb_landing},
    {PIPJOBB + 4u, pipjobb_landing},
    {PIPJOBB + 6u, pipjobb_landing},
    {PIPJOBB + 7u, pipjobb_landing},
    {PIPJOBB + 010u, pipjobb_landing},
    {PIPJOBB + 011u, pipjobb_landing},
    {PIPJOBB + 012u, pipjobb_after_checkg},
    {AINGOTN + 7u, aingotn_after_exit},
    {AINGOTN + 011u, aingotn_show},
    {VERTDRFT, vertdrft},
    {PON + 1u, pon_after_earthr_star},
    {VALMIS, valmis},
    {ENDTEST1, endtest1},
    {ENDTEST1 + 2u, endtest1_after_downflag},
    {ENDTEST1 + 4u, endtest1_after_newmodea},
    {COAALIGN, coaalign},
    {ALIGNCOA, aligncoa},
    {ALIGNCOA + 2u, aligncoa_after_bad_stall},
    {ALIGNCOA + 3u, aligncoa_after_stall},
    {IMUSLLLG, imuslllg},
    {FINIMUDD, finimudd},
    {FINIMUDD + 4u, finimudd_after_imufine},
    {IMUZERR, imuzerr},
    {IMUZERR + 4u, imuzerr_after_imuzero},
    {CHECKG, checkg},
    {CHECKG1, checkg1},
    {CHECKG1 + 5u, checkg_after_chang1},
    {CHECKG1 + 017u, checkg_caught},
    {CHECKG1 + 024u, checkg_after_finetime},
    {ZEROING, zeroing},
    {EARTHR_STAR, earthr_star},
    {EARTHR_STAR + 6u, earthr_star_after_exit},
    {EARTHR_STAR + 7u, earthr_star_after_imuslllg},
    {SHOW, show},
    {SHOW1, show1},
    {SHOW + 7u, show_after_terminate},
    {SHOW + 010u, show_after_proceed},
    {SHOW + 011u, show_after_enter},
    {FINETIME, finetime},
    {FINETIME + 1u, finetime_plus1},
    {FINETIME + 010u, finetime_reread},
    {ESTIMS, estims},
    {ESTIMS + 4u, estims_after_twiddle},
    {ESTIMS + 015u, estims_after_zeroing},
    {VERTSKIP + 1u, vertskip_after_exit},
    {ALLOOP, alloop},
    {ALLOOP + 023u, alloop_after_twiddle},
    {SPECSTS + 4u, specsts_after_findvac},
    {ALFLT, alflt},
    {NORMLOP, normlop},
    {PERFERAS + 1u, perferas_after_exit},
    {PERFERAS + 4u, perferas_after_atiginc},
    {SETUPER1 + 014u, setuper1_after_exit},
    {SETUPER1 + 017u, setuper1_after_imupulse},
    {GEOSTRT4, geostrt4},
    {GEOSTRT4 + 6u, geostrt4_after_exit},
    {SLEEPIE, sleepie},
    {SLEEPIE + 1u, sleepie_plus1},
    {SLEEPIE + 3u, sleepie_after_earthr_star},
    {SOMEERRR + 6u, someerrr_after_alarm},
    {SOMERR2, somerr2},
    {SOMERR2 + 2u, somerr2_after_varalarm},
    {SOMERR2 + 4u, somerr2_after_downflag},
};

/* Whether Z is in the file's basic code: bank 37 from REDO to SOMERR2's
 * end, under FB, or FINETIME's words in fixed-fixed. */
static bool in_file_basic(void)
{
    if (Z >= FINETIME && Z <= FINETIME + 021u) return true;
    return Z >= 02000u && Z < 04000u && physical(Z, FBANK) == B37(Z) && Z >= REDO &&
           Z <= SOMERR2 + 4u;
}

static void (*basic_at(void))(void)
{
    unsigned i;

    for (i = 0; i < COUNT(points); ++i)
        if (points[i].at == Z) return points[i].routine;
    return NULL;
}

/* ---- The interpretive code ---- */

/* Whether LOC, under BANKSET, is in the file's interpretive code (LATAZCHK
 * to GEOSTRT4's EXIT, with the basic words between). */
static bool in_file_code(void)
{
    return LOC >= 02000u && LOC < 04000u && physical(LOC, BANKSET) == B37(LOC) && LOC >= LATAZCHK &&
           LOC <= GEOSTRT4 + 5u;
}

static const agc_routine_entry pair_keys[IPT_INTERPRETIVE_ROUTINES];

/* The table whose order pair or store word LOC is, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i, k;

    for (i = 0; i < COUNT(pair_keys); ++i) {
        if (pair_keys[i].physical != B37(LOC)) continue;
        for (k = 0; k < IMU_PERFORMANCE_CODE_COUNT; ++k) {
            const struct interpretive_code *code = IMU_PERFORMANCE_CODES[k];
            if (LOC >= code->first && LOC < code->first + code->count) return code;
        }
    }
    return NULL;
}

static bool work_area(void)
{
    unsigned area;

    if (EDOP != 0u) return false;
    for (area = 0401u; area <= 0661u; area += 054u)
        if (FIXLOC == area) return PUSHLOC >= area && PUSHLOC <= area + 042u;
    return false;
}

/* A word of the job's work area (X1, X2, QPRET). */
static word work_word(unsigned offset)
{
    return agc.memory[(FIXLOC + offset) & 03777u];
}

/* X1 and X2 within +-0377, so that an indexed address stays the word it
 * names. */
static bool indexes_in_range(void)
{
    word x1 = work_word(K_X1), x2 = work_word(K_X1 + 1u);

    return (x1 < 0400u || x1 > 077377u) && (x2 < 0400u || x2 > 077377u);
}

/* Whether an order of a pair word is starred (indexed): the order's code,
 * kept in the word as code + 1, the left in bits 1-7 and the right in
 * bits 8-14 (0: no right order), with its low two bits set. */
static bool starred(word pair, unsigned shift)
{
    unsigned field = ((~(unsigned)pair & 077777u) >> shift) & 0177u;

    return field != 0u && ((field - 1u) & 3u) == 3u;
}

/* Whether the word is indexed: a starred order, or a STORE,1 or STORE,2
 * code. */
static bool indexed_word(word w)
{
    unsigned index;

    if (!(w & 040000u)) {
        index = (w >> 11) & 7u;
        return index == 1u || index == 2u;
    }
    return starred(w, 0u) || starred(w, 7u);
}

/* The words the faithful port's SIN and COS take: every word but A and L
 * one of 15 bits. (Q can hold 16: INTPRET's QXCH LOC sign-extends a LOC
 * with its top bit set.) */
static bool fifteen_bit_words(void)
{
    unsigned i;

    for (i = Q_ADDRESS; i < 04000u; ++i)
        if (agc.memory[i] > MINUS_ZERO) return false;
    return true;
}

/* The checks a pair's own words call for, before it runs. */
static bool pair_takes(word w)
{
    word x1;

    /* (DELMLP +7, PDDL DMP*, has its indexes checked after the pass: the
     * push may reach them before DMP* uses them.) */
    if (indexed_word(w) && LOC != DELMLP + 7u && !indexes_in_range()) return false;
    switch (LOC) {
    /* SIN or COS of MPAC as the pair finds it: the trigonometric orders
     * are modelled for double precision. */
    case LATAZCHK + 026u:
    case LATAZCHK + 032u:
    case LATAZCHK + 035u:
    case LATAZCHK + 040u:
    case ERTHRVSE + 3u:
    case BOOP + 4u:
    case BOOP + 011u: return MODE == 0u && fifteen_bit_words();
    /* PDDL SIN LATITUDE: the load makes MPAC double precision. */
    case ERTHRVSE + 4u: return fifteen_bit_words();
    /* VCOMP VXSC GEORGEJ: a vector in MPAC (a scalar would multiply the
     * vector GEORGEJ is not). */
    case SETUPER1 + 6u: return MODE >= 040000u;
    /* DLOAD* ALSK +1,1: one of the gains after DEC585. */
    case ALKLP + 021u:
        x1 = work_word(K_X1);
        x1 = add((word)(K_ALSK + 1u), negate(x1));
        return x1 >= K_DEC585 && x1 <= K_GEORGEK;
    /* RVQ: back to a caller in fixed memory. */
    case OVERFFIX + 3u:
    case ERTHRVSE + 016u: return (work_word(K_QPRET) & 074000u) != 0u;
    default: return true;
    }
}

/* AINGOTN's DDV divided by the pushdown word: one with a major part once
 * its two words agree in sign. */
static bool divisor_has_major_part(void)
{
    word hi = agc.memory[PUSHLOC & 03777u], lo = agc.memory[(PUSHLOC + 1u) & 03777u];
    bool negative = lo >= 040000u && lo != MINUS_ZERO, positive = lo < 040000u && lo != 0u;

    return !(hi == 0u || hi == MINUS_ZERO || (hi == 1u && negative) || (hi == 077776u && positive));
}

/* ---- The run ---- */

enum step { GO_ON, LEFT, IDLE, REFUSED };

static enum step step(const agc_native_program *program, apollo_overlay_state *state, lm_ipt_io *io)
{
    const struct interpretive_code *code;
    void (*routine)(void);
    word at = LOC;

    if (Z == NEWOPS) {
        if (!in_file_code()) return LEFT;
#ifdef AGC_FLIGHT_INTERPRETER
        return LEFT; /* the flight program's interpreter goes on */
#endif
        if (((FBANK ^ BANKSET) & 076000u) || !work_area()) return REFUSED;
        code = code_at();
        if (!code || !pair_takes(code->words[LOC - code->first])) return REFUSED;
        if (interpret_listing(program, state, &io->control, code) != EXEC_OK) return REFUSED;
        bind(program, state, io); /* (the pass pointed the machine at its own hooks) */
        if (at == AINGOTN && !divisor_has_major_part()) return REFUSED;
        if (at == DELMLP + 7u && !indexes_in_range()) return REFUSED;
        return GO_ON;
    }
    if (!in_file_basic()) return LEFT;
    routine = basic_at();
    if (!routine) return REFUSED; /* a word no function starts at */
    routine();
    if ((Z == CHECKG1 && physical(Z, FBANK) == B37(CHECKG1)) || Z == FINETIME + 1u) return IDLE;
    return GO_ON;
}

/* (A separate function, so that nothing setjmp returns through is a
 * local of run().) */
static enum step steps(const agc_native_program *program, apollo_overlay_state *state,
                       lm_ipt_io *io)
{
    unsigned n;

    if (setjmp(back)) return REFUSED;
    for (n = 0; n < RUN_LIMIT; ++n) {
        switch (step(program, state, io)) {
        case GO_ON: continue;
        case LEFT: return n > 0u ? LEFT : REFUSED;
        case IDLE: return IDLE;
        case REFUSED: return REFUSED;
        }
    }
    return REFUSED;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static const agc_native_program none;
    lm_ipt_io *io = device, entry;

    if (!state || !io || !io->undo || FLIGHT_UNDO_ALIAS(io, state)) return EXEC_OUT_OF_SCOPE;
    if (!program) program = &none;
    FLIGHT_UNDO_SAVE(io, state);
    entry = *io;
    bind(program, state, io);
    if (steps(program, state, io) != REFUSED) return EXEC_OK;
    FLIGHT_UNDO_RESTORE(&entry, state);
    *io = entry;
    return EXEC_OUT_OF_SCOPE;
}

/* ---- The keys, in the faithful tables' order ---- */

/* FINETIME's two (fixed-fixed sorts first), then bank 37's; F marks the
 * EXIT returns only the flight build keys. */
#define BASIC_KEYS(K, F, FFK)                                                                      \
    FFK(K_FINETIME + 1u)                                                                           \
    FFK(K_FINETIME + 010u)                                                                         \
    K(K_REDO) K(K_GEOIMUTT) K(K_LATAZCHK + 6u) K(K_LATAZCHK + 013u) K(K_LATAZCHK + 014u)           \
        K(K_LATAZCHK + 015u) F(K_POSGMBL + 3u) K(K_POSGMBL + 5u) K(K_POSGMBL + 014u)               \
            K(K_POSGMBL + 025u) K(K_GOESTIMS) K(K_GOESTIMS + 2u) K(K_PIPACHK + 022u) K(K_PIPATASK) \
                K(K_STARTPIP) K(K_STARTPIP + 4u) K(K_PIPJOBB) F(K_AINGOTN + 7u) K(K_ENDTEST1 + 2u) \
                    K(K_ENDTEST1 + 4u) K(K_ALIGNCOA) K(K_ALIGNCOA + 2u) K(K_ALIGNCOA + 3u)         \
                        K(K_FINIMUDD + 4u) K(K_IMUZERR + 4u) K(K_CHECKG1) K(K_CHECKG1 + 5u)        \
                            K(K_CHECKG1 + 017u) F(K_EARTHR_STAR + 6u) K(K_SHOW + 7u)               \
                                K(K_SHOW + 010u) K(K_SHOW + 011u) K(K_ESTIMS) K(K_ESTIMS + 4u)     \
                                    F(K_VERTSKIP + 1u) K(K_ALLOOP) K(K_ALLOOP + 023u)              \
                                        K(K_SPECSTS + 4u) K(K_ALFLT) K(K_NORMLOP)                  \
                                            F(K_PERFERAS + 1u) K(K_PERFERAS + 4u)                  \
                                                F(K_SETUPER1 + 014u) K(K_SETUPER1 + 017u)          \
                                                    F(K_GEOSTRT4 + 6u) K(K_SOMEERRR + 6u)          \
                                                        K(K_SOMERR2 + 2u) K(K_SOMERR2 + 4u)

/* Every order-pair and store word: each can follow a DANZIG, where a job
 * of higher priority may take the computer (CHANG2), and the Executive
 * resumes this one at NEWOPS with LOC there. */
#define PAIR_KEYS(X)                                                                               \
    /* LATAZCHK and POSGMBL */                                                                     \
    X(K_LATAZCHK)                                                                                  \
    X(K_LATAZCHK + 02u)                                                                            \
    X(K_LATAZCHK + 04u)                                                                            \
    X(K_LATAZCHK + 017u)                                                                           \
    X(K_LATAZCHK + 022u)                                                                           \
    X(K_LATAZCHK + 023u)                                                                           \
    X(K_LATAZCHK + 025u)                                                                           \
    X(K_LATAZCHK + 026u)                                                                           \
    X(K_LATAZCHK + 027u)                                                                           \
    X(K_LATAZCHK + 030u)                                                                           \
    X(K_LATAZCHK + 032u)                                                                           \
    X(K_LATAZCHK + 033u)                                                                           \
    X(K_LATAZCHK + 035u)                                                                           \
    X(K_LATAZCHK + 036u)                                                                           \
    X(K_LATAZCHK + 037u)                                                                           \
    X(K_LATAZCHK + 040u)                                                                           \
    X(K_LATAZCHK + 041u)                                                                           \
    X(K_LATAZCHK + 042u)                                                                           \
    X(K_POSGMBL)                                                                                   \
    X(K_POSGMBL + 2u)                                                                              \
    /* AINGOTN and OVERFFIX */                                                                     \
    X(K_AINGOTN - 6u)                                                                              \
    X(K_AINGOTN - 3u)                                                                              \
    X(K_AINGOTN)                                                                                   \
    X(K_AINGOTN + 2u)                                                                              \
    X(K_AINGOTN + 5u)                                                                              \
    X(K_AINGOTN + 6u)                                                                              \
    X(K_OVERFFIX)                                                                                  \
    X(K_OVERFFIX + 3u)                                                                             \
    /* ERTHRVSE, EARTHR and ERTHR */                                                               \
    X(K_ERTHRVSE)                                                                                  \
    X(K_ERTHRVSE + 3u)                                                                             \
    X(K_ERTHRVSE + 4u)                                                                             \
    X(K_ERTHRVSE + 6u)                                                                             \
    X(K_ERTHRVSE + 010u)                                                                           \
    X(K_ERTHRVSE + 011u)                                                                           \
    X(K_ERTHRVSE + 013u)                                                                           \
    X(K_ERTHRVSE + 015u)                                                                           \
    X(K_ERTHRVSE + 016u)                                                                           \
    X(K_EARTHR)                                                                                    \
    X(K_EARTHR + 3u)                                                                               \
    X(K_EARTHR + 4u)                                                                               \
    X(K_EARTHR + 7u)                                                                               \
    X(K_ERTHR)                                                                                     \
    X(K_ERTHR + 3u)                                                                                \
    X(K_ERTHR + 6u)                                                                                \
    X(K_ERTHR + 010u)                                                                              \
    X(K_ERTHR + 011u)                                                                              \
    X(K_ERTHR + 014u)                                                                              \
    /* EARTHR* */                                                                                  \
    X(K_EARTHR_STAR + 3u)                                                                          \
    X(K_EARTHR_STAR + 5u)                                                                          \
    /* ESTIMS */                                                                                   \
    X(K_ESTIMS + 016u)                                                                             \
    X(K_ESTIMS + 020u)                                                                             \
    X(K_ESTIMS + 022u)                                                                             \
    X(K_ESTIMS + 024u)                                                                             \
    X(K_ESTIMS + 025u)                                                                             \
    X(K_ESTIMS + 026u)                                                                             \
    X(K_ESTIMS + 030u)                                                                             \
    X(K_ESTIMS + 032u)                                                                             \
    X(K_VERTSKIP)                                                                                  \
    /* NORMLOP, from DLOAD INTVAL to BOOP's loop and PERFERAS */                                   \
    X(K_NORMLOP + 1u)                                                                              \
    X(K_NORMLOP + 3u)                                                                              \
    X(K_NORMLOP + 5u)                                                                              \
    X(K_NORMLOP + 7u)                                                                              \
    X(K_NORMLOP + 011u)                                                                            \
    X(K_NORMLOP + 013u)                                                                            \
    X(K_NORMLOP + 014u)                                                                            \
    X(K_NORMLOP + 017u)                                                                            \
    X(K_NORMLOP + 021u)                                                                            \
    X(K_ALCGKK)                                                                                    \
    X(K_ALKCG)                                                                                     \
    X(K_ALKCG2)                                                                                    \
    X(K_ALKCG2 + 3u)                                                                               \
    X(K_ALKCG2 + 4u)                                                                               \
    X(K_ALFLT3)                                                                                    \
    X(K_DELMLP)                                                                                    \
    X(K_DELMLP + 3u)                                                                               \
    X(K_DELMLP + 6u)                                                                               \
    X(K_DELMLP + 7u)                                                                               \
    X(K_DELMLP + 012u)                                                                             \
    X(K_DELMLP + 013u)                                                                             \
    X(K_DELMLP + 015u)                                                                             \
    X(K_DELMLP + 016u)                                                                             \
    X(K_ALILP)                                                                                     \
    X(K_ALILP + 3u)                                                                                \
    X(K_ALILP + 4u)                                                                                \
    X(K_ALKLP)                                                                                     \
    X(K_ALKLP + 3u)                                                                                \
    X(K_ALKLP + 6u)                                                                                \
    X(K_ALKLP + 010u)                                                                              \
    X(K_ALKLP + 011u)                                                                              \
    X(K_ALKLP + 014u)                                                                              \
    X(K_ALKLP + 015u)                                                                              \
    X(K_ALKLP + 020u)                                                                              \
    X(K_ALKLP + 021u)                                                                              \
    X(K_ALKLP + 024u)                                                                              \
    X(K_ALKLP + 026u)                                                                              \
    X(K_ALKLP + 027u)                                                                              \
    X(K_LOOSE)                                                                                     \
    X(K_LOOSE + 3u)                                                                                \
    X(K_LOOSE + 5u)                                                                                \
    X(K_LOOSE + 7u)                                                                                \
    X(K_LOOSE + 011u)                                                                              \
    X(K_LOOSE + 012u)                                                                              \
    X(K_LOOSE + 014u)                                                                              \
    X(K_LOOSE + 015u)                                                                              \
    X(K_LOOSE + 017u)                                                                              \
    X(K_LOOSE + 020u)                                                                              \
    X(K_LOOSE + 022u)                                                                              \
    X(K_BOOP)                                                                                      \
    X(K_BOOP + 3u)                                                                                 \
    X(K_BOOP + 4u)                                                                                 \
    X(K_BOOP + 5u)                                                                                 \
    X(K_BOOP + 7u)                                                                                 \
    X(K_BOOP + 010u)                                                                               \
    X(K_BOOP + 011u)                                                                               \
    X(K_BOOP + 012u)                                                                               \
    X(K_BOOP + 013u)                                                                               \
    X(K_PERFERAS)                                                                                  \
    /* SETUPER1 and GEOSTRT4 */                                                                    \
    X(K_SETUPER1 + 1u)                                                                             \
    X(K_SETUPER1 + 4u)                                                                             \
    X(K_SETUPER1 + 6u)                                                                             \
    X(K_SETUPER1 + 010u)                                                                           \
    X(K_SETUPER1 + 012u)                                                                           \
    X(K_SETUPER1 + 013u)                                                                           \
    X(K_GEOSTRT4 + 3u)                                                                             \
    X(K_GEOSTRT4 + 5u)

#define ENTRY(z)    {B37(z), run},
#define FF_ENTRY(z) {FF(z), run},
#ifdef AGC_FLIGHT_INTERPRETER
#define FLIGHT_ENTRY(z) ENTRY(z)
#else
#define FLIGHT_ENTRY(z)
#endif
#define NONE(z)
#define PHYSICAL(z)    B37(z),
#define FF_PHYSICAL(z) FF(z),

static const agc_routine_entry basic_keys[] = {BASIC_KEYS(ENTRY, FLIGHT_ENTRY, FF_ENTRY)};
static const agc_routine_entry pair_keys[IPT_INTERPRETIVE_ROUTINES] = {PAIR_KEYS(ENTRY)};

const agc_routine_table lm_imu_performance_routines = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_imu_performance_interpretive = {pair_keys, COUNT(pair_keys)};

/* The host keys, basic then interpretive (the port's cases run each). */
const uint16_t ipt_routine_keys[IPT_ROUTINES] = {BASIC_KEYS(PHYSICAL, NONE, FF_PHYSICAL)
                                                     PAIR_KEYS(PHYSICAL)};

#endif
