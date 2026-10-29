/* SPDX-License-Identifier: GPL-2.0-or-later
 * The spacecraft's rotation under its RCS jets, for tools/flight_host_run.c
 * (host only, NEVER part of the native port; docs/FLIGHT-STORE.md, Flight
 * scenarios). Included after flight_rhc.h; native side, scaled time only.
 * GPL like the rest of the yaAGC-linked harness: an MIT program (the x86_64
 * build) needs its own copy, not this file.
 *
 * FLIGHT_ATT=SCALE: a rigid body. Each jet the DAP fires gives an angular
 * acceleration about one body axis, times SCALE.
 *
 * LM: the acceleration the DAP itself counts on (1JACC, 1JACCQ, 1JACCR:
 * one jet's acceleration about P, Q and R, scaled at pi/4 rad/s/s), so 1 is
 * a vehicle exactly as the DAP thinks it is:
 * - channel 6 (the P jets): bits 1, 3, 5 and 7 turn +P, bits 2, 4, 6 and 8
 *   -P (P-AXIS_RCS_AUTOPILOT's JETSALL, +P 00125 and -P 00252);
 * - channel 5 (the Q,R jets): bits 3 and 8 turn +U, 4 and 7 -U, 1 and 6 +V,
 *   2 and 5 -V (Q_R-AXIS_RCS_AUTOPILOT's ALLJETS). A U jet gives +1JACCQ
 *   and +1JACCR, a V jet -1JACCQ and +1JACCR (the JETRATEQ and JETRATER
 *   formulas in P-AXIS_RCS_AUTOPILOT).
 * While the descent engine burns, its trim gimbal (dps_gimbal,
 * tools/flight_dps_gimbal.h) adds -ACCDOTQ sin p cos r about Q and
 * -ACCDOTR sin r about R per radian of gimbal angle (the jerk ACCDOTQ or
 * ACCDOTR over the 0.2 deg/s gimbal rate): the T L / I the DAP itself
 * counts on (AOSTASK), with p the pitch and r the roll trim angle.
 *
 * CM: ATT_JET newtons a jet (the 100 lbf service module jets), ATT_ARM
 * metres from the roll axis (the quads), over the inertia the AGC itself
 * holds (IXX for roll, IAVG for pitch and yaw, TVCMASSPROP's B+20 kg m^2),
 * so 1 is a CSM as heavy as the AGC thinks it is:
 * - channel 5 (JET_SELECTION_LOGIC's PYTABLE, masks PJETS and YJETS):
 *   bits 1 and 3 +pitch, 2 and 4 -pitch, 5 and 7 +yaw, 6 and 8 -yaw;
 * - channel 6 (RTABLE): bits 1, 3, 5 and 7 +roll, 2, 4, 6 and 8 -roll.
 * While the SPS is on (FLIGHT_SPS, sps_nat.on) the SPS model owns the attitude
 * and this one only follows it.
 *
 * Each native sample point turns the body by its rates over the time
 * since the last, in steps of 1 cs, with the jets the channels held at the
 * last sample point (a jet that fires and stops between two points is not
 * seen), and writes CDUX-Z (outer, inner, middle gimbal) from the new
 * attitude. If anything else moved CDUX-Z since (a coarse align, a
 * snapshot, gyro torquing, the SPS model), the attitude is taken from the
 * CDUs as they are and the body rates are kept. No CG offset (the LM's
 * engine torque comes from the trim angles alone), no propellant use, no
 * cross-coupling between the axes (no products of inertia) and no rate
 * limit on the gimbals. With FLIGHT_ATT the engine
 * models no longer set CDUX-Z to the DAP's desired angles. */
#if RK_LM || RK_CM
#if RK_LM
enum { ATT_ACCDOTQ = 03107, ATT_ACCDOTR = 03111, ATT_1JACC = 03130 };
#else
enum { ATT_IXX = 03070, ATT_IAVG = 03071 };
#define ATT_JET 444.8
#define ATT_ARM 1.96
#endif
static double att_scale;
static struct {
    unsigned long t;
    int live;
    agc_word c5, c6, cdu[3];
    int hold;        /* the SPS model owns the attitude (CM) */
    double rate[3];  /* about body X, Y, Z (LM P, Q, R), rad/s */
    double nb[3][3]; /* the body axes in stable-member axes, one row each */
} att;
static int att_pop(unsigned v)
{
    int n = 0;
    for (; v; v &= v - 1u) ++n;
    return n;
}
static double att_ang(agc_word v)
{
    return (double)(v & 077777u) * (6.283185307179586 / 32768.0);
}
static agc_word att_word(double a)
{
    long c = (long)floor(a / 6.283185307179586 * 32768.0 + 0.5);
    return (agc_word)((unsigned long)c & 077777ul);
}
static double att_ones(agc_word v)
{
    return (v & 040000u) ? -(double)((~v) & 037777u) : (double)v;
}
/* The angular accelerations, rad/s/s, of the jets in channels 5 and 6. */
static void att_accel(const agc_word *w, agc_word c5, agc_word c6, double acc[3])
{
#if RK_LM
    double u = (double)(att_pop(c5 & 0204u) - att_pop(c5 & 0110u)),
           v = (double)(att_pop(c5 & 0041u) - att_pop(c5 & 0022u)),
           s = 0.7853981633974483 / 16384.0 * att_scale;
    acc[0] = (double)(att_pop(c6 & 0125u) - att_pop(c6 & 0252u)) * att_ones(w[ATT_1JACC]) * s;
    acc[1] = (u - v) * att_ones(w[ATT_1JACC + 1u]) * s;
    acc[2] = (u + v) * att_ones(w[ATT_1JACC + 2u]) * s;
    if (eng_nat.on && !eng_nat.lifted) {
        /* The descent engine through its trim gimbal: the angular
         * acceleration per radian of gimbal the DAP itself counts on,
         * ACCDOTQ and ACCDOTR (the jerk while the gimbal turns at 0.2
         * deg/s, scaled at pi/2^7 rad/s^3; AOSTASK's T L / I). */
        double off[2], k = 3.141592653589793 / 128.0 / 16384.0 / DPS_GIMBAL_RATE * att_scale;
        dps_gimbal(&eng_nat.gim, off);
        acc[1] -= att_ones(w[ATT_ACCDOTQ]) * k * sin(off[0]) * cos(off[1]);
        acc[2] -= att_ones(w[ATT_ACCDOTR]) * k * sin(off[1]);
    }
#else
    double ixx = att_ones(w[ATT_IXX]) * 64.0, iavg = att_ones(w[ATT_IAVG]) * 64.0,
           f = ATT_JET * ATT_ARM * att_scale;
    acc[0] = ixx > 0.0 ? (double)(att_pop(c6 & 0125u) - att_pop(c6 & 0252u)) * f / ixx : 0.0;
    acc[1] = iavg > 0.0 ? (double)(att_pop(c5 & 0005u) - att_pop(c5 & 0012u)) * f / iavg : 0.0;
    acc[2] = iavg > 0.0 ? (double)(att_pop(c5 & 0120u) - att_pop(c5 & 0240u)) * f / iavg : 0.0;
#endif
}
/* The body axes from CDUX-Z: Rx(outer) Rz(middle) Ry(inner), as lr_nbsm
 * (tools/flight_lr.h, LM) and sps_matrix (CM). */
