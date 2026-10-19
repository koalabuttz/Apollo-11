/* SPDX-License-Identifier: MIT */
#ifndef LANDING_RADAR_H
#define LANDING_RADAR_H
/* The LM landing radar the AGC reads, outside the AGC. This is the model
 * this port wrote for its flight scenarios in the host differential
 * (tools/flight_lr.h), with the same arithmetic, so it gives the same
 * words; it contains no Virtual AGC code and links nothing. The host
 * differential still runs its own copy. platform/x86/test_landing_radar.c
 * checks it. The CM has no landing radar: there the functions do nothing.
 *
 * FLIGHT_LR_POS2=CS: the antenna. Channel 12 bit 13 (LRPOS2's command)
 * moves it to position 2 CS centiseconds later: channel 33 bit 7 (in
 * position 2, active low) goes to 0 and bit 6 (in position 1) to 1. Start
 * the scenario in position 1 (FLIGHT_CH33 bit 6 0, bit 7 1).
 *
 * FLIGHT_LR=CS[:DH]: the radar itself, powered from AGC centisecond CS, so
 * P63-P66 take LR altitude and velocity (SERVICER POSUPDAT, VELUPDAT;
 * after V57 the updates move the state vector). DH (metres, default 0)
 * raises the true surface above the AGC's landing-site radius /LAND/, so
 * the AGC starts DH too high and the updates have that error to remove.
 * FLIGHT_LR_DROP=T0-T1[,T0-T1...]: both data-good bits off in those
 * windows.
 *
 * The true trajectory. At the first sample point from CS with the AGC's
 * altitude HCALC set, the model takes the AGC's own state RN, VN
 * (reference axes at PIPTIME, into stable-member axes through REFSMMAT)
 * plus the PIPAs' velocity since PIPTIME, and from then on flies it
 * itself: central lunar gravity plus the velocity changes the engine and
 * RCS models count into the PIPAs (lr_dv, at each interval's midpoint).
 * Without updates the AGC's navigation stays within a few metres of it.
 * Attitude is perfect: the CDUs are the true attitude.
 *
 * The readings, at a read's start (channel 13 bit 4 rising with bit 3, the
 * LR select): bits 1-2 pick velocity X (0), Y (1), Z (2) or range (3),
 * INITREAD's LRVELX/Y/Z and LRALT.
 * - Beams: SERVICER SETPOS's geometry. The antenna's X and Y axes and the
 *   range beam HBEAMANT (-.4687018041, 0, -.1741224271, a half unit) go
 *   into nav base through the antenna angles (LRALPHA/LRBETA1 in position
 *   1, LRALPHA2/LRBETA2 in position 2: channel 33 bit 7 clear), velocity
 *   Z = X cross Y, then into stable-member axes through the CDUs.
 * - Range: the slant distance along the range beam to the true surface (a
 *   sphere), in 1.079 ft counts, less SERVICER's Doppler correction (V
 *   along the beam times RADSKAL, times SKALSKAL on low scale), so
 *   POSUPDAT's slant range is the true one. High scale (channel 33 bit 9
 *   set) above HISCALIM's 2,481.7 ft, 5.395 ft counts.
 * - Velocity: the velocity over the surface (true V plus the AGC's DELVS,
 *   the lunar-rotation term SERVICER adds) along the beam, in VXSCAL,
 *   VYSCAL, VZSCAL counts (-.644, 1.212, .8668 ft/s), biased 12288
 *   (LVELBIAS).
 * - Data good, channel 33 bit 5 (range) and bit 8 (velocity), active low,
 *   set at every sample point: on while the range beam meets the surface
 *   within 50,000 ft and the LM is more than 10 ft up, outside the dropout
 *   windows and before touchdown (FLIGHT_TOUCHDOWN).
 * The radar cycle itself (the activity bit reset and RADARUPT) is the
 * resident timer's, as for the rendezvous radar. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "agc_word.h"
#include "radar_models.h"

#define LR_FN static __attribute__((unused))

typedef struct {
    int on;             /* FLIGHT_LR given */
    unsigned long t0;   /* powered from this centisecond */
    double dh;          /* the true surface above /LAND/, metres */
    unsigned long pos2; /* FLIGHT_LR_POS2: centiseconds to position 2; 0 none */
    rm_drops drops;     /* FLIGHT_LR_DROP */
    int trace;          /* FLIGHT_LR_TRACE: print each reading */
} lr_config;

typedef struct {
    unsigned long pos2_at; /* 0 not commanded, 1 there, else when */
    int started, active, landed;
    unsigned long t;       /* the time of r, v (cs) */
    double rs, r[3], v[3]; /* the surface radius; the true state (m, m/s) */
} lr_state;

