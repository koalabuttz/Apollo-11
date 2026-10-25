/* SPDX-License-Identifier: GPL-2.0-or-later
 * The LM's rotation under its RCS jets, for tools/flight_host_run.c (host
 * only, NEVER part of the native port; docs/FLIGHT-STORE.md, Flight
 * scenarios). Included after flight_rhc.h; native side, scaled time only.
 *
 * FLIGHT_ATT=SCALE (LM): a rigid body. Each jet the DAP fires turns the LM
 * at the angular acceleration the DAP itself counts on (1JACC, 1JACCQ,
 * 1JACCR: one jet's acceleration about P, Q and R, scaled at pi/4 rad/s/s),
 * times SCALE (1 is a vehicle exactly as the DAP thinks it is):
 * - channel 6 (the P jets): bits 1, 3, 5 and 7 turn +P, bits 2, 4, 6 and 8
 *   -P (P-AXIS_RCS_AUTOPILOT's JETSALL, +P 00125 and -P 00252);
 * - channel 5 (the Q,R jets): bits 3 and 8 turn +U, 4 and 7 -U, 1 and 6 +V,
 *   2 and 5 -V (Q_R-AXIS_RCS_AUTOPILOT's ALLJETS). A U jet gives +1JACCQ
 *   and +1JACCR, a V jet -1JACCQ and +1JACCR (the JETRATEQ and JETRATER
 *   formulas in P-AXIS_RCS_AUTOPILOT).
 * Each native routine boundary turns the body by its rates over the time
 * since the last, in steps of at most 1 cs, and writes CDUX-Z (outer,
 * inner, middle gimbal) from the new attitude. If anything else moved
 * CDUX-Z since (a coarse align, a snapshot, gyro torquing), the attitude is
 * taken from the CDUs as they are and the body rates are kept. No engine
 * torque, no propellant use, no cross-coupling between the axes (no products
 * of inertia) and no rate limit on the gimbals. With FLIGHT_ATT the engine
 * model no longer sets CDUX-Z to CDUXD-ZD. */
#if RK_LM
enum { ATT_1JACC = 03130 };
static double att_scale;
static struct {
    unsigned long t;
    int live;
    agc_word c5, c6, cdu[3];
    double rate[3]; /* P, Q, R, rad/s, body axes */
    double nb[3][3]; /* nav base axes in stable-member axes, one row each */
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
/* One jet's acceleration about axis k (0 P, 1 Q, 2 R), rad/s/s. */
static double att_jacc(const agc_word *w, unsigned k)
{
    agc_word v = w[ATT_1JACC + k];
    double a = (v & 040000u) ? -(double)((~v) & 037777u) : (double)v;
    return a / 16384.0 * 0.7853981633974483 * att_scale;
}
/* The body axes from CDUX-Z (lr_nbsm, tools/flight_lr.h). */
static void att_from_cdus(const agc_word *w)
{
    static const double e[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    unsigned k;
    for (k = 0; k < 3u; ++k) lr_nbsm(att_ang(w[CDUY]), att_ang(w[CDUZ]), att_ang(w[CDUX]), e[k], att.nb[k]);
}
/* Gram-Schmidt on the rows. */
static void att_orthonormal(void)
{
    double (*r)[3] = att.nb, d, n;
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
    double (*r)[3] = att.nb, d[3][3];
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
/* CDUX-Z from the body axes: the inverse of lr_smnb's Rx(og) Rz(mg) Ry(ig). */
static void att_to_cdus(agc_word *w)
{
    double (*r)[3] = att.nb, mg = asin(r[0][1] > 1.0 ? 1.0 : r[0][1] < -1.0 ? -1.0 : r[0][1]);
    double ig = atan2(-r[0][2], r[0][0]), og = atan2(-r[2][1], r[1][1]);
    w[CDUX] = att.cdu[0] = att_word(og);
    w[CDUY] = att.cdu[1] = att_word(ig);
    w[CDUZ] = att.cdu[2] = att_word(mg);
}
static void att_due(unsigned long t, agc_word *w, const agc_hardware *hw)
{
    double acc[3], u, v;
    unsigned k;
    if (!att_scale) return;
    if (!att.live || w[CDUX] != att.cdu[0] || w[CDUY] != att.cdu[1] || w[CDUZ] != att.cdu[2]) {
        att_from_cdus(w);
        for (k = 0; k < 3u; ++k) att.cdu[k] = w[CDUX + k];
        att.live = 1;
    }
    if (att.t && t > att.t) {
        unsigned long n = t - att.t;
        u = (double)(att_pop(att.c5 & 0204u) - att_pop(att.c5 & 0110u));
        v = (double)(att_pop(att.c5 & 0041u) - att_pop(att.c5 & 0022u));
        acc[0] = (double)(att_pop(att.c6 & 0125u) - att_pop(att.c6 & 0252u)) * att_jacc(w, 0);
        acc[1] = (u - v) * att_jacc(w, 1);
        acc[2] = (u + v) * att_jacc(w, 2);
        if (acc[0] != 0.0 || acc[1] != 0.0 || acc[2] != 0.0 || att.rate[0] != 0.0 || att.rate[1] != 0.0 ||
            att.rate[2] != 0.0) {
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
