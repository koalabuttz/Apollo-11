#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM TLI scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"): Apollo
11's translunar injection, the S-IVB's second burn at 02:44:16 GET, as the
CMC monitored it: P47 (thrust monitor) counting the S-IVB's velocity on
V16N83, then V82.

The values are synthetic but self-consistent, built as cm_p52.py builds its
earth-orbit state:

- the launch REFSMMAT (Z down and X along the 72 degree launch azimuth at
  the pad at liftoff, 1969-07-16 13:32:00 UTC), the platform on it (P52
  has taken the drift out);
- the CSM in the circular 100 nmi parking orbit in the launch plane, at
  the point it reaches at LOAD_GET, TIG_CS - T_LOAD centiseconds before TIG;
- the S-IVB's thrust inertially fixed along the orbit's velocity half a
  burn after TIG, and the CSM's body X axis along it (the CDUs), rolled to
  keep the middle gimbal small;
- TEPHEM for the launch time, and the sun and moon as cm_p52.py has them.

Prints the burn's thrust vector in stable-member axes for the harness's
FLIGHT_SIVB=TIG_CS,UX,UY,UZ (tools/flight_host_run.c), integrates the same
burn (two-body earth gravity) and prints the injection state.

  cm_tli.py [OUT]   (default tools/flight_scenarios/cm-tli.load)
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
T_LOAD = 2000           # cs after the cold start
TIG_CS = 30000          # cs: TIG on the AGC clock
TIG_GET = 2 * 3600 + 44 * 60 + 16.0
LOAD_GET = TIG_GET - (TIG_CS - T_LOAD) / 100
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
PAD_LAT, PAD_LON, AZ = 28.608, -80.604, 72.0
R_ORBIT = 6378137.0 + 100.0 * 1852
INSERT_GET, INSERT_RANGE = 709.0, 21.0
# The S-IVB burn, as FLIGHT_SIVB gives it.
F_SIVB, M0, MDOT, BURN_S = 0.9e6, 140e3, 215.0, 347.0

gmst = (280.46061837 + 360.98564736629 * (JD_LIFTOFF - 2451545.0)) % 360
th = math.radians(gmst + PAD_LON)
la = math.radians(PAD_LAT)
up = [math.cos(la) * math.cos(th), math.cos(la) * math.sin(th), math.sin(la)]
east = [-math.sin(th), math.cos(th), 0.0]
north = cross(up, east)
az = math.radians(AZ)
down_range = [math.cos(az) * north[i] + math.sin(az) * east[i] for i in range(3)]
REFSMMAT = [down_range, cross([-c for c in up], down_range), [-c for c in up]]

h = unit(cross(up, down_range))
n_rate = math.sqrt(MU / R_ORBIT**3)
u0 = math.radians(INSERT_RANGE) - n_rate * INSERT_GET
along = cross(h, up)


def state(get):
    u = u0 + n_rate * get
    r = [R_ORBIT * (math.cos(u) * up[i] + math.sin(u) * along[i]) for i in range(3)]
    v = [R_ORBIT * n_rate * (-math.sin(u) * up[i] + math.cos(u) * along[i]) for i in range(3)]
    return r, v


R, V = state(LOAD_GET)
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
put('REFSMMAT', sum((dp(c / 2) for row in REFSMMAT for c in row), []))
flag(47)                # REFSMFLG

tephem = TEPHEM_CS + int(round(LOAD_GET * 100)) - T_LOAD
put('TEPHEM', tp(tephem))
put('TIMEMO', tp(tephem + T_LOAD))
EPS = math.radians(23.443)
JD_LOAD = JD_LIFTOFF + LOAD_GET / 86400


def ecliptic(lon, lat=0.0):
    lo, la_ = math.radians(lon), math.radians(lat)
    x, y, z = math.cos(la_) * math.cos(lo), math.cos(la_) * math.sin(lo), math.sin(la_)
    return [x, y * math.cos(EPS) - z * math.sin(EPS), y * math.sin(EPS) + z * math.cos(EPS)]


