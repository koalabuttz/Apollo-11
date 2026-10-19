#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM P37 scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"): return
to earth targeting on Apollo 11's transearth coast, as the crew ran it for
midcourse correction and abort pads (synthetic values, about 150 h GET).

The values are synthetic but self-consistent, built as cm_tli.py builds its
earth state:

- the CSM on a two-body earth ellipse from the moon's distance whose
  natural entry flight path angle at 400,000 ft is GAMMA_EI, inbound at
  R_LOAD from the earth (outside the moon's sphere, CMOONFLG clear);
- a REFSMMAT in local vertical axes at the load, the platform on it;
- TEPHEM for the GET, and the sun and moon as cm_p52.py has them.

The keys (tools/flight_host_differential.py) give P37 (V37E37E) a TIG of
1 h on the CMC clock on V06N33, and a 100 ft/s DV with a -6.50 degree
reentry angle on V06N60; with zeros there the conic iteration does not
converge (alarm 00605).

  cm_p37.py [OUT]   (default tools/flight_scenarios/cm-p37.load)
"""
import math
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Comanche055/assembly.lst'
SYMS = {}
for line in open(LST, errors='replace'):
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+(\S+)', line):
        SYMS[m.group(2)] = m.group(3)
COUNTER = {'CDUX': 0o32, 'CDUY': 0o33, 'CDUZ': 0o34}


def addr(name, off=0):
    if name in COUNTER:
        return COUNTER[name] + off
    s = SYMS[name]
    if ',' in s:
        b, a = s.split(',')
        a = int(b[1:]) * 0o400 + int(a, 8) - 0o1400
    else:
        a = int(s, 8)
    return a + off


def dp(f):
    n = int(round(f * 2**28))
    assert abs(n) < 2**28, f
    m = abs(n)
    hi, lo = m >> 14, m & 0o37777
    if n < 0:
        hi, lo = (~hi) & 0o77777, (~lo) & 0o77777
    return [hi, lo]


def tp(n):
    assert 0 <= n < 2**42
    return [n >> 28, (n >> 14) & 0o37777, n & 0o37777]


def cdu(deg):
    return int(round(deg / 360.0 * 2**15)) % 2**15


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def unit(a):
    n = math.sqrt(dot(a, a))
    return [c / n for c in a]


def mxv(m, v):
    return [dot(r, v) for r in m]


def mxm(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]


def tr(m):
    return [list(r) for r in zip(*m)]


def rot(axis, deg):
    c, s = math.cos(math.radians(deg)), math.sin(math.radians(deg))
    i, j = [(1, 2), (2, 0), (0, 1)][axis]
    m = [[1.0 if r == k else 0.0 for k in range(3)] for r in range(3)]
    m[i][i] = m[j][j] = c
    m[i][j], m[j][i] = s, -s
    return m


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


MU = 3.986032e14        # m^3/s^2 (Comanche's earth MU)
RE = 6378137.0
T_LOAD = 2000           # cs after the cold start
LOAD_GET = 150 * 3600.0
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
R_APO = 400000e3        # m: the ellipse from the moon's distance
R_EI = RE + 400000 * 0.3048
GAMMA_EI = -6.5         # deg at 400,000 ft
R_LOAD = 240000e3       # m from the earth at the load


def fpa_at_ei(rp):
    a = (R_APO + rp) / 2
    h = math.sqrt(MU * 2 * R_APO * rp / (R_APO + rp))
    v = math.sqrt(MU * (2 / R_EI - 1 / a))
    return -math.degrees(math.acos(min(1.0, h / (R_EI * v))))


lo, hi = RE - 2000e3, R_EI
for _ in range(100):
    mid = (lo + hi) / 2
    if fpa_at_ei(mid) < GAMMA_EI:
        lo = mid
    else:
        hi = mid
RP = (lo + hi) / 2
A = (R_APO + RP) / 2
ECC = (R_APO - RP) / (R_APO + RP)
P = A * (1 - ECC**2)

EPS = math.radians(23.443)
JD_LOAD = JD_LIFTOFF + LOAD_GET / 86400


def ecliptic(lon, lat=0.0):
    lo_, la_ = math.radians(lon), math.radians(lat)
    x, y, z = math.cos(la_) * math.cos(lo_), math.cos(la_) * math.sin(lo_), math.sin(la_)
    return [x, y * math.cos(EPS) - z * math.sin(EPS), y * math.sin(EPS) + z * math.cos(EPS)]


d = JD_LOAD - 2451545.0
moon_lon = (218.316 + 13.176396 * d) % 360
moon_lat = 5.13 * math.sin(math.radians((93.272 + 13.229350 * d) % 360))
MOON = [c * 3.9e8 for c in ecliptic(moon_lon, moon_lat)]
MOON_V = [c * 1022.0 * 0.01 for c in ecliptic(moon_lon + 90)]
# The ellipse in the moon's orbit plane, prograde, apogee where the moon
# was at TEI (the moon moves 13.2 degrees a day), so the CSM falls back
# toward the earth from behind it.
H_HAT = unit(cross(MOON, MOON_V))
apo_lon = moon_lon - 13.176396 * (LOAD_GET - (135 * 3600 + 23 * 60)) / 86400
X_APO = unit(ecliptic(apo_lon, moon_lat))
X_APO = unit([X_APO[i] - dot(X_APO, H_HAT) * H_HAT[i] for i in range(3)])
P_HAT = [-c for c in X_APO]          # toward perigee
Q_HAT = cross(H_HAT, P_HAT)
# True anomaly at R_LOAD, inbound (negative: before perigee).
NU = -math.acos((P / R_LOAD - 1) / ECC)
R = [R_LOAD * (math.cos(NU) * P_HAT[i] + math.sin(NU) * Q_HAT[i]) for i in range(3)]
vr = math.sqrt(MU / P) * ECC * math.sin(NU)
vt = math.sqrt(MU / P) * (1 + ECC * math.cos(NU))
V = [vr * R[i] / R_LOAD + vt * cross(H_HAT, R)[i] / R_LOAD for i in range(3)]
moon_dist = math.sqrt(sum((R[i] - MOON[i])**2 for i in range(3)))
assert moon_dist > 1.0e8, moon_dist

Vcs = [c * 0.01 for c in V]
vec('RRECTCSM', R, 29)
vec('VRECTCSM', Vcs, 7)
put('TETCSM', dp(T_LOAD / 2**28))
put('DELTACSM', [0] * 6)
put('NUVCSM', [0] * 6)
vec('RCVCSM', R, 29)
vec('VCVCSM', Vcs, 7)
put('TCCSM', [0, 0])
put('XKEPCSM', [0, 0])
flag(123, False)        # CMOONFLG: earth sphere
LV_Z = [-c / R_LOAD for c in R]
LV_Y = [-c for c in H_HAT]
LV_X = cross(LV_Y, LV_Z)
REFSMMAT = [LV_X, LV_Y, LV_Z]
put('REFSMMAT', sum((dp(c / 2) for row in REFSMMAT for c in row), []))
flag(47)                # REFSMFLG

tephem = TEPHEM_CS + int(round(LOAD_GET * 100)) - T_LOAD
put('TEPHEM', tp(tephem))
put('TIMEMO', tp(tephem + T_LOAD))
g = math.radians((357.529 + 0.98560028 * d) % 360)
sun_lon = (280.459 + 0.98564736 * d + 1.915 * math.sin(g) + 0.020 * math.sin(2 * g)) % 360
sun_au = 1.00014 - 0.01671 * math.cos(g) - 0.00014 * math.cos(2 * g)
vec('RESO', [c * sun_au * 1.495978707e11 for c in ecliptic(sun_lon)], 38)
vec('VESO', [c * 0.25 for c in ecliptic(sun_lon + 90)], 0)
put('OMEGAES', dp(1 / (365.2422 * 86400 * 100) * 2**26))
for k in range(10):
    put('VECOEM', [0] * 6, off=6 * k)
put('VECOEM', sum((dp(c / 2**31) for c in MOON), []), off=54)
put('VECOEM', sum((dp(c / 2**5) for c in MOON_V), []), off=48)

# Time to entry interface (Kepler, from the load).
def ecc_anom(nu):
    return 2 * math.atan(math.sqrt((1 - ECC) / (1 + ECC)) * math.tan(nu / 2))


def t_from_peri(nu):
    E = ecc_anom(nu)
    return (E - ECC * math.sin(E)) * math.sqrt(A**3 / MU)


nu_ei = -math.acos((P / R_EI - 1) / ECC)
t_ei = t_from_peri(nu_ei) - t_from_peri(NU)
print('ellipse %.0f x %.0f km (perigee %.1f km up), entry FPA %.2f deg' % (
    (R_APO - RE) / 1e3, (RP - RE) / 1e3, (RP - RE) / 1e3, fpa_at_ei(RP)))
print('load: %.0f km from earth, %.0f m/s, %.0f km from the moon; entry interface %.2f h after the load' % (
    R_LOAD / 1e3, math.sqrt(dot(V, V)), moon_dist / 1e3, t_ei / 3600))
path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/cm-p37.load'
with open(path, 'w') as f:
    f.write('# CM P37 (return to earth) scenario load at 20 s (tools/flight_scenarios/cm_p37.py)\n')
    for a_, w in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, w))
print('words', len(out))
