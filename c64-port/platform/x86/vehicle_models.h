/* SPDX-License-Identifier: MIT */
#ifndef VEHICLE_MODELS_H
#define VEHICLE_MODELS_H
/* The vehicle the AGC flies, outside the AGC: engines, jets, the
 * atmosphere, the IMU gimbals and the Saturn's boost. Each model reads the
 * AGC's own erasable memory (2,048 words, as the native port keeps them)
 * and the input channels, and writes what the hardware would: PIPA counts,
 * CDU angles and channel bits.
 *
 * These are the models this port wrote for its flight scenarios in the
 * host differential (tools/flight_host_run.c, docs/FLIGHT-STORE.md), with
 * the same physics and the same results; they contain no Virtual AGC code
 * and link nothing, so the native x86_64 flight program can fly them. The
 * host differential still runs its own copy; it moves to this header in a
 * separate change. platform/x86/test_vehicle_models.c checks each model.
 *
 * vm_config holds the scenario's settings, one field for each FLIGHT_*
 * variable; a model whose setting is 0 does nothing. Each model keeps its
 * state in its own struct, one per AGC it runs beside. The models run at
 * each sample point (the first idle pass through ADVAN in a centisecond),
 * t being AGC centiseconds. */
#include <math.h>
#include "agc_word.h"

#define VM_FN static __attribute__((unused))

typedef struct {
    double engine_kg;           /* FLIGHT_ENGINE (LM): descent stage mass */
    double engine_lag;          /* FLIGHT_ENGINE_LAG: throttle lag, s */
    unsigned long touchdown_cs; /* FLIGHT_TOUCHDOWN */
    double pilot_tau;           /* FLIGHT_PILOT: P66 crew time constant, s */
    double ascent_kg;           /* FLIGHT_ASCENT: ascent stage mass */
    unsigned long stage_cs;     /* FLIGHT_STAGE: abort stage */
    double rcs_kg;              /* FLIGHT_RCS (LM): vehicle mass for the jets */
    double sps_kg;              /* FLIGHT_SPS (CM): CSM mass */
    double aero_b;              /* FLIGHT_AERO (CM): ballistic coefficient */
    int imu;                    /* FLIGHT_IMU (CM): gimbal model on */
    unsigned long saturn_lo;    /* FLIGHT_SATURN (CM): liftoff */
    unsigned long sivb_tig;     /* FLIGHT_SIVB (CM): TLI ignition */
    double sivb_u[3];           /* FLIGHT_SIVB: TLI thrust direction */
} vm_config;
static vm_config vm_cfg;

/* A velocity change dv (m/s, stable-member axes) the engine or the jets
 * gave over the dt seconds to AGC centisecond t: the landing radar's true
 * trajectory takes it. */
typedef void (*vm_dv_hook)(const double *dv, unsigned long t, double dt);

/* Erasable addresses. */
enum {
    VM_CDUX = 032,
    VM_CDUY = 033,
    VM_CDUZ = 034,
    VM_PIPAX = 037,
    VM_AVEGWORD = 075,
    VM_AVEGBIT = 1,
    VM_FLAGWRD5 = 0101,
    VM_ENGONBIT = 0100,
    VM_MODREG = 01011,
    VM_THETAD = 01155,
    VM_RN = 01170,
    VM_VN = 01176,
    VM_PIPTIME = 01204,
    VM_REFSMMAT = 01735,
    VM_LM_CDUXD = 03235,
    VM_CM_CDUXD = 03246,
    VM_ROLLC = 03315,
    VM_P63FLAG = 03327,
    VM_PSEUDO55 = 03614,
    VM_TTHROT = 03617
};

#define VM_MOON_MU 4.9028e12 /* m^3/s^2 */
#define VM_TWO_PI  6.283185307179586

/* A ones-complement word as a signed number. */
VM_FN long vm_ones(agc_word v)
{
    return (v & 040000u) ? -(long)((~v) & 037777u) : (long)v;
}

/* A double-precision pair at a, scaled 2^scale. */
VM_FN double vm_dp(const agc_word *w, unsigned a, int scale)
{
    return ((double)vm_ones(w[a]) * 16384.0 + (double)vm_ones(w[a + 1u])) * ldexp(1.0, scale - 28);
}

