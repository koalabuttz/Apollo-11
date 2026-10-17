#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""LM rendezvous scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"):
the erasable words to set at 20 s after a cold start with the IMU operating,
so V37E20E runs P20 (R61's tracking attitude, R21's designate and lock-on,
R22's radar marks into the LM state vector) and V37E34E then targets TPI
(P34) while P20 keeps tracking.

The values are synthetic but self-consistent, shaped like Apollo 11's
rendezvous after CDH: the CSM in a 60 nmi (111 km) circular orbit, the LM
coelliptic 15 nmi below it and trailing it by 2.1 degrees, so the
line of sight rises toward TPI's 26.6 degree elevation about six minutes
into the run. The orbit plane is the P63 scenario's (lm_p63.py: up through
Tranquility Base at landing, downrange along the descent track), so the
LM, the CSM and REFSMMAT sit where the flight's did. REFSMMAT is the LM's
local vertical at the load: X up, Y the negative orbit normal, Z
downrange, and the LM body starts on it (CDUs 0), +Z near the CSM.

Prints the FLIGHT_RR truth (tools/flight_radar.h) for the same orbits.

  lm_rendezvous.py [OUT]   (default tools/flight_scenarios/lm-rendezvous.load)
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


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def unit(a):
    n = math.sqrt(sum(c * c for c in a))
    return [c / n for c in a]


MU = 4.9028e12          # m^3/s^2
RMOON = 1738e3
CS = 0.01
T_LOAD = 2000           # cs
NMI = 1852.0

# The P63 scenario's frame: P up through the site at landing, Q downrange.
land = [-1270721.0, 1087493.0, 463124.0]
P = unit(land)
Q = unit(cross([0, 0, 1.0], P))


def circular(radius, angle):
    """Position and velocity on a circular orbit in the (P, Q) plane, angle
    from P toward Q, moving toward Q."""
    v = math.sqrt(MU / radius)
    u = [math.cos(angle) * P[i] + math.sin(angle) * Q[i] for i in range(3)]
    t = [-math.sin(angle) * P[i] + math.cos(angle) * Q[i] for i in range(3)]
    return [radius * c for c in u], [v * c for c in t]


R_CSM = RMOON + 60 * NMI
R_LM = R_CSM - 15 * NMI
LM_ANGLE = math.radians(40.0)       # past the landing site's meridian
LEAD = math.radians(2.1)            # CSM ahead of the LM
rl, vl = circular(R_LM, LM_ANGLE)
rc, vc = circular(R_CSM, LM_ANGLE + LEAD)

# REFSMMAT rows: the stable member axes in reference coordinates, B1:
# X up through the LM, Y the negative orbit normal, Z downrange.
X = unit(rl)
Z = unit(vl)
Y = cross(Z, X)
put('REFSMMAT', sum((dp(c / 2) for row in (X, Y, Z) for c in row), []))
flag(47)                # REFSMFLG

# Permanent state vectors: RRECT, VRECT, TET, DELTA=0, NUV=0, RCV, VCV,
# TC=0, XKEP=0 (positions B27 m, velocities B5 m/cs about the moon).
for veh, (r, v) in (('LEM', (rl, vl)), ('CSM', (rc, vc))):
    V = [c * CS for c in v]
    vec('RRECT' + veh, r, 27); vec('VRECT' + veh, V, 5)
    put('TET' + veh, dp(T_LOAD / 2**28))
    put('DELTA' + veh, [0] * 6); put('NUV' + veh, [0] * 6)
    vec('RCV' + veh, r, 27); vec('VCV' + veh, V, 5)
    put('TC' + veh, [0, 0]); put('XKEP' + veh, [0, 0])
flag(124)               # LMOONFLG
flag(123)               # CMOONFLG
flag(40)                # AVFLAG: the LM is the active vehicle (P34)

# R22's W matrix and update limits (Apollo 11's pad values: 10,000 ft and
# 10 ft/s, 15 mrad on each RR axis; N49 above 2,000 ft or 2 ft/s).
out[addr('WRENDPOS')] = sp(3.048 / 2**7)
out[addr('WRENDVEL')] = sp(0.03048)
out[addr('WSHAFT')] = sp(0.015)
out[addr('WTRUN')] = sp(0.015)
out[addr('RMAX')] = sp(609.6 / 2**19)
out[addr('VMAX')] = sp(0.6096 * CS / 2**7)
out[addr('LEMMASS')] = 2800 // 4         # kg, B16: the ascent stage
out[addr('CSMMASS')] = 16500 // 4


def elevation(t):
    """TPI's elevation angle (degrees) of the CSM above the LM's local
    horizontal, t cs after the load."""
    n_l, n_c = math.sqrt(MU / R_LM**3), math.sqrt(MU / R_CSM**3)
    dt = t * CS
    a_l, a_c = LM_ANGLE + n_l * dt, LM_ANGLE + LEAD + n_c * dt
    th = a_c - a_l
    return math.degrees(math.atan2(R_CSM * math.cos(th) - R_LM, R_CSM * math.sin(th)))


t = 0
while elevation(t) < 26.6:
    t += 100
T_TPI = T_LOAD + t
# P34's pad: TIG of TPI (the estimate P34 iterates on), elevation 26.6
# degrees and 130 degrees of transfer.
put('TIG', dp(T_TPI / 2**28))
put('ELEV', dp(26.6 / 360))
put('CENTANG', dp(130.0 / 360))
out[addr('NN')] = 0

out_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/lm-rendezvous.load'
with open(out_path, 'w') as f:
    f.write('# LM rendezvous scenario load at 20 s (tools/flight_scenarios/lm_rendezvous.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
rr = lambda r, v: ','.join('%.3f' % c for c in r + v)
print('words', len(out), 'TPI (26.6 deg) at', T_TPI, 'cs; range %.1f km' %
      (math.dist(rl, rc) / 1000), 'elevation at load %.1f deg' % elevation(0))
print('FLIGHT_RR=%d:%s:%s' % (T_LOAD, rr(rl, vl), rr(rc, vc)))
