#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""LM coelliptic rendezvous scenario loads (docs/FLIGHT-STORE.md, "Flight
scenarios"): the erasable words to set at 20 s after a cold start with the
IMU operating, for the LM's rendezvous targeting programs before TPI: P32
(CSI) and P33 (CDH), and the CSM-active P72 and P73.

Shaped like Apollo 11's rendezvous after the LM's insertion: the CSM in a
60 nmi (111 km) circular orbit, the LM in a 9 x 45 nmi orbit behind and
below it, ten minutes before apolune, where CSI comes. The plane, frame,
REFSMMAT and R22 pads are lm_rendezvous.py's. TTPI is 97 minutes after
apolune (Apollo 11: CSI to CDH 58 min, CDH to TPI 39 min).

BURN=T:X,Y,Z (repeatable) changes the LM's orbit by an impulse at AGC time
T centiseconds, X, Y, Z in ft/s in the local vertical frame (X forward
horizontal, Y the negative orbit normal, Z down: P32's and P33's N81),
and loads the LM's state on the new orbit at 20 s, as if that burn had
already been made; CSMBURN=T:X,Y,Z the same for the CSM (P72's CSI, for
P73). LEAD=DEG is the CSM's lead at the load (default 9.5). The truth
printed for FLIGHT_RR (tools/flight_radar.h) follows.

  lm_coelliptic.py OUT [LEAD=DEG] [BURN=T:X,Y,Z ...] [CSMBURN=T:X,Y,Z ...]
