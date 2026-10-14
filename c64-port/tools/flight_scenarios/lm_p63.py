#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""LM P63 powered-descent scenario load (docs/FLIGHT-STORE.md, "Flight
scenarios"): the erasable words to set at 20 s after a cold start with the
IMU operating, so V37E63E runs P63 through IGNALG, R51, R60 and BURNBABY to
ignition, and SERVICER, LUNLAND and QUADGUID (the LLGE) run every 2 s from
TIG+26 s.

The values are synthetic but self-consistent (no Apollo 11 pad load is
reachable here): Tranquility Base, a 15 x 110 km descent orbit with
perilune at ignition 16 degrees uprange, Kepler-propagated to the load
time; REFSMMAT X up through the site at landing, Z downrange; braking and
approach targets shaped like the flight's. Scales are LLGE's: positions
B24 m, velocities B10 m/cs, accelerations B-4 m/cs^2, TTF = x * 2^17 cs.

  lm_p63.py [OUT]   (default tools/flight_scenarios/lm-p63.load)
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


MU = 4.9028e12          # m^3/s^2
CS = 0.01
T_LOAD = 2000           # cs
T_IG = 20000            # cs, planned ignition
GUIDDURN = 664.40       # s
ZOOM = 26.0             # s

# Landing site, moon fixed (Tranquility Base).
lat, lon, rls = math.radians(0.6875), math.radians(23.433), 1735470.0
RLS = [rls * math.cos(lat) * math.cos(lon), rls * math.cos(lat) * math.sin(lon), rls * math.sin(lat)]
T_LAND = T_IG + int((GUIDDURN + ZOOM) * 100)

# LAND: the site at TLAND in the reference frame, metres, as P63's
# initialization computes it from RLS (read back from an erasable dump).
land = [-1270721.0, 1087493.0, 463124.0]

n = math.sqrt(sum(c * c for c in land))
X = [c / n for c in land]
# Orbit normal: perpendicular to the site, near the lunar equator plane.
up = [0, 0, 1.0]
Z = [up[1] * X[2] - up[2] * X[1], up[2] * X[0] - up[0] * X[2], up[0] * X[1] - up[1] * X[0]]
zn = math.sqrt(sum(c * c for c in Z)); Z = [c / zn for c in Z]       # downrange
Y = [Z[1] * X[2] - Z[2] * X[1], Z[2] * X[0] - Z[0] * X[2], Z[0] * X[1] - Z[1] * X[0]]

# REFSMMAT rows: the stable member axes in reference coordinates, B1.
put('REFSMMAT', sum((dp(c / 2) for row in (X, Y, Z) for c in row), []))
flag(47)                # REFSMFLG

# Descent orbit: 15 km x 110 km above 1738 km, perilune at ignition, 16 deg uprange.
rp, ra = 1738e3 + 15e3, 1738e3 + 110e3
a = (rp + ra) / 2; e = (ra - rp) / (ra + rp)
th0 = -math.radians(16.0)          # true anomaly reference: perilune direction
def kepler_state(dt):
    """State dt seconds from perilune, in the (X, Z) plane, perilune at th0 from the site."""
    nmot = math.sqrt(MU / a**3)
    M = nmot * dt
    E = M
    for _ in range(50):
        E = E - (E - e * math.sin(E) - M) / (1 - e * math.cos(E))
    nu = 2 * math.atan2(math.sqrt(1 + e) * math.sin(E / 2), math.sqrt(1 - e) * math.cos(E / 2))
    r = a * (1 - e * math.cos(E))
    p = a * (1 - e * e)
    h = math.sqrt(MU * p)
    vr = MU / h * e * math.sin(nu); vt = h / r
    ang = th0 + nu
    pos = [r * (math.cos(ang) * X[i] + math.sin(ang) * Z[i]) for i in range(3)]
    vel = [vr * (math.cos(ang) * X[i] + math.sin(ang) * Z[i]) + vt * (-math.sin(ang) * X[i] + math.cos(ang) * Z[i]) for i in range(3)]
    return pos, vel

