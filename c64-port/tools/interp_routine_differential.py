#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Differential for compiled-routine milestones (tools/interp_compile.py).

Checks the generated sources are current, builds the yaAGC oracle
(tools/interp_routine_reference.c) and the native runner
(tools/interp_routine_native.c with the milestone's target file), generates
cases for every routine of each mission and compares the stop each side
reaches: Z, the 2048 erasable words (A, L 16-bit) and the interrupt-enable
state.

Cases: the routine's entry state (Z at NEWOPS with LOC, BANKSET, FB,
INTBIT15, or Z for a basic routine), a VAC area as FIXLOC, PUSHLOC inside
it, and fixed CADRs outside the file (or the genuine return points of the
file's callers) in QPRET, S1, S2 and every erasable word a GOTO or CALL of
the file reads a CADR from. The rest of the image is a seeded fill, with
three data shapes: the fill itself, small words in the VAC area and the
file's operands, and half-unit vectors at the file's operands.

  interp_routine_differential.py <spec> [--cases N] [--freeze] [--show K]

--freeze writes tests/<id>_vectors.h (a subset of cases with their oracle
digests) for the host test and the C64 diagnostic, and
build/<id>-proof.json."""
import argparse
import collections
import functools
import hashlib
import json
import math
import operator
import os
import random
import re
import shutil
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import interp_compile as IC
import interp_specs
import interp_listing as IL

R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
B = os.path.join(R, 'build')
E = os.path.join(R, 'deps/virtualagc/yaAGC')
ROPES = [os.path.join(B, 'fixed-data/Luminary099/MAIN.agc.bin'),
         os.path.join(B, 'fixed-data/Comanche055/MAIN.agc.bin')]
BODY_EXITS = {'lm': ['POODOO1', 'ALARM1'], 'cm': ['POODOO', 'ALARM']}
COMMON = ['src/lm_interp_orders.c', 'src/lm_interp_steps.c', 'src/cm_interp_orders.c',
          'src/cm_interp_steps.c', 'src/agc_image_ops.c', 'src/agc_hardware_divide.c',
          'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/interp_case.c']
MOS_CC = os.environ.get('MOS_CC', os.path.join(R, 'deps/llvm-mos/bin/mos-c64-clang'))
C64_FLAGS = ['-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
FLAGS = ['-std=c99', '-O2', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests']


def logical(phys):
    bank, off = divmod(phys, 1024)
    if bank in (2, 3):
        return 0o4000 + (bank - 2) * 1024 + off
    return 0o2000 + off


def cadr(phys):
    bank, off = divmod(phys, 1024)
    return (bank << 10) | off


def dp(x):
    """Double-precision words of x (|x| < 1)."""
    neg = x < 0
    v = int(round(abs(x) * (1 << 28)))
    v = min(v, (1 << 28) - 1)
    hi, lo = v >> 14, v & 0o37777
    if neg:
        hi, lo = (~hi) & 0o77777, (~lo) & 0o77777
    return hi, lo


class Mission:
    def __init__(self, spec, m):
        self.m = m
        self.cp = IC.Compiler(spec, m)
        self.cp.decode()
        L = self.cp.L
        self.L = L
        self.newops = self.cp.newops
        self.points = sorted(self.cp.blocks)
        self.kinds = {p: self.cp.starts[p][0] for p in self.points}
        self.externals = sorted(set(self.cp.externals))
        if self.cp.basic:
            # Whole-file basic code (tools/basic_compile.py): any transfer
            # from the file to a fixed address outside it but INTPRET stops.
            self.externals += [0xffff, 2048 + self.cp.intpret - 0o4000]
            # Counters yaAGC's device models empty (spec 'device_cells'):
            # only words the file's code never names.
            dev = spec.get('device_cells', [])
            named = self.cp.basic.erasable_refs
            if any(a in named for a in dev):
                raise SystemExit('device cells %s are named by the file' % dev)
            self.externals += list(dev)
        self.body_exits = sorted(set([L.labels[n] for n in BODY_EXITS[m]] + list(self.cp.externals_any)))
        self.returns = self.caller_returns(spec)
        self.operands = self.operand_addresses()
        self.ebank = spec.get('ebank')
        if self.ebank == 'listing':
            # Each routine runs under the EBANK= the listing has in effect at
            # its first word (files that switch EBANK= between routines).
            self.ebank = self.listing_ebanks()
        self.states = {self.point_of(k): v for k, v in spec.get('states', {}).items()}
        # Erasable cells every case of the milestone holds in range (physical
        # address: choices), as the rest of the program keeps them.
        self.cells = spec.get('cells', {})
        self.counters = spec.get('counters', False)
        self.io_channels = spec.get('io_channels', {})
        if self.io_channels == 'auto':
            # Every channel the file's channel orders name, any value, but
            # channel 14's IMU CDU and gyro drive bits (yaAGC's device
            # models act on them at once; tests/interp_case.h).
            self.io_channels = {k: (0o6777 if k == 0o14 else 0o77777) for k in self.cp.basic.channels_used}

    def point_of(self, k):
        """A states key: a label (or label+n), or 'bank,addr' for an
        unlabelled point."""
        if ',' in k:
            b, a = k.split(',')
            return int(b, 8) * 1024 + int(a, 8) - 0o2000
        return self.cp.label_phys(k)

    def listing_ebanks(self):
        eb, out = None, {}
        pat = re.compile(r'^\d{6},\d{6}: (\d\d),(\d{4})  (?:E(\d),\d{4})?\s+(\S+)?')
        for raw in open(IL.LISTINGS[self.m], errors='replace'):
            m = pat.match(raw)
            if not m:
                continue
            if m.group(3) is not None and 'EBANK=' in raw:
                eb = int(m.group(3))
            phys = int(m.group(1), 8) * 1024 + int(m.group(2), 8) - 0o2000
            if phys in self.points and eb is not None:
                out.setdefault(phys, eb)
        return out

    def caller_returns(self, spec):
        """Fixed CADRs of the words after CALL/STCALL operands elsewhere
        that name one of the file's entries."""
        ents = set(cadr(self.cp.label_phys(e)) for e in IC.entries(spec, self.m) if e != '@external')
        if '@external' in IC.entries(spec, self.m):
            ents |= set(cadr(t) for t, why in self.cp.external_entries())
        out = []
        lines = self.L.lines
        for i, ln in enumerate(lines):
            if self.cp.in_file(ln.phys) or not ln.words:
                continue
            if ln.op == '' and ln.words[0] in ents and i > 0:
                prev = lines[i - 1]
                if prev.op in ('CALL', 'STCALL') or prev.operand == 'CALL' or prev.op.startswith('STCALL'):
                    # (Returns into banks 30-43 would need the superbank.)
                    if not self.cp.in_file(ln.phys + 1) and ln.phys // 1024 < 0o30:
                        out.append(cadr(ln.phys + 1))
        return sorted(set(out))

    def operand_addresses(self):
        """Interpretive erasable addresses the file's direct operands name."""
        out = set()
        for p, ln in self.cp.code.items():
            if ln.op == '' and ln.words and ln.words[0] < 0o40000:
                a = ln.words[0] - 1
                if 0 <= a < 0o4000:
                    out.add(a)
        return sorted(out)

    def outside(self, rng):
        if self.returns and rng.random() < 0.5:
            return rng.choice(self.returns)
        while True:
            bank = rng.randrange(2, 0o30)
            off = rng.randrange(0, 0o1776)
            phys = bank * 1024 + off
            if not self.cp.in_file(phys):
                return cadr(phys)


class Absent:
    """A mission the spec does not compile: an empty header."""
    def __init__(self, m):
        self.m = m
        self.newops = 0
        self.externals = []
        self.body_exits = []
        self.points = []

        class C:
            lo = hi = 0
            ranges = []
        self.cp = C()


def case(mi, point, n, rng):
    """(seed, edits, ie) for case n of routine point."""
    E = collections.OrderedDict()
    seed = rng.getrandbits(32)
    bank = point // 1024
    if bank in (2, 3):
        # Fixed-fixed code runs under any FB.
        bank = rng.randrange(4, 0o30)
    fb = bank << 10
    if bank >= 0o40:
        # Banks 40-43: FB 30-33 with SUPERBNK 4 (a channel 7 edit).
        fb = (bank - 0o10) << 10
    eb = rng.randrange(8) if mi.ebank is None else mi.ebank.get(point, rng.randrange(8)) \
        if isinstance(mi.ebank, dict) else mi.ebank
    if mi.counters:
        # 010-061 from the seed too (tests/interp_case.h IRC_COUNTERS): any
        # TIME1-TIME6, CDU, PIPA and save-register values, not only zeros.
        E[0x4000] = 0
    E[3] = eb << 8
    E[4] = fb
    E[6] = fb | eb
    E[0o67] = 0
    if bank >= 0o40:
        E[0x8000 | 0o7] = 0o100
    vac = 0o401 + 0o54 * rng.randrange(5)
    E[0o120] = vac
    shape = n % 3
    if shape >= 1:
        for a in range(vac, vac + 0o46):
            w = rng.randrange(0o4000)
            E[a] = w if rng.random() < 0.5 else (~w) & 0o77777
    for a in mi.operands:
        phys = vac + a if a < 45 else a
        if shape == 1:
            for k in range(6):
                w = rng.randrange(0o4000)
                E[(phys + k) & 0o3777] = w if rng.random() < 0.5 else (~w) & 0o77777
        elif shape == 2:
            z = rng.uniform(-1, 1)
            t = rng.uniform(0, 2 * math.pi)
            r = math.sqrt(max(0.0, 1 - z * z))
            for k, x in enumerate((r * math.cos(t) / 2, r * math.sin(t) / 2, z / 2)):
                hi, lo = dp(x)
                E[(phys + 2 * k) & 0o3777] = hi
                E[(phys + 2 * k + 1) & 0o3777] = lo
    for off in (0o50, 0o51, 0o52):
        E[vac + off] = mi.outside(rng)
    for w in mi.cp.cadr_erasable:
        E[(vac + w) if w < 45 else w] = mi.outside(rng)
    # The interpreter's own state: PUSHLOC in the VAC area, MODE a mode.
    E[0o166] = vac + rng.randrange(0o42)
    E[0o163] = rng.choice([0o77776, 0, 1])
    # OVFIND clear in most cases, so BOV(B) falls through as often as not.
    if rng.random() < 0.75:
        E[0o121] = 0
    held = set()
    for k, choices in mi.states.get(point, {}).items():
        a = vac + int(k[4:], 8) if k.startswith('vac+') else int(k, 8)
        held.add(a)
        v = rng.choice(choices)
        if isinstance(v, str) and v.startswith('vac+'):
            v = vac + int(v[4:], 8)
        elif isinstance(v, str):
            lab, off = v.split('+')
            v = mi.L.word(mi.L.labels[lab] + int(off))
        E[a] = v
    if mi.cp.basic and mi.kinds[point] == 'b':
        # Compiled basic code: the words it names take zeros, small values
        # and saturating values often enough to reach every CCS exit, BZF and
        # BZMF branch and TS overflow (switched addresses through the case's
        # EBANK).
        for a in sorted(mi.cp.basic.erasable_refs):
            phys = (eb << 8) + (a - 0o1400) if a >= 0o1400 else a
            if phys in E:
                continue    # the interpreter state set above (FIXLOC, ...)
            r = rng.random()
            if r < 0.15:
                E[phys] = 0
            elif r < 0.3:
                E[phys] = 0o77777
            elif r < 0.5:
                k = rng.randrange(1, 4)
                E[phys] = k if rng.random() < 0.5 else 0o77777 - k
            elif r < 0.6:
                k = rng.randrange(0o37770, 0o40000)
                E[phys] = k if rng.random() < 0.5 else 0o77777 - k
    if mi.counters:
        relate(mi, E, rng, eb, vac, held)
    for a, choices in mi.cells.items():
        if isinstance(a, str):
            # 'E<bank>,<addr>': a switched cell, held only in cases whose
            # EBANK is that bank.
            b, x = a[1:].split(',')
            if int(b, 8) != eb:
                continue
            a = (eb << 8) + int(x, 8) - 0o1400
        E[a] = rng.choice(choices)
    # I/O channels the file reads or writes (spec 'io_channels', channel:
    # mask of the bits a case may set): a channel edit each, so the run and
    # the digest cover them (tests/interp_case.h).
    for k, mask in sorted(mi.io_channels.items()):
        E[0x8000 | k] = rng.getrandbits(15) & mask if rng.random() < 0.75 else rng.choice([0, mask])
    if mi.kinds[point] == 'i':
        # At NEWOPS the interpreter takes EDOP (023) as the second order of
        # a pair still to run: 0 there, as every interpretive entry leaves it
        # (cases without 'counters' already have 010-061 at 0).
        if mi.counters:
            E[0o23] = 0
        E[5] = mi.newops
        E[0o164] = logical(point)
        E[0o165] = fb | eb
        E[0o115] = fb & 0o40000
        stop = 0xffff
    else:
        E[5] = logical(point)
        # Q: a return in the caller's bank outside the file, the case's stop
        # (or the returns the spec's states give the point: banked ones in
        # the case's bank).
        fixed_q = mi.states.get(point, {}).get('2')
        if fixed_q:
            E[2] = rng.choice(fixed_q)
            stop = 2048 + E[2] - 0o4000 if E[2] >= 0o4000 else bank * 1024 + E[2] - 0o2000
        else:
            while True:
                q = bank * 1024 + rng.randrange(0o1776)
                if not mi.cp.in_file(q):
                    break
            E[2] = logical(q)
            stop = q
    return seed, list(E.items()), n & 1, stop


# The counters files compare with each other and with stored times and
# angles: TIME2/TIME1, TIME3-TIME6, the CDUs and the CDU commands.
COUNTERS = (0o24, 0o25, 0o26, 0o27, 0o30, 0o31, 0o32, 0o33, 0o34, 0o50, 0o51, 0o52, 0o53, 0o54)


def relate(mi, E, rng, eb, vac, held):
    """Relational families (spec 'counters'): a seeded image almost never
    holds the values flight brings together, so some cases make one word
    (or double word) the file reads equal to another, a few counts from it,
    or a counter just below its wrap: a deadline a few centiseconds from
    TIME2/TIME1, an exactly equal time, a CDU at its commanded angle."""
    def phys(a):
        return (eb << 8) + (a - 0o1400) if a >= 0o1400 else a
    refs = set(COUNTERS)
    if mi.cp.basic:
        refs |= {phys(a) for a in mi.cp.basic.erasable_refs if a < 0o4000}
    refs |= {vac + a if a < 45 else phys(a) for a in mi.operands}
    # Not the entry contract's words: NEWJOB, the interpreter's state
    # (OVFIND, FIXLOC, MODE, LOC, BANKSET, PUSHLOC), the VAC area's return
    # CADRs and the CADR words the file keeps.
    keep = set(held) | {0o67, 0o115, 0o120, 0o121, 0o163, 0o164, 0o165, 0o166, vac + 0o50, vac + 0o51, vac + 0o52}
    keep |= {(vac + w) if w < 45 else w for w in mi.cp.cadr_erasable}
    refs = sorted(a for a in refs if 0o10 <= a < 0o4000 and a not in (0o20, 0o21, 0o22, 0o23)
                  and a not in keep and a - 1 not in keep)
    r = rng.random()
    if r < 0.15:
        # A counter at or just below its wrap (TIME1 into TIME2, a CDU at
        # 0/180 degrees).
        a = rng.choice([c for c in COUNTERS if c not in keep])
        E[a] = rng.choice([0o37777 - rng.randrange(4), 0o40000 + rng.randrange(4), 0, 0o77777])
        return
    if r >= 0.5 or len(refs) < 2:
        return
    for _ in range(rng.randrange(1, 4)):
        a, b = rng.sample(refs, 2)
        if rng.random() < 0.5:
            a = rng.choice([c for c in COUNTERS if c not in keep and c - 1 not in keep]) if rng.random() < 0.5 else a
        d = rng.choice([0, 0, 1, -1, 2, -2, 3, -3, rng.randrange(-0o100, 0o101)])
        double = rng.random() < 0.5 and a + 1 in refs + list(COUNTERS) and b + 1 in refs + list(COUNTERS) \
            and a + 1 not in keep and b + 1 not in keep
        if b not in E:
            E[b] = rng.randrange(0o100000)
        if double:
            if b + 1 not in E:
                E[b + 1] = rng.randrange(0o100000)
            hi, lo = E[b], E[b + 1]
            v = (lo & 0o37777) + d
            E[a] = hi
            E[a + 1] = (lo & 0o40000) | (v & 0o37777)
        else:
            w = E[b]
            # (ones' complement: +d moves a positive word up, a negative
            # one toward zero)
            E[a] = (w + d) & 0o77777 if 0 <= (w & 0o37777) + d < 0o40000 else w


def write_cases(path, missions, cases):
    with open(path, 'wb') as f:
        f.write(struct.pack('<I', 0x32435249))
        for mi in missions:
            f.write(struct.pack('<2H', mi.newops, len(mi.cp.ranges)))
            for a, b in mi.cp.ranges:
                f.write(struct.pack('<2H', a, b))
            for ex in (mi.body_exits, mi.externals):
                f.write(struct.pack('<H', len(ex)))
                f.write(struct.pack('<%dH' % len(ex), *ex))
        f.write(struct.pack('<I', len(cases)))
        for c in cases:
            seed, cm, ie, edits = c['seed'], c['cm'], c['ie'], c['edits']
            f.write(struct.pack('<IBBHH', seed, cm, ie, c.get('stop', 0xffff), len(edits)))
            for a, w in edits:
                f.write(struct.pack('<HH', a, w))


def build(spec_id, spec):
    # The pinned engine with DV always through its hardware model
    # (tools/prepare_hardware_dv.py: the fast path mishandles a
    # sign-extended -0 in L), as the native divide follows. Spec 'engine':
    # 'flight-device' also leaves out the IMU and optics device models
    # (tools/prepare_flight_device_engine.py), for files that drive the CDU
    # output counters themselves.
    if spec.get('engine') == 'flight-device':
        ref = os.path.join(B, 'interp-routine-reference-flight-device')
        hw = os.path.join(B, 'agc_engine_flight_device.o')
        prep = 'tools/prepare_flight_device_engine.py'
    else:
        ref = os.path.join(B, 'interp-routine-reference')
        hw = os.path.join(B, 'agc_engine_hardware_dv.o')
        prep = 'tools/prepare_hardware_dv.py'
    if not os.path.exists(hw):
        subprocess.run([sys.executable, prep], cwd=R, check=True, capture_output=True)
    if not os.path.exists(ref) or os.path.getmtime(ref) < max(os.path.getmtime(os.path.join(R, s)) for s in (
            'tools/interp_routine_reference.c', 'tests/interp_case.c', 'tests/interp_case.h')):
        subprocess.run(['cc', '-std=c99', '-O2', '-Iinclude', '-Itests', '-I' + E,
                        '-DIRC_BUILD_ONLY', 'tools/interp_routine_reference.c', 'tests/interp_case.c', 'src/agc_word.c',
                        'tests/cm_executive_cases.c',
                        hw, os.path.join(E, 'agc_engine_init.o'),
                        os.path.join(E, 'rfopen.o'), '-o', ref], cwd=R, check=True)
    nat = os.path.join(B, '%s-native' % spec_id)
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + COMMON
    subprocess.run(['cc', *FLAGS, *srcs, spec['target'], 'tools/interp_routine_native.c', '-o', nat],
                   cwd=R, check=True)
    return ref, nat


def run(cmd):
    out = subprocess.run(cmd, cwd=R, check=True, capture_output=True, text=True).stdout
    return [l.split() for l in out.splitlines()]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('spec')
    ap.add_argument('--cases', type=int, default=64, help='cases per routine')
    ap.add_argument('--freeze', action='store_true')
    ap.add_argument('--show', type=int, default=3)
    ap.add_argument('--only', default='')
    a = ap.parse_args()
    spec = interp_specs.SPECS[a.spec]
    if IC.main(['', a.spec, '--check']):
        sys.exit('generated sources are stale: run tools/interp_compile.py ' + a.spec)
    missions = [Mission(spec, m) if m in spec['missions'] else Absent(m) for m in ('lm', 'cm')]
    rng = random.Random('interp-routine:' + a.spec)
    cases = []
    for cm, mi in enumerate(missions):
        if mi.m not in spec['missions']:
            continue
        for p in mi.points:
            for n in range(a.cases):
                seed, edits, ie, stop = case(mi, p, n, rng)
                cases.append({'cm': cm, 'point': p, 'seed': seed, 'edits': edits, 'ie': ie, 'stop': stop})
    os.makedirs(B, exist_ok=True)
    path = os.path.join(B, '%s-cases.bin' % a.spec)
    write_cases(path, missions, cases)
    ref, nat = build(a.spec, spec)
    rows_r = run([ref, *ROPES, path])
    rows_n = run([nat, path])
    bad = [i for i, (x, y) in enumerate(zip(rows_r, rows_n)) if x != y]
    stops = collections.defaultdict(collections.Counter)
    for c, r in zip(cases, rows_r):
        mi = missions[c['cm']]
        key = '%s %s %s' % (mi.m.upper(), mi.cp.where(c['point']), mi.cp.name(c['point']))
        z = int(r[2])
        stops[key]['%05o' % z] += 1
    if bad or len(rows_r) != len(rows_n):
        dr = run([ref, *ROPES, path, 'dump'])
        dn = run([nat, path, 'dump'])
        sel = [i for i in bad if not a.only or a.only in "%s %s" % (missions[cases[i]["cm"]].m, missions[cases[i]["cm"]].cp.where(cases[i]["point"]))]
        for i in sel[:a.show]:
            c = cases[i]
            mi = missions[c['cm']]
            print('case %d %s %s: oracle Z %05o kind %s, native Z %05o kind %s' % (
                i, mi.m, mi.cp.where(c['point']), int(rows_r[i][2]), rows_r[i][1], int(rows_n[i][2]), rows_n[i][1]))
            for k, (x, y) in enumerate(zip(dr[i][4:], dn[i][4:])):
                if x != y:
                    print('   %04o: oracle %06o native %06o' % (k, int(x), int(y)))
        by = collections.Counter('%s %s' % (missions[cases[i]['cm']].m, missions[cases[i]['cm']].cp.where(cases[i]['point'])) for i in bad)
        print('mismatches by routine:', dict(by))
        sys.exit('FAIL: %d of %d %s cases differ from the original program' % (len(bad), len(cases), a.spec))
    timeouts = [r for r in rows_r if r[1] != '0']
    if timeouts:
        sys.exit('FAIL: %d oracle timeouts' % len(timeouts))
    print('PASS: %d %s cases equal the original programs (%d routines)' % (
        len(cases), a.spec, sum(len(mi.points) for mi in missions if mi.m in spec['missions'])))
    for k in sorted(stops):
        print('  %s: %s' % (k, ', '.join('%s x%d' % (z, n) for z, n in sorted(stops[k].items()))))
    if a.freeze:
        vis = run([nat, path, 'visits'])
        visits = []
        for c, r in zip(cases, vis):
            idx = {p: k for k, p in enumerate(missions[c['cm']].points)}
            v = 0
            for t in r[4:]:
                if int(t[1:]) in idx:   # (erasable continuations are always kept)
                    v |= 1 << idx[int(t[1:])]
            visits.append(v)
        covers = None
        if spec.get('basic') == 'full':
            covers = edge_covers(spec, nat, path)
        freeze(a.spec, spec, missions, cases, rows_r, stops, visits, covers)


def cover_counts(covers, c64_ids):
    host = functools.reduce(operator.or_, covers, 0)
    c64 = functools.reduce(operator.or_, (covers[i] for i in c64_ids), 0)
    lab = (1 << 8192) - 1
    return {'host_labels': bin(host & lab).count('1'), 'c64_labels': bin(c64 & lab).count('1'),
            'host_edges': bin(host >> 8192).count('1'), 'c64_edges': bin(c64 >> 8192).count('1'),
            'host_edges_not_c64': bin(host & ~c64).count('1'),
            'note': 'compiled basic labels; and edges with comparison outcomes (clang -O0 '
                    '-fsanitize-coverage=trace-pc-guard,trace-cmp over every linked source but the host runner: '
                    'both outcomes of each branch, and equal/less/greater at each comparison) that the '
                    'differential\'s cases reach, and those the C64 cases reach'}


def gcov_branches(spec_id, spec, missions, cases, c64_ids):
    """The certificate reviewer's check: gcc -O0 --coverage builds of the
    host runner over every linked source, one run on all the differential's
    cases and one on the C64 cases; gcov -b's taken branch outcomes of each."""
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + COMMON
    srcs = list(dict.fromkeys(srcs + [spec['target']]))
    res = {}
    for tag, sel in (('host', range(len(cases))), ('c64', sorted(set(c64_ids)))):
        w = os.path.join(B, '%s-gcov-%s' % (spec_id, tag))
        shutil.rmtree(w, ignore_errors=True)
        os.makedirs(w)
        write_cases(os.path.join(w, 'cases.bin'), missions, [cases[i] for i in sel])
        objs = []
        for s in srcs:
            o = os.path.join(w, os.path.basename(s)[:-2] + '.o')
            subprocess.run(['gcc', '-std=c99', '-O0', '--coverage', '-Iinclude', '-Itests', '-c', s, '-o', o],
                           cwd=R, check=True)
            objs.append(o)
        subprocess.run(['gcc', '-O0', '--coverage', '-Iinclude', '-Itests', *objs, 'tools/interp_routine_native.c',
                        '-o', os.path.join(w, 'run')], cwd=R, check=True)
        subprocess.run([os.path.join(w, 'run'), os.path.join(w, 'cases.bin')], cwd=R, check=True,
                       stdout=subprocess.DEVNULL)
        taken = set()
        for o in objs:
            out = subprocess.run(['gcov', '-b', '-c', '-t', '-o', w, o[:-2] + '.gcda'], cwd=R, check=True,
                                 capture_output=True, text=True).stdout
            src = line = None
            for l in out.splitlines():
                m = re.match(r'\s*-:\s*0:Source:(.*)', l)
                if m:
                    src = m.group(1)
                    continue
                m = re.match(r'\s*[^:]+:\s*(\d+):', l)
                if m:
                    line = int(m.group(1))
                    continue
                m = re.match(r'branch\s+(\d+) taken (\d+)', l)
                if m and int(m.group(2)):
                    taken.add((src, line, int(m.group(1))))
        res[tag] = taken
        shutil.rmtree(w, ignore_errors=True)
    miss = sorted(res['host'] - res['c64'])
    return {'host_taken': len(res['host']), 'c64_taken': len(res['c64']), 'host_taken_not_c64': len(miss),
            'missing': ['%s:%d branch %d' % x for x in miss[:50]],
            'sources': srcs,
            'note': 'gcov -b -c over gcc -O0 --coverage builds of tools/interp_routine_native.c with every source '
                    'above: the branch outcomes all the differential cases take, and those the C64 cases take'}


def edge_covers(spec, nat, path):
    """Per case, the edges it takes in every linked source of the routines
    (clang -O0 edge coverage: both outcomes of each branch, each switch
    case), the outcomes (equal, less, greater) of each comparison they make
    (so conditions that only yield a value count too) and the compiled
    basic labels it reaches, as one bitmask; the C64 cases take every one
    the host cases take."""
    cov = nat + '-cover'
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + COMMON
    srcs = list(dict.fromkeys(srcs + [spec['target']]))
    od = cov + '.o.d'
    os.makedirs(os.path.join(R, od), exist_ok=True)
    flags = [f for f in FLAGS if f != '-O2'] + ['-O0', '-DIRC_COVER_ON', '-DIRC_EDGES']
    objs = []
    for k, s in enumerate(srcs):
        o = '%s/%d.o' % (od, k)
        subprocess.run(['clang', *flags, '-fsanitize-coverage=trace-pc-guard,trace-cmp', '-c', s, '-o', o],
                       cwd=R, check=True)
        objs.append(o)
    subprocess.run(['clang', *flags, *objs, 'tools/irc_edge_cover.c', 'tools/interp_routine_native.c', '-o', cov],
                   cwd=R, check=True)
    out, dense = [], {}
    for r in run([cov, path, 'cover']):
        m = 0
        for tok in r[4:]:
            if tok[0] == 'g':
                m |= 1 << (8192 + dense.setdefault(int(tok[1:]), len(dense)))
            else:
                m |= 1 << int(tok[1:])
        out.append(m)
    return out


def freeze(spec_id, spec, missions, cases, rows, stops, visits, covers=None):
    stride = spec.get('freeze_stride', 16)
    pick = list(range(0, len(cases), stride))
    sym = spec_id.replace('-', '_')
    out = ['/* SPDX-License-Identifier: MIT; generated by tools/interp_routine_differential.py',
           ' * --freeze: every %dth %s case with its yaAGC oracle digest (original' % (stride, spec_id),
           ' * Luminary099 and Comanche055). */',
           '#include "interp_case.h"']
    for j, i in enumerate(pick):
        c = cases[i]
        out.append('static const irc_edit %s_e%d[%d]={%s};' % (sym, j, len(c['edits']),
                   ','.join('{0%o,0%o}' % (a, w) for a, w in c['edits'])))
    out.append('#define %s_VECTORS %du' % (sym.upper(), len(pick)))
    out.append('static const irc_case %s_cases[%d]={' % (sym, len(pick)))
    out.append(',\n'.join(' {0x%08xul,%d,%d,0,%d,%s_e%d}' % (cases[i]['seed'], cases[i]['cm'], cases[i]['ie'],
                                                            len(cases[i]['edits']), sym, j) for j, i in enumerate(pick)))
    out.append('};')
    out.append('static const uint32_t %s_expected[%d]={' % (sym, len(pick)))
    out.append(',\n'.join(' 0x%08xul' % int(rows[i][3]) for i in pick))
    out.append('};')
    open(os.path.join(R, 'tests/%s_vectors.h' % sym), 'w').write('\n'.join(out) + '\n')
    c64 = freeze_c64(spec, sym, missions, cases, rows, visits, covers)
    blob = open(os.path.join(B, '%s-cases.bin' % spec_id), 'rb').read()
    proof = {'milestone': spec_id, 'missions': ['Luminary099', 'Comanche055'],
             'rope_sha256': [hashlib.sha256(open(p, 'rb').read()).hexdigest() for p in ROPES],
             'cases': len(cases), 'cases_sha256': hashlib.sha256(blob).hexdigest(),
             'oracle_sha256': hashlib.sha256('\n'.join(' '.join(r) for r in rows).encode()).hexdigest(),
             'frozen_vectors': len(pick), 'c64_vectors': c64,
             **({'c64_coverage': cover_counts(covers, [i for g in c64 for i in g['case_ids']]),
                 'c64_gcov_branches': gcov_branches(spec_id, spec, missions, cases,
                                                    [i for g in c64 for i in g['case_ids']])}
                if covers else {}),
             'stops': {k: dict(v) for k, v in sorted(stops.items())},
             'boundary': 'from each routine entry state to NEWOPS with LOC outside the file, an exit of the '
                         'interpreter bodies (%s), or a transfer to an outside basic routine (%s)%s' % (
                             '; '.join('%s: %s' % (mi.m, ', '.join(BODY_EXITS[mi.m])) for mi in missions),
                             '; '.join('%s: %s' % (mi.m, ', '.join(sorted(set(mi.cp.externals.values()) | set(mi.cp.externals_any.values()))) or 'none')
                                       for mi in missions if mi.m in spec['missions']),
                             '; whole-file basic code stops at any transfer from the file to a fixed address '
                             'outside it but INTPRET (stored words the file transfers to in erasable run on)'
                             if any(getattr(mi.cp, 'basic', None) for mi in missions) else ''),
             'compared': 'Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state' + (
                 ', the I/O channels %s and the 31A/31B/32 trap flags' % ', '.join(
                     '%02o' % k for k in sorted(set().union(*(getattr(mi, 'io_channels', {}) for mi in missions))))
                 if any(getattr(mi, 'io_channels', {}) for mi in missions) else ''),
             **({'engine': 'pinned yaAGC%s, its ScalerCounter held at 0 (TIME1-TIME6 and the timer '
                           'interrupts do not advance during a case)' % (
                               ' with hardware DV and without the IMU/optics device block '
                               '(tools/prepare_flight_device_engine.py)'
                               if spec.get('engine') == 'flight-device' else '')}
                if any(getattr(mi.cp, 'basic', None) for mi in missions) else {}),
             **({'start_state': 'every word of the image from the case seed, 010-061 included (TIME1-TIME6, the '
                                'CDUs, PIPAs and other counters, the interrupt save words, the editing registers; '
                                'IRC_COUNTERS in tests/interp_case.h), then the case edits; half the cases also tie words '
                                'the file reads together (relate(): one equal to another, a few counts or a double '
                                'word apart, TIME2/TIME1, TIME3-TIME6, the CDUs and CDU commands included) or put a '
                                'counter at its wrap'}
                if spec.get('counters') else {})}
    open(os.path.join(B, '%s-proof.json' % spec_id), 'w').write(json.dumps(proof, indent=1) + '\n')
    smap = {}
    for mi in missions:
        if mi.m not in spec['missions']:
            continue
        rows_m = mi.cp.source_map()
        loose = [r for r in rows_m if 'routine' not in r and 'constant' not in r]
        if loose and mi.m not in spec.get('partial', {}):
            sys.exit('%s words no routine compiles: %s' % (mi.m, ', '.join(r['address'] for r in loose)))
        for r in loose:
            r['untranslated'] = spec['partial'][mi.m]
        smap[spec['file'][mi.m] and ('Luminary099/' if mi.m == 'lm' else 'Comanche055/') + spec['file'][mi.m]] = {
            'native_file': 'src/%s.c' % spec['base'][mi.m], 'words': len(rows_m), 'untranslated_words': len(loose),
            'routines': len(mi.points), 'map': rows_m}
    open(os.path.join(B, '%s-source-map.json' % spec_id), 'w').write(json.dumps(smap, indent=1) + '\n')
    print('froze %d vectors to tests/%s_vectors.h' % (len(pick), sym))




# The program, its data and bss end below the $c000 mailbox (the soft stack
# grows down from $d000), with room to spare.
C64_TOP = 0xbf00


def c64_sources(spec, m):
    return (['src/%s.c' % spec['base'][m], 'src/%s_interp_orders.c' % m, 'src/%s_interp_steps.c' % m]
            + spec.get('native_sources', []) + [x for x in COMMON if '_interp_' not in x] + [spec['target']])


def keep_defs(name, mask, n):
    """-D flags for a group's KEEP mask: KEEP<k> holds points 60k..60k+59
    (a file with more than 60 routines needs every word defined)."""
    return ['-D%s_KEEP%d=0x%x' % (name, k, (mask >> (60 * k)) & ((1 << 60) - 1) if n > 60 else mask)
            for k in range(max(1, (n + 59) // 60))]


POINTS = {}     # mission -> routine count (set by freeze_c64)


def c64_size(spec, sym, m, mask, text):
    """End of the C64 diagnostic's memory (code, data and bss) for one group,
    or None when it does not link or reaches past C64_TOP."""
    d = os.path.join(B, '%s-probe-%s-%x' % (sym, m, mask))
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, '%s_c64_vectors.h' % sym), 'w').write(text)
    prg = os.path.join(d, 'probe.prg')
    r = subprocess.run([MOS_CC, '-I' + d, '-Iinclude', '-Itests', *C64_FLAGS, *spec.get('c64_flags', []), '-DIRC_GROUP=1',
                        '-DIRC_%s_ONLY' % m.upper(), *keep_defs(spec['symbol'][m].upper(), mask, POINTS.get(m, 0)),
                        *c64_sources(spec, m), 'platform/c64/%s_diagnostic.c' % sym,
                        '-Wl,-Map,' + os.path.join(d, 'probe.map'), '-o', prg],
                       cwd=R, capture_output=True, text=True)
    end = None
    if r.returncode == 0:
        for ln in open(os.path.join(d, 'probe.map')):
            if '__heap_start = ' in ln:
                end = int(ln.split()[0], 16)
    if os.environ.get('IRC_DEBUG'):
        print('probe %s %x: rc=%d end=%s %s' % (m, mask, r.returncode, end and hex(end), r.stderr[-1500:]), file=sys.stderr)
    subprocess.run(['rm', '-rf', d])
    return end if end is not None and end <= C64_TOP else None


def group_text(sym, groups, cases, rows):
    """The vectors header: per group, its cases and their oracle digests."""
    out = ['/* SPDX-License-Identifier: MIT; generated by tools/interp_routine_differential.py',
           ' * --freeze: the C64 diagnostics\' cases. Group g (IRC_GROUP) keeps the routines',
           ' * of its KEEP mask (tools/mk/%s.groups); its cases are those whose whole' % sym.replace('_', '-'),
           ' * run stays within them, with their yaAGC oracle digests. */',
           '#include "interp_case.h"']
    for g, (m, mask, pick) in enumerate(groups, 1):
        pre = '%s_g%d' % (sym, g)
        out.append('#if IRC_GROUP == %d' % g)
        for j, i in enumerate(pick):
            out.append('static const irc_edit %s_e%d[%d]={%s};' % (pre, j, len(cases[i]['edits']),
                       ','.join('{0%o,0%o}' % (a, w) for a, w in cases[i]['edits'])))
        out.append('#define IRC_C64_VECTORS %du' % len(pick))
        out.append('static const irc_case irc_c64_cases[%d]={' % len(pick))
        out.append(',\n'.join(' {0x%08xul,%d,%d,0,%d,%s_e%d}' % (cases[i]['seed'], cases[i]['cm'], cases[i]['ie'],
                                                                len(cases[i]['edits']), pre, j) for j, i in enumerate(pick)))
        out.append('};')
        out.append('static const uint32_t irc_c64_expected[%d]={' % len(pick))
        out.append(',\n'.join(' 0x%08xul' % int(rows[i][3]) for i in pick))
        out.append('};')
        out.append('#endif')
    return '\n'.join(out) + '\n'


def freeze_c64(spec, sym, missions, cases, rows, visits, covers=None):
    """Groups of routines whose diagnostic fits below $c000, each with the
    spec's c64_per_point cases (default 2) per routine whose run stays in
    the group (fewest edits first). Writes tests/<id>_c64_vectors.h and
    tools/mk/<id>.groups; returns the groups."""
    per = spec.get('c64_per_point', 2)
    groups = []
    for cm, mi in enumerate(missions):
        if mi.m not in spec['missions']:
            continue
        bit = {p: 1 << k for k, p in enumerate(mi.points)}
        POINTS[mi.m] = len(mi.points)
        need = {}
        got = 0
        for p in mi.points:
            mine = sorted((i for i, c in enumerate(cases) if c['cm'] == cm and c['point'] == p),
                          key=lambda i: (bin(visits[i]).count('1'), len(cases[i]['edits']), i))
            if covers is None:
                need[p] = mine[:per]
                continue
            # Greedy: the case adding the most labels and edges no case
            # taken so far (this routine's or an earlier one's) reaches,
            # until the routine's cases add none (at least one case).
            pick_p = []
            while True:
                best = max(mine, key=lambda i: bin(covers[i] & ~got).count('1'), default=None)
                if best is None or (pick_p and not covers[best] & ~got):
                    break
                pick_p.append(best)
                got |= covers[best]
                mine = [i for i in mine if i != best]
            need[p] = pick_p
        # Pack the routines' cases into groups in routine order, a routine's
        # cases together while they fit, else case by case.
        mask, pick = 0, []
        for p in mi.points:
            units = [need[p]]
            want = 0
            for i in need[p]:
                want |= visits[i]
            if len(need[p]) > 1 and c64_size(spec, sym, mi.m, want,
                                             group_text(sym, [(mi.m, want, need[p])], cases, rows)) is None:
                units = [[i] for i in need[p]]
            for u in units:
                want = 0
                for i in u:
                    want |= visits[i]
                trial = mask | want
                tp = pick + u
                if mask and c64_size(spec, sym, mi.m, trial, group_text(sym, [(mi.m, trial, tp)], cases, rows)) is None:
                    groups.append((mi.m, mask, pick))
                    mask, pick = want, list(u)
                else:
                    mask, pick = trial, tp
        groups.append((mi.m, mask, pick))
    for m, mask, pick in groups:
        # Every case of a group whose run stays in it, so later routines'
        # closures don't drop earlier cases.
        assert all(visits[i] & ~mask == 0 for i in pick)
        if c64_size(spec, sym, m, mask, group_text(sym, [(m, mask, pick)], cases, rows)) is None:
            sys.exit('a single routine group of %s does not fit the C64' % m)
    open(os.path.join(R, 'tests/%s_c64_vectors.h' % sym), 'w').write(group_text(sym, groups, cases, rows))
    mk = ['# Generated by tools/interp_routine_differential.py --freeze: the C64',
          '# diagnostic groups (mission, KEEP mask) for tools/mk/%s.mk.' % sym.replace('_', '-')]
    v = sym.upper()
    mk.append('%s_GROUPS = %s' % (v, ' '.join(str(g) for g in range(1, len(groups) + 1))))
    for g, (m, mask, pick) in enumerate(groups, 1):
        mk.append('%s_GROUP_MISSION_%d = %s' % (v, g, m))
        mk.append('%s_GROUP_FLAGS_%d = -DIRC_GROUP=%d -DIRC_%s_ONLY %s' % (
            v, g, g, m.upper(), ' '.join(keep_defs(spec['symbol'][m].upper(), mask, POINTS.get(m, 0)))))
    open(os.path.join(R, 'tools/mk/%s.groups' % sym.replace('_', '-')), 'w').write('\n'.join(mk) + '\n')
    return [{'mission': m, 'keep': '0x%x' % mask, 'cases': len(pick), 'case_ids': list(pick)} for m, mask, pick in groups]


if __name__ == '__main__':
    main()