/* A CDU angle in radians. */
VM_FN double vm_angle(agc_word v)
{
    return (double)(v & 077777u) * (VM_TWO_PI / 32768.0);
}

/* Stable member to nav base through gimbal angles inner ig (Y), middle mg
 * (Z), outer og (X), as *SMNB*; vm_nbsm is its inverse. */
VM_FN void vm_smnb(double ig, double mg, double og, const double *a, double *b)
{
    double x1 = a[0] * cos(ig) - a[2] * sin(ig), z1 = a[0] * sin(ig) + a[2] * cos(ig), y1 = a[1],
           x2 = x1 * cos(mg) + y1 * sin(mg), y2 = -x1 * sin(mg) + y1 * cos(mg);
    b[0] = x2;
    b[1] = y2 * cos(og) + z1 * sin(og);
    b[2] = -y2 * sin(og) + z1 * cos(og);
}
VM_FN void vm_nbsm(double ig, double mg, double og, const double *b, double *a)
{
    double y2 = b[1] * cos(og) - b[2] * sin(og), z1 = b[1] * sin(og) + b[2] * cos(og),
           x1 = b[0] * cos(mg) - y2 * sin(mg), y1 = b[0] * sin(mg) + y2 * cos(mg);
    a[0] = x1 * cos(ig) + z1 * sin(ig);
    a[1] = y1;
    a[2] = -x1 * sin(ig) + z1 * cos(ig);
}

/* Add n counts to PIPA j (0-2), saturating at +-037777. */
VM_FN void vm_pipa_add(agc_word *w, unsigned j, long n)
{
    long v = vm_ones(w[VM_PIPAX + j]) + n;
    v = v > 037777l ? 037777l : v < -037777l ? -037777l : v;
    v = v < 0 ? (long)((~(unsigned long)(-v)) & 077777ul) : v;
    w[VM_PIPAX + j] = (agc_word)v;
}

/* Velocity f (m/s, stable-member axes) into the PIPAs at 5.85 cm/s a
 * count (the CM's), carrying the fraction. */
VM_FN void vm_pipa_carry(double *carry, const double f[3], agc_word *w)
{
    unsigned k;
    for (k = 0; k < 3u; ++k) {
        long n;
        carry[k] += f[k] * 100.0 / 5.85;
        n = (long)carry[k];
        carry[k] -= (double)n;
        vm_pipa_add(w, k, n);
    }
}

/* FLIGHT_ENGINE=KG (LM): a minimal descent engine, so DVMON sees thrust
 * and the landing guidance flies. From the first sample point with
 * ENGONFLG set (FLAGWRD5 bit 7) it thrusts at 10% (373 throttle bits of
 * 12.53 N; BITPERF), moves by each new throttle command (PSEUDO55, taken
 * when TTHROT changes) within 373 and FMAXPOS (3467), and burns KG
 * kilograms down at DPSVEX (2955.9 m/s). The thrust acts along body X, put
 * into stable-member axes from CDUY (inner gimbal) and CDUZ (middle):
 * (cos IG cos MG, sin MG, -sin IG cos MG). Each sample point adds the
 * velocity since the last one to PIPAX-Z (1 cm/s a count), carrying the
 * fraction. No attitude dynamics: while the engine is on, each sample point
 * sets CDUX-Z to the DAP's desired angles CDUXD-ZD (perfect attitude
 * control).
 *
 * FLIGHT_ENGINE_LAG=S: the throttle as the hardware drives it. Each command
 * (PSEUDO55, sent to the THRUST counter) reaches the engine as pulses at
 * 3,200 a second (THROTTLE's FRATE, 32 bits a centisecond), and the thrust
 * follows them with a first-order lag of S seconds (THROTLAG assumes
 * 0.2 s). THROTTLE's FWEIGHT expects that: P66's 1 s rate-of-descent loop
 * reads the PIPAs a few centiseconds after SERVICER, often with no idle
 * pass between, and oscillates without it; so with a lag the program also
 * runs the engine at every native routine boundary, not only at sample
 * points.
 *
 * FLIGHT_TOUCHDOWN=CS: touchdown at CS. The engine gives no thrust from
 * then on; at contact the PIPAs take the velocity change that stops the LM
 * (minus its velocity over the surface), and after it the surface's
 * reaction, lunar gravity (GM/r^2) up along RN, put into stable-member axes
 * through REFSMMAT, so the navigation holds the LM still.
 *
 * FLIGHT_ASCENT=KG: the ascent stage's mass at liftoff. The first time the
 * engine is on after touchdown and an engine-off there, the LM lifts off on
 * the ascent engine: 15,569 N (3,500 lbf) along the X axis at 3,050 m/s
 * exhaust velocity, no throttle; with the engine off after that the PIPAs
 * stay still.
 *
 * FLIGHT_STAGE=CS: an abort stage at CS (P71: the crew's ABORT STAGE, set
 * on channel 30 bit 4). The descent stage separates in flight: from CS the
 * engine is the ascent engine of FLIGHT_ASCENT, as at liftoff, on or off as
 * ENGONFLG says.
 *
 * In P70 (MODREG 70) the descent engine goes to full thrust: THROTUP
 * writes BIT13 to THRUST directly, more than FMAXPOS, with no PSEUDO55 or
 * TTHROT change. */
