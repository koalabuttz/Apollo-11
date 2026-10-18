#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""LM DOI scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"): the
descent orbit insertion burn of Apollo 11, from the NASSP core of
tools/scenarios/luminary099-p63-ignition.core (102:25 GET, after DOI) with
the burn backed out.

The core's LM state (RRECTLEM, VRECTLEM at TETLEM, on the 15 x 114 km
descent orbit) is propagated back to that orbit's apolune, where DOI was
made, and the burn undone there: 76.4 ft/s (23.29 m/s) added along the
velocity (DOI's delta-V, Apollo 11 Mission Report). That gives the
114 x 117 km pre-DOI orbit. The scenario moves DOI to TIG (below, 8 min
after the core's clock), so the load is the pre-DOI state at TETLEM on
that orbit, two-body Kepler with the moon's GM (no Encke deviations:
DELTALEM, NUVLEM, TCLEM and XKEPLEM stay 0, as in the core). The rest of
the core (REFSMMAT, the CSM, the clock) is the flown one.

Scales (moon sphere): positions B27 m, velocities B5 m/cs.

  lm_doi.py [OUT]   (default tools/flight_scenarios/lm-doi.load)
"""
import math
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Luminary099/assembly.lst'
CORE = ROOT / 'tools/scenarios/luminary099-p63-ignition.core'
SYMS = {}
for line in open(LST, errors='replace'):
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+(\S+)', line):
        SYMS.setdefault(m.group(2), m.group(3))
MU = 4.9028e12          # m^3/s^2
DV_DOI = 76.4 * 0.3048  # m/s
TIG = (102 * 3600 + 34 * 60) * 100   # cs GET: 102:34:00, 8 min after the core's clock


def addr(name, off=0):
    s = SYMS[name]
    if ',' in s:
        b, a = s.split(',')
        a = int(b[1:]) * 0o400 + int(a, 8) - 0o1400
    else:
        a = int(s, 8)
    return a + off


E = [int(x, 8) for x in open(CORE).read().split()][512:512 + 2048]


def s15(w):
    return -(~w & 0o77777) if w & 0o40000 else w


def get_dp(a):
    return (s15(E[a]) * 2**14 + s15(E[a + 1])) / 2**28


def dp(f):
    n = int(round(f * 2**28))
    assert abs(n) < 2**28, f
    m = abs(n)
    hi, lo = m >> 14, m & 0o37777
    if n < 0:
        hi, lo = (~hi) & 0o77777, (~lo) & 0o77777
    return [hi, lo]


def vec_of(name, scale):
    a = addr(name)
    return [get_dp(a + 2 * i) * 2**scale for i in range(3)]


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def norm(a):
    return math.sqrt(dot(a, a))


def stumpff(z):
    if z > 1e-8:
        s = math.sqrt(z)
        return (1 - math.cos(s)) / z, (s - math.sin(s)) / s**3
    if z < -1e-8:
        s = math.sqrt(-z)
        return (math.cosh(s) - 1) / (-z), (math.sinh(s) - s) / s**3
    return 0.5, 1 / 6


def kepler(r0, v0, dt):
    """Two-body state dt seconds from (r0, v0), universal variables."""
    r0n = norm(r0)
    vr = dot(r0, v0) / r0n
    alpha = 2 / r0n - dot(v0, v0) / MU
    sm = math.sqrt(MU)
    x = sm * abs(alpha) * dt
    for _ in range(200):
        z = alpha * x * x
        c, s = stumpff(z)
        f = r0n * vr / sm * x * x * c + (1 - alpha * r0n) * x**3 * s + r0n * x - sm * dt
        df = r0n * vr / sm * x * (1 - alpha * x * x * s) + (1 - alpha * r0n) * x * x * c + r0n
        x -= f / df
        if abs(f / df) < 1e-9:
            break
    z = alpha * x * x
    c, s = stumpff(z)
    f, g = 1 - x * x / r0n * c, dt - x**3 / sm * s
    r = [f * a + g * b for a, b in zip(r0, v0)]
    rn = norm(r)
    fd, gd = sm / (rn * r0n) * (alpha * x**3 * s - x), 1 - x * x / rn * c
    return r, [fd * a + gd * b for a, b in zip(r0, v0)]


def orbit(r, v):
    """Perilune and apolune altitudes (km) over a 1,738.09 km moon."""
    en = dot(v, v) / 2 - MU / norm(r)
    a = -MU / (2 * en)
    h = [r[1] * v[2] - r[2] * v[1], r[2] * v[0] - r[0] * v[2], r[0] * v[1] - r[1] * v[0]]
    e = math.sqrt(max(0.0, 1 - dot(h, h) / (MU * a)))
    return round((a * (1 - e) - 1738090) / 1e3, 1), round((a * (1 + e) - 1738090) / 1e3, 1)


tet = (E[addr('TETLEM')] * 2**14 + E[addr('TETLEM') + 1]) / 100     # s
r0, v0 = vec_of('RRECTLEM', 27), [c * 100 for c in vec_of('VRECTLEM', 5)]
# The descent orbit's apolune before TETLEM, to 0.1 s.
best = max(((norm(kepler(r0, v0, -k)[0]), k) for k in range(0, 7200)))
best = max(((norm(kepler(r0, v0, -(best[1] + d / 10))[0]), best[1] + d / 10) for d in range(-20, 21)))
ra, va = kepler(r0, v0, -best[1])
vp = [c * (norm(va) + DV_DOI) / norm(va) for c in va]
# On the pre-DOI orbit, the LM reaches that point at TIG.
r1, v1 = kepler(ra, vp, -(TIG / 100 - tet))
print('descent orbit', orbit(r0, v0), 'km; DOI point %.1f s before TETLEM; pre-DOI orbit' % best[1],
      orbit(ra, vp), 'km; TIG %d cs' % TIG)

out = {}


def put(name, words):
    for i, w in enumerate(words):
        out[addr(name) + i] = w


for n, v, scale in (('RRECTLEM', r1, 27), ('RCVLEM', r1, 27),
                    ('VRECTLEM', [c / 100 for c in v1], 5), ('VCVLEM', [c / 100 for c in v1], 5)):
    put(n, sum((dp(c / 2**scale) for c in v), []))

out_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/lm-doi.load'
with open(out_path, 'w') as f:
    f.write('# LM DOI scenario load (tools/flight_scenarios/lm_doi.py): the pre-DOI state at TETLEM\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