pos, vel = kepler_state((T_LOAD - T_IG) / 100.0)
R = [c for c in pos]; V = [c * CS for c in vel]     # m, m/cs
# Permanent LM state: RRECT, VRECT, TET, DELTA=0, NUV=0, RCV, VCV, TC=0, XKEP=0.
vec('RRECTLEM', R, 27); vec('VRECTLEM', V, 5)
put('TETLEM', dp(T_LOAD / 2**28))
put('DELTALEM', [0] * 6); put('NUVLEM', [0] * 6)
vec('RCVLEM', R, 27); vec('VCVLEM', V, 5)
put('TCLEM', [0, 0]); put('XKEPLEM', [0, 0])
flag(124)               # LMOONFLG

vec('RLS', RLS, 27)
put('TLAND', dp(T_LAND / 2**28))

# ---- P63 pad loads (W matrix), synthetic but self-consistent ----
# Guidance frame: x up through the site, y crossrange, z downrange.
# Scales (from LLGE): positions B24 m, velocities B10 m/cs,
# accelerations B-4 m/cs^2; TTF = x * 2^17 cs, so the cubic's
# coefficients are VDG2TTF = 18 VDGz / 8 (B10), ADG2TTF = 6 ADGz (B-4),
# JDG2TTF = JDGz * 2^21 (m/cs^3). TTF/8-unit words count 8 cs of TTF.
def guid(prefix, r, v, acc, jz, gain, tcgf, tcgi):
    v = [c * CS for c in v]; acc = [c * CS * CS for c in acc]; jz = jz * CS**3
    vec(prefix[0], r, 24); vec(prefix[1], v, 10); vec(prefix[2], acc, -4)
    put(prefix[3], dp(18 * v[2] / 8 / 2**10))
    put(prefix[4], dp(6 * acc[2] / 2**-4))
    put(prefix[5], dp(jz * 2**21))
    put(prefix[6], dp(gain)); put(prefix[7], [tcgf, tcgi])

# Braking phase: aimed past high gate (2.3 km up, 8 km uprange).
guid(('RBRFG', 'VBRFG', 'ABRFG', 'VBRFG*', 'ABRFG*', 'JBRFG*', 'GAINBRAK', 'TCGFBRAK'),
     [2300.0, 0.0, -8000.0], [-40.0, 0.0, 150.0], [0.5, 0.0, -1.2], 0.002, 0.0, 0o37777, 0)
# Approach phase: to 30 m above the site, nearly stopped.
guid(('RAPFG', 'VAPFG', 'AAPFG', 'VAPFG*', 'AAPFG*', 'JAPFG*', 'GAINAPPR', 'TCGFAPPR'),
     [30.0, 0.0, -10.0], [-1.0, 0.0, 0.5], [0.05, 0.0, -0.1], 0.0002, 0.0, 0o37777, 0)
# Ignition: K gains 0, so TIG puts the LM RIGNZ downrange of the site.
rig = kepler_state(0.0)[0]
rg = [sum((rig[i] - land[i]) * row[i] for i in range(3)) for row in (X, Y, Z)]
put('VIGN', dp(1690.0 * CS / 2**10))
put('RIGNX', dp(rg[0] / 2**24)); put('RIGNZ', dp(rg[2] / 2**24))
for k in ('KIGNX/B4', 'KIGNY/B8', 'KIGNV/B4'):
    put(k, [0, 0])
put('LOWCRIT', [2131, 2320])            # LOWCRIT, HIGHCRIT: throttle bits (12.5 N)
out[addr('ZOOMTIME')] = 2600            # cs
out[addr('TENDBRAK')] = 6000 // 8       # braking ends 60 s before its target
out[addr('TENDAPPR')] = 1000 // 8
out[addr('DELTTFAP')] = 0
out[addr('LEADTIME')] = sp(-26 / 2**14)
out[addr('LEMMASS')] = 15100 // 4       # kg, B16
out[addr('CSMMASS')] = 0
print('ignition target RG', [round(c) for c in rg])

out_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/lm-p63.load'
with open(out_path, 'w') as f:
    f.write('# LM P63 scenario load at 20 s (tools/flight_scenarios/lm_p63.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
print('LAND addr %04o' % addr('LAND'), 'TIG addr %04o' % addr('TIG'), 'words', len(out))
