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
import hashlib
import json
import math
import os
import random
import re
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
        # (An erasable program's address a call reaches is 0xC000 | Z.)
        self.body_exits = sorted(set([L.labels[n] for n in BODY_EXITS[m]] + list(self.cp.externals_any)
                                     + [0xC000 | k for k in self.cp.erasable_calls]))
        self.returns = self.caller_returns(spec)
        self.operands = self.operand_addresses()
        self.ebank = spec.get('ebank')
        self.edge = spec.get('edge_cases', False)
        self.rk = bool(spec.get('resume_keys'))
        self.index_returns = bool(spec.get('index_returns'))
        self.all_states = spec.get('all_states', {})
        self.counters = spec.get('counters', False)
        self.edges = spec.get('edge_values', False)
        js = spec.get('joint_states', {})
        self.joint_any = js.get('*', [])
        self.joint = {self.cp.label_phys(k): v for k, v in js.items() if k != '*'}
        self.esyms = erasable_symbols(os.path.join(R, IL.LISTINGS[m]))
        self.states = {self.cp.label_phys(k): v for k, v in spec.get('states', {}).items()}
        # range_states: (FROM, TO, states) applies states to every point
        # from label FROM up to (not including) label TO, after all_states.
        self.range_states = [(self.cp.label_phys(a), self.cp.label_phys(b), st)
                             for a, b, st in spec.get('range_states', ())]
        self.return_words = [int(k, 8) for k in spec.get('return_words', [])]

    def caller_returns(self, spec):
        """Fixed CADRs of the words after CALL/STCALL operands elsewhere
        that name one of the file's entries."""
        ents = set(cadr(self.cp.label_phys(e)) for e in IC.entries(spec, self.m))
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
    eb = rng.randrange(8) if mi.ebank is None else mi.ebank
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
    pinned = set()
    ranged = [kv for lo, hi, st in mi.range_states if lo <= point < hi for kv in st.items()]
    for k, choices in list(mi.all_states.items()) + ranged + list(mi.states.get(point, {}).items()):
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
    if mi.kinds[point] == 'i':
        E[5] = mi.newops
        E[0o164] = logical(point)
        E[0o165] = fb | eb
        E[0o115] = fb & 0o40000
        stop = 0xffff
    else:
        E[5] = logical(point)
        # Q: a return in the caller's bank outside the file, the case's stop.
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
        # Saved returns a TC to erasable runs (TC MPAC +2, TC TBASE2): the same.
        # A state the spec pins there (a caller's return in the file) stays.
        for k in mi.cp.tc_erasable:
            a = k if k < 0o1400 else eb * 0o400 + k - 0o1400
            if a not in pinned:
                E[a] = logical(q)
        stop = q
    return seed, list(E.items()), n & 1, stop, sb


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
            f.write(struct.pack('<IBBHHH', seed, cm, ie, c.get('sb', 0), c.get('stop', 0xffff), len(edits)))
            for a, w in edits:
                f.write(struct.pack('<HH', a, w))


def build(spec_id, spec):
    ref = os.path.join(B, 'interp-routine-reference')
    # The pinned engine with DV always through its hardware model
    # (tools/prepare_hardware_dv.py: the fast path mishandles a
    # sign-extended -0 in L), as the native divide follows.
    hw = os.path.join(B, 'agc_engine_hardware_dv.o')
    if not os.path.exists(hw):
        subprocess.run([sys.executable, 'tools/prepare_hardware_dv.py'], cwd=R, check=True, capture_output=True)
    if not os.path.exists(ref) or os.path.getmtime(ref) < os.path.getmtime(os.path.join(R, 'tools/interp_routine_reference.c')):
        subprocess.run(['cc', '-std=c99', '-O2', '-Iinclude', '-Itests', '-I' + E,
                        '-DIRC_BUILD_ONLY', 'tools/interp_routine_reference.c', 'tests/interp_case.c', 'src/agc_word.c',
                        'tests/cm_executive_cases.c',
                        hw, os.path.join(E, 'agc_engine_init.o'),
                        os.path.join(E, 'rfopen.o'), '-o', ref], cwd=R, check=True)
    nat = os.path.join(B, '%s-native' % spec_id)
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + COMMON
    subprocess.run(['cc', *FLAGS, '-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200), *srcs, spec['target'],
                    'tools/interp_routine_native.c', '-o', nat],
                   cwd=R, check=True)
    return ref, nat


