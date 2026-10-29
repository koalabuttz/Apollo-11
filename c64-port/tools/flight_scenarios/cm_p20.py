#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM P20 scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"): the
CSM side of the LM rendezvous, so V37E20E runs Comanche's P20 (R61CSM's
tracking attitude, R52's auto optics on the LM, R22's VHF range and SXT
marks into the LM state vector and the W matrix).

The orbits are lm_rendezvous.py's (Lane D2's LM P20/P34 scenario), the
same moment seen from the CSM: the CSM in a 60 nmi circular lunar orbit,
the LM coelliptic 15 nmi below it, trailing by 2.1 degrees, in the P63
scenario's orbit plane (the CMC's LM state 1.8 km too far); REFSMMAT the LM's local vertical at the load (X up,
Y the negative orbit normal, Z downrange). The CSM starts at R61's
preferred tracking attitude. The LM is the active vehicle (AVFLAG), as on Apollo 11.

Prints the FLIGHT_RR truth (tools/flight_radar.h) for the same orbits: the
CM side of the model answers VHF range reads and points the optics at the
LM.

  cm_p20.py [OUT]   (default tools/flight_scenarios/cm-p20.load)
"""
import math, re, sys
from pathlib import Path

from sighting import unit

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


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


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


MU = 4.9028e12          # m^3/s^2
RMOON = 1738e3
CS = 0.01
T_LOAD = 2000           # cs
NMI = 1852.0

# lm_rendezvous.py's orbits: the P63 scenario's plane (P up through the
# site at landing, Q downrange), retrograde about the reference Z axis as
# Apollo's lunar orbits were (INITVEL picks Lambert's sense from it).
land = [-1270721.0, 1087493.0, 463124.0]
P = unit(land)
Q = unit(cross(P, [0, 0, 1.0]))


def circular(radius, angle):
    v = math.sqrt(MU / radius)
    u = [math.cos(angle) * P[i] + math.sin(angle) * Q[i] for i in range(3)]
    t = [-math.sin(angle) * P[i] + math.cos(angle) * Q[i] for i in range(3)]
    return [radius * c for c in u], [v * c for c in t]


R_CSM = RMOON + 60 * NMI
R_LM = R_CSM - 15 * NMI
LM_ANGLE = math.radians(40.0)
LEAD = math.radians(2.1)
rl, vl = circular(R_LM, LM_ANGLE)
rc, vc = circular(R_CSM, LM_ANGLE + LEAD)

X = unit(rl)
Z = unit(vl)
Y = cross(Z, X)
SM = [X, Y, Z]
put('REFSMMAT', sum((dp(c / 2) for row in SM for c in row), []))
flag(47)                # REFSMFLG

# Permanent state vectors, moon centred (positions B27 m, velocities B5
# m/cs): the CSM's own and the LM's.
# The CMC's LM state is off the truth (FLIGHT_RR's) by ERR_LM metres along
# the line of sight (the LM farther than it is), so the first VHF range
# update exceeds RMAX and R22 asks for it (V06N49, RENDISP7's wait on the
# crew's answer), while R61's tracking attitude is unchanged.
ERR_LM = 1800.0
los = unit([rl[i] - rc[i] for i in range(3)])
rl_est = [rl[i] + ERR_LM * los[i] for i in range(3)]
for veh, (r, v) in (('CSM', (rc, vc)), ('LEM', (rl_est, vl))):
    V = [c * CS for c in v]
    vec('RRECT' + veh, r, 27); vec('VRECT' + veh, V, 5)
    put('TET' + veh, dp(T_LOAD / 2**28))
    put('DELTA' + veh, [0] * 6); put('NUV' + veh, [0] * 6)
    vec('RCV' + veh, r, 27); vec('VCV' + veh, V, 5)
    put('TC' + veh, [0, 0]); put('XKEP' + veh, [0, 0])
flag(123)               # CMOONFLG
flag(124)               # LMOONFLG
flag(40)                # AVFLAG: the LM is the active vehicle

# R22's W matrix initial values, VHF variance and update limits (as Lane
# D2's LM side: 10,000 ft and 10 ft/s; V06N49 above 2,000 ft or 2 ft/s).
# WRENDPOS is metres B-14, WRENDVEL m/cs (the erasable comments). RVAR is
# the VHF range's relative variance, scaled as SXTVAR (B+16): R22 multiplies
# it by the range squared (REND10). Left zero, the VHF mark's variance is
# the integration variance alone and the filter takes the whole residual.
# RVARMIN (the variance floor, stored negative) stays zero: no floor.
out[addr('WRENDPOS')] = sp(3048.0 / 2**14)
out[addr('WRENDVEL')] = sp(0.03048)
put('RVAR', dp(0.001**2 * 2**16))         # 0.1 % of range
out[addr('RMAX')] = sp(609.6 / 2**19)
out[addr('VMAX')] = sp(0.6096 * CS / 2**7)
out[addr('LEMMASS')] = 2800 // 4         # kg, B16: the ascent stage
out[addr('CSMMASS')] = 16500 // 4

# Attitude: R61's preferred tracking attitude on this load, the angles R63
# computes (THETAD 00003 31622 00001 at the first pass, 56 s: a pitch of
# 145.04 degrees in the orbit plane). Within R61's 10 degrees, P20 takes no
# KALCMANU maneuver (the host run flies no CM DAP; KALCMANU's default
# 0.05 degree/s rate would take the whole run).
put('CDUX', [0o00003]); put('CDUY', [0o31622]); put('CDUZ', [0o00001])
# The DAP's desired angles hold the same attitude (the host's attitude
# model, tools/flight_radar.h, keeps CDUX-Z on CDUXD-ZD).
put('CDUXD', [0o00003]); put('CDUYD', [0o31622]); put('CDUZD', [0o00001])

out_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'tools/flight_scenarios/cm-p20.load'
with open(out_path, 'w') as f:
    f.write('# CM P20 scenario load at 20 s (tools/flight_scenarios/cm_p20.py)\n')
    for a_, v in sorted(out.items()):
        f.write('%04o %05o\n' % (a_, v))
rr = lambda r, v: ','.join('%.3f' % c for c in r + v)
print('words', len(out), 'range %.1f km' % (math.dist(rl, rc) / 1000))
print('FLIGHT_RR=%d:%s:%s' % (T_LOAD, rr(rl, vl), rr(rc, vc)))
