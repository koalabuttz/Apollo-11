/* SPDX-License-Identifier: MIT */
/* Checks for platform/x86/radar_models.h on a hand-built erasable image,
 * built once for each mission (-DRK_LM=1, -DRK_CM=1). Links nothing but
 * libm.
 *
 *   cc -std=c99 -DRK_LM=1 -Iinclude -Iplatform/x86 platform/x86/test_radar_models.c -lm
 *
 * make x86-flight-check builds and runs it first, for both missions. */
#include <stdio.h>
#include <string.h>
#include "radar_models.h"

#if RK_LM
#define MISSION "LM"
#else
#define MISSION "CM"
#endif

static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s %s\n", MISSION, what);
        ++failures;
    }
}

static agc_word w[2048];

/* REFSMMAT the identity (scaled 2^1: 1.0 is 0.5 of the word pair). */
static void identity_refsmmat(unsigned at)
{
    unsigned j;
    for (j = 0; j < 3u; ++j) w[at + 8u * j] = 020000u;
}

static void test_parse(void)
{
    rm_rr_config c;
    memset(&c, 0, sizeof c);
    check(rm_rr_parse(&c, 0) && !c.on, "no FLIGHT_RR: off");
    check(rm_rr_parse(&c, "2000:1,2,3,4,5,6:7,8,9,10,11,12") && c.on && c.t0 == 2000ul &&
              c.lm[5] == 6.0 && c.csm[0] == 7.0,
          "FLIGHT_RR read");
    check(!rm_rr_parse(&c, "2000:1,2,3,4,5,6:7,8,9,10,11"), "FLIGHT_RR short refused");
    check(!rm_rr_parse(&c, "2000:1,2,3,4,5,6:7,8,9,10,11,12x"), "FLIGHT_RR trailing refused");
    memset(&c, 0, sizeof c);
    check(rm_drops_parse(&c.drops, "100-200,300-400") && c.drops.n == 2u, "drops read");
    check(rm_in_drop(&c.drops, 100) && rm_in_drop(&c.drops, 199) && !rm_in_drop(&c.drops, 200) &&
              !rm_in_drop(&c.drops, 250),
          "drops are [T0, T1)");
    memset(&c, 0, sizeof c);
    check(!rm_drops_parse(&c.drops, "200-100"), "empty drop refused");
}

static void test_kepler(void)
{
    /* A circular orbit 100 km above the moon comes back after one period. */
    const double r = 1837.4e3, v = sqrt(RM_MOON_MU / r), period = 6.283185307179586 * r / v;
    double s[6] = {r, 0.0, 0.0, 0.0, v, 0.0}, o[6];
    rm_kepler(s, period / 4.0, o);
    check(fabs(o[0]) < 1.0 && fabs(o[1] - r) < 1.0 && fabs(o[3] + v) < 1e-3, "a quarter orbit");
    rm_kepler(s, period, o);
    check(fabs(o[0] - r) < 1.0 && fabs(o[1]) < 1.0 && fabs(o[4] - v) < 1e-3, "a whole orbit");
    rm_kepler(s, 0.0, o);
    check(o[0] == r && o[4] == v, "no time, no motion");
}

/* The LM 10 km behind the CSM on the same circular orbit's tangent: the
 * CSM along stable-member X. */
static void setup(rm_rr_config *c, double range)
{
    const double r = 1837.4e3, v = sqrt(RM_MOON_MU / r);
    memset(w, 0, sizeof w);
    memset(c, 0, sizeof *c);
    c->on = 1;
    c->t0 = 1000ul;
    c->lm[1] = r;
    c->lm[3] = v;
    c->csm[0] = range;
    c->csm[1] = r;
    c->csm[3] = v;
}

