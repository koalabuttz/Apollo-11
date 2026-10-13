#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile a file's interpretive code (and the short basic sequences between
EXIT and TC INTPRET) from the yaYUL listing into native routines of the
control-transfer ABI (include/agc_transfer.h).

Each order pair becomes the compiled lm_interp_steps/lm_interp_orders path
(cmi_ names for Comanche055) with its literal order and operand words, as in
src/lm_conic_timerad.c; nothing reads the rope at run time. A routine starts
at every entry the spec names, every label a branch or CALL in the file
reaches, every CALL return point and every resume point after an outside
call, and runs to the next control transfer or the next routine start. An
interpretive routine is keyed by the physical LOC at NEWOPS, a basic one by
the physical Z.

Usage: interp_compile.py <spec> [--check]
  writes src/<base>.c and include/<base>.h (or compares them with --check)
  for each mission of the spec in tools/interp_specs.py."""
import os
import re
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import interp_listing as IL
import interp_specs
import basic_compile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# INDJUMP names of the address orders, and their bodies: (C body, takes k).
ADDR = {
    'VLOAD': ('vload', True), 'DLOAD': ('dload_op', True), 'PDVL': ('pdvl', True),
    'PDDL': ('pddl_op', True), 'TAD': ('tad', True), 'SIGN': ('sign', True),
    'VXSC': ('vxsc', True), 'DMPR': ('dmpr', True), 'DDV': ('ddv', True),
    'BDDV': ('bddv', True), 'VAD': ('vad', True), 'VSU': ('vsu', True),
    'BVSU': ('bvsu', True), 'DOT': ('dot', True), 'VXV': ('vxv', True),
    'DSU': ('dsu', True), 'BDSU': ('bdsu', True), 'DAD': ('dad', True),
    'DMP1': ('dmp', True), 'TLOAD': ('tload', False), 'SLOAD': ('sload', True),
    'TSLC': ('tslc', False), 'GSHIFT': ('gshift', False), 'SETPD': ('setpd', False),
    'SSP': (None, False), 'MXV': ('mxv', True), 'VXM': ('vxm', True), 'V/SC': ('vsc', True), 'VPROJ': ('vproj', True),
    'CGOTO': (None, False),
}
# Fixed operands longer than a vector (the words a K array holds).
K_WORDS = {'MXV': 18, 'VXM': 18}
# Input channels a 'channel_inputs' spec reads from the caller's latches.
INPUT_CHANNELS = {0o3: '3', 0o4: '4', 0o16: '16', 0o30: '30', 0o31: '31', 0o32: '32', 0o33: '33'}
ADDR_ALIAS = {'DMP': 'DMP1', 'NORM': 'TSLC', 'SL': 'GSHIFT', 'SR': 'GSHIFT', 'SLR': 'GSHIFT',
              'SRR': 'GSHIFT', 'VSL': 'GSHIFT', 'VSR': 'GSHIFT'}
MISC = {'AXT': 'AXT', 'AXC': 'AXC', 'LXA': 'LXA', 'LXC': 'LXC', 'SXA': 'SXA', 'XCHX': 'XCHX', 'INCR': 'INCR',
        'TIX': 'TIX', 'XAD': 'XAD', 'XSU': 'XSU', 'GOTO': 'BZE/GOTO', 'BZE': 'BZE/GOTO',
        'BPL': 'BPL/BMN', 'BMN': 'BPL/BMN', 'RTB': 'RTB/BHIZ', 'BHIZ': 'RTB/BHIZ',
        'CALL': 'CALL/ITA', 'ITA': 'CALL/ITA', 'STQ': 'CALL/ITA', 'BOV': 'BOV(B)', 'BOVB': 'BOV(B)'}
INDEXED_MISC = ('AXT', 'AXC', 'LXA', 'LXC', 'SXA', 'XCHX', 'INCR', 'TIX', 'XAD', 'XSU')
SWITCH_ALIAS = {'CLR': 'CLEAR', 'BOF': 'BOFF', 'BOFCLR': 'BOFFCLR', 'BOFSET': 'BOFFSET', 'BOFINV': 'BOFFINV'}
SWITCH_BRANCH = ('BON', 'BOFF', 'BOFFCLR', 'BOFFSET', 'BOFFINV', 'BONCLR', 'BOFCLR', 'BONSET', 'BOFSET', 'BONINV', 'BOFINV',
                 'SETGO', 'CLRGO', 'INVGO')
SWITCH_PLAIN = ('SET', 'CLEAR', 'INVERT')
UNARY = {'SQRT': 'SQRT', 'SIN': 'SINE', 'COS': 'COSINE', 'SINE': 'SINE', 'COSINE': 'COSINE', 'ASIN': 'ARCSIN', 'ACOS': 'ARCCOS', 'ARCSIN': 'ARCSIN', 'ARCCOS': 'ARCCOS',
         'DSQ': 'DSQ', 'ROUND': 'ROUND', 'DCOMP': 'COMP', 'VCOMP': 'COMP', 'VDEF': 'VDEF',
         'UNIT': 'UNIT', 'ABVAL': 'ABVALABS', 'ABS': 'ABVALABS', 'VSQ': 'VSQ', 'STADR': 'STADR',
         'RVQ': 'RVQ', 'PUSH': 'PUSH'}
SHORTS = set(['SR%d' % i for i in range(1, 5)] + ['SL%d' % i for i in range(1, 5)] +
             ['SR%dR' % i for i in range(1, 5)] + ['SL%dR' % i for i in range(1, 5)] +
             ['VSL%d' % i for i in range(1, 9)] + ['VSR%d' % i for i in range(1, 9)])
STORES = ('STORE', 'STODL', 'STOVL', 'STCALL', 'STODL*', 'STOVL*')


DATA_OPS = ('DEC', '2DEC', 'OCT', '2OCT', 'CADR', '2CADR', 'ADRES', 'ECADR', 'GENADR', 'REMADR',
            'BBCON', '2FCADR', '2BCADR', 'DEC*', '2DEC*', 'VN', 'FCADR', 'BBCON*', '=', 'EQUALS', 'MM', 'NV')


class CompileError(Exception):
    pass


def entries(spec, m):
    """The spec's entry labels for mission m: a list shared by both
    missions, or a {mission: list} dict."""
    e = spec['entries']
    return e[m] if isinstance(e, dict) else e


def split_op(op):
    """(base mnemonic, starred, index tag) of an order column entry."""
    tag = 0
    if op == 'CALRB':
        op = 'CALL'     # CALRB assembles as CALL
    if op.endswith(',1') or op.endswith(',2'):
        tag = int(op[-1])
        op = op[:-2]
    star = op.endswith('*')
    op = op[:-1] if star else op
    # CALRB ("return in basic") is CALL's opcode under another name.
    return ('CALL' if op == 'CALRB' else op), star, tag


def classify(op):
    base, star, tag = split_op(op)
    base = SWITCH_ALIAS.get(base, base)
    if base == 'EXIT':
        return 'exit', base
    if base in SHORTS and not star:
        return 'short', base
    if base in UNARY and not star:
        return 'unary', UNARY[base]
    if base in MISC:
        return 'misc', MISC[base]
    if base in SWITCH_BRANCH:
        return 'switch_branch', base
    if base in SWITCH_PLAIN:
        return 'switch', base
    name = ADDR_ALIAS.get(base, base)
    if name in ADDR:
        return 'addr', name
    raise CompileError('unknown interpretive order %r' % op)


def is_interp_op(op):
    if not op:
        return False
    if op in STORES:
        return True
    try:
        classify(op)
        return True
    except CompileError:
        return False


class Compiler:
    def __init__(self, spec, mission):
        self.spec = spec
        self.m = mission
        self.L = IL.load(mission)
        self.L.rope_fallback = spec.get('rope_fallback', False)
        if spec.get('rope_bin'):
            self.L.rope = {**self.L.bin_words, **self.L.rope}
        self.p = 'lmi_' if mission == 'lm' else 'cmi_'
        self.lines = self.L.file_lines(spec['file'][mission])
        if not self.lines:
            raise CompileError('no lines for %s' % spec['file'][mission])
        self.code = {l.phys: l for l in self.lines}
        self.words = set(l.phys + i for l in self.lines for i in range(len(l.words)))
        # 'outside': {'LABEL': words} leaves code the compiler can't model
        # yet (VNPOOH's XCH FBANK) to the original: a transfer there leaves
        # the file, and its words are untranslated (the spec's 'partial').
        for lab_, n_ in spec.get('outside', {}).items():
            p_ = self.L.labels[lab_]
            self.words -= set(range(p_, p_ + n_))
        # (Only a file wholly in banks 040-043: a file with code in 030-033 as
        # well, as P40-P47 with DAPFIG in 042, keeps FB 030-033 as banks 030-033.)
        banked = [l.phys for l in self.lines if l.phys is not None and l.phys >= 4 * 1024]
        self.superbank = bool(banked) and all(p >= 0o40 * 1024 for p in banked)
        self.high_banks = any(l.phys >= 0o30 * 1024 for l in self.lines)
        # resume_keys: every word after a DANZIG is a routine of its own, so
        # a job whose DANZIG finds NEWJOB positive stops at CHANG2 and the
        # Executive's resume (NEWOPS at LOC +1) has its key.
        self.rk = bool(spec.get('resume_keys'))
        # 'any_fixloc': basic routines that never name FIXLOC take any FIXLOC
        # (tasks and NOVAC jobs call them too), not only a VAC area.
        self.any_fixloc = bool(spec.get('any_fixloc'))
        self.sload_k = False
        self.uses_channels = False
        self.kx = {}           # index_guard fixed tables: window start -> (words, base)
        self.kx_used = False
        # erasable_calls: erasable addresses (logical) called as programs.
        self.erasable_calls = set(spec.get('erasable_calls', []))
        # idle: loop heads where the code spins until hardware moves an
        # input (CHECKG1 waits for a PIPA pulse). Reaching one ends the run
        # with EXEC_IDLE and Z there; its key runs the loop once more.
        self.idle = set(self.label_phys(x) for x in spec.get('idle', []))
        self.lo = min(self.words)
        self.hi = max(self.words)
        # The file's fixed words as contiguous physical ranges.
        self.ranges = []
        for q in sorted(self.words):
            if self.ranges and self.ranges[-1][1] == q - 1:
                self.ranges[-1][1] = q
            else:
                self.ranges.append([q, q])
        self.T = {}
        self.T.update(self.L.jump_table('INDJUMP', 32))
        self.T.update(self.L.jump_table('MISCJUMP', 16))
        self.T.update(self.L.jump_table('UNAJUMP', 15))
        lab = self.L.labels
        self.newops = 0o4000 + lab['NEWOPS'] - 2048
        self.intpret = 0o4000 + lab['INTPRET'] - 2048
        self.shortt = 0o2000 + lab['SHORTT']
        self.dloadstar = self.L.word(lab['DLOAD*'])
        self.vloadstar = 0o40001
        self.starts = {}       # phys -> ('i'|'b', reason)
        self.consts = {}       # phys -> label
        self.ktabs = {}        # indexed fixed table phys -> (words, label)
        self.lists = {}        # CGOTO CADR list phys -> length
        self.const_len = {}    # phys -> words in its K array
        self.tc_erasable = set()  # erasable words a TC runs as instructions
        self.basic_fixed = set()  # rope words basic instructions read as operands
        self.cadr_erasable = set()
        self.ktables = {}      # phys -> (words, label, range) for indexed fixed operands
        self.externals = {}    # outside basic address (physical) -> name
        self.externals_any = {}  # the same, reached through interpreter code (BOVB)
        self.rtb = spec.get('rtb', {})
        # INCR Q anywhere in the file: in-file subroutine calls may return
        # one or two words past their return address.
        self.incr_q = any(l.op == 'INCR' and l.words == [0o24002] for l in self.lines)
        self.basic = basic_compile.Basic(self) if spec.get('basic') == 'full' else None
        if self.basic and spec.get('index_guard'):
            raise CompileError('index_guard and basic both name kx and KX tables')

    # ---- addresses -------------------------------------------------------
    def word(self, phys):
        ln = self.code.get(phys)
        if ln is None:
            # second word of a two-word line
            return self.L.word(phys)
        return ln.words[0]

    def label_phys(self, lab):
        """Physical address of 'LABEL' or 'LABEL+n' / 'LABEL-n'."""
        import re
        m = re.match(r'^(.*?)([+-]\d+)?$', lab)
        base = self.L.labels.get(m.group(1))
        return None if base is None else base + int(m.group(2) or 0)

    def name(self, phys):
        return self.L.label_at(phys) or ''

    def where(self, phys):
        return '%02o,%04o' % (phys // 1024, 0o2000 + phys % 1024)

    def cadr_phys(self, w):
        """Physical address of a fixed CADR word (as GOTO forms it). A file
        in banks 040-043 runs with superbank 4, under which FB 030-033 are
        banks 040-043."""
        b = (w >> 10) & 0o37
        # (In a file with code both in 030-037 and 040-043, the bank of the
        # routine being compiled decides: FB 30-37 is a superbank bank only
        # from code that runs with the superbank bit.)
        ctx = getattr(self, 'cur_start', None)
        if b >= 0o30 and (self.superbank if ctx is None else ctx // 1024 >= 0o40):
            b += 8
        return b * 1024 + (w & 0o1777)

    def in_file(self, phys):
        return phys in self.words

    def fixed_operand(self, w, bank):
        """Physical address of an address-order operand word naming fixed
        memory, or None for erasable."""
        a = (w - 1) & 0o77777
        if a < 0o4000:
            return None
        # The half-memory bit is FB's; in banks 040-043 FB is the bank less
        # 010, with superbank 4 (channel 7 bit 0100) adding it back.
        fbb = bank - 8 if bank >= 0o40 else bank
        b = (a >> 10) | (fbb & 0o20)
        if bank >= 0o40 and b >= 0o30:
            b += 8
        return b * 1024 + (a & 0o1777)

    def add_start(self, phys, kind, why):
        if phys in self.starts:
            if self.starts[phys][0] != kind:
                raise CompileError('%s is both interpretive and basic' % self.where(phys))
            return
        self.starts[phys] = (kind, why)
        self.work.append(phys)

    # ---- decoding --------------------------------------------------------
    def decode(self):
        self.work = []
        for phys in sorted(self.idle):
            self.add_start(phys, 'b', 'idle point')
            self.externals_any[phys] = 'idle point'
        for t, why in self.external_entries() if '@external' in entries(self.spec, self.m) else []:
            self.add_start(t, 'i' if is_interp_op(self.code[t].op) else 'b', why)
        for lab in entries(self.spec, self.m):
            if lab == '@external':
                continue
            phys = self.label_phys(lab)
            if phys is None:
                raise CompileError('entry %s not found' % lab)
            ln = self.code[phys]
            self.add_start(phys, 'i' if is_interp_op(ln.op) else 'b', 'entry ' + lab)
        if self.basic:
            for t, why in self.basic.erasable_targets():
                self.add_start(t, 'b', why)
            # The file's own CADR constants that name its code.
            for ln in self.lines:
                if ln.op in ('2CADR', 'CADR', 'GENADR', 'FCADR', '2FCADR') and ln.operand:
                    t = self.L.labels.get(ln.operand.split()[0])
                    if (t is not None and self.in_file(t) and t in self.code and len(ln.operand.split()) == 1
                            and self.code[t].op not in self.DATA_OPS):
                        self.add_start(t, 'i' if is_interp_op(self.code[t].op) else 'b', ln.op + ' ' + ln.operand)
        self.blocks = {}
        self.owner = {}
        while self.work:
            phys = self.work.pop()
            kind = self.starts[phys][0]
            if kind == 'b' and self.basic:
                self.basic.enter(phys)
                self.blocks[phys] = None
            else:
                self.blocks[phys] = self.interp_block(phys) if kind == 'i' else self.basic_block(phys)
            if not self.work and self.spec.get('resume_orphans'):
                # Basic words nothing in the file reaches are entered from
                # outside (display and restart returns): routines of their own.
                # (The words of the spec's native_ranges run natively.)
                native = set(self.label_phys(lab) + i for lab, (n, _) in self.spec.get('native_ranges', {}).items()
                             if self.label_phys(lab) is not None for i in range(n))
                for q in sorted(self.code):
                    if q in native:
                        continue
                    prev = self.code.get(q - 1)
                    if prev is not None and prev.op in ('EXTEND', 'INDEX', 'NDX'):
                        continue    # (a prefixed word is its prefix's)
                    if self.is_basic(q) and self.in_file(q) and q not in self.owner and q not in self.starts and not (
                            self.basic and (q in self.basic.words or q in self.basic.operands)):
                        self.add_start(q, 'b', 'resumed from outside')
                        break
                    # With 'all', also an order word nothing reaches (the EXIT
                    # after LITTLSUB's GOTO QPLAC), entered at NEWOPS.
                    ln = self.code[q]
                    if self.spec.get('resume_orphans') == 'all' and ln.op and is_interp_op(ln.op) \
                            and q not in self.owner and q not in self.starts:
                        self.add_start(q, 'i', 'resumed from outside')
                        break
        # Second pass: a block stops where another starts.
        self.owner = {}        # word phys -> routine phys that compiles it
        for phys in list(self.blocks):
            kind = self.starts[phys][0]
            if kind == 'b' and self.basic:
                continue
            self.blocks[phys] = self.interp_block(phys) if kind == 'i' else self.basic_block(phys)
        if self.basic:
            for q in self.basic.words:
                self.owner.setdefault(q, 'basic')
            for q in self.basic.operands:
                self.owner.setdefault(q, 'basic')
        # Basic routines the spec runs by hand (its native_ranges) that other
        # files reach directly: 'native_entries' {label: {mission: C call
        # returning RTB_OK}}, registered as routines of their own.
        for lab, calls in self.spec.get('native_entries', {}).items():
            phys = self.label_phys(lab)
            if phys is None or not self.is_basic(phys):
                raise CompileError('native entry %s is not a basic word of the file' % lab)
            self.starts[phys] = ('b', 'native entry ' + lab)
            # (they end TC DANZIG, run here as a pair's end: a left-hand RTB,
            # EDOP holding its pair's right order, is out of scope)
            self.blocks[phys] = ['if(W[023])return EXEC_OUT_OF_SCOPE;',
                                 'if(%s!=RTB_OK)return EXEC_OUT_OF_SCOPE;' % calls[self.m],
                                 '(void)next(s);return EXEC_OK; /* %s, DANZIG */' % lab]
        # Routines that refuse at an I/O channel instruction on some path.
        self.channel_blocks = set(p for p, b in self.blocks.items() if b and any('I/O channel */' in x for x in b))

    DATA_OPS = ('DEC', '2DEC', 'OCT', 'OCTAL', 'CADR', '2CADR', 'FCADR', '2FCADR', 'GENADR', 'ECADR', 'BBCON',
                'ADRES', 'REMADR', 'DEC*', '2DEC*', 'MM', 'VN', 'NV', '=', 'EQUALS', 'ERASE', 'BANK', 'SETLOC',
                'BLOCK', 'COUNT*', 'COUNT', 'EBANK=', 'SBANK=')

    def external_entries(self):
        """Code words of the file that another file's operand names (a label,
        or label +n): the points reached from outside it (spec entry
        '@external')."""
        mine = {v: k for k, v in self.L.labels.items() if self.in_file(v) and v in self.code
                and self.code[v].op not in self.DATA_OPS}
        found = {}
        for ln in self.L.lines:
            if ln.src == self.spec['file'][self.m] or not ln.operand:
                continue
            t = ln.operand.replace(' ', '').split(',')[0]
            if t[:1] in '+-':
                continue    # relative to the line itself (TC +5), not a label
            off = 0
            for sep in ('+', '-') if t not in self.L.labels else ():   # (labels like PRE-HUNT)
                if sep in t[1:]:
                    i = t.index(sep, 1)
                    try:
                        off = int(t[i + 1:]) * (1 if sep == '+' else -1)
                    except ValueError:
                        off = None
                    t = t[:i]
                    break
            if off is None or t not in self.L.labels:
                continue
            q = self.L.labels[t] + off
            if q in mine or (off and q in self.code and self.L.labels[t] in mine
                             and self.code[q].op not in self.DATA_OPS):
                found.setdefault(q, 'entry %s (%s %s %s)' % (self.L.label_at(q) or self.where(q), ln.src, ln.op,
                                                             ln.operand))
        return sorted(found.items())

    def target(self, w, why):
        """A fixed branch target in the file becomes a routine start."""
        if w & 0o74000:
            t = self.cadr_phys(w)
            if self.in_file(t):
                if t not in self.code or not is_interp_op(self.code[t].op):
                    raise CompileError('branch to non-interpretive %s' % self.where(t))
                self.add_start(t, 'i', why)
            return t
        return None

    def call_return(self, p, why):
        """A CALL's return point is a routine start, unless no code
        follows it in the file: a call that never returns (R31's STCALL
        12D R34ANG) ends the file."""
        if not self.in_file(p):
            return
        ln = self.code.get(p)
        if ln is not None and not is_interp_op(ln.op):
            # Basic code follows: the CALL is a transfer that never returns
            # there (STCALL DELLT4 with QTEMP1, a saved QPRET, as its target).
            return
        self.add_start(p, 'i', why)

    def interp_block(self, start):
        """C statements for the interpretive routine at start."""
        out = []
        p = start
        self.cur_start = start
        bank = start // 1024
        first = True
        while True:
            if not first and p in self.starts:
                out.append('return EXEC_OK; /* on to %s */' % self.where(p))
                return out
            first = False
            ln = self.code.get(p)
            if ln is None or not is_interp_op(ln.op):
                raise CompileError('interpretive flow runs into %s (%r)' % (self.where(p), ln))
            stmts, nxt, done = self.slot(p, bank)
            if self.rk and not done and stmts and stmts[-1] in ('next(s);', 'setpd_next(s);'):
                stmts[-1] = 'if(%s)return EXEC_OK;' % stmts[-1][:-1]
                self.add_start(nxt, 'i', 'CHANG2 resume')
                self.externals_any[self.L.labels['CHANG2']] = 'CHANG2'
            for q in range(p, nxt):
                self.owner.setdefault(q, start)
            p = nxt
            out.append('/* %s %s %s */' % (self.where(ln.phys), ln.op, ln.operand))
            out.append('TRACE(s);')
            out.extend(stmts)
            if done:
                return out

    def indexed_fixed(self, fp):
        """kx(...) for an indexed operand based at fixed fp: the spec's
        'indexed_fixed' {label: (first, last)} word offsets the index
        reaches, plus the five words an order reads past them."""
        lab = self.name(fp)
        rng = self.spec.get('indexed_fixed', {}).get(lab)
        if rng is None:
            raise CompileError('indexed fixed operand %s at %s needs an indexed_fixed range' % (lab, self.where(fp)))
        lo, hi = fp + rng[0], fp + rng[1] + 5
        self.ktables[lo] = (hi - lo + 1, lab, rng)
        return 'kx(s,KX%o,%uu,%uu,&k)' % (lo, lo, hi - lo + 1)

    def fixed_index(self, p, takes_k, what):
        """Physical addresses an indexed operand at p reaches when it names
        a label the spec's fixed_index lists ({'LABEL': [offsets]}), else
        None. The interpreter subtracts C(X) from the address."""
        ln = self.code.get(p)
        lab = ln.operand.split(',')[0].replace(' ', '') if ln is not None and ln.operand else ''
        import re
        offs = self.spec.get('fixed_index', {}).get(re.match(r'^(.*?)([+-]\d+)?$', lab).group(1))
        if offs is None:
            return None
        if not takes_k or split_op(what)[0] in ('VXM', 'MXV'):
            raise CompileError('%s with indexed fixed operand' % what)
        base = self.label_phys(lab)
        out = []
        for o in offs:
            fp = base + o
            if fp // 1024 in (2, 3) or fp // 1024 >= 0o40:
                raise CompileError('indexed fixed operand %s in fixed-fixed or a superbank' % self.where(fp))
            self.consts[fp] = self.name(fp)
            self.const_len[fp] = max(self.const_len.get(fp, 6), K_WORDS.get(what, 6))
            out.append(fp)
        return out

    @staticmethod
    def superbank_guard(fx):
        """FB 30-37 is bank 30-37 only with the superbank bits clear; the
        effective address the switch computes assumes that."""
        return 'if(io->superbank&0160u)return EXEC_OUT_OF_SCOPE;' if any(fp // 1024 >= 0o30 for fp in fx) else ''

    def operand_k(self, w, bank, takes_k, starred, what, n=6):
        if starred or what == 'GSHIFT':
            # (A shift's operand word is its count and kind, not an address.)
            return '0'
        fp = self.fixed_operand(w, bank)
        if fp is None or what == 'GSHIFT':
            # A shift's operand word is its count (SL 14D is 020217), not
            # an address the order reads.
            return '0'
        if not takes_k:
            if not self.basic and split_op(what)[0] in ('GSHIFT', 'SL', 'SR', 'SLR', 'SRR', 'VSL', 'VSR'):
                return '0'   # (a shift count, not an address)
            raise CompileError('%s with fixed operand' % what)
        self.consts[fp] = self.name(fp)
        if self.basic:
            self.const_len[fp] = max(self.const_len.get(fp, 6), K_WORDS.get(what, 6), n)
        else:
            self.const_len[fp] = max(self.const_len.get(fp, 6), K_WORDS.get(split_op(what)[0], 6))
        return 'K%o' % fp

    def guarded_k(self, w, bank, takes_k, what, st):
        """index_guard: the operand of an indexed order. A fixed table (DLOAD*
        ALSK +1,1) is read from a window of rope words around its base; a sum
        outside the window, or an erasable base indexed into fixed memory,
        is refused."""
        a = ((w - 1) if w < 0o40000 else ((~w & 0o77777) - 1)) & 0o77777
        fp = None if a < 0o4000 else self.fixed_operand(a + 1, bank)
        if fp is None:
            st[-1] += 'if(W[0116]>=02000u)return EXEC_OUT_OF_SCOPE;'
            return '0'
        if not takes_k:
            raise CompileError('%s with an indexed fixed operand' % what)
        need = K_WORDS.get(what, 6)
        b0 = fp // 1024 * 1024
        lo, hi = max(b0, fp - 0o40), min(b0 + 1024, fp + 0o40 + need)
        self.kx[lo] = (hi - lo, fp)
        self.kx_used = True
        st[-1] += '{uint8_t bad=0;const agc_word *k=kx(s,KX%o,%uu,%uu,%uu,&bad);if(bad)return EXEC_OUT_OF_SCOPE;' % (lo, lo, hi - lo, need)
        return 'k'

    def indexed_k(self, w, bank, takes_k, what):
        """An indexed operand whose base (index 0) is fixed memory reads a
        compiled table of the words the spec says the index reaches:
        'indexed_fixed': {label: (lowest, highest) offset from the base}."""
        a = ((~w & 0o77777) if w & 0o40000 else w) - 1
        if a < 0o4000 or not takes_k or 'indexed_fixed' not in self.spec:   # (erasable, or a shift count)
            return '0'
        b = (a >> 10) | (0o20 if (bank & 0o20 and a < 0o10000) else 0)
        fp = b * 1024 + (a & 0o1777)
        lab = self.L.label_at(fp)
        rng = self.spec.get('indexed_fixed', {}).get(lab)
        if rng is None:
            raise CompileError('%s indexed into fixed %s (%s) has no indexed_fixed range' % (what, self.where(fp), lab))
        lo, hi = fp + rng[0], fp + rng[1] + 6
        self.ktabs[lo] = (hi - lo, lab)
        return 'KX%o,0%ou,%du' % (lo, lo, hi - lo)

    def slot(self, p, bank):
        """Statements for the NEWOPS slot at p: (stmts, next slot, ends)."""
        ln = self.code[p]
        P = self.p
        w = ln.words[0]
        cur = p + 1
        if ln.op in STORES:
            st = ['%sstore_code(s,0%05o);' % (P, w)]
            more, cur, done = self.store_tail(ln.op, cur, bank)
            return st + more, cur, done
        st = ['%snewops(s,0%05o);' % (P, w)]
        ops = [ln.op] + ([ln.operand] if ln.operand else [])
        prev_setpd = False
        for i, op in enumerate(ops):
            if i == 1:
                st.append('%snoibnksw_right(s);' % P if prev_setpd else 'right(s);')
            self.last_order = i == len(ops) - 1
            body, cur, done, prev_setpd = self.order(op, cur, bank)
            st.extend(body)
            if done == 'pair':
                return st, cur, False
            if done:
                return st, cur, True
        st.append('setpd_next(s);' if prev_setpd else 'next(s);')
        return st, cur, False

    def call_return(self, cur, why):
        """The word after a CALL is a routine when it is interpretive code; a
        CALL whose callee never returns (STCALL TNUV A-PCHK) is followed by
        anything."""
        ln = self.code.get(cur)
        if ln is not None and is_interp_op(ln.op):
            self.add_start(cur, 'i', why)

    def goto(self, w, why, call=False):
        """GOTO 06646 (after a taken branch or CALL) to the target."""
        P = self.p
        if w & 0o74000:
            t = self.target(w, why)
            if self.spec.get('self_loops') and t == self.cur_start:
                # A branch back to the routine's own start (P32/P72F's BPL)
                # loops here rather than through the run loop's step limit.
                return ['%sgoto(s);' % P, 'if(++loops<4096u)goto top;', 'return EXEC_OK;']
            return ['%sgoto(s);' % P, 'return EXEC_OK;']
        self.cadr_erasable.add(w)
        return ['%sgoto_any(s);' % P, 'return EXEC_OK;']

    def order(self, op, cur, bank):
        """(stmts, cursor, ends, setpd) for one order of a pair."""
        P = self.p
        kind, name = classify(op)
        base, star, tag = split_op(op)
        T = self.T
        if kind == 'short':
            return ['shorts(s);'], cur, False, False
        if kind == 'exit':
            # EXIT continues in basic code at the word after the pair.
            self.add_start(cur, 'b', 'after EXIT')
            return ['%sexit(s);return EXEC_OK;' % P], cur, True, False
        if kind == 'unary':
            if name == 'STADR':
                sw = self.word(cur)
                st = ['%sunary(s,0%05o);%sstadr(s,0%05o);' % (P, T['STADR'], P, sw)]
                sop = self.code[cur].op
                if sop not in STORES:
                    raise CompileError('STADR without store at %s' % self.where(cur))
                more, cur, done = self.store_tail(sop, cur + 1, bank)
                # The store's tail ends the pair (DANZIG) or transfers.
                return st + more, cur, (True if done else 'pair'), False
            body = {'SQRT': 'if(%ssqrt(s))return EXEC_OK;', 'SINE': '%ssin(s);',
                    'COSINE': '%scos(s);', 'ARCSIN': 'if(%sasin(s))return EXEC_OK;',
                    'ARCCOS': 'if(%sacos(s))return EXEC_OK;', 'DSQ': '%sdsq(s);',
                    'ROUND': '%sround(s);', 'COMP': '%scomp(s);', 'VDEF': '%svdef(s);',
                    'UNIT': 'if(%sunit(s))return EXEC_OK;',
                    'ABVALABS': 'if(%sabvalabs(s))return EXEC_OK;', 'VSQ': '%svsq(s);',
                    'PUSH': '%spush(s);', 'RVQ': '%srvq(s);return EXEC_OK;'}[name] % P
            st = ['%sunary(s,0%05o);' % (P, T[name]) + body]
            return st, cur, name == 'RVQ', False
        if kind == 'addr':
            fn, takes_k = ADDR[name]
            if self.basic and fn == 'vsc':
                fn = 'vdsc'   # (Lane G's V/SC body, which its frozen files call)
            if name == 'CGOTO':
                # CGOTO X LIST: the operand, then the CADR list's word.
                if star:
                    raise CompileError('CGOTO* at %s' % self.where(cur))
                a, lw = self.word(cur), self.word(cur + 1)
                lp = self.cadr_phys(lw)
                n = self.spec.get('cgoto', {}).get(self.L.label_at(lp))
                if not n:
                    raise CompileError('CGOTO list %s has no length in the spec' % self.L.label_at(lp))
                ws = [self.L.word(lp + i) for i in range(n)]
                for x in ws:
                    self.target(x, 'CGOTO target')
                self.lists[lp] = n
                tab = '(const agc_word[]){%s}' % ','.join('0%05o' % x for x in ws)
                return ['%sdirect(s,0%05o,0%05o);if(!%scgoto(s,0%05o,%s,%du))return EXEC_OUT_OF_SCOPE;return EXEC_OK;'
                        % (P, a, T['CGOTO'], P, lw, tab, n)], cur + 2, True, False
            if name == 'SSP':
                a, v = self.word(cur), self.word(cur + 1)
                ln = self.code.get(cur + 1)
                t = self.L.labels.get(ln.operand) if ln is not None else None
                if self.basic and t is not None and self.in_file(t) and t in self.code \
                        and self.code[t].op not in self.DATA_OPS:
                    # SSP storing the address of the file's own code (a GOTO
                    # selector such as POSEXIT = SCALEPOP): a routine start.
                    self.add_start(t, 'i' if is_interp_op(self.code[t].op) else 'b', 'SSP %s' % ln.operand)
                return ['%sdirect(s,0%05o,0%05o);%sssp(s,0%05o);' % (P, a, T['SSP'], P, v)], cur + 2, False, False
            nxt = self.word(cur)
            if star:
                st = ['%sindexed(s,0%05o,0%05o);' % (P, nxt, T[name])]
                fx = self.fixed_index(cur, takes_k or name == 'SLOAD', op)
                if fx and name == 'SLOAD':
                    # SLOAD* of a fixed table (ALARM/TB -1,1): the word at
                    # the effective address, loaded as SLOAD's fixed path.
                    st[-1] += self.superbank_guard(fx) + '{agc_word w;switch((unsigned)((W[4]>>10)*1024u+W[0116]-02000u)){'
                    st.extend('  case %du:w=K%o[0];break;' % (fp, fp) for fp in fx)
                    st.append('  default:return EXEC_OUT_OF_SCOPE;}')
                    st.append('W[1]=0;ai_caf(s,w);ai_dxch(s,0154);ai_caf(s,0);(void)ai_ts(s,0156);'
                              '(void)ai_ts(s,0163);W[5]=0%o;}' % (0o6061 if self.m == 'lm' else 0o6030))
                    for fp in fx:
                        self.const_len[fp] = 1
                    return st, cur + 1, False, False
                if fx:
                    # An indexed fixed operand: the rope words at each
                    # address the spec's index values reach; any other
                    # effective address is refused.
                    st[-1] += self.superbank_guard(fx) + '{const agc_word *k;switch((unsigned)((W[4]>>10)*1024u+W[0116]-02000u)){'
                    st.extend('  case %du:k=K%o;break;' % (fp, fp) for fp in fx)
                    st.append('  default:return EXEC_OUT_OF_SCOPE;}')
                    st.append('%s%s(s,k);}' % (P, fn))
                    return st, cur + 1, False, False
                cur += 1
                if self.basic:
                    # (whole-file basic files: the spec's indexed_fixed ranges
                    # through kx, refusing outside them)
                    k = '0'
                    fp = self.fixed_operand(nxt, bank)
                    if fp is not None and takes_k and self.name(fp) in self.spec.get('indexed_fixed', {}):
                        k = self.indexed_fixed(fp)
                elif self.spec.get('index_guard'):
                    k = self.guarded_k(nxt, bank, takes_k, name, st) if name != 'GSHIFT' else '0'
                else:
                    k = self.indexed_k(nxt, bank, takes_k, op)
                    if name == 'SLOAD' and ((~nxt & 0o77777) if nxt & 0o40000 else nxt) - 1 >= 0o4000:
                        # (SLOAD*'s indexed path reads erasable: a fixed table
                        # needs the spec's fixed_index)
                        raise CompileError('SLOAD* at %s indexes fixed memory without a fixed_index entry' % self.where(cur - 1))
                    if k != '0':
                        st[-1] += '{const agc_word *k;if(!kix(s,%s,&k))return EXEC_OUT_OF_SCOPE;%s%s(s,k);}' % (k, P, fn)
                        return st, cur, False, name == 'SETPD'
            elif nxt & 0o40000:
                if name == 'SETPD':
                    raise CompileError('SETPD push-up')
                st = ['%saddress_pushup(s,0%05o,0%05o);' % (P, nxt, T[name])]
                k = '0'
            else:
                st = ['%sdirect(s,0%05o,0%05o);' % (P, nxt, T[name])]
                if self.basic:
                    if name in ('TLOAD', 'SLOAD') and self.fixed_operand(nxt, bank) is not None:
                        fn, takes_k = name.lower() + '_fixed', True     # a load of a rope constant
                    # A shift's word is a count code (020200 + n), not an address.
                    k = '0' if name == 'GSHIFT' else self.operand_k(nxt, bank, takes_k, False, op,
                                                                    18 if name in ('MXV', 'VXM') else 6)
                else:
                    if name == 'TLOAD' and self.fixed_operand(nxt, bank) is not None:
                        # TLOAD of a fixed triple (the interpreter's own path,
                        # with the three rope words).
                        fp = self.fixed_operand(nxt, bank)
                        self.consts[fp] = self.name(fp)
                        self.const_len[fp] = max(self.const_len.get(fp, 3), 3)
                        st[-1] += '%stload_fixed(s,K%o);' % (P, fp)
                        return st, cur + 1, False, False
                    if name == 'SLOAD' and self.fixed_operand(nxt, bank) is not None:
                        # SLOAD of a fixed constant (DECTWO): ZL; CAF K; DXCH
                        # MPAC; CAF ZERO; TS MPAC+2; TS MODE; on to DANZIG.
                        fp = self.fixed_operand(nxt, bank)
                        self.consts[fp] = self.name(fp)
                        self.const_len.setdefault(fp, 1)
                        st[-1] += ('W[1]=0;ai_caf(s,K%o[0]);ai_dxch(s,0154);ai_caf(s,0);(void)ai_ts(s,0156);'
                                   '(void)ai_ts(s,0163);W[5]=0%o;' % (fp, 0o6061 if self.m == 'lm' else 0o6030))
                        return st, cur + 1, False, False
                    k = self.operand_k(nxt, bank, takes_k, False, op)
                cur += 1
            if takes_k and k.startswith('kx('):
                # An indexed fixed operand: the rope words at the address the
                # index gives, from the spec's table (erasable: the image).
                st[-1] += '{const agc_word *k;if(%s)return EXEC_INVALID_ACTION;%s%s(s,k);}' % (k, P, fn)
            elif name == 'SLOAD' and not self.basic and k != '0':
                # SLOAD of a fixed word (SCHZEROS): sload_k below.
                self.sload_k = True
                st[-1] += 'sload_k(s,%s);' % k
            elif name == 'SLOAD' and k == '0':
                st[-1] += '%ssload(s);' % P
            elif takes_k:
                st[-1] += '%s%s(s,%s);' % (P, fn, k)
                if k == 'k':
                    st[-1] += '}'
            else:
                st[-1] += '%s%s(s);' % (P, fn)
            return st, cur, False, name == 'SETPD'
        if kind in ('switch', 'switch_branch'):
            sw = self.word(cur)
            st = ['%smisc(s,0%05o,0%05o);' % (P, sw, T['SW/'])]
            cur += 1
            if kind == 'switch':
                st[-1] += 'if(%sswitch(s,io))return EXEC_OUT_OF_SCOPE;' % P
                return st, cur, False, False
            bw = self.word(cur)
            cur += 1
            if op.endswith('GO'):
                # SETGO/CLRGO/INVGO always branch.
                st[-1] += 'if(!%sswitch(s,io))return EXEC_OUT_OF_SCOPE;' % P
                st.extend(self.swbranch(bw))
                return st, cur, True, False
            st[-1] += 'if(%sswitch(s,io)){' % P
            st.extend(['  ' + x for x in self.swbranch(bw)])
            st.append('}')
            return st, cur, False, False
        # misc
        w = self.word(cur)
        cur += 1
        st = ['%smisc(s,0%05o,0%05o);' % (P, w, T[name])]
        if base in INDEXED_MISC:
            if base == 'TIX':
                st.append('if(%stix(s)){' % P)
                st.extend(['  ' + x for x in self.goto(w, 'TIX target')])
                st.append('}')
                return st, cur, False, False
            st[-1] += '%s%s(s);' % (P, base.lower())
            return st, cur, False, False
        if base in ('BPL', 'BMN', 'BZE', 'BHIZ'):
            fn = {'BPL': 'bplbmn', 'BMN': 'bplbmn', 'BZE': 'bzegoto', 'BHIZ': 'bhiz'}[base]
            if base == 'BHIZ':
                # RTB/BHIZ CCS CYR (negative) comes before BHIZ's CCS MPAC.
                st[-1] += '(void)ai_ccs(s,020);'
            st.append('if(%s%s(s)){' % (P, fn))
            st.extend(['  ' + x for x in self.goto(w, base + ' target')])
            st.append('}')
            return st, cur, False, False
        if base == 'GOTO':
            if w & 0o74000:
                self.target(w, 'GOTO target')
                st[-1] += '%sgoto_order(s);return EXEC_OK;' % P
            else:
                self.cadr_erasable.add(w)
                st[-1] += '(void)ai_ccs(s,020);%sgoto_any(s);return EXEC_OK;' % P
            return st, cur, True, False
        if base == 'CALL':
            st[-1] += '%scall(s);' % P
            if self.basic:
                if op.startswith('CALRB'):
                    # CALRB: the callee comes back in basic code (BANKJUMP to
                    # the CADR CALL saved), at the word after the pair.
                    self.add_start(cur, 'b', 'CALRB return')
                else:
                    self.call_return_g(cur, w, 'CALL return')
            else:
                self.call_return(cur, 'CALL return')
            st.extend(self.goto(w, 'CALL target', call=True))
            return st, cur, True, False
        if base in ('ITA', 'STQ'):
            st[-1] += '%sita(s);' % P
            return st, cur, False, False
        if base == 'RTB':
            target = self.cadr_phys(w)
            lab = self.L.label_at(target)
            if lab not in self.rtb and self.basic and (not self.in_file(target) or (
                    target in self.code and not is_interp_op(self.code[target].op))):
                # RTB to basic code with no native body (whole-file basic):
                # the interpreter transfers there, to a routine of the file or
                # an outside one; its TC DANZIG resumes at NEWOPS.
                if self.in_file(target):
                    self.add_start(target, 'b', 'RTB target')
                else:
                    self.externals_any[target] = 'RTB ' + (lab or '')
                if cur in self.code and is_interp_op(self.code[cur].op):
                    self.add_start(cur, 'i', 'after RTB %s' % lab)
                elif self.rk:
                    # RTB with an order on its right: DANZIG runs that order,
                    # then NEWOPS at the word after its operands.
                    q = cur
                    while q in self.code and not is_interp_op(self.code[q].op):
                        q += 1
                    if q in self.code:
                        self.add_start(q, 'i', 'after RTB %s' % lab)
                st[-1] += '(void)ai_ccs(s,020);%srtb(s);return EXEC_OK; /* RTB %s */' % (P, lab)
                return st, cur, True, False
            if lab not in self.rtb:
                raise CompileError('RTB %s has no native routine in the spec' % lab)
            st[-1] += '(void)ai_ccs(s,020);%srtb(s);' % P
            if self.rtb[lab] is None:
                # No native routine yet (or the file's own basic code): the
                # routine ends at its basic code.
                if not self.in_file(target):
                    self.externals_any[target] = 'RTB ' + lab
                if not self.last_order:
                    raise CompileError('RTB %s with an order after it at %s' % (lab, self.where(cur)))
                # Its DANZIG resumes the file at the next pair.
                self.add_start(cur, 'i', 'RTB %s return' % lab)
                st.append('return EXEC_OK; /* RTB %s */' % lab)
                return st, cur, True, False
            call = self.rtb[lab][self.m]
            if isinstance(call, tuple):
                # (call, 'stops'): the call returns RTB_OK to go on, 2 when
                # the run ends outside the file (an alarm), else refuses.
                st.append('{int r=%s;if(r==2)return EXEC_OK;if(r!=RTB_OK)return EXEC_OUT_OF_SCOPE;} /* %s */'
                          % (call[0], lab))
                return st, cur, False, False
            st.append('if(%s!=RTB_OK)return EXEC_OUT_OF_SCOPE; /* %s */' % (call, lab))
            return st, cur, False, False
        if base in ('BOV', 'BOVB'):
            st.append('switch(%sbov(s)){case %sBOV_NONE:break;case %sBOV_GOTO:' % (P, P.upper(), P.upper()))
            t = self.cadr_phys(w) if base == 'BOVB' else None
            if t is not None and self.in_file(t) and t in self.code \
                    and not is_interp_op(self.code[t].op) and self.L.label_at(t) not in self.rtb:
                # BOVB to the file's own basic code (P61's BOVB RETRN1,
                # FINDCDUW's BOVB UNIT NOATTCNT): a basic routine of the
                # file; the body takes the RTB path.
                self.add_start(t, 'b', 'BOVB target')
                st.append('  return EXEC_OUT_OF_SCOPE;')
                st.append('case %sBOV_RTB:%srtb(s);return EXEC_OK;' % (P.upper(), P))
            else:
                st.extend(['  ' + x for x in self.goto(w, 'BOV target')])
                if base == 'BOVB' and self.L.label_at(t) in self.rtb:
                    # BOVB to a basic routine the spec runs natively (as RTB).
                    lab = self.L.label_at(t)
                    st.append('case %sBOV_RTB:%srtb(s);if(%s!=RTB_OK)return EXEC_OUT_OF_SCOPE;break; /* %s */'
                              % (P.upper(), P, self.rtb[lab][self.m], lab))
                elif base == 'BOVB':
                    # BOVB's branch is to basic code, as RTB transfers.
                    if self.in_file(t):
                        raise CompileError('BOVB into the file at %s' % self.where(t))
                    self.externals_any[t] = 'BOVB ' + (self.L.label_at(t) or '')
                    st.append('case %sBOV_RTB:%srtb(s);return EXEC_OK;' % (P.upper(), P))
            st.append('default:return EXEC_OUT_OF_SCOPE;}')
            return st, cur, False, False
        raise CompileError('unsupported order %s' % op)

    def swbranch(self, w):
        P = self.p
        if w & 0o74000:
            self.target(w, 'switch branch target')
            return ['%sswbranch(s,0%05o);return EXEC_OK;' % (P, w)]
        self.cadr_erasable.add(w)
        return ['(void)ai_ca(s,0165);(void)ai_ts(s,4);ai_caf(s,0%05o);(void)ai_ts(s,0117);%sgoto_any(s);return EXEC_OK;' % (w, P)]

    def call_return_g(self, cur, w, why):
        """The word after a CALL/STCALL is a routine start, unless the call
        goes through an erasable CADR (a return the file keeps with STQ) and
        no order follows it."""
        ln = self.code.get(cur)
        if (not (w & 0o74000) or (ln is not None and ln.op in self.DATA_OPS)) and (
                ln is None or not is_interp_op(ln.op)):
            # (Also a CALL used as a GOTO, with constants after it.)
            return
        if self.in_file(cur) and ln is not None and self.is_basic(cur) and ln.op != 'INCR':
            # Basic code follows, as in call_return (P11's GETDOWN: STCALL
            # 8D LALOTORV +5, which leaves through the INCORPEX return
            # GETDOWN saved): the call does not come back here.
            return
        self.add_start(cur, 'i', why)

    def store_tail(self, sop, cur, bank):
        """After the store itself (Z at the STORJUMP continuation)."""
        P = self.p
        T = self.T
        if sop == 'STORE':
            return ['next(s);'], cur, False
        if sop == 'STCALL':
            w = self.word(cur)
            cur += 1
            st = ['%s15bitadr(s,0%05o,0%05o);%scall(s);' % (P, w, T['CALL/ITA'], P)]
            if self.basic:
                self.call_return_g(cur, w, 'STCALL return')
            else:
                self.call_return(cur, 'STCALL return')
            return st + self.goto(w, 'STCALL target', call=True), cur, True
        w = self.word(cur)
        load, fn = ('DLOAD', 'dload_op') if sop.startswith('STODL') else ('VLOAD', 'vload')
        if sop.endswith('*'):
            star = self.dloadstar if load == 'DLOAD' else self.vloadstar
            fx = self.fixed_index(cur, True, load)
            if fx:
                # STODL*/STOVL* of an indexed fixed operand (MUTABLE +2,1).
                st = ['ai_caf(s,0%05o);(void)ai_ts(s,020);%sindexed(s,0%05o,0%05o);' % (star, P, w, T[load]) +
                      self.superbank_guard(fx) + '{const agc_word *k;switch((unsigned)((W[4]>>10)*1024u+W[0116]-02000u)){']
                st.extend('  case %du:k=K%o;break;' % (fp, fp) for fp in fx)
                st.append('  default:return EXEC_OUT_OF_SCOPE;}')
                st.append('%s%s(s,k);}' % (P, fn))
                return st + ['next(s);'], cur + 1, False
            k = self.indexed_k(w, bank, True, sop)
            st = ['ai_caf(s,0%05o);(void)ai_ts(s,020);%sindexed(s,0%05o,0%05o);' % (star, P, w, T[load])]
            if k != '0':
                # (a starred load from a fixed table: STOVL* COGA MUTABLE,1)
                st[-1] += '{const agc_word *k;if(!kix(s,%s,&k))return EXEC_OUT_OF_SCOPE;%s%s(s,k);}' % (k, P, fn)
            else:
                st[-1] += '%s%s(s,0);' % (P, fn)
            return st + ['next(s);'], cur + 1, False
        pre = '%sdodload(s);' % P if load == 'DLOAD' else '%sdovload(s);' % P
        if w & 0o40000:
            st = [pre + '%sdiradres_pushup(s,0%05o,0%05o);%s%s(s,0);' % (P, w, T[load], P, fn)]
        else:
            k = self.operand_k(w, bank, True, False, sop)
            st = [pre + '%sdiradres(s,0%05o,0%05o);%s%s(s,%s);' % (P, w, T[load], P, fn, k)]
            cur += 1
        return st + ['next(s);'], cur, False

    # ---- basic code ------------------------------------------------------
    def phys_of(self, k, bank):
        """Physical fixed address of a 12-bit address field (None if erasable)."""
        if k < 0o2000:
            return None
        if k >= 0o4000:
            return 2048 + (k - 0o4000)
        if bank in (2, 3):
            raise CompileError('banked address %04o from fixed-fixed code' % k)
        return bank * 1024 + (k - 0o2000)

    def logical_of(self, phys):
        bank, off = divmod(phys, 1024)
        if bank in (2, 3):
            return 0o4000 + (bank - 2) * 1024 + off
        return 0o2000 + off

    def basic_unit(self, p, bank):
        st, succ = self._basic_unit(p, bank)
        if self._ix:
            w, k, mask = self._ix
            head = st[0]
            self._st_tail = st[1:]
            if self._ix2:
                # EXTEND; INDEX K; INDEX T (T fixed); X: the second INDEX
                # adds C(T + C(K)), a word of the rope, for the offsets the
                # spec's basic_fixed_index lists (any other C(K) refuses).
                k1, a2, bank2, offs = self._ix2
                expr = ''.join('W[0%o]==0%05ou?(agc_word)(0%05ou+0%05ou):' % (
                    k1, off & 0o77777 if off >= 0 else 0o77777 + off, w, self.L.word(self.phys_of(a2 + off, bank2)))
                    for off in offs) + '077777u'
                for off in offs:
                    self.basic_fixed.add(self.phys_of(a2 + off, bank2))
                st = [head, '{agc_word iw=%s;if((iw^0%05ou)&0%05ou)return EXEC_OUT_OF_SCOPE;' % (
                    expr, w, 0o77777 & ~mask)]
            else:
                st = [head, '{agc_word iw=ixw(s,0%05o,0%ou);if((iw^0%05ou)&0%05ou)return EXEC_OUT_OF_SCOPE;' % (
                    self._ix_base if self._ix_base is not None else w, k, w, 0o77777 & ~mask)]
            if mask == 0o7777 and self.spec.get('index_guard'):
                # A 12-bit erasable operand: the sum must stay erasable (an
                # INDEX POSITON far past the table would read the rope).
                st.append('  if((iw&07777u)>=02000u)return EXEC_OUT_OF_SCOPE;')
            if self._ix_write:
                e, n = self._ix_write
                st.append('  if(%s<=6u&&%s+%du>=3u)return EXEC_OUT_OF_SCOPE;' % (e, e, n - 1))
            if self._ix_read:
                st.append('  if(%s>=02000u)return EXEC_OUT_OF_SCOPE;' % self._ix_read)
            st += ['  ' + x for x in self._st_tail] + ['}']
        return st, succ

    def _basic_unit(self, p, bank):
        """(statements, successors, words, branch targets) for the basic
        instruction (with any EXTEND/INDEX prefix) at p. Successors are
        physical addresses compiled into the same routine; a statement list
        that ends the routine has none."""
        ext = self.spec.get('externals', {})
        w = self.L.word(p)
        start = p
        extended = False
        index = None
        self._ix2 = None
        while True:
            if w == 0o6:          # EXTEND
                extended = True
                p += 1
                w = self.L.word(p)
                continue
            op, qc, k12, k10 = (w >> 12) & 7, (w >> 10) & 3, w & 0o7777, w & 0o1777
            if op == 5 and qc == 0 and not extended and index is None and k10 != 0o17:
                index = k10       # INDEX K: the next instruction's address + C(K)
                p += 1
                w = self.L.word(p)
                continue
            if op == 5 and extended and index is None and k12 < 0o2000:
                index = k12       # extended INDEX: next stays extended
                p += 1
                w = self.L.word(p)
                continue
            if op == 5 and extended and index is not None and self._ix2 is None and k12 >= 0o2000 and \
                    self.code.get(p) and self.code[p].operand.split()[0] in self.spec.get('basic_fixed_index', {}):
                # A second INDEX, of a fixed table, under the first (CGCALC's
                # EXTEND; INDEX WCHPHASE; INDEX TARGTDEX; DCA TCGFBRAK).
                self._ix2 = (index, k12, bank, self.spec['basic_fixed_index'][self.code[p].operand.split()[0]])
                p += 1
                w = self.L.word(p)
                continue
            break
        self._ix_base = None
        carry = self.spec.get('index_carry', {}).get(self.where(p))
        if carry is not None and index is not None:
            # INDEX K; TS 0 -1 (AOTMARK's MKVACFND): the index carries the
            # word into another quarter code. Compile the instruction the
            # sum becomes (carry: its opcode and quarter code, address
            # from the sum); any other sum refuses.
            self._ix_base = w
            w = carry
            op, qc, k12, k10 = (w >> 12) & 7, (w >> 10) & 3, w & 0o7777, w & 0o1777
        nxt = p + 1
        self.unit_words = range(start, p + 1)
        ln = self.code.get(p)
        if ln is None or p not in self.words:
            raise CompileError('basic flow runs into %s' % self.where(p))
        here = '/* %s %s %s */' % (self.where(p), ln.op, ln.operand)
        if index is None and op != 0 and (w & 0o7777) == 5:
            # An instruction that reads Z (VNPOOH's... CA Z) sees the
            # address after it, as the hardware's Z holds.
            here += 'W[5]=0%04o;' % (ln.loc + 1)

        self._ix = None
        self._ix_write = None
        self._ix_read = None
        if self._ix2:
            self._ix = (w, index, 0o7777)
        if (index is not None and not extended and op == 1 and qc == 0 and k10 < 0o10
                and self.spec.get('index_returns')):
            # INDEX K; TCF 0 +n with K a saved Q (P37's INDEX SPRTEX, TCF 0
            # +1): a return n words past the caller's. The sum is a TCF
            # when it carries into the address's top bits; the run loop
            # takes Z from there.
            return [here, '{agc_word iw=ixw(s,0%05o,0%ou);' % (w, index) +
                    'if((iw&070000u)!=010000u||(iw&07777u)<02000u)return EXEC_OUT_OF_SCOPE;' +
                    'W[5]=(agc_word)(iw&07777u);return EXEC_OK;}'], []

        def ea(k):
            if index is None:
                return '0%04ou' % k
            # INDEX adds C(K) to the whole instruction word at run time
            # (ixw); the opcode bits must not change.
            mask = 0o7777 if (w & 0o7777) - k in (0, 1) else 0o1777
            self._ix = (w, index, mask)
            e = '(unsigned)((iw&0%ou)-%du)' % (mask, (w & mask) - k)
            if mask == 0o7777 and self.spec.get('index_erasable'):
                # (A sum past the erasable words is refused: LLGE's INDEX
                # MPAC +3; CA 5 reads a power table there.)
                self._ix_read = e
            return e

        def eras(k, what):
            writes = what in ('ts', 'xch', 'lxch', 'qxch', 'dxch', 'das', 'ads', 'incr', 'aug', 'dim')
            if index is not None:
                e = ea(k)
                if writes:
                    # The indexed address must not be a bank or Z register.
                    n = 2 if what in ('dxch', 'das') else 1
                    self._ix_write = (e, n)
                return e
            if k in (4, 5, 6) and writes:
                raise CompileError('%s writes a bank or Z register at %s' % (what, self.where(p)))
            return ea(k)

        def fixed(k):
            if index is not None:
                raise CompileError('INDEX on a fixed operand at %s' % self.where(p))
            fp = self.phys_of(k, bank)
            self.basic_fixed.add(fp)
            return fp

        def fixed_indexed(fn):
            # INDEX K; CAF T (ABRTJASK's INDEX Q; CAF MODE70) or MP T
            # (FLTRSUB's INDEX NDXCDUW; MP GAINFLTR): the spec's
            # basic_fixed_index lists the offsets K can hold; any other
            # effective address is refused.
            offs = self.spec.get('basic_fixed_index', {}).get(ln.operand.split()[0])
            if offs is None:
                raise CompileError('INDEX on a fixed operand at %s (no basic_fixed_index)' % self.where(p))
            self._ix = (w, index, 0o7777)
            cases = []
            for off in offs:
                fp = self.phys_of(k12 + off, bank)
                self.basic_fixed.add(fp)
                cases.append('case 0%04ou:ai_%sf(s,0%05o);break;' % (k12 + off, fn, self.L.word(fp)))
            return 'switch(iw&07777u){%sdefault:return EXEC_OUT_OF_SCOPE;}' % ''.join(cases)

        def leave(z_phys, why, note=''):
            """Transfer to a basic address: in the file a goto, else the end."""
            if self.in_file(z_phys):
                return ['goto L%o;' % z_phys], [z_phys]
            self.externals[z_phys] = why
            return ['W[5]=0%04o;return EXEC_OK;%s' % (self.logical_of(z_phys), note)], []

        st = [here]
        if not extended:
            if op == 0:
                if index is not None:
                    return self.indexed_tc(p, w, index, ln, nxt, bank)
                if k12 == 3:
                    return st + ['io->interrupt_enabled=1u;'], [nxt]
                if k12 == 4:
                    return st + ['io->interrupt_enabled=0u;'], [nxt]
                if k12 == 2:
                    # TC Q runs C(Q) as an instruction: TC its address, Q = 3
                    # (a return inside this routine is a goto; basic_block
                    # fills in the dispatch).
                    return st + ['@TCQ@'], []
                if k12 in self.erasable_calls:
                    # A call to an erasable program (TC LAT(SPL) in E7):
                    # the run ends there, with Z the erasable address.
                    st.append('ai_tc(s,0%04o);' % (ln.loc + 1))
                    return st + ['W[5]=0%04o;return EXEC_OK; /* erasable program %s */' % (k12, ln.operand.split()[0])], []
                if (k12 == 0 or 0o10 <= k12 < 0o2000) and index is None:
                    # (TC A, FASTCHNG's return, runs the TC in A: Q = 1.)
                    # TC to an erasable word runs it as an instruction (a
                    # saved return): Z = its address, Q = the word's own + 1.
                    # 01400-01777 is the switched bank EB selects (QSAVED).
                    st.append('ai_tc(s,0%04o);' % (ln.loc + 1))
                    self.tc_erasable.add(k12)
                    return st + ['return tc_e(s,0%04ou);' % k12], []
                if k12 < 0o2000:
                    raise CompileError('TC to erasable %04o at %s' % (k12, self.where(p)))
                tz = self.phys_of(k12, bank)
                lab = self.L.label_at(tz) or ''
                st.append('ai_tc(s,0%04o);' % (ln.loc + 1))
                if lab == 'INTPRET':
                    st.append('W[5]=0%04o;%sintpret(s,io);return EXEC_OK;' % (self.intpret, self.p))
                    self.add_start(p + 1, 'i', 'after TC INTPRET')
                    return st, []
                if lab in self.spec.get('intpret_via', ()):
                    self.add_start(p + 1, 'i', 'after TC %s' % lab)
                if self.in_file(tz):
                    # (A TC followed by data, as TC SBANDANT +2, does not return.)
                    if self.is_basic(nxt):
                        self.add_start(nxt, 'b', 'return from %s' % (lab or self.where(tz)))
                    # The return, and the skips INCR Q makes, stay in this routine.
                    rets = [nxt] + ([nxt + 1, nxt + 2, nxt + 3] if self.incr_q else [])
                    # Returns past the TC the callee makes itself (ROOTPSRS's
                    # INDEX RETROOT; TCF 2): keys of their own.
                    for k in self.spec.get('in_file_returns', {}).get(lab, ()):
                        self.add_start(nxt + k, 'b', 'return from %s +%d' % (lab, k))
                        rets.append(nxt + k)
                    rets = [q for q in rets if self.is_basic(q)]
                    self.returns_seen.update(rets)
                    return st + ['goto L%o;' % tz], [tz] + rets
                if not lab and ln.op in ('READ', 'WRITE', 'RAND', 'WAND', 'ROR', 'WOR', 'RXOR'):
                    # A channel instruction run without its EXTEND (a TS
                    # overflow skip over the EXTEND): no producer leaves
                    # overflow there, so it is refused.
                    return st + ['return EXEC_OUT_OF_SCOPE; /* %s without EXTEND */' % ln.op], []
                nc = self.spec.get('native_calls', {}).get(lab, {}).get(self.m)
                if nc:
                    # A basic subroutine outside the file run natively in
                    # place (it returns to the word after the TC, Q as its own
                    # TC Q leaves it).
                    return st + ['if(%s!=RTB_OK)return EXEC_OUT_OF_SCOPE; /* %s */' % (nc, lab)], [nxt]
                if lab not in ext:
                    raise CompileError('TC %s at %s: not in the spec externals' % (lab or '%04o' % k12, self.where(p)))
                n = ext[lab]
                if n == 'auto':
                    # Its inline words: the data words that follow the TC.
                    n, q = 0, nxt
                    while q in self.code and self.code[q].op in DATA_OPS:
                        n += len(self.code[q].words)
                        q += len(self.code[q].words)
                self.externals[tz] = lab
                if n is not None and nxt in self.code and \
                        self.spec.get('returns', {}).get(' '.join(self.code[nxt].operand.split()), 1) == 0:
                    # A callee that never returns: only its inline CADR is
                    # the call's (what follows is the file's data).
                    self.unit_words = range(start, nxt + 1)
                    return st + ['W[5]=0%04o;return EXEC_OK; /* %s */' % (k12, lab)], []
                if n is not None:
                    # The inline words (the call's operands) belong to this TC.
                    self.unit_words = range(start, nxt + n)
                    # (Data after the inline words: the call does not return
                    # there, as BANKCALL; CADR R53C1 jumping into a routine.)
                    # A display (GOXDSPF, GOMARKF...) called through its
                    # inline CADR returns to one of several words; 'returns'
                    # 0 is a callee that never returns (BANKCALL DISPEXIT +3).
                    callee = self.code[nxt].operand.strip() if n == 1 and nxt in self.code else None
                    if lab not in self.spec.get('noreturn', ()) and self.is_basic(nxt + n) and \
                            self.spec.get('returns', {}).get(callee, 1):
                        self.add_start(nxt + n, 'b', 'return from ' + lab)
                        for i in range(1, self.spec.get('returns', {}).get(callee, 1)):
                            if self.is_basic(nxt + n + i):
                                self.add_start(nxt + n + i, 'b', 'return %d from %s' % (i + 1, callee))
                return st + ['W[5]=0%04o;return EXEC_OK; /* %s */' % (k12, lab)], []
            if op == 1 and qc == 0:
                # A CCS outcome landing outside basic code (the source's
                # data or interpretive words) has no producer: refused.
                arms, succ = [], []
                for q in (nxt, nxt + 1, nxt + 2, nxt + 3):
                    # (Spec 'refuse_unextended': an arm on the word after
                    # an EXTEND would run it unextended, so it has no
                    # producer and is refused like data.)
                    if self.is_basic(q) and not (self.spec.get('refuse_unextended') and q - 1 > p
                                                 and self.L.word(q - 1) == 0o6):
                        arms.append('goto L%o;' % q)
                        succ.append(q)
                    else:
                        arms.append('return EXEC_OUT_OF_SCOPE;')
                st.append('switch(ai_ccs(s,%s)){case 0:%scase 1:%scase 2:%sdefault:%s}' % ((eras(k10, 'ccs'),) + tuple(arms)))
                return st, succ
            if op == 1 and index is not None and not extended and \
                    ln.operand.split()[0] in self.spec.get('index_tcf', {}):
                # INDEX K; TCF T into a table of the file (LLGE's INDEX
                # WCHPHASE; TCF WHATGUID): the spec's index_tcf lists the
                # offsets K can hold; any other target is refused.
                offs = self.spec['index_tcf'][ln.operand.split()[0]]
                self._ix = (w, index, 0o7777)
                cases, succ = [], []
                for off in offs:
                    t = self.phys_of(k12 + off, bank)
                    if not self.is_basic(t):
                        raise CompileError('index_tcf %s%+d is not basic code' % (ln.operand, off))
                    cases.append('case 0%04ou:goto L%o;' % ((k12 + off) & 0o7777, t))
                    succ.append(t)
                return st + ['switch(iw&07777u){%sdefault:return EXEC_OUT_OF_SCOPE;}' % ''.join(cases)], succ
            if op == 1:
                if index is not None:
                    return self.indexed_tc(p, w, index, ln, nxt, bank, tcf=True)
                tz = self.phys_of(k12, bank)
                if self.L.label_at(tz) == 'INTPRET' and self.spec.get('intpret_via'):
                    # TCF INTPRET in a subroutine the file TCs to (LLGE's
                    # INTPRETX): the interpretive code starts at the
                    # caller's Q, as after TC INTPRET.
                    return st + ['W[5]=0%04o;%sintpret(s,io);return EXEC_OK;' % (self.intpret, self.p)], []
                more, succ = leave(tz, 'TCF %s' % ln.operand)
                return st + more, succ
            if op == 2:
                if qc == 2 and k10 == 6 and index is None:
                    # INCR BBANK (TDISPSET's EBANK 5 to 7): EB + 1 through BB;
                    # past EB 7 (into BB's unused bits) is refused.
                    return st + ['if((W[6]&7u)==7u)return EXEC_OUT_OF_SCOPE;',
                                 'W[6]=(agc_word)(W[6]+1u);W[3]=(agc_word)((W[6]&7u)<<8);'], [nxt]
                fn = ('das', 'lxch', 'incr', 'ads')[qc]
                a = k10 - 1 if fn == 'das' else k10
                return st + ['ai_%s(s,%s);' % (fn, eras(a, fn))], [nxt]
            if op in (3, 4, 6, 7):
                fn = {3: 'ca', 4: 'cs', 6: 'ad', 7: 'mask'}[op]
                if k12 >= 0o2000 and index is not None:
                    return st + [fixed_indexed(fn)], [nxt]
                if k12 >= 0o2000:
                    fp = fixed(k12)
                    return st + ['ai_%sf(s,0%05o);' % (fn, self.L.word(fp))], [nxt]
                return st + ['ai_%s(s,%s);' % (fn, eras(k12, fn))], [nxt]
            if op == 5:
                if qc == 0 and k10 == 0o17 and index is None:
                    # RESUME (INDEX 17): the next instruction is BRUPT's,
                    # run in place (yaAGC's substitute instruction); here a
                    # TCF, as CONTABRT stores (any other word is refused).
                    return st + ['{agc_word b=W[017];if((b&070000u)!=010000u||(b&07777u)<02000u)return EXEC_OUT_OF_SCOPE;'
                                 'W[5]=(agc_word)(b&07777u);return EXEC_OK;}'], []
                if qc == 1 and k10 - 1 == 5 and index is None:
                    # DTCB (DXCH Z): Z,BB <-> A,L; BB sets FB and EB. The
                    # run loop goes on from the new Z under the new FB.
                    return st + ['{agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);W[0]=0%04o;W[1]=agc_extend(W[6]);'
                                 'W[6]=(agc_word)(bb&076007u);W[3]=(agc_word)((bb&7u)<<8);W[4]=(agc_word)(bb&076000u);'
                                 'W[5]=z;return EXEC_OK;}' % (ln.loc + 1)], []
                if qc == 1:
                    return st + ['ai_dxch(s,%s);' % eras(k10 - 1, 'dxch')], [nxt]
                if qc == 2:
                    if not self.is_basic(nxt + 1):
                        # An overflow skip out of the code has no producer.
                        return st + ['if(ai_ts(s,%s))return EXEC_OUT_OF_SCOPE;' % eras(k10, 'ts')], [nxt]
                    return st + ['if(ai_ts(s,%s))goto L%o;' % (eras(k10, 'ts'), nxt + 1)], [nxt, nxt + 1]
                return st + ['ai_xch(s,%s);' % eras(k10, 'xch')], [nxt]
        else:
            w = ln.words[0] if ln.words else 0
            io_op, chan = (w >> 9) & 7, w & 0o777
            if op == 0 and index is None and chan in (1, 2):
                # Channels 1 and 2 are L and Q: whole 16-bit register
                # operations, as yaAGC does them.
                r = 'W[%d]' % chan
                return st + [{0: 'W[0]=%s;' % r, 1: '%s=W[0];' % r,
                              2: 'W[0]=(agc_word)(W[0]&%s);' % r, 3: 'W[0]=%s=(agc_word)(W[0]&%s);' % (r, r),
                              4: 'W[0]=(agc_word)(W[0]|%s);' % r, 5: 'W[0]=%s=(agc_word)(W[0]|%s);' % (r, r),
                              6: 'W[0]=(agc_word)(W[0]^%s);' % r}[io_op]], [nxt]
            if op == 0 and self.spec.get('channel_inputs') and index is None and io_op in (0, 2, 4, 6) \
                    and chan in INPUT_CHANNELS:
                # READ, RAND, ROR, RXOR of an input channel the caller latches
                # (include/agc_input_channels.h): A overflow-corrected, then
                # sign-extended, as yaAGC does.
                v = 'AGC_CHANNEL(%s)' % INPUT_CHANNELS[chan]
                if io_op:
                    v = '((((W[0]>>1)&040000u)|(W[0]&037777u))%s%s)' % ({2: '&', 4: '|', 6: '^'}[io_op], v)
                self.uses_channels = True
                return st + ['ai_caf(s,(agc_word)%s);' % v], [nxt]
            if op == 0:
                # I/O channels are not in the image: the instruction (and the
                # routine from it) waits for a channel model, uncompiled.
                self.unit_words = range(0)
                return st + ['return EXEC_OUT_OF_SCOPE; /* %s %s: I/O channel */' % (ln.op, ln.operand)], []
            if op == 1 and qc == 0:
                return st + ['ai_dv(s,%s);' % eras(k10, 'dv')], [nxt]
            if op in (1, 6) and qc != 0:
                fn = 'bzf' if op == 1 else 'bzmf'
                if index is not None:
                    # INDEX K; BZMF T into a table (LLGE's INDEX WCHPHASE;
                    # BZMF WHATEXIT): as an indexed TCF (index_tcf).
                    offs = self.spec.get('index_tcf', {}).get(ln.operand.split()[0])
                    if offs is None:
                        raise CompileError('indexed %s at %s' % (fn, self.where(p)))
                    self._ix = (w, index, 0o7777)
                    cases, succ = [], [nxt]
                    for off in offs:
                        t = self.phys_of(k12 + off, bank)
                        if not self.is_basic(t):
                            raise CompileError('index_tcf %s%+d is not basic code' % (ln.operand, off))
                        cases.append('case 0%04ou:goto L%o;' % ((k12 + off) & 0o7777, t))
                        succ.append(t)
                    return st + ['if(ai_%s(s)){switch(iw&07777u){%sdefault:return EXEC_OUT_OF_SCOPE;}}' % (
                        fn, ''.join(cases))], succ
                more, succ = leave(self.phys_of(k12, bank), fn.upper())
                return st + ['if(ai_%s(s)){' % fn] + ['  ' + x for x in more] + ['}'], succ + [nxt]
            if op == 2:
                fn = ('msu', 'qxch', 'aug', 'dim')[qc]
                if fn == 'msu':
                    return st + ['msu(s,%s);' % eras(k10, fn)], [nxt]
                return st + ['ai_%s(s,%s);' % (fn, eras(k10, fn))], [nxt]
            if op in (3, 4):
                fn = 'dca' if op == 3 else 'dcs'
                if k12 >= 0o2000:
                    fp = fixed(k12) - 1
                    self.basic_fixed.add(fp)
                    return st + ['{static const agc_word k[2]={0%05o,0%05o};ai_%sf(s,k);}' % (
                        self.L.word(fp), self.L.word(fp + 1), fn)], [nxt]
                return st + ['ai_%s(s,%s);' % (fn, eras(k12 - 1, fn))], [nxt]
            if op == 6 and qc == 0:
                return st + ['ai_su(s,%s);' % eras(k10, 'su')], [nxt]
            if op == 7:
                if k12 >= 0o2000:
                    if index is not None:
                        return st + [fixed_indexed('mp')], [nxt]
                    return st + ['ai_mpf(s,0%05o);' % self.L.word(fixed(k12))], [nxt]
                return st + ['ai_mp(s,%s);' % eras(k12, 'mp')], [nxt]
        raise CompileError('unsupported basic instruction %05o (%s %s) at %s' % (w, ln.op, ln.operand, self.where(p)))

    def indexed_tc(self, p, w, index, ln, nxt, bank, tcf=False):
        """INDEX K; TC T (or TCF T): a jump table the spec's tc_tables bounds
        ('LABEL+n': entries), each entry T+i a basic word of the file. Any
        other C(K) is refused. TC leaves Q the return; TCF leaves Q alone."""
        tables = {self.label_phys(k): n for k, n in self.spec.get('tc_tables', {}).items()}
        if p not in tables and not tcf and index == 2 and (w & 0o7777) < 0o10:
            # INDEX Q; TC n: a return n words past the caller's (the
            # caller's spec 'returns' registers those words).
            return ['/* %s %s %s */' % (self.where(p), ln.op, ln.operand),
                    '{agc_word iw=ixw(s,0%05o,2u);if((iw&070000u)||(iw&07777u)<010u)return EXEC_OUT_OF_SCOPE;'
                    'ai_tc(s,0%04o);W[5]=(agc_word)(iw&07777u);return EXEC_OK;}' % (w, ln.loc + 1)], []
        if p not in tables:
            raise CompileError('indexed %s at %s' % ('TCF' if tcf else 'TC', self.where(p)))
        tz = self.phys_of(w & 0o7777, bank)
        st = ['/* %s %s %s */' % (self.where(p), ln.op, ln.operand),
              '{agc_word iw=ixw(s,0%05o,0%ou);%sswitch(iw){' % (w, index, '' if tcf else 'ai_tc(s,0%04o);' % (ln.loc + 1))]
        succ = []
        for i in range(tables[p]):
            t = tz + i
            if not self.is_basic(t):
                raise CompileError('jump table entry %s is not basic code' % self.where(t))
            st.append('  case 0%05ou:goto L%o;' % (w + i, t))
            succ.append(t)
        st.append('  default:return EXEC_OUT_OF_SCOPE;}}')
        if not tcf and self.is_basic(nxt):
            self.add_start(nxt, 'b', 'return from the %s table' % self.where(p))
            self.returns_seen.add(nxt)
            succ.append(nxt)
        return st, succ

    def is_basic(self, q):
        ln = self.code.get(q)
        return (ln is not None and ln.op and (ln.op == 'INCR' or not is_interp_op(ln.op)) and ln.op not in DATA_OPS)

    def novac(self, phys):
        """Under 'any_fixloc', a basic routine that never names FIXLOC (0120)
        and does not enter the interpreter (TC INTPRET)."""
        return (self.any_fixloc and self.starts[phys][0] == 'b'
                and not any(re.search(r'\b0120u?\b|intpret\(', st) for st in self.blocks[phys]))

    def basic_block(self, start):
        """C statements for the basic routine at start: every instruction
        reachable from it without leaving the file, in address order, with
        labels where control jumps."""
        bank = start // 1024
        self.cur_start = start
        units = {}
        self.returns_seen = set()
        work = [start]
        while work:
            p = work.pop()
            if p in units:
                continue
            if p != start and p in self.starts and self.starts[p][0] == 'i':
                raise CompileError('basic flow runs into interpretive %s' % self.where(p))
            if p != start and p in self.idle:
                units[p] = (['W[5]=0%04o;return EXEC_IDLE; /* idle point */' % self.logical_of(p)], [])
                self.unit_words = range(p, p)
                continue
            st, succ = self.basic_unit(p, bank)
            units[p] = (st, succ)
            for q in self.unit_words:
                self.owner.setdefault(q, start)
            work.extend(succ)
        order = sorted(units)
        order.remove(start)
        order.insert(0, start)
        tcq = 'return tc_q(s);'
        if self.returns_seen:
            tcq = 'switch(W[2]){%sdefault:return tc_q(s);}' % ''.join(
                'case 0%04ou:W[2]=3u;goto L%o;' % (self.logical_of(r), r) for r in sorted(self.returns_seen))
        out = []
        for i, p in enumerate(order):
            st, succ = units[p]
            st = [x.replace('@TCQ@', tcq) for x in st]
            out.append('L%o:;' % p)
            out.extend(st)
            last = st[-1].strip()
            if last.startswith('goto ') or last.startswith('switch(') or last.startswith('return ') or last.endswith('return EXEC_OK;') or 'return EXEC_OK; /*' in last:
                continue
            fall = [q for q in succ if q not in self._jumps(st)]
            if fall:
                f = fall[0]
                if i + 1 < len(order) and order[i + 1] == f:
                    continue
                out.append('goto L%o;' % f)
        if start in self.idle:
            # Back at the idle point the routine started from: the spin.
            back = 'W[5]=0%04o;return EXEC_IDLE; /* idle point */' % self.logical_of(start)
            out = [x.replace('goto L%o;' % start, back) for x in out]
        # Labels nothing jumps to are dropped (-Wunused-label).
        used = set()
        for x in out:
            used.update(self._jumps([x]))
        return [x for x in out if not (x.startswith('L') and x.endswith(':;') and int(x[1:-2], 8) not in used)]

    @staticmethod
    def _jumps(stmts):
        import re
        out = set()
        for x in stmts:
            out.update(int(m, 8) for m in re.findall(r'goto L([0-7]+);', x))
        return out

    def source_map(self):
        """Every emitted word of the file: the routine that compiles it, the
        fixed operand (K array) it is a word of, or neither."""
        const = {}
        for fp in self.consts:
            for i in range(self.const_len.get(fp, 6)):
                const.setdefault(fp + i, fp)
        for lo, (n, lab) in self.ktabs.items():
            for i in range(n):
                const.setdefault(lo + i, 'X%o' % lo)
        lists = {lp + i: lp for lp, n in self.lists.items() for i in range(n)}
        # Basic routines the spec implements by hand (reached through RTB):
        # 'native_ranges' {first label: (word count, C function)}.
        native = {}
        for lab, (n, fn) in self.spec.get('native_ranges', {}).items():
            if self.label_phys(lab) is None:
                continue   # (a label only the other mission has)
            for i in range(n):
                native[self.label_phys(lab) + i] = fn
        rows = []
        for ln in self.lines:
            for i in range(len(ln.words)):
                q = ln.phys + i
                if self.owner.get(q) == 'basic':
                    # Basic words compile into the file's one basic_run.
                    use = {'routine': 'basic code', 'function': self.basic.run_name(q // 1024)}
                elif q in self.owner:
                    use = {'routine': self.where(self.owner[q]), 'function': 'r%o' % self.owner[q]}
                elif q in native:
                    use = {'routine': 'native', 'function': native[q]}
                elif q in lists:
                    use = {'cadr_list': self.where(lists[q])}
                elif q in const:
                    use = {'constant': 'K%s' % (const[q] if isinstance(const[q], str) else '%o' % const[q])}
                elif q in self.basic_fixed:
                    use = {'constant': 'basic operand (inline)'}
                elif self.basic and q in self.basic.operands:
                    use = {'constant': 'inline operand of an outside call'}
                elif self.basic and ln.op in self.DATA_OPS:
                    use = {'constant': 'data no instruction of the file reads (another file or an '
                                       'interrupt table may)'}
                elif ln.op in DATA_OPS:
                    use = {'constant': 'data word (no instruction in the file reads it)'}
                else:
                    use = {}
                rows.append(dict(address=self.where(q), word='%05o' % self.L.word(q),
                                 source='%s %s %s' % (ln.label, ln.op, ln.operand) if i == 0 else '(+%d)' % i, **use))
        return rows

    # ---- output ----------------------------------------------------------
    def emit(self):
        s = self.spec
        base = s['base'][self.m]
        sym = s['symbol'][self.m]
        hdr = base.upper() + '_H'
        P = self.p
        prog = 'Luminary099' if self.m == 'lm' else 'Comanche055'
        c = []
        c.append('/* SPDX-License-Identifier: MIT */')
        c.append('/* Generated by tools/interp_compile.py from the %s assembly listing:' % prog)
        c.append(' * do not edit. %s/%s as native routines of the' % (prog, s['file'][self.m]))
        c.append(' * control-transfer ABI. Each order pair is the compiled interpreter path')
        c.append(' * with its literal order and operand words; fixed operands are the words')
        c.append(' * of the rope at the addresses they name. */')
        c.append('#include "%s.h"' % base)
        c.append('#include "%s"' % ('lm_interp_steps.h"\n#include "lm_interp_orders.h' if self.m == 'lm' else 'cm_interp.h'))
        c.append('#include "agc_image_ops.h"')
        if self.uses_channels:
            c.append('#include "agc_input_channels.h"')
        for inc in s.get('includes', []):
            c.append('#include "%s"' % inc)
        c.append('#define W s->words')
        c.append('/* A group build (KEEP masks below) leaves some helpers and operands unused. */')
        c.append('#ifdef __GNUC__')
        c.append('#define MAYBE_UNUSED __attribute__((unused))')
        c.append('#else')
        c.append('#define MAYBE_UNUSED')
        c.append('#endif')
        c.append('#ifdef INTERP_ROUTINE_TRACE')
        c.append('void interp_routine_trace(const apollo_overlay_state *);')
        c.append('#define TRACE(s) interp_routine_trace(s)')
        c.append('#else')
        c.append('#define TRACE(s) ((void)0)')
        c.append('#endif')
        c.append('')
        if self.consts:
            c.append('/* Fixed operands (the rope words from each address). */')
            for fp in sorted(self.consts):
                n = self.const_len.get(fp, 6)
                ws = ','.join('0%05o' % self.L.word(fp + i) for i in range(n))
                c.append('static const agc_word K%o[%d] MAYBE_UNUSED={%s}; /* %s %s */' % (fp, n, ws, self.where(fp), self.consts[fp] or ''))
            c.append('')
        if self.kx_used:
            c.append('/* Fixed tables indexed orders read: rope windows around their bases. */')
            for lo in sorted(self.kx):
                n, fp = self.kx[lo]
                c.append('static const agc_word KX%o[%d] MAYBE_UNUSED={%s}; /* %s.. (%s %s) */' % (
                    lo, n, ','.join('0%05o' % self.L.word(lo + i) for i in range(n)), self.where(lo), self.where(fp), self.name(fp)))
            c.append('/* The operand an indexed order reads: 0 for erasable (ADDRWD below')
            c.append(' * 02000), else its words in the window k (FB and ADDRWD as the')
            c.append(' * fetch left them); a word outside the window is refused. */')
            c.append('static MAYBE_UNUSED const agc_word *kx(const apollo_overlay_state *s,const agc_word *k,uint16_t lo,uint16_t n,uint16_t need,uint8_t *bad){')
            c.append(' unsigned p;if(W[0116]<02000u)return 0;')
            c.append(' p=((W[4]>>10)&037u)*1024u+(W[0116]&01777u);')
            c.append(' if(p<lo||p+need>(unsigned)lo+n){*bad=1;return 0;}')
            c.append(' return k+(p-lo);')
            c.append('}')
        if self.ktables:
            c.append('/* Indexed fixed operands: the rope words over the range the index reaches. */')
            for lo in sorted(self.ktables):
                n, lab, rng = self.ktables[lo]
                c.append('static const agc_word KX%o[%d] MAYBE_UNUSED={%s}; /* %s %s%+d..%+d and five more */' % (
                    lo, n, ','.join('0%05o' % self.L.word(lo + i) for i in range(n)), self.where(lo), lab, rng[0], rng[1]))
            c.append('/* The operand words at the indexed address (FBANK, ADDRWD) when it is')
            c.append(' * fixed: k into the table, 1 when outside it; 0 for erasable (k = 0). */')
            c.append('static MAYBE_UNUSED uint8_t kx(const apollo_overlay_state *s,const agc_word *t,uint16_t first,uint16_t n,const agc_word **k){')
            c.append(' unsigned a=W[0116],p;')
            c.append(' if(a<02000u){*k=0;return 0;}')
            c.append(' p=((W[4]>>10)&037u)*1024u+(a-02000u);')
            c.append(' if(p<first||p+6u>(unsigned)first+n)return 1;')
            c.append(' *k=t+(p-first);return 0;')
            c.append('}')
            c.append('')
        if self.ktabs:
            c.append('/* Fixed tables indexed operands reach (the rope words). */')
            for lo in sorted(self.ktabs):
                n, lab = self.ktabs[lo]
                ws = ','.join('0%05o' % self.L.word(lo + i) for i in range(n))
                c.append('static const agc_word KX%o[%d] MAYBE_UNUSED={%s}; /* %s.. (%s) */' % (lo, n, ws, self.where(lo), lab))
            c.append('/* The indexed operand at ADDRWD: erasable (k = 0), or within the')
            c.append(' * table (k into it); anything else is out of scope. */')
            c.append('static MAYBE_UNUSED uint8_t kix(const apollo_overlay_state *s,const agc_word *t,uint16_t lo,uint16_t n,const agc_word **k){')
            c.append(' unsigned a=W[0116]&077777u,b=(W[4]>>10)&037u,p;')
            c.append(' if(a<02000u){*k=0;return 1;}')
            c.append(' if(a>=04000u||b>=030u)return 0;')
            c.append(' p=b*1024u+(a-02000u);if(p<lo||p+6u>(unsigned)lo+n)return 0;')
            c.append(' *k=t+(p-lo);return 1;')
            c.append('}')
            c.append('')
        if self.rk:
            ch2 = self.L.labels['CHANG2']
            assert ch2 // 1024 in (2, 3)
            ch2 = self.logical_of(ch2)
            c.append('/* DANZIG at the end of a pair: NEWJOB +0 goes on to NEWOPS; positive')
            c.append(' * goes to CHANG2 (CA BANKSET; TS BBANK; CCS EDOP; CCS NEWJOB; TCF')
            c.append(' * CHANG2) and the routine stops there. */')
            c.append('static MAYBE_UNUSED uint8_t next(apollo_overlay_state *s){')
            c.append(' if(!W[067]){%sdanzig_next(s);return 0;}' % P)
            c.append(' ai_ca(s,0165);(void)ai_ts(s,6);(void)ai_ccs(s,023);(void)ai_ccs(s,067);W[5]=0%04ou;return 1;' % ch2)
            c.append('}')
        else:
            c.append('static MAYBE_UNUSED void next(apollo_overlay_state *s){%sdanzig_next(s);}' % P)
        c.append('static MAYBE_UNUSED void right(apollo_overlay_state *s){%sdanzig_right(s);}' % P)
        if self.uses_channels:
            c.append('/* The input channels of the run (READ, RAND, ROR, RXOR), set by the wrappers. */')
            c.append('static MAYBE_UNUSED const agc_input_channels *agc_channels_in;')
        if self.sload_k:
            c.append('/* SLOAD of a fixed word: ZL; INDEX ADDRWD, CA 0; SLOAD2 DXCH MPAC;')
            c.append(' * CAF ZERO; TS MPAC +2; NEWMODE TS MODE; DANZIG. */')
            c.append('static MAYBE_UNUSED void sload_k(apollo_overlay_state *s,const agc_word *k){')
            c.append(' W[1]=0;ai_caf(s,k[0]);ai_dxch(s,0154);ai_caf(s,0);(void)ai_ts(s,0156);(void)ai_ts(s,0163);W[5]=0%04ou;' % self.logical_of(self.L.labels['DANZIG']))
            c.append('}')
        c.append('/* SETPD as the last order of a pair: NOIBNKSW CCS EDOP (+0); CCS NEWJOB')
        c.append(' * (+0); INCR LOC; NEWOPS. */')
        if self.rk:
            c.append('static MAYBE_UNUSED uint8_t setpd_next(apollo_overlay_state *s){')
            c.append(' (void)ai_ccs(s,023);if(!W[067]){(void)ai_ccs(s,067);ai_incr(s,0164);W[5]=0%04o;return 0;}' % self.newops)
            c.append(' (void)ai_ccs(s,067);W[5]=0%04ou;return 1;' % ch2)
            c.append('}')
        else:
            c.append('static MAYBE_UNUSED void setpd_next(apollo_overlay_state *s){(void)ai_ccs(s,023);(void)ai_ccs(s,067);ai_incr(s,0164);W[5]=0%04o;}' % self.newops)
        c.append('static MAYBE_UNUSED void shorts(apollo_overlay_state *s){%sshort_dispatch(s);if(W[5]==0%04ou)%sshortt(s);else %sshortv(s);}' % (P, self.shortt, P, P))
        c.append('')
        c.append('/* The image as the routines need it: ordinary words (A, L and Q may')
        c.append(' * hold 16 bits), coherent banks, NEWJOB %s, FIXLOC a VAC area and the' % ('+0 or positive' if self.rk else '+0'))
        c.append(' * latches in range. */')
        c.append('static MAYBE_UNUSED uint8_t sane(const apollo_overlay_state *s,const p66_bias_io *io){')
        c.append(' unsigned i,vac=W[0120];')
        c.append(' if(W[7]||%s||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)((W[4]&076000u)|(W[3]>>8)))return 0;'
                 % ('W[067]>037777u' if self.rk else 'W[067]'))
        c.append(' if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;')
        c.append(' if(vac<0401u||vac>0661u||(vac-0401u)%054u)return 0;')
        c.append(' for(i=3;i<2048u;++i)if(W[i]>AGC_MASK)return 0;')
        c.append(' return 1;')
        c.append('}')
        if self.any_fixloc:
            c.append('/* The same for a basic routine that never names FIXLOC: any FIXLOC. */')
            c.append('static MAYBE_UNUSED uint8_t sane_nv(const apollo_overlay_state *s,const p66_bias_io *io){')
            c.append(' unsigned i;')
            c.append(' if(W[7]||%s||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)((W[4]&076000u)|(W[3]>>8)))return 0;'
                     % ('W[067]>037777u' if self.rk else 'W[067]'))
            c.append(' if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;')
            c.append(' for(i=3;i<2048u;++i)if(W[i]>AGC_MASK)return 0;')
            c.append(' return 1;')
            c.append('}')
        c.append('/* An interpretive routine at NEWOPS of its bank (LOC was matched by the')
        c.append(' * trampoline): BANKSET is BBANK, FB its bank, INTBIT15 its bit 15. */')
        # Only a file in banks 030-043 can depend on the superbank bit.
        if self.high_banks:
            c.append('/* Banks 040-043 are FB 030-033 with the superbank bit (channel 7 0100);')
            c.append(' * FB 030-037 without it are banks 030-037. fb carries 1 in bit 0 for a')
            c.append(' * superbank bank. */')
            c.append('static MAYBE_UNUSED uint8_t sb_ok(const p66_bias_io *io,agc_word fb){')
            c.append(' if((fb&060000u)!=060000u)return 1;')
            c.append(' return (fb&1u)?(io->superbank&0100u)!=0:(io->superbank&0100u)==0;')
            c.append('}')
        c.append('static MAYBE_UNUSED uint8_t at_newops(const apollo_overlay_state *s,const p66_bias_io *io,agc_word fb){')
        sb = 'W[4]==(fb&076000u)&&sb_ok(io,fb)' if self.high_banks else 'W[4]==fb'
        c.append(' return W[5]==0%04ou&&%s&&W[0165]==W[6]&&W[0115]==(fb&040000u)&&sane(s,io);' % (self.newops, sb))
        c.append('}')
        c.append('/* INDEX K: C(K) (A, L, Q overflow-corrected) added to the instruction')
        c.append(' * word w in 16-bit ones\' complement, then overflow-corrected. */')
        c.append('static MAYBE_UNUSED agc_word ixw(const apollo_overlay_state *s,agc_word w,unsigned k){')
        c.append(' unsigned i=k<3u?((W[k]&037777u)|((W[k]>>1)&040000u)):ai_word(s,k),a,b,sum;')
        if self.spec.get('index_carry16'):
            # (Spec 'index_carry16': the end-around carry without a 17-bit
            # sum, so a C64 build, where unsigned is 16 bits, keeps it.)
            c.append(' a=(i&040000u)?(i|0100000u):i;b=(w&040000u)?(w|0100000u):w;sum=(a+b)&0177777u;')
            c.append(' if(sum<a)sum=(sum+1u)&0177777u;')
        else:
            c.append(' a=(i&040000u)?(i|0100000u):i;b=(w&040000u)?(w|0100000u):w;sum=a+b;')
            c.append(' if(sum&0200000u)sum=(sum+1u)&0177777u;')
        c.append(' return (agc_word)(((sum&037777u)|((sum>>1)&040000u))&077777u);')
        c.append('}')
        c.append('/* MSU K: two\'s-complement A - C(K) to ones\' complement (no agc_image_ops form). */')
        c.append('static MAYBE_UNUSED void msu(apollo_overlay_state *s,unsigned k){')
        c.append(' unsigned ui,uj,d;agc_word v;')
        c.append(' if(k<3u){ui=W[0]&0177777u;uj=~(unsigned)W[k]&0177777u;}')
        c.append(' else{ui=((W[0]&037777u)|((W[0]>>1)&040000u))&077777u;uj=~(unsigned)ai_word(s,k)&077777u;}')
        c.append(' d=ui+uj+1u;if(d&040000u){d|=0100000u;d--;}')
        c.append(' if(k==2u)W[0]=(agc_register)(d&0177777u);')
        c.append(' else{v=(agc_word)(d&077777u);W[0]=(agc_register)((v&040000u)?(v|0100000u):v);}')
        c.append(' if(k>=020u&&k<=023u)ai_put(s,k,ai_word(s,k));')
        c.append(' W[1]=(agc_register)((((W[1]&037777u)|((W[1]>>1)&040000u))&040000u)?(((W[1]&037777u)|((W[1]>>1)&040000u))|0100000u):((W[1]&037777u)|((W[1]>>1)&040000u)));')
        c.append('}')
        c.append('/* TC Q: the word in Q runs as an instruction; a TC leaves Q = 3. */')
        c.append('static MAYBE_UNUSED enum exec_status tc_q(apollo_overlay_state *s){')
        c.append(' agc_word q=W[2];')
        c.append(' if((q&070000u)||q<010u)return EXEC_OUT_OF_SCOPE;')
        c.append(' W[2]=3u;W[5]=(agc_word)(q&07777u);return EXEC_OK;')
        c.append('}')
        c.append('/* TC K, K erasable: C(K) runs as an instruction; a TC leaves Q = K+1. */')
        c.append('static MAYBE_UNUSED enum exec_status tc_e(apollo_overlay_state *s,uint16_t k){')
        # Switched erasable (01400-01777) is read through EB.
        c.append(' agc_word w=%s;' % ('ai_word(s,k)' if any(k >= 0o1400 for k in self.tc_erasable) else 'W[k]'))
        c.append(' if((w&070000u)||w<010u)return EXEC_OUT_OF_SCOPE;')
        c.append(' W[2]=(agc_word)(k+1u);W[5]=(agc_word)(w&07777u);return EXEC_OK;')
        c.append('}')
        c.append('static MAYBE_UNUSED uint8_t at_basic(const apollo_overlay_state *s,const p66_bias_io *io,agc_word fb){')
        c.append(' return %s&&sane(s,io);' % sb)
        c.append('}')
        if self.any_fixloc:
            c.append('static MAYBE_UNUSED uint8_t at_basic_nv(const apollo_overlay_state *s,const p66_bias_io *io,agc_word fb){')
            c.append(' return %s&&sane_nv(s,io);' % sb)
            c.append('}')
        c.append('')
        if self.basic:
            c.extend(self.basic.emit(sym))
            c.append('/* A basic routine: FB its bank (banks 30-37 with SUPERBNK clear), coherent')
            c.append(' * banks, ordinary words (A, L and Q may hold 16 bits), latches in range. */')
            c.append('static MAYBE_UNUSED uint8_t basic_sane(const apollo_overlay_state *s,const cm_native_io *io,agc_word fb){')
            c.append(' unsigned i;')
            if any(p // 1024 >= 0o40 for p in self.blocks if self.starts[p][0] == 'b'):
                c.append(' /* fb bit 15: a bank 40-43 routine, FB 30-33 with SUPERBNK 0100 */')
                c.append(' agc_word sb=(agc_word)(fb>>15);fb&=077777u;')
                c.append(' if(W[4]!=fb||W[7]||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)((W[4]&076000u)|(W[3]>>8)))return 0;')
                c.append(' if(io->interrupt_enabled>1u||(io->superbank&~0160u)||(sb?(io->superbank&0160u)!=0100u:(fb>=060000u&&(io->superbank&0100u))))return 0;')
            else:
                c.append(' if(W[4]!=fb||W[7]||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)((W[4]&076000u)|(W[3]>>8)))return 0;')
                c.append(' if(io->interrupt_enabled>1u||(io->superbank&~0160u)||(fb>=060000u&&(io->superbank&0100u)))return 0;')
            c.append(' for(i=3;i<2048u;++i)if(W[i]>AGC_MASK)return 0;')
            c.append(' return 1;')
            c.append('}')
            c.append('')
        self.erasable_ents = []
        if self.basic:
            ec, self.erasable_ents = self.basic.erasable_points()
            c.extend(ec)
        routines = []
        keep = sym.upper() + '_KEEP'
        nwords = (len(self.blocks) + 59) // 60
        c.append('/* %s<n> bit i (i = 60n+bit, the order of %s_points) keeps routine i;' % (keep, sym))
        c.append(' * a build that defines them keeps only a group (the C64 diagnostics). */')
        for n in range(nwords):
            c.append('#ifndef %s%d' % (keep, n))
            c.append('#define %s%d 0xfffffffffffffff' % (keep, n))
            c.append('#endif')
        def guard(i):
            return '#if (%s%d >> %d) & 1' % (keep, i // 60, i % 60)
        for i, phys in enumerate(sorted(self.blocks)):
            kind, why = self.starts[phys]
            lab = self.name(phys)
            fn = 'r%o' % phys
            routines.append((phys, kind, fn, lab, why))
            bank = phys // 1024
            # Banks 040-043: FB 030-033 under the superbank (bit 0 marks it).
            fb = ((bank - 8) << 10) | 1 if bank >= 0o40 else bank << 10
            # basic_sane's superbank form: FB 30-33 with bit 15 for SUPERBNK 0100.
            bfb = (bank - 0o10) << 10 | 0o100000 if bank >= 0o40 else bank << 10
            c.append(guard(i))
            c.append('/* %s %s (%s) */' % (self.where(phys), lab, why))
            if kind == 'b' and self.basic:
                c.append('static enum exec_status %s_abi(const agc_native_program *p,apollo_overlay_state *s,void *v){' % fn)
                c.append(' (void)p;if(!s||!v||!basic_sane(s,(cm_native_io *)v,0%05ou))return EXEC_OUT_OF_SCOPE;' % bfb)
                if self.uses_channels:
                    c.append(' agc_channels_in=&((cm_channel_io *)v)->in; /* cm_channel_io: the record, then the inputs */')
                c.append(' return %s(s,(cm_native_io *)v,%uu);' % (self.basic.run_name(phys // 1024), phys))
                c.append('}')
                c.append('#endif')
                continue
            c.append('static enum exec_status %s(apollo_overlay_state *s,p66_bias_io *io){' % fn)
            nv = '_nv' if self.novac(phys) else ''
            if kind == 'b' and phys // 1024 in (2, 3):
                c.append(' if(!sane%s(s,io))return EXEC_OUT_OF_SCOPE;' % nv)
            else:
                chk = 'at_newops' if kind == 'i' else 'at_basic' + nv
                if any('goto top;' in st for st in self.blocks[phys]):
                    c.append(' uint16_t loops=0;')
                    c.append(' top:')
                    c.append(' if(!%s(s,io,0%05ou))return loops?EXEC_OK:EXEC_OUT_OF_SCOPE;' % (chk, fb))
                else:
                    c.append(' if(!%s(s,io,0%05ou))return EXEC_OUT_OF_SCOPE;' % (chk, fb))
            for st in self.blocks[phys]:
                c.append(' ' + st)
            c.append('}')
            c.append('#endif')
        c.append('')
        # Wrappers: the mission's io record.
        if self.m == 'lm' and self.uses_channels:
            c.append('/* The routines take lm_channel_io: the mission record, then the inputs. */')
            c.append('#define WRAP(f) static enum exec_status f##_abi(const agc_native_program *p,apollo_overlay_state *s,void *v){(void)p;if(!s||!v)return EXEC_OUT_OF_SCOPE;agc_channels_in=&((lm_channel_io *)v)->in;return f(s,&((lm_channel_io *)v)->io);}')
        elif self.m == 'lm':
            c.append('#define WRAP(f) static enum exec_status f##_abi(const agc_native_program *p,apollo_overlay_state *s,void *v){(void)p;return s&&v?f(s,(p66_bias_io *)v):EXEC_OUT_OF_SCOPE;}')
        else:
            c.append('/* Comanche routines take the two latches the bodies use from the')
            c.append(' * mission\'s cm_native_io and copy them back. */')
            c.append('static %senum exec_status latched(enum' % ('MAYBE_UNUSED ' if self.basic else '') + ' exec_status (*f)(apollo_overlay_state *,p66_bias_io *),apollo_overlay_state *s,void *v){')
            c.append(' cm_native_io *io=v;p66_bias_io l;enum exec_status st;')
            c.append(' if(!s||!io)return EXEC_OUT_OF_SCOPE;')
            if self.uses_channels:
                c.append(' agc_channels_in=&((cm_channel_io *)v)->in; /* cm_channel_io: the record, then the inputs */')
            c.append(' l.superbank=io->superbank;l.interrupt_enabled=io->interrupt_enabled;st=f(s,&l);')
            c.append(' io->superbank=l.superbank;io->interrupt_enabled=l.interrupt_enabled;return st;')
            c.append('}')
            c.append('#define WRAP(f) static enum exec_status f##_abi(const agc_native_program *p,apollo_overlay_state *s,void *v){(void)p;return latched(f,s,v);}')
        index = {r[0]: i for i, r in enumerate(routines)}
        for i, (phys, kind, fn, lab, why) in enumerate(routines):
            if kind == 'b' and self.basic:
                continue
            c.append(guard(i))
            c.append('WRAP(%s)' % fn)
            c.append('#endif')
        c.append('/* Each table ends with a sentinel the count leaves out. */')
        for kind, tab in (('i', 'interpretive'), ('b', 'basic')):
            rs = [r for r in routines if r[1] == kind]
            if not rs:
                c.append('const agc_routine_table %s_%s={0,0};' % (sym, tab))
                continue
            c.append('static const agc_routine_entry %s_entries[]={' % tab)
            for phys, kind, fn, lab, why in rs:
                c.append(guard(index[phys]))
                c.append(' {%u,%s_abi},' % (phys, fn))
                c.append('#endif')
            if kind == 'b' and self.basic:
                for key, fn in self.erasable_ents:
                    c.append(' {%u,%s_abi},' % (key, fn))
            c.append(' {0xffffu,0}')
            c.append('};')
            c.append('const agc_routine_table %s_%s={%s_entries,(uint16_t)(sizeof %s_entries/sizeof %s_entries[0]-1u)};'
                     % (sym, tab, tab, tab, tab))
        # Test metadata.
        c.append('const %s_point %s_points[%s_POINTS]={' % (sym, sym, sym.upper()))
        c.append(',\n'.join(' {%u,%u}' % (phys, 1 if kind == 'i' else 0) for phys, kind, fn, lab, why in routines))
        c.append('};')
        ce = sorted(self.cadr_erasable)
        c.append('const agc_word %s_cadr_words[%s_CADR_WORDS]={%s};' % (sym, sym.upper(), ','.join('0%04o' % x for x in ce) or '0'))
        ex = sorted(set(self.externals) | set(self.externals_any))
        c.append('const agc_word %s_externals[%s_EXTERNALS]={%s};' % (sym, sym.upper(), ','.join('0%04o' % x for x in ex) or '0'))
        h = []
        h.append('/* SPDX-License-Identifier: MIT */')
        h.append('/* Generated by tools/interp_compile.py: do not edit. */')
        h.append('#ifndef %s' % hdr)
        h.append('#define %s' % hdr)
        h.append('#include "agc_transfer.h"')
        h.append('#include "p66_bias.h"')
        if self.m == 'cm':
            h.append('#include "cm_native_image.h"')
        h.append('/* %s/%s: interpretive routines keyed by the physical' % (prog, s['file'][self.m]))
        h.append(' * LOC at NEWOPS (%04o), basic ones by physical Z. */' % self.newops)
        h.append('extern const agc_routine_table %s_interpretive,%s_basic;' % (sym, sym))
        h.append('/* Every routine (physical address, 1 interpretive), the erasable')
        h.append(' * words its GOTOs and CALLs read a CADR from (work-area offsets below')
        h.append(' * 045), and the outside basic addresses it transfers to. */')
        h.append('typedef struct {uint16_t physical;uint8_t interpretive;} %s_point;' % sym)
        h.append('#define %s_POINTS %du' % (sym.upper(), len(routines)))
        h.append('#define %s_CADR_WORDS %du' % (sym.upper(), max(1, len(ce))))
        h.append('#define %s_EXTERNALS %du' % (sym.upper(), max(1, len(ex))))
        h.append('#define %s_FIRST %uu' % (sym.upper(), self.lo))
        h.append('#define %s_LAST %uu' % (sym.upper(), self.hi))
        h.append('extern const %s_point %s_points[%s_POINTS];' % (sym, sym, sym.upper()))
        h.append('extern const agc_word %s_cadr_words[%s_CADR_WORDS];' % (sym, sym.upper()))
        h.append('extern const agc_word %s_externals[%s_EXTERNALS];' % (sym, sym.upper()))
        h.append('#endif')
        return '\n'.join(c) + '\n', '\n'.join(h) + '\n'


def main(argv):
    check = '--check' in argv
    names = [a for a in argv[1:] if not a.startswith('--')]
    bad = 0
    for n in names:
        spec = interp_specs.SPECS[n]
        for m in spec['missions']:
            cp = Compiler(spec, m)
            cp.decode()
            csrc, hsrc = cp.emit()
            base = spec['base'][m]
            for path, text in (('src/%s.c' % base, csrc), ('include/%s.h' % base, hsrc)):
                full = os.path.join(HERE, path)
                if check:
                    old = open(full).read() if os.path.exists(full) else None
                    if old != text:
                        print('%s is not what tools/interp_compile.py generates' % path)
                        bad = 1
                else:
                    open(full, 'w').write(text)
                    print('wrote %s (%d routines)' % (path, len(cp.blocks)))
    return bad


if __name__ == '__main__':
    sys.exit(main(sys.argv))
