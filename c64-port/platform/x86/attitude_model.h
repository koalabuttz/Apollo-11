/* SPDX-License-Identifier: MIT */
#ifndef ATTITUDE_MODEL_H
#define ATTITUDE_MODEL_H
/* The spacecraft turning under its RCS jets, for the x86 flight program
 * (docs/X86-FLIGHT.md). Same input and arithmetic as the harness's
 * FLIGHT_ATT (tools/flight_attitude.h), written for the x86 program.
 *
 * FLIGHT_ATT=SCALE: a rigid body. Each jet the DAP fires gives an angular
 * acceleration about one body axis, times SCALE, so 1 is a vehicle exactly
 * as the AGC thinks it is.
 *
 * LM: one jet's acceleration is what the DAP counts on, 1JACC, 1JACCQ and
 * 1JACCR (scaled at pi/4 rad/s/s):
 * - channel 6, the P jets: bits 1, 3, 5 and 7 turn +P, bits 2, 4, 6 and 8 -P;
 * - channel 5, the Q,R jets: bits 3 and 8 turn +U, 4 and 7 -U, 1 and 6 +V,
 *   2 and 5 -V. A U jet gives +1JACCQ and +1JACCR, a V jet -1JACCQ and
 *   +1JACCR.
 * While the descent engine burns, its trim gimbal (vm_gimbal) adds
 * -ACCDOTQ sin p cos r about Q and -ACCDOTR sin r about R per radian of
 * gimbal (the jerk ACCDOTQ or ACCDOTR, scaled at pi/2^7 rad/s^3, over the
 * 0.2 deg/s gimbal rate), p and r the pitch and roll trim angles.
 *
 * CM: 444.8 N a jet at 1.96 m from the roll axis, over the inertia the AGC
 * holds (IXX for roll, IAVG for pitch and yaw, scaled at 2^20 kg m^2):
 * - channel 5: bits 1 and 3 +pitch, 2 and 4 -pitch, 5 and 7 +yaw, 6 and 8
 *   -yaw;
 * - channel 6: bits 1, 3, 5 and 7 +roll, 2, 4, 6 and 8 -roll.
 * While the SPS burns, the SPS model turns the CSM, and this one starts
 * again from its CDUs, at rest, after.
 *
 * Each sample point turns the body by its rates over the time since the
 * last, in 1 cs steps, with the jets the channels held at the last sample
 * point, and writes CDUX-Z (outer, inner, middle gimbal) from the new
 * attitude. If anything else moved CDUX-Z since (a coarse align, a start
 * state), the attitude is taken from the CDUs as they are and the body
 * rates are kept. With FLIGHT_ATT the engine, SPS and entry models no
 * longer set CDUX-Z to the AGC's desired angles. */
#include "vehicle_models.h"

#if RK_LM
enum { ATT_ACCDOTQ = 03107, ATT_ACCDOTR = 03111, ATT_1JACC = 03130 };
#else
enum { ATT_IXX = 03070, ATT_IAVG = 03071 };
#define ATT_JET 444.8 /* N */
#define ATT_ARM 1.96  /* m */
#endif
#define ATT_TWO_PI 6.283185307179586

typedef struct {
    unsigned long t;
    int live;
    agc_word c5, c6, cdu[3];
    double rate[3];  /* about body X, Y, Z (LM P, Q, R), rad/s */
    double nb[3][3]; /* the body axes in stable-member axes, one row each */
} vm_attitude;

VM_FN int att_bits(unsigned v)
{
    int n = 0;
    for (; v; v &= v - 1u) ++n;
    return n;
}

VM_FN double att_angle(agc_word v)
{
    return (double)(v & 077777u) * (ATT_TWO_PI / 32768.0);
}

VM_FN agc_word att_cdu(double a)
{
    long c = (long)floor(a / ATT_TWO_PI * 32768.0 + 0.5);
    return (agc_word)((unsigned long)c & 077777ul);
}

