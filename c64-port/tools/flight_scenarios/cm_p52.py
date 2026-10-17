#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM P52 scenario loads (docs/FLIGHT-STORE.md, "Flight scenarios"): Apollo
11's first IMU realignment in earth parking orbit, at about 00:44 GET, on
stars 30 (Menkent) and 37 (Nunki), the pair the crew used.

The values are synthetic but self-consistent:

- the launch REFSMMAT (Z down and X along the 72 degree launch azimuth at
  the pad at liftoff, 1969-07-16 13:32:00 UTC) and a platform that has
  drifted from it by DRIFT (the gyro drift since liftoff, what P52 torques
  out);
- the CSM in a circular 100 nmi parking orbit in the launch plane, at the
  point it reaches at LOAD_GET;
- the CSM held still at an attitude that puts the sextant's shaft axis
  between the two stars (each star about 30 degrees out in trunnion);
- TEPHEM for the launch time, and the sun and moon (pad loaded: RESO,
  VESO, OMEGAES and the constant and linear VECOEM terms) where they were.

Outputs, all octal "ADDR VALUE" loads for FLIGHT_LOAD:

- cm-p52.load: the state at the load (20 s after a cold start);
- cm-p52-star1.load, cm-p52-star2.load: the optics CDUs (CDUS, CDUT) on
  each star as the true (drifted) platform sees it, aberration included:
  where the crew's manual optics put the star on the reticle before MARK.

  cm_p52.py [DIR]   (default tools/flight_scenarios)
