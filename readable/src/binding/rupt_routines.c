/* SPDX-License-Identifier: MIT */
/* The readable interrupt lead-ins (src/lead_ins/), the end of an interrupt
 * (RESUME and the rest, src/waitlist/) and the LM's T6 interrupt
 * (src/t6rupt/) as routines of the flight program, under the table names
 * and keys of the faithful port's: lm_lead_in_routines,
 * lm_lead_in_hot_routines and cm_lead_in_routines
 * (c64-port/src/rupt_lead_ins.c, which holds the CM's RESUME entries too),
 * and lm_t6rupt_routines with its two hot subsets
 * (c64-port/src/lm_t6rupt.c, which holds the LM's T6RUPT lead-in and
 * RESUME entries).
 *
 * Plumbing only: point the machine at the program's state, give the T6
 * routines channels 5, 6 and 13 from the program's hardware record, and
 * take the states the faithful routines take:
 *   - every lead-in but GO's inside its interrupt, and a DTCB lead-in's
 *     2CADR in fixed memory (02000-07777);
 *   - RESUME and the rest inside the interrupt, with an address in ZRUPT
 *     (1-010000), a bank word in BANKRUPT where they restore it, and BRUPT
 *     the word at ZRUPT -1 under the banks they restore (the AGC's RESUME
 *     reruns BRUPT, the readable one the word at ZRUPT -1);
 *   - T6JOBCHK (and DOT6RUPT, in EBANK 6 inside the interrupt, FBANK at
 *     bank 17) with TIME6 not negative, at -0 in EBANK 6 with NXT6ADR
 *     naming a writer, and a fixed return address in Q unless TIME6 is +0;
 *   - the writers with a fixed return address in Q, the -1 entries in
 *     EBANK 6, WRITEU and WRITEV with no overflow in A. */
#include <stddef.h>
#include "binding.h"
#include "../lead_ins/lead_ins.h"
#include "../waitlist/waitlist.h"
#include "../t6rupt/t6rupt.h"
#include "../agc/names.h"
#include "agc_transfer.h"

static agc_hardware *hardware_now;

static bool in_interrupt(void)
{
    return hardware_now && hardware_now->in_interrupt == 1u;
}

/* The CADR of a 2CADR in erasable (below 01400: no bank), in fixed memory. */
static bool fixed_cadr(unsigned address)
{
    word cadr = agc.memory[address];
    return cadr >= 02000u && cadr <= 07777u;
}

/* ---- The lead-ins ---- */

