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
file's operands, and half-unit vectors at the file's operands. A spec with
'edge_cases': True adds a fourth, tools/edge_cases.py's flight edge values.

  interp_routine_differential.py <spec> [--cases N] [--freeze] [--show K]

--freeze writes tests/<id>_vectors.h (a subset of cases with their oracle
digests) for the host test and the C64 diagnostic, and
build/<id>-proof.json. A spec with 'c64_rows': 'cover' picks the C64 cases
by branch outcome (tools/c64_row_cover.py) instead of per routine."""
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
import types

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import c64_row_cover
import edge_cases
import interp_compile as IC
import interp_listing as IL
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
# 'c64_cuts' specs also link the truncated-case runner (tests/interp_cut.h).
CUT = ['tests/interp_cut.c']


def common(spec):
    return COMMON + (CUT if spec.get('c64_cuts') else [])


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
        # (An erasable program's address a call reaches is 0xC000 | Z.)
        self.body_exits = sorted(set([L.labels[n] for n in BODY_EXITS[m]] + list(self.cp.externals_any)
                                     + [0xC000 | k for k in self.cp.erasable_calls]))
        self.returns = self.caller_returns(spec)
        self.operands = self.operand_addresses()
        self.ebank = spec.get('ebank')
        if self.ebank == 'listing':
            # Each routine runs under the EBANK= the listing has in effect at
            # its first word (files that switch EBANK= between routines).
            self.ebank = self.listing_ebanks()
        self.edge = spec.get('edge_cases', False)
        self.rk = bool(spec.get('resume_keys'))
        self.index_returns = bool(spec.get('index_returns'))
        self.all_states = spec.get('all_states', {})
        if self.all_states and set(self.all_states) <= {'lm', 'cm'}:
            self.all_states = self.all_states.get(m, {})
        self.counters = spec.get('counters', False)
        # (Lane G) 010-061 from the case seed (IRC_COUNTERS) plus relate().
        self.seed_counters = spec.get('seed_counters', False)
        self.edges = spec.get('edge_values', False)
        js = spec.get('joint_states', {})
        self.joint_any = js.get('*', [])
        if js and set(js) <= {'lm', 'cm'}:
            js = js.get(m, {})
            self.joint_any = js.get('*', [])
        self.joint = {self.cp.label_phys(k): v for k, v in js.items() if k != '*'}
        self.esyms = erasable_symbols(os.path.join(R, IL.LISTINGS[m]))
        # 'states' may name each mission's own ({'lm': {...}, 'cm': {...}}).
        st = spec.get('states', {})
        if st and set(st) <= {'lm', 'cm'}:
            st = st.get(m, {})
        self.states = {self.point_of(k): v for k, v in st.items()}
        if spec.get('index_states'):
            for p, v in self.index_states(spec).items():
                self.states[p] = dict(v, **self.states.get(p, {}))
        # range_states: (FROM, TO, states) applies states to every point
        # from label FROM up to (not including) label TO, after all_states.
        self.range_states = [(self.cp.label_phys(a), self.cp.label_phys(b), st)
                             for a, b, st in spec.get('range_states', ())]
        self.return_words = [int(k, 8) for k in spec.get('return_words', [])]
        self.flow = {}   # ('flow_states') point -> states its entries' runs reach it in
        # Erasable cells every case of the milestone holds in range (physical
        # address: choices), as the rest of the program keeps them.
        self.cells = spec.get('cells', {})
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

    def index_states(self, spec):
        """'index_states': X1 and X2 at each point where the next indexed
        operand reading them (in listing order, before an instruction sets
        them) names erasable: X = base - target, for targets across the
        switched erasable bank (with room for a matrix past them), so the
        operand lands in erasable as the program's AXC/AXT settings do.
        Explicit 'states' for the point win."""
        IX = re.compile(r'^(AXT|AXC|LXA|LXC|SXA|XCHX|TIX|INCR|XAD|XSU),([12])$')
        lines = sorted(self.L.file_lines(spec['file'][self.m]), key=lambda ln: ln.phys)
        targets = list(range(0o1400, 0o1740, 0o24))
        enc = lambda x: x & 0o77777 if x >= 0 else ~(-x) & 0o77777

        def base(text):
            m = re.match(r'^([^\s,+-]*)\s*([+-]\s*\d+D?)?$', text.strip())
            if not m:
                return None
            name, off = m.group(1), (m.group(2) or '').replace(' ', '')
            n = (int(off[:-1]) if off.endswith('D') else int(off, 8)) if off else 0
            if re.fullmatch(r'\d+D', name):
                return int(name[:-1]) + n
            if re.fullmatch(r'[0-7]+', name):
                return int(name, 8) + n
            if name in self.esyms:
                ph = self.esyms[name]
                return (ph if ph < 0o1400 else 0o1400 + ph % 256) + n
            return None     # fixed memory (the spec's indexed_fixed and fixed_index)

        events = []     # (phys, 'set'|'use', index, base)
        for ln in lines:
            toks = [t for t in (ln.op, ln.operand) if t]
            ops = [t for t in toks if IX.match(t.replace(' ', ''))]
            for t in ops:
                op, x = IX.match(t.replace(' ', '')).groups()
                if op not in ('SXA', 'TIX', 'INCR'):
                    events.append((ln.phys, 'set', x, None))
            text = ln.operand or ''
            m = re.match(r'^(.*),([12])$', text.replace(' ', '')) if not ops else None
            if m and not ln.op:
                events.append((ln.phys, 'use', m.group(2), base(ln.operand.rsplit(',', 1)[0])))
            elif m and ln.op:
                events.append((ln.phys, 'use', m.group(2), base(ln.operand.rsplit(',', 1)[0])))
        # A CALL or GOTO (either order of the word) to a label of the file
        # continues there: its operand is the first label among the next
        # operand words.
        jumps = {}
        for i, ln in enumerate(lines):
            if 'CALL' in (ln.op, ln.operand) or 'GOTO' in (ln.op, ln.operand):
                for nxt in lines[i + 1:i + 3]:
                    if nxt.op:
                        break
                    if nxt.operand in self.L.labels:
                        jumps[ln.phys] = self.L.labels[nxt.operand]
                        break
        out = {}
        for p in self.points:
            for x, key in (('1', 'vac+046'), ('2', 'vac+047')):
                q0, seen = p, 0
                while seen < 3:
                    ev = [e for e in events if q0 <= e[0] <= q0 + 0o60 and e[2] == x]
                    jp = [q for q in jumps if q0 <= q <= q0 + 0o60 and (not ev or q < ev[0][0])]
                    if jp:
                        q0, seen = jumps[min(jp)], seen + 1
                        continue
                    if ev and ev[0][1] == 'use' and ev[0][3] is not None:
                        out.setdefault(p, {})[key] = [enc(ev[0][3] - t) for t in targets]
                    break
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
                # 055-077 are counters and NEWJOB, never data operands: a
                # word there is a switch bit (CLEAR MOONFLAG, 063).
                if 0 <= a < 0o4000 and not 0o55 <= a < 0o100:
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


def erasable_symbols(path):
    """Erasable symbol -> physical address, from the listing's symbol table
    ('TM  E6,1733' is E6 1733, physical 3333; 'WAITTEMP  0064' is 0064)."""
    out = {}
    for m in re.finditer(r'\d+,E:\s+(\S+)\s+(?:E(\d),(\d{4})|(\d{4}))', open(path, errors='replace').read()):
        out[m.group(1)] = (int(m.group(2)) * 256 + int(m.group(3), 8) - 0o1400) if m.group(2) else int(m.group(4), 8)
    return out


def state_address(mi, k, vac):
    """'vac+n', an octal address, or an erasable symbol with an optional +n."""
    if k.startswith('vac+'):
        return vac + int(k[4:], 8)
    if re.fullmatch(r'[0-7]+', k):
        return int(k, 8)
    name, _, off = k.partition('+')
    return mi.esyms[name] + (int(off) if off else 0)


def counter_states(E, rng):
    """TIME2/TIME1 (024/025) and CDUX-Z (032-034), which irc_build zeroes:
    TIME1 across its range, at 0 and just below its 037777 wrap."""
    E[0o24] = rng.randrange(0o40000)
    r = rng.random()
    E[0o25] = (rng.randrange(0o40000) if r < 0.6 else 0o37777 - rng.randrange(8) if r < 0.8
               else rng.randrange(8))
    for a in (0o32, 0o33, 0o34):
        E[a] = rng.getrandbits(15)


def counter_edges(E, rng):
    """CDUs at 0, 45, 90, 180, 270 degrees and -0, for the edge shape."""
    for a in (0o32, 0o33, 0o34):
        if rng.random() < 0.7:
            E[a] = rng.choice([0, 0o10000, 0o20000, 0o40000, 0o60000, 0o77777])


# Edge values for the 'edge_values' shape: +-0, one LSB, 1/2 (a unit
# component, and ASIN/ACOS's exact .5), 1/4 and just below 1, as (hi, lo).
EDGE_SCALARS = [(0, 0), (0o77777, 0o77777), (0, 1), (0o77777, 0o77776), (0o20000, 0),
                (0o57777, 0o77777), (0o10000, 0), (0o67777, 0o77777), (0o37777, 0o37777),
                (0o40000, 0o40000), (0, 0o37777), (0o77777, 0o40000)]
ZEROS = [(0, 0), (0o77777, 0o77777)]


def edge_vector(rng, row):
    """Three (hi, lo) components of an edge-case vector: zero (either
    sign), a half-unit axis, a unit in a coordinate plane, a tiny vector,
    or palette components. 'row' is the axis an axis-set case gives this
    address, so a matrix's three rows form a signed permutation."""
    kind = rng.randrange(6)
    if kind == 0:
        return [rng.choice(ZEROS) for _ in range(3)]
    if kind == 1 or (kind == 5 and row is None):
        ax = rng.randrange(3)
    elif kind == 5:
        ax = row
    if kind in (1, 5):
        v = [rng.choice(ZEROS) for _ in range(3)]
        v[ax] = rng.choice([(0o20000, 0), (0o57777, 0o77777)])
        return v
    if kind == 2:
        z = rng.randrange(3)
        t = rng.uniform(0, 2 * math.pi)
        c = [dp(math.cos(t) / 2), dp(math.sin(t) / 2)]
        v = [None] * 3
        v[z] = rng.choice(ZEROS)
        for i in range(3):
            if v[i] is None:
                v[i] = c.pop()
        return v
    if kind == 3:
        return [rng.choice([(0, 0), (0, 1), (0o77777, 0o77776), (0o77777, 0o77777), (0, 3)]) for _ in range(3)]
    return [rng.choice(EDGE_SCALARS) for _ in range(3)]


