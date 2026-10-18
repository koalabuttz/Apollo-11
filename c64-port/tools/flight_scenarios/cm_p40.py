#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM P30/P40 scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"):
lunar orbit insertion (LOI-1), an external delta-V SPS burn.

The values are synthetic but self-consistent, with Apollo 11's LOI-1 as the
reference (TIG 75:49:50 GET, 889 m/s retrograde, about 6 minutes):

- the CSM/LM on a selenocentric hyperbola (two-body, moon only), retrograde
  to the moon's orbit, periselene PERI_ALT above the moon at TIG + about
  half the burn, the excess speed chosen so the impulsive DV leaves a
  111 x 313 km orbit; the CMC's state is the true one (CMOONFLG set);
- TIG and DELVSLV (P30's N33 and N81) loaded, DELVSLV the retrograde DV
  along the velocity at TIG in local vertical axes (as S30.1's LOMAT);
- a REFSMMAT with X along that DV and Z toward the moon, the vehicle at
  the ignition attitude S40.1 computes from it (and the RCS DAP's CDUXD-ZD
  there too), so P40's R60 maneuver to the burn attitude is nil;
- the TVC DAP pads (N46 CSM+LM, N47 masses, WEIGHT/G) and the SPS pads
  (EMDOT, EK1VAL-EK3VAL, FANG, DVTHRESH, ETDECAY);
- TEPHEM, the sun and moon pads (as cm_p23.py) at the load.

The host harness's FLIGHT_SPS model (tools/flight_host_run.c) gives the
thrust while ENGONFLG is set: SPS_THRUST along the engine bell (body X
turned by the mechanical biases), burning the FLIGHT_SPS mass (CSM_KG +
LM_KG) down at SPS_VEX.

  cm_p40.py [OUT]   (default tools/flight_scenarios/cm-p40.load)
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
TIG_GET = 75 * 3600 + 49 * 60 + 50.0
LOAD_GET = TIG_GET - (TIG_CS - T_LOAD) / 100
JD_LIFTOFF = 2440418.5 + (13 + 32 / 60) / 24
TEPHEM_CS = int(round(((16 - 1) * 86400 + 13 * 3600 + 32 * 60) * 100))
DV = 889.0              # m/s, LOI-1
PERI_ALT, APO_ALT = 111000.0, 313000.0  # m: the orbit an impulsive DV would give
SPS_THRUST, SPS_VEX = 91188.0, 3080.0   # N, m/s (FLIGHT_SPS)
CSM_KG, LM_KG = 28800.0, 15100.0
BURN_S = (CSM_KG + LM_KG) * SPS_VEX / SPS_THRUST * (1 - math.exp(-DV / SPS_VEX))

# The hyperbola: periselene speed for the target orbit plus DV.
RP = R_MOON + PERI_ALT
RA = R_MOON + APO_ALT
VP = math.sqrt(MUM * (2 / RP - 2 / (RP + RA))) + DV
SMA = 1 / (2 / RP - VP**2 / MUM)        # negative
ECC = 1 - RP / SMA
N_HYP = math.sqrt(MUM / -SMA**3)
T_PERI = TIG_GET + BURN_S / 2

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
# Retrograde to the moon's orbit, periselene on the far side.
N_HAT = unit(cross(MOON_V, MOON))
P_HAT = unit(lin((1, unit(MOON)), (-dot(unit(MOON), N_HAT), N_HAT)))
Q_HAT = cross(N_HAT, P_HAT)


def state(get):
    """The true state (two-body, moon-centered) at GET get."""
    M = N_HYP * (get - T_PERI)
    H = math.asinh(M / ECC) if M else 0.0
    for _ in range(60):
        H = H - (ECC * math.sinh(H) - H - M) / (ECC * math.cosh(H) - 1)
    a = -SMA
    x = a * (ECC - math.cosh(H))
    y = a * math.sqrt(ECC**2 - 1) * math.sinh(H)
    hdot = N_HYP / (ECC * math.cosh(H) - 1)
    vx = -a * math.sinh(H) * hdot
    vy = a * math.sqrt(ECC**2 - 1) * math.cosh(H) * hdot
    return lin((x, P_HAT), (y, Q_HAT)), lin((vx, P_HAT), (vy, Q_HAT))


R_LOAD, V_LOAD = state(LOAD_GET)
R_TIG, V_TIG = state(TIG_GET)
# S30.1's local vertical axes: Y = -unit(r x v), Z = -unit(r), X = Y x Z.
LV_Y = [-c for c in unit(cross(R_TIG, V_TIG))]
LV_Z = [-c for c in unit(R_TIG)]
LV_X = cross(LV_Y, LV_Z)
DELV = [-DV * c for c in unit(V_TIG)]
DELVSLV = [dot(DELV, ax) for ax in (LV_X, LV_Y, LV_Z)]

Vcs = [c * 0.01 for c in V_LOAD]
vec('RRECTCSM', R_LOAD, 27); vec('VRECTCSM', Vcs, 5)
put('TETCSM', dp(T_LOAD / 2**28))
put('DELTACSM', [0] * 6); put('NUVCSM', [0] * 6)
vec('RCVCSM', R_LOAD, 27); vec('VCVCSM', Vcs, 5)
put('TCCSM', [0, 0]); put('XKEPCSM', [0, 0])
flag(123)               # CMOONFLG: moon sphere
put('TIG', dp(TIG_CS / 2**28))
vec('DELVSLV', [c * 0.01 for c in DELVSLV], 7)

# The platform: X along the DV, Z toward the moon. The vehicle at P40's
# preferred attitude: S40.2,3 points the engine bell (PBIAS -2.15 and YBIAS
# +0.95 degrees off body X with the trims at zero) along the DV, which in
# this platform is inner gimbal +2.15 and middle gimbal -0.95 degrees.
SM_X = unit(DELV)
SM_Z = unit(lin((1, LV_Z), (-dot(LV_Z, SM_X), SM_X)))
SM_Y = cross(SM_Z, SM_X)
put('REFSMMAT', sum((dp(c / 2) for row in (SM_X, SM_Y, SM_Z) for c in row), []))
flag(47)                # REFSMFLG
# S40.1 turns the impulsive DV back by half the burn arc (THETAT/2) to get
# the ignition thrust direction VGTIG, so P40's preferred attitude is
# pitched up from the DV. These are the gimbal angles the AGC computes
# (THETAD after S40.2,3); starting there makes R60's maneuver nil.
IGN_CDU = (0, 0o3224, 0o77651)
put('CDUX', [IGN_CDU[0]]); put('CDUY', [IGN_CDU[1]]); put('CDUZ', [IGN_CDU[2]])
# The RCS DAP's desired angles (DP, high words) start at the same attitude.
put('CDUXD', [IGN_CDU[0], 0, IGN_CDU[1], 0, IGN_CDU[2], 0])

# TVC DAP: N46 R1 CSM+LM (A digit 2), quads; N47 masses (B+16 kg); EMDOT
# (kg/cs B+3); no trim offsets.
put('DAPDATR1', [0o21102]); put('DAPDATR2', [0o01111])
put('CSMMASS', [int(round(CSM_KG / 2**16 * 2**14))])
put('LEMMASS', [int(round(LM_KG / 2**16 * 2**14))])
put('EMDOT', [int(round(SPS_THRUST / SPS_VEX * 0.01 / 8 * 2**14))])
put('PACTOFF', [0]); put('YACTOFF', [0])
put('WEIGHT/G', [int(round((CSM_KG + LM_KG) / 2**16 * 2**14))])
# SPS pads (synthetic): the 1-second impulse and the tail-off impulse
# (N s / 100, B+23), the thrust (N / 1E4: EK3VAL B+9, FANG B+7), the
# low-thrust threshold (1 m/s in a 2-second cycle, m/cs B-2) and the
# tail-off decay time (25 cs, B+14).
put('EK1VAL', dp(SPS_THRUST / 100 / 2**23)); put('EK2VAL', dp(10000 / 100 / 2**23))
put('EK3VAL', [int(round(SPS_THRUST / 1e4 / 2**9 * 2**14))])
put('FANG', [int(round(SPS_THRUST / 1e4 / 2**7 * 2**14))])
put('DVTHRESH', [int(round(0.01 / 2**-2 * 2**14))])
put('ETDECAY', [25])

# Ephemerides (as cm_p23.py).
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

print('hyperbola: v_inf %.0f m/s, periselene %.0f km at TIG + %.0f s, speed %.0f m/s' % (
    math.sqrt(MUM / -SMA), PERI_ALT / 1e3, BURN_S / 2, VP))
print('load at %.0f s before TIG: %.0f km from the moon, %.0f m/s' % (
    (TIG_CS - T_LOAD) / 100, norm(R_LOAD) / 1e3, norm(V_LOAD)))
print('DELVSLV (N81) %.1f %.1f %.1f m/s; burn about %.0f s' % (*DELVSLV, BURN_S))

out_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/cm-p40.load'
with open(out_path, 'w') as f:
    f.write('# CM P30/P40 (LOI-1) scenario load at 20 s (tools/flight_scenarios/cm_p40.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
print('words', len(out))
