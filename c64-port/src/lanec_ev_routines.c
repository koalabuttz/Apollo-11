/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_ev_routines.h"
#include "lm_interp_orders.h"

static lanec_io io;   /* the trampoline runs one routine at a time */
/* the channels the modules use beyond 11, as carried by the device records */
#define CHANNELS(X, r) X(012, r channel12) X(013, r channel13) X(014, r channel14) X(030, r channel30) \
    X(031, r channel31) X(032, r channel32) X(033, r channel33)
#define IN(ch, field) io.channel[ch] = (uint16_t)(field);
#define OUT(ch, field) field = io.channel[ch];

enum exec_status lanec_ev_run_lm(const agc_native_program *p, apollo_overlay_state *s, lm_dntm_io *x,
                                 lanec_ev_dispatch f)
{
    if (!p || !s || !x || !p->hardware) return EXEC_OUT_OF_SCOPE;
    memset(&io, 0, sizeof io);
    io.superbank = x->control.superbank; io.interrupt_enabled = x->control.interrupt_enabled;
    io.channel[011] = p->hardware->channel11;
    CHANNELS(IN, x->)
    if (f(s, &io, LANEC_LM) != LANEC_OK) return EXEC_OUT_OF_SCOPE;
    x->control.superbank = io.superbank; x->control.interrupt_enabled = io.interrupt_enabled;
    p->hardware->channel11 = io.channel[011];
    CHANNELS(OUT, x->)
    return EXEC_OK;
}

enum exec_status lanec_ev_run_cm(const agc_native_program *p, apollo_overlay_state *s, cm_dntm_io *x,
                                 lanec_ev_dispatch f)
{
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

/* TC INTPRET from this file's basic code: INTPRET (RELINT; QXCH LOC; BANKSET,
 * INTBIT15 and EDOP from BB; TCF NEWOPS) for a Q naming one of the
 * interpretive points of the table given, which the trampoline then runs at
 * NEWOPS. Any other caller is the interpreter's own open work and is refused. */
enum exec_status lanec_ev_intpret_lm(const agc_native_program *p, apollo_overlay_state *s, lm_dntm_io *x,
                                     const agc_routine_table *points)
{
    uint16_t i, key;
    if (!p || !s || !x || s->words[LANEC_Z] != 06037u || !lanec_registers_valid(s)) return EXEC_OUT_OF_SCOPE;
    memset(&io, 0, sizeof io);
    io.superbank = x->control.superbank;
    if (s->words[LANEC_Q] < 02000u || s->words[LANEC_Q] >= 04000u) return EXEC_OUT_OF_SCOPE;
    key = (uint16_t)(lanec_current_bank(s, &io) * 1024u + s->words[LANEC_Q] - 02000u);
    for (i = 0; i < points->count && points->entries[i].physical != key; ++i) continue;
    if (i == points->count) return EXEC_OUT_OF_SCOPE;
    lmi_intpret(s, &x->control);
    return EXEC_OK;
}
