#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM P22 scenario loads (docs/FLIGHT-STORE.md, "Flight scenarios"): lunar
orbit navigation on a known landmark, as Collins tracked landmark 130 near
the landing site on Apollo 11 (synthetic values, about 98 h GET).

The values are synthetic but self-consistent:

- the moon's orientation is the CMC's own (PLANETARY_INERTIAL_ORIENTATION:
  MOONMX with the rope's constants, TIMSUBO the same epoch as TEPHEM, no
  libration), so the landmark the crew loads on N89 is where this script
  puts it;
- the landmark (LANDMARK_LAT, LANDMARK_LON, LANDMARK_ALT above the CMC's
  lunar radius 504RM) near the landing site;
- the CSM in a circular 60 nmi orbit, retrograde in the moon's equator
  plane, over the landmark's longitude at MARK_MID_CS; the CMC's state
  vector is that state moved by STATE_ERROR (what the marks correct);
- the platform in local vertical axes at MARK_MID_CS (X along the
  velocity, Z down), and the CSM held at an attitude that puts the
  sextant's shaft axis on the landmark then, rolled to keep the middle
  gimbal small;
- TEPHEM, the sun and moon pads, and the orbital navigation W matrix pads
  (WORBPOS, WORBVEL, S22WSUBL).

Outputs, all octal "ADDR VALUE" loads for FLIGHT_LOAD:

- cm-p22.load: the state at the load (20 s after a cold start);
- cm-p22-mark1.load, cm-p22-mark2.load: the optics CDUs (CDUS, CDUT) on
  the landmark from the true orbit at each mark (MARK_CS): where the
  crew's manual optics hold the landmark on the reticle at MARK.

  cm_p22.py [DIR]   (default tools/flight_scenarios)
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
RM = 1738090.0          # 504RM
T_LOAD = 2000           # cs after the cold start
LOAD_GET = 98 * 3600.0  # s at the load (synthetic)
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
ALT_ORBIT = 60.0 * 1852
LANDMARK_LAT, LANDMARK_LON, LANDMARK_ALT = 1.20, 23.70, -1500.0    # degrees, degrees, m
STATE_ERROR = (1200.0, -600.0, 400.0)   # m: the CMC's position minus the true one
WORBPOS, WORBVEL, S22WSUBL = 2000, 328, 1000    # m B-14, m/cs B0 (2 m/s), m B-14
MARK_CS = (13200, 15200)                # cs: the two marks
MARK_MID_CS = sum(MARK_CS) // 2
SHAFT_AXIS_DEG = 32.523
TRUNNION_OFFSET = 19.775

tephem = TEPHEM_CS + int(round(LOAD_GET * 100)) - T_LOAD


def moon_m(cs):
    """MOONMX: the moon-fixed axes (rows) in reference axes at AGC time
    cs, with TIMSUBO = TEPHEM (the rope's constants, radians and rad/s)."""
    t = (cs + tephem) / 100.0
    n = 6.19653663041 - 1.07047011e-8 * t
    f = 5.20932947829 + 2.67240410e-6 * t
    b = 0.40916190299 - 7.19757301e-14 * t
    cosi, sini = .99964173, .02676579
    cob, sob = math.cos(b), math.sin(b)
    a_v = [math.cos(n), cob * math.sin(n), sob * math.sin(n)]
    b_v = [-math.sin(n), cob * math.cos(n), sob * math.cos(n)]
    c_v = [0.0, -sob, cob]
    m2 = lin((sini, b_v), (cosi, c_v))
    d_v = lin((cosi, b_v), (-sini, c_v))
    m1 = lin((math.sin(f), a_v), (-math.cos(f), d_v))
    m0 = [-c for c in lin((math.cos(f), a_v), (math.sin(f), d_v))]
    return [m0, m1, m2]


def landmark(cs):
    la, lo = math.radians(LANDMARK_LAT), math.radians(LANDMARK_LON)
    rp = [math.cos(la) * math.cos(lo), math.cos(la) * math.sin(lo), math.sin(la)]
    return [c * (RM + LANDMARK_ALT) for c in mxv(tr(moon_m(cs)), rp)]