typedef struct {
    int on, lifted, stopped, landed;
    unsigned long t;
    agc_word tthrot;
    double bits, cmd, ramp, kg, carry[3];
    vm_dv_hook dv; /* 0: none */
} vm_engine;

/* The velocity over the surface in stable-member axes, m/s: the LLGE's V
 * plus DELVS (the surface's own velocity, negated), as VHORCOMP forms it. */
VM_FN void vm_surface_v(const agc_word *w, double v[3])
{
    unsigned k;
    for (k = 0; k < 3u; ++k)
        v[k] = vm_dp(w, 03526u + 2u * k, 7) * 100.0 + vm_dp(w, 03733u + 2u * k, 5) * 100.0;
}

/* On the surface: contact, then the surface's reaction. */
VM_FN void vm_surface(unsigned long t, vm_engine *e, agc_word *w)
{
    double r[3], rm = 0.0, dt;
    unsigned j, k;
    /* Contact: the surface stops the LM, a velocity change the PIPAs count. */
    if (!e->landed) {
        double v[3];
        e->landed = 1;
        vm_surface_v(w, v);
        for (j = 0; j < 3u; ++j) vm_pipa_add(w, j, (long)floor(-v[j] * 100.0 + 0.5));
    }
    dt = e->t && t > e->t ? (double)(t - e->t) * 0.01 : 0.0;
    e->t = t;
    for (k = 0; k < 3u; ++k) {
        r[k] = vm_dp(w, 01220u + 2u * k, 29);
        rm += r[k] * r[k];
    }
    rm = sqrt(rm);
    if (rm <= 0.0) return;
    for (j = 0; j < 3u; ++j) {
        long n;
        double u = 0.0;
        for (k = 0; k < 3u; ++k) u += vm_dp(w, 01733u + 6u * j + 2u * k, 1) * r[k] / rm;
        e->carry[j] += VM_MOON_MU / (rm * rm) * dt * u * 100.0;
        n = (long)e->carry[j];
        e->carry[j] -= (double)n;
        vm_pipa_add(w, j, n);
    }
}

/* FLIGHT_PILOT=TAU: in P66 (MODREG 66) before touchdown, the crew's
 * attitude: the thrust axis tilts from the local vertical against the
 * velocity over the surface (the LLGE's V plus DELVS, stable-member axes),
 * nulling it with time constant TAU seconds, at most 15 degrees; the
 * stable-member CDUs and the DAP's desired CDUs (rate-command attitude hold
 * following the hand controller) take that attitude, outer gimbal kept. */