#if RK_LM
static void test_lm(void)
{
    rm_rr_config c;
    rm_rr e = {0};
    agc_word ch33 = 077777u;
    setup(&c, 10000.0);
    identity_refsmmat(RM_LM_REFSMMAT);
    w[RM_LM_CDUXD] = 01111u;
    w[RM_LM_CDUXD + 1u] = 02222u;
    w[RM_LM_CDUXD + 2u] = 03333u;
    rm_rr_step(&c, 1000ul, &e, w, 0u, 0u, 0u, &ch33);
    check(w[RM_CDUX] == 01111u && w[RM_CDUX + 1u] == 02222u && w[RM_CDUX + 2u] == 03333u,
          "the CDUs take the DAP's desired angles");
    check((ch33 & 010u) != 0u, "no data good without the tracker");
    w[RM_LM_CDUXD] = w[RM_LM_CDUXD + 1u] = w[RM_LM_CDUXD + 2u] = 0u;
    rm_rr_step(&c, 1000ul, &e, w, 020000u, 0u, 0u, &ch33);
    check(!(ch33 & 010u) && (ch33 & 04u), "data good with the tracker on; high scale");
    check(w[RM_CDUT] == 0u && w[RM_CDUS] == 020000u, "the antenna points along nav base X");
    rm_rr_step(&c, 1000ul, &e, w, 020000u, 011u, 0u, &ch33);
    check(e.active && w[RM_RNRAD] == (agc_word)lround(10000.0 / (2.859024 * 8.0)), "range read");
    w[RM_RNRAD] = 0u;
    rm_rr_step(&c, 1000ul, &e, w, 020000u, 011u, 0u, &ch33);
    check(w[RM_RNRAD] == 0u, "one answer a read");
    rm_rr_step(&c, 1000ul, &e, w, 020000u, 0u, 0u, &ch33);
    rm_rr_step(&c, 1000ul, &e, w, 020000u, 012u, 0u, &ch33);
    check(w[RM_RNRAD] == 17000u, "range rate read: none on the same orbit at t0");
    w[RM_CDUT] = w[RM_CDUS] = 0777u;
    w[RM_CDUTCMD] = w[RM_CDUTCMD + 1u] = 5u;
    rm_rr_step(&c, 1000ul, &e, w, 1u, 0u, 06000u, &ch33);
    check(w[RM_CDUT] == 0u && w[RM_CDUS] == 0u, "zero RR CDU");
    check(w[RM_CDUTCMD] == 0u && w[RM_CDUTCMD + 1u] == 0u, "the drive empties the commands");
    c.drops.n = 1u;
    c.drops.t[0][0] = 900ul;
    c.drops.t[0][1] = 1100ul;
    rm_rr_step(&c, 1000ul, &e, w, 020000u, 0u, 0u, &ch33);
    check((ch33 & 010u) != 0u, "no data good in a dropout");
    setup(&c, 401.0 * 1852.0);
    rm_rr_step(&c, 1000ul, &e, w, 020000u, 0u, 0u, &ch33);
    check((ch33 & 010u) != 0u, "no data good beyond 400 nautical miles");
    check(rm_rr_ch33_bits(&c) == 010u, "the LM radar owns channel 33 bit 4");
}
#else
static void test_cm(void)
{
    rm_rr_config c;
    rm_rr e = {0};
    agc_word ch33 = 077777u;
    setup(&c, -18520.0); /* the LM 10 nautical miles along X from the CSM */
    identity_refsmmat(RM_CM_REFSMMAT);
    w[RM_CM_CDUXD] = 01111u;
    w[RM_CM_CDUXD + 2u] = 02222u;
    w[RM_CM_CDUXD + 4u] = 03333u;
    rm_rr_step(&c, 1000ul, &e, w, 0u, 0u, 0u, &ch33);
    check(w[RM_CDUX] == 01111u && w[RM_CDUX + 1u] == 02222u && w[RM_CDUX + 2u] == 03333u,
          "the CDUs take the DAP's desired angles (two words apart)");
    check(!(ch33 & 02u), "VHF data good");
    w[RM_CM_CDUXD] = w[RM_CM_CDUXD + 2u] = w[RM_CM_CDUXD + 4u] = 0u;
    /* The LM is behind: rel = LM - CSM = (+18520, 0, 0) since CSM sits at -18520. */
    rm_rr_step(&c, 1000ul, &e, w, 0u, 011u, 0u, &ch33);
    check(w[RM_RNRAD] == 1000u, "VHF range in 0.01 nautical miles");
    check(w[RM_CDUS] == 0u, "shaft 0 with the LM in the X-Z plane");
    check(w[RM_CDUT] == rm_angle(4.0 * ((1.5707963267948966 - 0.567635) -
                                        19.775 * 3.141592653589793 / 180.0)),
          "trunnion from the shaft axis less the bias, four counts to one");
    c.drops.n = 1u;
    c.drops.t[0][0] = 900ul;
    c.drops.t[0][1] = 1100ul;
    rm_rr_step(&c, 1000ul, &e, w, 0u, 0u, 0u, &ch33);
    check((ch33 & 02u) != 0u, "no VHF data good in a dropout");
    check(rm_rr_ch33_bits(&c) == 02u, "the CM link owns channel 33 bit 2");
}
#endif

int main(void)
{
    test_parse();
    test_kepler();
#if RK_LM
    test_lm();
#else
    test_cm();
#endif
    if (failures) {
        printf("radar models (%s): %d failed\n", MISSION, failures);
        return 1;
    }
    printf("radar models (%s): all checks pass\n", MISSION);
    return 0;
}