M_MID = moon_m(MARK_MID_CS)
for i in range(3):
    for j in range(3):
        assert abs(dot(M_MID[i], M_MID[j]) - (i == j)) < 1e-6
# The orbit: retrograde about the moon's pole, over the landmark's
# longitude at MARK_MID_CS.
H_HAT = [-c for c in M_MID[2]]
lo = math.radians(LANDMARK_LON)
P_HAT = mxv(tr(M_MID), [math.cos(lo), math.sin(lo), 0.0])
Q_HAT = cross(H_HAT, P_HAT)
R_ORB = RM + ALT_ORBIT
N_RATE = math.sqrt(MUM / R_ORB**3)


def state(cs):
    u = N_RATE * (cs - MARK_MID_CS) / 100
    r = lin((R_ORB * math.cos(u), P_HAT), (R_ORB * math.sin(u), Q_HAT))
    v = lin((-R_ORB * N_RATE * math.sin(u), P_HAT), (R_ORB * N_RATE * math.cos(u), Q_HAT))
    return r, v


R_LOAD, V_LOAD = state(T_LOAD)
R_CMC = lin((1, R_LOAD), (1, STATE_ERROR))
Vcs = [c * 0.01 for c in V_LOAD]
vec('RRECTCSM', R_CMC, 27)
vec('VRECTCSM', Vcs, 5)
put('TETCSM', dp(T_LOAD / 2**28))
put('DELTACSM', [0] * 6)
put('NUVCSM', [0] * 6)
vec('RCVCSM', R_CMC, 27)
vec('VCVCSM', Vcs, 5)
put('TCCSM', [0, 0])
put('XKEPCSM', [0, 0])
flag(123)               # CMOONFLG: moon sphere

# The platform: local vertical at the middle of the marks.
R_MID, V_MID = state(MARK_MID_CS)
SM_Z = [-c for c in unit(R_MID)]
SM_Y = unit(cross(SM_Z, V_MID))
SM_X = cross(SM_Y, SM_Z)
SM = [SM_X, SM_Y, SM_Z]
put('REFSMMAT', sum((dp(c / 2) for row in SM for c in row), []))
flag(47)                # REFSMFLG

put('LANDLAT', dp(LANDMARK_LAT / 360))
put('LANDLONG', dp(LANDMARK_LON / 720))   # half revs: LLASRDA doubles it into LONG
put('LANDALT', dp(LANDMARK_ALT / 2**29))
put('WORBPOS', [WORBPOS])
put('WORBVEL', [WORBVEL])
put('S22WSUBL', [S22WSUBL])
flag(54, False)         # ORBWFLAG: W matrix invalid, P22 initializes it

put('TEPHEM', tp(tephem))
put('TIMEMO', tp(tephem + T_LOAD))
put('TIMSUBO', tp(tephem))
put('504LM', [0] * 6)
EPS = math.radians(23.443)


def ecliptic(lon_, lat=0.0):
    lo_, la_ = math.radians(lon_), math.radians(lat)
    x, y, z = math.cos(la_) * math.cos(lo_), math.cos(la_) * math.sin(lo_), math.sin(la_)
    return [x, y * math.cos(EPS) - z * math.sin(EPS), y * math.sin(EPS) + z * math.cos(EPS)]


dl = JD_LIFTOFF + LOAD_GET / 86400 - 2451545.0
g = math.radians((357.529 + 0.98560028 * dl) % 360)
sun_lon = (280.459 + 0.98564736 * dl + 1.915 * math.sin(g) + 0.020 * math.sin(2 * g)) % 360
sun_au = 1.00014 - 0.01671 * math.cos(g) - 0.00014 * math.cos(2 * g)
vec('RESO', [c * sun_au * 1.495978707e11 for c in ecliptic(sun_lon)], 38)
vec('VESO', [c * 0.25 for c in ecliptic(sun_lon + 90)], 0)
put('OMEGAES', dp(1 / (365.2422 * 86400 * 100) * 2**26))
moon_lon = (218.316 + 13.176396 * dl) % 360
moon_lat = 5.13 * math.sin(math.radians((93.272 + 13.229350 * dl) % 360))
for k in range(10):
    put('VECOEM', [0] * 6, off=6 * k)
