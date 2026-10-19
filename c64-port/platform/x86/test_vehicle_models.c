/* SPDX-License-Identifier: MIT */
/* Checks for platform/x86/vehicle_models.h: each model on a hand-built
 * erasable image, against the physics its comment states. Links nothing
 * but libm.
 *
 *   cc -std=c99 -Iinclude -Iplatform/x86 platform/x86/test_vehicle_models.c -lm
 *
 * make x86-flight-check builds and runs it first. */
#include <stdio.h>
#include <string.h>
#include "vehicle_models.h"

static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s\n", what);
        ++failures;
    }
}

static void check_count(long got, long want, const char *what)
{
    if (got != want) {
        printf("FAIL %s: %ld, expected %ld\n", what, got, want);
        ++failures;
    }
}

static agc_word w[2048];

static void reset(void)
{
    memset(w, 0, sizeof w);
    memset(&vm_cfg, 0, sizeof vm_cfg);
}

static long pipa(unsigned k)
{
    return vm_ones(w[VM_PIPAX + k]);
}

/* A signed number as a ones-complement word. */
static agc_word word(long v)
{
    return (agc_word)(v < 0 ? (~(unsigned long)(-v)) & 077777ul : (unsigned long)v);
}

/* A double-precision value at a, scaled 2^scale, as vm_dp reads it. */
static void put_dp(unsigned a, double v, int scale)
{
    long n = lround(v / ldexp(1.0, scale - 28));
    w[a] = word(n / 16384l);
    w[a + 1u] = word(n % 16384l);
}

static void test_words(void)
{
    double a[3] = {0.3, -0.5, 0.8}, b[3], c[3];
    unsigned k;
    check_count(vm_ones(077776u), -1, "ones -1");
    check_count(vm_ones(037777u), 16383, "ones max");
    put_dp(0100, -1234.5, 14);
    check(vm_dp(w, 0100, 14) == -1234.5, "dp round trip");
    check(fabs(vm_angle(020000u) - 3.141592653589793 / 2.0) < 1e-12, "angle 90 degrees");
    vm_smnb(0.3, -0.2, 1.1, a, b);
    vm_nbsm(0.3, -0.2, 1.1, b, c);
    for (k = 0; k < 3u; ++k) check(fabs(a[k] - c[k]) < 1e-12, "nbsm inverts smnb");
    w[VM_PIPAX] = word(037770);
    vm_pipa_add(w, 0, 100);
    check_count(pipa(0), 037777, "PIPA saturates high");
    w[VM_PIPAX + 1u] = word(-5);
    vm_pipa_add(w, 1, -10);
    check_count(pipa(1), -15, "PIPA counts down in ones complement");
}

static void test_engine(void)
{
    vm_engine e;
    double a;
    reset();
    memset(&e, 0, sizeof e);
    vm_cfg.engine_kg = 15000.0;
    w[VM_LM_CDUXD] = 01234u;
    vm_engine_step(100, &e, w);
    check(!e.on && pipa(0) == 0, "engine off without ENGONFLG");
    w[VM_FLAGWRD5] = VM_ENGONBIT;
    vm_engine_step(100, &e, w);
    check(e.on && e.kg == 15000.0 && e.bits == 373.0, "engine lights at 10%");
    check_count(w[VM_CDUX], 01234, "CDUX takes CDUXD");
    w[VM_LM_CDUXD] = 0u;
    vm_engine_step(200, &e, w);
    /* 1 s at 373 bits of 12.53 N along body X, CDUs zero: +X only. */
    a = 373.0 * 12.53 / 15000.0;
    check_count(pipa(0), (long)(a * 100.0), "engine 1 s along X");
    check_count(pipa(1), 0, "engine nothing on Y");
    check_count(pipa(2), 0, "engine nothing on Z");
    check(fabs(e.kg - (15000.0 - 373.0 * 12.53 / 2955.9)) < 1e-9, "engine burns at DPSVEX");
    /* A throttle command: PSEUDO55 taken when TTHROT changes, held to FMAXPOS. */
    w[VM_PSEUDO55] = word(5000);
    w[VM_TTHROT] = 1u;
    vm_engine_step(201, &e, w);
    check(e.cmd == 3467.0 && e.bits == 3467.0, "throttle up to FMAXPOS");
    w[VM_PSEUDO55] = word(-10000);
    w[VM_TTHROT] = 2u;
    vm_engine_step(202, &e, w);
    check(e.cmd == 373.0, "throttle down to 10%");
    /* Pitch 90 degrees (CDUY 1/4 turn): thrust along -Z. */
    memset(&e, 0, sizeof e);
    memset(w + VM_PIPAX, 0, 3 * sizeof w[0]);
    w[VM_LM_CDUXD + 1u] = 020000u;
    vm_engine_step(0, &e, w);
    vm_engine_step(100, &e, w);
    check_count(pipa(0), 0, "pitched engine nothing on X");
    check_count(pipa(2), -(long)(a * 100.0), "pitched engine along -Z");
}