static enum exec_status run_lead_in(word entry)
{
    if (entry == LEAD_IN(RESTART_GO)) {
        go_lead_in();
        return EXEC_OK;
    }
    if (!in_interrupt()) return EXEC_OUT_OF_SCOPE;
#if LUMINARY099
    if (entry == LEAD_IN(RUPT_T5)) {
        if (!fixed_cadr(ADDRESS_OF(T5ADR))) return EXEC_OUT_OF_SCOPE;
        t5rupt_lead_in();
        return EXEC_OK;
    }
#else
    if (entry == LEAD_IN(RUPT_T6)) {
        if (!fixed_cadr(ADDRESS_OF(T6LOC))) return EXEC_OUT_OF_SCOPE;
        t6rupt_lead_in();
        return EXEC_OK;
    }
    if (entry == LEAD_IN(RUPT_T5) || entry == T5RUPT) {
        if (!fixed_cadr(ADDRESS_OF(T5LOC))) return EXEC_OUT_OF_SCOPE;
        if (entry == T5RUPT)
            t5rupt();
        else
            t5rupt_lead_in();
        return EXEC_OK;
    }
#endif
    switch (entry) {
    case LEAD_IN(RUPT_T4): t4rupt_lead_in(); break;
    case LEAD_IN(RUPT_KEY1): keyrupt1_lead_in(); break;
    case LEAD_IN(RUPT_KEY2): keyrupt2_lead_in(); break;
    case LEAD_IN(RUPT_UP): uprupt_lead_in(); break;
    case LEAD_IN(RUPT_DOWN): downrupt_lead_in(); break;
    case LEAD_IN(RUPT_RADAR): radar_rupt_lead_in(); break;
#if LUMINARY099
    case LEAD_IN(RUPT_10): rupt10_lead_in(); break;
#else
    case LEAD_IN(RUPT_10): hand_control_rupt_lead_in(); break;
#endif
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* ---- The end of an interrupt ---- */

/* BRUPT is the word at ZRUPT -1 under FBANK fb and EBANK eb. */
static bool resumes_exactly(const agc_native_program *program, const apollo_overlay_state *state,
                            word fb, word eb)
{
    return agc_resume_exact(program, state, fb, eb, *agc.superbank);
}

static enum exec_status run_resume(const agc_native_program *program,
                                   const apollo_overlay_state *state, word entry)
{
    bool restores_bbank = entry == RESUME || entry == NOQRSM;
    word bb;

    if (!in_interrupt() || ZRUPT == 0u || ZRUPT > 010000u) return EXEC_OUT_OF_SCOPE;
    if (restores_bbank && (BANKRUPT & (word)~076167u)) return EXEC_OUT_OF_SCOPE;
    /* The banks the AGC's RESUME reads BRUPT's word under: BANKRUPT's
     * where it is restored, else the current ones (the CM's RESUME +3:
     * the BBANK in A). */
#if LUMINARY099
    if (!restores_bbank) {
        if (!resumes_exactly(program, state, FBANK, EBANK)) return EXEC_OUT_OF_SCOPE;
    } else
#endif
    {
        bb = entry == NOQBRSM ? BBANK : entry == RESUME + 3u ? a_word() : BANKRUPT;
        if (!resumes_exactly(program, state, (word)(bb & 076000u), (word)((bb & 7u) << 8)))
            return EXEC_OUT_OF_SCOPE;
    }

    if (entry == RESUME)
        resume();
    else if (entry == NOQRSM)
        noqrsm();
    else if (entry == RESUME + 3u)
        resume_plus3();
    else if (entry == NOQBRSM)
        noqbrsm();
    else
        noqbrsm_plus1();
    return EXEC_OK;
}

#if LUMINARY099
/* ---- The LM's T6 interrupt ---- */

#define T6_BANK 036000u /* bank 17 */

/* Channels 5 and 6 (the RCS jets) and 13 (TIME6's enable, bit 15). */
static word channel_in(unsigned channel)
{
    if (channel == CHAN5) return hardware_now->channel5;
    if (channel != CHAN13) not_modelled();
    return hardware_now->channel13;
}

static void channel_out(unsigned channel, word value)
{
    if (channel == CHAN5)
        hardware_now->channel5 = value;
    else if (channel == CHAN6)
        hardware_now->channel6 = value;
    else if (channel == CHAN13)
        hardware_now->channel13 = (uint16_t)(value & 043777u); /* the trap resets do not stay set */
    else
        not_modelled();
}

/* A return address in fixed memory. */
static bool returnable(word q)
{
    return q >= 02000u && q <= 07777u;
}

/* T6JOBCHK's domain, returning to q. */
static bool t6jobchk_takes(word q)
{
    word time6 = ERASABLE(K_TIME6);

    if (time6 != MINUS_ZERO && (time6 & SIGN_BIT)) return false;
    if (time6 == MINUS_ZERO) {
        word writer;
        if (EBANK != EBANK6) return false;
        writer = ERASABLE(K_NXT6ADR);
        if (writer != 0u && writer != 4u && writer != 13u) return false;
    }
    return returnable(q) || time6 == PLUS_ZERO;
}

static enum exec_status run_t6(word entry)
{
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    switch (entry) {
    case LEAD_IN(RUPT_T6):
        if (!in_interrupt()) return EXEC_OUT_OF_SCOPE;
        t6rupt_lead_in();
        return EXEC_OK;
    case T6JOBCHK:
        if (!t6jobchk_takes(Q)) return EXEC_OUT_OF_SCOPE;
        t6jobchk();
        return EXEC_OK;
    case DOT6RUPT:
        if (!in_interrupt() || FBANK != T6_BANK || EBANK != EBANK6 ||
            !t6jobchk_takes((word)(DOT6RUPT + 4u)))
            return EXEC_OUT_OF_SCOPE;
        dot6rupt();
        return EXEC_OK;
    default: break;
    }
    /* The writers. */
    if (!returnable(Q)) return EXEC_OUT_OF_SCOPE;
    if ((entry == WRITEP - 1u || entry == WRITEU - 1u || entry == WRITEV - 1u) && EBANK != EBANK6)
        return EXEC_OUT_OF_SCOPE;
    if ((entry == WRITEU || entry == WRITEV) && overflow_of(A) != 0) return EXEC_OUT_OF_SCOPE;
    switch (entry) {
    case WRITEP - 1u: writep_minus1(); break;
    case WRITEP: writep(); break;
    case WRITEU - 1u: writeu_minus1(); break;
    case WRITEU: writeu(); break;
    case WRITEV - 1u: writev_minus1(); break;
    case WRITEV: writev(); break;
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}
#endif

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;

    if (!state || !program || !bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    entry = Z;
    if (entry == RESUME || entry == NOQRSM || entry == RESUME + 3u || entry == NOQBRSM ||
        entry == NOQBRSM + 1u)
        return run_resume(program, state, entry);
#if LUMINARY099
    if (entry == LEAD_IN(RUPT_T6) || entry == T6JOBCHK || entry == DOT6RUPT || entry >= WRITEP - 1u)
        return hardware_now ? run_t6(entry) : EXEC_OUT_OF_SCOPE;
#endif
    return run_lead_in(entry);
}

/* ---- The tables, in the faithful ones' order ---- */

#define LEAD_IN_KEY(n) FF(LEAD_IN(n))

#if LUMINARY099
#define B17(z) (uint16_t)(017u * 1024u + ((z) - 02000u))

static const agc_routine_entry lead_ins[] = {
    {LEAD_IN_KEY(RESTART_GO), run}, {LEAD_IN_KEY(RUPT_T5), run},    {LEAD_IN_KEY(RUPT_T4), run},
    {LEAD_IN_KEY(RUPT_KEY1), run},  {LEAD_IN_KEY(RUPT_KEY2), run},  {LEAD_IN_KEY(RUPT_UP), run},
    {LEAD_IN_KEY(RUPT_DOWN), run},  {LEAD_IN_KEY(RUPT_RADAR), run}, {LEAD_IN_KEY(RUPT_10), run}};
const agc_routine_table lm_lead_in_routines = {lead_ins, COUNT(lead_ins)};
/* The same entries, which the C64's flight store packs with its idle and
 * T4RUPT modules. */
const agc_routine_table lm_lead_in_hot_routines = {lead_ins, COUNT(lead_ins)};

static const agc_routine_entry t6rupt[] = {
    {LEAD_IN_KEY(RUPT_T6), run}, {FF(K_RESUME), run},       {FF(K_NOQRSM), run},
    {FF(K_NOQBRSM), run},        {FF(K_NOQBRSM + 1u), run}, {FF(K_WRITEP - 1u), run},
    {FF(K_WRITEP), run},         {FF(K_WRITEU - 1u), run},  {FF(K_WRITEU), run},
    {FF(K_WRITEV - 1u), run},    {FF(K_WRITEV), run},       {B17(K_T6JOBCHK), run},
    {B17(K_DOT6RUPT), run}};
const agc_routine_table lm_t6rupt_routines = {t6rupt, COUNT(t6rupt)};
/* The RESUME entries (pinned on the C64) and the writers (carried with
 * the autopilot's modules). */
const agc_routine_table lm_t6rupt_hot_routines = {t6rupt + 1, 4};
const agc_routine_table lm_t6rupt_jets_hot_routines = {t6rupt + 5, 6};
#else
static const agc_routine_entry lead_ins[] = {{LEAD_IN_KEY(RESTART_GO), run},
                                             {LEAD_IN_KEY(RUPT_T6), run},
                                             {LEAD_IN_KEY(RUPT_T5), run},
                                             {LEAD_IN_KEY(RUPT_T4), run},
                                             {LEAD_IN_KEY(RUPT_KEY1), run},
                                             {LEAD_IN_KEY(RUPT_KEY2), run},
                                             {LEAD_IN_KEY(RUPT_UP), run},
                                             {LEAD_IN_KEY(RUPT_DOWN), run},
                                             {LEAD_IN_KEY(RUPT_RADAR), run},
                                             {LEAD_IN_KEY(RUPT_10), run},
                                             {FF(K_T5RUPT), run},
                                             {FF(K_RESUME), run},
                                             {FF(K_NOQRSM), run},
                                             {FF(K_RESUME + 3u), run},
                                             {FF(K_NOQBRSM), run}};
const agc_routine_table cm_lead_in_routines = {lead_ins, COUNT(lead_ins)};
#endif
