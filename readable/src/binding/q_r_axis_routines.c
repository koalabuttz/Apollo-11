/* SPDX-License-Identifier: MIT */
/* The readable Q_R-AXIS_RCS_AUTOPILOT (src/q_r_axis/), the LM's, as
 * routines of its flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_q_r_axis.c): lm_q_r_axis_routines, its
 * 30 keys in bank 17; lm_q_r_axis_hot_routines, the 12 one DAP pass
 * reaches; and lm_q_r_axis_tg_hot_routines, CLOSEOUT. The faithful
 * object's other tables (lm_q_r_axis_sites, lm_q_r_axis_data) stay the
 * faithful port's.
 *
 * Plumbing only: point the machine at the program's state, give it
 * channels 4, 5, 13 and 31 from the program's hardware record (channel 13
 * keeps only the bits that read back), the rope's words at the fixed
 * addresses the file reads by INDEX (ALLJETS with TYPEPOLY, INDEXES,
 * AXISDIFF, LIMITS -1 to +1) and MAKERUPT's EDRUPT, and take the states
 * the faithful routines take: FB bank 17, EBANK 6; for QERRCALC, JTLST
 * and ROT-TOUV a return in Q in fixed memory; at CALLQERR and the returns
 * into the file SKIPU and SKIPV 0 or 4 (what every producer stores); at
 * CALLQERR ALLOWGTS not -0 and channel 5 not negative; at the returns but
 * TIMEGMBL's no SENSETYP from 6 to 37777 (it picks a jet policy); and at
 * the returns into the axis loop AXISCTR 0 or 1, NUMBERT 0-4, and with
 * AXISCTR 1 one of the file's three return addresses in RETJADR (but at
 * WRITEU's and WRITEV's returns, which close the pass). A run that stops
 * at not_modelled() leaves the state, the io record and the hardware
 * record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "lm_q_r_axis.h"
#include "../q_r_axis/q_r_axis.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK17          017u
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
    case LOSCALAR: return &hardware_now->channel4;
    case CHAN5: return &hardware_now->channel5;
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
    if (number != CHAN13) not_modelled(); /* the file writes channel 13 only */
    *channel(number) = value & CHANNEL13_KEPT;
}

/* A word of bank 17 between first and first + count - 1. */
static bool in_bank17(unsigned physical, word first, unsigned count)
{
    unsigned at = BANK17 * 02000u + (first - 02000u);
    return physical >= at && physical < at + count;
}

/* The rope's words at the tables the file reads by INDEX, and EDRUPT. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (!in_bank17(physical, ALLJETS, 9u) && !in_bank17(physical, INDEXES, 2u) &&
        !in_bank17(physical, AXISDIFF, 2u) && physical != FF(LIMITS - 1u) &&
        physical != FF(LIMITS) && physical != FF(LIMITS + 1u) && physical != FF(MAKERUPT + 1u))
        not_modelled();
    if (!apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* What a key asks of the state beyond FB and EBANK. */
enum entry {
    PART,         /* a word the file's own runs hand Z on to */
    SUBROUTINE,   /* QERRCALC, JTLST, ROT-TOUV */
    FROM_P_AXIS,  /* CALLQERR */
    SKIPS,        /* TIMEGMBL's return */
    POLICY,       /* the returns from ALARM in X-translation and ZATTEROR */
    LOOP,         /* the returns into the axis loop */
    LOOP_CLOSING, /* WRITEU's and WRITEV's returns */
};

static bool skip_word(unsigned address)
{
    word skip = fetch(address);
    return skip == 0u || skip == 4u;
}

static bool known(enum entry entry)
{
    word sensetyp = fetch(ADDRESS_OF(SENSETYP)), axisctr = fetch(ADDRESS_OF(AXISCTR));
    word retjadr = fetch(ADDRESS_OF(RETJADR));

    switch (entry) {
    case PART: return true;
    case SUBROUTINE: return Q >= 02000u && Q <= 07777u;
    default: break;
    }
    if (!skip_word(ADDRESS_OF(SKIPU)) || !skip_word(ADDRESS_OF(SKIPV))) return false;
    switch (entry) {
    case FROM_P_AXIS:
        return fetch(ADDRESS_OF(ALLOWGTS)) != MINUS_ZERO_WORD &&
               !(hardware_now->channel5 & 040000u);
    case SKIPS: return true;
    default: break;
    }
    if (sensetyp > 5u && sensetyp < 040000u) return false;
    if (entry == POLICY) return true;
    if (axisctr > 1u || fetch(ADDRESS_OF(NUMBERT)) > 4u) return false;
    return entry == LOOP_CLOSING || axisctr == 0u || retjadr == MINRTN || retjadr == BACKHAND ||
           retjadr == TJLAW + 3u;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, enum entry entry, void (*routine)(void))
{
    mission_io *io = device, kept_io;
    agc_hardware kept_hardware;
    apollo_overlay_state before;

    if (!state || !program || !program->hardware || !bind_machine(program, state, io))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    if ((FBANK & 076000u) != (word)(BANK17 << 10) || EBANK != EBANK6_REGISTER || !known(entry))
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

#define KEY(name, entry)                                                                           \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, entry, name);                                           \
    }
