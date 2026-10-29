/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_ev_routines.h"
#include "lanec_ev_front.h"
#include "lanec_ev_short.h"
#include "lanec_ev_align.h"
#include "lanec_ev_radar.h"
#include "lanec_ev_dap.h"
#include "lanec_ev_time.h"
#include "lanec_ev_track.h"
#include "lanec_ev_lmi.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"
#include "lm_interp_orders.h"

typedef enum lanec_status (*dispatch)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
static lanec_io io;   /* the trampoline runs one routine at a time */
/* the channels the modules use beyond 11, as carried by the device records */
#define CHANNELS(X, r) X(012, r channel12) X(013, r channel13) X(014, r channel14) X(030, r channel30) \
    X(031, r channel31) X(032, r channel32) X(033, r channel33)
#define IN(ch, field) io.channel[ch] = (uint16_t)(field);
#define OUT(ch, field) field = io.channel[ch];

static enum exec_status lm(const agc_native_program *p, apollo_overlay_state *s, void *v, dispatch f)
{
    lm_dntm_io *x = v;
    if (!p || !s || !x || !p->hardware) return EXEC_OUT_OF_SCOPE;
    memset(&io, 0, sizeof io);
    io.superbank = x->control.superbank; io.interrupt_enabled = x->control.interrupt_enabled;
    io.channel[011] = p->hardware->channel11;
    CHANNELS(IN, x->)
    if (f(s, &io, LANEC_LM) != LANEC_OK) return EXEC_OUT_OF_SCOPE;
    x->control.superbank = io.superbank; x->control.interrupt_enabled = io.interrupt_enabled;
    p->hardware->channel11 = x->channel11 = io.channel[011];
    CHANNELS(OUT, x->)
    return EXEC_OK;
}

static enum exec_status cm(const agc_native_program *p, apollo_overlay_state *s, void *v, dispatch f)
{
    cm_dntm_io *x = v;
    if (!p || !s || !x) return EXEC_OUT_OF_SCOPE;
    memset(&io, 0, sizeof io);
    io.superbank = x->io.superbank; io.interrupt_enabled = x->io.interrupt_enabled;
    io.channel[011] = x->io.channel11; io.channel[012] = x->io.channel12;
    io.channel[013] = x->io.channel13; io.channel[014] = x->io.channel14;
    io.channel[030] = x->channel30; io.channel[031] = x->channel31;
    io.channel[032] = x->channel32; io.channel[033] = x->channel33;
    if (f(s, &io, LANEC_CM) != LANEC_OK) return EXEC_OUT_OF_SCOPE;
    x->io.superbank = io.superbank; x->io.interrupt_enabled = io.interrupt_enabled;
    x->io.channel11 = io.channel[011]; x->io.channel12 = io.channel[012];
    x->io.channel13 = io.channel[013]; x->io.channel14 = io.channel[014];
    x->channel30 = io.channel[030]; x->channel31 = io.channel[031];
    x->channel32 = io.channel[032]; x->channel33 = io.channel[033];
    return EXEC_OK;
}

#define LM(m) static enum exec_status m##_lm(const agc_native_program *p, apollo_overlay_state *s, void *v) \
    { return lm(p, s, v, lanec_ev_##m##_dispatch); }
#define CM(m) static enum exec_status m##_cm(const agc_native_program *p, apollo_overlay_state *s, void *v) \
    { return cm(p, s, v, lanec_ev_##m##_dispatch); }
LM(front) LM(short) LM(align) LM(radar) LM(dap) LM(time) LM(track) LM(lmi)
CM(front) CM(short) CM(align) CM(dap) CM(time) CM(track)

#include "lanec_ev_routines.inc"
const agc_routine_table lanec_ev_lm_routines = { lm_entries, LANEC_EV_LM_ROUTINES };
const agc_routine_table lanec_ev_lm_interpretive = { lm_interp_entries, LANEC_EV_LM_INTERP_ROUTINES };

/* TC INTPRET from this file's basic code: INTPRET (RELINT; QXCH LOC; BANKSET,
 * INTBIT15 and EDOP from BB; TCF NEWOPS) for a Q naming one of the
 * interpretive points above, which the trampoline then runs at NEWOPS. Any
 * other caller is the interpreter's own open work and is refused. */
static enum exec_status intpret_lm(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    lm_dntm_io *x = v;
    uint16_t i, key;
    if (!p || !s || !x || s->words[LANEC_Z] != 06037u || !lanec_registers_valid(s)) return EXEC_OUT_OF_SCOPE;
    memset(&io, 0, sizeof io);
    io.superbank = x->control.superbank;
    if (s->words[LANEC_Q] < 02000u || s->words[LANEC_Q] >= 04000u) return EXEC_OUT_OF_SCOPE;
    key = (uint16_t)(lanec_current_bank(s, &io) * 1024u + s->words[LANEC_Q] - 02000u);
    for (i = 0; i < LANEC_EV_LM_INTERP_ROUTINES && lm_interp_entries[i].physical != key; ++i) continue;
    if (i == LANEC_EV_LM_INTERP_ROUTINES) return EXEC_OUT_OF_SCOPE;
    lmi_intpret(s, &x->control);
    return EXEC_OK;
}
static const agc_routine_entry intpret_entry[1] = { { 2048u + (06037u - 04000u), intpret_lm } };
const agc_routine_table lanec_ev_lm_intpret = { intpret_entry, 1u };
const agc_routine_table lanec_ev_cm_routines = { cm_entries, LANEC_EV_CM_ROUTINES };
