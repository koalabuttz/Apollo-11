#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""How often each interpretive order runs in the flight scenarios, from
yaAGC (tools/interp_order_trace.c), and what loading a set of "cold"
orders as a second job-slot module would cost: one load each time the
order stream enters the cold set and one each time it leaves it, at
SWAP_CYCLES each (steward's route 2 test; docs/INTERPRETER-RUNTIME.md).

Scenarios are tools/flight_host_differential.py's LM ones (fresh start,
then DSKY keys), plus V37E63E with the IMU operating and the DAP enabled
(FLIGHT_CH30=036377 FLIGHT_CH31=057777).

  interp_order_rates.py [--cold SQRT,SIN,...] [--cs N] [--mission lm|cm]

Writes build/interp-rates/<mission>.json and prints per scenario the
orders a second, the busiest second, and cold-set loads a second."""
import argparse
import collections
import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import interp_listing as IL  # noqa: E402

R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
B = os.path.join(R, 'build')
YAAGC = os.path.join(R, 'deps/virtualagc/yaAGC')
ROPE = {'lm': 'evidence/fixed-data-lm-original.rope.bin', 'cm': 'evidence/fixed-data-cm-original.rope.bin'}
TABLES = (('INDJUMP', 32), ('MISCJUMP', 16), ('UNAJUMP', 15))
IMU_DAP = {'FLIGHT_CH30': '036377', 'FLIGHT_CH31': '057777'}
SCENARIOS = {'lm': [('P00 idle', 4000, '', {}), ('V37E00E', 3000, 'V37E00E', {}), ('V37E12E', 3000, 'V37E12E', {}),
                    ('V37E63E', 3000, 'V37E63E', {}), ('V37E63E, IMU and DAP on', 3000, 'V37E63E', IMU_DAP)],
             'cm': [('P00 idle', 4000, '', {}), ('V37E00E', 3000, 'V37E00E', {}), ('V37E06E', 3000, 'V37E06E', {})]}
COLD = 'SQRT,SIN,COS,ASIN,ACOS,UNIT,DDV,BDDV,VXV,MXV,VXM,V/SC,VPROJ,DOT,VSQ,VXSC'
SWAP_CYCLES = 30000
PAL_CYCLES_PER_S = 985248


def names(L):
    """{(table, offset): order name} from the TCF operands of the tables."""
    out = {}
    for lab, n in TABLES:
        base = L.labels[lab]
        for k in range(n):
            ln = L.by_phys.get(base + k)
            out[(lab, k)] = ln.operand.split()[0] if ln is not None and ln.op == 'TCF' and ln.operand else '?'
    return out


def build():
    exe = os.path.join(B, 'interp-order-trace')
    subprocess.run(['cc', '-std=c99', '-O2', '-Wall', '-Wextra', '-Iinclude', '-I' + YAAGC, 'tools/interp_order_trace.c',
                    'build/agc_engine_hardware_dv.o', os.path.join(YAAGC, 'agc_engine_init.o'),
                    os.path.join(YAAGC, 'rfopen.o'), '-lm', '-o', exe], cwd=R, check=True)
    return exe


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--mission', choices=('lm', 'cm'), default='lm')
    ap.add_argument('--cold', default=COLD, help='comma-separated order names (the TCF targets of the tables)')
    a = ap.parse_args()
    L = IL.load(a.mission)
    nm = names(L)
    cold = set(a.cold.split(','))
    exe = build()
    args = ['%d:%s:%d' % (L.labels[lab], lab, n) for lab, n in TABLES]
    report = []
    for title, cs, keys, env in SCENARIOS[a.mission]:
        r = subprocess.run([exe, ROPE[a.mission], str(cs), keys, *args], cwd=R, capture_output=True, text=True,
                           env=dict(os.environ, **env))
        if r.returncode:
            sys.exit('%s: trace failed: %s' % (title, r.stderr.strip()))
        events = []
        for line in r.stdout.splitlines():
            t, lab, off = line.split()
            events.append((int(t), nm[(lab, int(off))]))
        secs = cs / 100.0
        count = collections.Counter(n for _, n in events)
        per_sec = collections.Counter(t // 100 for t, _ in events)
        loads, inside = 0, False
        loads_sec = collections.Counter()
        for t, n in events:
            if (n in cold) != inside:
                inside = not inside
                loads += 1
                loads_sec[t // 100] += 1
        row = dict(scenario=title, cs=cs, orders=len(events), orders_per_s=round(len(events) / secs, 1),
                   busiest_second=max(per_sec.values()) if per_sec else 0,
                   cold_orders_per_s=round(sum(count[c] for c in cold) / secs, 2),
                   cold_loads_per_s=round(loads / secs, 2), cold_loads_busiest_second=max(loads_sec.values()) if loads_sec else 0,
                   cold_load_cpu=round(loads / secs * SWAP_CYCLES / PAL_CYCLES_PER_S, 3),
                   by_order={k: round(v / secs, 2) for k, v in count.most_common()})
        report.append(row)
        print('%-26s %6.1f orders/s (busiest second %d); cold set %5.2f orders/s, %5.2f loads/s '
              '(busiest second %d) = %.1f%% of a PAL C64 at %d cycles a load' %
              (title, row['orders_per_s'], row['busiest_second'], row['cold_orders_per_s'], row['cold_loads_per_s'],
               row['cold_loads_busiest_second'], 100 * row['cold_load_cpu'], SWAP_CYCLES))
        print('    ' + ', '.join('%s %.1f' % kv for kv in list(row['by_order'].items())[:14]))
    os.makedirs(os.path.join(B, 'interp-rates'), exist_ok=True)
    with open(os.path.join(B, 'interp-rates', a.mission + '.json'), 'w') as f:
        json.dump(dict(cold=sorted(cold), swap_cycles=SWAP_CYCLES, scenarios=report), f, indent=1)
    return 0


if __name__ == '__main__':
    sys.exit(main())