enum {
    LR_CDUX = 032,
    LR_PIPAX = 037,
    LR_RNRAD = 046,
    LR_RN = 01220,
    LR_VN = 01226,
    LR_PIPTIME = 01234,
    LR_RADSKAL = 01354,
    LR_SKALSKAL = 01356,
    LR_REFSMMAT = 01733,
    LR_LAND = 02332,
    LR_ALPHA = 02522,
    LR_HCALC = 03534,
    LR_DELVS = 03733
};

#define LR_FT    0.3048
#define LR_HSCAL 0.3288792 /* metres a low-scale count (HSCAL) */

/* FLIGHT_LR's value (on when given), FLIGHT_LR_POS2 and FLIGHT_LR_DROP into
 * c; 0 on a bad one. */
LR_FN int lr_parse(lr_config *c, const char *lr, const char *pos2, const char *drop)
{
    int n = 0;
    if (pos2) c->pos2 = strtoul(pos2, 0, 10);
    if (!rm_drops_parse(&c->drops, drop)) return 0;
    if (!lr) return 1;
    if (sscanf(lr, "%lu%n", &c->t0, &n) != 1) return 0;
    lr += n;
    if (*lr == ':') {
        if (sscanf(lr + 1, "%lf%n", &c->dh, &n) != 1) return 0;
        lr += 1 + n;
    }
    if (*lr) return 0;
    c->on = 1;
    return 1;
}

#if RK_LM
/* The channel 33 bits the models own (a FLIGHT_CHAN write leaves them). */
LR_FN agc_word lr_ch33_bits(const lr_config *c)
{
    return (agc_word)((c->on ? 0720u : 0u) | (c->pos2 ? 0140u : 0u));
}

/* The antenna: channel 12 bit 13 moves it to position 2 pos2 later. */
LR_FN void lr_pos2_step(const lr_config *c, unsigned long t, lr_state *s, agc_word ch12,
                        agc_word *ch33)
{
    if (!c->pos2 || s->pos2_at == 1ul) return;
    if (!s->pos2_at) {
        if (ch12 & 010000u) s->pos2_at = t + c->pos2;
        return;
    }
    if (t < s->pos2_at) return;
    s->pos2_at = 1ul;
    *ch33 = (agc_word)((*ch33 & ~0100u) | 040u);
}

static long lr_ones(agc_word v)
{
    return (v & 040000u) ? -(long)((~v) & 037777u) : (long)v;
}
static double lr_dp(const agc_word *w, unsigned a, int scale)
{
    return ((double)lr_ones(w[a]) * 16384.0 + (double)lr_ones(w[a + 1u])) * ldexp(1.0, scale - 28);
}
static double lr_angle(agc_word v)
{
    return (double)(v & 077777u) * (6.283185307179586 / 32768.0);
}
/* Centiseconds from PIPTIME to now on the AGC clock (TIME2, TIME1). */
static double lr_since_pip(const agc_word *w)
{
    double now = (double)(w[024] & 037777u) * 16384.0 + (double)(w[025] & 037777u),
           pip = (double)lr_ones(w[LR_PIPTIME]) * 16384.0 + (double)lr_ones(w[LR_PIPTIME + 1u]),
           d = now - pip;
    while (d < 0.0) d += 268435456.0;
    return d;
}

/* Stable member to nav base through gimbal angles inner ig (Y), middle mg
 * (Z), outer og (X), as *SMNB*; lr_nbsm is its inverse. */
LR_FN void lr_smnb(double ig, double mg, double og, const double *a, double *b)
{
    double x1 = a[0] * cos(ig) - a[2] * sin(ig), z1 = a[0] * sin(ig) + a[2] * cos(ig), y1 = a[1],
           x2 = x1 * cos(mg) + y1 * sin(mg), y2 = -x1 * sin(mg) + y1 * cos(mg);
    b[0] = x2;
    b[1] = y2 * cos(og) + z1 * sin(og);
    b[2] = -y2 * sin(og) + z1 * cos(og);
}
LR_FN void lr_nbsm(double ig, double mg, double og, const double *b, double *a)
{
    double y2 = b[1] * cos(og) - b[2] * sin(og), z1 = b[1] * sin(og) + b[2] * cos(og),
           x1 = b[0] * cos(mg) - y2 * sin(mg), y1 = b[0] * sin(mg) + y2 * cos(mg);
    a[0] = x1 * cos(ig) + z1 * sin(ig);
    a[1] = y1;
    a[2] = -x1 * sin(ig) + z1 * cos(ig);
}

