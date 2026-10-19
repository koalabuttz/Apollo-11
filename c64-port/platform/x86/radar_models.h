/* SPDX-License-Identifier: MIT */
#ifndef RADAR_MODELS_H
#define RADAR_MODELS_H
/* The rendezvous radar the AGC reads, outside the AGC: on the LM the
 * rendezvous radar locked on the CSM, on the CM the VHF ranging link to the
 * LM and the crew's sextant. Both fly the two vehicles' true orbits as
 * Kepler conics about the moon.
 *
 * This is the model this port wrote for its flight scenarios in the host
 * differential (tools/flight_radar.h, docs/FLIGHT-STORE.md), with the same
 * arithmetic, so it gives the same words (the LM's conic is computed before
 * the CDU steps rather than after; neither reads the other); it contains no
 * Virtual AGC code and links nothing. The host differential still runs its
 * own copy. platform/x86/test_radar_models.c checks it.
 *
 * FLIGHT_RR=CS:LM:CSM, each of LM and CSM six numbers (rx,ry,rz,vx,vy,vz:
 * metres and metres per second, basic reference coordinates, moon centred)
 * at AGC centisecond CS. FLIGHT_RR_DROP=T0-T1[,T0-T1...] (centiseconds):
 * data-good dropouts, from the first sample point at or after T0 to the
 * last before T1.
 *
 * At each sample point rm_rr_step runs:
 * - Attitude: CDUX-Z take the DAP's desired angles CDUXD-ZD (perfect
 *   attitude control, as the engine model does).
 * - LM: channel 12 bit 1 (zero RR CDU) zeroes CDUT and CDUS; channel 14
 *   bits 12 and 11 (RR drive) empty CDUTCMD and CDUSCMD. With channel 12
 *   bit 2 (error counter enable) or bit 14 (auto tracker) on, CDUT and
 *   CDUS take the true line of sight to the CSM in nav base axes (LEM
 *   GEOMETRY RRNB's mode 1). Channel 33 bit 4 (data good, active low) is
 *   low while the tracker is on, the CSM is within 400 nautical miles and
 *   no dropout is on.
 * - CM: CDUS and CDUT hold the sextant's line of sight to the LM (the shaft
 *   axis 32.523 degrees from nav base Z toward X, the trunnion less
 *   SXTLOGIC's 19.775 degree bias, four CDU counts to one). Channel 33
 *   bit 2 (VHF data good) is low except in a dropout.
 * - A read: when channel 13 bit 4 (activity) comes on, RNRAD takes the
 *   reading the select bits (channel 13 bits 1-3) name: on the LM 1 the
 *   range (high scale: channel 33 bit 3 stays high; counts of 2.859024 x 8
 *   m) and 2 the range rate (biased 17000, counts of 0.19135344 m/s); on
 *   the CM 1 the VHF range in 0.01 nautical miles. The read can start between sample
 *   points, so rm_rr_step also runs after each routine while no read is
 *   being answered (fs_after_routine). The radar cycle itself (channel 13
 *   bit 4 reset and RADARUPT nine TIME5 counts on) is the resident
 *   timer's (rk_timer_phase). */
#include <math.h>
#include <stdio.h>
#include "agc_word.h"

#define RM_FN static __attribute__((unused))

/* Up to eight dropout windows [T0, T1) in centiseconds. */
typedef struct {
    unsigned n;
    unsigned long t[8][2];
} rm_drops;

typedef struct {
    int on;
    unsigned long t0;     /* the centisecond of the states below */
    double lm[6], csm[6]; /* m, m/s, moon centred reference axes */
    rm_drops drops;
} rm_rr_config;

/* One radar beside one AGC: whether a read is being answered. */
typedef struct {
    int active;
} rm_rr;

enum {
    RM_CDUX = 032,
    RM_CDUT = 035,
    RM_CDUS = 036,
    RM_RNRAD = 046,
    RM_CDUTCMD = 053,
    RM_LM_REFSMMAT = 01733,
    RM_CM_REFSMMAT = 01735,
    RM_LM_CDUXD = 03235,
    RM_CM_CDUXD = 03246
};

#define RM_MOON_MU 4.9028e12 /* m^3/s^2 */
#define RM_TWO_PI  6.283185307179586

/* FLIGHT_RR_DROP's list; 0 on a bad one. */
RM_FN int rm_drops_parse(rm_drops *d, const char *t)
{
    while (t && *t) {
        int n = 0;
        if (d->n >= 8u || sscanf(t, "%lu-%lu%n", &d->t[d->n][0], &d->t[d->n][1], &n) < 2 || !n ||
            d->t[d->n][1] <= d->t[d->n][0])
            return 0;
        ++d->n;
        t += n;
        if (*t == ',')
            ++t;
        else if (*t)
            return 0;
    }
    return 1;
}