static void att_from_cdus(const agc_word *w)
{
#if RK_LM
    static const double e[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    unsigned k;
    for (k = 0; k < 3u; ++k)
        lr_nbsm(att_ang(w[CDUY]), att_ang(w[CDUZ]), att_ang(w[CDUX]), e[k], att.nb[k]);
#else
    double o = att_ang(w[CDUX]), g = att_ang(w[CDUZ]), i = att_ang(w[CDUY]);
    double co = cos(o), so = sin(o), cg = cos(g), sg = sin(g), ci = cos(i), si = sin(i),
           (*m)[3] = att.nb;
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
/* Gram-Schmidt on the rows. */
static void att_orthonormal(void)
{
    double(*r)[3] = att.nb, d, n;
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
/* Turn the body by its rates for dt seconds: each body axis moves by
 * rate x axis (the axes' own frame turning at rate). */
static void att_turn(double dt)
{
    double(*r)[3] = att.nb, d[3][3];
    unsigned i, k;
    for (i = 0; i < 3u; ++i)
        for (k = 0; k < 3u; ++k) {
            unsigned j = (i + 1u) % 3u, l = (i + 2u) % 3u;
            /* d(axis i)/dt = sum over j of rate_j (e_j x e_i), in SM axes */
            d[i][k] = (att.rate[l] * r[j][k] - att.rate[j] * r[l][k]) * dt;
        }
    for (i = 0; i < 3u; ++i)
        for (k = 0; k < 3u; ++k) r[i][k] += d[i][k];
    att_orthonormal();
}
/* CDUX-Z from the body axes: the inverse of att_from_cdus. */
static void att_to_cdus(agc_word *w)
{
    double(*r)[3] = att.nb, mg = asin(r[0][1] > 1.0 ? 1.0 : r[0][1] < -1.0 ? -1.0 : r[0][1]);
    double ig = atan2(-r[0][2], r[0][0]), og = atan2(-r[2][1], r[1][1]);
    w[CDUX] = att.cdu[0] = att_word(og);
    w[CDUY] = att.cdu[1] = att_word(ig);
    w[CDUZ] = att.cdu[2] = att_word(mg);
}
static void att_due(unsigned long t, agc_word *w, const agc_hardware *hw)
{
    double acc[3];
    unsigned k;
    if (!att_scale) return;
    if (att.hold) {
        /* The SPS model turns the CM: start again from its CDUs, at rest. */
        att.live = 0;
        for (k = 0; k < 3u; ++k) att.rate[k] = 0.0;
        if (t > att.t) att.t = t;
        att.c5 = hw->channel5;
        att.c6 = hw->channel6;
        return;
    }
    if (!att.live || w[CDUX] != att.cdu[0] || w[CDUY] != att.cdu[1] || w[CDUZ] != att.cdu[2]) {
        att_from_cdus(w);
        for (k = 0; k < 3u; ++k) att.cdu[k] = w[CDUX + k];
        att.live = 1;
    }
    if (att.t && t > att.t) {
        unsigned long n = t - att.t;
        att_accel(w, att.c5, att.c6, acc);
        if (acc[0] != 0.0 || acc[1] != 0.0 || acc[2] != 0.0 || att.rate[0] != 0.0 ||
            att.rate[1] != 0.0 || att.rate[2] != 0.0) {
            for (; n; --n) {
                for (k = 0; k < 3u; ++k) att.rate[k] += acc[k] * 0.005;
                att_turn(0.01);
                for (k = 0; k < 3u; ++k) att.rate[k] += acc[k] * 0.005;
            }
            att_to_cdus(w);
        }
    }
    if (t > att.t) att.t = t;
    att.c5 = hw->channel5;
    att.c6 = hw->channel6;
}
#else
static double att_scale;
static void att_due(unsigned long t, agc_word *w, const agc_hardware *hw)
{
    (void)t;
    (void)w;
    (void)hw;
}
#endif