d = JD_LOAD - 2451545.0
g = math.radians((357.529 + 0.98560028 * d) % 360)
sun_lon = (280.459 + 0.98564736 * d + 1.915 * math.sin(g) + 0.020 * math.sin(2 * g)) % 360
sun_au = 1.00014 - 0.01671 * math.cos(g) - 0.00014 * math.cos(2 * g)
vec('RESO', [c * sun_au * 1.495978707e11 for c in ecliptic(sun_lon)], 38)
vec('VESO', [c * 0.25 for c in ecliptic(sun_lon + 90)], 0)
put('OMEGAES', dp(1 / (365.2422 * 86400 * 100) * 2**26))
moon_lon = (218.316 + 13.176396 * d) % 360
moon_lat = 5.13 * math.sin(math.radians((93.272 + 13.229350 * d) % 360))
for k in range(10):
    put('VECOEM', [0] * 6, off=6 * k)
put('VECOEM', sum((dp(c * 3.9e8 / 2**31) for c in ecliptic(moon_lon, moon_lat)), []), off=54)
put('VECOEM', sum((dp(c * 1022.0 * 0.01 / 2**5) for c in ecliptic(moon_lon + 90)), []), off=48)

# The thrust: inertially fixed along the velocity half a burn after TIG.
U = unit(state(TIG_GET + BURN_S / 2)[1])
U_SM = mxv(REFSMMAT, U)


def gimbals(nb_ref, sm_ref):
    """CDU angles (outer X, inner Y, middle Z, degrees), NB from SM =
    Rx(o) Rz(m) Ry(i)."""
    m = mxm(nb_ref, tr(sm_ref))
    mg = math.degrees(math.asin(max(-1, min(1, m[0][1]))))
    ig = math.degrees(math.atan2(-m[0][2], m[0][0]))
    og = math.degrees(math.atan2(-m[2][1], m[1][1]))
    return og, ig, mg


def attitude(roll):
    """Body axes in reference axes: X along U, rolled about it from the
    orbit normal."""
    y0 = unit(cross(h, U))
    z0 = cross(U, y0)
    rr = math.radians(roll)
    y = [math.cos(rr) * y0[i] + math.sin(rr) * z0[i] for i in range(3)]
    return [U, y, cross(U, y)]


best = min(range(0, 360, 5), key=lambda r: abs(gimbals(attitude(r), REFSMMAT)[2]))
og, ig, mg = gimbals(attitude(best), REFSMMAT)
put('CDUX', [cdu(og)])
put('CDUY', [cdu(ig)])
put('CDUZ', [cdu(mg)])

# The burn, integrated as the harness gives it.
r, v = state(TIG_GET)
t, dt = 0.0, 0.01
while t < BURN_S:
    a = F_SIVB / (M0 - MDOT * (t + dt / 2))
    rr = math.sqrt(dot(r, r))
    v = [v[i] + (a * U[i] - MU * r[i] / rr**3) * dt for i in range(3)]
    r = [r[i] + v[i] * dt for i in range(3)]
    t += dt
rr, vv = math.sqrt(dot(r, r)), math.sqrt(dot(v, v))
energy = vv * vv / 2 - MU / rr
sma = -MU / (2 * energy)
hh = math.sqrt(dot(cross(r, v), cross(r, v)))
ecc = math.sqrt(max(0.0, 1 + 2 * energy * hh * hh / MU**2))
print('roll %d: gimbals outer %.3f inner %.3f middle %.3f deg' % (best, og, ig, mg))
print('FLIGHT_SIVB=%d,%.6f,%.6f,%.6f' % (TIG_CS, *U_SM))
print('cutoff: %.1f km up, %.0f m/s (%.0f ft/s), apogee %.0f km, e %.4f, C3 %.2f km2/s2'
      % ((rr - 6378137) / 1e3, vv, vv / 0.3048, (sma * (1 + ecc) - 6378137) / 1e3 if ecc < 1 else float('inf'),
         ecc, 2 * energy / 1e6))
path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/cm-tli.load'
with open(path, 'w') as f:
    f.write('# CM P47 (TLI) scenario load at 20 s (tools/flight_scenarios/cm_tli.py)\n')
    for a_, w in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, w))