/* A single-precision word, ones' complement. */
VM_FN double att_single(agc_word v)
{
    return (v & 040000u) ? -(double)((~v) & 037777u) : (double)v;
}

/* The angular accelerations, rad/s/s, of the jets in channels 5 and 6
 * (and on the LM the descent engine through its trim gimbal). */
VM_FN void att_accel(const agc_word *w, agc_word c5, agc_word c6, const vm_engine *engine,
                     double acc[3])
{
    const double scale = vm_cfg.att;
#if RK_LM
    double u = (double)(att_bits(c5 & 0204u) - att_bits(c5 & 0110u)),
           v = (double)(att_bits(c5 & 0041u) - att_bits(c5 & 0022u)),
           s = 0.7853981633974483 / 16384.0 * scale;
    acc[0] = (double)(att_bits(c6 & 0125u) - att_bits(c6 & 0252u)) * att_single(w[ATT_1JACC]) * s;
    acc[1] = (u - v) * att_single(w[ATT_1JACC + 1u]) * s;
    acc[2] = (u + v) * att_single(w[ATT_1JACC + 2u]) * s;
    if (engine && engine->on && !engine->lifted) {
        double p = engine->gimbal.angle[0], r = engine->gimbal.angle[1],
               k = 3.141592653589793 / 128.0 / 16384.0 / VM_TRIM_RATE * scale;
        acc[1] -= att_single(w[ATT_ACCDOTQ]) * k * sin(p) * cos(r);
        acc[2] -= att_single(w[ATT_ACCDOTR]) * k * sin(r);
    }
#else
    double ixx = att_single(w[ATT_IXX]) * 64.0, iavg = att_single(w[ATT_IAVG]) * 64.0,
           f = ATT_JET * ATT_ARM * scale;
    (void)engine;
    acc[0] = ixx > 0.0 ? (double)(att_bits(c6 & 0125u) - att_bits(c6 & 0252u)) * f / ixx : 0.0;
    acc[1] = iavg > 0.0 ? (double)(att_bits(c5 & 0005u) - att_bits(c5 & 0012u)) * f / iavg : 0.0;
    acc[2] = iavg > 0.0 ? (double)(att_bits(c5 & 0120u) - att_bits(c5 & 0240u)) * f / iavg : 0.0;
#endif
}

