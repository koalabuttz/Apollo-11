/* SPDX-License-Identifier: MIT */
/* The readable P-AXIS_RCS_AUTOPILOT (src/p_axis/), the LM's, as routines of
 * its flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_p_axis.c): lm_p_axis_routines, its 28 keys in
 * bank 16; lm_p_axis_hot_routines, the 14 one DAP pass reaches; and
 * lm_p_axis_tg_hot_routines, PAXFILT +4 (where ACDT+C12 returns). The
 * faithful object's other tables (lm_p_axis_sites, lm_p_axis_data) stay
 * the faithful port's.
 *
 * Plumbing only: point the machine at the program's state, give it
 * channels 12, 13 and 31 from the program's hardware record (channel 13
 * keeps only the bits that read back), the rope's words at the fixed
 * addresses the file reads by INDEX (JETSALL with TYPEP, INDXYZ, MINTIMES,
 * and BIT15 and POSMAX, either side of BIT15 -1), and take the states the
 * faithful routines take: FB bank 16, EBANK 6; for CALCPERR a return in Q
 * in fixed memory; for the keys of the rate loop and the P axis no
 * OLDPMIN -0 (its CCS's last exit enters an instruction's second word),
 * and for PJETSLEC +25 a ROTINDEX of +0 or 1 (MINTIMES's two words). A run
 * that stops at not_modelled() leaves the state, the io record and the
 * hardware record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "lm_p_axis.h"
#include "../p_axis/p_axis.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK16          016u
#define EBANK6_REGISTER 03000u
#define MINUS_ZERO_WORD 077777u

/* The channel 13 bits that read back (12-14, the HANDRUPT traps, do not). */
#define CHANNEL13_KEPT 043777u

static jmp_buf back;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

static uint16_t *channel(unsigned number)
{
    switch (number) {
    case CHAN12: return &hardware_now->channel12;
    case CHAN13: return &hardware_now->channel13;
    case CHAN31: return &hardware_now->channel31;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return *channel(number);
}

static void channel_out(unsigned number, word value)
{
    if (number == CHAN31) not_modelled(); /* the file writes no input channel */
    if (number == CHAN13) value &= CHANNEL13_KEPT;
    *channel(number) = value;
}

/* A word of bank 16 between first and first + count - 1. */
static bool in_bank16(unsigned physical, word first, unsigned count)
{
    unsigned at = BANK16 * 02000u + (first - 02000u);
    return physical >= at && physical < at + count;
}

/* The rope's words at the tables the file reads by INDEX. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (!in_bank16(physical, JETSALL, 016u) && !in_bank16(physical, INDXYZ, 020u) &&
        !in_bank16(physical, MINTIMES, 2u) && physical != FF(BIT15_ADDRESS) &&
        physical != FF(BIT15_ADDRESS - 2u))
        not_modelled();
    if (!apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* What a key asks of the state beyond FB and EBANK. */
enum entry { ANY, FLOW, ROTINDEX_0_OR_1, RETURN_IN_Q };

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, enum entry entry, void (*routine)(void))
{
    mission_io *io = device, kept_io;
    agc_hardware kept_hardware;
    apollo_overlay_state before;

    if (!state || !program || !program->hardware || !bind_machine(program, state, io))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    if ((FBANK & 076000u) != (word)(BANK16 << 10) || EBANK != EBANK6_REGISTER)
        return EXEC_OUT_OF_SCOPE;
    if (entry != ANY && entry != RETURN_IN_Q && fetch(ADDRESS_OF(OLDPMIN)) == MINUS_ZERO_WORD)
        return EXEC_OUT_OF_SCOPE;
    if (entry == ROTINDEX_0_OR_1 && fetch(ADDRESS_OF(ROTINDEX)) > 1u) return EXEC_OUT_OF_SCOPE;
    if (entry == RETURN_IN_Q && (Q < 02000u || Q > 07777u)) return EXEC_OUT_OF_SCOPE;
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

#define KEY(name, entry)                                                                           \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, entry, name);                                           \
    }
