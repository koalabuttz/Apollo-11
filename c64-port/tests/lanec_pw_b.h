/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_B_H
#define LANEC_PW_B_H
/* Adapters shared by the batch-b pw-* suites (tests/lanec_pw_*_cases.c,
 * tools/mk/lanec-pw-b.mk): the module entry signatures that
 * LANEC_PW_P66_RUN does not cover. Test-only. */
#include "lanec_pw.h"
#include "lanec_pw_b_run.h"
#include "dsky_lamp_test.h"
#include "dsky_nv_admission.h"

/* Staged modules (one native call runs one explicit source stage, selected
 * by the image's Z): move a generated state up to `stages` stages along its
 * own path with the native module, stopping early at the last state the
 * module still accepts. Every stage was frozen against the original program
 * by the module's own differential; the suite then checks the next stage
 * against Virtual AGC from that state. */
static inline void lanec_pw_advance(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission,
                                    lanec_run run, uint32_t stages)
{
    static apollo_overlay_state t, v;
    lanec_io u, x;
    for (; stages; --stages) {
        t = *s; u = *io;
        if (run(&t, &u, mission) != LANEC_OK) return;
        lanec_pw_settle(&t);
        v = t; x = u;
        if (run(&v, &x, mission) != LANEC_OK) return;
        *s = t; *io = u;
    }
}
#endif
