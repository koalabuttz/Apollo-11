/* SPDX-License-Identifier: GPL-2.0-or-later
 * The LM descent engine's trim gimbal for tools/flight_host_run.c (host
 * only, NEVER part of the native port; docs/FLIGHT-STORE.md, Flight
 * scenarios). Included before the engine model (FLIGHT_ENGINE).
 *
 * Channel 12 bits 9 and 10 drive the pitch trim (about body Y, the Q
 * axis), bits 11 and 12 the roll trim (about body Z, the R axis), as the
 * trim gimbal control (TRIM_GIMBAL_CONTROL_SYSTEM.agc ACDT+C12) sets them.
 * Bits 10 and 12 turn the gimbal the positive way, which the DAP expects
 * to change the angular acceleration negatively (AOSTASK_AND_AOSJOB.agc
 * GIMBLBTS, "POSITIVE ROTATION, NEGATIVE Q(R)ACCDOT"); bits 9 and 11 turn
 * it back; both bits of an axis on drive nothing. While the engine model
 * runs, each sample point turns the gimbal by the time since the last one
 * at 0.2 deg/s, within +-6 deg, on both sides from their own channel 12.
 * The gimbal starts at 0 and keeps its angles with the engine off.
 *
 * dps_gimbal(g, off): the angles in radians, off[0] pitch, off[1] roll.
 * The thrust leaves the engine (behind the center of mass, on -X) along
 * body (cos p cos r, sin r, -sin p cos r), so a positive angle gives a
 * negative torque: about Y -T L sin p cos r, about Z -T L sin r, for
 * thrust T and pivot-to-CG arm L. An attitude model reads this (D1's
 * FLIGHT_ATT, to come).
 *
 * FLIGHT_TRIM=1: the thrust follows the gimbal. engine_due puts it along
 * that tilted axis, into stable-member axes through all three CDUs. Off by
 * default: the engine model holds the attitude perfectly (CDUX-Z set to
 * the DAP's desired angles), so nothing turns the LM back and the tilt
 * would only bend the trajectory. */
enum { DPS_PITCH_PLUS = 01000, DPS_PITCH_MINUS = 0400, DPS_ROLL_PLUS = 04000, DPS_ROLL_MINUS = 02000 };
#define DPS_GIMBAL_RATE (0.2 * 3.141592653589793 / 180.0) /* rad/s */
#define DPS_GIMBAL_LIMIT (6.0 * 3.141592653589793 / 180.0) /* rad */
struct dps_gim {
    unsigned long t;
    int started;
    double off[2];
};
static int dps_trim;
static void dps_gimbal(const struct dps_gim *g, double off[2])
{
    off[0] = g->off[0];
    off[1] = g->off[1];
}
/* The drive of one axis: +1, -1 or 0. */
static int dps_drive(agc_word c12, unsigned plus, unsigned minus)
{
    return ((c12 & plus) != 0) - ((c12 & minus) != 0);
}
static void dps_gimbal_due(unsigned long t, struct dps_gim *g, agc_word c12)
{
    double dt = g->started && t > g->t ? (double)(t - g->t) * 0.01 : 0.0;
    int d[2], k;
    g->t = t;
    g->started = 1;
    d[0] = dps_drive(c12, DPS_PITCH_PLUS, DPS_PITCH_MINUS);
    d[1] = dps_drive(c12, DPS_ROLL_PLUS, DPS_ROLL_MINUS);
    for (k = 0; k < 2; ++k) {
        g->off[k] += d[k] * DPS_GIMBAL_RATE * dt;
        g->off[k] = g->off[k] > DPS_GIMBAL_LIMIT ? DPS_GIMBAL_LIMIT : g->off[k] < -DPS_GIMBAL_LIMIT ? -DPS_GIMBAL_LIMIT : g->off[k];
    }
}
/* The thrust direction in body axes (unit). */
static void dps_thrust_body(const struct dps_gim *g, double b[3])
{
    double p = dps_trim ? g->off[0] : 0.0, r = dps_trim ? g->off[1] : 0.0;
    b[0] = cos(p) * cos(r);
    b[1] = sin(r);
    b[2] = -sin(p) * cos(r);
}
/* The thrust direction in stable-member axes, from the outer, inner and
 * middle gimbal angles (CDUX, CDUY, CDUZ, radians): body to stable member
 * as Rx(outer) Rz(middle) Ry(inner). */
static void dps_thrust_sm(const struct dps_gim *g, double o, double ig, double mg, double dir[3])
{
    double b[3], co = cos(o), so = sin(o), cg = cos(mg), sg = sin(mg), ci = cos(ig), si = sin(ig);
    double x[3] = {cg * ci, sg, -cg * si}, y[3] = {-co * sg * ci + so * si, co * cg, co * sg * si + so * ci},
           z[3] = {so * sg * ci + co * si, -so * cg, -so * sg * si + co * ci};
    int j;
    dps_thrust_body(g, b);
    for (j = 0; j < 3; ++j)
        dir[j] = b[0] * x[j] + b[1] * y[j] + b[2] * z[j];
}