static void test_engine_lag_and_stage(void)
{
    vm_engine e;
    reset();
    memset(&e, 0, sizeof e);
    vm_cfg.engine_kg = 15000.0;
    vm_cfg.engine_lag = 0.2;
    w[VM_FLAGWRD5] = VM_ENGONBIT;
    vm_engine_step(0, &e, w);
    w[VM_PSEUDO55] = word(3000);
    w[VM_TTHROT] = 1u;
    vm_engine_step(1, &e, w);
    check(e.cmd == 3373.0 && e.bits < 400.0, "lagged throttle starts slow");
    vm_engine_step(200, &e, w);
    check(e.bits > 3000.0 && e.bits < 3373.0, "lagged throttle follows");
    /* Abort stage: the ascent engine from FLIGHT_STAGE. */
    reset();
    memset(&e, 0, sizeof e);
    vm_cfg.engine_kg = 15000.0;
    vm_cfg.ascent_kg = 4888.0;
    vm_cfg.stage_cs = 50;
    w[VM_FLAGWRD5] = VM_ENGONBIT;
    vm_engine_step(0, &e, w);
    vm_engine_step(50, &e, w);
    check(e.lifted && e.kg < 4888.0, "abort stage on the ascent engine");
    vm_engine_step(150, &e, w);
    /* Half a second at stage, then a second, each at the mass it began with. */
    {
        double c = 15569.0 / 4888.0 * 0.5 * 100.0;
        c += 15569.0 / (4888.0 - 15569.0 / 3050.0 * 0.5) * 100.0;
        check_count(pipa(0), (long)c, "ascent engine thrust");
    }
}

static void test_touchdown(void)
{
    vm_engine e;
    unsigned k;
    reset();
    memset(&e, 0, sizeof e);
    vm_cfg.engine_kg = 15000.0;
    vm_cfg.touchdown_cs = 1000;
    /* Velocity over the surface: V (03526, m/cs at 2^7) 1 m/s along X. */
    put_dp(03526, 0.01, 7);
    w[VM_FLAGWRD5] = VM_ENGONBIT;
    vm_engine_step(1000, &e, w);
    check(e.landed && !e.on, "touchdown: no thrust, contact");
    check(pipa(0) < 0, "contact stops the LM");
    /* On the surface: gravity up along RN through REFSMMAT (identity). */
    for (k = 0; k < 3u; ++k) w[VM_PIPAX + k] = 0u;
    /* RN (01220, metres at 2^29) on X at the lunar radius; REFSMMAT
     * (01733, at 2^1) the identity. */
    put_dp(01220, 1738000.0, 29);
    for (k = 0; k < 3u; ++k) put_dp(01733u + 6u * k + 2u * k, 1.0, 1);
    vm_engine_step(1100, &e, w);
    check(pipa(0) > 150 && pipa(0) < 170, "lunar gravity 1.62 m/s^2 for 1 s");
    check_count(pipa(1), 0, "surface reaction only along RN");
}

static void test_rcs(void)
{
    vm_rcs r;
    reset();
    memset(&r, 0, sizeof r);
    vm_cfg.rcs_kg = 444.8;
    vm_rcs_step(100, &r, w, 02u, 0u); /* one +X jet from now */
    check_count(pipa(0), 0, "RCS first boundary only notes the jets");
    vm_rcs_step(200, &r, w, 0u, 0u);
    check_count(pipa(0), 100, "one 444.8 N jet on 444.8 kg for 1 s: 1 m/s");
    vm_rcs_step(300, &r, w, 0u, 0u);
    check_count(pipa(0), 100, "RCS off");
    vm_rcs_step(400, &r, w, 0u, 0011u); /* +Z jets */
    vm_rcs_step(500, &r, w, 0u, 0u);
    check_count(pipa(2), 200, "two +Z jets");
}

static void test_sps(void)
{
    vm_sps s;
    double ig = -0.0375245789, mg = 0.0165806279, a = 91188.0 / 30000.0;
    reset();
    memset(&s, 0, sizeof s);
    vm_cfg.sps_kg = 30000.0;
    w[VM_CM_CDUXD] = 01111u;
    w[VM_CM_CDUXD + 2u] = 02222u;
    w[VM_CM_CDUXD + 4u] = 03333u;
    vm_sps_step(0, &s, w);
    check(w[VM_CDUX] == 01111u && w[VM_CDUY] == 02222u && w[VM_CDUZ] == 03333u,
          "SPS holds the RCS DAP's desired attitude");
    /* Desired attitude zero, then the engine lights. */
    w[VM_CM_CDUXD + 2u] = w[VM_CM_CDUXD + 4u] = 0u;
    w[VM_FLAGWRD5] = VM_ENGONBIT;
    vm_sps_step(100, &s, w);
    w[VM_CM_CDUXD + 2u] = 07777u;
    vm_sps_step(200, &s, w);
    check_count(w[VM_CDUY], 0, "after ignition the attitude holds");
    check_count(pipa(0), (long)(a * cos(ig) * cos(mg) * 100.0 / 5.85), "SPS along X");
    check_count(pipa(2), (long)(-a * sin(ig) * cos(mg) * 100.0 / 5.85), "SPS PBIAS on Z");
}