def case(mi, point, n, rng):
    """(seed, edits, ie) for case n of routine point."""
    E = collections.OrderedDict()
    seed = rng.getrandbits(32)
    bank = point // 1024
    if bank in (2, 3):
        # Fixed-fixed code runs under any FB.
        bank = rng.randrange(4, 0o30)
    # Banks 040-043 run as FB 030-033 under the superbank (channel 7 0100).
    sb = 0o100 if bank >= 0o40 else 0
    fb = (bank - 8 if bank >= 0o40 else bank) << 10
    eb = rng.randrange(8) if mi.ebank is None else mi.ebank.get(point, rng.randrange(8)) \
        if isinstance(mi.ebank, dict) else mi.ebank
    if mi.seed_counters:
        # 010-061 from the seed too (tests/interp_case.h IRC_COUNTERS): any
        # TIME1-TIME6, CDU, PIPA and save-register values, not only zeros.
        E[0x4000] = 0
    E[3] = eb << 8
    E[4] = fb
    E[6] = fb | eb
    # NEWJOB +0, or (resume_keys) positive in a quarter of the cases: a job
    # waiting, so the next DANZIG goes to CHANG2.
    E[0o67] = newjob = rng.randrange(1, 0o40000) if mi.rk and rng.random() < 0.25 else 0
    vac = 0o401 + 0o54 * rng.randrange(5)
    E[0o120] = vac
    # Shapes: 0 the VAC area and operands as they are, 1 random words,
    # 2 random half-unit vectors, and a fourth with 'edge_values' (edge
    # vectors below) or 'edge_cases' (tools/edge_cases.py).
    shape = n % 4 if (mi.edges or mi.edge) else n % 3
    if shape >= 1:
        for a in range(vac, vac + 0o46):
            w = rng.randrange(0o4000)
            E[a] = w if rng.random() < 0.5 else (~w) & 0o77777
    for a in mi.operands:
        phys = vac + a if a < 45 else a
        if shape == 1:
            for k in range(6):
                w = rng.randrange(0o4000)
                w = w if rng.random() < 0.5 else (~w) & 0o77777
                # (An operand at the top of erasable, as P37's E7 03774,
                # does not run on into A and L.)
                if phys + k <= 0o3777:
                    E[phys + k] = w
        elif shape == 2:
            z = rng.uniform(-1, 1)
            t = rng.uniform(0, 2 * math.pi)
            r = math.sqrt(max(0.0, 1 - z * z))
            for k, x in enumerate((r * math.cos(t) / 2, r * math.sin(t) / 2, z / 2)):
                hi, lo = dp(x)
                for j, v in ((2 * k, hi), (2 * k + 1, lo)):
                    if phys + j <= 0o3777:
                        E[phys + j] = v
    if shape == 3:
        if mi.edges:
            perm = rng.sample(range(3), 3) if rng.random() < 0.5 else None
            # MPAC (0154-0160) and the top of the push-down list as well.
            for a in [0o154, 0] + mi.operands:
                phys = vac + a if a < 45 else a
                row = perm[(phys // 6) % 3] if perm else None
                for k, (hi, lo) in enumerate(edge_vector(rng, row)):
                    E[(phys + 2 * k) & 0o3777] = hi
                    E[(phys + 2 * k + 1) & 0o3777] = lo
        else:
            E.update(edge_cases.edits(n // 4, rng, [vac + a if a < 45 else a for a in mi.operands]))
    E[0o67] = newjob   # (NEWJOB, which an operand's randomizing may have hit)
    for off in (0o50, 0o51, 0o52):
        E[vac + off] = mi.outside(rng)
    for w in mi.cp.cadr_erasable:
        E[(vac + w) if w < 45 else w] = mi.outside(rng)
    # Words the spec names as saved returns (INTWAKE parks QPRET in TBASE2).
    for a in mi.return_words:
        E[a] = mi.outside(rng)
    # The interpreter's own state: PUSHLOC in the VAC area, MODE a mode.
    E[0o166] = vac + rng.randrange(0o42)
    E[0o163] = rng.choice([0o77776, 0, 1])
    # OVFIND clear in most cases, so BOV(B) falls through as often as not.
    if rng.random() < 0.75:
        E[0o121] = 0
    if mi.counters:
        counter_states(E, rng)
        if shape == 3:
            counter_edges(E, rng)
    held = pinned = set()
    ranged = [kv for lo, hi, st in mi.range_states if lo <= point < hi for kv in st.items()]
    for k, choices in (list(mi.all_states.items()) + ranged + list(mi.flow.get(point, {}).items())
                       + list(mi.states.get(point, {}).items())):
        a = state_address(mi, k, vac)
        pinned.add(a)
        if isinstance(choices, dict):
            choices = choices[mi.m]
        v = rng.choice(choices)
        if v == 'outside':
            v = mi.outside(rng)
        elif isinstance(v, str) and v.startswith('vac+'):
            v = vac + int(v[4:], 8)
        elif isinstance(v, str):
            lab, off = v.split('+')
            v = mi.L.word(mi.L.labels[lab] + int(off))
        E[a] = v
    # Joint states: values that only mean something together (TM a little
    # past TIME2/TIME1), one set per case; '*' sets go to half of every
    # routine's cases.
    joint = mi.joint.get(point)
    if joint is None and mi.joint_any and rng.random() < 0.5:
        joint = mi.joint_any
    if joint:
        if isinstance(joint, dict):
            joint = joint[mi.m]
        for k, v in rng.choice(joint).items():
            E[state_address(mi, k, vac)] = v & 0o77777
    if mi.cp.novac(point) and rng.random() < 0.5:
        # 'any_fixloc': a basic routine that never names FIXLOC runs from
        # tasks and NOVAC jobs too, so half its cases leave the VAC areas.
        E[0o120] = rng.randrange(0o4000)
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
    if mi.seed_counters:
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
        # (cases without 'seed_counters' already have 010-061 at 0).
        if mi.seed_counters:
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
                if mi.index_returns:
                    # (Returns up to four words past it, INDEX SPRTEX; TCF 0 +4,
                    # stay in the bank and outside the file.)
                    q = min(q, bank * 1024 + 0o1773)
                    if not any(mi.cp.in_file(q + i) for i in range(5)):
                        break
                elif not mi.cp.in_file(q):
                    break
            E[2] = logical(q)
            stop = q
        # Saved returns a TC to erasable runs (TC MPAC +2, TC TBASE2): the same.
        # A state the spec pins there (a caller's return in the file) stays.
        for k in mi.cp.tc_erasable:
            a = k if k < 0o1400 else eb * 0o400 + k - 0o1400
            if a not in pinned:
                E[a] = E[2]
    return seed, list(E.items()), n & 1, stop, sb


# The counters files compare with each other and with stored times and
# angles: TIME2/TIME1, TIME3-TIME6, the CDUs and the CDU commands.
COUNTERS = (0o24, 0o25, 0o26, 0o27, 0o30, 0o31, 0o32, 0o33, 0o34, 0o50, 0o51, 0o52, 0o53, 0o54)


def relate(mi, E, rng, eb, vac, held):
    """Relational families (spec 'seed_counters'): a seeded image almost never
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


class Absent:
    """A mission the spec leaves out: no ranges, exits or routines, so a
    one-mission spec (cm-r31) writes the same case-file layout."""
    def __init__(self, m):
        self.m = m
        self.newops = 0
        self.cp = types.SimpleNamespace(ranges=[], where=lambda p: '%05o' % p)
        self.points = []
        self.body_exits = []
        self.externals = []


class Edits:
    """A case's (address, word) edits, packed (a whole milestone's cases
    would not fit in memory as tuples)."""
    __slots__ = ('w',)

    def __init__(self, pairs):
        import array
        self.w = array.array('I', [a << 16 | v for a, v in pairs])

    def __len__(self):
        return len(self.w)

    def __iter__(self):
        return ((x >> 16, x & 0xffff) for x in self.w)


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
            # (A cut: flagged in the interrupt-enable byte, after the edit count.)
            f.write(struct.pack('<IBBHHH', seed, cm, ie | (0x80 if c.get('cut') else 0), c.get('sb', 0),
                                c.get('stop', 0xffff), len(edits)))
            if c.get('cut'):
                f.write(struct.pack('<H', c['cut']))
            for a, w in edits:
                f.write(struct.pack('<HH', a, w))


def flow_seed(missions, cases, rng, ref, path):
    """'resume_flow': a CHANG2 resume LOC's cases start from the state the
    original program is in when it gets there. Each case of the routine
    before it runs on the oracle to NEWOPS with LOC at the resume LOC (and
    so on along a run of resume LOCs); the state there, with NEWJOB
    positive in a quarter of the cases, is the case. A case that never gets
    there takes another case's state; a LOC no case reaches keeps random
    states."""
    at = collections.defaultdict(list)
    for i, c in enumerate(cases):
        at[c['cm'], c['point']].append(i)
    pred, level = {}, {}
    for cm, mi in enumerate(missions):
        if mi.m == 'absent' or not mi.points or not getattr(mi, 'rk', False):
            continue
        for k, p in enumerate(mi.points):
            if k and mi.cp.starts[p] == ('i', 'CHANG2 resume'):
                q = mi.points[k - 1]
                pred[cm, p] = (cm, q)
                level[cm, p] = level.get((cm, q), 0) + 1
    sub = path + '.flow'
    flowed = set()
    for lv in range(1, max(level.values(), default=0) + 1):
        batch = [(r, j) for r in sorted(level) if level[r] == lv for j in at[r]]
        runs = []
        for r, j in batch:
            k = at[r].index(j)
            c = dict(cases[at[pred[r]][k]])
            c['stop'] = r[1]
            runs.append(c)
        write_cases(sub, missions, runs)
        out = subprocess.run([ref, *ROPES, sub], cwd=R, check=True, capture_output=True, text=True,
                             env=dict(os.environ, IRC_SNAP='1')).stdout.splitlines()
        snaps = collections.defaultdict(list)
        got = {}
        for (r, j), c, line in zip(batch, runs, out):
            f = line.split()
            if f[1] == 'S':
                E = collections.OrderedDict(c['edits'])
                E.update((int(f[k]), int(f[k + 1])) for k in range(2, len(f), 2))
                got[j] = (c, E)
                snaps[r].append(j)
        for r, j in batch:
            if not snaps[r]:
                continue
            c, E = got[j] if j in got else got[snaps[r][at[r].index(j) % len(snaps[r])]]
            E = collections.OrderedDict(E)
            E[0o67] = rng.randrange(1, 0o40000) if rng.random() < 0.25 else 0
            cases[j] = dict(cases[j], seed=c['seed'], ie=c['ie'], sb=c['sb'], stop=0xffff, edits=list(E.items()))
            flowed.add(r)
    if os.path.exists(sub):
        os.remove(sub)
    print('resume_flow: %d of %d resume LOCs start from the original program\'s state there%s' % (
        len(flowed), len(level), (' (random states: %s)' % ', '.join(
            '%s %s' % (missions[r[0]].m, missions[r[0]].cp.where(r[1])) for r in sorted(set(level) - flowed)))
        if set(level) - flowed else ''))


def cut_flags(spec):
    return ['-DIRC_CUTS'] if spec.get('c64_cuts') else []


def build(spec_id, spec, extra=(), tag=''):
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
    nat = os.path.join(B, '%s-native%s' % (spec_id, tag))
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + common(spec)
    subprocess.run(['cc', *FLAGS, *extra, '-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200), *cut_flags(spec), *srcs, spec['target'],
                    'tools/interp_routine_native.c', '-o', nat],
                   cwd=R, check=True)
    return ref, nat


FLOW_KEYS = ('0166', '0163', 'vac+046', 'vac+047', 'vac+050', 'vac+051')


def flow_states(spec_id, spec, missions, per_entry=64):
    """'flow_states': a resume point mid-sequence starts from the states the
    file's own runs reach it in. The oracle runs cases from every entry
    (IRC_FLOW lists PUSHLOC, MODE, X1, X2, S1 and S2 at each NEWOPS), and
    each other routine's cases draw those registers from the values seen
    there (spec states still win). A routine no entry reaches keeps
    random ones."""
    rng = random.Random('interp-flow:' + spec_id)
    cases = []
    for cm, mi in enumerate(missions):
        if mi.m not in spec['missions']:
            continue
        for p in mi.points:
            if mi.cp.starts[p][1].startswith('entry'):
                for n in range(per_entry):
                    seed, edits, ie, stop, sb = case(mi, p, n, rng)
                    cases.append({'cm': cm, 'point': p, 'seed': seed, 'edits': edits, 'ie': ie, 'stop': stop, 'sb': sb})
    os.makedirs(B, exist_ok=True)
    path = os.path.join(B, '%s-flow.bin' % spec_id)
    write_cases(path, missions, cases)
    ref, _ = build(spec_id, spec)
    err = subprocess.run([ref, *ROPES, path], cwd=R, check=True, capture_output=True, text=True,
                         env={**os.environ, 'IRC_FLOW': '1'}).stderr
    seen = collections.defaultdict(lambda: collections.defaultdict(set))
    for line in err.splitlines():
        f = line.split()
        if f[:1] != ['F']:
            continue
        n, p, *vals = (int(x) for x in f[1:])
        mi = missions[cases[n]['cm']]
        if p not in mi.cp.starts or mi.cp.starts[p][1].startswith('entry'):
            continue
        for k, v in zip(FLOW_KEYS, vals):
            # A PUSHLOC off the pushdown list is an entry case's own random
            # one, not a state the file reaches.
            if k == '0166' and v >= 0o46:
                continue
            seen[(cases[n]['cm'], p)][k].add('vac+%o' % v if k == '0166' else v)
    for (cm, p), d in seen.items():
        missions[cm].flow[p] = {k: sorted(v, key=str) for k, v in d.items()}
    print('flow states: %d routines reached from %d entry cases' % (len(seen), len(cases)))


def build_edges(spec_id, spec, extra=(), tag=''):
    """The native runner with every linked source (the milestone's routines,
    the interpreter bodies, the image ops and native RTBs; not the test
    harness) instrumented per edge, so each branch outcome is a guard."""
    d = os.path.join(B, '%s-edges%s' % (spec_id, tag))
    os.makedirs(d, exist_ok=True)
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + \
        [x for x in COMMON if not x.startswith('tests/')]
    objs = []
    sites = spec.get('site_edges')
    mine = {'src/%s.c' % spec['base'][m] for m in spec['missions']}
    for src in srcs:
        o = os.path.join(d, os.path.basename(src) + '.o')
        subprocess.run(['clang', '-std=c99', '-O0', '-g', '-fno-pie', '-Iinclude', '-Itests', *extra,
                        '-fsanitize-coverage=trace-pc-guard', '-c', src, '-o', o], cwd=R, check=True)
        objs.append(o)
    nat = os.path.join(d, 'native')
    subprocess.run(['clang', '-std=c99', '-O0', '-g', '-no-pie', '-Iinclude', '-Itests', '-DIRC_EDGES',
                    *(['-DIRC_SITES', '-Wl,-Map,%s.map' % nat] if sites else []),
                    '-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200), *objs,
                    'tests/interp_case.c', spec['target'], 'tools/interp_routine_native.c', 'tools/irc_edges.c',
                    '-o', nat], cwd=R, check=True)
    if sites:
        # 'site_edges' (tools/irc_edges.c): the code address ranges, from
        # the link map's input sections, of the milestone's routines
        # (.text.r<key>: kind 1), the trampoline (2) and the milestone's
        # own helpers (3, counted per call site like the shared code).
        kind = {os.path.basename(src) + '.o': 1 for src in mine}
        kind['agc_transfer.c.o'] = 2
        out, text, name = [], False, None
        for line in open(nat + '.map'):
            if re.match(r'^[.\w]', line):
                # An output section starts at column 0.
                text, name = line.startswith('.text'), None
                continue
            m = re.match(r'^ (\.\S+)(?:\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+))?\s*$', line)
            if m:
                name = m.group(1)
                if not m.group(2):
                    continue
                start, size, obj = m.group(2), m.group(3), m.group(4)
            else:
                m = re.match(r'^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)\s*$', line)
                if not m or name is None:
                    continue
                start, size, obj = m.groups()
            k = kind.get(os.path.basename(obj))
            if text and k and int(size, 16):
                if k == 1 and not re.fullmatch(r'\.text\.r\d+', name):
                    k = 3
                out.append((int(start, 16), int(start, 16) + int(size, 16), k))
        assert {1, 2} <= {k for _, _, k in out}, 'site_edges: no milestone routines or trampoline in the link map'
        with open(nat + '.sites', 'w') as f:
            f.writelines('%x %x %d\n' % r for r in sorted(out))
    return nat


def edge_sets(nat, path):
    """Per case, the guards it reached; and each guard's source line."""
    # (Streamed, with equal sets shared: most cases reach the same edges.
    # 'site_edges': "s<site>.<guard>", a shared guard under the call site's
    # block, numbered from 1 << 32 so they never meet a guard.)
    seen, pairs = {}, {}

    def edge(t):
        if t[0] == 'e':
            return int(t[1:])
        return pairs.setdefault(t[1:], (1 << 32) + len(pairs))
    env = dict(os.environ, IRC_SITE_RANGES=nat + '.sites') if os.path.exists(nat + '.sites') else None
    with subprocess.Popen([nat, path, 'edges'], cwd=R, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, text=True, env=env) as pr:
        import threading
        err = []
        th = threading.Thread(target=lambda: err.append(pr.stderr.read()))
        th.start()
        sets = []
        for l in pr.stdout:
            e = frozenset(edge(t) for t in l.split()[4:] if t[0] in 'es')
            sets.append(seen.setdefault(e, e))
        th.join()
    if pr.returncode:
        raise subprocess.CalledProcessError(pr.returncode, nat)
    pcs = [l.split()[1:] for l in err[0].splitlines() if l.startswith('pc ')]
    sym = subprocess.run(['llvm-symbolizer', '--obj=' + nat],
                         input='\n'.join(a for _, a in pcs) + '\n', cwd=R, capture_output=True, text=True).stdout
    lines = [b.split('\n')[1] for b in sym.strip().split('\n\n')]
    where = Where()
    for (g, _), ln in zip(pcs, lines):
        f, _, rest = ln.partition(':')
        f = os.path.relpath(f, R) if f.startswith('/') else f
        where[int(g)] = '%s:%s' % (f, rest.split(':')[0])
        where.col[int(g)] = '%s:%s' % (f, rest)
    for k, n in pairs.items():
        a, b = (int(x) for x in k.split('.'))
        where[n] = '%s from %s' % (where.get(b, '?'), where.get(a, '?'))
        where.col[n] = '%s from %s' % (where.col.get(b, '?'), where.col.get(a, '?'))
    return sets, where


class Where(dict):
    """Each guard's source line; .col its line and column, which name the
    same outcome across builds (a segment runner's KEEP mask renumbers the
    guards)."""
    def __init__(self):
        super().__init__()
        self.col = {}



def segment_rows(spec_id, spec, vsym, missions, mi, cases, rows, visits, live, edges, got):
    """'c64_segments': C64 rows for the branch outcomes that only runs too
    long for one diagnostic take, where they lie before a CHANG2 resume. The
    row runs its case from its routine to the resume LOC after it (that
    routine left out of the group's KEEP mask, so the run stops there),
    against the oracle stopped at NEWOPS with LOC there (IRC_SNAPROW). Each
    segment runs on the host with the same KEEP mask first and must equal
    the oracle. Appends the rows' cases, oracle rows, visits and edge sets
    (in the full build's guards); returns {point: [case]}."""
    sets, where = edges
    k_of = {p: k for k, p in enumerate(mi.points)}
    nxt = {p: mi.points[k + 1] for k, p in enumerate(mi.points[:-1])
           if mi.cp.starts[mi.points[k + 1]] == ('i', 'CHANG2 resume')}
    target = set().union(*(sets[i] for i in live)) - got
    cand = collections.defaultdict(list)
    for i in sorted(live, key=lambda i: (len(cases[i]['edits']), i)):
        p = cases[i]['point']
        if p in nxt and sets[i] & target and len(cand[p]) < 64:
            cand[p].append(i)
    fullcol = collections.defaultdict(list)
    for g, c in where.col.items():
        fullcol[c].append(g)
    out = {}
    sym = spec['symbol'][mi.m]
    for p, ii in sorted(cand.items()):
        if not target:
            break
        k = k_of[p]
        mask = 1 << k
        flags = keep_flags(sym, mask)
        tag = '-seg-%s-%x' % (mi.m, k)
        seg = [dict(cases[i], stop=nxt[p], src=i) for i in ii]
        path = os.path.join(B, '%s%s.bin' % (spec_id, tag))
        write_cases(path, missions, seg)
        ref, nat = build(spec_id, spec, flags, tag)
        rr = [l.split() for l in subprocess.run([ref, *ROPES, path], cwd=R, check=True, capture_output=True, text=True,
                                                env=dict(os.environ, IRC_SNAP='1', IRC_SNAPROW='1')).stdout.splitlines()]
        nn = run([nat, path])
        ok = []
        for j, (x, y) in enumerate(zip(rr, nn)):
            if x[1] == 'N':
                continue
            if x != y:
                sys.exit('%s segment %s to %s, case %d: oracle %s, native %s' % (
                    mi.m, mi.cp.where(p), mi.cp.where(nxt[p]), ii[j], ' '.join(x[1:]), ' '.join(y[1:])))
            ok.append(j)
        es, ew = edge_sets(build_edges(spec_id, spec, flags, tag), path)
        segcol = collections.defaultdict(list)
        for g, c in ew.col.items():
            segcol[c].append(g)
        tr = {}
        for c, gs in segcol.items():
            tr.update(zip(sorted(gs), sorted(fullcol.get(c, []))))
        got_s = {j: frozenset(tr[g] for g in es[j] if g in tr) for j in ok}
        while target:
            j = max(ok, key=lambda j: (len(got_s[j] & target), -j), default=None)
            if j is None or not got_s[j] & target:
                break
            ok.remove(j)
            idx = len(cases)
            cases.append(seg[j])
            rows.append(rr[j])
            visits.append(mask)
            if c64_size(spec, vsym, mi.m, mask, group_text(vsym, [(mi.m, mask, [idx])], cases, rows)) is None:
                cases.pop(); rows.pop(); visits.pop()
                continue
            sets.append(got_s[j])
            target -= got_s[j]
            out.setdefault(p, []).append(idx)
            SEGMENTS.append({'mission': mi.m, 'from': mi.cp.where(p), 'to': mi.cp.where(nxt[p]), 'case': ii[j]})
    return out


SEGMENTS = []   # the C64 segment rows (proof field c64_segments)


def greedy_cover(cands, sets, covered=(), key=lambda i: i, accept=lambda i: True):
    """Cases (in pick order) that together reach every guard the candidates
    reach, each adding the most not yet reached (lazy greedy)."""
    import heapq
    cov = set(covered)
    heap = [(-len(sets[i] - cov), key(i), i) for i in cands]
    heapq.heapify(heap)
    picked = []
    while heap:
        g, k, i = heapq.heappop(heap)
        gain = len(sets[i] - cov)
        if gain == 0:
            continue
        if gain != -g:
            heapq.heappush(heap, (-gain, k, i))
            continue
        if not accept(i):
            continue
        picked.append(i)
        cov |= sets[i]
    return picked, cov


# A native or reference run that outlives this (seconds) is a hang, not a
# slow spec: an interpreter loop with no step budget, say (#224).
RUN_TIMEOUT = int(os.environ.get('IRC_RUN_TIMEOUT', '1200'))


def run(cmd):
    try:
        out = subprocess.run(cmd, cwd=R, check=True, capture_output=True, text=True,
                             timeout=RUN_TIMEOUT).stdout
    except subprocess.TimeoutExpired:
        sys.exit('FAIL: %s ran over %d s (IRC_RUN_TIMEOUT); a case hangs' % (' '.join(cmd), RUN_TIMEOUT))
    except subprocess.CalledProcessError as e:
        sys.exit('FAIL: %s exited %d\n%s' % (' '.join(cmd), e.returncode, e.stderr))
    return [l.split() for l in out.splitlines()]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('spec')
    ap.add_argument('--cases', type=int, default=64, help='cases per routine')
    ap.add_argument('--freeze', action='store_true')
    ap.add_argument('--show', type=int, default=3)
    ap.add_argument('--only', default='')
    ap.add_argument('--cover-check', metavar='RECORD', nargs='?', const='',
                    help='re-derive the committed C64 branch cover (default record: '
                         'evidence/<id>/<id>-c64-cover.json, else -proof.json) and print it as JSON')
    a = ap.parse_args()
    spec = interp_specs.SPECS[a.spec]
    if spec.get('channel_inputs'):
        os.environ['IRC_INPUTS'] = '1'   # (the oracle latches the cases' input channels)
    if IC.main(['', a.spec, '--check']):
        sys.exit('generated sources are stale: run tools/interp_compile.py ' + a.spec)
    missions = [Mission(spec, m) if m in spec['missions'] else Absent(m) for m in ('lm', 'cm')]
    if spec.get('flow_states'):
        flow_states(a.spec, spec, missions)
    rng = random.Random('interp-routine:' + a.spec)
    proof = os.path.join(R, 'evidence/%s/%s-proof.json' % (a.spec, a.spec))
    if a.cover_check is not None and os.path.exists(proof):
        # (The freeze's cases per routine, from the case count it froze.)
        n = sum(len(mi.points) for mi in missions if mi.m in spec['missions'])
        a.cases = max(1, json.load(open(proof)).get('cases', a.cases * n) // n)
    cases = []
    for cm, mi in enumerate(missions):
        if mi.m not in spec['missions']:
            continue
        for p in mi.points:
            for n in range(a.cases):
                seed, edits, ie, stop, sb = case(mi, p, n, rng)
                cases.append({'cm': cm, 'point': p, 'seed': seed, 'edits': Edits(edits), 'ie': ie, 'stop': stop, 'sb': sb})
    os.makedirs(B, exist_ok=True)
    path = os.path.join(B, '%s-cases.bin' % a.spec)
    ref, nat = build(a.spec, spec)
    BUILT[:] = [ref, nat]
    if spec.get('resume_flow'):
        flow_seed(missions, cases, rng, ref, path)
    write_cases(path, missions, cases)
    if a.cover_check is not None:
        sys.exit(cover_check(a.spec, spec, missions, cases, path, a.cover_check))
    rows_r = run([ref, *ROPES, path])
    rows_n = run([nat, path])
    bad = [i for i, (x, y) in enumerate(zip(rows_r, rows_n)) if x != y]
    # A native run that ends refusing in a routine with an I/O channel
    # instruction waits for a channel model: counted, not compared.
    chan = {}
    for i in bad:
        mi = missions[cases[i]['cm']]
        z = int(rows_n[i][2])
        hit = [p for p in mi.cp.channel_blocks if mi.cp.logical_of(p) == z]
        if rows_n[i][1] != '0' and hit:
            chan[i] = '%s %s' % (mi.m, mi.cp.where(hit[0]))
    bad = [i for i in bad if i not in chan]
    if chan:
        print('waiting for an I/O channel model: %d cases (%s)' % (len(chan), ', '.join(
            '%s x%d' % kv for kv in sorted(collections.Counter(chan.values()).items()))))
        # Where the oracle went from each refused case: the refused domain.
        for k, v in sorted(chan_outcomes(chan, rows_r).items()):
            print('  refused at %s, oracle: %s' % (k, ', '.join('%s x%d' % kv for kv in sorted(v.items()))))
    stops = collections.defaultdict(collections.Counter)
    for i, (c, r) in enumerate(zip(cases, rows_r)):
        mi = missions[c['cm']]
        key = '%s %s %s' % (mi.m.upper(), mi.cp.where(c['point']), mi.cp.name(c['point']))
        if i in chan:
            stops[key]['I/O channel'] += 1
            continue
        z = int(r[2])
        stops[key]['%05o' % z] += 1
    if bad or len(rows_r) != len(rows_n):
        sel = [i for i in bad if not a.only or a.only in "%s %s" % (missions[cases[i]["cm"]].m, missions[cases[i]["cm"]].cp.where(cases[i]["point"]))]
        # Dumps of the shown cases only (all of them would not fit in memory).
        show = sel[:a.show]
        sub = path + '.show'
        write_cases(sub, missions, [cases[i] for i in show])
        dr = dict(zip(show, run([ref, *ROPES, sub, 'dump'])))
        dn = dict(zip(show, run([nat, sub, 'dump'])))
        for i in show:
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
    timeouts = [r for i, r in enumerate(rows_r) if r[1] != '0' and i not in chan]
    if timeouts:
        sys.exit('FAIL: %d oracle timeouts' % len(timeouts))
    print('PASS: %d %s cases equal the original programs (%d routines)' % (
        len(cases), a.spec, sum(len(mi.points) for mi in missions if mi.m in spec['missions'])))
    for k in sorted(stops):
        print('  %s: %s' % (k, ', '.join('%s x%d' % (z, n) for z, n in sorted(stops[k].items()))))
    if a.freeze:
        idxs = [{p: k for k, p in enumerate(mi.points)} for mi in missions]
        visits = []
        with subprocess.Popen([nat, path, 'visits'], cwd=R, stdout=subprocess.PIPE, text=True) as pr:
            for c, l in zip(cases, pr.stdout):
                idx = idxs[c['cm']]
                v = 0
                for t in l.split()[4:]:
                    if int(t[1:]) in idx:   # (erasable continuations are always kept)
                        v |= 1 << idx[int(t[1:])]
                visits.append(v)
        if pr.returncode:
            raise subprocess.CalledProcessError(pr.returncode, nat)
        covers = None
        if spec.get('basic') == 'full':
            covers = edge_covers(spec, nat, path)
        edges = None
        if spec.get('cover'):
            edges = edge_sets(build_edges(a.spec, spec), path)
        freeze(a.spec, spec, missions, cases, rows_r, stops, visits, chan=chan, edges=edges, covers=covers)


def cover_check(spec_id, spec, missions, cases, path, record):
    """--cover-check: the committed record's C64 rows against the host
    cases, both re-run now (gcov_branches, host only, no oracle). The cases
    are the frozen ones when their bytes hash to the record's cases_sha256
    (a -proof.json); a -c64-cover.json pins none. Prints JSON; returns 1
    when the rows miss a branch the host cases take, or the cases differ."""
    if not record:
        record = next((p for p in ('evidence/%s/%s-c64-cover.json' % (spec_id, spec_id),
                                   'evidence/%s/%s-proof.json' % (spec_id, spec_id))
                       if os.path.exists(os.path.join(R, p))), None)
    if not record:
        print(json.dumps({'milestone': spec_id, 'verified': False, 'reason': 'no committed cover record'}))
        return 1
    rec = json.load(open(os.path.join(R, record)))
    # (The proof next to a cover record pins the frozen cases, and lists the
    # C64 rows when the record only counts them.)
    proof = os.path.join(R, 'evidence/%s/%s-proof.json' % (spec_id, spec_id))
    if isinstance(rec.get('c64_rows'), list):
        rows = rec['c64_rows']
    else:
        src = rec if 'c64_vectors' in rec or not os.path.exists(proof) else json.load(open(proof))
        groups = src.get('c64_vectors', [])
        rows = [i for g in groups for i in g['case_ids']] if all('case_ids' in g for g in groups) else []
    out = {'milestone': spec_id, 'record': record, 'cases': len(cases), 'c64_rows': len(rows)}
    sha = rec.get('cases_sha256') or (json.load(open(proof)).get('cases_sha256') if os.path.exists(proof) else None)
    out['cases_match'] = hashlib.sha256(open(path, 'rb').read()).hexdigest() == sha if sha else None
    beyond = [i for i in rows if i >= len(cases)]
    if out.get('cases_match') is False or beyond or not rows:
        out.update(verified=False, reason='the regenerated cases are not the frozen ones' if rows else
                   'the record does not list its C64 rows (refreeze to record them)')
        print(json.dumps(out))
        return 1
    if spec.get('cover'):
        build_edges(spec_id, spec)      # (a freeze links it: catch a broken link here)
    g = gcov_branches(spec_id, spec, missions, cases, rows)
    out.update(verified=True, host_taken=g['host_taken'], c64_taken=g['c64_taken'],
               missing=g['missing'], missing_count=g['host_taken_not_c64'])
    print(json.dumps(out))
    return 1 if g['host_taken_not_c64'] else 0


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
    # The same step limit as the freeze (host_arcs), or a case that runs
    # past 200 routines reports FAILED instead of STOPPED.
    defs = ['-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200)]
    res = {}
    for tag, sel in (('host', range(len(cases))), ('c64', sorted(set(c64_ids)))):
        w = os.path.join(B, '%s-gcov-%s' % (spec_id, tag))
        shutil.rmtree(w, ignore_errors=True)
        os.makedirs(w)
        write_cases(os.path.join(w, 'cases.bin'), missions, [cases[i] for i in sel])
        objs = []
        for s in srcs:
            o = os.path.join(w, os.path.basename(s)[:-2] + '.o')
            subprocess.run(['gcc', '-std=c99', '-O0', '--coverage', '-Iinclude', '-Itests', *defs, '-c', s, '-o', o],
                           cwd=R, check=True)
            objs.append(o)
        subprocess.run(['gcc', '-O0', '--coverage', '-Iinclude', '-Itests', *defs, *objs, 'tools/interp_routine_native.c',
                        '-o', os.path.join(w, 'run')], cwd=R, check=True)
        subprocess.run([os.path.join(w, 'run'), os.path.join(w, 'cases.bin')], cwd=R, check=True,
                       stdout=subprocess.DEVNULL)
        taken, seen = set(), set()
        for o in objs:
            out = subprocess.run(['gcov', '-b', '-c', '-t', '-o', w, o[:-2] + '.gcda'], cwd=R, check=True,
                                 capture_output=True, text=True).stdout
            src = line = None
            for l in out.splitlines():
                m = re.match(r'\s*-:\s*0:Source:(.*)', l)
                if m:
                    src = m.group(1)
                    seen.add((os.path.basename(o)[:-2], src))
                    continue
                m = re.match(r'\s*[^:]+:\s*(\d+):', l)
                if m:
                    line = int(m.group(1))
                    continue
                m = re.match(r'branch\s+(\d+) taken (\d+)', l)
                if m and int(m.group(2)):
                    taken.add((os.path.basename(o)[:-2], src, line, int(m.group(1))))
        res[tag] = c64_row_cover.arc_keys(taken, seen)
        shutil.rmtree(w, ignore_errors=True)
    miss = sorted(res['host'] - res['c64'], key=lambda a: [int(t) if t.isdigit() else t for t in re.split(r'(\d+)', a)])
    return {'host_taken': len(res['host']), 'c64_taken': len(res['c64']), 'host_taken_not_c64': len(miss),
            'missing': miss[:50],
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
    flags = [f for f in FLAGS if f != '-O2'] + ['-O0', '-DIRC_COVER_ON', '-DIRC_EDGE_COVER']
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


def chan_outcomes(chan, rows):
    """{channel block: {oracle stop Z (or 'timeout'): cases}} of the refused cases."""
    out = collections.defaultdict(collections.Counter)
    for i, k in chan.items():
        out[k]['%05o' % int(rows[i][2]) if rows[i][1] == '0' else 'timeout'] += 1
    return out


def freeze(spec_id, spec, missions, cases, rows, stops, visits, chan=(), edges=None, covers=None):
    stride = spec.get('freeze_stride', 16)
    pick = [i for i in range(0, len(cases), stride) if i not in chan]
    cover = None
    if edges is not None:
        # With 'cover': the sample, plus the fewest cases that reach every
        # edge (branch outcome) any case reaches.
        sets, where = edges
        live = [i for i in range(len(cases)) if i not in chan]
        target = set().union(*(sets[i] for i in live))
        base = set().union(*(sets[i] for i in pick))
        more, _ = greedy_cover(live, sets, base, key=lambda i: (len(cases[i]['edits']), i))
        pick = sorted(set(pick) | set(more))
        cover = {'method': 'clang -fsanitize-coverage=trace-pc-guard edges (each branch outcome) over every '
                           'linked source but the test harness, per case (tools/irc_edges.c)' + (
                               '; with site_edges, every edge of the shared code and the milestone\'s own helpers '
                               'counts once per call site (the routine block that made the call)'
                               if spec.get('site_edges') else ''),
                 'edges_all_cases': len(target), 'edges_host_vectors': len(set().union(*(sets[i] for i in pick)))}
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
    out.append(',\n'.join(' {0x%08xul,%d,%d,%s,%d,%s_e%d}' % (cases[i]['seed'], cases[i]['cm'], cases[i]['ie'], '0%o' % cases[i].get('sb', 0) if cases[i].get('sb') else '0',
                                                            len(cases[i]['edits']), sym, j) for j, i in enumerate(pick)))
    out.append('};')
    out.append('static const uint32_t %s_expected[%d]={' % (sym, len(pick)))
    out.append(',\n'.join(' 0x%08xul' % int(rows[i][3]) for i in pick))
    out.append('};')
    open(os.path.join(R, 'tests/%s_vectors.h' % sym), 'w').write('\n'.join(out) + '\n')
    c64, rows_cover = freeze_c64(spec_id, spec, sym, missions, cases, rows, visits, covers, chan, edges)
    if cover is not None:
        sets, where = edges
        got = set().union(*(sets[i] for g in C64_PICKS for i in g))
        live = [i for i in range(len(cases)) if i not in chan]
        target = set().union(*(sets[i] for i in live))
        cover['edges_c64'] = len(got)
        cover['edges_not_on_c64'] = sorted(set(where.get(e, '?') for e in target - got))
    blob = open(os.path.join(B, '%s-cases.bin' % spec_id), 'rb').read()
    proof = {'milestone': spec_id, 'missions': ['Luminary099', 'Comanche055'],
             'rope_sha256': [hashlib.sha256(open(p, 'rb').read()).hexdigest() for p in ROPES],
             'cases': len(cases), 'cases_sha256': hashlib.sha256(blob).hexdigest(),
             'oracle_sha256': hashlib.sha256('\n'.join(' '.join(r) for r in rows).encode()).hexdigest(),
             'frozen_vectors': len(pick), 'c64_vectors': c64,
             **({'c64_coverage': cover_counts(covers, [i for g in c64 for i in g['case_ids']])} if covers else {}),
             **({'c64_gcov_branches': gcov_branches(spec_id, spec, missions, cases,
                                                    [i for g in c64 for i in g['case_ids']] if covers else
                                                    [i for g in C64_PICKS for i in g])}
                if covers or spec.get('c64_rows') == 'cover' else {}),
             **({'c64_host_only': C64_HOST_ONLY} if C64_HOST_ONLY else {}),
             **({'c64_segments': {'method': 'from the case\'s routine to the CHANG2 resume LOC after it (that routine '
                                            'outside the KEEP mask), against yaAGC stopped at NEWOPS with LOC there '
                                            '(IRC_SNAPROW); each equal to the oracle on the host with the same mask',
                                  'rows': SEGMENTS}} if SEGMENTS else {}),
             **({'c64_too_big_cases': [{'case': i, 'point': cases[i]['point']} for i in C64_TOO_BIG]} if C64_TOO_BIG else {}),
             **({'outcome_cover': cover} if cover is not None else {}),
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
                if spec.get('seed_counters') else {})}
    proof.update(rows_cover)
    if 'c64_gcov_branches' in proof:
        # (tools/cert_precheck.py reads the C64 rows' gap from this key.)
        proof['host_branches_missing_on_c64'] = proof['c64_gcov_branches']['missing']
    if chan:
        proof['waiting_for_io_channels'] = dict(collections.Counter(chan.values()))
        proof['io_channel_refused_oracle_outcomes'] = {k: dict(sorted(v.items())) for k, v in sorted(chan_outcomes(chan, rows).items())}
    open(os.path.join(B, '%s-proof.json' % spec_id), 'w').write(json.dumps(proof, indent=1) + '\n')
    smap = {}
    for mi in missions:
        if mi.m not in spec['missions']:
            continue
        rows_m = mi.cp.source_map()
        loose = [r for r in rows_m if 'routine' not in r and 'constant' not in r and 'cadr_list' not in r]
        if loose and mi.m not in spec.get('partial', {}):
            sys.exit('%s words no routine compiles: %s' % (mi.m, ', '.join(r['address'] for r in loose)))
        for r in loose:
            r['untranslated'] = spec['partial'][mi.m]
        smap[('Luminary099/' if mi.m == 'lm' else 'Comanche055/') + spec.get('source_path', spec['file'])[mi.m]] = {
            'native_file': 'src/%s.c' % spec['base'][mi.m], 'words': len(rows_m), 'untranslated_words': len(loose),
            'routines': len(mi.points), 'map': rows_m}
    open(os.path.join(B, '%s-source-map.json' % spec_id), 'w').write(json.dumps(smap, indent=1) + '\n')
    print('froze %d vectors to tests/%s_vectors.h' % (len(pick), sym))




# The program, its data and bss end below the $c000 mailbox (the soft stack
# grows down from $d000), with room to spare.
C64_TOP = 0xbf00


def c64_sources(spec, m):
    return (['src/%s.c' % spec['base'][m], 'src/%s_interp_orders.c' % m, 'src/%s_interp_steps.c' % m]
            + spec.get('native_sources', []) + [x for x in common(spec) if '_interp_' not in x] + [spec['target']])


KEEP_WORDS = {}   # symbol -> the generated file's KEEP words


def keep_flags(symbol, mask):
    """-D<SYM>_KEEP<n> for a routine mask, 60 routines per word (every
    word the file has, so none defaults to all)."""
    n = KEEP_WORDS[symbol]
    return ['-D%s_KEEP%d=0x%x' % (symbol.upper(), w, (mask >> (60 * w)) & ((1 << 60) - 1)) for w in range(n)]


C64_SIZES = {}   # probe memo: selection reruns probe the same groups


def c64_size(spec, sym, m, mask, text, pick=()):
    """End of the C64 diagnostic's memory (code, data and bss) for one group,
    or None when it does not link or reaches past C64_TOP."""
    key = (sym, m, mask, hashlib.sha256(text.encode()).hexdigest(),
           tuple(GROUP_EXTRA.get(sym, lambda mask, pick: [])(mask, pick)))
    if key not in C64_SIZES:
        C64_SIZES[key] = c64_probe(spec, sym, m, mask, text, pick)
    return C64_SIZES[key]


def c64_probe(spec, sym, m, mask, text, pick=()):
    d = os.path.join(B, '%s-probe-%s-%x' % (sym, m, mask))
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, '%s_c64_vectors.h' % sym), 'w').write(text)
    prg = os.path.join(d, 'probe.prg')
    # (Channel and counter edits need their case code on the C64 too.)
    io = ['-DIRC_CHANNELS'] if spec.get('seed_counters') or spec.get('channels') else []
    io += ['-DIRC_INPUTS'] if spec.get('channel_inputs') else []
    r = subprocess.run([MOS_CC, '-I' + d, '-Iinclude', '-Itests', *C64_FLAGS, *io, *spec.get('c64_flags', []), '-DIRC_GROUP=1',
                        '-DIRC_%s_ONLY' % m.upper(), *keep_flags(spec['symbol'][m], mask),
                        *GROUP_EXTRA.get(sym, lambda mask, pick: [])(mask, pick),
                        *c64_sources(spec, m), 'platform/c64/%s_diagnostic.c' % sym,
                        '-Wl,-Map,' + os.path.join(d, 'probe.map'), '-o', prg],
                       cwd=R, capture_output=True, text=True)
    end = None
    if r.returncode == 0:
        for ln in open(os.path.join(d, 'probe.map')):
            if '__heap_start = ' in ln:
                end = int(ln.split()[0], 16)
    if os.environ.get('IRC_C64_DEBUG'):
        print('c64 probe %s %x: rc %d end %s %s' % (m, mask, r.returncode, end and hex(end), r.stderr[-800:]), file=sys.stderr)
    else:
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
        out.append(',\n'.join(' {0x%08xul,%d,%d,%s,%d,%s_e%d}' % (cases[i]['seed'], cases[i]['cm'], cases[i]['ie'], '0%o' % cases[i].get('sb', 0) if cases[i].get('sb') else '0',
                                                                len(cases[i]['edits']), pre, j) for j, i in enumerate(pick)))
        out.append('};')
        out.append('static const uint32_t irc_c64_expected[%d]={' % len(pick))
        out.append(',\n'.join(' 0x%08xul' % int(rows[i][3]) for i in pick))
        out.append('};')
        if any(cases[i].get('cut') for i in pick):
            # Each case's cut (0: its whole run), for irc_run_cut.
            out.append('#define IRC_C64_CUTS 1')
            out.append('static const uint16_t irc_c64_cut[%d]={%s};' % (
                len(pick), ','.join('0%o' % cases[i]['cut'] if cases[i].get('cut') else '0' for i in pick)))
        out.append('#endif')
    return '\n'.join(out) + '\n'


def cover_sources(spec):
    """The sources the cover runner links (host_arcs)."""
    return (['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + common(spec) +
            [spec['target']])


def write_cover_record(spec_id, spec, arcs, rows, full=None, cut=None):
    """build/<id>-c64-cover.json, the c64_branch_outcomes record a
    certificate cites (evidence/<id>/<id>-c64-cover.json). full: the arcs
    of every case run in one process (wider_arcs); cut: the truncated rows
    ('c64_cuts')."""
    host = frozenset().union(*arcs.values())
    taken = frozenset().union(*(arcs[n] for n in rows))
    sources = cover_sources(spec)
    per = {s: [0, 0] for s in sources}
    for arc in host:
        per.setdefault(arc.split(':', 1)[0], [0, 0])[0] += 1
    for arc in taken:
        per.setdefault(arc.split(':', 1)[0], [0, 0])[1] += 1
    record = {'milestone': spec_id,
              'method': 'gcov -b branch arcs (file:line branch N) of the native runner built at -O0 --coverage '
                        'over every linked source, one host case at a time; the rows are the C64 vector '
                        'header\'s case list, each run on VICE against its frozen digest '
                        '(tools/interp_routine_differential.py, tools/c64_row_cover.py)',
              'linked_sources': {s: hashlib.sha256(open(os.path.join(R, s), 'rb').read()).hexdigest()
                                 for s in sources},
              'host_cases': len(arcs), 'host_case_stride': spec.get('cover_stride', 1), 'c64_rows': sorted(rows),
              'host_branch_arcs': len(host), 'c64_branch_arcs': len(taken),
              'arcs_by_source_host_c64': {s: per[s] for s in sorted(per)},
              'host_branches_missing_on_c64': sorted(host - taken)}
    if full is not None:
        record['all_cases_branch_arcs'] = len(full)
        record['all_cases_branches_missing_on_c64'] = sorted(full - taken)
    if cut:
        record['truncated_rows'] = cut
    open(os.path.join(B, '%s-c64-cover.json' % spec_id), 'w').write(json.dumps(record, indent=1) + '\n')


def host_arcs(spec_id, spec, missions, cases):
    """{case: branch arcs} of the native runner, one case at a time
    (tools/c64_row_cover.py)."""
    assert spec.get('freeze_stride', 16) % spec.get('cover_stride', 1) == 0
    d = os.path.join(B, '%s-cover' % spec_id)
    exe = c64_row_cover.build(d, [*cover_sources(spec), 'tools/interp_routine_native.c'],
                              [f for f in FLAGS if f != '-O2'] + ['-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200)] +
                              cut_flags(spec))
    one = os.path.join(d, 'cases')
    os.makedirs(one)

    def argv(i):
        p = os.path.join(one, '%d.bin' % i)
        write_cases(p, missions, [cases[i]])
        return [str(exe), p]
    # 'cover_stride': every Nth case (a divisor of freeze_stride, so every
    # frozen host vector is among them).
    return c64_row_cover.trace(d, range(0, len(cases), spec.get('cover_stride', 1)), argv)


def wider_arcs(spec_id, missions, cases, arcs):
    """With cover_stride the traced cases are a sample: every case in one
    process of host_arcs' runner gives the arcs all the cases take. When
    that adds arcs, the untraced cases (fewest edits first) run in blocks of
    512, 64 and 8, each size within the first four blocks that take each
    new arc, then one at a time. Returns
    (all the arcs, {case: arcs} of the cases newly traced)."""
    d = os.path.join(B, '%s-cover' % spec_id)
    exe, one = os.path.join(d, 'runner'), os.path.join(d, 'cases')

    def argv(ii):
        p = os.path.join(one, '%d-%d.bin' % (ii[0], len(ii)))
        write_cases(p, missions, [cases[i] for i in ii])
        return [exe, p]
    every = tuple(range(len(cases)))
    full = c64_row_cover.trace(d, [every], argv, jobs=1)[every]
    new = full - frozenset().union(*arcs.values())
    if not new:
        return full, {}
    hit = sorted((i for i in every if i not in arcs), key=lambda i: (len(cases[i]['edits']), i))
    for n in (512, 64, 8):
        order = [tuple(hit[k:k + n]) for k in range(0, len(hit), n)]
        blocks = c64_row_cover.trace(d, order, argv)
        keep = set()
        for arc in new:
            keep.update([b for b in order if arc in blocks[b]][:4])
        hit = [i for b in order if b in keep for i in b]
    more = c64_row_cover.trace(d, hit, lambda i: argv((i,)))
    print('  C64 cover: %d arcs only untraced cases take; %d more cases traced' % (len(new), len(hit)))
    return full, more


def trace_cases(spec_id, missions, cases, ii):
    """{case: arcs} of the given cases, one at a time, with host_arcs' runner."""
    d = os.path.join(B, '%s-cover' % spec_id)
    exe, one = os.path.join(d, 'runner'), os.path.join(d, 'cases')

    def argv(i):
        p = os.path.join(one, 'late-%d.bin' % i)
        write_cases(p, missions, [cases[i]])
        return [exe, p]
    return c64_row_cover.trace(d, ii, argv)


C64_HOST_ONLY = []   # routines whose runs do not fit one C64 diagnostic
GROUP_EXTRA = {}     # symbol -> (mask, pick) -> extra C64 group flags
C64_TOO_BIG = []     # cases whose run alone does not fit one C64 diagnostic


def bank_flags(spec, mi, covers):
    """Spec 'basic_split': -D<SYM>_BANKS for a group, the banks of its
    basic routines and of every basic word its cases reach (the labels in
    their covers)."""
    if not spec.get('basic_split') or covers is None:
        return None
    nb = [k[0] // 1024 for k in sorted(mi.cp.basic.nodes)]   # (IRC_COVER numbering)
    sym = spec['symbol'][mi.m].upper()
    case_banks = {}

    def of(i):
        if i not in case_banks:
            lab = covers[i] & ((1 << 8192) - 1)
            case_banks[i] = {nb[n] for n in range(len(nb)) if lab >> n & 1}
        return case_banks[i]

    def flags(mask, pick):
        banks = {p // 1024 for k, p in enumerate(mi.points) if mask >> k & 1 and mi.kinds[p] == 'b'}
        for i in pick:
            banks |= of(i)
        return ['-D%s_BANKS=0x%xull' % (sym, sum(1 << b for b in banks))]
    return flags


BUILT = []           # the oracle and the native runner (main)


def cut_cases(spec_id, spec, sym, missions, cases, rows, visits, arcs, want):
    """'c64_cuts': for each arc in want (taken by cases whose whole runs fit
    no C64 diagnostic), a truncated case: one such case (fewest edits first)
    run instead to a cut, the first NEWOPS with LOC at an interpretive
    routine its run enters (tests/interp_cut.h), the earliest whose prefix
    takes the arc, when that prefix fits one diagnostic. The yaAGC oracle
    runs the same case to the same cut: a cut it does not stop at first is
    refused, and its state there is the row's digest. Appends to cases,
    rows, visits and arcs (case keys 'cut', 'of', 'ran')."""
    ref, nat = BUILT
    d = os.path.join(B, '%s-cover' % spec_id)
    exe, one = os.path.join(d, 'runner'), os.path.join(d, 'cases')
    os.makedirs(one, exist_ok=True)
    trial = {}

    def argv(key):
        p = os.path.join(one, '%s.bin' % key)
        write_cases(p, missions, [trial[key]])
        return [exe, p]
    want, made, refused, differ = set(want), 0, 0, 0
    for arc in sorted(want):
        if arc not in want:
            continue
        cands = sorted((i for i in arcs if arc in arcs[i] and not cases[i].get('cut')),
                       key=lambda i: (len(cases[i]['edits']), i))[:8]
        sub = os.path.join(B, '%s-cuts.bin' % spec_id)
        write_cases(sub, missions, [cases[i] for i in cands])
        seqs = [[int(x[1:]) for x in r[4:]] for r in run([nat, sub, 'visits'])]
        for i, seq in zip(cands, seqs):
            mi = missions[cases[i]['cm']]
            at = [k for k in range(1, len(seq)) if seq[k] not in seq[:k] and mi.kinds[seq[k]] == 'i']
            took = {}

            def takes(j):
                key = 'cut-%d-%d' % (i, at[j])
                trial[key] = dict(cases[i], cut=seq[at[j]])
                took[j] = c64_row_cover.trace(d, [key], argv)[key]
                return arc in took[j]
            if not at or not takes(len(at) - 1):
                continue
            lo, hi = -1, len(at) - 1        # takes(hi); not takes(lo)
            while hi - lo > 1:
                mid = (lo + hi) // 2
                lo, hi = (lo, mid) if takes(mid) else (mid, hi)
            k = at[hi]
            c = dict(cases[i], cut=seq[k], of=i, ran=k)
            idx = {p: b for b, p in enumerate(mi.points)}
            mask = 0
            for p in seq[:k]:
                mask |= 1 << idx[p]
            n = len(cases)
            if c64_size(spec, sym, mi.m, mask, group_text(sym, [(mi.m, mask, [n])], cases + [c],
                                                         rows + [['0', '0', '0', '0']])) is None:
                continue
            write_cases(sub, missions, [c])
            rr, rn = run([ref, *ROPES, sub]), run([nat, sub])
            if rr[0][1] != '0':
                refused += 1
                continue
            if rr[0][1:] != rn[0][1:]:
                differ += 1
                print('  C64 cut: case %d at %s: oracle %s, native %s' % (i, mi.cp.where(seq[k]), rr[0][1:], rn[0][1:]))
                continue
            cases.append(c)
            rows.append([str(n)] + rr[0][1:])
            visits.append(mask)
            arcs[n] = took[hi]
            want -= took[hi]
            made += 1
            break
    print('  C64 cuts: %d truncated rows (%d cuts the oracle does not stop at, %d differing), %d arcs left' % (
        made, refused, differ, len(want)))
    if differ:
        # The oracle and the native code stop at the same cut in the same
        # state, or the native code is wrong there.
        sys.exit('%s: %d C64 cuts differ from the original program' % (spec_id, differ))


C64_PICKS = []       # each group's cases, for the outcome cover


def _freeze_c64_rows(spec_id, spec, sym, missions, cases, rows, visits, chan=(), edges=None):
    """Groups of routines whose diagnostic fits below $c000, each with the
    spec's c64_per_point cases (default 2) per routine whose run stays in
    the group (fewest routines visited first). With 'c64_rows': 'cover', the
    cases are instead the fewest whose native runs take every branch arc all
    the cases take (tools/c64_row_cover.py), among the cases whose routines
    fit one diagnostic, plus the first of a routine none of them enters, and
    groups fill case by case. With 'cover' (edges given), the cases a greedy
    cover picks to reach every edge (branch outcome) the differential's
    cases reach, and the fewest-visit case of a routine the cover leaves
    out. Writes tests/<id>_c64_vectors.h and tools/mk/<id>.groups; returns
    the groups and the row cover's proof fields."""
    per = spec.get('c64_per_point', 2)
    # (Segment rows append cases of their own: these lists are the C64's.)
    cases, rows, visits = list(cases), list(rows), list(visits)
    for mi in missions:
        if mi.m not in spec['missions']:
            continue
        # (every KEEP word is defined, so a group drops the routines past 60 too)
        if mi.m in spec['missions']:
            KEEP_WORDS[spec['symbol'][mi.m]] = max(1, (len(mi.points) + 59) // 60)
    cover = spec.get('c64_rows') == 'cover'
    full, fits = None, {}

    def pick(arcs, seed=()):
        cost = lambda i: (bin(visits[i]).count('1'), len(cases[i]['edits']))
        chosen, okall = set(), set()
        for cm, mi in enumerate(missions):
            if mi.m not in spec['missions']:
                continue
            KEEP_WORDS[spec['symbol'][mi.m]] = (len(mi.points) + 59) // 60
            mine = [i for i in arcs if cases[i]['cm'] == cm and i not in chan]
            # Each routine set the cases visit, probed with its largest case;
            # when that one does not fit (edge cases add many edits), the
            # largest edit count that does, found by bisection.
            by = collections.defaultdict(list)
            for i in sorted(mine, key=lambda i: len(cases[i]['edits'])):
                by[visits[i]].append(i)
            limit = {}
            for v, ii in by.items():
                def fit(i, v=v):
                    if (v, i) not in fits:
                        fits[v, i] = c64_size(spec, sym, mi.m, v, group_text(sym, [(mi.m, v, [i])], cases, rows)) is not None
                    return fits[v, i]
                if fit(ii[-1]):
                    limit[v] = len(cases[ii[-1]]['edits'])
                elif fit(ii[0]):
                    lo, hi = 0, len(ii) - 1
                    while hi - lo > 1:
                        mid = (lo + hi) // 2
                        lo, hi = (mid, hi) if fit(ii[mid]) else (lo, mid)
                    limit[v] = len(cases[ii[lo]]['edits'])
            ok = {i: arcs[i] for i in mine if visits[i] in limit and len(cases[i]['edits']) <= limit[visits[i]]}
            s = [i for i in seed if i in mine]
            chosen |= set(c64_row_cover.cover({**ok, **{i: arcs[i] for i in s}}, cost, seed=s))
            okall.update(ok)
        return chosen, okall
    if cover:
        arcs = host_arcs(spec_id, spec, missions, cases)
        chosen, okall = pick(arcs)
        if spec.get('cover_stride', 1) > 1:
            # Rows for the arcs only untraced cases take (greedy, the rows
            # so far kept).
            full, more = wider_arcs(spec_id, missions, cases, arcs)
            if more:
                arcs = {**arcs, **more}
                chosen, okall = pick(arcs, sorted(chosen))
        if spec.get('c64_cuts'):
            # Arcs only runs too long for one diagnostic take: truncated rows.
            want = (full or frozenset().union(*arcs.values())) - frozenset().union(*(arcs[i] for i in chosen))
            if want:
                cases, rows, visits = list(cases), list(rows), list(visits)
                cut_cases(spec_id, spec, sym, missions, cases, rows, visits, arcs, want)
                chosen, okall = pick(arcs, sorted(chosen))
    groups = []
    for cm, mi in enumerate(missions):
        if mi.m not in spec['missions']:
            continue
        bit = {p: 1 << k for k, p in enumerate(mi.points)}
        need = {}
        fewest = lambda i: (bin(visits[i]).count('1'), len(cases[i]['edits']), i)
        for p in mi.points:
            mine = sorted((i for i, c in enumerate(cases) if c['cm'] == cm and c['point'] == p and i not in chan
                           and (not c.get('cut') or i in chosen)), key=fewest)
            if cover:
                # (With cover_stride, from the traced cases.)
                mine = [i for i in mine if i in arcs] or mine
            # (With both 'cover' and c64_rows 'cover', the edge cover starts
            # from the rows that take every gcov arc: gcov counts the ?:
            # outcomes clang folds to selects.)
            need[p] = ((mine[:1] + [i for i in mine[1:] if cover and i in chosen]) if edges is not None else
                       ([i for i in mine if i in chosen] or mine[:1]) if cover else mine[:per])
        if edges is not None:
            sets = edges[0]
            fits = {}

            def alone(i):
                v = visits[i]
                if v not in fits:
                    fits[v] = c64_size(spec, sym, mi.m, v, group_text(sym, [(mi.m, v, [i])], cases, rows)) is not None
                return fits[v]
            live = [i for i, c in enumerate(cases) if c['cm'] == cm and i not in chan]
            base = set().union(*(sets[i] for p in mi.points for i in need[p] if alone(i)))
            more, _ = greedy_cover(live, sets, base, key=fewest, accept=alone)
            for i in more:
                need[cases[i]['point']].append(i)
            segs = {}
            if spec.get('c64_segments'):
                segs = segment_rows(spec_id, spec, sym, missions, mi, cases, rows, visits, live, edges,
                                    base.union(*(sets[i] for i in more)))
        if edges is not None:
            mask, pick = 0, []
            # Case by case (the cover's cases already fit alone): a new
            # group when the next case's routines do not fit with the last.
            for p in mi.points:
                if not any(alone(i) for i in need[p]):
                    C64_HOST_ONLY.append('%s %s' % (mi.m, mi.cp.where(p)))
                for i in need[p]:
                    if not alone(i):
                        continue
                    trial, tp = mask | visits[i], pick + [i]
                    if mask and c64_size(spec, sym, mi.m, trial, group_text(sym, [(mi.m, trial, tp)], cases, rows)) is None:
                        groups.append((mi.m, mask, pick))
                        mask, pick = visits[i], [i]
                    else:
                        mask, pick = trial, tp
            if pick:
                groups.append((mi.m, mask, pick))
            # (one group per segment routine: the routine after it stays out)
            for p, ii in sorted(segs.items()):
                groups.append((mi.m, 1 << mi.points.index(p), ii))
            continue
        mask, pick = 0, []
        for p, part in (((p, [i]) for p in mi.points for i in need[p]) if cover else ((p, need[p]) for p in mi.points)):
            want = 0
            for i in part:
                want |= visits[i]
            if part and (mask & want) != want and c64_size(
                    spec, sym, mi.m, want, group_text(sym, [(mi.m, want, part)], cases, rows)) is None:
                # Even this routine's shortest runs reach more code than one
                # C64 program holds: it stays host-checked only.
                if '%s %s' % (mi.m, mi.cp.where(p)) not in C64_HOST_ONLY:
                    print('  C64: %s %s runs too much code for one program; host only' % (mi.m.upper(), mi.cp.where(p)))
                    C64_HOST_ONLY.append('%s %s' % (mi.m, mi.cp.where(p)))
                continue
            trial = mask | want
            tp = pick + part
            if c64_size(spec, sym, mi.m, trial, group_text(sym, [(mi.m, trial, tp)], cases, rows)) is None:
                # (With mask 0 the trial is the routine's own run.)
                if not mask or c64_size(spec, sym, mi.m, want, group_text(sym, [(mi.m, want, part)], cases, rows)) is None:
                    # The routine's whole run (with every routine it reaches)
                    # does not fit one diagnostic: its cases stay host-only.
                    if '%s %s' % (mi.m, mi.cp.where(p)) not in C64_HOST_ONLY:
                        C64_HOST_ONLY.append('%s %s' % (mi.m, mi.cp.where(p)))
                    continue
                groups.append((mi.m, mask, pick))
                mask, pick = want, list(part)
            else:
                mask, pick = trial, tp
        if mask:
            groups.append((mi.m, mask, pick))
    for m, mask, pick in groups:
        # Every case of a group whose run stays in it, so later routines'
        # closures don't drop earlier cases.
        assert all(visits[i] & ~mask == 0 for i in pick)
        if c64_size(spec, sym, m, mask, group_text(sym, [(m, mask, pick)], cases, rows)) is None:
            if os.environ.get('IRC_C64_DEBUG'):
                sys.stderr.write('group %s mask %x picks %s edits %s\n' % (m, mask, [cases[i]['point'] for i in pick], [len(cases[i]['edits']) for i in pick]))
            mi = [x for x in missions if x.m == m][0]
            sys.exit('a single routine group of %s does not fit the C64: %s' % (
                m, ', '.join(mi.cp.where(p) for k, p in enumerate(mi.points) if mask >> k & 1)))
    C64_PICKS[:] = [pick for m, mask, pick in groups]
    extra = {}
    if cover:
        picked = [i for m, mask, pick in groups for i in pick]
        # Cases that wait for an I/O channel are listed, not compared, on
        # the host too: their arcs are no C64 row's to take.
        arcs = {i: a for i, a in arcs.items() if i not in chan}
        # Rows the edge cover chose outside the cover_stride sample: trace
        # their arcs too, so the record counts what they take.
        late = [i for i in picked if i not in arcs and i not in chan]
        if late:
            arcs.update(trace_cases(spec_id, missions, cases, late))
        picked = [i for i in picked if i not in chan]
        if os.environ.get('IRC_C64_DEBUG'):
            for arc in sorted(c64_row_cover.missing(arcs, picked))[:3]:
                allw = [i for i in arcs if arc in arcs[i]]
                sys.stderr.write('%s: %d cases, %d ok, points %s\n' % (arc, len(allw), sum(i in okall for i in allw),
                                 sorted(set(missions[cases[i]['cm']].cp.where(cases[i]['point']) for i in allw))[:8]))
                who = allw[:5]
                sys.stderr.write('%s: cases %s chosen %s ok %s edits %s\n' % (
                    arc, who, [i in chosen for i in who], [i in okall for i in who], [len(cases[i]['edits']) for i in who]))
        c64_row_cover.check(arcs, picked, '%s C64 rows' % spec_id)
        extra = c64_row_cover.summary(arcs, picked)
        write_cover_record(spec_id, spec, arcs, picked, full, [
            {'row': i, 'case': cases[i]['of'], 'cut': missions[cases[i]['cm']].cp.where(cases[i]['cut']),
             'routines_run': cases[i]['ran']} for i in sorted(picked) if cases[i].get('cut')])
    open(os.path.join(R, 'tests/%s_c64_vectors.h' % sym), 'w').write(group_text(sym, groups, cases, rows))
    mk = ['# Generated by tools/interp_routine_differential.py --freeze: the C64',
          '# diagnostic groups (mission, KEEP mask) for tools/mk/%s.mk.' % sym.replace('_', '-')]
    v = sym.upper()
    mk.append('%s_GROUPS = %s' % (v, ' '.join(str(g) for g in range(1, len(groups) + 1))))
    for g, (m, mask, pick) in enumerate(groups, 1):
        mk.append('%s_GROUP_MISSION_%d = %s' % (v, g, m))
        mk.append('%s_GROUP_FLAGS_%d = -DIRC_GROUP=%d -DIRC_%s_ONLY %s' % (
            v, g, g, m.upper(), ' '.join(keep_flags(spec['symbol'][m], mask))))
    open(os.path.join(R, 'tools/mk/%s.groups' % sym.replace('_', '-')), 'w').write('\n'.join(mk) + '\n')
    # (case_ids: the differential's case of each row, a segment row's too)
    return [{'mission': m, 'keep': '0x%x' % mask, 'cases': len(pick),
             'case_ids': [cases[i].get('src', i) for i in pick]} for m, mask, pick in groups], extra



def freeze_c64(spec_id, spec, sym, missions, cases, rows, visits, covers=None, chan=(), edges=None):
    """Groups of routines whose diagnostic fits below $c000, each with the
    spec's c64_per_point cases (default 2) per routine whose run stays in
    the group (fewest edits first). With 'c64_rows': 'cover', the cases are
    instead the fewest whose native runs take every branch arc all the
    cases take (tools/c64_row_cover.py), among the cases whose routines fit
    one diagnostic, plus the first of a routine none of them enters, and
    groups fill case by case. Writes tests/<id>_c64_vectors.h and
    tools/mk/<id>.groups; returns the groups and the cover's proof fields."""
    per = spec.get('c64_per_point', 2)
    if covers is None:
        # (Files without whole-file basic code: the shared row choice.)
        return _freeze_c64_rows(spec_id, spec, sym, missions, cases, rows, visits, chan, edges)

    def select(excluded):
        groups = []
        for cm, mi in enumerate(missions):
            if mi.m not in spec['missions']:
                continue
            bit = {p: 1 << k for k, p in enumerate(mi.points)}
            KEEP_WORDS[spec['symbol'][mi.m]] = (len(mi.points) + 59) // 60
            bf = bank_flags(spec, mi, covers)
            if bf:
                GROUP_EXTRA[sym] = bf
            need = {}
            got = 0
            for p in mi.points:
                mine = sorted((i for i, c in enumerate(cases) if c['cm'] == cm and c['point'] == p and i not in chan
                               and i not in excluded),
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
                if covers is None:
                    trial = mask | want
                    tp = pick + need[p]
                    if mask and c64_size(spec, sym, mi.m, trial, group_text(sym, [(mi.m, trial, tp)], cases, rows), tp) is None:
                        if c64_size(spec, sym, mi.m, want, group_text(sym, [(mi.m, want, need[p])], cases, rows), need[p]) is None:
                            # The routine's whole run (with every routine it reaches)
                            # does not fit one diagnostic: its cases stay host-only.
                            C64_HOST_ONLY.append('%s %s' % (mi.m, mi.cp.where(p)))
                            continue
                        groups.append((mi.m, mask, pick))
                        mask, pick = want, list(need[p])
                    else:
                        mask, pick = trial, tp
                    continue
                if len(need[p]) > 1 and c64_size(spec, sym, mi.m, want,
                                                 group_text(sym, [(mi.m, want, need[p])], cases, rows), need[p]) is None:
                    units = [[i] for i in need[p]]
                for u in units:
                    want = 0
                    for i in u:
                        want |= visits[i]
                    trial = mask | want
                    tp = pick + u
                    if mask and c64_size(spec, sym, mi.m, trial, group_text(sym, [(mi.m, trial, tp)], cases, rows), tp) is None:
                        groups.append((mi.m, mask, pick))
                        mask, pick = want, list(u)
                    else:
                        mask, pick = trial, tp
            groups.append((mi.m, mask, pick))
        return groups

    # A case whose run alone does not fit one diagnostic (its routines,
    # bodies and banks) is left to the host; selection runs again without it.
    excluded = set()
    while True:
        groups = select(excluded)
        bad = None
        for m, mask, pick in groups:
            # Every case of a group whose run stays in it, so later routines'
            # closures don't drop earlier cases.
            assert all(visits[i] & ~mask == 0 for i in pick)
            if c64_size(spec, sym, m, mask, group_text(sym, [(m, mask, pick)], cases, rows), pick) is None:
                if os.environ.get('IRC_C64_DEBUG'):
                    sys.stderr.write('group %s mask %x picks %s edits %s\n' % (m, mask, [cases[i]['point'] for i in pick], [len(cases[i]['edits']) for i in pick]))
                if len(pick) != 1:
                    sys.exit('a group of %s does not fit the C64' % m)
                bad = pick[0]
                break
        if bad is None:
            break
        # (So do the cases of its routine whose runs reach every routine its
        # run reaches: one pass each, not one probe per case.)
        for i, c in enumerate(cases):
            if i not in excluded and c['cm'] == cases[bad]['cm'] and c['point'] == cases[bad]['point'] \
                    and visits[bad] & ~visits[i] == 0 and len(c['edits']) >= len(cases[bad]['edits']):
                sys.stderr.write('case %d (%s) does not fit one C64 diagnostic: host only\n' % (i, c['point']))
                excluded.add(i)
                C64_TOO_BIG.append(i)
    open(os.path.join(R, 'tests/%s_c64_vectors.h' % sym), 'w').write(group_text(sym, groups, cases, rows))
    mk = ['# Generated by tools/interp_routine_differential.py --freeze: the C64',
          '# diagnostic groups (mission, KEEP mask) for tools/mk/%s.mk.' % sym.replace('_', '-')]
    v = sym.upper()
    mk.append('%s_GROUPS = %s' % (v, ' '.join(str(g) for g in range(1, len(groups) + 1))))
    for g, (m, mask, pick) in enumerate(groups, 1):
        mk.append('%s_GROUP_MISSION_%d = %s' % (v, g, m))
        mk.append('%s_GROUP_FLAGS_%d = -DIRC_GROUP=%d -DIRC_%s_ONLY %s' % (
            v, g, g, m.upper(), ' '.join(keep_flags(spec['symbol'][m], mask)
                                         + GROUP_EXTRA.get(sym, lambda mask, pick: [])(mask, pick))))
    open(os.path.join(R, 'tools/mk/%s.groups' % sym.replace('_', '-')), 'w').write('\n'.join(mk) + '\n')
    return [{'mission': m, 'keep': '0x%x' % mask, 'cases': len(pick), 'case_ids': list(pick)} for m, mask, pick in groups], {}


if __name__ == '__main__':
    main()
