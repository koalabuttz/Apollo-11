/* SPDX-License-Identifier: MIT */
/* The readable ATTITUDE_MANEUVER_ROUTINE (src/attitude_maneuver/) as
 * routines of the LM's flight program, under the table names and keys of
 * the faithful port's (c64-port/src/lm_attitude_maneuver.c):
 * lm_attitude_maneuver_basic (the basic code) and
 * lm_attitude_maneuver_interpretive (the order pairs), all in bank 22.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (ALARM, BANKCALL, WAITLIST, ENDOFJOB, TLOAD +6) or reaches
 * NEWOPS through a TC INTPRET. An order pair is one pass of the readable
 * interpreter over the listing's words (interpret_listing()); a pass that
 * ends at RTB CDULOGIC (LOOPSIN's) runs CDULOGIC (src/rtb_op_codes/) and
 * the DANZIG it ends at, as the faithful pair does. A pass that ends at
 * RTB READCDUK, at an EXIT, or at a CALL out of the file stops there, as
 * the faithful pair does.
 *
 * The states the faithful routines take: FB bank 22; at an order pair Z at
 * NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. Always ZERO
 * +0, the bank registers agreeing, NEWJOB +0 or a core set, FIXLOC one of
 * the VAC areas, the interrupt and superbank latches in range and every
 * erasable word an ordinary one. A run the readable code refuses part way
 * leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../attitude_maneuver/attitude_maneuver.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* Keys and physical addresses in bank 22. */
#define BANK22 044000u
#define B22(z) (uint16_t)(022u * 1024u + ((z) - 02000u))

/* CDULOGIC's bank (FB), from its FCADR. */
#define CDULOGIC_BANK ((word)(I_CDULOGIC & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands (MINANG, MAXANG, LOCKANGL, DPHALF and
 * the rest) are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
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

