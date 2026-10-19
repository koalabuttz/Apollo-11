#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The DSKY sweep: verb and noun key sequences every Pinball display and
load path takes, for a flight run from a fresh start (Lane C, #674).

  python3 tools/dsky_sweep_cases.py --list            # one case a line
  python3 tools/dsky_sweep_cases.py --host [--jobs N] # run each on the host

A case is (mission, keys, centiseconds): keys in the form of the flight
runs' key argument (tools/flight_host_run.c keys_parse: V, N, digits, +, -,
E for ENTR, C for CLR, K for KEY REL, R for RSET; PRO is channel 32 bit 14,
FLIGHT_CHAN, not a key), keyed at the start of the run, and the run length.
Noun 15 (increment machine address) is run only after N01 sets NOUNADD: from
the fresh-start NOUNADD it reads central registers 1-3, which the
certificate's flight domain refuses (docs/LANEC-PIN-CERT.md). The host
check runs build/flight-store/flight-host-run-MISSION (make flight-store,
or tools/flight_store_build.py --host-only) in scaled time
(FLIGHT_SCALED=1,1,2) and fails on any Pinball point it reports as
refused or unregistered, or any native routine yaAGC did not match. A C64
sweep reuses cases() and compares the C64 run with the host's.
"""
import argparse, concurrent.futures, os, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
NOUNS = [f'{n:02d}' for n in range(1, 100)]
# nouns for the monitor verbs: low erasable, mixed, time, top of erasable
MONITOR_NOUNS = ['05', '13', '16', '20', '21', '34', '36', '43', '49', '54', '62', '65', '68', '92']
CS = 1000


def cases():
    out = []
    for m in ('lm', 'cm'):
        for v in ('01', '02', '03', '04', '05', '06', '21', '22', '23', '24', '25'):
            out += [(m, f'V{v}N{n}E', CS) for n in NOUNS if n != '15' or v in ('06', '21', '22', '23', '24', '25')]
        for v in ('11', '12', '13', '14', '15', '16', '17'):
            out += [(m, f'V{v}N{n}E', CS) for n in MONITOR_NOUNS]
        out += [(m, k, CS) for k in ('V32E', 'V33E', 'V34E', 'V35E', 'V36E', 'V37E00E', 'V37E06E', 'V37E12E',
                                     'V82E', 'V83E', 'V16N36EV32E', 'V27N02E02000E',
                                     'V01N01E01000EV01N15E', 'V05N01E01000EV05N15E', 'V21N01E00010E',
                                     'V16N36EK', 'V16N36EKR', 'V05N09EC')]
    return out


def host(case):
    m, keys, cs = case
    exe = ROOT / f'build/flight-store/flight-host-run-{m}'
    rope = ROOT / f'evidence/fixed-data-{m}-original.rope.bin'
    r = subprocess.run([str(exe), str(rope), str(cs), keys], cwd=ROOT, capture_output=True,
                       env=dict(os.environ, FLIGHT_SCALED='1,1,2'), errors='replace')
    text = r.stdout + r.stderr
    bad, points = [], False
    for line in text.splitlines():
        if line.startswith('unregistered or refused points'):
            points = True
            continue
        # a Pinball point (banks 40-43) nothing claims or that was refused
        if points and re.match(r'\s+\w+\s+4[0-3],[0-7]+ ', line):
            bad.append('unregistered: ' + line.strip())
        elif points and not line.startswith(' '):
            points = False
        if 'refused by lanec_pin' in line or re.search(r'did not match \([1-9]', line):
            bad.append(line.strip())
    return case, r.returncode, bad


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--list', action='store_true')
    p.add_argument('--host', action='store_true')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 1)
    a = p.parse_args()
    if a.list or not a.host:
        for m, keys, cs in cases():
            print(m, keys, cs)
        sys.exit(0)
    failed = 0
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
        for (m, keys, cs), rc, bad in pool.map(host, cases()):
            if rc or bad:
                failed += 1
                print(f'FAIL: {m} {keys}: ' + ('; '.join(bad) or f'exit {rc}'))
    n = len(cases())
    print(f'{"FAIL" if failed else "PASS"}: DSKY sweep on the host: {n - failed} of {n} runs with no refused or '
          f'unregistered Pinball point')
    sys.exit(1 if failed else 0)
