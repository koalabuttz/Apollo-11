/* SPDX-License-Identifier: MIT */
/* The readable POWERED_FLIGHT_SUBROUTINES (src/powered_flight/) as
 * routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_powered_subs.c and
 * cm_powered_subs.c): <m>_powered_subs_basic and
 * <m>_powered_subs_interpretive. The keys come from the faithful tables,
 * in their order (powered_flight_keys.inc and cm_powered_flight_keys.inc,
 * which tools/p20_p25_keys.py writes from them).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file: at a TC or TCF to a routine outside it (USPRCADR, MPACVBUF,
 * SPSIN, SPCOS, the CM's DMPSUB, the LM's BANKCALL, VXV and SWRETURN),
 * into the interpreter (INTPRET), or back to a caller. A return to a
 * place in the file (CD*TR*GS's TC TEM2 to CDUTRIG +2, AX*SR*T's TC
 * RTNSAVER to C*MM*N2 +1) goes on there, as the faithful port does. An order pair is one pass of the readable interpreter over
 * the listing's words (interpret_listing()), in the part of the code LOC
 * is in. The words after the TC USPRCADRs (CADR CDULOGIC, COSINE and SINE
 * +1) and the LM's TCF VXV are order pairs too: reached at DANZIG with an
 * EXIT left in EDOP still to run, which goes back to the basic code after
 * them.
 *
 * The states the faithful routines take: FB the key's bank, ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range,
 * every erasable word an ordinary one (and in the CM, at a basic key,
 * the program's hardware record, which the file never uses); at an order
 * pair also NEWJOB +0 or a core set and FIXLOC one of the VAC areas, and
 * Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15 (after a
 * USPRCADR or VXV, Z at DANZIG and BANKSET the bank's, with EDOP 1). A
 * run the readable code refuses part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../powered_flight/powered_flight.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The banks of the file's code: POWFLITE, and the LM's FLESHLOC. */
#define POWFLITE_BANK CODE_BANK(I_CDUTRIG)
#if LUMINARY099
#define FLESHLOC_BANK CODE_BANK(I_CALCSMSC)
#endif

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The file reads and writes no channel. */
static word channel_in(unsigned number)
{
    (void)number;
    not_modelled();
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled();
}

/* The file's constants are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static bool bind(const agc_native_program *program, apollo_overlay_state *state, mission_io *io)
{
    if (!bind_machine(program, state, io)) return false;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
    return true;
}

/* The states the faithful routines take (their sane(), and basic_sane(),
 * which leaves out NEWJOB and the work area). */
