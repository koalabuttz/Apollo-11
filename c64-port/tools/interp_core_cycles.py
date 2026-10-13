#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""6502 cycles per interpretive order of the compact interpreter
(src/lm_interp_core.c), C64 flight flags, in llvm-mos's simulator.

The fuzz (tools/interp_core_fuzz.c, FUZZ_SINGLE=1: one order per word)
writes each slot that matched the verification interpreter as a record;
tools/interp_core_cycles.c runs each record's slot once in mos-sim with the
image at $7000 and checks the result against the host's. A slot's C64 cost
is its cycles plus its rope reads times the REU read path (measured, plus
the two DMA cycles). Prints a table per mission and writes
build/interp-cycles/<m>.json. Fails if any slot's 6502 result differs from
the host's.

  interp_core_cycles.py [--cases N] [--seed S]"""
import argparse
import collections
import json
import os
import statistics
import subprocess
import sys

R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
B = os.path.join(R, 'build')
OUT = os.path.join(B, 'interp-cycles')
CC = os.environ.get('MOS_SIM_CC', os.path.join(R, 'deps/llvm-mos/bin/mos-sim-clang'))
SIM = os.path.join(os.path.dirname(CC), 'mos-sim')
# The flight store's flags (tools/resident_flight_build.py COMMON).
FLAGS = ['-Oz', '-fno-static-stack', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', '-Iplatform/c64']
ROPES = [os.path.join(B, 'fixed-data/Luminary099/MAIN.agc.bin'), os.path.join(B, 'fixed-data/Comanche055/MAIN.agc.bin')]
ADDR = ['VLOAD', 'TAD', 'SIGN', 'VXSC', 'CGOTO', 'TLOAD', 'DLOAD', 'V/SC', 'SLOAD', 'SSP', 'PDDL', 'MXV', 'PDVL',
        'CCALL', 'VXM', 'TSLC', 'DMPR', 'DDV', 'BDDV', 'GSHIFT', 'VAD', 'VSU', 'BVSU', 'DOT', 'VXV', 'VPROJ', 'DSU',
        'BDSU', 'DAD', '-', 'DMP', 'SETPD']
MISC = ['AXT', 'AXC', 'LXA', 'LXC', 'SXA', 'XCHX', 'INCR', 'TIX', 'XAD', 'XSU', 'BZE/GOTO', 'BPL/BMN', 'RTB/BHIZ',
        'CALL/ITA', 'SWITCH', 'BOV']
UNARY = ['SQRT', 'SIN', 'COS', 'ASIN', 'ACOS', 'DSQ', 'ROUND', 'DCOMP', 'VDEF', 'UNIT', 'ABVAL/ABS', 'VSQ', 'STADR',
         'RVQ', 'PUSH']
STORE = ['STORE', 'STORE,1', 'STORE,2', 'STODL', 'STODL,*', 'STOVL', 'STOVL,*', 'STCALL']


def order_name(word):
    if not word & 0o40000:
        return STORE[(word >> 11) & 7]
    c = (((~word) & 0o77777) - 1) & 0o177
    if c == 0:
        return 'EXIT'
    if c & 1:
        return ADDR[c >> 2]
    if c & 2:
        return MISC[c >> 3]
    if c & 4:
        return 'SHIFT'
    return UNARY[(c >> 3) - 1]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--cases', type=int, default=6000)
    ap.add_argument('--seed', type=int, default=5)
    a = ap.parse_args()
    os.makedirs(OUT, exist_ok=True)
    fuzz = os.path.join(B, 'interp-core-fuzz')
    subprocess.run(['make', '-s', 'build/interp-core-fuzz'], cwd=R, check=True)
    rr = os.path.join(OUT, 'rope-reu.o')
    subprocess.run([CC, *FLAGS, '-Dapollo_rope_read=reu_rope_read', '-c', 'platform/c64/rope_image_reu.c', '-o', rr],
                   cwd=R, check=True)
    bad = 0
    for mi, m in enumerate(('lm', 'cm')):
        prg = os.path.join(OUT, 'cycles-%s' % m)
        subprocess.run([CC, *FLAGS, '-mlto-zp=0', '-DINTERP_CYCLES_MISSION=%d' % mi, '-T', 'tools/interp_core_cycles.ld',
                        'tools/interp_core_cycles.c', 'src/%s_interp_core.c' % m, 'src/agc_word.c', 'src/agc_multiword.c',
                        'src/agc_hardware_divide.c', 'src/overlay_reu.c', rr, '-o', prg], cwd=R, check=True)
        dump = os.path.join(OUT, '%s.dump' % m)
        subprocess.run([fuzz, *ROPES, str(a.cases), str(a.seed), str(mi)], cwd=R, check=True, stdout=subprocess.DEVNULL,
                       env=dict(os.environ, FUZZ_SINGLE='1', FUZZ_DUMP=dump))
        with open(dump, 'rb') as f:
            out = subprocess.run([SIM, prg], stdin=f, cwd=R, check=True, capture_output=True, text=True).stdout.splitlines()
        os.remove(dump)
        rope = int(out[0].split()[1]) + 2
        per = collections.defaultdict(list)
        for l in out[1:]:
            t = l.split()
            if t[-1] == 'MISMATCH':
                bad += 1
                continue
            per[order_name(int(t[1], 8))].append(int(t[2]) + int(t[3]) * rope)
        table = {k: dict(slots=len(v), median=int(statistics.median(v)), p90=sorted(v)[int(len(v) * 0.9)], max=max(v))
                 for k, v in sorted(per.items())}
        with open(os.path.join(OUT, '%s.json' % m), 'w') as f:
            json.dump(dict(rope_read_cycles=rope, slots=len(out) - 1, orders=table), f, indent=1)
        print('%s: %d slots, rope read %d cycles; per slot (one order), C64 cycles' % (m.upper(), len(out) - 1, rope))
        print('  %-10s %6s %8s %8s %8s' % ('order', 'slots', 'median', 'p90', 'max'))
        for k, v in sorted(table.items(), key=lambda kv: -kv[1]['median']):
            print('  %-10s %6d %8d %8d %8d' % (k, v['slots'], v['median'], v['p90'], v['max']))
    if bad:
        print('%d slots differ from the host' % bad)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