RM_FN int rm_in_drop(const rm_drops *d, unsigned long t)
{
    unsigned i;
    for (i = 0; i < d->n; ++i)
        if (t >= d->t[i][0] && t < d->t[i][1]) return 1;
    return 0;
}

/* FLIGHT_RR's value into c (on when given); 0 on a bad one. */
RM_FN int rm_rr_parse(rm_rr_config *c, const char *t)
{
    int n = 0;
    if (!t) return 1;
    if (sscanf(t, "%lu:%lf,%lf,%lf,%lf,%lf,%lf:%lf,%lf,%lf,%lf,%lf,%lf%n", &c->t0, &c->lm[0],
               &c->lm[1], &c->lm[2], &c->lm[3], &c->lm[4], &c->lm[5], &c->csm[0], &c->csm[1],
               &c->csm[2], &c->csm[3], &c->csm[4], &c->csm[5], &n) != 13 ||
        t[n])
        return 0;
    c->on = 1;
    return 1;
}

/* The conic state dt seconds after s (Battin's f and g in the eccentric
 * anomaly difference, solved by Newton's method). */
RM_FN void rm_kepler(const double *s, double dt, double *out)
{
    double r0 = sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]),
           v2 = s[3] * s[3] + s[4] * s[4] + s[5] * s[5], a = 1.0 / (2.0 / r0 - v2 / RM_MOON_MU),
           sg = (s[0] * s[3] + s[1] * s[4] + s[2] * s[5]) / sqrt(RM_MOON_MU),
           n = sqrt(RM_MOON_MU / (a * a * a)), sa = sqrt(a), de = n * dt, r, f, g, fd, gd;
    unsigned i;
    for (i = 0; i < 50u; ++i) {
        double fn = de + sg / sa * (1.0 - cos(de)) - (1.0 - r0 / a) * sin(de) - n * dt,
               dfn = 1.0 + sg / sa * sin(de) - (1.0 - r0 / a) * cos(de);
        de -= fn / dfn;
    }
    r = a + (r0 - a) * cos(de) + sg * sa * sin(de);
    f = 1.0 - a / r0 * (1.0 - cos(de));
    g = a * sg / sqrt(RM_MOON_MU) * (1.0 - cos(de)) + r0 * sqrt(a / RM_MOON_MU) * sin(de);
    fd = -sqrt(RM_MOON_MU * a) / (r * r0) * sin(de);
    gd = 1.0 - a / r * (1.0 - cos(de));
    for (i = 0; i < 3u; ++i) {
        out[i] = f * s[i] + g * s[i + 3u];
        out[i + 3u] = fd * s[i] + gd * s[i + 3u];
    }
}

RM_FN long rm_ones(agc_word v)
{
    return (v & 040000u) ? -(long)((~v) & 037777u) : (long)v;
}
/* A double-precision pair at a, scaled 2^scale. */
RM_FN double rm_dp(const agc_word *w, unsigned a, int scale)
{
    return ((double)rm_ones(w[a]) * 16384.0 + (double)rm_ones(w[a + 1u])) * ldexp(1.0, scale - 28);
}
RM_FN double rm_cdu(agc_word v)
{
    return (double)(v & 077777u) * (RM_TWO_PI / 32768.0);
}
RM_FN agc_word rm_angle(double a)
{
    long c = lround(a / RM_TWO_PI * 32768.0);
    return (agc_word)((unsigned long)c & 077777ul);
}

/* The unit line of sight u (reference axes) in nav base axes nb: into
 * stable-member axes through REFSMMAT at refsmmat, then through the IMU
 * CDUs (inner Y, middle Z, outer X). */
RM_FN void rm_los_nb(const agc_word *w, unsigned refsmmat, const double *rel, double range,
                     double *nb)
{
    double sm[3], ig, mg, og, x1, y1, z1, x2, y2;
    unsigned j, k;
    for (j = 0; j < 3u; ++j) {
        sm[j] = 0.0;
        for (k = 0; k < 3u; ++k) sm[j] += rm_dp(w, refsmmat + 6u * j + 2u * k, 1) * rel[k] / range;
    }
    ig = rm_cdu(w[RM_CDUX + 1u]);
    mg = rm_cdu(w[RM_CDUX + 2u]);
    og = rm_cdu(w[RM_CDUX]);
    x1 = sm[0] * cos(ig) - sm[2] * sin(ig);
    z1 = sm[0] * sin(ig) + sm[2] * cos(ig);
    y1 = sm[1];
    x2 = x1 * cos(mg) + y1 * sin(mg);
    y2 = -x1 * sin(mg) + y1 * cos(mg);
    nb[0] = x2;
    nb[1] = y2 * cos(og) + z1 * sin(og);
    nb[2] = -y2 * sin(og) + z1 * cos(og);
}