static bool sane(bool work_area)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (work_area && (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u || FIXLOC < 0401u ||
                      FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

/* The places a run starts at or goes on at: the keys, and the returns
 * into the file. In bank 23, but for the LM's in bank 10. */
static const struct place {
    word at;
    void (*routine)(void);
} places[] = {
    {CDUTRIG + 1u, cdutrig_plus1},
    {CDUTRIG + 2u, cdutrig_plus2},
    {CD_STAR_TR_STAR_G + 1u, cd_star_tr_star_g_plus1},
    {CD_STAR_TR_STAR_G + 2u, cd_star_tr_star_g_plus2},
    {TR_STAR_GL_STAR_STAR_P + 010u, tr_star_gl_star_star_p_after_cdulogic},
    {TR_STAR_GL_STAR_STAR_P + 016u, tr_star_gl_star_star_p_after_cosine},
    {TR_STAR_GL_STAR_STAR_P + 026u, tr_star_gl_star_star_p_after_sine},
    {QUICTRIG, quictrig_to_spsin},
    {QUICTRIG + 011u, quictrig_after_spsin},
    {QUICTRIG + 020u, quictrig_after_spcos},
    {CDU_STAR_SMNB + 1u, cdu_star_smnb_plus1},
    {CDU_STAR_SMNB + 2u, cdu_star_smnb_plus2},
    {TRG_STAR_SMNB + 1u, trg_star_smnb_plus1},
    {C_STAR_MM_STAR_N1, c_star_mm_star_n1},
    {C_STAR_MM_STAR_N1 + 1u, c_star_mm_star_n1_plus1},
    {C_STAR_MM_STAR_N2 + 1u, c_star_mm_star_n2_plus1},
    {CDU_STAR_NBSM + 1u, cdu_star_nbsm_plus1},
    {CDU_STAR_NBSM + 2u, cdu_star_nbsm_plus2},
    {TRG_STAR_NBSM + 1u, trg_star_nbsm_plus1},
    {C_STAR_MM_STAR_N3, c_star_mm_star_n3},
    {C_STAR_MM_STAR_N3 + 1u, c_star_mm_star_n3_plus1},
    {STAR_SMNB_STAR + 1u, star_smnb_star_plus1},
    {STAR_NBSM_STAR + 1u, star_nbsm_star_plus1},
#if LUMINARY099
    {AX_STAR_SR_STAR_T, ax_star_sr_star_t},
#else
    {LOOP1 + 5u, loop1_after_sine_product},
    {LOOP1 + 023u, loop1_after_cosine_product},
#endif
};

#if LUMINARY099
static const struct place fleshloc_places[] = {
    {CALCSMSC + 1u, calcsmsc_plus1},        {CALCSMSC + 3u, calcsmsc_plus3},
    {FLESHPOT - 1u, fleshpot_minus1},       {FLESHPOT, fleshpot},
    {FLESHPOT + 0135u, fleshpot_after_vxv},
};
#endif

static const struct place *place_in(const struct place *table, unsigned count, word at)
{
    unsigned i;

    for (i = 0; i < count; ++i)
        if (table[i].at == at) return &table[i];
    return NULL;
}

static const struct place *place_at(unsigned bank, word at)
{
    if (bank == POWFLITE_BANK) return place_in(places, COUNT(places), at);
#if LUMINARY099
    if (bank == FLESHLOC_BANK) return place_in(fleshloc_places, COUNT(fleshloc_places), at);
#endif
    return NULL;
}

/* After a routine: on at the place in the file a return reached, as the
 * faithful port does; anywhere else the run stops. */
static void run_on(void)
{
    const struct place *place;

    while (Z >= 02000u && Z <= 03777u && (place = place_at(FBANK >> 10, Z)) != NULL)
        place->routine();
}

/* ---- The order pairs ---- */

/* The file's interpretive code, in its parts. */
static const struct interpretive_code *const codes[] = {
    &CDUTRIG_CODE,       &CDUTRIG_RVQ_CODE,    &CD_STAR_TR_STAR_G_CODE, &CD_STAR_TR_STAR_G_RVQ_CODE,
    &CDU_STAR_SMNB_CODE, &TRG_STAR_SMNB_CODE,  &C_STAR_MM_STAR_N2_CODE, &CDU_STAR_NBSM_CODE,
    &TRG_STAR_NBSM_CODE, &STAR_SMNB_STAR_CODE, &STAR_NBSM_STAR_CODE,
#if LUMINARY099
    &CALCSMSC_CODE,      &CALCSMSC_RVQ_CODE,
#else
    &NBSM_CODE,          &SMCDURES_CODE,
#endif
};

/* The part of the code an address of a bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* The words where an EXIT in EDOP comes back to basic code: after
 * TR*GL**P's TC USPRCADRs (CADR CDULOGIC, COSINE and SINE +1), and the
 * LM's TCF VXV. */
static bool exit_pending(unsigned bank, word at)
{
#if LUMINARY099
    if (bank == FLESHLOC_BANK && at == FLESHPOT + 0134u) return true;
#endif
    return bank == POWFLITE_BANK &&
           (at == TR_STAR_GL_STAR_STAR_P + 7u || at == TR_STAR_GL_STAR_STAR_P + 015u ||
            at == TR_STAR_GL_STAR_STAR_P + 025u);
}

/* ---- The keys, in the faithful tables' order ---- */

/* A basic routine, run, and on at the places it returns to; a run it
 * refuses part way leaves the state as it was. */
static enum exec_status run(void (*routine)(void), apollo_overlay_state *state, mission_io *io)
{
    mission_io kept = *io;
    apollo_overlay_state before = *state;

    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    run_on();
    return EXEC_OK;
}

/* Each basic key's routine. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct place *place = place_at(key / 1024u, (word)(02000u + key % 1024u));
    mission_io *io = device;

    if (!place || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
#if COMANCHE055
    if (!program->hardware) return EXEC_OUT_OF_SCOPE;
#endif
    if (FBANK != (word)((key / 1024u) << 10) || !sane(false)) return EXEC_OUT_OF_SCOPE;
    return run(place->routine, state, io);
}

/* Each order pair's routine: one pass of the readable interpreter. */
static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    word at = (word)(02000u + key % 1024u);
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, at);
    enum exec_status status;

    if ((!code && !exit_pending(bank, at)) || !program || !state || !io ||
        !bind(program, state, io))
        return EXEC_OUT_OF_SCOPE;
    if (code) {
        if (Z != NEWOPS || FBANK != (word)(bank << 10) || LOC != at || BANKSET != BBANK ||
            INTBIT15 != (FBANK & 040000u) || !sane(true))
            return EXEC_OUT_OF_SCOPE;
    } else if (Z != DANZIG || (BANKSET & 076000u) != (word)(bank << 10) || (BANKSET & 01770u) ||
               EDOP != 1u || !sane(true)) {
        return EXEC_OUT_OF_SCOPE;
    }

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool powered_flight_readable(uint16_t key, bool order_pair)
{
    unsigned bank = key / 1024u;
    word at = (word)(02000u + key % 1024u);

    if (order_pair) return code_at(bank, at) != NULL || exit_pending(bank, at);
    return place_at(bank, at) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(KEY(bank, address), program, state, device);                                   \
    }
#if LUMINARY099
#include "powered_flight_keys.inc"
#else
#include "cm_powered_flight_keys.inc"
#endif
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#if LUMINARY099
#include "powered_flight_keys.inc"
#else
#include "cm_powered_flight_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
#include "powered_flight_keys.inc"
#else
#include "cm_powered_flight_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#if LUMINARY099
const agc_routine_table lm_powered_subs_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_powered_subs_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_powered_subs_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_powered_subs_basic = {basic_keys, COUNT(basic_keys)};
#endif
