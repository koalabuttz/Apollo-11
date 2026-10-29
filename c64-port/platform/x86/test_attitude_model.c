/* SPDX-License-Identifier: MIT */
/* Checks platform/x86/attitude_model.h, either vehicle turning under its
 * jets: the CDUs round trip, a jet spins the body up at the rate the AGC's
 * own numbers give, the CDUs follow, and the CSM holds still while the SPS
 * burns (make x86-flight-check builds it once for each mission). */
#include <stdio.h>
#include <string.h>
#include "attitude_model.h"

static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s\n", what);
        ++failures;
    }
}

static agc_word w[2048];

int main(void)
{
    vm_attitude a;
    unsigned k;
    memset(&a, 0, sizeof a);
    memset(&vm_cfg, 0, sizeof vm_cfg);
    /* The CDUs to the body axes and back, at a general attitude. */
    w[VM_CDUX] = 01234u;
    w[VM_CDUY] = 05670u;
    w[VM_CDUZ] = 00321u;
    att_from_cdus(&a, w);
    att_to_cdus(&a, w);
    check(w[VM_CDUX] == 01234u && w[VM_CDUY] == 05670u && w[VM_CDUZ] == 00321u, "CDUs round trip");

    /* Off: nothing moves. */
    vm_attitude_step(&a, 100, w, 0377u, 0377u, 0, 0);
    check(w[VM_CDUX] == 01234u && a.t == 0, "FLIGHT_ATT unset: no change");

    memset(&a, 0, sizeof a);
    memset(w, 0, sizeof w);
    vm_cfg.att = 1.0;
#if RK_LM
    /* One +P jet (channel 6 bit 1) at 1JACC = 16384 (pi/4 rad/s/s) for 1 s. */
    w[ATT_1JACC] = 040u;
    vm_attitude_step(&a, 100, w, 0, 01u, 0, 0);
    vm_attitude_step(&a, 200, w, 0, 0, 0, 0);
    check(a.rate[0] > 0.0 && a.rate[1] == 0.0 && a.rate[2] == 0.0, "a +P jet spins up P alone");
    check(a.rate[0] > 0.999 * 040 * 0.7853981633974483 / 16384.0 &&
              a.rate[0] < 1.001 * 040 * 0.7853981633974483 / 16384.0,
          "P rate is 1JACC times a second");
#else
    /* Two +roll jets (channel 6 bits 1 and 3) for 1 s, IXX 1000 (2^20 kg m^2 / 64 a count). */
    w[ATT_IXX] = 1000u;
    vm_attitude_step(&a, 100, w, 0, 05u, 0, 0);
    vm_attitude_step(&a, 200, w, 0, 0, 0, 0);
    {
        double want = 2.0 * ATT_JET * ATT_ARM / (1000.0 * 64.0);
        check(a.rate[0] > 0.999 * want && a.rate[0] < 1.001 * want,
              "roll rate from two jets over IXX");
    }
    /* While the SPS burns the attitude stays with the SPS model, at rest. */
    vm_attitude_step(&a, 300, w, 0, 05u, 1, 0);
    check(a.rate[0] == 0.0 && !a.live, "held during the SPS burn");
#endif
    /* Coasting at that rate the outer gimbal follows the body. */
    memset(&a, 0, sizeof a);
    memset(w, 0, sizeof w);
    vm_attitude_step(&a, 100, w, 0, 0, 0, 0);
    a.rate[0] = 0.1;
    vm_attitude_step(&a, 200, w, 0, 0, 0, 0);
    check(w[VM_CDUX] == att_cdu(0.1) && w[VM_CDUY] == 0 && w[VM_CDUZ] == 0,
          "coasting 1 s at 0.1 rad/s about X");
    /* A CDU moved by something else is taken as the new attitude. */
    w[VM_CDUY] = 01000u;
    a.rate[0] = 0.0;
    vm_attitude_step(&a, 300, w, 0, 0, 0, 0);
    for (k = 0; k < 3u; ++k) check(a.cdu[k] == w[VM_CDUX + k], "attitude taken from moved CDUs");
    if (failures) return 1;
    printf("attitude model: all checks pass\n");
    return 0;
}
