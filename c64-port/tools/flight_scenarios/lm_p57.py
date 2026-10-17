#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""LM P57 scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"):
Apollo 11's two-star lunar surface alignment at about 103:45 GET, on stars
12 (Rigel, AOT detent 6, left rear) and 03 (Navi, detent 4, right rear),
the stars and detents Aldrin used.

The values are synthetic but self-consistent:

- the LM at Tranquility Base (0.674 N, 23.473 E) on 1969-07-20, X up,
  facing west 13 degrees left (south) of the approach heading, as it
  landed;
- the landing REFSMMAT (X up through the site at touchdown, Z along the
  approach heading, 270 degrees) and a platform that has drifted from it
  by DRIFT since then (what P57 torques out);
- the sky: the moon's position (low-precision series), its pole along
  the ecliptic pole and its prime meridian toward the earth (libration
  left out, a few degrees);
- TEPHEM for the launch time and the AOT detents' pad words (AOTAZ,
  AOTEL: azimuths -60, 0, 60, 120, 180, -120 degrees, elevation 45).

The crew's inputs, written as a keys string (P57_KEYS below, the same as
flight_host_differential.py's): for each star the X MARK, then the cursor
and spiral angles read off the AOT reticle, loaded on the flashing
V06N79 (V21, V22), here the angles the true (drifted) platform gives.

The LM turns with the moon (0.55 degrees an hour) while the platform
holds still, so the CDUs move: lm-p57-star1.load and lm-p57-star2.load
set them as they read at each X MARK (the harness does not drive them).

  lm_p57.py [DIR]   (default tools/flight_scenarios): writes lm-p57.load,
                    lm-p57-star1.load and lm-p57-star2.load