KEY(callqerr, FROM_P_AXIS)
KEY(rcs, PART)
KEY(rcs_plus1, PART)
KEY(senseget_after_alarm, POLICY)
KEY(qrcontrl, PART)
KEY(qrcontrl_after_zatteror, POLICY)
KEY(minrtn, PART)
KEY(chekstik, PART)
KEY(enteruv_after_zatteror, POLICY)
KEY(backhand, PART)
KEY(trygts, PART)
KEY(chkingts_after_timegmbl, SKIPS)
KEY(qerrcalc, SUBROUTINE)
KEY(stillrcs, PART)
KEY(tjlaw_plus3, PART)
KEY(tjlaw_after_tjetlaw, LOOP)
KEY(tjlaw_after_spsrcs, LOOP)
KEY(dorotat_after_writep, LOOP)
KEY(norotat_after_writep, LOOP)
KEY(looper, PART)
KEY(doskip, PART)
KEY(doskip_plus1, PART)
KEY(notmin_after_writep, LOOP)
KEY(xtrans, PART)
KEY(xtrans_after_writeu, LOOP_CLOSING)
KEY(xtrans_after_writev, LOOP_CLOSING)
KEY(jtlst, SUBROUTINE)
KEY(rot_touv, SUBROUTINE)
KEY(selctsub_after_alarm, LOOP)
KEY(closeout, PART)

#define B17(z) (uint16_t)(BANK17 * 1024u + ((z) - 02000u))

/* By address, as the faithful tables have them. */
static const agc_routine_entry keys[] = {
    {B17(CALLQERR), key_callqerr},
    {B17(RCS), key_rcs},
    {B17(RCS + 1u), key_rcs_plus1},
    {B17(TSNUMBRT + 6u), key_senseget_after_alarm},
    {B17(QRCONTRL), key_qrcontrl},
    {B17(QRCONTRL + 014u), key_qrcontrl_after_zatteror},
    {B17(MINRTN), key_minrtn},
    {B17(CHEKSTIK), key_chekstik},
    {B17(ENTERUV + 3u), key_enteruv_after_zatteror},
    {B17(BACKHAND), key_backhand},
    {B17(TRYGTS), key_trygts},
    {B17(CHKINGTS + 6u), key_chkingts_after_timegmbl},
    {B17(QERRCALC), key_qerrcalc},
    {B17(STILLRCS), key_stillrcs},
    {B17(TJLAW + 3u), key_tjlaw_plus3},
    {B17(TJLAW + 026u), key_tjlaw_after_tjetlaw},
    {B17(TJLAW + 036u), key_tjlaw_after_spsrcs},
    {B17(DOROTAT + 032u), key_dorotat_after_writep},
    {B17(NOROTAT + 5u), key_norotat_after_writep},
    {B17(LOOPER), key_looper},
    {B17(DOSKIP), key_doskip},
    {B17(DOSKIP + 1u), key_doskip_plus1},
    {B17(NOTMIN + 010u), key_notmin_after_writep},
    {B17(XTRANS), key_xtrans},
    {B17(XTRANS + 011u), key_xtrans_after_writeu},
    {B17(XTRANS + 020u), key_xtrans_after_writev},
    {B17(JTLST), key_jtlst},
    {B17(ROT_TOUV), key_rot_touv},
    {B17(SELCTSUB + 031u), key_selctsub_after_alarm},
    {B17(CLOSEOUT), key_closeout},
};

/* The keys one 100 ms DAP pass reaches in attitude hold or jet firing. */
static const agc_routine_entry hot_keys[] = {
    {B17(CALLQERR), key_callqerr},
    {B17(RCS), key_rcs},
    {B17(QRCONTRL), key_qrcontrl},
    {B17(TRYGTS), key_trygts},
    {B17(STILLRCS), key_stillrcs},
    {B17(TJLAW + 3u), key_tjlaw_plus3},
    {B17(TJLAW + 026u), key_tjlaw_after_tjetlaw},
    {B17(DOROTAT + 032u), key_dorotat_after_writep},
    {B17(NOROTAT + 5u), key_norotat_after_writep},
    {B17(LOOPER), key_looper},
    {B17(DOSKIP), key_doskip},
    {B17(NOTMIN + 010u), key_notmin_after_writep},
};

/* CLOSEOUT, which GTS reaches by DTCB: the trim gimbal file's module
 * carries it. */
static const agc_routine_entry tg_hot_keys[] = {
    {B17(CLOSEOUT), key_closeout},
};

const agc_routine_table lm_q_r_axis_routines = {keys, COUNT(keys)};
const agc_routine_table lm_q_r_axis_hot_routines = {hot_keys, COUNT(hot_keys)};
const agc_routine_table lm_q_r_axis_tg_hot_routines = {tg_hot_keys, COUNT(tg_hot_keys)};

#endif