"""
import math, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Luminary099/assembly.lst'
SYMS = {}
for line in open(LST, errors='replace'):
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+(\S+)', line):
        SYMS[m.group(2)] = m.group(3)


def addr(name, off=0):
    s = SYMS[name]
    if ',' in s:
        b, a = s.split(',')
        a = int(b[1:]) * 0o400 + int(a, 8) - 0o1400
    else:
        a = int(s, 8)
    return a + off


def sp(f):
    """Single precision fraction -> AGC word (ones' complement)."""
    n = int(round(f * 2**14))
    assert -0o37777 <= n <= 0o37777, f
    return n & 0o77777 if n >= 0 else (~(-n)) & 0o77777


def dp(f):
    n = int(round(f * 2**28))
    assert abs(n) < 2**28, f
    m = abs(n)
    hi, lo = m >> 14, m & 0o37777
    if n < 0:
        hi, lo = (~hi) & 0o77777, (~lo) & 0o77777
    return [hi, lo]


# FLAGWRD0-9 at 20 s after a cold start with the IMU operating.
BASE_FLAGS = [0, 0, 0o40000, 0o2000, 0, 0, 0, 0o100, 0, 0]
base = {addr('FLAGWRD0') + i: v for i, v in enumerate(BASE_FLAGS)}
out = {}


def put(name, words, off=0):
    for i, w in enumerate(words):
        out[addr(name, off) + i] = w


def vec(name, v, scale):
    put(name, sum((dp(c / 2**scale) for c in v), []))


def flag(n, on=True):
    a = addr('FLAGWRD0') + n // 15
    bit = 1 << (14 - n % 15)
    v = out.get(a, base[a])
    out[a] = (v | bit) if on else (v & ~bit)


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def unit(a):
    n = math.sqrt(dot(a, a))
    return [c / n for c in a]


MU = 4.9028e12          # m^3/s^2, as tools/flight_radar.h
RMOON = 1738e3
CS = 0.01
T_LOAD = 2000           # cs
NMI = 1852.0
FPS = 0.3048

# lm_rendezvous.py's frame: P up through the P63 landing site, Q downrange;
# the orbits run retrograde about the reference Z axis.
land = [-1270721.0, 1087493.0, 463124.0]
P = unit(land)
Q = unit(cross(P, [0, 0, 1.0]))


def kepler(s, dt):
    """The conic state dt seconds after s (r, v), as flight_radar.h's
    rr_kepler: Battin's f and g in the eccentric anomaly difference."""
    r, v = s
    r0 = math.sqrt(dot(r, r))
    a = 1.0 / (2.0 / r0 - dot(v, v) / MU)
    sg = dot(r, v) / math.sqrt(MU)
    n = math.sqrt(MU / a**3)
    sa = math.sqrt(a)
    de = n * dt
    for _ in range(50):
        fn = de + sg / sa * (1 - math.cos(de)) - (1 - r0 / a) * math.sin(de) - n * dt
        de -= fn / (1 + sg / sa * math.sin(de) - (1 - r0 / a) * math.cos(de))
    rr = a + (r0 - a) * math.cos(de) + sg * sa * math.sin(de)
    f = 1 - a / r0 * (1 - math.cos(de))
    g = a * sg / math.sqrt(MU) * (1 - math.cos(de)) + r0 * math.sqrt(a / MU) * math.sin(de)
    fd = -math.sqrt(MU * a) / (rr * r0) * math.sin(de)
    gd = 1 - a / rr * (1 - math.cos(de))
    return ([f * r[i] + g * v[i] for i in range(3)], [fd * r[i] + gd * v[i] for i in range(3)])


def in_plane(radius, angle, speed_r, speed_t):
    """Position at angle from P toward Q, with radial and transverse speed."""
    u = [math.cos(angle) * P[i] + math.sin(angle) * Q[i] for i in range(3)]
    t = [-math.sin(angle) * P[i] + math.cos(angle) * Q[i] for i in range(3)]
    return [radius * c for c in u], [speed_r * u[i] + speed_t * t[i] for i in range(3)]


# The CSM: 60 nmi circular. The LM: 9 x 45 nmi, ten minutes before
# apolune at the load, 40 degrees past the landing site's meridian.
R_CSM = RMOON + 60 * NMI
RP, RA = RMOON + 9 * NMI, RMOON + 45 * NMI
A = (RP + RA) / 2
E = (RA - RP) / (RA + RP)
N_LM = math.sqrt(MU / A**3)
T_APO = 600.0                     # s from the load to apolune
M0 = math.pi - N_LM * T_APO       # mean anomaly at the load
ea = M0
for _ in range(50):
    ea -= (ea - E * math.sin(ea) - M0) / (1 - E * math.cos(ea))
nu = 2 * math.atan2(math.sqrt(1 + E) * math.sin(ea / 2), math.sqrt(1 - E) * math.cos(ea / 2))
r0 = A * (1 - E * math.cos(ea))
h = math.sqrt(MU * A * (1 - E * E))
LM_ANGLE = math.radians(40.0)
lm = in_plane(r0, LM_ANGLE, MU / h * E * math.sin(nu), h / r0)
# The CSM leads by LEAD (degrees, chosen so CSI comes out near Apollo 11's
# 51.5 ft/s with TPI 97 minutes after apolune).
LEAD = math.radians(float(next((a[5:] for a in sys.argv[2:] if a.startswith('LEAD=')), '9.5')))
csm = in_plane(R_CSM, LM_ANGLE + LEAD, 0.0, math.sqrt(MU / R_CSM))

# Burns already made: propagate to each, add the impulse, propagate back.
def burned(s, arg):
    t, d = arg.split(':')
    dt = (int(t) - T_LOAD) * CS
    r, v = kepler(s, dt)
    z = [-c for c in unit(r)]
    y = unit(cross(v, r))
    x = cross(y, z)
    dv = [float(c) * FPS for c in d.split(',')]
    v = [v[i] + dv[0] * x[i] + dv[1] * y[i] + dv[2] * z[i] for i in range(3)]
    return kepler((r, v), -dt)


for arg in sys.argv[2:]:
    if arg.startswith('BURN='):
        lm = burned(lm, arg[5:])
    elif arg.startswith('CSMBURN='):
        csm = burned(csm, arg[8:])

rl, vl = lm
rc, vc = csm
# REFSMMAT: the LM's local vertical at the load (X up, Y the negative
# orbit normal, Z along the velocity), as lm_rendezvous.py.
X = unit(rl)
Z = unit([vl[i] - dot(vl, X) * X[i] for i in range(3)])
Y = cross(Z, X)
put('REFSMMAT', sum((dp(c / 2) for row in (X, Y, Z) for c in row), []))
flag(47)                # REFSMFLG

for veh, (r, v) in (('LEM', (rl, vl)), ('CSM', (rc, vc))):
    V = [c * CS for c in v]
    vec('RRECT' + veh, r, 27); vec('VRECT' + veh, V, 5)
    put('TET' + veh, dp(T_LOAD / 2**28))
    put('DELTA' + veh, [0] * 6); put('NUV' + veh, [0] * 6)
    vec('RCV' + veh, r, 27); vec('VCV' + veh, V, 5)
    put('TC' + veh, [0, 0]); put('XKEP' + veh, [0, 0])
flag(124)               # LMOONFLG
flag(123)               # CMOONFLG
flag(40)                # AVFLAG: the LM is the active vehicle

# R22's pads, as lm_rendezvous.py.
out[addr('WRENDPOS')] = sp(3048.0 / 2**14)
out[addr('WRENDVEL')] = sp(0.03048)
out[addr('WSHAFT')] = sp(0.015 * 2**5)
out[addr('WTRUN')] = sp(0.015 * 2**5)
put('RANGEVAR', dp(0.001**2 * 2**12))
put('RATEVAR', dp(0.005**2 * 2**12))
out[addr('RVARMIN')] = 4 * 20**2
out[addr('VVARMIN')] = int(round((0.3 * CS)**2 * 2**26))
out[addr('RMAX')] = sp(609.6 / 2**19)
out[addr('VMAX')] = sp(0.6096 * CS / 2**7)
out[addr('LEMMASS')] = 2800 // 4
out[addr('CSMMASS')] = 16500 // 4

# The pads P32 shows: N55 (NN 1: CDH at the first apsidal crossing after
# CSI; TPI at 26.6 degrees elevation) and N37 (TTPI).
T_TPI = T_LOAD + int((T_APO + 97 * 60) / CS)
put('TTPI', dp(T_TPI / 2**28))
put('TTPIO', dp(T_TPI / 2**28))   # P33 takes TTPI from P32's TTPIO
put('TIG', dp(T_TPI / 2**28))
# ELEV=DEG: the TPI elevation, 26.6 seen from the LM; the CSM-active
# programs (P72-P74) look down at the LM, 180 + 26.6.
ELEV = float(next((a[5:] for a in sys.argv[2:] if a.startswith('ELEV=')), '26.6'))
put('ELEV', dp(ELEV / 360))
put('CENTANG', dp(130.0 / 360))
put('NN', [0, 1])

with open(sys.argv[1], 'w') as f:
    f.write('# LM coelliptic rendezvous load at 20 s (%s)\n'
            % ' '.join(['tools/flight_scenarios/lm_coelliptic.py'] + sys.argv[2:]))
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
rr = lambda r, v: ','.join('%.3f' % c for c in r + v)
print('words', len(out), 'apolune at', T_LOAD + int(T_APO / CS), 'cs; TTPI', T_TPI,
      'cs; range %.1f km' % (math.dist(rl, rc) / 1000))
print('FLIGHT_RR=%d:%s:%s' % (T_LOAD, rr(rl, vl), rr(rc, vc)))
