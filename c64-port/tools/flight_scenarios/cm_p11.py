#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""CM P01/P02/P11 scenario load (docs/FLIGHT-STORE.md, "Flight scenarios"):
the prelaunch alignment and earth orbit insertion monitor of Apollo 11.

The pad loads P01, P02 and P11 read (P11.agc "ERASABLE INITIALIZATION"):

- LATITUDE and PADLONG: launch complex 39A, 28.608389 N 80.604139 W
  (revolutions); PGNCSALT 60 m (B29);
- LAUNCHAZ (LUNCHAZ1, P01's gyrocompass azimuth and P11's REFSMMAT):
  Apollo 11's flight azimuth, 72.058 degrees; AZIMUTH (the vehicle's
  azimuth on the pad, which places the navigation base for coarse align):
  90 degrees;
- TEPHEM: the epoch-to-CMC-clock-zero time, so that the clock at liftoff
  (the FLIGHT_SATURN time) is 16 July 1969 13:32:00 UT, counted from
  1 July 1969 0h;
- UNITW (-AYO, AXO and the polar component): the earth's pole along
  reference Z; AZO 0. These and PGNCSALT are illustrative, not the flight's
  pad values (which no listing gives);
- POLYNUM 5 with zero coefficients (pitch 0: the FDAI attitude error
  display holds the liftoff attitude), RPSTART 10 s and POLYSTOP -123 s
  (centiseconds), SATRLRT 0.

The harness's FLIGHT_IMU model (tools/flight_host_run.c) zeros and coarse
aligns the gimbals; FLIGHT_SATURN gives liftoff and the boost's specific
force. This script also integrates that boost (the same thrust and pitch
program, two-body earth gravity, the pad's eastward speed along the
azimuth) and prints the trajectory.

  cm_p11.py LIFTOFF_CS [OUT]   (default tools/flight_scenarios/cm-p11.load)
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

LAT, LON, ALT = 28.608389, -80.604139, 60.0
LAUNCHAZ, AZIMUTH = 72.058, 90.0
LIFTOFF_EPOCH_S = ((15 * 24 + 13) * 60 + 32) * 60   # 16 July 13:32 UT from 1 July 0h


def addr(name, off=0):
    s = SYMS[name]
    if ',' in s:
        b, a = s.split(',')
        a = int(b[1:]) * 0o400 + int(a, 8) - 0o1400
    else:
        a = int(s, 8)
    return a + off


def sp(n):
    assert -0o37777 <= n <= 0o37777, n
    return n & 0o77777 if n >= 0 else (~(-n)) & 0o77777


def dp(f):
    n = int(round(f * 2**28))
    n = max(-(2**28 - 1), min(2**28 - 1, n))
    m = abs(n)
    hi, lo = m >> 14, m & 0o37777
    if n < 0:
        hi, lo = (~hi) & 0o77777, (~lo) & 0o77777
    return [hi, lo]


def tp(n):
    assert 0 <= n < 2**42
    return [n >> 28, (n >> 14) & 0o37777, n & 0o37777]


# The boost, as tools/flight_host_run.c's FLIGHT_SATURN gives it.
def saturn_acc(t):
    if t < 0:
        return 0.0
    if t < 135.2:
        return 35.1e6 / (2960e3 - 13.0e3 * t)
    if t < 161.6:
        return 0.8 * 35.1e6 / (2960e3 - 13.0e3 * 135.2 - 10.4e3 * (t - 135.2))
    if t < 164.0:
        return 0.0
    if t < 460.0:
        return 5.1e6 / (665e3 - 1.22e3 * (t - 164.0))
    if t < 548.0:
        return 4.08e6 / (665e3 - 1.22e3 * 296.0 - 0.98e3 * (t - 460.0))
    if t < 552.0 or t >= 823.0:
        return 0.0
    return 0.9e6 / (167e3 - 0.21e3 * (t - 552.0))


def saturn_pitch(t):
    if t < 12.0:
        d = 0.0
    elif t < 161.6:
        d = 52.2 * (t - 12.0) / 149.6
    elif t < 400.0:
        d = 52.2 + 29.0 * (t - 161.6) / 238.4
    elif t < 700.0:
        d = 81.2 + 12.1 * (t - 400.0) / 300.0
    else:
        d = 93.3
    return math.radians(d)


def trajectory():
    mu, re_ = 3.986004e14, 6373338.0
    x, z = 0.0, re_ + ALT
    vx, vz = 465.1 * math.cos(math.radians(LAT)) * math.sin(math.radians(LAUNCHAZ)), 0.0
    t, dt = 0.0, 0.01
    while t < 830.0:
        a, th = saturn_acc(t + dt / 2), saturn_pitch(t + dt / 2)
        r = math.hypot(x, z)
        g = -mu / r**3
        vx += (a * math.sin(th) + g * x) * dt
        vz += (a * math.cos(th) + g * z) * dt
        x += vx * dt
        z += vz * dt
        t += dt
        if abs(t / 60 - round(t / 60)) < dt / 120:
            print('  %4.0f s  %5.1f m/s2  pitch %5.1f  %6.1f km  %6.0f m/s'
                  % (t, a, math.degrees(th), (math.hypot(x, z) - re_) / 1e3, math.hypot(vx, vz)))
    r, v = math.hypot(x, z), math.hypot(vx, vz)
    sma = -mu / (2 * (v * v / 2 - mu / r))
    h = x * vz - z * vx
    e = math.sqrt(max(0.0, 1 - h * h / (mu * sma)))
    print('insertion: %.0f x %.0f km, %.0f m/s' % ((sma * (1 - e) - re_) / 1e3, (sma * (1 + e) - re_) / 1e3, v))


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    lo_cs = int(sys.argv[1])
    out = {}

    def put(name, words, off=0):
        for i, w in enumerate(words):
            out[addr(name, off) + i] = w

    put('LATITUDE', dp(LAT / 360))
    put('PADLONG', dp(LON / 360))
    put('PGNCSALT', dp(ALT / 2**29))
    put('LUNCHAZ1', dp(LAUNCHAZ / 360))
    put('AZIMUTH', dp(AZIMUTH / 360))
    put('TEPHEM', tp(LIFTOFF_EPOCH_S * 100 - lo_cs))
    put('AZO', dp(0.0))
    put('UNITW', dp(0.0) + dp(0.0) + dp(1.0))
    put('POLYNUM', [5] + [0] * 14)
    put('SATRLRT', dp(0.0))
    put('RPSTART', [sp(1000)])
    put('POLYSTOP', [sp(-12300)])
    trajectory()
    path = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'tools/flight_scenarios/cm-p11.load'
    with open(path, 'w') as f:
        f.write('# CM P01/P02/P11 scenario load (tools/flight_scenarios/cm_p11.py %d): pad loads\n' % lo_cs)
        for a, v in sorted(out.items()):
            f.write('%04o %05o\n' % (a, v))


main()