static const struct {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {KALCMAN3, kalcman3},
    {TRNSPSPD + 1u, trnspspd_after_exit},
    {READCDUK, readcduk},
    {TOOBADF + 1u, toobadf_after_exit},
    {TOOBADF + 3u, toobadf_after_alarm},
    {NOGO - 2u, nogo_minus2},
    {NOGO, nogo},
    {NOGO + 2u, nogo_after_stoprate},
    {NOGO + 7u, nogo_after_waitlist},
    {TOOBADI + 1u, toobadi_after_exit},
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
    if (i == COUNT(basic_entries) || FBANK != BANK22 || !sane()) return EXEC_OUT_OF_SCOPE;

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

/* The table LOC is in, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < ATTITUDE_MANEUVER_CODE_COUNT; ++i) {
        const struct interpretive_code *code = ATTITUDE_MANEUVER_CODES[i];

        if (LOC >= code->first && LOC < code->first + code->count) return code;
    }
    return NULL;
}

/* One pass; after RTB CDULOGIC, CDULOGIC and its DANZIG. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != CDULOGIC || FBANK != CDULOGIC_BANK) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    cdulogic();
    return interpret_listing(program, state, io, code);
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
    if (Z != NEWOPS || FBANK != BANK22 || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane())
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

/* Each where an order pair starts that the faithful port enters, by the
 * table it is in. */
#define K22(z) {B22(z), interpretive}
static const agc_routine_entry pairs[] = {
    /* KALCMAN3 */
    K22(02005),
    K22(02007),
    K22(02010),
    K22(02012),
    K22(02015),
    K22(02020),
    K22(02022),
    K22(02025),
    /* SECAD */
    K22(02027),
    K22(02032),
    K22(02034),
    K22(02036),
    K22(02040),
    K22(02043),
    K22(02045),
    K22(02047),
    K22(02051),
    K22(02053),
    K22(02056),
    K22(02060),
    K22(02062),
    K22(02064),
    K22(02067),
    K22(02072),
    K22(02075),
    K22(02076),
    K22(02077),
    K22(02102),
    K22(02105),
    K22(02106),
    K22(02107),
    K22(02110),
    K22(02113),
    K22(02115),
    /* CHECKMAX */
    K22(02117),
    K22(02122),
    K22(02125),
    K22(02126),
    K22(02127),
    /* ALTCALC */
    K22(02131),
    K22(02134),
    K22(02135),
    K22(02137),
    K22(02141),
    K22(02143),
    K22(02145),
    K22(02146),
    K22(02150),
    K22(02153),
    K22(02156),
    K22(02161),
    K22(02163),
    K22(02166),
    K22(02170),
    K22(02173),
    K22(02174),
    K22(02175),
    /* COFMAXGO */
    K22(02176),
    K22(02201),
    K22(02204),
    K22(02207),
    K22(02211),
    K22(02214),
    /* METHOD2 */
    K22(02216),
    K22(02221),
    K22(02223),
    K22(02224),
    K22(02227),
    K22(02231),
    K22(02232),
    K22(02235),
    K22(02237),
    K22(02240),
    /* METHOD1 */
    K22(02242),
    K22(02245),
    K22(02247),
    K22(02250),
    K22(02253),
    K22(02255),
    K22(02256),
    K22(02261),
    K22(02263),
    K22(02264),
    /* METHOD3 */
    K22(02266),
    K22(02271),
    K22(02273),
    K22(02274),
    K22(02277),
    K22(02301),
    K22(02302),
    K22(02305),
    K22(02307),
    K22(02310),
    /* MXM3 */
    K22(02312),
    K22(02315),
    K22(02320),
    K22(02323),
    K22(02325),
    /* TRANSPOS */
    K22(02326),
    K22(02331),
    K22(02334),
    K22(02335),
    /* TRNSPSPD +024 */
    K22(02361),
    /* CDUTODCM */
    K22(02410),
    K22(02414),
    K22(02415),
    K22(02417),
    K22(02422),
    K22(02423),
    K22(02425),
    K22(02426),
    K22(02431),
    K22(02433),
    K22(02434),
    K22(02437),
    K22(02442),
    K22(02444),
    K22(02446),
    K22(02447),
    K22(02452),
    K22(02455),
    K22(02457),
    K22(02461),
    K22(02462),
    K22(02464),
    K22(02465),
    K22(02467),
    K22(02471),
    K22(02472),
    K22(02475),
    K22(02476),
    K22(02477),
    K22(02502),
    K22(02503),
    K22(02504),
    K22(02505),
    K22(02507),
    K22(02512),
    K22(02513),
    K22(02515),
    K22(02516),
    K22(02520),
    K22(02523),
    K22(02524),
    K22(02526),
    /* DELCOMP */
    K22(02527),
    K22(02531),
    K22(02532),
    K22(02533),
    K22(02534),
    K22(02537),
    K22(02541),
    K22(02543),
    K22(02545),
    K22(02547),
    K22(02551),
    K22(02553),
    K22(02555),
    K22(02557),
    K22(02561),
    K22(02563),
    K22(02565),
    K22(02567),
    K22(02570),
    K22(02573),
    K22(02575),
    K22(02600),
    K22(02602),
    K22(02604),
    K22(02605),
    K22(02606),
    K22(02610),
    K22(02612),
    K22(02615),
    K22(02617),
    K22(02621),
    K22(02623),
    K22(02625),
    K22(02626),
    K22(02627),
    K22(02631),
    K22(02633),
    K22(02636),
    K22(02640),
    K22(02642),
    K22(02644),
    K22(02646),
    K22(02647),
    K22(02650),
    K22(02652),
    K22(02653),
    /* DCMTOCDU */
    K22(02654),
    K22(02656),
    K22(02657),
    K22(02661),
    K22(02662),
    K22(02664),
    K22(02666),
    K22(02671),
    K22(02672),
    K22(02675),
    K22(02677),
    K22(02701),
    K22(02702),
    K22(02704),
    K22(02706),
    K22(02711),
    K22(02712),
    K22(02715),
    K22(02717),
    K22(02722),
    K22(02723),
    /* TOOBADF */
    K22(02724),
    /* TOOBADI */
    K22(02742),
};

#define B(z) {B22(z), basic}
static const agc_routine_entry basic_keys[] = {
    B(K_KALCMAN3),  B(K_TRNSPSPD + 1u), B(K_READCDUK),  B(K_TOOBADF + 1u), B(K_TOOBADF + 3u),
    B(K_NOGO - 2u), B(K_NOGO),          B(K_NOGO + 2u), B(K_NOGO + 7u),    B(K_TOOBADI + 1u),
};

const agc_routine_table lm_attitude_maneuver_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_attitude_maneuver_basic = {basic_keys, COUNT(basic_keys)};

#endif