/* Gravity only, from s->t to t (cs), in steps of at most 10 cs. */
LR_FN void lr_fly(lr_state *s, unsigned long t)
{
    while (s->t < t) {
        unsigned long n = t - s->t > 10ul ? 10ul : t - s->t;
        double dt = (double)n * 0.01, rm;
        unsigned k;
        rm = sqrt(s->r[0] * s->r[0] + s->r[1] * s->r[1] + s->r[2] * s->r[2]);
        for (k = 0; k < 3u; ++k) {
            s->v[k] -= RM_MOON_MU / (rm * rm * rm) * s->r[k] * dt * 0.5;
            s->r[k] += s->v[k] * dt;
        }
        rm = sqrt(s->r[0] * s->r[0] + s->r[1] * s->r[1] + s->r[2] * s->r[2]);
        for (k = 0; k < 3u; ++k) s->v[k] -= RM_MOON_MU / (rm * rm * rm) * s->r[k] * dt * 0.5;
        s->t += n;
    }
}

/* The engine or jets' velocity change dv (m/s, stable-member axes) over the
 * dt seconds to AGC centisecond t, taken at the interval's midpoint. */
LR_FN void lr_dv(lr_state *s, const double *dv, unsigned long t, double dt)
{
    unsigned long mid = t - (unsigned long)lround(dt * 50.0);
    unsigned k;
    if (!s->started) return;
    if (mid > s->t) lr_fly(s, mid);
    for (k = 0; k < 3u; ++k) s->v[k] += dv[k];
}

/* The four beams in stable-member axes: 0 velocity X, 1 Y, 2 Z, 3 range. */
LR_FN void lr_beams(const agc_word *w, agc_word ch33, double b[4][3])
{
    static const double ux[3] = {1.0, 0.0, 0.0}, uy[3] = {0.0, 1.0, 0.0},
                        hb[3] = {-.4687018041 * 2.0, 0.0, -.1741224271 * 2.0};
    unsigned p = (ch33 & 0100u) ? 0u : 2u, k;
    double nb[4][3], ig, mg, og, al = lr_angle(w[LR_ALPHA + p]),
                                 be = lr_angle(w[LR_ALPHA + p + 1u]);
    lr_smnb(be, 0.0, al, ux, nb[0]);
    lr_smnb(be, 0.0, al, uy, nb[1]);
    lr_smnb(be, 0.0, al, hb, nb[3]);
    nb[2][0] = nb[0][1] * nb[1][2] - nb[0][2] * nb[1][1];
    nb[2][1] = nb[0][2] * nb[1][0] - nb[0][0] * nb[1][2];
    nb[2][2] = nb[0][0] * nb[1][1] - nb[0][1] * nb[1][0];
    ig = lr_angle(w[LR_CDUX + 1u]);
    mg = lr_angle(w[LR_CDUX + 2u]);
    og = lr_angle(w[LR_CDUX]);
    for (k = 0; k < 4u; ++k) lr_nbsm(ig, mg, og, nb[k], b[k]);
}

/* The slant range (m) along unit u from r to the sphere of radius rs, or -1
 * if the beam misses it. */
LR_FN double lr_slant(const double *r, const double *u, double rs)
{
    double ru = r[0] * u[0] + r[1] * u[1] + r[2] * u[2],
           rr = r[0] * r[0] + r[1] * r[1] + r[2] * r[2], d = ru * ru - (rr - rs * rs);
    if (d < 0.0) return -1.0;
    d = -ru - sqrt(d);
    return d > 0.0 ? d : -1.0;
}

/* The true state from the AGC's own, once descent navigation (HCALC) runs. */
static void lr_start(const lr_config *c, lr_state *s, unsigned long t, const agc_word *w)
{
    unsigned j, k;
    double rm = 0.0, rn[3], vn[3];
    if (!w[LR_HCALC] && !w[LR_HCALC + 1u]) return; /* no descent navigation yet */
    for (k = 0; k < 3u; ++k) {
        rn[k] = lr_dp(w, LR_RN + 2u * k, 29);
        vn[k] = lr_dp(w, LR_VN + 2u * k, 7) * 100.0;
    }
    for (j = 0; j < 3u; ++j) {
        s->r[j] = s->v[j] = 0.0;
        for (k = 0; k < 3u; ++k) {
            double m1 = lr_dp(w, LR_REFSMMAT + 6u * j + 2u * k, 1);
            s->r[j] += m1 * rn[k];
            s->v[j] += m1 * vn[k];
        }
        rm += s->r[j] * s->r[j];
    }
    if (rm < 1e12) return;
    for (k = 0; k < 3u; ++k) s->v[k] += (double)lr_ones(w[LR_PIPAX + k]) * 0.01;
    s->t = t - (unsigned long)lr_since_pip(w);
    s->rs = lr_dp(w, LR_LAND, 24) + c->dh;
    s->started = 1;
    lr_fly(s, t);
    if (c->trace) printf("LR start %lu cs: |R| %.1f m, surface %.1f m\n", t, sqrt(rm), s->rs);
}

