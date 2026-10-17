#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM P23 scenario loads (docs/FLIGHT-STORE.md, "Flight scenarios"):
cislunar midcourse navigation in translunar coast, at about 24:30 GET,
one star / earth near-horizon measurement.

The values are synthetic but self-consistent:

- the CSM on a translunar ellipse from TLI in the launch plane (two-body,
  earth only), at the point it reaches at LOAD_GET; the CMC's state vector
  is that state moved by STATE_ERROR (what the measurement corrects);
- the launch REFSMMAT (as cm_p52.py), the platform on it;
- the CSM held at an attitude that puts the sextant's shaft axis (the
  landmark line of sight) on the earth's near horizon toward the star,
  rolled to keep the middle gimbal small;
- TEPHEM, the sun and moon pads (as cm_p52.py) at the load;
- the P23 pads: WMIDPOS and WMIDVEL (the W matrix on its first pass) and
  HORIZALT (the horizon's altitude above the Fischer ellipsoid).

Outputs, all octal "ADDR VALUE" loads for FLIGHT_LOAD:

- cm-p23.load: the state at the load (20 s after a cold start);
- cm-p23-cal.load: the optics for R57's calibration mark (the star line
  of sight on the landmark line, trunnion TRUNBIAS_DEG);
- cm-p23-mark.load: the optics at the P23 mark: the star on the horizon
  as the true trajectory sees it at MARK_CS, aberration included.

  cm_p23.py [DIR]   (default tools/flight_scenarios)
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


MU = 3.986032e14        # m^3/s^2 (Comanche's earth MU)
A_F, B_F = 6378166.0, 6356784.0         # Fischer ellipsoid (GETERAD)
C = 299792458.0
T_LOAD = 2000           # cs after the cold start
LOAD_GET = 24 * 3600 + 30 * 60          # s: 24:30:00 GET at the load
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
PAD_LAT, PAD_LON, AZ = 28.608, -80.604, 72.0
R_ORBIT = 6378137.0 + 100.0 * 1852
INSERT_GET, INSERT_RANGE = 709.0, 21.0
TLI_GET, TLI_V = 9920.0, 10900.0        # s (02:45:20 GET, TLI cutoff), m/s at perigee
STATE_ERROR = (60000.0, -40000.0, 25000.0)    # m: the CMC's position minus the true one
HORIZALT = 28000.0      # m above the Fischer ellipsoid
WMIDPOS, WMIDVEL = 0o1000, 0o2000       # W matrix diagonal pads (single words)
TRUNBIAS_DEG = 0.011    # the calibration mark's trunnion (R57 stores it)
SHAFT_AXIS_DEG = 32.523
TRUNNION_OFFSET = 19.775
CAL_CS, MARK_CS = 11000, 17000          # cs: the calibration mark and the P23 mark
STAR = None             # picked below (octal star code)


def star(code):
    rows = []
    for line in open(ROOT / 'build/fixed-data/Comanche055/STAR_TABLES.agc'):
        m = re.match(r'\s+2DEC\s+([+-]?\.\d+) B-1\s+# STAR\s*(\d+)\s+([XYZ])', line)
        if m:
            rows.append((int(m.group(2)), m.group(3), float(m.group(1))))
    v = {(n, a): x for n, a, x in rows}
    return [v[(code, 'X')], v[(code, 'Y')], v[(code, 'Z')]]


# The launch REFSMMAT and the parking orbit (as cm_p52.py).
gmst = (280.46061837 + 360.98564736629 * (JD_LIFTOFF - 2451545.0)) % 360
th = math.radians(gmst + PAD_LON)
la = math.radians(PAD_LAT)
up = [math.cos(la) * math.cos(th), math.cos(la) * math.sin(th), math.sin(la)]
east = [-math.sin(th), math.cos(th), 0.0]
north = cross(up, east)
az = math.radians(AZ)
down_range = [math.cos(az) * north[i] + math.sin(az) * east[i] for i in range(3)]
REFSMMAT = [down_range, cross([-c for c in up], down_range), [-c for c in up]]
SM = REFSMMAT
h = unit(cross(up, down_range))
n_rate = math.sqrt(MU / R_ORBIT**3)
u0 = math.radians(INSERT_RANGE) - n_rate * INSERT_GET
along = cross(h, up)