/* The body axes from CDUX-Z: Rx(outer) Rz(middle) Ry(inner). */
VM_FN void att_from_cdus(vm_attitude *a, const agc_word *w)
{
#if RK_LM
    static const double axis[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    unsigned k;
    for (k = 0; k < 3u; ++k)
        vm_nbsm(att_angle(w[VM_CDUY]), att_angle(w[VM_CDUZ]), att_angle(w[VM_CDUX]), axis[k],
                a->nb[k]);
#else
    double o = att_angle(w[VM_CDUX]), g = att_angle(w[VM_CDUZ]), i = att_angle(w[VM_CDUY]);
    double co = cos(o), so = sin(o), cg = cos(g), sg = sin(g), ci = cos(i), si = sin(i);
    double (*m)[3] = a->nb;
    m[0][0] = cg * ci;
    m[0][1] = sg;
    m[0][2] = -cg * si;
    m[1][0] = -co * sg * ci + so * si;
    m[1][1] = co * cg;
    m[1][2] = co * sg * si + so * ci;
    m[2][0] = so * sg * ci + co * si;
    m[2][1] = -so * cg;
    m[2][2] = -so * sg * si + co * ci;
#endif
}

/* Gram-Schmidt on the rows: X kept, Y made square to it, Z = X x Y. */
VM_FN void att_orthonormal(double r[3][3])
{
    double d, n;
    unsigned k;
    n = sqrt(r[0][0] * r[0][0] + r[0][1] * r[0][1] + r[0][2] * r[0][2]);
    for (k = 0; k < 3u; ++k) r[0][k] /= n;
    d = r[1][0] * r[0][0] + r[1][1] * r[0][1] + r[1][2] * r[0][2];
    for (k = 0; k < 3u; ++k) r[1][k] -= d * r[0][k];
    n = sqrt(r[1][0] * r[1][0] + r[1][1] * r[1][1] + r[1][2] * r[1][2]);
    for (k = 0; k < 3u; ++k) r[1][k] /= n;
    r[2][0] = r[0][1] * r[1][2] - r[0][2] * r[1][1];
    r[2][1] = r[0][2] * r[1][0] - r[0][0] * r[1][2];
    r[2][2] = r[0][0] * r[1][1] - r[0][1] * r[1][0];
}

/* Turn the body by its rates for dt seconds: d(axis i)/dt is the sum over
 * the other axes j of rate_j (axis j x axis i). */
VM_FN void att_turn(vm_attitude *a, double dt)
{
    double (*r)[3] = a->nb, d[3][3];
    unsigned i, k;
    for (i = 0; i < 3u; ++i) {
        unsigned j = (i + 1u) % 3u, l = (i + 2u) % 3u;
        for (k = 0; k < 3u; ++k) d[i][k] = (a->rate[l] * r[j][k] - a->rate[j] * r[l][k]) * dt;
    }
    for (i = 0; i < 3u; ++i)
        for (k = 0; k < 3u; ++k) r[i][k] += d[i][k];
    att_orthonormal(r);
}

/* CDUX-Z from the body axes: the inverse of att_from_cdus. */
VM_FN void att_to_cdus(vm_attitude *a, agc_word *w)
{
    double (*r)[3] = a->nb;
    double middle = asin(r[0][1] > 1.0 ? 1.0 : r[0][1] < -1.0 ? -1.0 : r[0][1]);
    double inner = atan2(-r[0][2], r[0][0]), outer = atan2(-r[2][1], r[1][1]);
    w[VM_CDUX] = a->cdu[0] = att_cdu(outer);
    w[VM_CDUY] = a->cdu[1] = att_cdu(inner);
    w[VM_CDUZ] = a->cdu[2] = att_cdu(middle);
}

/* One sample point at t: c5 and c6 the jet channels now, hold set while
 * the SPS model turns the CSM, engine the LM's descent engine (0 on the
 * CM). */
VM_FN void vm_attitude_step(vm_attitude *a, unsigned long t, agc_word *w, agc_word c5, agc_word c6,
                            int hold, const vm_engine *engine)
{
    double acc[3];
    unsigned k;
    if (vm_cfg.att == 0.0) return;
    if (hold) {
        a->live = 0;
        for (k = 0; k < 3u; ++k) a->rate[k] = 0.0;
    } else {
        if (!a->live || w[VM_CDUX] != a->cdu[0] || w[VM_CDUY] != a->cdu[1] ||
            w[VM_CDUZ] != a->cdu[2]) {
            att_from_cdus(a, w);
            for (k = 0; k < 3u; ++k) a->cdu[k] = w[VM_CDUX + k];
            a->live = 1;
        }
        if (a->t && t > a->t) {
            unsigned long n = t - a->t;
            att_accel(w, a->c5, a->c6, engine, acc);
            if (acc[0] != 0.0 || acc[1] != 0.0 || acc[2] != 0.0 || a->rate[0] != 0.0 ||
                a->rate[1] != 0.0 || a->rate[2] != 0.0) {
                for (; n; --n) {
                    for (k = 0; k < 3u; ++k) a->rate[k] += acc[k] * 0.005;
                    att_turn(a, 0.01);
                    for (k = 0; k < 3u; ++k) a->rate[k] += acc[k] * 0.005;
                }
                att_to_cdus(a, w);
            }
        }
    }
    if (t > a->t) a->t = t;
    a->c5 = c5;
    a->c6 = c6;
}
#endif