static void test_aero(void)
{
    vm_aero e;
    reset();
    memset(&e, 0, sizeof e);
    vm_cfg.aero_b = 355.0;
    w[VM_P63FLAG] = 1u;
    w[VM_THETAD] = 01u;
    w[VM_THETAD + 1u] = 02u;
    w[VM_THETAD + 2u] = 03u;
    vm_aero_step(0, &e, w);
    check(w[VM_CDUX] == 1u && w[VM_CDUY] == 2u && w[VM_CDUZ] == 3u, "P62: the CDUs take THETAD");
    /* AVEGFLAG set, 60 km up, 7.5 km/s along Y: drag along -Y. */
    w[VM_AVEGWORD] = VM_AVEGBIT;
    put_dp(VM_RN, 6433338.0, 29);
    put_dp(VM_VN + 2u, 75.0, 7); /* 7.5 km/s in m/cs */
    {
        unsigned k;
        for (k = 0; k < 3u; ++k) put_dp(VM_REFSMMAT + 6u * k + 2u * k, 1.0, 1);
    }
    vm_aero_step(100, &e, w);
    check(pipa(1) < 0, "drag against the velocity");
    check(pipa(0) > 0, "lift up, roll 0");
    /* Above 140 km: nothing. */
    memset(&e, 0, sizeof e);
    memset(w + VM_PIPAX, 0, 3 * sizeof w[0]);
    put_dp(VM_RN, 6573338.0, 29);
    vm_aero_step(100, &e, w);
    check(pipa(0) == 0 && pipa(1) == 0, "no atmosphere above 140 km");
}

static void test_imu(void)
{
    reset();
    vm_cfg.imu = 1;
    w[VM_CDUX] = 07u;
    w[VM_THETAD] = 05u;
    vm_imu_step(w, 0u);
    check_count(w[VM_CDUX], 07, "IMU idle moves nothing");
    vm_imu_step(w, 020u);
    check_count(w[VM_CDUX], 0, "zero ICDUs");
    vm_imu_step(w, 050u);
    check_count(w[VM_CDUX], 05, "coarse align to THETAD");
}

static void test_saturn(void)
{
    vm_saturn s;
    agc_word c30 = 037377u;
    double v = 0.0;
    unsigned long c;
    reset();
    memset(&s, 0, sizeof s);
    vm_cfg.saturn_lo = 1000;
    vm_saturn_step(999, &s, w, &c30);
    check(c30 == 037377u && !s.lo, "no liftoff before FLIGHT_SATURN");
    vm_saturn_step(1000, &s, w, &c30);
    check(c30 == 037357u, "liftoff clears channel 30 bit 5");
    vm_saturn_step(1100, &s, w, &c30);
    for (c = 0; c < 100; ++c) v += vm_saturn_acc(((double)c + 0.5) * 0.01) * 0.01;
    check_count(pipa(0), 0, "vertical for the first 12 s");
    check_count(pipa(2), -(long)(v * 100.0 / 5.85), "S-IC thrust up (-Z)");
    check(fabs(vm_saturn_acc(10.0) - 35.1e6 / (2960e3 - 13.0e3 * 10.0)) < 1e-9,
          "S-IC acceleration");
    check(vm_saturn_acc(162.0) == 0.0 && vm_saturn_acc(830.0) == 0.0, "staging gaps and cutoff");
    check(fabs(vm_saturn_pitch(161.6) - 52.2 * 3.141592653589793 / 180.0) < 1e-12,
          "pitch 52.2 degrees at S-IC cutoff");
    /* P11 zeroes the clock: time counts from liftoff again. */
    vm_saturn_step(50, &s, w, &c30);
    check(s.lo_cs == 0 && s.t == 50, "clock zeroed at liftoff");
}

static void test_sivb(void)
{
    vm_sivb s;
    double v = 0.0;
    unsigned long c;
    reset();
    memset(&s, 0, sizeof s);
    vm_cfg.sivb_tig = 1000;
    vm_cfg.sivb_u[1] = 1.0;
    vm_sivb_step(900, &s, w);
    check(pipa(1) == 0, "no TLI before ignition");
    vm_sivb_step(1100, &s, w);
    for (c = 0; c < 100; ++c) v += 0.9e6 / (140e3 - 215.0 * (((double)c + 0.5) * 0.01)) * 0.01;
    check_count(pipa(1), (long)(v * 100.0 / 5.85), "TLI along its vector");
    check(pipa(0) == 0 && pipa(2) == 0, "TLI nothing off its vector");
    vm_sivb_step(1000 + 34700 + 500, &s, w);
    v = (double)pipa(1);
    vm_sivb_step(1000 + 34700 + 1000, &s, w);
    check(pipa(1) == (long)v, "TLI cutoff after 347 s");
}

int main(void)
{
    test_words();
    test_engine();
    test_engine_lag_and_stage();
    test_touchdown();
    test_rcs();
    test_sps();
    test_aero();
    test_imu();
    test_saturn();
    test_sivb();
    if (failures) {
        printf("vehicle models: %d failed\n", failures);
        return 1;
    }
    printf("vehicle models: all checks pass\n");
    return 0;
}