/* The channel 33 bits the radar owns while on: a FLIGHT_CHAN write to
 * channel 33 leaves them to the model. */
RM_FN agc_word rm_rr_ch33_bits(const rm_rr_config *c)
{
#if RK_LM
    return c->on ? 010u : 0u;
#else
    return c->on ? 02u : 0u;
#endif
}

/* The model at AGC centisecond t: w the erasable image, ch12-ch14 and
 * ch33 the channels. */
RM_FN void rm_rr_step(const rm_rr_config *c, unsigned long t, rm_rr *e, agc_word *w, agc_word ch12,
                      agc_word ch13, agc_word ch14, agc_word *ch33)
{
    double l[6], v[6], rel[6], nb[3], range;
    unsigned k;
    if (!c->on) return;
    rm_kepler(c->lm, ((double)t - (double)c->t0) * 0.01, l);
    rm_kepler(c->csm, ((double)t - (double)c->t0) * 0.01, v);
#if RK_LM
    {
        double rdot;
        for (k = 0; k < 3u; ++k) w[RM_CDUX + k] = w[RM_LM_CDUXD + k];
        if (ch12 & 1u) w[RM_CDUT] = w[RM_CDUS] = 0u;
        for (k = 0; k < 2u; ++k)
            if (ch14 & (04000u >> k)) w[RM_CDUTCMD + k] = 0u;
        for (k = 0; k < 6u; ++k) rel[k] = v[k] - l[k]; /* the CSM from the LM */
        range = sqrt(rel[0] * rel[0] + rel[1] * rel[1] + rel[2] * rel[2]);
        rdot = (rel[0] * rel[3] + rel[1] * rel[4] + rel[2] * rel[5]) / range;
        if (ch12 & 020002u) {
            rm_los_nb(w, RM_LM_REFSMMAT, rel, range, nb);
            w[RM_CDUT] = rm_angle(asin(-nb[1]));
            w[RM_CDUS] = rm_angle(atan2(nb[0], nb[2]));
        }
        if ((ch12 & 020000u) && range < 400.0 * 1852.0 && !rm_in_drop(&c->drops, t))
            *ch33 &= (agc_word)~010u;
        else
            *ch33 |= 010u;
        *ch33 |= 04u; /* range on high scale */
        if (!(ch13 & 010u)) {
            e->active = 0;
            return;
        }
        if (e->active) return;
        e->active = 1;
        if ((ch13 & 7u) == 1u)
            w[RM_RNRAD] = (agc_word)lround(range / (2.859024 * 8.0)) & 077777u;
        else if ((ch13 & 7u) == 2u)
            w[RM_RNRAD] = (agc_word)(17000l - lround(rdot / 0.19135344)) & 077777u;
    }
#else
    {
        const double sa = 0.567635; /* the shaft axis from nav base Z, rad */
        double o1, o3, tn;
        (void)ch12;
        (void)ch14;
        for (k = 0; k < 3u; ++k) w[RM_CDUX + k] = w[RM_CM_CDUXD + 2u * k];
        for (k = 0; k < 6u; ++k) rel[k] = l[k] - v[k]; /* the LM from the CSM */
        range = sqrt(rel[0] * rel[0] + rel[1] * rel[1] + rel[2] * rel[2]);
        rm_los_nb(w, RM_CM_REFSMMAT, rel, range, nb);
        o1 = nb[0] * cos(sa) - nb[2] * sin(sa);
        o3 = nb[0] * sin(sa) + nb[2] * cos(sa);
        tn = acos(o3 < -1.0 ? -1.0 : o3 > 1.0 ? 1.0 : o3) - 19.775 * 3.141592653589793 / 180.0;
        w[RM_CDUS] = rm_angle(atan2(nb[1], o1));
        w[RM_CDUT] = rm_angle(4.0 * tn);
        if (rm_in_drop(&c->drops, t))
            *ch33 |= 02u;
        else
            *ch33 &= (agc_word)~02u;
        if (!(ch13 & 010u)) {
            e->active = 0;
            return;
        }
        if (e->active) return;
        e->active = 1;
        if ((ch13 & 7u) == 1u) w[RM_RNRAD] = (agc_word)lround(range / 18.52) & 077777u;
    }
#endif
}
#endif