VM_FN void vm_pilot(agc_word *w)
{
    double r[3], v[3], u[3], rm = 0.0, vu = 0.0, h[3], hm = 0.0, t[3], tm = 0.0, ig, mg,
                             lim = tan(15.0 * 3.141592653589793 / 180.0);
    unsigned k;
    vm_surface_v(w, v);
    for (k = 0; k < 3u; ++k) {
        r[k] = vm_dp(w, 03520u + 2u * k, 24);
        rm += r[k] * r[k];
    }
    rm = sqrt(rm);
    if (rm <= 0.0) return;
    for (k = 0; k < 3u; ++k) {
        u[k] = r[k] / rm;
        vu += v[k] * u[k];
    }
    for (k = 0; k < 3u; ++k) {
        h[k] = -(v[k] - vu * u[k]) / (vm_cfg.pilot_tau * 1.62);
        hm += h[k] * h[k];
    }
    hm = sqrt(hm);
    if (hm > lim)
        for (k = 0; k < 3u; ++k) h[k] *= lim / hm;
    for (k = 0; k < 3u; ++k) {
        t[k] = u[k] + h[k];
        tm += t[k] * t[k];
    }
    tm = sqrt(tm);
    for (k = 0; k < 3u; ++k) t[k] /= tm;
    mg = asin(t[1]);
    ig = atan2(-t[2], t[0]);
    {
        long cy = (long)floor(ig / (2.0 * 3.141592653589793) * 32768.0 + 0.5),
             cz = (long)floor(mg / (2.0 * 3.141592653589793) * 32768.0 + 0.5);
        agc_word y = (agc_word)((unsigned long)cy & 077777ul),
                 z = (agc_word)((unsigned long)cz & 077777ul);
        w[VM_CDUY] = y;
        w[VM_CDUZ] = z;
        w[VM_LM_CDUXD + 1u] = y;
        w[VM_LM_CDUXD + 2u] = z;
    }
}

VM_FN void vm_engine_step(unsigned long t, vm_engine *e, agc_word *w)
{
    const vm_config *c = &vm_cfg;
    double f, a, dt, ig, mg, dir[3];
    unsigned k;
    if (c->engine_kg == 0.0) return;
    if (c->touchdown_cs && t >= c->touchdown_cs && !e->lifted) {
        int on = (w[VM_FLAGWRD5] & VM_ENGONBIT) != 0;
        if (!on) e->stopped = 1;
        if (!(c->ascent_kg != 0.0 && e->stopped && on)) {
            vm_surface(t, e, w);
            return;
        }
        e->lifted = 1;
        e->on = 0;
        e->kg = c->ascent_kg;
    }
    if (c->stage_cs && t >= c->stage_cs && !e->lifted) {
        e->lifted = 1;
        e->kg = c->ascent_kg;
    }
    if (!(w[VM_FLAGWRD5] & VM_ENGONBIT)) {
        e->on = 0;
        return;
    }
    for (k = 0; k < 3u; ++k) w[VM_CDUX + k] = w[VM_LM_CDUXD + k];
    if (c->pilot_tau != 0.0 && !e->lifted && w[VM_MODREG] == 0102u &&
        (!c->touchdown_cs || t < c->touchdown_cs))
        vm_pilot(w);
    if (!e->on) {
        e->on = 1;
        e->t = t;
        e->tthrot = w[VM_TTHROT];
        if (e->kg == 0.0) {
            e->kg = c->engine_kg;
            e->bits = e->cmd = e->ramp = 373.0;
        }
        return;
    }
    if (!e->lifted && w[VM_MODREG] == 0106u) {
        e->cmd = 3467.0;
        if (c->engine_lag == 0.0) e->bits = e->cmd;
    }
    if (!e->lifted && w[VM_TTHROT] != e->tthrot) {
        e->tthrot = w[VM_TTHROT];
        e->cmd += (double)vm_ones(w[VM_PSEUDO55]);
        e->cmd = e->cmd < 373.0 ? 373.0 : e->cmd > 3467.0 ? 3467.0 : e->cmd;
        if (c->engine_lag == 0.0) e->bits = e->cmd;
    }
    dt = (double)(t - e->t) * 0.01;
    e->t = t;
    f = e->lifted ? 15569.0 : e->bits * 12.53;
    a = f / e->kg;
    e->kg -= f / (e->lifted ? 3050.0 : 2955.9) * dt;
    if (!e->lifted && c->engine_lag != 0.0) {
        double r = 32.0 * 100.0 * dt;
        e->ramp += e->cmd - e->ramp > r ? r : e->cmd - e->ramp < -r ? -r : e->cmd - e->ramp;
        e->bits += (e->ramp - e->bits) * (1.0 - exp(-dt / c->engine_lag));
    }
    ig = (double)w[VM_CDUY] * (VM_TWO_PI / 32768.0);
    mg = (double)w[VM_CDUZ] * (VM_TWO_PI / 32768.0);
    dir[0] = cos(ig) * cos(mg);
    dir[1] = sin(mg);
    dir[2] = -sin(ig) * cos(mg);
    if (e->dv) {
        double dv[3];
        for (k = 0; k < 3u; ++k) dv[k] = a * dt * dir[k];
        e->dv(dv, t, dt);
    }
    for (k = 0; k < 3u; ++k) {
        long n;
        e->carry[k] += a * dt * dir[k] * 100.0;
        n = (long)e->carry[k];
        e->carry[k] -= (double)n;
        vm_pipa_add(w, k, n);
    }
}