# TLI: perigee of the translunar ellipse where the parking orbit is at
# TLI_GET, speed TLI_V there.
u_tli = u0 + n_rate * TLI_GET
P_HAT = [math.cos(u_tli) * up[i] + math.sin(u_tli) * along[i] for i in range(3)]
Q_HAT = cross(h, P_HAT)
SMA = 1 / (2 / R_ORBIT - TLI_V**2 / MU)
ECC = 1 - R_ORBIT / SMA
N_ELL = math.sqrt(MU / SMA**3)


def state(get):
    """The true CSM state (two-body) at GET get."""
    M = N_ELL * (get - TLI_GET)
    E = M
    for _ in range(50):
        E = E - (E - ECC * math.sin(E) - M) / (1 - ECC * math.cos(E))
    x = SMA * (math.cos(E) - ECC)
    y = SMA * math.sqrt(1 - ECC**2) * math.sin(E)
    edot = N_ELL / (1 - ECC * math.cos(E))
    vx = -SMA * math.sin(E) * edot
    vy = SMA * math.sqrt(1 - ECC**2) * math.cos(E) * edot
    return lin((x, P_HAT), (y, Q_HAT)), lin((vx, P_HAT), (vy, Q_HAT))


def get_at(cs):
    return LOAD_GET + (cs - T_LOAD) / 100


def horizon(r, s):
    """The near horizon toward star s seen from r: the line of sight that
    grazes the sphere of the Fischer radius at the tangent point's
    latitude plus HORIZALT, in the plane of the earth's center and the
    star (HORIZ)."""
    d = [-c for c in r]
    rn = norm(r)
    rh = A_F + HORIZALT
    for _ in range(5):
        a = math.asin(rh / rn)
        ud = unit(d)
        perp = unit(lin((1, s), (-dot(s, ud), ud)))
        los = lin((math.cos(a), ud), (math.sin(a), perp))
        tp_ = lin((1, r), (rn * math.cos(a), los))
        lat = math.asin(tp_[2] / norm(tp_))
        rf = B_F / math.sqrt(1 - (1 - (B_F / A_F)**2) * math.cos(lat)**2)
        rh = rf + HORIZALT
    return unit(los)


def apparent(u, v):
    return unit([u[i] + v[i] / C for i in range(3)])


R_LOAD, V_LOAD = state(LOAD_GET)
# Pick a star 20 to 40 degrees from the earth's center, as clear of the
# sun as the table allows.
SUN_DIR = None
R_mark, V_mark = state(get_at(MARK_CS))
cands = []
for code in range(1, 0o46):
    s = unit(star(code))
    sep = math.degrees(math.acos(dot(s, unit([-c for c in R_mark]))))
    if 20 < sep < 40:
        cands.append((abs(sep - 30), code, sep))
cands.sort()
STAR = cands[0][1]
S_REF = unit(star(STAR))

# The CMC's state: the true one moved by STATE_ERROR.
R_CMC = lin((1, R_LOAD), (1, STATE_ERROR))
Vcs = [c * 0.01 for c in V_LOAD]
vec('RRECTCSM', R_CMC, 29); vec('VRECTCSM', Vcs, 7)
put('TETCSM', dp(T_LOAD / 2**28))
put('DELTACSM', [0] * 6); put('NUVCSM', [0] * 6)
vec('RCVCSM', R_CMC, 29); vec('VCVCSM', Vcs, 7)
put('TCCSM', [0, 0]); put('XKEPCSM', [0, 0])
flag(123, False)        # CMOONFLG: earth sphere
put('REFSMMAT', sum((dp(c / 2) for row in REFSMMAT for c in row), []))
flag(47)                # REFSMFLG
put('WMIDPOS', [WMIDPOS]); put('WMIDVEL', [WMIDVEL])
put('HORIZALT', dp(HORIZALT / 2**29))
flag(54, False)         # ORBWFLAG: W matrix invalid, P23 initializes it

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
SUN = [c * sun_au * 1.495978707e11 for c in ecliptic(sun_lon)]
vec('RESO', SUN, 38)
vec('VESO', [c * 0.25 for c in ecliptic(sun_lon + 90)], 0)
put('OMEGAES', dp(1 / (365.2422 * 86400 * 100) * 2**26))
moon_lon = (218.316 + 13.176396 * d) % 360
moon_lat = 5.13 * math.sin(math.radians((93.272 + 13.229350 * d) % 360))
MOON = [c * 3.9e8 for c in ecliptic(moon_lon, moon_lat)]
MOON_V = [c * 1022.0 * 0.01 for c in ecliptic(moon_lon + 90)]
for k in range(10):
    put('VECOEM', [0] * 6, off=6 * k)