"""
import math, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Comanche055/assembly.lst'
SYMS = {}
for line in open(LST, errors='replace'):
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+(\S+)', line):
        SYMS[m.group(2)] = m.group(3)
# Counters (not in the listing's symbol column).
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


def tp(n):
    """A non-negative integer as a triple precision word triple."""
    assert 0 <= n < 2**42
    return [n >> 28, (n >> 14) & 0o37777, n & 0o37777]


def cdu(deg):
    """An angle as a CDU counter (two's complement, 2^15 per turn)."""
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
    """Frame rotation (passive) about axis 0, 1 or 2."""
    c, s = math.cos(math.radians(deg)), math.sin(math.radians(deg))
    i, j = [(1, 2), (2, 0), (0, 1)][axis]
    m = [[1.0 if r == k else 0.0 for k in range(3)] for r in range(3)]
    m[i][i] = m[j][j] = c
    m[i][j], m[j][i] = s, -s
    return m


out = {}


def put(name, words, off=0, into=None):
    d = out if into is None else into
    for i, w in enumerate(words):
        d[addr(name, off) + i] = w


def vec(name, v, scale, into=None):
    put(name, sum((dp(c / 2**scale) for c in v), []), into=into)


def flag(n, on=True):
    a = addr('FLAGWRD0') + n // 15
    bit = 1 << (14 - n % 15)
    v = out.get(a, 0o200 if a == addr('FLAGWRD5') else 0)
    out[a] = (v | bit) if on else (v & ~bit)


MU = 3.986032e14        # m^3/s^2 (Comanche's earth MU)
RE = 6378166.0          # m (RSUBE, the occultation radius)
C = 299792458.0         # m/s
T_LOAD = 2000           # cs after the cold start
LOAD_GET = 2640.0       # s: 00:44:00 GET at the load
# Liftoff: 1969-07-16 13:32:00 UTC (JD 2440419.0639); TEPHEM counts from
# midnight before July 1 (UTC) to liftoff.
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
PAD_LAT, PAD_LON, AZ = 28.608, -80.604, 72.0
R_ORBIT = 6378137.0 + 100.0 * 1852       # 100 nmi circular
INSERT_GET, INSERT_RANGE = 709.0, 21.0   # s, degrees of orbit arc from the pad at insertion
DRIFT = (0.012, -0.021, 0.009)           # degrees about SM X, Y, Z since liftoff
STARS = (0o30, 0o37)                     # Menkent, Nunki (octal star codes, as the DSKY shows them)
SHAFT_AXIS_DEG = 32.523                  # the SXT shaft axis from NB Z toward NB X
TRUNNION_OFFSET = 19.775                 # SXTLOGIC: trunnion = CDUT / 4 + 19.775 degrees


def star(code):
    """The rope's star table (STAR_TABLES.agc), star 1 last: the unit
    vector of star `code` in the basic reference frame."""
    rows = []
    for line in open(ROOT / 'build/fixed-data/Comanche055/STAR_TABLES.agc'):
        m = re.match(r'\s+2DEC\s+([+-]?\.\d+) B-1\s+# STAR\s*(\d+)\s+([XYZ])', line)
        if m:
            rows.append((int(m.group(2)), m.group(3), float(m.group(1))))
    v = {(n, a): x for n, a, x in rows}
    return [v[(code, 'X')], v[(code, 'Y')], v[(code, 'Z')]]


# Earth rotation at liftoff (GMST), the pad and the launch REFSMMAT.
gmst = (280.46061837 + 360.98564736629 * (JD_LIFTOFF - 2451545.0)) % 360
th = math.radians(gmst + PAD_LON)
la = math.radians(PAD_LAT)
up = [math.cos(la) * math.cos(th), math.cos(la) * math.sin(th), math.sin(la)]
east = [-math.sin(th), math.cos(th), 0.0]
north = cross(up, east)
az = math.radians(AZ)
down_range = [math.cos(az) * north[i] + math.sin(az) * east[i] for i in range(3)]
REFSMMAT = [down_range, cross([-c for c in up], down_range), [-c for c in up]]
# Rows: SM X, Y, Z in reference axes (Y = Z x X).
assert abs(dot(REFSMMAT[1], cross(REFSMMAT[2], REFSMMAT[0])) - 1) < 1e-12
# The platform now: REFSMMAT turned by the drift (small rotations).
SM = mxm(mxm(mxm(rot(2, DRIFT[2]), rot(1, DRIFT[1])), rot(0, DRIFT[0])), REFSMMAT)

# The parking orbit in the launch plane: at GET t the CSM is u(t) along it
# from the pad's inertial position at liftoff.
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
vec('RRECTCSM', R, 29); vec('VRECTCSM', Vcs, 7)
put('TETCSM', dp(T_LOAD / 2**28))
put('DELTACSM', [0] * 6); put('NUVCSM', [0] * 6)
vec('RCVCSM', R, 29); vec('VCVCSM', Vcs, 7)
put('TCCSM', [0, 0]); put('XKEPCSM', [0, 0])
flag(123, False)        # CMOONFLG: earth sphere

put('REFSMMAT', sum((dp(c / 2) for row in REFSMMAT for c in row), []))
flag(47)                # REFSMFLG

# Ephemerides: TEPHEM so the AGC clock plus TEPHEM is the time since July
# 1 at the load; TIMEMO (the ephemeris epoch) at the load.
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
SUN_V = ecliptic(sun_lon + 90)          # direction of the sun's apparent motion
vec('RESO', SUN, 38)
vec('VESO', [c * 0.25 for c in SUN_V], 0)
put('OMEGAES', dp(1 / (365.2422 * 86400 * 100) * 2**26))
# Moon: two and a half days after new moon (14 July 03:12 UTC), about 33
# degrees east of the sun, 390,000 km. VECOEM's last vector is the
# constant term (meters B-31 before REM's VSL2), the one before it the
# linear term in centiseconds B-26 (m/cs B-5).
moon_lon = (218.316 + 13.176396 * d) % 360
moon_lat = 5.13 * math.sin(math.radians((93.272 + 13.229350 * d) % 360))
MOON = [c * 3.9e8 for c in ecliptic(moon_lon, moon_lat)]
MOON_V = [c * 1022.0 * 0.01 for c in ecliptic(moon_lon + 90)]
for k in range(10):
    put('VECOEM', [0] * 6, off=6 * k)
put('VECOEM', sum((dp(c / 2**31) for c in MOON), []), off=54)
put('VECOEM', sum((dp(c / 2**5) for c in MOON_V), []), off=48)

# Attitude: the shaft axis (NB) between the two stars, rolled about it to
# keep the middle gimbal small.
# The table counts stars in decimal; the DSKY codes are octal (30 = 24th).
S_REF = [unit(star(c)) for c in STARS]
mid = unit([S_REF[0][i] + S_REF[1][i] for i in range(3)])
sa = math.radians(SHAFT_AXIS_DEG)
NB2NB1 = [[math.cos(sa), 0, -math.sin(sa)], [0, 1, 0], [math.sin(sa), 0, math.cos(sa)]]
shaft_nb = [math.sin(sa), 0, math.cos(sa)]


def attitude(roll):
    """Body (NB) axes in reference axes: NB's shaft axis on `mid`, rolled
    `roll` degrees about it."""
    a = unit(cross(mid, [0, 0, 1]))
    b = cross(mid, a)
    rr = math.radians(roll)
    p = [math.cos(rr) * a[i] + math.sin(rr) * b[i] for i in range(3)]   # NB Y
    q = cross(p, mid)
    # Optics frame (NB1) axes: Z = mid, Y = p, X = q; NB = NB1NB2 * NB1.
    nb1 = [q, p, mid]
    return mxm(tr(NB2NB1), nb1)


def gimbals(nb_ref, sm_ref):
    """CDU angles (outer X, inner Y, middle Z, degrees) with NB from SM =
    Rx(o) Rz(m) Ry(i) (*SMNB*)."""
    m = mxm(nb_ref, tr(sm_ref))
    mg = math.degrees(math.asin(max(-1, min(1, m[0][1]))))
    ig = math.degrees(math.atan2(-m[0][2], m[0][0]))
    og = math.degrees(math.atan2(-m[2][1], m[1][1]))
    chk = mxm(mxm(rot(0, og), rot(2, mg)), rot(1, ig))
    assert max(abs(chk[i][j] - m[i][j]) for i in range(3) for j in range(3)) < 1e-9
    return og, ig, mg


def sxt(nb_ref, s_ref):
    """Shaft and trunnion (degrees) on a star (CALCSXA, SXTANG1)."""
    o = mxv(NB2NB1, mxv(nb_ref, s_ref))
    return math.degrees(math.atan2(o[1], o[0])) % 360, math.degrees(math.acos(o[2]))


best = min(range(0, 360, 5), key=lambda r: abs(gimbals(attitude(r), SM)[2]))
NB = attitude(best)
og, ig, mg = gimbals(NB, SM)
put('CDUX', [cdu(og)]); put('CDUY', [cdu(ig)]); put('CDUZ', [cdu(mg)])

# Occultation check (the AGC repeats it in PICAPAR): stars clear of the
# earth's disc by 5 degrees.
earth = math.degrees(math.asin(RE / R_ORBIT)) + 5
for c, s in zip(STARS, S_REF):
    off = math.degrees(math.acos(-dot(unit(R), s)))
    assert off > earth, (c, off, earth)

# The stars as the true platform sees them: apparent (aberration) and
# into the optics.
marks = []
for c, s in zip(STARS, S_REF):
    app = unit([s[i] + V[i] / C for i in range(3)])
    sh, tn = sxt(NB, app)
    assert tn < 50, (c, tn)
    marks.append((c, sh, tn))

print('GMST %.3f deg; CSM altitude %.1f km, %.1f m/s' % (gmst, (R_ORBIT - 6378137) / 1e3, math.sqrt(dot(V, V))))
print('roll %d: gimbals outer %.3f inner %.3f middle %.3f deg' % (best, og, ig, mg))
for c, sh, tn in marks:
    print('star %02o: shaft %.3f, trunnion %.3f deg; nadir %.1f deg' % (
        c, sh, tn, math.degrees(math.acos(-dot(unit(R), S_REF[STARS.index(c)])))))

d_out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios'
with open(d_out / 'cm-p52.load', 'w') as f:
    f.write('# CM P52 scenario load at 20 s (tools/flight_scenarios/cm_p52.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
for k, (c, sh, tn) in enumerate(marks, 1):
    with open(d_out / f'cm-p52-star{k}.load', 'w') as f:
        f.write('# CM P52: the optics on star %02o (tools/flight_scenarios/cm_p52.py)\n' % c)
        f.write('%04o %05o\n' % (addr('CDUS'), cdu(sh)))
        f.write('%04o %05o\n' % (addr('CDUT'), cdu((tn - TRUNNION_OFFSET) * 4)))
print('words', len(out))
