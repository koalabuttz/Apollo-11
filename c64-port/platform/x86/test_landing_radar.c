/* SPDX-License-Identifier: MIT */
/* Checks for platform/x86/landing_radar.h on a hand-built LM erasable
 * image. Links nothing but libm.
 *
 *   cc -std=c99 -DRK_LM=1 -Iinclude -Iplatform/x86 platform/x86/test_landing_radar.c -lm
 *
 * make x86-flight-check builds and runs it first. */
#include <stdio.h>
#include <string.h>
#include "landing_radar.h"

static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s\n", what);
        ++failures;
    }
}

static agc_word w[2048];

/* A double precision value at a scale (as lr_dp reads it). */
static void put_dp(unsigned a, double v, int scale)
{
    long n = lround(ldexp(v, 28 - scale)), hi = n / 16384, lo = n - hi * 16384;
    w[a] = (agc_word)(hi < 0 ? (~(unsigned long)(-hi)) & 077777ul : (unsigned long)hi);
    w[a + 1u] = (agc_word)(lo < 0 ? (~(unsigned long)(-lo)) & 077777ul : (unsigned long)lo);
}

static void set_time(unsigned long t)
{
    w[024] = (agc_word)((t >> 14) & 037777u);
    w[025] = (agc_word)(t & 037777u);
}

#define SURFACE 1737400.0

/* The LM 10 km above the surface on stable-member X, at rest, at t. The
 * range beam (CDUs and antenna angles zero) points mostly down -X. */
static void setup(unsigned long t)
{
    unsigned j;
    memset(w, 0, sizeof w);
    set_time(t);
    for (j = 0; j < 3u; ++j) w[LR_REFSMMAT + 8u * j] = 020000u;
    put_dp(LR_RN, SURFACE + 10000.0, 29);
    put_dp(LR_PIPTIME, (double)t, 28);
    put_dp(LR_LAND, SURFACE, 24);
    put_dp(LR_HCALC, 10000.0, 24);
}

static void test_parse(void)
{
    lr_config c;
    memset(&c, 0, sizeof c);
    check(lr_parse(&c, 0, 0, 0) && !c.on && !c.pos2, "nothing given: off");
    check(lr_parse(&c, "60000:150", "500", "100-200") && c.on && c.t0 == 60000ul && c.dh == 150.0 &&
              c.pos2 == 500ul && c.drops.n == 1u,
          "FLIGHT_LR, FLIGHT_LR_POS2, FLIGHT_LR_DROP read");
    memset(&c, 0, sizeof c);
    check(lr_parse(&c, "60000", 0, 0) && c.on && c.dh == 0.0, "no DH: 0");
    memset(&c, 0, sizeof c);
    check(!lr_parse(&c, "60000x", 0, 0), "trailing text refused");
    memset(&c, 0, sizeof c);
    check(!lr_parse(&c, "60000", 0, "200-100"), "empty dropout refused");
    c.on = 1;
    c.pos2 = 1;
    check(lr_ch33_bits(&c) == 0760u, "the models own channel 33 bits 5-9");
}

static void test_fly(void)
{
    const double r = SURFACE + 100000.0, v = sqrt(RM_MOON_MU / r),
                 quarter = 6.283185307179586 * r / v / 4.0;
    lr_state s;
    memset(&s, 0, sizeof s);
    s.r[0] = r;
    s.v[1] = v;
    lr_fly(&s, (unsigned long)lround(quarter * 100.0));
    check(fabs(s.r[0]) < 2000.0 && fabs(s.r[1] - r) < 50.0, "a quarter orbit of gravity alone");
    {
        double u[3] = {-1.0, 0.0, 0.0}, p[3] = {SURFACE + 1000.0, 0.0, 0.0};
        check(fabs(lr_slant(p, u, SURFACE) - 1000.0) < 1e-6, "straight down: the altitude");
        u[0] = 1.0;
        check(lr_slant(p, u, SURFACE) < 0.0, "pointing away: no surface");
    }
}