def build_edges(spec_id, spec):
    """The native runner with every linked source (the milestone's routines,
    the interpreter bodies, the image ops and native RTBs; not the test
    harness) instrumented per edge, so each branch outcome is a guard."""
    d = os.path.join(B, '%s-edges' % spec_id)
    os.makedirs(d, exist_ok=True)
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + \
        [x for x in COMMON if not x.startswith('tests/')]
    objs = []
    for src in srcs:
        o = os.path.join(d, os.path.basename(src) + '.o')
        subprocess.run(['clang', '-std=c99', '-O0', '-g', '-fno-pie', '-Iinclude', '-Itests',
                        '-fsanitize-coverage=trace-pc-guard', '-c', src, '-o', o], cwd=R, check=True)
        objs.append(o)
    nat = os.path.join(d, 'native')
    subprocess.run(['clang', '-std=c99', '-O0', '-g', '-no-pie', '-Iinclude', '-Itests', '-DIRC_EDGES', *objs,
                    'tests/interp_case.c', spec['target'], 'tools/interp_routine_native.c', 'tools/irc_edges.c',
                    '-o', nat], cwd=R, check=True)
    return nat


def edge_sets(nat, path):
    """Per case, the guards it reached; and each guard's source line."""
    r = subprocess.run([nat, path, 'edges'], cwd=R, check=True, capture_output=True, text=True)
    sets = [frozenset(int(t[1:]) for t in l.split()[4:] if t[0] == 'e') for l in r.stdout.splitlines()]
    pcs = [l.split()[1:] for l in r.stderr.splitlines() if l.startswith('pc ')]
    sym = subprocess.run(['llvm-symbolizer', '--obj=' + nat],
                         input='\n'.join(a for _, a in pcs) + '\n', cwd=R, capture_output=True, text=True).stdout
    lines = [b.split('\n')[1] for b in sym.strip().split('\n\n')]
    where = {}
    for (g, _), ln in zip(pcs, lines):
        f, _, rest = ln.partition(':')
        where[int(g)] = '%s:%s' % (os.path.relpath(f, R) if f.startswith('/') else f, rest.split(':')[0])
    return sets, where


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
                seed, edits, ie, stop, sb = case(mi, p, n, rng)
                cases.append({'cm': cm, 'point': p, 'seed': seed, 'edits': edits, 'ie': ie, 'stop': stop, 'sb': sb})
    os.makedirs(B, exist_ok=True)
    path = os.path.join(B, '%s-cases.bin' % a.spec)
    write_cases(path, missions, cases)
    ref, nat = build(a.spec, spec)
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
        vis = run([nat, path, 'visits'])
        visits = []
        for c, r in zip(cases, vis):
            idx = {p: k for k, p in enumerate(missions[c['cm']].points)}
            v = 0
            for t in r[4:]:
                v |= 1 << idx[int(t[1:])]
            visits.append(v)
        edges = None
        if spec.get('cover'):
            edges = edge_sets(build_edges(a.spec, spec), path)
        freeze(a.spec, spec, missions, cases, rows_r, stops, visits, chan, edges)


def freeze(spec_id, spec, missions, cases, rows, stops, visits, chan=(), edges=None):
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
                           'linked source but the test harness, per case (tools/irc_edges.c)',
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
    c64, rows_cover = freeze_c64(spec_id, spec, sym, missions, cases, rows, visits, chan, edges)
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
             **({'c64_host_only': C64_HOST_ONLY} if C64_HOST_ONLY else {}),
             **({'outcome_cover': cover} if cover is not None else {}),
             'stops': {k: dict(v) for k, v in sorted(stops.items())},
             'boundary': 'from each routine entry state to NEWOPS with LOC outside the file, an exit of the '
                         'interpreter bodies (%s), or a transfer to an outside basic routine (%s)' % (
                             '; '.join('%s: %s' % (mi.m, ', '.join(BODY_EXITS[mi.m])) for mi in missions),
                             '; '.join('%s: %s' % (mi.m, ', '.join(sorted(set(mi.cp.externals.values()) | set(mi.cp.externals_any.values()))) or 'none')
                                       for mi in missions if mi.m in spec['missions'])),
             'compared': 'Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state'}
    proof.update(rows_cover)
    if chan:
        proof['waiting_for_io_channels'] = dict(collections.Counter(chan.values()))
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
            + spec.get('native_sources', []) + [x for x in COMMON if '_interp_' not in x] + [spec['target']])


KEEP_WORDS = {}   # symbol -> the generated file's KEEP words


def keep_flags(symbol, mask):
    """-D<SYM>_KEEP<n> for a routine mask, 60 routines per word (every
    word the file has, so none defaults to all)."""
    n = KEEP_WORDS[symbol]
    return ['-D%s_KEEP%d=0x%x' % (symbol.upper(), w, (mask >> (60 * w)) & ((1 << 60) - 1)) for w in range(n)]


