/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_di_routines.h"

static lanec_io io;   /* the trampoline runs one routine at a time */

enum exec_status lanec_di_run_lm(const agc_native_program *p, apollo_overlay_state *s, p66_bias_io *x,
                                 lanec_di_dispatch f)
{
    enum lanec_status status;
    if (!p || !s || !x || !p->hardware) return EXEC_OUT_OF_SCOPE;
    memset(&io, 0, sizeof io);
    io.superbank = x->superbank; io.interrupt_enabled = x->interrupt_enabled;
    io.channel[011] = p->hardware->channel11;
    status = f(s, &io, LANEC_LM);
    if (status != LANEC_OK) return EXEC_OUT_OF_SCOPE;
    x->superbank = io.superbank; x->interrupt_enabled = io.interrupt_enabled;
    p->hardware->channel11 = io.channel[011];
    return EXEC_OK;
}

enum exec_status lanec_di_run_cm(const agc_native_program *p, apollo_overlay_state *s, cm_native_io *x,
                                 lanec_di_dispatch f)
{
    enum lanec_status status;
    if (!p || !s || !x) return EXEC_OUT_OF_SCOPE;
    memset(&io, 0, sizeof io);
    io.superbank = x->superbank; io.interrupt_enabled = x->interrupt_enabled;
    io.channel[011] = x->channel11;
    status = f(s, &io, LANEC_CM);
    if (status != LANEC_OK) return EXEC_OUT_OF_SCOPE;
    x->superbank = io.superbank; x->interrupt_enabled = io.interrupt_enabled;
    x->channel11 = io.channel[011];
    return EXEC_OK;
}
