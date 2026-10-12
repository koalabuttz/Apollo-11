/* SPDX-License-Identifier: MIT */
/* pw-nv-admission: NVSUB/NVMONOPT and the internal NVSUB1/NVSUB2 bodies
 * through the selected callee entry (src/dsky_nv_admission.c). The
 * generator chooses mission and entry; each entry is admitted only at its
 * own Z, so run() takes the first entry the module accepts. */
#include "lanec_pw.h"
#include "dsky_nv_admission.h"
#include "dsky_nv_admission_cases.h"

#define CASES 1024u
/* past the domain: NVSUB1 with NVTEMP +0 and -0 (TC GODSPALM) */
static const uint16_t extra[] = { 1, 2049, 4096, 4097, 5121 };
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    enum dsky_nv_entry entry;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_NV_ADMISSION_CASES);
    lanec_fill(s, io, n);
    if (k >= DSKY_NV_ADMISSION_CASES) k = extra[k - DSKY_NV_ADMISSION_CASES];
    dsky_nv_admission_input(s, &p, k, &mission, &entry);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_nv_admission_run);
}
const lanec_suite lanec_suite_pw_nv_admission = { "pw-nv-admission", CASES + sizeof extra / sizeof extra[0], make, lanec_pw_nv_admission_run };