static void test_radar(void)
{
    lr_config c;
    lr_state s;
    agc_word ch33 = 077777u;
    double b[4][3], slant;
    long expect;
    memset(&c, 0, sizeof c);
    memset(&s, 0, sizeof s);
    c.on = 1;
    c.t0 = 500ul;
    setup(1000ul);
    w[LR_HCALC] = w[LR_HCALC + 1u] = 0u;
    lr_step(&c, &s, 1000ul, 0ul, w, 0u, &ch33);
    check(!s.started && ch33 == 077777u, "no start before descent navigation (HCALC)");
    setup(1000ul);
    lr_step(&c, &s, 400ul, 0ul, w, 0u, &ch33);
    check(!s.started, "not before FLIGHT_LR's time");
    lr_step(&c, &s, 1000ul, 0ul, w, 0u, &ch33);
    check(s.started && fabs(s.r[0] - (SURFACE + 10000.0)) < 1.0 && s.rs == SURFACE,
          "started from the AGC's own state");
    check(!(ch33 & 0220u) && (ch33 & 0400u), "data good; high scale above 2,481.7 ft");
    lr_beams(w, ch33, b);
    slant = lr_slant(s.r, b[3], s.rs);
    expect = lround(slant / LR_HSCAL / 5.0);
    lr_step(&c, &s, 1000ul, 0ul, w, 017u, &ch33);
    check(s.active && labs((long)w[LR_RNRAD] - expect) <= 1,
          "a range read: the slant range, high scale");
    w[LR_RNRAD] = 0u;
    lr_step(&c, &s, 1000ul, 0ul, w, 017u, &ch33);
    check(w[LR_RNRAD] == 0u, "one answer a read");
    lr_step(&c, &s, 1000ul, 0ul, w, 0u, &ch33);
    check(!s.active, "the read ends with the activity bit");
    lr_step(&c, &s, 1000ul, 0ul, w, 014u, &ch33);
    check(w[LR_RNRAD] == 12288u, "velocity X at rest: the bias");
    {
        double dv[3] = {0.0, 0.0, -10.0};
        lr_dv(&s, dv, 1000ul, 0.0);
        check(fabs(s.v[2] + 10.0) < 1e-9, "the engine's velocity change goes into the true state");
    }
    c.drops.n = 1u;
    c.drops.t[0][0] = 900ul;
    c.drops.t[0][1] = 1100ul;
    lr_step(&c, &s, 1000ul, 0ul, w, 0u, &ch33);
    check((ch33 & 0220u) == 0220u, "no data good in a dropout");
    c.drops.n = 0u;
    lr_step(&c, &s, 1000ul, 1000ul, w, 0u, &ch33);
    check((ch33 & 0220u) == 0220u && s.landed, "no data good after touchdown");
}

static void test_pos2(void)
{
    lr_config c;
    lr_state s;
    agc_word ch33 = 077677u; /* position 1: bit 6 low, bit 7 high */
    memset(&c, 0, sizeof c);
    memset(&s, 0, sizeof s);
    c.pos2 = 500ul;
    lr_pos2_step(&c, 50ul, &s, 0u, &ch33);
    check(s.pos2_at == 0ul, "no command, no move");
    lr_pos2_step(&c, 100ul, &s, 010000u, &ch33);
    lr_pos2_step(&c, 599ul, &s, 0u, &ch33);
    check(ch33 == 077677u, "still in position 1 before CS");
    lr_pos2_step(&c, 600ul, &s, 0u, &ch33);
    check(!(ch33 & 0100u) && (ch33 & 040u), "in position 2 CS after the command");
}

int main(void)
{
    test_parse();
    test_fly();
    test_radar();
    test_pos2();
    if (failures) {
        printf("landing radar: %d failed\n", failures);
        return 1;
    }
    printf("landing radar: all checks pass\n");
    return 0;
}
