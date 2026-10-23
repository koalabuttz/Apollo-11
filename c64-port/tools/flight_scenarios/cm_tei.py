#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM TEI scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"): Apollo
11's transearth injection, the SPS burn on the far side that left lunar
orbit (TIG 135:23:42 GET, 3,283.6 ft/s, about 2.5 minutes), set up as
cm_p40.py sets up LOI-1 (the same TVC and SPS pads and FLIGHT_SPS model):

- the CSM alone (the LM jettisoned) in a circular 60 nmi lunar orbit,
  retrograde to the moon's orbit, at TIG on the far side; the CMC's state
  is the true one (CMOONFLG set);
- the TEI-1 pad keyed in P30: V25 on N33 for TIG (on the CMC clock, 300 s,
  which TEPHEM makes 135:23:42.28 GET) and V25 on N81 for +3283.6 ft/s
  along local horizontal; TIG and DELVSLV are not loaded;
- a REFSMMAT with X along the DV (prograde) and Z toward the moon, the
  vehicle at the ignition attitude S40.1 computes from it (and the RCS
  DAP's CDUXD-ZD there too), so P40's R60 maneuver is nil;
- RLS, so N42 and N44 show altitudes above the lunar radius;
- TEPHEM so the AGC clock's 300 s is TIG, and the sun and moon pads.

  cm_tei.py [OUT]   (default tools/flight_scenarios/cm-tei.load)
"""
import math, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Comanche055/assembly.lst'
SYMS = {}
for line in open(LST, errors='replace'):
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+(\S+)', line):
        SYMS[m.group(2)] = m.group(3)
COUNTER = {'CDUX': 0o32, 'CDUY': 0o33, 'CDUZ': 0o34, 'CDUT': 0o35, 'CDUS': 0o36}


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


def sp(n):
    """A signed integer as one AGC word (ones' complement)."""
    assert -0o37777 <= n <= 0o37777, n
    return n & 0o77777 if n >= 0 else (~(-n)) & 0o77777


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


def norm(a):
    return math.sqrt(dot(a, a))


def unit(a):
    n = norm(a)
    return [c / n for c in a]


def lin(*terms):
    return [sum(k * v[i] for k, v in terms) for i in range(3)]


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


MUM = 4.902778e12       # m^3/s^2 (Comanche's moon MU)
R_MOON = 1738090.0      # 504RM
T_LOAD = 2000           # cs after the cold start
TIG_CS = 30000          # cs: TIG on the AGC clock
TIG_GET = 135 * 3600 + 23 * 60 + 42.28
LOAD_GET = TIG_GET - (TIG_CS - T_LOAD) / 100
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
DV_FPS = 3283.6         # TEI-1, N81 DVX
DV = DV_FPS * 0.3048
ALT = 60 * 1852.0       # circular orbit
SPS_THRUST, SPS_VEX = 91188.0, 3080.0   # N, m/s (FLIGHT_SPS)
CSM_KG = 16400.0        # the CSM at TEI (LM jettisoned)
BURN_S = CSM_KG * SPS_VEX / SPS_THRUST * (1 - math.exp(-DV / SPS_VEX))
# S40.1's ignition attitude (outer, inner, middle CDU): the angles R60's
# V50N18 asks for from this platform (0.00, 357.60, 359.04 degrees).
IGN_CDU = (0o6, 0o77663, 0o77460)

RO = R_MOON + ALT
VO = math.sqrt(MUM / RO)
N_ORB = VO / RO
EPS = math.radians(23.443)
JD_TIG = JD_LIFTOFF + TIG_GET / 86400


def ecliptic(lon, lat=0.0):
    lo, la_ = math.radians(lon), math.radians(lat)
    x, y, z = math.cos(la_) * math.cos(lo), math.cos(la_) * math.sin(lo), math.sin(la_)
    return [x, y * math.cos(EPS) - z * math.sin(EPS), y * math.sin(EPS) + z * math.cos(EPS)]


d = JD_TIG - 2451545.0
moon_lon = (218.316 + 13.176396 * d) % 360
moon_lat = 5.13 * math.sin(math.radians((93.272 + 13.229350 * d) % 360))
MOON = [c * 3.9e8 for c in ecliptic(moon_lon, moon_lat)]
MOON_V = [c * 1022.0 * 0.01 for c in ecliptic(moon_lon + 90)]
# Retrograde to the moon's orbit; at TIG the CSM is over the far side
# (away from the earth), moving so the burn sends it back toward the earth.
N_HAT = unit(cross(MOON_V, MOON))
P_HAT = unit(lin((1, unit(MOON)), (-dot(unit(MOON), N_HAT), N_HAT)))
Q_HAT = cross(N_HAT, P_HAT)


def state(get):
    """The true state (circular, moon-centered) at GET get."""
    u = N_ORB * (get - TIG_GET)
    r = lin((RO * math.cos(u), P_HAT), (RO * math.sin(u), Q_HAT))
    v = lin((-VO * math.sin(u), P_HAT), (VO * math.cos(u), Q_HAT))
    return r, v


R_LOAD, V_LOAD = state(LOAD_GET)
R_TIG, V_TIG = state(TIG_GET)
LV_Y = [-c for c in unit(cross(R_TIG, V_TIG))]
LV_Z = [-c for c in unit(R_TIG)]
LV_X = cross(LV_Y, LV_Z)
DELV = [DV * c for c in LV_X]

Vcs = [c * 0.01 for c in V_LOAD]
vec('RRECTCSM', R_LOAD, 27); vec('VRECTCSM', Vcs, 5)
put('TETCSM', dp(T_LOAD / 2**28))
put('DELTACSM', [0] * 6); put('NUVCSM', [0] * 6)
vec('RCVCSM', R_LOAD, 27); vec('VCVCSM', Vcs, 5)
put('TCCSM', [0, 0]); put('XKEPCSM', [0, 0])
flag(123)               # CMOONFLG: moon sphere
# The landing site vector (B-27 m, moon-fixed): only its length is used
# here, as the radius P30's N42 and V82's N44 take altitudes from.
vec('RLS', [R_MOON, 0, 0], 27)

SM_X = unit(DELV)
SM_Z = unit(lin((1, LV_Z), (-dot(LV_Z, SM_X), SM_X)))
SM_Y = cross(SM_Z, SM_X)
put('REFSMMAT', sum((dp(c / 2) for row in (SM_X, SM_Y, SM_Z) for c in row), []))
flag(47)                # REFSMFLG
put('CDUX', [IGN_CDU[0]]); put('CDUY', [IGN_CDU[1]]); put('CDUZ', [IGN_CDU[2]])
put('CDUXD', [IGN_CDU[0], 0, IGN_CDU[1], 0, IGN_CDU[2], 0])

# TVC DAP: N46 R1 CSM alone (A digit 1), quads; N47 mass (B+16 kg); EMDOT
# (kg/cs B+3); no trim offsets.
put('DAPDATR1', [0o11102]); put('DAPDATR2', [0o01111])
put('CSMMASS', [int(round(CSM_KG / 2**16 * 2**14))])
put('LEMMASS', [0])
put('EMDOT', [int(round(SPS_THRUST / SPS_VEX * 0.01 / 8 * 2**14))])
# Trim (the gimbal angles that put the thrust through the centre of mass)
# and the TVC DAP's gains, Apollo 11's CMC pad load (NASSP's Comanche055
# pad load worksheet, github.com/orbiternassp/padloads): PACTOFF -1.541
# and YACTOFF +1.321 degrees (85.41 arcseconds a bit); EKPRIME (steering)
# CSM 0.1, CSM/LM 0.15; EKTLX/I (filter) CSM 8.4, CSM/LM 0.6265, CSM/LM low
# bandwidth 0.05786; EREPFRAC (alignment correction) CSM 0.25, LM 0.0375.
# Without them VARK and KPRIMEDT are 0: the DAP neither steers nor damps.
put('PACTOFF', [0o77676]); put('YACTOFF', [0o70])
put('EKPRIME', [0o123, 0o175]); put('EKTLX/I', [0o37071, 0o2244, 0o156])
put('EREPFRAC', [0o1000, 0o232])
put('WEIGHT/G', [int(round(CSM_KG / 2**16 * 2**14))])
# SPS pads as cm_p40.py.
put('EK1VAL', dp(SPS_THRUST / 100 / 2**23)); put('EK2VAL', dp(10000 / 100 / 2**23))
put('EK3VAL', [int(round(SPS_THRUST / 1e4 / 2**9 * 2**14))])
put('FANG', [int(round(SPS_THRUST / 1e4 / 2**7 * 2**14))])
put('DVTHRESH', [int(round(0.01 / 2**-2 * 2**14))])
put('ETDECAY', [25])

tephem = TEPHEM_CS + int(round(LOAD_GET * 100)) - T_LOAD
put('TEPHEM', tp(tephem))
put('TIMEMO', tp(tephem + T_LOAD))
dl = JD_LIFTOFF + LOAD_GET / 86400 - 2451545.0
g = math.radians((357.529 + 0.98560028 * dl) % 360)
sun_lon = (280.459 + 0.98564736 * dl + 1.915 * math.sin(g) + 0.020 * math.sin(2 * g)) % 360
sun_au = 1.00014 - 0.01671 * math.cos(g) - 0.00014 * math.cos(2 * g)
SUN = [c * sun_au * 1.495978707e11 for c in ecliptic(sun_lon)]
vec('RESO', SUN, 38)
vec('VESO', [c * 0.25 for c in ecliptic(sun_lon + 90)], 0)
put('OMEGAES', dp(1 / (365.2422 * 86400 * 100) * 2**26))
for k in range(10):
    put('VECOEM', [0] * 6, off=6 * k)
put('VECOEM', sum((dp(c / 2**31) for c in MOON), []), off=54)
put('VECOEM', sum((dp(c / 2**5) for c in MOON_V), []), off=48)

# The burn, integrated as FLIGHT_SPS gives it (thrust inertially fixed
# along the ignition direction is close enough for the summary).
r, v, m, t = list(R_TIG), list(V_TIG), CSM_KG, 0.0
while t < BURN_S:
    dt = min(0.01, BURN_S - t)
    rr = norm(r)
    a = SPS_THRUST / m
    v = [v[i] + (a * SM_X[i] - MUM * r[i] / rr**3) * dt for i in range(3)]
    r = [r[i] + v[i] * dt for i in range(3)]
    m -= SPS_THRUST / SPS_VEX * dt
    t += dt
vinf2 = dot(v, v) - 2 * MUM / norm(r)
print('orbit %.1f km circular, %.0f m/s; DV %.1f m/s (%.1f ft/s), burn about %.0f s' % (
    ALT / 1e3, VO, DV, DV_FPS, BURN_S))
print('cutoff: %.0f m/s, v_inf %.0f m/s' % (norm(v), math.sqrt(max(vinf2, 0))))

out_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/cm-tei.load'
with open(out_path, 'w') as f:
    f.write('# CM P30/P40 (TEI) scenario load at 20 s (tools/flight_scenarios/cm_tei.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
print('words', len(out))