/* FLIGHT_RCS=KG (LM): the RCS jets' translation, for a vehicle of KG
 * kilograms. Each jet gives 444.8 N (100 lbf) along the axis the DAP's jet
 * tables name: channel 5's even bits +X, odd bits -X; channel 6's bits 5
 * and 8 +Y, 6 and 7 -Y, 1 and 4 +Z, 2 and 3 -Z (rotation couples cancel).
 * Each native routine boundary adds the jets' velocity since the last, to
 * the centisecond, put into stable-member axes from CDUX-Z, to PIPAX-Z
 * (1 cm/s a count), carrying the fraction. No attitude dynamics and no
 * propellant use. The crew's translation hand controller is channel 31
 * bits 7-12 (+X, -X, +Y, -Y, +Z, -Z, active 0). */
typedef struct {
    unsigned long t;
    agc_word c5, c6;
    double carry[3];
    vm_dv_hook dv; /* 0: none */
} vm_rcs;

VM_FN int vm_popcount(unsigned v)
{
    int n = 0;
    for (; v; v &= v - 1u) ++n;
    return n;
}

/* channel5, channel6: the jet commands now in force. */
VM_FN void vm_rcs_step(unsigned long t, vm_rcs *e, agc_word *w, agc_word channel5,
                       agc_word channel6)
{
    double b[3], d[3], dt;
    unsigned k;
    if (vm_cfg.rcs_kg == 0.0) return;
    if (e->t && t > e->t) {
        dt = (double)(t - e->t) * 0.01;
        b[0] = (double)(vm_popcount(e->c5 & 0252u) - vm_popcount(e->c5 & 0125u));
        b[1] = (double)(vm_popcount(e->c6 & 0220u) - vm_popcount(e->c6 & 0140u));
        b[2] = (double)(vm_popcount(e->c6 & 0011u) - vm_popcount(e->c6 & 0006u));
        if (b[0] != 0.0 || b[1] != 0.0 || b[2] != 0.0) {
            for (k = 0; k < 3u; ++k) b[k] *= 444.8 / vm_cfg.rcs_kg * dt;
            vm_nbsm(vm_angle(w[VM_CDUY]), vm_angle(w[VM_CDUZ]), vm_angle(w[VM_CDUX]), b, d);
            if (e->dv) e->dv(d, t, dt);
            for (k = 0; k < 3u; ++k) {
                long n;
                e->carry[k] += d[k] * 100.0;
                n = (long)e->carry[k];
                e->carry[k] -= (double)n;
                vm_pipa_add(w, k, n);
            }
        }
    }
    if (t > e->t) e->t = t;
    e->c5 = channel5;
    e->c6 = channel6;
}

/* FLIGHT_SPS=KG (CM): the service propulsion engine, so P40 sees its burn.
 * From the first sample point with ENGONFLG set (FLAGWRD5 bit 7) it thrusts
 * 91,188 N along the engine bell, body X turned by the mechanical biases
 * P40 assumes (PBIAS -2.15, YBIAS +0.95 degrees; small angles: CDUY less
 * 2.15 and CDUZ plus 0.95 degrees in the FLIGHT_ENGINE direction), burning
 * KG kilograms down at 3,080 m/s, and each sample point adds the velocity
 * since the last one to PIPAX-Z (5.85 cm/s a count), carrying the fraction.
 * No attitude dynamics: each sample point sets CDUX-Z to the RCS DAP's
 * desired angles CDUXD-ZD (perfect attitude control, so P40's R60 maneuver
 * ends) until the engine first lights; from then on the TVC DAP reuses
 * those cells as temporaries, so the attitude holds at ignition for the
 * rest of the run. */
