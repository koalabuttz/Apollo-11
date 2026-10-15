/* SPDX-License-Identifier: MIT */
#include "dntm_rupt_routines.h"
#include "lm_downtelemetry.h"
#include "lm_t6rupt_routines.h"
#include "agc_image_ops.h"
#define W     s->words
#define FF(z) (uint16_t)(2048u + ((z) - 04000u))

/* DOWNRUPT 04040: DXCH ARUPT; CAF DNRPTBB (12100); XCH BBANK; TCF DODOWNTM. */
static enum exec_status vector(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    if (!s || !v || !p || !p->hardware || p->hardware->in_interrupt != 1u || W[5] != 04040u)
        return EXEC_OUT_OF_SCOPE;
    ai_dxch(s, 010u);
    ai_caf(s, 012100u);
    ai_xch(s, 6u);
    W[5] = 03506u;
    return EXEC_OK;
}
/* RESUME 05270: QXCH QRUPT; CA BANKRUPT; XCH BBANK; DXCH ARUPT; RELINT;
 * RESUME to ZRUPT-1, BRUPT the word there under the banks restored (#144). */
static enum exec_status resume(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    agc_hardware *h = p ? p->hardware : 0;
    lm_dntm_io *x = v;
    agc_word bank;
    if (!h || !x || !s || h->in_interrupt != 1u || W[015] == 0u || W[015] > 010000u ||
        W[5] != LM_RESUME)
        return EXEC_OUT_OF_SCOPE;
    bank = W[016];
    if (bank & (agc_word)~076167u) return EXEC_OUT_OF_SCOPE;
    if (!agc_resume_exact(p, s, (agc_word)(bank & 076000u), (agc_word)((bank & 7u) << 8),
                          x->control.superbank))
        return EXEC_OUT_OF_SCOPE;
    ai_qxch(s, 012u);
    ai_ca(s, 016u);
    ai_xch(s, 6u);
    ai_dxch(s, 010u);
    x->control.interrupt_enabled = 1;
    W[5] = (agc_word)(W[015] - 1u);
    W[7] = 0;
    h->in_interrupt = 0;
    return EXEC_OK;
}
static const agc_routine_entry entries[2] = {{FF(04040u), vector}, {FF(LM_RESUME), resume}};
const agc_routine_table lm_dntm_rupt_routines = {entries, 2};