put('VECOEM', sum((dp(c * 3.9e8 / 2**31) for c in ecliptic(moon_lon, moon_lat)), []), off=54)
put('VECOEM', sum((dp(c * 1022.0 * 0.01 / 2**5) for c in ecliptic(moon_lon + 90)), []), off=48)

# Attitude: the shaft axis on the landmark at the middle of the marks.
sa = math.radians(SHAFT_AXIS_DEG)
NB2NB1 = [[math.cos(sa), 0, -math.sin(sa)], [0, 1, 0], [math.sin(sa), 0, math.cos(sa)]]
LOS_MID = unit(lin((1, landmark(MARK_MID_CS)), (-1, R_MID)))


def attitude(roll):
    a = unit(cross(LOS_MID, H_HAT))
    b = cross(LOS_MID, a)
    rr = math.radians(roll)
    p = [math.cos(rr) * a[i] + math.sin(rr) * b[i] for i in range(3)]
    q = cross(p, LOS_MID)
    return mxm(tr(NB2NB1), [q, p, LOS_MID])


def gimbals(nb_ref, sm_ref):
    m = mxm(nb_ref, tr(sm_ref))
    mg = math.degrees(math.asin(max(-1, min(1, m[0][1]))))
    ig = math.degrees(math.atan2(-m[0][2], m[0][0]))
    og = math.degrees(math.atan2(-m[2][1], m[1][1]))
    chk = mxm(mxm(rot(0, og), rot(2, mg)), rot(1, ig))
    assert max(abs(chk[i][j] - m[i][j]) for i in range(3) for j in range(3)) < 1e-9
    return og, ig, mg


def sxt(nb_ref, u):
    o = mxv(NB2NB1, mxv(nb_ref, u))
    return math.degrees(math.atan2(o[1], o[0])) % 360, math.degrees(math.acos(max(-1, min(1, o[2]))))


best = min(range(0, 360, 5), key=lambda r: abs(gimbals(attitude(r), SM)[2]))
NB = attitude(best)
og, ig, mg = gimbals(NB, SM)
put('CDUX', [cdu(og)])
put('CDUY', [cdu(ig)])
put('CDUZ', [cdu(mg)])
marks = []
for cs in MARK_CS:
    r, _ = state(cs)
    los = unit(lin((1, landmark(cs)), (-1, r)))
    sh, tn = sxt(NB, los)
    assert tn < 50, tn
    marks.append((cs, sh, tn, math.degrees(math.acos(dot(los, [-c for c in unit(r)])))))

print('orbit %.1f km, %.1f m/s; landmark %.2f N %.2f E, %.0f m' % (
    ALT_ORBIT / 1e3, R_ORB * N_RATE, LANDMARK_LAT, LANDMARK_LON, LANDMARK_ALT))
print('roll %d: gimbals outer %.3f inner %.3f middle %.3f deg' % (best, og, ig, mg))
for cs, sh, tn, nad in marks:
    print('mark at %d cs: shaft %.3f, trunnion %.3f deg; %.1f deg off nadir' % (cs, sh, tn, nad))

d_out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios'
with open(d_out / 'cm-p22.load', 'w') as f:
    f.write('# CM P22 scenario load at 20 s (tools/flight_scenarios/cm_p22.py)\n')
    for a_, w in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, w))
for k, (cs, sh, tn, _) in enumerate(marks, 1):
    with open(d_out / f'cm-p22-mark{k}.load', 'w') as f:
        f.write('# CM P22: the optics on the landmark at %d cs (tools/flight_scenarios/cm_p22.py)\n' % cs)
        f.write('%04o %05o\n' % (addr('CDUS'), cdu(sh)))
        f.write('%04o %05o\n' % (addr('CDUT'), cdu((tn - TRUNNION_OFFSET) * 4)))
print('words', len(out))