typedef struct {
    int on, lit;
    unsigned long t;
    double kg, carry[3];
} vm_sps;

VM_FN void vm_sps_step(unsigned long t, vm_sps *e, agc_word *w)
{
    double a, dt, ig, mg, dir[3];
    unsigned k;
    if (vm_cfg.sps_kg == 0.0) return;
    if (!e->lit)
        for (k = 0; k < 3u; ++k) w[VM_CDUX + k] = w[VM_CM_CDUXD + 2u * k];
    if (!(w[VM_FLAGWRD5] & VM_ENGONBIT)) {
        e->on = 0;
        return;
    }
    if (!e->on) {
        e->on = e->lit = 1;
        e->t = t;
        if (e->kg == 0.0) e->kg = vm_cfg.sps_kg;
        return;
    }
    dt = (double)(t - e->t) * 0.01;
    e->t = t;
    a = 91188.0 / e->kg;
    e->kg -= 91188.0 / 3080.0 * dt;
    ig = (double)w[VM_CDUY] * (VM_TWO_PI / 32768.0) - 0.0375245789;
    mg = (double)w[VM_CDUZ] * (VM_TWO_PI / 32768.0) + 0.0165806279;
    dir[0] = cos(ig) * cos(mg);
    dir[1] = sin(mg);
    dir[2] = -sin(ig) * cos(mg);
    for (k = 0; k < 3u; ++k) {
        long n;
        e->carry[k] += a * dt * dir[k] * 100.0 / 5.85;
        n = (long)e->carry[k];
        e->carry[k] -= (double)n;
        vm_pipa_add(w, k, n);
    }
}

/* FLIGHT_AERO=B (CM): drag and lift on a capsule of ballistic coefficient
 * B kg/m^2 and L/D 0.3, so the entry programs see the atmosphere. While
 * SERVICER runs (AVEGFLAG) and the CM is below 140 km, each sample point
 * takes position and velocity from the AGC's own RN and VN (extrapolated
 * from PIPTIME to TIME1/TIME2: with exact PIPAs the AGC's navigation is the
 * trajectory), drag D = rho v^2 / 2B against the velocity relative to the
 * turning earth (rho = 1.225 exp(-h / 6.7 km), h above ERAD), lift 0.3 D
 * perpendicular to it, turned from straight up by the commanded roll ROLLC
 * (perfect roll control), and adds the velocity since the last point to
 * PIPAX-Z (5.85 cm/s a count, stable member axes through REFSMMAT),
 * carrying the fraction. While P62 waits for the maneuver to entry
 * attitude (P63FLAG +1) each sample point sets CDUX-Z to the desired
 * angles THETAD (perfect attitude control), so the entry DAP sees ALFA
 * within 45 degrees and wakes P62. */
typedef struct {
    unsigned long t;
    double carry[3];
} vm_aero;