KEY(paxis, ANY)
KEY(paxis_plus10, ANY)
KEY(paxis_after_chekbits, ANY)
KEY(backp, FLOW)
KEY(backp_plus0133, FLOW)
KEY(q_rate, FLOW)
KEY(r_rate, FLOW)
KEY(paxfilt_after_acdt_c12, ANY)
KEY(superjob, ANY)
KEY(pretimck, FLOW)
KEY(chkvisfz, FLOW)
KEY(abortyz_plus2, FLOW)
KEY(tsnextp_minus1, FLOW)
KEY(tsnextp, FLOW)
KEY(detentck, FLOW)
KEY(ratedone_after_zatteror, FLOW)
KEY(raterror, FLOW)
KEY(pjetslec_minus6, FLOW)
KEY(pjetslec_minus1, FLOW)
KEY(pjetslec_after_writep, ROTINDEX_0_OR_1)
KEY(pjetslec_after_jtlst, FLOW)
KEY(jetsoff_after_writep, FLOW)
KEY(calcperr, RETURN_IN_Q)
KEY(purgency, FLOW)
KEY(purgency_plus4, FLOW)
KEY(purgency_after_spsrcs, FLOW)
KEY(headtjet_after_tjetlaw, FLOW)
KEY(selectp_after_alarm, FLOW)

#define B16(z) (uint16_t)(BANK16 * 1024u + ((z) - 02000u))

/* By address, as the faithful tables have them. */
static const agc_routine_entry keys[] = {
    {B16(PAXIS), key_paxis},
    {B16(PAXIS + 010u), key_paxis_plus10},
    {B16(PAXIS + 011u), key_paxis_after_chekbits},
    {B16(BACKP), key_backp},
    {B16(BACKP + 0133u), key_backp_plus0133},
    {B16(Q_RATE), key_q_rate},
    {B16(R_RATE), key_r_rate},
    {B16(PAXFILT + 4u), key_paxfilt_after_acdt_c12},
    {B16(SUPERJOB), key_superjob},
    {B16(PRETIMCK), key_pretimck},
    {B16(CHKVISFZ), key_chkvisfz},
    {B16(ABORTYZ + 2u), key_abortyz_plus2},
    {B16(TSNEXTP - 1u), key_tsnextp_minus1},
    {B16(TSNEXTP), key_tsnextp},
    {B16(DETENTCK), key_detentck},
    {B16(RATEDONE + 016u), key_ratedone_after_zatteror},
    {B16(RATERROR), key_raterror},
    {B16(PJETSLEC - 6u), key_pjetslec_minus6},
    {B16(PJETSLEC - 1u), key_pjetslec_minus1},
    {B16(PJETSLEC + 025u), key_pjetslec_after_writep},
    {B16(PJETSLEC + 046u), key_pjetslec_after_jtlst},
    {B16(JETSOFF + 1u), key_jetsoff_after_writep},
    {B16(CALCPERR), key_calcperr},
    {B16(PURGENCY), key_purgency},
    {B16(PURGENCY + 4u), key_purgency_plus4},
    {B16(PURGENCY + 015u), key_purgency_after_spsrcs},
    {B16(HEADTJET + 5u), key_headtjet_after_tjetlaw},
    {B16(SELECTP + 017u), key_selectp_after_alarm},
};

/* The keys one 100 ms DAP pass reaches in attitude hold or jet firing. */
static const agc_routine_entry hot_keys[] = {
    {B16(PAXIS), key_paxis},       {B16(PAXIS + 011u), key_paxis_after_chekbits},
    {B16(BACKP), key_backp},       {B16(BACKP + 0133u), key_backp_plus0133},
    {B16(Q_RATE), key_q_rate},     {B16(R_RATE), key_r_rate},
    {B16(SUPERJOB), key_superjob}, {B16(PRETIMCK), key_pretimck},
    {B16(CHKVISFZ), key_chkvisfz}, {B16(TSNEXTP), key_tsnextp},
    {B16(DETENTCK), key_detentck}, {B16(JETSOFF + 1u), key_jetsoff_after_writep},
    {B16(PURGENCY), key_purgency}, {B16(HEADTJET + 5u), key_headtjet_after_tjetlaw},
};

/* PAXFILT +4, which the trim gimbal file's module carries. */
static const agc_routine_entry tg_hot_keys[] = {
    {B16(PAXFILT + 4u), key_paxfilt_after_acdt_c12},
};

const agc_routine_table lm_p_axis_routines = {keys, COUNT(keys)};
const agc_routine_table lm_p_axis_hot_routines = {hot_keys, COUNT(hot_keys)};
const agc_routine_table lm_p_axis_tg_hot_routines = {tg_hot_keys, COUNT(tg_hot_keys)};

#endif