/* A sample point, or a read's start (channel 13 bit 4 rising with bit 3):
 * the data-good and scale bits, and the reading into RNRAD. touchdown:
 * FLIGHT_TOUCHDOWN (0 none). */
LR_FN void lr_step(const lr_config *c, lr_state *s, unsigned long t, unsigned long touchdown,
                   agc_word *w, agc_word ch13, agc_word *ch33)
{
    double b[4][3], slant, alt, rm, vr[3], n;
    unsigned k, sel;
    if (!c->on || t < c->t0) return;
    if (!s->started) {
        lr_start(c, s, t, w);
        if (!s->started) return;
    }
    if (touchdown && t >= touchdown) s->landed = 1;
    lr_fly(s, t);
    lr_beams(w, *ch33, b);
    rm = sqrt(s->r[0] * s->r[0] + s->r[1] * s->r[1] + s->r[2] * s->r[2]);
    alt = rm - s->rs;
    slant = lr_slant(s->r, b[3], s->rs);
    if (!s->landed && slant > 0.0 && slant < 50000.0 * LR_FT && alt > 10.0 * LR_FT &&
        !rm_in_drop(&c->drops, t))
        *ch33 &= (agc_word)~0220u;
    else
        *ch33 |= 0220u;
    if (slant > 0.0) *ch33 = (agc_word)(slant > 2481.7 * LR_FT ? *ch33 | 0400u : *ch33 & ~0400u);
    if (!(ch13 & 010u)) {
        s->active = 0;
        return;
    }
    if (s->active || !(ch13 & 04u)) return;
    s->active = 1;
    for (k = 0; k < 3u; ++k) vr[k] = s->v[k] + lr_dp(w, LR_DELVS + 2u * k, 5) * 100.0;
    sel = ch13 & 3u;
    if (sel == 3u) {
        int hi = (*ch33 & 0400u) != 0;
        double vd = (vr[0] * b[3][0] + vr[1] * b[3][1] + vr[2] * b[3][2]) * 0.01, d;
        if (slant < 0.0) return;
        /* POSUPDAT: slant = (HMEAS + D) HSCAL, D = (V . HBEAM) RADSKAL, times
         * SKALSKAL on low scale; V in m/cs at 2(7), D in counts at 2(28). */
        d = vd / 128.0 * lr_dp(w, LR_RADSKAL, 0) * 268435456.0;
        if (!hi) d *= (double)lr_ones(w[LR_SKALSKAL]) / 16384.0;
        n = slant / LR_HSCAL - d;
        if (hi) n /= 5.0;
        if (c->trace)
            printf("LR %lu cs: range %.1f m slant, %.1f m up, LGC HCALC %.1f m (%s scale)\n", t,
                   slant, alt, lr_dp(w, LR_HCALC, 24), hi ? "high" : "low");
    } else {
        static const double scale[3] = {-.644 * LR_FT, 1.212 * LR_FT, .8668 * LR_FT};
        double vb = vr[0] * b[sel][0] + vr[1] * b[sel][1] + vr[2] * b[sel][2];
        n = 12288.0 + vb / scale[sel];
        if (c->trace) printf("LR %lu cs: velocity %c %.2f m/s\n", t, "XYZ"[sel], vb);
    }
    w[LR_RNRAD] = (agc_word)((unsigned long)lround(n < 0.0       ? 0.0
                                                   : n > 32767.0 ? 32767.0
                                                                 : n) &
                             077777ul);
}
#else
/* The CM has no landing radar. */
LR_FN agc_word lr_ch33_bits(const lr_config *c)
{
    (void)c;
    return 0u;
}
LR_FN void lr_pos2_step(const lr_config *c, unsigned long t, lr_state *s, agc_word ch12,
                        agc_word *ch33)
{
    (void)c, (void)t, (void)s, (void)ch12, (void)ch33;
}
LR_FN void lr_dv(lr_state *s, const double *dv, unsigned long t, double dt)
{
    (void)s, (void)dv, (void)t, (void)dt;
}
LR_FN void lr_step(const lr_config *c, lr_state *s, unsigned long t, unsigned long touchdown,
                   agc_word *w, agc_word ch13, agc_word *ch33)
{
    (void)c, (void)s, (void)t, (void)touchdown, (void)w, (void)ch13, (void)ch33;
}
#endif
#endif