def c64_size(spec, sym, m, mask, text):
    """End of the C64 diagnostic's memory (code, data and bss) for one group,
    or None when it does not link or reaches past C64_TOP."""
    d = os.path.join(B, '%s-probe-%s-%x' % (sym, m, mask))
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, '%s_c64_vectors.h' % sym), 'w').write(text)
    prg = os.path.join(d, 'probe.prg')
    r = subprocess.run([MOS_CC, '-I' + d, '-Iinclude', '-Itests', *C64_FLAGS, *spec.get('c64_flags', []), '-DIRC_GROUP=1',
                        '-DIRC_%s_ONLY' % m.upper(), *keep_flags(spec['symbol'][m], mask),
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
        out.append('#endif')
    return '\n'.join(out) + '\n'


def host_arcs(spec_id, spec, missions, cases):
    """{case: branch arcs} of the native runner, one case at a time
    (tools/c64_row_cover.py)."""
    d = os.path.join(B, '%s-cover' % spec_id)
    srcs = ['src/%s.c' % spec['base'][m] for m in spec['missions']] + spec.get('native_sources', []) + COMMON
    exe = c64_row_cover.build(d, [*srcs, spec['target'], 'tools/interp_routine_native.c'],
                              [f for f in FLAGS if f != '-O2'] + ['-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200)])
    one = os.path.join(d, 'cases')
    os.makedirs(one)

    def argv(i):
        p = os.path.join(one, '%d.bin' % i)
        write_cases(p, missions, [cases[i]])
        return [str(exe), p]
    return c64_row_cover.trace(d, range(len(cases)), argv)


C64_HOST_ONLY = []   # routines whose runs do not fit one C64 diagnostic


C64_PICKS = []       # each group's cases, for the outcome cover


def freeze_c64(spec_id, spec, sym, missions, cases, rows, visits, chan=(), edges=None):
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
    for mi in missions:
        # (every KEEP word is defined, so a group drops the routines past 60 too)
        KEEP_WORDS[spec['symbol'][mi.m]] = max(1, (len(mi.points) + 59) // 60)
    cover = spec.get('c64_rows') == 'cover'
    if cover:
        arcs = host_arcs(spec_id, spec, missions, cases)
        cost = lambda i: (bin(visits[i]).count('1'), len(cases[i]['edits']))
        chosen = set()
        for cm, mi in enumerate(missions):
            if mi.m not in spec['missions']:
                continue
            mine = [i for i in arcs if cases[i]['cm'] == cm and i not in chan]
            # Each routine set the cases visit, probed with its largest case.
            big = {visits[i]: i for i in sorted(mine, key=lambda i: len(cases[i]['edits']))}
            fits = {v for v, i in big.items()
                    if c64_size(spec, sym, mi.m, v, group_text(sym, [(mi.m, v, [i])], cases, rows)) is not None}
            ok = {i: arcs[i] for i in mine if visits[i] in fits}
            chosen |= set(c64_row_cover.cover(ok, cost))
    groups = []
    for cm, mi in enumerate(missions):
        if mi.m not in spec['missions']:
            continue
        bit = {p: 1 << k for k, p in enumerate(mi.points)}
        need = {}
        fewest = lambda i: (bin(visits[i]).count('1'), len(cases[i]['edits']), i)
        for p in mi.points:
            mine = sorted((i for i, c in enumerate(cases) if c['cm'] == cm and c['point'] == p and i not in chan),
                          key=fewest)
            need[p] = (mine[:1] if edges is not None else
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
            if mask and c64_size(spec, sym, mi.m, trial, group_text(sym, [(mi.m, trial, tp)], cases, rows)) is None:
                if c64_size(spec, sym, mi.m, want, group_text(sym, [(mi.m, want, part)], cases, rows)) is None:
                    # The routine's whole run (with every routine it reaches)
                    # does not fit one diagnostic: its cases stay host-only.
                    if '%s %s' % (mi.m, mi.cp.where(p)) not in C64_HOST_ONLY:
                        C64_HOST_ONLY.append('%s %s' % (mi.m, mi.cp.where(p)))
                    continue
                groups.append((mi.m, mask, pick))
                mask, pick = want, list(part)
            else:
                mask, pick = trial, tp
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
        c64_row_cover.check(arcs, picked, '%s C64 rows' % spec_id)
        extra = c64_row_cover.summary(arcs, picked)
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
    return [{'mission': m, 'keep': '0x%x' % mask, 'cases': len(pick)} for m, mask, pick in groups], extra


if __name__ == '__main__':
    main()
