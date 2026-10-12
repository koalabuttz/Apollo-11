/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_di_routines.h"
#include "lanec_di_dsp.h"
#include "lanec_di_mark.h"
#include "lanec_di_play.h"
#include "lanec_di_sleep.h"
#include "lanec_di_idle.h"
#include "lanec_di_nvdsp.h"
#include "p66_bias.h"
#include "cm_native_image.h"

typedef enum lanec_status (*dispatch)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
static lanec_io io;   /* the trampoline runs one routine at a time */

static enum exec_status lm(const agc_native_program *p, apollo_overlay_state *s, void *v, dispatch f)
{
    p66_bias_io *x = v;
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

static enum exec_status cm(const agc_native_program *p, apollo_overlay_state *s, void *v, dispatch f)
{
    cm_native_io *x = v;
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

#define ROUTINES(m) \
    static enum exec_status m##_##lm(const agc_native_program *p, apollo_overlay_state *s, void *v) \
    { return lm(p, s, v, lanec_di_##m##_dispatch); } \
    static enum exec_status m##_##cm(const agc_native_program *p, apollo_overlay_state *s, void *v) \
    { return cm(p, s, v, lanec_di_##m##_dispatch); }
ROUTINES(dsp)
ROUTINES(mark)
ROUTINES(play)
ROUTINES(sleep)
ROUTINES(idle)
ROUTINES(nvdsp)

#include "lanec_di_routines.inc"
const agc_routine_table lanec_di_lm_routines = { lm_entries, LANEC_DI_LM_ROUTINES };
const agc_routine_table lanec_di_cm_routines = { cm_entries, LANEC_DI_CM_ROUTINES };