put('VECOEM', sum((dp(c / 2**31) for c in MOON), []), off=54)
put('VECOEM', sum((dp(c / 2**5) for c in MOON_V), []), off=48)

# Attitude: the shaft axis on the horizon at the mark, rolled to keep the
# middle gimbal small.
sa = math.radians(SHAFT_AXIS_DEG)
NB2NB1 = [[math.cos(sa), 0, -math.sin(sa)], [0, 1, 0], [math.sin(sa), 0, math.cos(sa)]]
HOR = apparent(horizon(R_mark, S_REF), V_mark)


def attitude(roll):
    a = unit(cross(HOR, [0, 0, 1]))
    b = cross(HOR, a)
    rr = math.radians(roll)
    p = [math.cos(rr) * a[i] + math.sin(rr) * b[i] for i in range(3)]
    q = cross(p, HOR)
    return mxm(tr(NB2NB1), [q, p, HOR])


def gimbals(nb_ref, sm_ref):
    m = mxm(nb_ref, tr(sm_ref))
    mg = math.degrees(math.asin(max(-1, min(1, m[0][1]))))
    ig = math.degrees(math.atan2(-m[0][2], m[0][0]))
    og = math.degrees(math.atan2(-m[2][1], m[1][1]))
    chk = mxm(mxm(rot(0, og), rot(2, mg)), rot(1, ig))
    assert max(abs(chk[i][j] - m[i][j]) for i in range(3) for j in range(3)) < 1e-9
    return og, ig, mg


def sxt(nb_ref, s_ref):
    o = mxv(NB2NB1, mxv(nb_ref, s_ref))
    return math.degrees(math.atan2(o[1], o[0])) % 360, math.degrees(math.acos(max(-1, min(1, o[2]))))


best = min(range(0, 360, 5), key=lambda r: abs(gimbals(attitude(r), SM)[2]))
NB = attitude(best)
og, ig, mg = gimbals(NB, SM)
put('CDUX', [cdu(og)]); put('CDUY', [cdu(ig)]); put('CDUZ', [cdu(mg)])
sh, tn = sxt(NB, apparent(S_REF, V_mark))
assert 5 < tn < 50, tn


def trunnion_cdu(deg):
    return cdu((deg - TRUNNION_OFFSET) * 4)


rn = norm(R_mark)
print('TLI ellipse: a %.0f km, e %.4f; at the load %.0f km from the earth, %.0f m/s' % (
    SMA / 1e3, ECC, norm(R_LOAD) / 1e3, norm(V_LOAD)))
print('star %02o: %.2f deg from the earth center, earth radius %.2f deg' % (
    STAR, math.degrees(math.acos(dot(S_REF, unit([-c for c in R_mark])))), math.degrees(math.asin(A_F / rn))))
print('roll %d: gimbals outer %.3f inner %.3f middle %.3f deg' % (best, og, ig, mg))
print('mark at %d cs: shaft %.3f, star-horizon angle %.4f deg (+ bias %.3f)' % (MARK_CS, sh, tn, TRUNBIAS_DEG))

d_out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios'
with open(d_out / 'cm-p23.load', 'w') as f:
    f.write('# CM P23 scenario load at 20 s (tools/flight_scenarios/cm_p23.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
with open(d_out / 'cm-p23-cal.load', 'w') as f:
    f.write('# CM P23: the optics at R57\'s calibration mark (tools/flight_scenarios/cm_p23.py)\n')
    f.write('%04o %05o\n' % (addr('CDUS'), cdu(sh)))
    f.write('%04o %05o\n' % (addr('CDUT'), trunnion_cdu(TRUNBIAS_DEG)))
with open(d_out / 'cm-p23-mark.load', 'w') as f:
    f.write('# CM P23: the optics on star %02o and the horizon (tools/flight_scenarios/cm_p23.py)\n' % STAR)
    f.write('%04o %05o\n' % (addr('CDUS'), cdu(sh)))
    f.write('%04o %05o\n' % (addr('CDUT'), trunnion_cdu(tn + TRUNBIAS_DEG)))
print('words', len(out))