"""
import math, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Luminary099/assembly.lst'
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
    """An angle as a CDU word (two's complement, 2^15 per turn)."""
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


def lin(*terms):
    return [sum(k * v[i] for k, v in terms) for i in range(3)]


out = {}
# FLAGWRD0-9 at 20 s after a cold start with the IMU operating.
BASE_FLAGS = [0, 0, 0o40000, 0o2000, 0, 0, 0, 0o100, 0, 0]
base = {addr('FLAGWRD0') + i: v for i, v in enumerate(BASE_FLAGS)}


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


T_LOAD = 2000                           # cs after the cold start
LOAD_GET = 103 * 3600 + 40 * 60         # s: 103:40:00 GET at the load
LAND_GET = 102 * 3600 + 45 * 60 + 40    # s: touchdown, 102:45:40
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
SITE_LAT, SITE_LON = 0.674, 23.473      # degrees (Tranquility Base)
APPROACH_AZ = 270.0                     # degrees east of north: westward
YAW_LEFT = 13.0                         # degrees, the landed yaw left of the approach
DRIFT = (0.021, -0.014, 0.017)          # degrees about SM X, Y, Z since touchdown
STARS = ((0o12, 6), (0o03, 4))          # Rigel in detent 6, Navi in detent 4 (octal codes)
AOT_AZ = (-60.0, 0.0, 60.0, 120.0, 180.0, -120.0)   # detents 1-6
AOT_EL = 45.0
EPS = math.radians(23.443)
MOON_POLE = [0.0, -math.sin(EPS), math.cos(EPS)]     # the ecliptic pole, equatorial axes


def star(code):
    """CATLOG (CONTROLLED_CONSTANTS.agc): the unit vector of star `code`
    in the basic reference frame (the table numbers stars in decimal)."""
    rows = {}
    for line in open(ROOT.parent / 'Luminary099/CONTROLLED_CONSTANTS.agc'):
        m = re.match(r'\s+2DEC\s+([+-]?\.\d+) B-1\s+# STAR\s*(\d+)\s+([XYZ])', line)
        if m:
            rows[(int(m.group(2)), m.group(3))] = float(m.group(1))
    return [rows[(code, a)] for a in 'XYZ']


def ecliptic(lon, lat=0.0):
    lo, la = math.radians(lon), math.radians(lat)
    x, y, z = math.cos(la) * math.cos(lo), math.cos(la) * math.sin(lo), math.sin(la)
    return [x, y * math.cos(EPS) - z * math.sin(EPS), y * math.sin(EPS) + z * math.cos(EPS)]


def site_frame(get):
    """Up, east and north at the site (reference axes) at `get` seconds."""
    d = JD_LIFTOFF + get / 86400 - 2451545.0
    lon = (218.316 + 13.176396 * d) % 360
    lat = 5.13 * math.sin(math.radians((93.272 + 13.229350 * d) % 360))
    to_earth = [-c for c in ecliptic(lon, lat)]
    e = unit(lin((1, to_earth), (-dot(to_earth, MOON_POLE), MOON_POLE)))   # prime meridian
    e90 = cross(MOON_POLE, e)
    la, lo = math.radians(SITE_LAT), math.radians(SITE_LON)
    up = lin((math.cos(la) * math.cos(lo), e), (math.cos(la) * math.sin(lo), e90), (math.sin(la), MOON_POLE))
    east = unit(cross(MOON_POLE, up))
    north = cross(up, east)
    return up, east, north


def heading(get, az):
    up, east, north = site_frame(get)
    a = math.radians(az)
    return up, lin((math.cos(a), north), (math.sin(a), east))


# The landing REFSMMAT: X up at touchdown, Z along the approach, Y = Z x X.
up0, fwd0 = heading(LAND_GET, APPROACH_AZ)
REFSMMAT = [up0, cross(fwd0, up0), fwd0]
assert abs(dot(REFSMMAT[1], cross(REFSMMAT[2], REFSMMAT[0])) - 1) < 1e-12
SM = mxm(mxm(mxm(rot(2, DRIFT[2]), rot(1, DRIFT[1])), rot(0, DRIFT[0])), REFSMMAT)


def body(get):
    """The LM's NB axes (rows, reference axes) at `get`: X up, Z forward
    13 degrees left of the approach heading; it turns with the moon."""
    up, fwd = heading(get, APPROACH_AZ - YAW_LEFT)
    return [up, cross(fwd, up), fwd]


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


def optic_axis(az, el):
    """OANB: the optic axis and the X and Y mark plane vectors (NB)."""
    a, e = math.radians(az), math.radians(el)
    oa = [math.sin(e), math.cos(e) * math.sin(a), math.cos(e) * math.cos(a)]
    uyp = [0.0, math.cos(a), -math.sin(a)]
    uxp = unit(cross(uyp, oa))
    return oa, uxp, uyp


def reticle(s_nb, det):
    """The cursor and spiral angles (degrees) of a star (NB unit vector)
    in detent `det`: SURFSTAR run backward. OPTAXIS turns the mark planes
    by the detent's apparent field rotation AOTAZ+1 - az."""
    az = AOT_AZ[det - 1]
    oa, uxp, uyp = optic_axis(az, AOT_EL)
    r = math.radians(AOT_AZ[1] - az)
    xp = lin((math.cos(r), uxp), (math.sin(r), uyp))
    yp = lin((math.cos(r), uyp), (-math.sin(r), uxp))
    sep = math.degrees(math.acos(max(-1, min(1, dot(s_nb, oa)))))
    upp = unit(lin((1, s_nb), (-dot(s_nb, oa), oa)))
    l = cross(oa, upp)                  # the cursor line: cos(Y) yp - sin(Y) xp
    yrot = math.degrees(math.atan2(-dot(l, xp), dot(l, yp))) % 360
    srot = (yrot + 12 * sep) % 360
    return yrot, srot, sep


# The load: state at T_LOAD.
put('REFSMMAT', sum((dp(c / 2) for row in REFSMMAT for c in row), []))
flag(47)                # REFSMFLG
flag(124)               # LMOONFLG
flag(127)               # SURFFLAG: on the lunar surface
put('AOTAZ', [cdu(a) for a in AOT_AZ])
put('AOTEL', [cdu(AOT_EL)] * 6)
put('504LM', [0] * 6)
tephem = TEPHEM_CS + int(round(LOAD_GET * 100)) - T_LOAD
put('TEPHEM', tp(tephem))
up_s = site_frame(LOAD_GET)[0]
RLS_MF = [1735470.0 * c for c in (math.cos(math.radians(SITE_LAT)) * math.cos(math.radians(SITE_LON)),
                                  math.cos(math.radians(SITE_LAT)) * math.sin(math.radians(SITE_LON)),
                                  math.sin(math.radians(SITE_LAT)))]
vec('RLS', RLS_MF, 27)
NB = body(LOAD_GET)
vec('GSAV', mxv(NB, up_s), 1)           # unit gravity (up) in NB, from an earlier gravity sighting
og, ig, mg = gimbals(NB, SM)
put('CDUX', [cdu(og)]); put('CDUY', [cdu(ig)]); put('CDUZ', [cdu(mg)])

# The keys (AGC centiseconds) and the reticle readings at each X MARK.
MARK_CS = (16000, 30000)
P57_KEYS = ('@3000V37E57E@4000V33E@5000V22E2E@6000V33E'
            '@12000V21E{0:05o}E@13000V33E@14000V33E@15000V33E@{m1}x@17000V21E+{c1}E@18000V22E+{s1}E@19000V33E'
            '@26000V21E{1:05o}E@27000V33E@28000V33E@29000V33E@{m2}x@31000V21E+{c2}E@32000V22E+{s2}E@33000V33E'
            '@40000V33E@42000V33E@48000E@50000V34E@52000V37E00E')
fields = {}
mark_cdus = []
for k, ((code, det), mark) in enumerate(zip(STARS, MARK_CS), 1):
    get = LOAD_GET + (mark - T_LOAD) / 100
    nb = body(get)
    s_nb = mxv(nb, unit(star(code)))
    y, s, sep = reticle(s_nb, det)
    mark_cdus.append(gimbals(nb, SM))   # the moon has turned the LM since the load
    print('star %02o detent %d: off axis %.2f deg; cursor %.2f spiral %.2f' % (code, det, sep, y, s))
    assert sep < 30, (code, det, sep)
    fields['m%d' % k] = mark
    fields['c%d' % k] = '%05d' % round(y * 100)
    fields['s%d' % k] = '%05d' % round(s * 100)
KEYS = P57_KEYS.format(STARS[0][0], STARS[1][0], **fields)

print('gimbals outer %.3f inner %.3f middle %.3f deg' % (og, ig, mg))
print('up at the load, reference axes:', [round(c, 4) for c in up_s])
print('KEYS', KEYS)
d_out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios'
with open(d_out / 'lm-p57.load', 'w') as f:
    f.write('# LM P57 scenario load at 20 s (tools/flight_scenarios/lm_p57.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
for k, (o_, i_, m_) in enumerate(mark_cdus, 1):
    with open(d_out / f'lm-p57-star{k}.load', 'w') as f:
        f.write('# LM P57: the CDUs at star %d\'s mark (tools/flight_scenarios/lm_p57.py)\n' % k)
        for n_, a_ in (('CDUX', o_), ('CDUY', i_), ('CDUZ', m_)):
            f.write('%04o %05o\n' % (addr(n_), cdu(a_)))
print('words', len(out))