VM_FN void vm_aero_step(unsigned long t, vm_aero *e, agc_word *w)
{
    double r[3], v[3], vr[3], lt[3], sd[3], acc[3], rt, h, vm, d, ph, un, dt, tn;
    unsigned k, j;
    if (vm_cfg.aero_b == 0.0) return;
    if (w[VM_P63FLAG] == 1u)
        for (k = 0; k < 3u; ++k) w[VM_CDUX + k] = w[VM_THETAD + k];
    dt = (double)(t - e->t) * 0.01;
    e->t = t;
    if (!(w[VM_AVEGWORD] & VM_AVEGBIT) || dt <= 0.0) return;
    tn = (double)(w[024] & 037777u) * 16384.0 + (double)(w[025] & 037777u) -
         vm_dp(w, VM_PIPTIME, 28);
    for (k = 0; k < 3u; ++k) {
        v[k] = vm_dp(w, VM_VN + 2u * k, 7) * 100.0;
        r[k] = vm_dp(w, VM_RN + 2u * k, 29) + v[k] * tn * 0.01;
    }
    rt = sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    h = rt - 6373338.0;
    if (rt < 6.0e6 || h > 140000.0) return;
    vr[0] = v[0] + 7.29211494e-5 * r[1];
    vr[1] = v[1] - 7.29211494e-5 * r[0];
    vr[2] = v[2];
    vm = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);
    d = 1.225 * exp(-h / 6700.0) * vm * vm / (2.0 * vm_cfg.aero_b);
    for (k = 0; k < 3u; ++k) vr[k] /= vm;
    un = (r[0] * vr[0] + r[1] * vr[1] + r[2] * vr[2]) / rt;
    for (k = 0; k < 3u; ++k) lt[k] = r[k] / rt - un * vr[k];
    un = sqrt(lt[0] * lt[0] + lt[1] * lt[1] + lt[2] * lt[2]);
    for (k = 0; k < 3u; ++k) lt[k] /= un;
    sd[0] = lt[1] * vr[2] - lt[2] * vr[1];
    sd[1] = lt[2] * vr[0] - lt[0] * vr[2];
    sd[2] = lt[0] * vr[1] - lt[1] * vr[0];
    ph = vm_dp(w, VM_ROLLC, 0) * VM_TWO_PI;
    for (k = 0; k < 3u; ++k) acc[k] = d * (-vr[k] + 0.3 * (cos(ph) * lt[k] + sin(ph) * sd[k]));
    for (j = 0; j < 3u; ++j) {
        long n;
        double a = 0.0;
        for (k = 0; k < 3u; ++k) a += vm_dp(w, VM_REFSMMAT + 6u * j + 2u * k, 1) * acc[k];
        e->carry[j] += a * dt * 100.0 / 5.85;
        n = (long)e->carry[j];
        e->carry[j] -= (double)n;
        vm_pipa_add(w, j, n);
    }
}

/* FLIGHT_IMU=1 (CM): the IMU gimbals, for the prelaunch alignment (P01):
 * while channel 12 bit 5 (zero the ICDUs) is set each sample point holds
 * CDUX-Z at 0; while bits 4 and 6 (coarse align, error counters enabled)
 * are set it sets them to the desired angles THETAD (a perfect coarse
 * align, so COARS's 2 degree check passes). Fine align and gyro torquing
 * move nothing: the platform stays where coarse align put it, and the
 * gyrocompass sees level PIPAs. channel12: output channel 12 now. */
VM_FN void vm_imu_step(agc_word *w, agc_word channel12)
{
    unsigned k;
    if (!vm_cfg.imu) return;
    if (channel12 & 020u) {
        for (k = 0; k < 3u; ++k) w[VM_CDUX + k] = 0u;
        return;
    }
    if ((channel12 & 050u) == 050u)
        for (k = 0; k < 3u; ++k) w[VM_CDUX + k] = w[VM_THETAD + k];
}

/* FLIGHT_SATURN=CS (CM): liftoff at CS and the Saturn V's boost to earth
 * orbit. At CS channel 30 bit 5 (liftoff, active 0) goes to 0, so P02's
 * compass loop starts P11. From then on each sample point adds the boost's
 * specific force since the last one to PIPAX-Z (5.85 cm/s a count,
 * carrying the fraction), in stable-member axes as the prelaunch alignment
 * leaves them (X downrange at the launch azimuth, Z down): thrust
 * acceleration a along the vehicle axis, pitched from the launch vertical
 * toward downrange by THETA, f = a (sin THETA, 0, -cos THETA). a: S-IC
 * 35.1 MN on 2,960 t less 13.0 t/s, centre engine out at 135.2 s (80%,
 * 10.4 t/s), cutoff 161.6 s; S-II from 164.0 s, 5.1 MN on 665 t less
 * 1.22 t/s, centre engine out at 460 s (80%, 0.98 t/s), cutoff 548 s;
 * S-IVB from 552 s, 0.9 MN on 167 t less 0.21 t/s, cutoff at 823 s. THETA:
 * 0 to 12 s, then linear to 52.2 degrees at 161.6 s, 81.2 at 400 s and
 * 93.3 at 700 s, held after. With the earth's rotation and gravity that
 * puts the vehicle in a 142 x 218 km orbit at 823 s
 * (tools/flight_scenarios/cm_p11.py prints the trajectory). No attitude
 * model: the CDUs stay at the liftoff angles. */
