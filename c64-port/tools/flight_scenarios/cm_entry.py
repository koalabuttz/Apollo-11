#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM entry scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"): the
erasable words to set at 20 s after a cold start with the IMU operating, so
V37E61E runs P61 (S61.1 starts SERVICER from the CSM state vector), P62
(CM/SM separation, entry attitude) and P63, which waits for 0.05 g; with
FLIGHT_AERO the drag then takes it through P64 to P67.

The values are synthetic but self-consistent (no Apollo 11 entry pad is
reachable here): a lunar-return entry interface at 400,000 ft, 11.03 km/s,
-6.5 degrees, reached BACK seconds after the load; the CSM state vector
two-body propagated back to the load time; REFSMMAT Y along the orbit
normal (the S61.1 test), Z down at EI, X forward; the splash point
RANGE km down the ground track at the predicted landing time (AZO, AXO,
AYO and TEPHEM 0: the earth-fixed frame is the reference frame turned by
WEARTH t about Z). Scales: earth positions B29 m, velocities B7 m/cs.

  cm_entry.py [OUT]   (default tools/flight_scenarios/cm-entry.load)
"""
import math, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Comanche055/assembly.lst'
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


out = {}


def put(name, words, off=0):
    for i, w in enumerate(words):
        out[addr(name, off) + i] = w


def vec(name, v, scale):
    put(name, sum((dp(c / 2**scale) for c in v), []))


def flag(n, on=True):
    a = addr('FLAGWRD0') + n // 15
    bit = 1 << (14 - n % 15)
    v = out.get(a, 0o200 if a == addr('FLAGWRD5') else 0)
    out[a] = (v | bit) if on else (v & ~bit)


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def unit(a):
    n = math.sqrt(sum(c * c for c in a))
    return [c / n for c in a]


MU = 3.986032e14        # m^3/s^2 (Comanche's earth MU)
RE = 6373338.0          # m, the pad radius ERAD
WE = 7.29211494e-5      # rad/s, WEARTH
T_LOAD = 2000           # cs
BACK = 150.0            # s from the load to EI
RANGE = 2400e3          # m, EI to splash along the ground
FLIGHT = 900.0          # s, EI to splash (the earth turns under it)
INC = math.radians(30.0)

# EI: 400,000 ft above ERAD, 11.03 km/s, flight path -6.5 deg, on a 30 deg
# inclined plane, at the ascending node's right ascension 0, 40 deg past it.
r_ei = RE + 121920.0
v_ei, gam = 11030.0, math.radians(-6.5)
node = [1.0, 0.0, 0.0]
inplane = [0.0, math.cos(INC), math.sin(INC)]
u = math.radians(40.0)
R_EI = [r_ei * (math.cos(u) * node[i] + math.sin(u) * inplane[i]) for i in range(3)]
T_EI = [-math.sin(u) * node[i] + math.cos(u) * inplane[i] for i in range(3)]
up = unit(R_EI)
V_EI = [v_ei * (math.sin(gam) * up[i] + math.cos(gam) * T_EI[i]) for i in range(3)]


def accel(r):
    n = math.sqrt(sum(c * c for c in r))
    return [-MU * c / n**3 for c in r]


def propagate(r, v, t, h=0.5):
    """Two-body RK4 for t seconds (negative: backwards)."""
    n = int(abs(t) / h + 0.5); h = t / n
    for _ in range(n):
        k1v = accel(r); k1r = v
        k2v = accel([r[i] + h / 2 * k1r[i] for i in range(3)]); k2r = [v[i] + h / 2 * k1v[i] for i in range(3)]
        k3v = accel([r[i] + h / 2 * k2r[i] for i in range(3)]); k3r = [v[i] + h / 2 * k2v[i] for i in range(3)]
        k4v = accel([r[i] + h * k3r[i] for i in range(3)]); k4r = [v[i] + h * k3v[i] for i in range(3)]
        r = [r[i] + h / 6 * (k1r[i] + 2 * k2r[i] + 2 * k3r[i] + k4r[i]) for i in range(3)]
        v = [v[i] + h / 6 * (k1v[i] + 2 * k2v[i] + 2 * k3v[i] + k4v[i]) for i in range(3)]
    return r, v


R, V = propagate(R_EI, V_EI, -BACK)
print('load: altitude %.1f km, speed %.1f m/s' % ((math.sqrt(sum(c * c for c in R)) - RE) / 1e3,
                                                   math.sqrt(sum(c * c for c in V))))
Vcs = [c * 0.01 for c in V]
# Permanent CSM state: RRECT, VRECT, TET, DELTA=0, NUV=0, RCV, VCV, TC=0, XKEP=0.
vec('RRECTCSM', R, 29); vec('VRECTCSM', Vcs, 7)
put('TETCSM', dp(T_LOAD / 2**28))
put('DELTACSM', [0] * 6); put('NUVCSM', [0] * 6)
vec('RCVCSM', R, 29); vec('VCVCSM', Vcs, 7)
put('TCCSM', [0, 0]); put('XKEPCSM', [0, 0])
flag(123, False)        # CMOONFLG: earth sphere

# REFSMMAT: Y along the orbit normal (S61.1 wants it within 30 deg of
# unit(VN x RN)), Z down at EI, X = Y x Z (forward).
Y = unit(cross(V_EI, R_EI))
Z = [-c for c in up]
X = cross(Y, Z)
put('REFSMMAT', sum((dp(c / 2) for row in (X, Y, Z) for c in row), []))
flag(47)                # REFSMFLG

# Splash: RANGE down the ground track from EI, then into earth-fixed axes
# at the landing time (geocentric latitude).
th = RANGE / RE
S = [math.cos(th) * up[i] + math.sin(th) * T_EI[i] for i in range(3)]
t_land = T_LOAD / 100 + BACK + FLIGHT
lat = math.asin(S[2])
lng = math.atan2(S[1], S[0]) - WE * t_land
lng = (lng + math.pi) % (2 * math.pi) - math.pi
print('splash lat %.3f lng %.3f deg' % (math.degrees(lat), math.degrees(lng)))
put('LAT(SPL)', dp(lat / (2 * math.pi)))
put('LNG(SPL)', dp(lng / (2 * math.pi)))

# Entry pads: EMS 0.05 g altitude, trim angle, nominal and final L/D.
put('EMSALT', dp(297431 * 0.3048 / 2**29))
out[addr('ALFAPAD')] = sp(-20.0 / 180)
out[addr('LADPAD')] = sp(0.3)
out[addr('LODPAD')] = sp(0.18)

out_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/cm-entry.load'
with open(out_path, 'w') as f:
    f.write('# CM entry scenario load at 20 s (tools/flight_scenarios/cm_entry.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
print('words', len(out))