typedef struct {
    int lo;
    unsigned long t, lo_cs;
    double carry[3];
} vm_saturn;

VM_FN double vm_saturn_acc(double t)
{
    if (t < 0.0) return 0.0;
    if (t < 135.2) return 35.1e6 / (2960e3 - 13.0e3 * t);
    if (t < 161.6) return 0.8 * 35.1e6 / (2960e3 - 13.0e3 * 135.2 - 10.4e3 * (t - 135.2));
    if (t < 164.0) return 0.0;
    if (t < 460.0) return 5.1e6 / (665e3 - 1.22e3 * (t - 164.0));
    if (t < 548.0) return 4.08e6 / (665e3 - 1.22e3 * 296.0 - 0.98e3 * (t - 460.0));
    if (t < 552.0 || t >= 823.0) return 0.0;
    return 0.9e6 / (167e3 - 0.21e3 * (t - 552.0));
}

VM_FN double vm_saturn_pitch(double t)
{
    double d;
    if (t < 12.0)
        d = 0.0;
    else if (t < 161.6)
        d = 52.2 * (t - 12.0) / 149.6;
    else if (t < 400.0)
        d = 52.2 + 29.0 * (t - 161.6) / 238.4;
    else if (t < 700.0)
        d = 81.2 + 12.1 * (t - 400.0) / 300.0;
    else
        d = 93.3;
    return d * (3.141592653589793 / 180.0);
}

/* channel30: input channel 30, whose liftoff bit the model clears. */
VM_FN void vm_saturn_step(unsigned long t, vm_saturn *e, agc_word *w, agc_word *channel30)
{
    double f[3] = {0.0, 0.0, 0.0};
    unsigned long c;
    if (!vm_cfg.saturn_lo) return;
    if (!e->lo) {
        if (t < vm_cfg.saturn_lo) return;
        e->lo = 1;
        e->t = e->lo_cs = t;
        *channel30 = (agc_word)(*channel30 & ~020u);
        return;
    }
    /* P11 zeroes the clock at liftoff: from the first point after that,
     * time counts from liftoff on the AGC clock. */
    if (t < e->t) {
        e->lo_cs = 0;
        e->t = 0;
    }
    /* The velocity since the last point, in 10 ms steps. */
    for (c = e->t; c < t; ++c) {
        double tl = ((double)(c - e->lo_cs) + 0.5) * 0.01, a = vm_saturn_acc(tl) * 0.01,
               th = vm_saturn_pitch(tl);
        f[0] += a * sin(th);
        f[2] -= a * cos(th);
    }
    e->t = t;
    vm_pipa_carry(e->carry, f, w);
}

/* FLIGHT_SIVB=CS,UX,UY,UZ (CM): the S-IVB's second burn, translunar
 * injection, from CS on the AGC clock: 0.9 MN on 140 t less 215 kg/s for
 * 347 s (about 3,190 m/s), along the unit vector (UX,UY,UZ) in
 * stable-member axes (an inertially fixed attitude; the CSM rides on the
 * S-IVB, so the CDUs stay put). Specific force into PIPAX-Z as for
 * FLIGHT_SATURN. tools/flight_scenarios/cm_tli.py gives the vector. */
typedef struct {
    unsigned long t;
    double carry[3];
} vm_sivb;

VM_FN void vm_sivb_step(unsigned long t, vm_sivb *e, agc_word *w)
{
    double f[3] = {0.0, 0.0, 0.0};
    unsigned long c;
    unsigned k;
    if (!vm_cfg.sivb_tig) return;
    for (c = e->t > vm_cfg.sivb_tig ? e->t : vm_cfg.sivb_tig; c < t; ++c) {
        double tl = ((double)(c - vm_cfg.sivb_tig) + 0.5) * 0.01, a;
        if (tl >= 347.0) break;
        a = 0.9e6 / (140e3 - 215.0 * tl) * 0.01;
        for (k = 0; k < 3u; ++k) f[k] += a * vm_cfg.sivb_u[k];
    }
    e->t = t;
    vm_pipa_carry(e->carry, f, w);
}

#endif
