# SPDX-License-Identifier: MIT
"""Basic-code half of tools/interp_compile.py for files that mix basic and
interpretive code (spec 'basic': 'full').

Every basic word a file can execute from its basic starts (entries, returns
from outside calls, EXIT points) is decoded from the assembled word itself,
with the EXTEND and INDEX state it is reached in, and becomes one
agc_image_ops call with its literal operand (a fixed operand is the rope
word, written as a literal). All basic code of the file is one C function
with a label per word; each basic start is a routine of the transfer ABI
that enters it at its label. Branches inside the file are gotos; TC Q,
DTCB, TC A and INDEX'd TC/TCF dispatch on the computed Z to the file's
labels, or leave the file with Z there. A TC or TCF to an outside address
leaves with Z (and Q) as the original sets them; the trampoline continues
there. The word after an outside TC's inline operands is a start (the
return). TS overflow skips and all four CCS exits go where the original's
go, decoded the same way.

INDEX takes the operand's signed value (A, L and Q as 16-bit registers
corrected to 15 bits, -0 as 0) and adds it to the next word's address; an
address outside the order's field (which would change the order) is
EXEC_INVALID_ACTION, as is a fixed read outside the spec's 'fixed_tables'.
"""

import collections
import re

QUARTER = {1: 'CCS', 2: ('DAS', 'LXCH', 'INCR', 'ADS'), 5: ('INDEX', 'DXCH', 'TS', 'XCH')}
EXT_QUARTER = {1: 'DV', 2: ('MSU', 'QXCH', 'AUG', 'DIM'), 6: 'SU'}
CHANNEL_OPS = ('READ', 'WRITE', 'RAND', 'WAND', 'ROR', 'WOR', 'RXOR', 'EDRUPT')
# Erasable-reading orders and their fixed-operand forms.
FIXED_FORM = {'CA': 'caf', 'CS': 'csf', 'AD': 'adf', 'MASK': 'maskf', 'MP': 'mpf', 'SU': 'suf',
              'DCA': 'dcaf', 'DCS': 'dcsf', 'CCS': 'ccsf', 'DV': 'dvf'}
DOUBLE = ('DCA', 'DCS', 'DXCH', 'DAS')


class BasicError(Exception):
    pass


# Input channels a 'channel_inputs' spec reads from the caller's latches
# (as tools/interp_compile.py's INPUT_CHANNELS).
INPUT_CHANNELS = {0o3: '3', 0o4: '4', 0o16: '16', 0o30: '30', 0o31: '31', 0o32: '32', 0o33: '33'}
# The channels a Luminary 'channels': 'all' file's orders reach, by order
# (bits 10-12: READ, WRITE, RAND, WAND, ROR, WOR, RXOR, EDRUPT), as
# include/agc_channel_ops.h models them: the outputs in the program's
# agc_hardware, the latched inputs and SUPERBNK (7).
LM_OUTPUTS = {0o5, 0o6, 0o11, 0o12, 0o13, 0o14}
LM_READS = LM_OUTPUTS | {0o3, 0o4, 0o7, 0o16, 0o30, 0o31, 0o32, 0o33}
LM_WRITES = LM_OUTPUTS | {0o7, 0o30, 0o31, 0o32, 0o33}
LM_CHANNELS = (LM_READS, LM_WRITES, LM_READS, LM_WRITES, LM_READS, LM_WRITES, LM_READS, set())


def decode(w, ext):
    """(op, address field, field limit) of word w (EXTEND state ext)."""
    code = (w >> 12) & 7
    qc = (w >> 10) & 3
    if not ext:
        if code == 0:
            if w == 3:
                return 'RELINT', None, None
            if w == 4:
                return 'INHINT', None, None
            if w == 6:
                return 'EXTEND', None, None
            return 'TC', w & 0o7777, 0o7777
        if code == 1:
            if qc == 0:
                return 'CCS', w & 0o1777, 0o1777
            return 'TCF', w & 0o7777, 0o7777
        if code in (2, 5):
            op = QUARTER[code][qc]
            if code == 5 and qc == 0 and (w & 0o1777) == 0o17:
                return 'RESUME', None, None
            return op, w & 0o1777, 0o1777
        return {3: 'CA', 4: 'CS', 6: 'AD', 7: 'MASK'}[code], w & 0o7777, 0o7777
    if code == 0:
        return CHANNEL_OPS[(w >> 9) & 7], w & 0o777, 0o777
    if code == 1:
        if qc == 0:
            return 'DV', w & 0o1777, 0o1777
        return 'BZF', w & 0o7777, 0o7777
    if code == 2:
        return EXT_QUARTER[2][qc], w & 0o1777, 0o1777
    if code == 6:
        if qc == 0:
            return 'SU', w & 0o1777, 0o1777
        return 'BZMF', w & 0o7777, 0o7777
    return {3: 'DCA', 4: 'DCS', 5: 'INDEX', 7: 'MP'}[code], w & 0o7777, 0o7777


class Basic:
    def __init__(self, cp):
        self.cp = cp
        # The mission's io record: Luminary routines take p66_bias_io, Comanche
        # ones cm_native_io.
        self.io = 'p66_bias_io' if cp.m == 'lm' else 'cm_native_io'
        self.L = cp.L
        self.spec = cp.spec
        self.nodes = {}          # (phys, ext) -> list of C statements (or None while walking)
        self.dispatch = set()    # phys reachable by a computed transfer (labels)
        self.entries = set()     # basic starts
        # Words an INDEX can turn into another order (spec 'index_words'
        # {'bank,addr': [index values]}).
        self.index_words = {}
        for k, vals in self.spec.get('index_words', {}).items():
            b, a = k.split(',')
            self.index_words[int(b, 8) * 1024 + int(a, 8) - 0o2000] = vals
        # The word EB holds under the spec's fixed 'ebank', when it names
        # the file's own code (a TC through register 3).
        self.eb_word = None
        if isinstance(self.spec.get('ebank'), int) and (self.spec['ebank'] << 8) >= 0o2000:
            self.eb_word = self.spec['ebank'] << 8
        # Indexed TC/TCF words whose sum stays in the file (spec
        # 'index_targets' {'bank,addr': largest index}).
        self.index_targets = {}
        for k, n in self.spec.get('index_targets', {}).items():
            b, a = k.split(',')
            self.index_targets[int(b, 8) * 1024 + int(a, 8) - 0o2000] = n
        # Erasable cells holding an order code an INDEX adds to the next
        # word (spec 'index_cells' {logical address: [values]}).
        self.index_cells = {k: v
                            for k, v in self.spec.get('index_cells', {}).items()}
        self.fixed_tables = []   # (lo phys, hi phys)
        for lab, (lo, hi) in self.spec.get('fixed_tables', {}).items():
            # A label, or the fixed-fixed logical address (octal) of an
            # EQUALS name such as LIMITS.
            b = self.L.labels[lab] if lab in self.L.labels else 2048 + int(lab, 8) - 0o4000
            self.fixed_tables.append((b + lo, b + hi, lab, lo))
            self.cp.basic_fixed.update(range(b + lo, b + hi + 1))
        self.channels = self.spec.get('channels', {})
        # Luminary 'all': include/agc_channel_ops.h (no per-file helpers).
        self.lm_channels = self.channels == 'all' and self.io == 'p66_bias_io'
        if self.lm_channels:
            self.channels = {}
        elif self.channels == 'all':
            # Every channel of the CM io record (include/cm_native_image.h),
            # and 5, 6 and 30-33 in the program's agc_hardware (set by the
            # wrappers, which refuse a program without one).
            self.channels = {k: 'channel%02o' % k for k in (0o11, 0o12, 0o13, 0o14)}
            self.channels.update({k: 'agc_channels_hw->channel%o' % k for k in (0o5, 0o6, 0o30, 0o31, 0o32, 0o33)})
            self.cp.uses_hw = True
            # SUPERBNK (7) too, only in files that name it (EXTENDED_VERBS'
            # GOLOADLV), so no other file's helpers change.
            self.cm_superbank = True
        self.work = []
        self.words = set()       # every word compiled as basic code
        self.operands = set()    # inline operand words of outside TCs
        self.data_landings = set()
        self.erasable_refs = set()   # erasable addresses the basic code names
        self.channels_used = set()   # I/O channels its channel orders name
        self.inputs_used = set()     # latched input channels it reads (AGC_CHANNEL)

    # ---- addresses -------------------------------------------------------
    def bank(self, phys):
        return phys // 1024

    def logical(self, phys):
        b = phys // 1024
        if b in (2, 3):
            return 0o4000 + (b - 2) * 1024 + phys % 1024
        return 0o2000 + phys % 1024

    def phys_of(self, logical, here):
        """Physical address of a fixed logical address seen from bank here."""
        if logical >= 0o6000:
            return 3 * 1024 + logical - 0o6000
        if logical >= 0o4000:
            return 2 * 1024 + logical - 0o4000
        if logical >= 0o2000:
            return self.bank(here) * 1024 + logical - 0o2000
        return None

    def in_file(self, phys):
        return phys is not None and self.cp.in_file(phys)

    def where(self, phys):
        return self.cp.where(phys)

    def label(self, phys, ext=False, clean=False):
        return ('X%o' if ext else 'B%o') % phys + ('c' if clean else '')

    def text(self, phys):
        ln = self.L.by_phys.get(phys)
        if ln is None:
            # second word of a two-word line
            ln = self.L.by_phys.get(phys - 1)
            return '%s (+1)' % (('%s %s %s' % (ln.label, ln.op, ln.operand)).strip() if ln else '')
        return ('%s %s %s' % (ln.label, ln.op, ln.operand)).strip()

    # ---- walking ---------------------------------------------------------
    def add(self, phys, ext=False, clean=False):
        """Label of the word at phys reached with EXTEND state ext and A
        known free of overflow (clean), queued for compiling."""
        if not self.in_file(phys) and not self.spec.get('fall_out'):
            raise BasicError('basic flow leaves the file at %s' % self.where(phys))
        k = (phys, ext, clean)
        if k not in self.nodes:
            self.nodes[k] = None
            self.work.append(k)
        return self.label(phys, ext, clean)

    def start(self, phys):
        self.entries.add(phys)
        self.dispatch.add(phys)

    def enter(self, phys):
        """Walk every basic word reachable from the start phys."""
        self.start(phys)
        self.add(phys)
        while self.work:
            k = self.work.pop()
            if not self.in_file(k[0]) and k[0] in self.cp.cut_words and not k[1]:
                # Spec 'inside': flow runs on into a segment the spec leaves
                # to other code (INTERPRETER's core); the routine stops there.
                self.nodes[k] = ['/* %s: outside the spec\'s segments */' % self.where(k[0]),
                                 'W[5]=0%04ou;return EXEC_OK;' % self.logical(k[0])]
                continue
            if not self.in_file(k[0]):
                # Spec 'fall_out': a TS overflow skip or CCS exit past the
                # file's last word, which no admitted state takes (the
                # differential checks it never is).
                self.data_landings.add(k[0])
                self.nodes[k] = ['/* %s: past the end of the file, never reached */' % self.where(k[0]),
                                 'return EXEC_INVALID_ACTION;']
                continue
            if (self.is_data(k[0]) or self.where(k[0]) in self.spec.get('interp_landings', ())) and \
                    self.where(k[0]) not in self.spec.get('execute_data', ()):
                # Control reaching a constant, or an interpretive word the
                # spec's 'interp_landings' names (P11's CCS SATSW -0 exit
                # into STEERSAT's TLOAD): the file's flow never does (the
                # paths here are CCS exits and TS skips no admitted state
                # takes); the differential checks none is taken.
                self.data_landings.add(k[0])
                self.nodes[k] = ['/* %s %s: data, never executed */' % (self.where(k[0]), self.text(k[0])),
                                 'return EXEC_INVALID_ACTION;']
                continue
            self.nodes[k] = self.compile(*k)

    BASIC_NAMES = ('TC', 'TCF', 'CCS', 'DAS', 'LXCH', 'INCR', 'ADS', 'CA', 'CAF', 'CAE', 'CS', 'INDEX', 'DXCH',
                   'TS', 'XCH', 'AD', 'MASK', 'READ', 'WRITE', 'RAND', 'WAND', 'ROR', 'WOR', 'RXOR', 'EDRUPT',
                   'DV', 'BZF', 'MSU', 'QXCH', 'AUG', 'DIM', 'DCA', 'DCS', 'SU', 'BZMF', 'MP', 'EXTEND',
                   'RELINT', 'INHINT', 'NOOP', 'RETURN', 'COM', 'DOUBLE', 'ZL', 'ZQ', 'DDOUBL', 'DCOM',
                   'SQUARE', 'OVSK', 'TCAA', 'RESUME', 'XLQ', 'XXALQ', 'DTCB', 'DTCF')

    def is_data(self, phys):
        ln = self.L.by_phys.get(phys)
        if ln is None:
            return True     # second word of a two-word constant
        if self.spec.get('interp_not_basic'):
            # Spec 'interp_not_basic': an interpretive order or operand word
            # that a CCS exit or skip lands on is never run as basic code
            # (CCS DIFEQCNT's negative exits after the MOVE* loops).
            import interp_compile
            op = ln.op.split(',')[0]
            if ln.op == '' or (interp_compile.is_interp_op(ln.op) and op not in self.BASIC_NAMES):
                return True
        return ln.op in self.CONST_OPS

    def outside_tc(self, phys, target_logical):
        """TC to an outside routine: the return is the word after the
        inline operands (constant lines after the TC)."""
        ret = phys + 1
        inline = []
        while True:
            ln = self.L.by_phys.get(ret)
            if ln is None or ln.op not in self.CONST_OPS:
                break
            self.operands.update(range(ret, ret + len(ln.words)))
            inline.append(ln.operand.split()[0] if ln.operand.split() else '')
            ret += len(ln.words)
        name = self.L.label_at(self.phys_of(target_logical, phys)) or ('%04o' % target_logical)
        self.cp.externals[self.phys_of(target_logical, phys)] = name
        # Spec 'noreturn' also names routines reached through BANKCALL's
        # CADR (INTWAKE1 ends the job; GOBAC's RVQ returns interpretively).
        noreturn = self.NORETURN + tuple(self.spec.get('noreturn', ()))
        if self.in_file(ret) and name not in noreturn and not (inline and inline[0] in noreturn):
            self.cp.add_start(ret, 'b', 'return from ' + name)
            # Spec 'returns': a callee (by inline CADR, else by name) that
            # also returns to the words after L+1 (GOFLASH: L+2 proceed, L+3
            # recycle; IMUSTALL and OPTSTALL: L+2 good). Each is a routine.
            returns = self.spec.get('returns', {})
            callee = inline[0] if len(inline) == 1 and inline[0] in returns else name
            for i in range(1, returns.get(callee, 1)):
                if self.in_file(ret + i) and not self.is_data(ret + i):
                    self.cp.add_start(ret + i, 'b', 'return %d from %s' % (i + 1, callee))
        return name

    # Outside routines that never come back to the word after the call.
    NORETURN = ('POSTJUMP', 'ENDOFJOB', 'TASKOVER', 'RESUME')

    CONST_OPS = ('OCT', 'OCTAL', 'DEC', 'DEC*', '2DEC', '2DEC*', 'ADRES', 'CADR', 'FCADR', 'GENADR',
                 '2CADR', '2FCADR', 'BBCON', 'REMADR', '2OCT', '2BCADR', 'ECADR', 'MM', 'VN', 'NV', 'BBCON*')

    def jump(self, phys, target_logical, call, clean=False):
        """Statements for TC/TCF (call) to a static target."""
        here = phys
        st = []
        if call:
            st.append('ai_tc(s,0%05ou);' % ((self.logical(here) + 1) & 0o7777))
        tp = self.phys_of(target_logical, here)
        if target_logical == 0o2 and call:
            raise BasicError('TC Q handled elsewhere')
        if tp == here and not call:
            # TCF +0: the AGC spins until its TC trap forces a restart; the
            # port refuses instead of spinning.
            return ['return EXEC_INVALID_ACTION; /* TCF +0 (TC trap) */']
        if tp is not None and self.in_file(tp):
            st.append('goto %s;' % self.add(tp, False, clean))
            if call and self.L.label_at(tp) in self.spec.get('noreturn', ()):
                # (an in-file routine the spec says never returns: DSPLY81
                # goes on in interpretive code)
                return st
            if call:
                self.dispatch.add(here + 1)
                self.add(here + 1)
                # Spec 'call_skips' {label: n}: a subroutine that may return
                # up to n words past the call's +1 (INCR Q; TC Q).
                for i in range(self.spec.get('call_skips', {}).get(self.L.label_at(tp), 0)):
                    self.dispatch.add(here + 2 + i)
                    self.add(here + 2 + i)
            return st
        if tp is not None and self.L.labels.get('INTPRET') == tp and call:
            # TC INTPRET: the interpretive code at the next word.
            st[-1:] = ['ai_tc(s,0%05ou);W[5]=0%04o;intpret(s,io);return EXEC_OK; /* INTPRET */'
                       % ((self.logical(here) + 1) & 0o7777, self.cp.intpret)]
            self.cp.add_start(here + 1, 'i', 'after TC INTPRET')
            return st
        if (tp is not None and not call and self.L.word(tp) == 0o2
                and target_logical in self.spec.get('tcq_words', ())):
            # Spec 'tcq_words' (fixed-fixed addresses): an outside word that
            # is TC Q (TCQ, the interpreter's 06711) runs here as the file's
            # own TC Q: Q's word runs as a TC from 2, so Q = 3.
            self.cp.basic_fixed.add(tp)
            st.append('e=(long)(agc_correct(W[2])&077777u); /* %04o: TC Q */' % target_logical)
            st.append('if(e<010||e>07777)return EXEC_OUT_OF_SCOPE;')
            st.append('ai_tc(s,03u);W[5]=(agc_word)e;if(e<02000)return EXEC_OK;goto dispatch;')
            return st
        if tp is None:
            raise BasicError('transfer to erasable %04o at %s' % (target_logical, self.where(here)))
        nc = self.spec.get('native_calls', {}).get(self.L.label_at(tp), {}).get(self.cp.m) if call else None
        if nc:
            # Spec 'native_calls' {label: {mission: call}}: an outside basic
            # subroutine run natively in place; it leaves Q as its own TC Q
            # does, and the run goes on at the word after the TC. '{adres}'
            # is the ADRES word after the TC (TC DMP), and the run goes on
            # past it.
            nxt = here + 1
            if '{adres}' in nc:
                ln = self.L.by_phys.get(nxt)
                if ln is None or ln.op != 'ADRES':
                    raise BasicError('TC %s at %s: no ADRES after it' % (self.L.label_at(tp), self.where(here)))
                nc = nc.replace('{adres}', '0%ou' % self.L.word(nxt))
                nxt += 1
            st.append('if(%s!=RTB_OK)return EXEC_OUT_OF_SCOPE; /* %s */' % (nc, self.L.label_at(tp)))
            st.append('goto %s;' % self.add(nxt, False, clean))
            return st
        name = self.outside_tc(here, target_logical) if call else (self.L.label_at(tp) or '%04o' % target_logical)
        if not call:
            self.cp.externals[tp] = name
        st.append('W[5]=0%04ou;return EXEC_OK; /* %s */' % (target_logical, name))
        return st

    def fixed_lookup(self, here):
        return 'fx(s,io,(unsigned)e,&k)'

    def operand_stmt(self, op, a, here, dyn):
        """Statement for a memory order on address a (or e when dyn)."""
        A = '(unsigned)e' if dyn else '0%o' % a
        lo = op.lower()
        if op in ('TS',):
            raise BasicError('TS handled by caller')
        if op in DOUBLE:
            # The instruction word carries K+1.
            A = '(unsigned)(e-1)' if dyn else '0%o' % ((a - 1) & 0o7777)
            if not dyn and a - 1 < 0:
                raise BasicError('double order on 0 at %s' % self.where(here))
        return 'ai_%s(s,%s);' % (lo, A)

    def compile(self, phys, ext, clean):
        """C statements for the word at phys (EXTEND state ext)."""
        w = self.L.word(phys)
        op, a, lim = decode(w, ext)
        if a is not None and 0o10 <= a < 0o2000 and op not in CHANNEL_OPS and op not in ('TC', 'TCF'):
            self.erasable_refs.update((a - 1, a) if op in DOUBLE else (a,))
        self.words.add(phys)
        nxt = phys + 1
        st = ['/* %s %s%s */' % (self.where(phys), 'EXTENDED ' if ext else '', self.text(phys))]
        if op == 'EXTEND':
            st.append('goto %s;' % self.add(nxt, True, clean))
            return st
        if op == 'INHINT':
            st.append('io->interrupt_enabled=0;')
            st.append('goto %s;' % self.add(nxt, False, clean))
            return st
        if op == 'RELINT':
            st.append('io->interrupt_enabled=1;')
            st.append('goto %s;' % self.add(nxt, False, clean))
            return st
        if op == 'INDEX':
            words = self.index_words.get(nxt, self.index_cells.get(a))
            if words is not None:
                st.extend(self.indexed_words(a, phys, nxt, ext, clean, words))
                return st
            if self.spec.get('short_index'):
                self.index_word = self.index_expr(a, phys)
            else:
                st.append(self.index_value(a, phys))
            st.extend(self.indexed(nxt, ext, clean))
            return st
        if op in ('CA', 'CS', 'AD', 'SU', 'MASK', 'MP', 'DV', 'DCA', 'DCS') and a is not None and \
                (a == 5 or (op in DOUBLE and a == 6)):
            # A read of Z (FLESHPOT's DCA Z, AD FOUR, DXCH LOC): Z holds the
            # address of the next word, as the hardware has it then.
            st.append('W[5]=0%o;' % ((self.logical(phys) + 1) & 0o7777))
        st.extend(self.order(op, a, lim, phys, False, ext, clean))
        return st

    # Orders after which A holds no overflow (a value read from memory or a
    # channel, a product, quotient or CCS result) and orders that leave A
    # as it was; any other order may leave A overflowed.
    CLEAN = ('CA', 'CS', 'DCA', 'DCS', 'MASK', 'XCH', 'DXCH', 'MP', 'DV', 'TS', 'CCS',
             'READ', 'RAND', 'WAND', 'ROR', 'WOR', 'RXOR', 'MSU')
    PASS = ('LXCH', 'QXCH', 'INCR', 'AUG', 'DIM', 'WRITE', 'BZF', 'BZMF')

    def clean_after(self, op, a, dyn, clean):
        if op in CHANNEL_OPS and a in (1, 2):
            return clean if op == 'WRITE' else False
        if op in self.CLEAN:
            if not dyn and a in (0, 2) and op in ('CA', 'CS', 'XCH', 'DXCH', 'DCA', 'DCS', 'MASK'):
                return False
            return True
        if op in self.PASS:
            return clean
        return False

    def indexed_words(self, a, here, phys, ext, clean, values):
        """INDEX K before a word whose sum with K's value can change the
        order itself (INDEX I, 0 SGNRT: TC with I = 0, CS with I = BIT15):
        one static order per value the spec's 'index_words' gives, anything
        else refused."""
        if a >= 0o2000 or a < 0o10:
            raise BasicError('index_words INDEX of %o at %s' % (a, self.where(here)))
        w = self.L.word(phys)
        self.words.add(phys)
        st = ['/* %s %s(indexed by %s) %s */' % (self.where(phys), 'EXTENDED ' if ext else '',
                                                  ', '.join('%05o' % v for v in values), self.text(phys))]
        st.append('switch(ai_word(s,0%o)){' % a)
        for v in values:
            x = w + v
            x = (x & 0o77777) + (x >> 15)     # ones' complement sum
            op, k, lim = decode(x & 0o77777, ext)
            if k is not None and 0o10 <= k < 0o2000 and op not in CHANNEL_OPS:
                self.erasable_refs.update(range(k, k + 3))
            st.append(' case 0%05ou: /* %s %s */' % (v, op, '%04o' % k if k is not None else ''))
            st.extend('  ' + y for y in self.order(op, k, lim, phys, False, ext, clean))
        st.append('}')
        st.append('return EXEC_INVALID_ACTION;')
        return st

    def index_expr(self, a, here):
        """Spec 'short_index': the word INDEX adds, for ixe()."""
        if a < 3:
            return 'agc_correct(W[%d])' % a
        self.index_value(a, here)
        return 'ai_word(s,0%o)' % a

    def index_sum(self, a, lim):
        """e = a + the INDEX value, refused outside 0..lim."""
        if self.spec.get('short_index'):
            w, self.index_word = self.index_word, None
            return 'e=ixe(%s,0%o,0%o);if(e<0)return EXEC_INVALID_ACTION;' % (w, a, lim)
        return 'e=(long)0%o+ix;if(e<0||e>0%o)return EXEC_INVALID_ACTION;' % (a, lim)

    def index_value(self, a, here):
        if a == 0:
            return 'ix=areg(s);'
        if a == 1:
            return 'ix=lreg(s);'
        if a == 2:
            return 'ix=qreg(s);'
        if 0o20 <= a <= 0o23 or a in (3, 4, 5, 6, 7):
            raise BasicError('INDEX of register %o at %s' % (a, self.where(here)))
        if a >= 0o2000:
            raise BasicError('INDEX of fixed memory at %s' % self.where(here))
        return 'ix=agc_to_int(ai_word(s,0%o));' % a

    def indexed(self, phys, ext, clean):
        """The word at phys executed with its address plus ix."""
        if not self.in_file(phys):
            raise BasicError('INDEX at the end of the file')
        w = self.L.word(phys)
        op, a, lim = decode(w, ext)
        self.words.add(phys)
        if a is not None and 0o10 <= a < 0o2000 and op not in CHANNEL_OPS:
            self.erasable_refs.update(range(a, a + 3))
        st = ['/* %s %s(indexed) %s */' % (self.where(phys), 'EXTENDED ' if ext else '', self.text(phys))]
        if op == 'INDEX' and a is not None and 0o10 <= a < 0o2000:
            # INDEX of INDEX (AX*SR*T's INDEX BUF, INDEX DEX1, DCA VBUF): the
            # second INDEX reads the erasable word at its address plus ix,
            # and that word indexes the order after it.
            if self.spec.get('short_index'):
                st.append(self.index_sum(a, 0o1777))
                st.append('if(e<010||(e>=020&&e<=023))return EXEC_INVALID_ACTION;')
                self.index_word = 'ai_word(s,(unsigned)e)'
            else:
                st.append('e=(long)0%o+ix;if(e<010||e>01777||(e>=020&&e<=023))return EXEC_INVALID_ACTION;' % a)
                st.append('ix=agc_to_int(ai_word(s,(unsigned)e));')
            st.extend(self.indexed(phys + 1, ext, clean))
            return st
        if a is None or op in ('EXTEND', 'INDEX', 'RESUME') or op in CHANNEL_OPS:
            raise BasicError('INDEX of %s at %s' % (op, self.where(phys)))
        if op in ('TC', 'TCF') and phys in self.index_targets:
            # A computed transfer into the file's own words (INDEX A, TC +1):
            # each word the spec's 'index_targets' allows is a dispatch label.
            for v in range(self.index_targets[phys] + 1):
                t = self.phys_of(a + v, phys)
                self.dispatch.add(t)
                self.add(t)
        if op == 'CCS' and not ext:
            # Opcode 1: CCS on an erasable sum, TCF on a fixed one (INDEX A,
            # TCF 0 through a table of GENADRs).
            st.append(self.index_sum(a, 0o7777))
            st.append('if(e>=02000){W[5]=(agc_word)e;goto dispatch;}')
        else:
            st.append(self.index_sum(a, lim))
        st.extend(self.order(op, a, lim, phys, True, False, clean))
        return st

    def order(self, op, a, lim, phys, dyn, ext, clean):
        """Statements for order op on address a (e when dyn) at phys, A
        free of overflow on arrival when clean."""
        nxt = phys + 1
        st = []
        here = phys
        out = self.clean_after(op, a, dyn, clean)
        cont = lambda: 'goto %s;' % self.add(nxt, False, out)
        if op == 'RESUME' and not dyn:
            # RESUME (INDEX 17): the next instruction is BRUPT's, run in place
            # (yaAGC's substitute instruction); here a TCF to fixed memory, as
            # CONTABRT stores (any other word is refused). RESUME also ends
            # an interrupt (yaAGC clears InIsr): R10,R11 reaches CONTABRT as
            # a T3RUPT task. With Luminary channels the flag is the program's
            # agc_hardware in_interrupt.
            # Without them the clear would be skipped silently.
            if not self.lm_channels:
                raise BasicError('RESUME at %s ends an interrupt: needs Luminary channels' % self.where(here))
            clr = 'if(agc_channels_hw)agc_channels_hw->in_interrupt=0u;'
            st.append('{agc_word b=W[017];if((b&070000u)!=010000u||(b&07777u)<02000u)return EXEC_OUT_OF_SCOPE;'
                      '%sW[5]=(agc_word)(b&07777u);}goto dispatch;' % clr)
            return st
        if op in ('TC', 'TCF'):
            call = op == 'TC'
            if dyn:
                if call:
                    st.append('ai_tc(s,0%05ou);' % ((self.logical(here) + 1) & 0o7777))
                    self.dispatch.add(nxt)
                    self.add(nxt)
                st.append('W[5]=(agc_word)e;goto dispatch;')
                return st
            if a < 3 and call:
                # TC A (XXALQ), TC L (XLQ), TC Q (RETURN): the register's word
                # runs as an instruction, a TC (Q = K+1) to fixed memory or
                # to an erasable continuation (tools/basic_compile.py's
                # erasable points); anything else is refused.
                reg = {0: 'W[0]', 1: 'W[1]', 2: 'W[2]'}[a]
                st.append('e=(long)(agc_correct(%s)&077777u);' % reg)
                if a == 2 and self.eb_word is not None:
                    # TC Q with Q = 3 (left by an earlier TC Q): Q's word 3
                    # runs as RELINT, then the word in EB as TC EB (CM entry
                    # DAP's 2D/SDZ after CCS JETAG -0 reaches CM/FDAIR -1
                    # this way).
                    st.append('if(e==3&&W[3]>=02000u){io->interrupt_enabled=1;ai_tc(s,04u);W[5]=W[3];goto dispatch;}')
                    t = self.phys_of(self.eb_word, here)
                    if self.in_file(t) and not self.is_data(t):
                        self.dispatch.add(t)
                        self.add(t)
                st.append('if(e<010||e>07777)return EXEC_OUT_OF_SCOPE;')
                st.append('ai_tc(s,0%ou);W[5]=(agc_word)e;if(e<02000)return EXEC_OK;goto dispatch;' % (a + 1))
                return st
            if 0o10 <= a < 0o1400 and call and not 0o20 <= a <= 0o23:
                # TC to an unswitched erasable word (SERVICER207's TC RUPTREG1
                # after QXCH RUPTREG1, P40-P47's TC TEMPR60): the stored word
                # runs as a TC, Q = K+1, as for TC Q; anything but a TC is
                # refused.
                st.append('e=(long)(W[0%o]&077777u);' % a)
                if self.spec.get('switched_tc'):
                    self.cp.tc_erasable.add(a)   # (the differential seeds a return there)
                # The words the spec's states hold there (TEMPR60: the TC
                # OUTPUT returns) are dispatch targets in the file.
                for held in self.spec.get('states', {}).values():
                    for v in held.get('%o' % a, []):
                        t = self.phys_of(v, here) if isinstance(v, int) and v >= 0o2000 else None
                        if t is not None and self.in_file(t) and not self.is_data(t):
                            self.dispatch.add(t)
                            self.add(t)
                if self.spec.get('tcf_words'):
                    # Spec 'tcf_words': the stored word may be a TCF (a
                    # stored instruction such as CA ACCHERE's TCF ACCTHERE),
                    # which from there transfers as the TC does (Q K+1 kept).
                    st.append('if(e>=012000&&e<=017777)e&=07777;')
                st.append('if(e<010||e>07777)return EXEC_OUT_OF_SCOPE;')
                st.append('ai_tc(s,0%ou);W[5]=(agc_word)e;if(e<02000)return EXEC_OK;goto dispatch;' % (a + 1))
                return st
            if 0o1400 <= a < 0o2000 and call and self.spec.get('switched_tc') and a not in self.cp.erasable_calls:
                # Spec 'switched_tc': TC to a switched erasable word
                # (IMU_CALIBRATION's TC QPLACE after QXCH QPLACE) runs the
                # word in the bank EB selects as a TC, Q = K+1, as
                # tools/interp_compile.py's tc_e does. The spec's states for
                # that word in its EBANK are dispatch targets, and the
                # differential seeds a caller's return there (unswitched TC
                # MPAC too), as for tc_e files.
                st.append('e=(long)(ai_word(s,0%o)&077777u);' % a)
                self.cp.tc_erasable.add(a)
                for held in list(self.spec.get('states', {}).values()) + [self.spec.get('all_states', {})]:
                    for key in ('%05o' % (self.spec['ebank'] * 0o400 + a - 0o1400),):
                        for v in held.get(key, []):
                            t = self.phys_of(v, here) if isinstance(v, int) and v >= 0o2000 else None
                            if t is not None and self.in_file(t) and not self.is_data(t):
                                self.dispatch.add(t)
                                self.add(t)
                st.append('if(e<010||e>07777)return EXEC_OUT_OF_SCOPE;')
                st.append('ai_tc(s,0%ou);W[5]=(agc_word)e;if(e<02000)return EXEC_OK;goto dispatch;' % (a + 1))
                return st
            if a < 0o2000 and call and a not in (3, 4, 5, 6, 7) and not 0o20 <= a <= 0o23:
                # TC to a stored word: Z = K, a transfer to the erasable
                # continuation registered for it (spec 'erasable_points').
                st.append('ai_tc(s,0%05ou);W[5]=0%o;return EXEC_OK; /* TC %s */' % (
                    (self.logical(here) + 1) & 0o7777, a, self.L.label_at(a) or '%04o' % a))
                return st
            if a < 0o2000:
                raise BasicError('%s to erasable %o at %s' % (op, a, self.where(here)))
            st.extend(self.jump(here, a, call, clean))
            return st
        if op in ('BZF', 'BZMF'):
            f = 'ai_bzf' if op == 'BZF' else 'ai_bzmf'
            if dyn:
                raise BasicError('indexed %s at %s' % (op, self.where(here)))
            st.append('if(%s(s)){' % f)
            st.extend(['  ' + x for x in self.jump(here, a, False, clean)])
            st.append('}')
            st.append(cont())
            return st
        if op == 'CCS':
            if dyn:
                st.append('switch(ai_ccs(s,(unsigned)e)){')
            elif a >= 0o2000:
                raise BasicError('CCS on fixed at %s' % self.where(here))
            else:
                st.append('switch(ai_ccs(s,0%o)){' % a)
            for i in range(4):
                st.append(' case %d:goto %s;' % (i, self.add(nxt + i, False, True)))
            st.append('}')
            st.append('return EXEC_INVALID_ACTION;')
            return st
        if op == 'TS':
            A = '(unsigned)e' if dyn else '0%o' % a
            if not dyn and a == 5:
                # TCAA: Z from A.
                st.append('e=areg(s)&07777;(void)ai_ts(s,0);W[5]=(agc_word)e;goto dispatch;')
                return st
            if not dyn and a in (4, 6):
                st.append('if(ai_ts(s,%s)){W[5]=0%ou;goto dispatch;}' % (A, (self.logical(here) + 2) & 0o7777))
                st.append('W[5]=0%ou;goto dispatch;' % ((self.logical(here) + 1) & 0o7777))
                self.dispatch.update((nxt, nxt + 1))
                self.add(nxt)
                self.add(nxt + 1)
                return st
            if dyn:
                st.append('if(e==5||e==4||e==6)return EXEC_INVALID_ACTION;')
            if clean:
                # A holds no overflow here: the TS cannot skip.
                st.append('(void)ai_ts(s,(unsigned)%s);' % A)
            else:
                st.append('if(ai_ts(s,(unsigned)%s))goto %s;' % (A, self.add(nxt + 1, False, True)))
            st.append(cont())
            return st
        if op in CHANNEL_OPS and a in (1, 2) and op != 'EDRUPT':
            # Channels 1 and 2 are L and Q: the 16-bit registers themselves.
            st.append({'READ': 'W[0]=W[%o];', 'WRITE': 'W[%o]=W[0];', 'RAND': 'W[0]&=W[%o];',
                       'WAND': 'W[0]&=W[%o];W[%o]=W[0];', 'ROR': 'W[0]|=W[%o];', 'WOR': 'W[0]|=W[%o];W[%o]=W[0];',
                       'RXOR': 'W[0]^=W[%o];'}[op].replace('%o', '%o' % a))
            st.append(cont())
            return st
        if op in ('READ', 'RAND', 'ROR', 'RXOR') and self.spec.get('channel_inputs') and a in INPUT_CHANNELS:
            # An input channel the caller latches (spec 'channel_inputs',
            # include/agc_input_channels.h), as tools/interp_compile.py reads it.
            v = 'AGC_CHANNEL(%s)' % INPUT_CHANNELS[a]
            self.inputs_used.add(a)
            st.append({'READ': 'W[0]=agc_extend(%s);', 'RAND': 'W[0]=agc_extend((agc_word)(oc16(W[0])&%s));',
                       'ROR': 'W[0]=agc_extend((agc_word)(oc16(W[0])|%s));',
                       'RXOR': 'W[0]=agc_extend((agc_word)(oc16(W[0])^%s));'}[op] % v)
            self.cp.uses_channels = True
            st.append(cont())
            return st
        if op in CHANNEL_OPS and self.lm_channels:
            k = CHANNEL_OPS.index(op)
            if a not in LM_CHANNELS[k]:
                raise BasicError('channel %o at %s' % (a, self.where(here)))
            self.channels_used.add(a)
            self.cp.uses_channels = self.cp.uses_hw = True
            st.append('if(!agc_ch_op(s,agc_channels_hw,agc_channels_in,&io->superbank,%d,0%o))return EXEC_OUT_OF_SCOPE;' % (k, a))
            st.append(cont())
            return st
        if op in CHANNEL_OPS:
            if a == 0o7 and getattr(self, 'cm_superbank', False):
                self.channels[0o7] = 'superbank'
            if a not in self.channels:
                raise BasicError('channel %o at %s' % (a, self.where(here)))
            if op == 'EDRUPT':
                raise BasicError('EDRUPT at %s' % self.where(here))
            self.channels_used.add(a)
            st.append('ch_%s(s,io,0%o);' % (op.lower(), a))
            st.append(cont())
            return st
        if op == 'DXCH' and not dyn and a == 6:
            # DTCB: DXCH Z (Z and BB with A and L).
            st.append('dtcb(s,0%05ou);goto dispatch;' % self.logical(here))
            return st
        if op == 'DXCH' and not dyn and a == 5:
            # DTCF: DXCH FB (FB and Z with A and L).
            st.append('dtcf(s,0%05ou);goto dispatch;' % self.logical(here))
            return st
        if op == 'MSU' and dyn and 0o20 <= a < 0o2000:
            # Indexed (INDEX MKNDX; MSU CDUX): the sum must stay an ordinary
            # erasable word.
            self.uses_msu = True
            st.append('if(e<020||e>=02000)return EXEC_INVALID_ACTION;')
            st.append('msu(s,(unsigned)e);')
            st.append(cont())
            return st
        if op == 'MSU':
            # On an ordinary erasable word, as yaAGC takes it. INDEXed
            # (OPTDRIVE's INDEX OPTIND; MSU CDUT), the word it lands on must
            # be one too.
            if a >= 0o2000 or (not dyn and a < 0o20):
                raise BasicError('MSU on %o at %s' % (a, self.where(here)))
            self.uses_msu = True
            if dyn:
                st.append('if(e<020||e>=02000)return EXEC_INVALID_ACTION;')
                st.append('msu(s,(unsigned)e);')
            else:
                st.append('msu(s,0%o);' % a)
            st.append(cont())
            return st
        # Memory orders.
        writes = op in ('XCH', 'LXCH', 'QXCH', 'DXCH', 'DAS', 'ADS', 'INCR', 'AUG', 'DIM')
        if dyn:
            if writes:
                st.append('if(e<010)return EXEC_INVALID_ACTION;')
            else:
                st.append('if(e==5)return EXEC_INVALID_ACTION;')
            if op in FIXED_FORM and lim == 0o7777:
                lo_name = FIXED_FORM[op]
                if op in ('DCA', 'DCS'):
                    st.append('if(e>=02000){agc_word k2[2];if(!fx(s,(unsigned)(e-1),k2)||!fx(s,(unsigned)e,k2+1))return EXEC_INVALID_ACTION;ai_%s(s,k2);}' % lo_name)
                else:
                    st.append('if(e>=02000){agc_word k;if(!fx(s,(unsigned)e,&k))return EXEC_INVALID_ACTION;ai_%s(s,k);}' % lo_name)
                st.append('else %s' % self.operand_stmt(op, a, here, True))
            else:
                st.append(self.operand_stmt(op, a, here, True))
            st.append(cont())
            return st
        if a >= 0o2000:
            if op not in FIXED_FORM or lim != 0o7777:
                raise BasicError('%s on fixed at %s' % (op, self.where(here)))
            lo_name = FIXED_FORM[op]
            fp = self.phys_of(a, here)
            sym = self.L.label_at(fp) or ''
            self.cp.basic_fixed.add(fp)
            if op in ('DCA', 'DCS'):
                fp0 = self.phys_of(a - 1, here)
                self.cp.basic_fixed.add(fp0)
                st.append('{static const agc_word k[2]={0%05ou,0%05ou};ai_%s(s,k);} /* %s */' % (
                    self.L.word(fp0), self.L.word(fp), lo_name, self.L.label_at(fp0) or sym))
            else:
                st.append('ai_%s(s,0%05ou); /* %s */' % (lo_name, self.L.word(fp), sym))
            st.append(cont())
            return st
        # Spec 'bb_writes': words that may write BBANK (INTRSM's LXCH BBANK,
        # which TCFs to fixed-fixed code next, so no banked transfer follows
        # under the new FB).
        bb_ok = op in ('LXCH', 'TS', 'XCH') and a == 6 and self.where(here) in self.spec.get('bb_writes', ())
        if writes and a in (4, 5, 6) and not bb_ok or (op == 'DXCH' and a in (4, 5, 6, 7)):
            raise BasicError('%s on %o at %s' % (op, a, self.where(here)))
        st.append(self.operand_stmt(op, a, here, False))
        st.append(cont())
        return st

    # ---- output ----------------------------------------------------------
    def erasable_points(self):
        """Routines for the file's erasable continuations (spec
        'erasable_points' {'E<bank>,<addr>': [words]}): keyed
        AGC_TRANSFER_ERASABLE_BASE + the physical address, each runs the
        stored word only when it is one of the TC words the file stores
        there, else refuses. Returns (C lines, [(key, function)])."""
        c, ents = [], []
        for k, words in sorted(self.spec.get('erasable_points', {}).items()):
            b, a = k[1:].split(',')
            b, a = int(b, 8), int(a, 8)
            phys = b * 256 + (a - 0o1400) if a >= 0o1400 else a
            key = 40960 + phys
            fn = 'e%o' % phys
            c.append('/* %s: the TC words the file stores there (%s). */' % (k, ', '.join('%05o' % w for w in words)))
            c.append('static enum exec_status %s_abi(const agc_native_program *p,apollo_overlay_state *s,void *v){' % fn)
            c.append(' %s *io=(%s *)v;agc_word w;(void)p;' % (self.io, self.io))
            c.append(' if(!s||!v||!basic_sane(s,io,W[4])||agc_erasable_point(W[5],W[3])!=%uu)return EXEC_OUT_OF_SCOPE;' % key)
            c.append(' w=W[0%o];' % phys)
            c.append(' switch(w){%s break;default:return EXEC_OUT_OF_SCOPE;}' % ''.join('case 0%05ou:' % x for x in words))
            c.append(' ai_tc(s,(unsigned)((W[5]&07777u)+1u));W[5]=w;')
            c.append(' return EXEC_OK;')
            c.append('}')
            ents.append((key, fn))
        return c, ents

    def erasable_targets(self):
        """The file's own words its erasable continuations transfer to:
        each is a routine of its own ((phys, why) pairs)."""
        out = []
        for k, words in sorted(self.spec.get('erasable_points', {}).items()):
            for x in words:
                for q in (self.word_targets(x) if x >= 0o2000 else []):
                    if self.in_file(q) and q in self.cp.code:
                        out.append((q, 'TC %05o stored in %s' % (x, k)))
        return out

    def word_targets(self, w):
        """Physical addresses a stored TC word w may name (banked words: the
        file's banks)."""
        if w >= 0o4000:
            return [2048 + w - 0o4000]
        return [bk * 1024 + w - 0o2000 for bk in set(q // 1024 for q in self.cp.words)]

    def index_locals(self):
        """Spec 'short_index': 16-bit index locals (every e and ix the
        orders form fits: a 12-bit address plus a 15-bit signed index, or a
        15-bit register word), which the 6502 handles in a third of the code."""
        return ' %s ix=0,e=0;uint16_t p;' % ('int' if self.spec.get('short_index') else 'long')

    def emit(self, sym):
        c = []
        if self.spec.get('short_index'):
            c.append('/* INDEX: address b plus the signed index word k, or -1 outside 0..lim')
            c.append(' * (one out-of-line copy for the file\'s indexed orders). */')
            c.append('#ifdef __GNUC__')
            c.append('__attribute__((noinline))')
            c.append('#endif')
            c.append('static MAYBE_UNUSED int ixe(agc_word k,unsigned b,unsigned lim){')
            c.append(' int e=(int)b+agc_to_int(k);return e<0||e>(int)lim?-1:e;}')
        c.append('/* Signed index values: A, L and Q are 16-bit registers. */')
        c.append('static MAYBE_UNUSED long areg(const apollo_overlay_state *s){return agc_to_int(agc_correct(W[0]));}')
        c.append('static MAYBE_UNUSED long lreg(const apollo_overlay_state *s){return agc_to_int(agc_correct(W[1]));}')
        c.append('static MAYBE_UNUSED long qreg(const apollo_overlay_state *s){return agc_to_int(agc_correct(W[2]));}')
        c.append('/* DTCB (DXCH Z) and DTCF (DXCH FB) executed at `at`. */')
        c.append('static MAYBE_UNUSED void dtcb(apollo_overlay_state *s,agc_word at){agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);')
        c.append(' W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=(agc_word)(z&07777u);ai_put(s,6,bb);}')
        c.append('static MAYBE_UNUSED void dtcf(apollo_overlay_state *s,agc_word at){agc_word fb=agc_correct(W[0]),z=agc_correct(W[1]);')
        c.append(' W[0]=agc_extend(W[4]);W[1]=(agc_word)(at+1u);W[5]=(agc_word)(z&07777u);ai_put(s,4,fb);}')
        c.append('/* TC INTPRET: the interpreter entry with the latches the bodies use. */')
        if self.io == 'p66_bias_io':
            c.append('static MAYBE_UNUSED void intpret(apollo_overlay_state *s,p66_bias_io *io){lmi_intpret(s,io);}')
        else:
            c.append('static MAYBE_UNUSED void intpret(apollo_overlay_state *s,cm_native_io *io){p66_bias_io l;')
            c.append(' l.superbank=io->superbank;l.interrupt_enabled=io->interrupt_enabled;cmi_intpret(s,&l);')
            c.append(' io->superbank=l.superbank;io->interrupt_enabled=l.interrupt_enabled;}')
        if self.channels:
            c.extend(self.channel_helpers())
        elif any('oc16(' in x for body in self.nodes.values() for x in body):
            # (latched input channels only: RAND/ROR/RXOR still take A's 16 bits)
            c.append('static MAYBE_UNUSED agc_word oc16(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}')
        if self.fixed_tables:
            c.append('/* Fixed words the file reads under INDEX (rope words). */')
            for lo, hi, lab, off in self.fixed_tables:
                c.append('static const agc_word T%o[%d]={%s}; /* %s %s%+d .. %+d */' % (
                    lo, hi - lo + 1, ','.join('0%05o' % self.L.word(p) for p in range(lo, hi + 1)),
                    self.where(lo), lab, off, off + hi - lo))
        c.append('/* A fixed word read under INDEX (logical e under this bank), from the')
        c.append(' * tables above: 0 outside them. */')
        c.append('static MAYBE_UNUSED uint8_t fx(const apollo_overlay_state *s,unsigned e,agc_word *k){')
        c.append(' unsigned p;(void)s;')
        fb = self.bank(getattr(self, 'fx_entry', None) or (min(self.entries) if self.entries else self.cp.lo))
        # Switched tables all in one bank: e 02000-03777 is that bank (a file
        # in two banks, such as Luminary's POWERED_FLIGHT_SUBROUTINES, reads
        # INDEXI from bank 23 while its lowest entry is in bank 10).
        tb = {self.bank(lo) for lo, hi, lab, off in self.fixed_tables if self.bank(lo) not in (2, 3)}
        if len(tb) == 1:
            fb = tb.pop()
        c.append(' if(e>=06000u)p=3u*1024u+e-06000u;else if(e>=04000u)p=2u*1024u+e-04000u;else p=0%ou*1024u+e-02000u;' % fb)
        for lo, hi, lab, off in self.fixed_tables:
            c.append(' if(p>=%uu&&p<=%uu){*k=T%o[p-%uu];return 1;}' % (lo, hi, lo, lo))
        c.append(' (void)p;(void)k;return 0;')
        c.append('}')
        if self.spec.get('fixed_rope'):
            # Spec 'fixed_rope' (INTERPRETER's subroutines): an INDEX read of
            # fixed memory reads the caller's word from the mission's rope
            # image under FB and SUPERBNK, as the AGC does (DMP's ADRES after
            # its TC, POLY's inline coefficients). The rope is the program's
            # data here, as it is for the interpreter core that runs the
            # same file (include/rope_image.h).
            c.append('#include "rope_image.h"')
            c.append('static MAYBE_UNUSED uint8_t fxr(const apollo_overlay_state *s,const %s *io,unsigned e,agc_word *k){' % self.io)
            c.append(' unsigned b=(W[4]>>10)&037u,sb=io->superbank&0160u;uint16_t p;')
            c.append(' if(e>=04000u)p=(uint16_t)(2048u+e-04000u);')
            c.append(' else{if(b>=030u&&(sb&0100u)){if(sb!=0100u)return 0;b+=010u;}p=(uint16_t)(b*1024u+(e&01777u));}')
            c.append(' return apollo_rope_read(%du,p,k);' % (0 if self.io == 'p66_bias_io' else 1))
            c.append('}')
            c.append('#define fx(s,e,k) fxr(s,io,e,k)')
        # (msu: tools/interp_compile.py's helper.)
        c.append('/* Host coverage builds (tools/interp_routine_differential.py --freeze)')
        c.append(' * count each label a case reaches, to pick C64 cases that take every')
        c.append(' * branch outcome the host cases take. */')
        c.append('#ifdef IRC_COVER_ON')
        c.append('void irc_cover(unsigned n);')
        c.append('#define IRC_COVER(n) irc_cover(n)')
        c.append('#else')
        c.append('#define IRC_COVER(n) ((void)0)')
        c.append('#endif')
        order = sorted(self.nodes)
        self.node_banks = [k[0] // 1024 for k in order]
        if self.spec.get('basic_split'):
            return c + self.emit_split(sym, order)
        if self.spec.get('basic_parts'):
            return c + self.emit_parts(sym, order)
        fp = self.spec.get('flight_parts')
        if fp:
            # (One function: every word leaves at another part's word.)
            S = sym.upper()
            c += self.flight_parts(S, {self.label(*k): k for k in order})
        c.append('/* Every basic word of the file. */')
        c.append('static MAYBE_UNUSED enum exec_status basic_run(apollo_overlay_state *s,%s *io,uint16_t at){' % self.io)
        c.append(self.index_locals())
        c.append(' (void)ix;(void)e;')
        c.append(' switch(at){')
        for p in sorted(self.entries):
            c.append(' case %uu:goto %s;' % (p, self.label(p)))
        extra = [q for q in getattr(self, 'flight_extra', []) if q not in self.entries] if fp else []
        if extra:
            c.append('#ifdef %s_FLIGHT_PART' % S)
            for q in extra:
                if (q, False, False) in self.nodes:
                    c.append(' case %uu:goto %s;' % (q, self.label(q)))
                else:
                    c.append(' case %uu:if((W[0]^(W[0]>>1))&040000u)return EXEC_OUT_OF_SCOPE;goto %s;' % (
                        q, self.label(q, False, True)))
            c.append('#endif')
        c.append(' default:return EXEC_OUT_OF_SCOPE;')
        c.append(' }')
        # The spec's idle points (a word that spins until hardware moves,
        # such as VERB69's TC VERB69): reached again, the run idles there,
        # as the oracle stops there; an entry there runs its word once.
        idle = getattr(self.cp, 'idle', set())
        spin = {self.label(*k): '{W[5]=0%04o;return EXEC_IDLE;}' % self.logical(k[0]) for k in self.nodes if k[0] in idle}
        for n, k in enumerate(sorted(self.nodes)):
            c.append('%s:' % self.label(*k))
            c.append(' %s_FP(%uu) IRC_COVER(%du);' % (S, k[0], n) if fp else ' IRC_COVER(%du);' % n)
            body = self.nodes[k]
            if spin:
                body = [re.sub(r'goto ([A-Za-z][A-Za-z0-9]*);', lambda m: spin.get(m.group(1), m.group(0)), x) for x in body]
            twin = (k[0], k[1], False)
            if k[2] and twin in self.nodes and self.nodes[twin] == [x.replace(self.label(*k), self.label(*twin)) for x in body]:
                # The same code as the copy reached with A unknown.
                c.append(' goto %s;' % self.label(*twin))
                continue
            for x in body:
                c.append(' ' + x)
        if not any('goto dispatch' in x for x in c):
            # No computed transfer: the return words of in-file calls whose
            # subroutine never returns by TC Q are never reached.
            body = '\n'.join(c)
            for i, x in enumerate(c):
                if x.endswith(':') and ('goto %s;' % x[:-1]) not in body:
                    c[i] = x + ' __attribute__((unused));'
            c.append(' (void)p;')
            at = max(i for i, x in enumerate(c) if 'enum exec_status basic_run(' in x)
            if not any('io->' in x or 'io,' in x or 'io)' in x for x in c[at + 1:]):
                c.append(' (void)io;')   # (a file without channel orders or calls that take io)
            c.append('}')
            return c
        c.append('dispatch:')
        c.append(' p=agc_fixed_physical(W[5],W[4],io->superbank);')
        c.append(' switch(p){')
        for p in sorted(self.dispatch):
            if p in idle:
                c.append(' case %uu:W[5]=0%04o;return EXEC_IDLE;' % (p, self.logical(p)))
            elif (p, False, False) in self.nodes:
                c.append(' case %uu:goto %s;' % (p, self.label(p)))
        c.append(' default:return EXEC_OK;')
        c.append(' }')
        c.append('}')
        if spin:
            # (A clean copy whose only jump an idle point replaced is never
            # reached: FINETIME's INHINT ahead of its idle EXTEND.)
            body = '\n'.join(c)
            for i, x in enumerate(c):
                if x.endswith(':') and x != 'dispatch:' and ('goto %s;' % x[:-1]) not in body:
                    c[i] = x + ' __attribute__((unused));'
        return c

    def run_name(self, bank, phys=None):
        if self.spec.get('basic_parts') and phys is not None:
            return 'basic_part_%d' % self.part_of(phys)
        return 'basic_run_%o' % bank if self.spec.get('basic_split') else 'basic_run'

    # ---- parts (spec 'basic_parts') ---------------------------------------
    # The flight store links each routine with its closure; one basic_run
    # holding the whole file puts every routine over its slot. With
    # 'basic_parts': {'budget': n}, the basic words are cut, in address
    # order, into part functions of about n characters of C each. A transfer
    # into another part's word sets Z to it and returns EXEC_OK, so the
    # transfer runner goes on in that word's routine (a part key): the run,
    # and every case's outcome, is the same.
    def part_of(self, phys):
        import bisect
        return bisect.bisect_right(getattr(self, 'cuts', []), phys)

    def partition(self):
        """Choose the cuts; return the words another part enters (the part
        keys: targets of gotos across a cut, and every computed-transfer
        target when there is more than one part)."""
        budget = self.spec['basic_parts']['budget']
        # (fx's bank comes from the file's own starts, not part keys.)
        if self.entries and not hasattr(self, 'fx_entry'):
            self.fx_entry = min(self.entries)
        size = collections.Counter()
        ext_at = set()
        for k, body in self.nodes.items():
            size[k[0]] += sum(len(x) for x in (body or ())) + 16
            if k[1]:
                ext_at.add(k[0])
        cuts, acc = [], 0
        for a in sorted(size):
            # (Never cut before a word reached under EXTEND: the extend
            # state does not cross a hand-off.)
            if acc and acc + size[a] > budget and a not in ext_at:
                cuts.append(a)
                acc = 0
            acc += size[a]
        self.cuts = cuts
        keys = set()
        lab = {self.label(*k): k for k in self.nodes}
        for k, body in self.nodes.items():
            for x in body or ():
                for m in re.finditer(r'goto ([BX][0-7]+c?);', x):
                    t = lab.get(m.group(1))
                    if t is not None and self.part_of(t[0]) != self.part_of(k[0]):
                        if t[1]:
                            raise BasicError('extended word %s across a part cut' % self.where(t[0]))
                        keys.add(t[0])
        if cuts:
            keys |= {p for p in self.dispatch if (p, False, False) in self.nodes}
        return keys

    def emit_parts(self, sym, order):
        """The part functions (see partition())."""
        lab = {self.label(*k): k for k in order}
        nparts = len(getattr(self, 'cuts', [])) + 1
        c = []
        for j in range(nparts):
            mine = [(n, k) for n, k in enumerate(order) if self.part_of(k[0]) == j]
            body = []
            for n, k in mine:
                body.append('%s:' % self.label(*k))
                body.append(' IRC_COVER(%du);' % n)
                twin = (k[0], k[1], False)
                if k[2] and twin in self.nodes and self.nodes[twin] == [x.replace(self.label(*k), self.label(*twin)) for x in self.nodes[k]]:
                    body.append(' goto %s;' % self.label(*twin))
                    continue
                for x in self.nodes[k]:
                    body.append(' ' + x)

            def cross(m):
                k = lab.get(m.group(1))
                if k is None or self.part_of(k[0]) == j:
                    return m.group(0)
                # Hand Z to the word's routine (a part key).
                return '{W[5]=0%o;return EXEC_OK;} /* part key %s */' % (self.logical(k[0]), self.where(k[0]))
            body = [re.sub(r'goto ([BX][0-7]+c?);', cross, x) for x in body]
            into = sorted(p for p in self.entries if self.part_of(p) == j and (p, False, False) in self.nodes)
            fn = ['/* Basic words, part %d of %d. */' % (j + 1, nparts),
                  'static MAYBE_UNUSED enum exec_status basic_part_%d(apollo_overlay_state *s,%s *io,uint16_t at){' % (j, self.io),
                  self.index_locals(), ' (void)ix;(void)e;(void)p;(void)io;', ' switch(at){']
            fn += [' case %uu:goto %s;' % (q, self.label(q)) for q in into]
            fn += [' default:return EXEC_OUT_OF_SCOPE;', ' }']
            fn += body
            if any('goto dispatch' in x for x in body):
                fn += ['dispatch:', ' p=agc_fixed_physical(W[5],W[4],io->superbank);', ' switch(p){']
                fn += [' case %uu:goto %s;' % (q, self.label(q)) for q in sorted(self.dispatch)
                       if (q, False, False) in self.nodes and self.part_of(q) == j]
                # (A word of another part: Z already names it, a part key.)
                fn += [' default:return EXEC_OK;', ' }']
            used = set(re.findall(r'goto ([A-Za-z][A-Za-z0-9]*);', '\n'.join(fn)))
            fn = [x + ' __attribute__((unused));' if x.endswith(':') and not x.startswith(' ') and x[:-1] not in used else x
                  for x in fn]
            c += fn
            c.append('}')
        return c

    def emit_split(self, sym, order):
        """Spec 'basic_split': one function per bank (basic_run_<bank>),
        each kept only when <SYM>_BANKS has the bank's bit, so a C64 group
        links the banks its cases run in (a file whose basic code does not
        fit one diagnostic). A transfer to another bank's word calls that
        bank's function."""
        S = sym.upper()
        banks = sorted(set(self.node_banks))
        lab = {}
        for k in order:
            lab[self.label(*k)] = k
        c = self.flight_parts(S, lab) if self.spec.get('flight_parts') else []
        cut_banks = {x // 1024 for x in getattr(self, 'flight_cuts', [])
                     if any(k[0] // 1024 == x // 1024 and k[0] < x for k in order)}
        c += ['#ifndef %s_BANKS' % S, '#define %s_BANKS 0xffffffffffffffffull' % S, '#endif',
              '#define %s_HAS(b) ((%s_BANKS >> (b)) & 1u)' % (S, S)]
        # Words another bank's code may reach (computed transfers) and
        # the entries, per bank.
        into = {b: set() for b in banks}
        for p in self.entries | self.dispatch:
            if (p, False, False) in self.nodes:
                into[p // 1024].add(p)
        for b in banks:
            c.append('#if %s_HAS(0%o)' % (S, b))
            c.append('static enum exec_status basic_run_%o(apollo_overlay_state *s,%s *io,uint16_t at);' % (b, self.io))
            c.append('#endif')
        c += self.callee_defs(sym)
        for b in banks:
            mine = [(n, k) for n, k in enumerate(order) if k[0] // 1024 == b]
            body = []
            for n, k in mine:
                body.append('%s:' % self.label(*k))
                if b in cut_banks:
                    # (A flight part build leaves at a word of another part.)
                    body.append(' %s_FP(%uu) IRC_COVER(%du);' % (S, k[0], n))
                else:
                    body.append(' IRC_COVER(%du);' % n)
                twin = (k[0], k[1], False)
                if k[2] and twin in self.nodes and self.nodes[twin] == [x.replace(self.label(*k), self.label(*twin)) for x in self.nodes[k]]:
                    body.append(' goto %s;' % self.label(*twin))
                    continue
                for x in self.nodes[k]:
                    body.append(' ' + x)
            # A goto into another bank's words calls that bank's function.
            def cross(m):
                k = lab.get(m.group(1))
                if k is None or k[0] // 1024 == b:
                    return m.group(0)
                if k[1]:
                    raise BasicError('extended transfer across banks at %s' % self.where(k[0]))
                into[k[0] // 1024].add(k[0])
                return 'return %s(s,io,%uu);' % (self.callee(S, k[0] // 1024), k[0])
            body = [re.sub(r'goto ([A-Z][0-7]+c?);', cross, x) for x in body]
            c.append('#if %s_HAS(0%o)' % (S, b))
            c.append('/* The basic words of bank %02o. */' % b)
            c.append('static enum exec_status basic_run_%o(apollo_overlay_state *s,%s *io,uint16_t at){' % (b, self.io))
            c.append(self.index_locals())
            c.append(' (void)ix;(void)e;(void)p;')
            if b in cut_banks:
                c.append(' %s_FP(at)' % S)
            c.append(' switch(at){')
            for q in sorted(into[b]):
                c.append(' case %uu:goto %s;' % (q, self.label(q)))
            extra = [q for q in getattr(self, 'flight_extra', []) if q // 1024 == b and q not in into[b]]
            if extra:
                c.append('#ifdef %s_FLIGHT_PART' % S)
                for q in extra:
                    if (q, False, False) in self.nodes:
                        c.append(' case %uu:goto %s;' % (q, self.label(q)))
                    else:
                        # (Only reached with A free of overflow: the part
                        # that stopped there.)
                        c.append(' case %uu:if((W[0]^(W[0]>>1))&040000u)return EXEC_OUT_OF_SCOPE;goto %s;' % (
                            q, self.label(q, False, True)))
                c.append('#endif')
            c.append(' default:return EXEC_OUT_OF_SCOPE;')
            c.append(' }')
            c.extend(body)
            if any('goto dispatch' in x for x in body):
                c.append('dispatch:')
                c.append(' p=agc_fixed_physical(W[5],W[4],io->superbank);')
                c.append(' switch(p){')
                for q in sorted(self.dispatch):
                    if (q, False, False) not in self.nodes:
                        continue
                    if q // 1024 == b:
                        c.append(' case %uu:goto %s;' % (q, self.label(q)))
                    else:
                        c.append(' case %uu:return %s(s,io,%uu);' % (q, self.callee(S, q // 1024), q))
                c.append(' default:return EXEC_OK;')
                c.append(' }')
            c.append('}')
            c.append('#endif')
        # (Labels a function never jumps to are fine: C labels need no use.)
        return c

    def callee(self, S, b):
        return '%s_RUN_%o' % (S, b)

    def callee_defs(self, sym):
        S = sym.upper()
        out = []
        for b in sorted(set(self.node_banks)):
            out += ['#if %s_HAS(0%o)' % (S, b), '#define %s_RUN_%o basic_run_%o' % (S, b, b)]
            if self.spec.get('flight_parts'):
                out += ['#elif defined(%s_FLIGHT_PART)' % S,
                        '#define %s_RUN_%o(s,io,at) ((void)(io),%s_leave((s),(at)))' % (S, b, S.lower())]
            out += ['#else', '#define %s_RUN_%o(s,io,at) ((void)(s),(void)(io),EXEC_OUT_OF_SCOPE)' % (S, b), '#endif']
        return out

    # ---- flight parts (spec 'flight_parts') -------------------------------
    # The C64 flight store links a module per part when the file's basic
    # code does not fit one job slot. 'flight_parts' lists the cuts
    # (physical addresses, ascending); <SYM>_FLIGHT_PART=j (1-based) builds
    # part j: the banks it holds (<SYM>_BANKS), the words of a cut bank on
    # its side of the cut (<SYM>_FP), and a routine table of its keys. A
    # run that reaches a word of another part sets Z to it and returns
    # EXEC_OK when the word is a key of some part, so the transfer engine
    # goes on in that part's module; any other word is refused. Part keys
    # (flight builds only) are the words a goto across a cut reaches and
    # every computed-transfer target. Without <SYM>_FLIGHT_PART the file
    # compiles as before.
    def flight_parts(self, S, lab):
        import bisect
        cuts = sorted(self.spec['flight_parts'])
        self.flight_cuts = cuts
        # 'flight_part_map' (optional): the part (1-based) of each range
        # between the cuts, so one part can hold ranges that are not next to
        # each other; without it range j is part j.
        pmap = self.spec.get('flight_part_map') or list(range(1, len(cuts) + 2))
        if len(pmap) != len(cuts) + 1 or sorted(set(pmap)) != list(range(1, max(pmap) + 1)):
            raise BasicError('flight_part_map needs a part for each of the %d ranges' % (len(cuts) + 1))
        self.flight_map = pmap
        part = lambda p: pmap[bisect.bisect_right(cuts, p)]
        extra = set()
        for k, body in self.nodes.items():
            for x in body or ():
                for m in re.finditer(r'goto ([A-Z][0-7]+c?);', x):
                    t = lab.get(m.group(1))
                    if t is not None and part(t[0]) != part(k[0]):
                        if t[1]:
                            raise BasicError('extended word %s across a flight cut' % self.where(t[0]))
                        extra.add(t[0])
        extra |= {p for p in self.dispatch if (p, False, False) in self.nodes}
        self.flight_extra = sorted(extra - self.entries)
        # A fixed-fixed key runs under any FB and SUPERBNK only in a spec
        # that says so ('fixed_fixed_any_fb'): the flight entry cannot tell
        # which FB a key there needs (interp_compile.py flight_table).
        if not self.spec.get('fixed_fixed_any_fb') and any(p // 1024 in (2, 3) for p in self.entries | extra):
            raise BasicError('flight parts need every key in a switched bank')
        words = sorted({k[0] for k in self.nodes})
        bounds = [0] + cuts + [0o200000]
        c = ['/* Flight parts (spec \'flight_parts\', docs/FLIGHT-PARTS.md): %s_FLIGHT_PART=j' % S,
             ' * builds part j of %d for the C64 flight store, the basic words between' % max(pmap),
             ' * its cuts (%s_OUT: the others); a run that reaches another part\'s word stops there' % S,
             ' * when it is a key (%s_leave), and is refused otherwise. */' % S.lower(),
             '#ifdef %s_FLIGHT_PART' % S]
        for j in range(max(pmap)):
            spans = [(bounds[i], bounds[i + 1]) for i in range(len(pmap)) if pmap[i] == j + 1]
            mask = 0
            for w in words:
                if any(lo <= w < hi for lo, hi in spans):
                    mask |= 1 << (w // 1024)
            if len(spans) == 1:
                lo, hi = spans[0]
                out = ' || '.join(t for t in ('(at)<%uu' % lo if lo else '', '(at)>=%uu' % hi if hi < bounds[-1] else '')
                                  if t)
            else:
                out = '!(%s)' % '||'.join('(%s)' % '&&'.join(t for t in ('(at)>=%uu' % lo if lo else '',
                                                                        '(at)<%uu' % hi if hi < bounds[-1] else '') if t)
                                          for lo, hi in spans)
            c += ['#%s %s_FLIGHT_PART==%d' % ('if' if j == 0 else 'elif', S, j + 1),
                  '#define %s_BANKS 0x%xull' % (S, mask), '#define %s_OUT(at) (%s)' % (S, out)]
        c += ['#else', '#error %s_FLIGHT_PART names no part' % S, '#endif',
              'static MAYBE_UNUSED enum exec_status %s_leave(apollo_overlay_state *s,uint16_t at);' % S.lower(),
              '#define %s_FP(at) if(%s_OUT(at)){return %s_leave(s,(at));}' % (S, S, S.lower()),
              '#else', '#define %s_FP(at)' % S, '#endif']
        return c

    def channel_helpers(self):
        c = ['/* Channel orders on the mission\'s output channels, as yaAGC\'s. */']
        c.append('static MAYBE_UNUSED agc_word *chan(cm_native_io *io,unsigned k){')
        c.append(' switch(k){%s default:return 0;}' % ''.join(
            'case 0%o:return &%s;' % (k, v if '->' in v else 'io->' + v) for k, v in sorted(self.channels.items())))
        c.append('}')
        c.append('static MAYBE_UNUSED agc_word oc16(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}')
        c.append('static MAYBE_UNUSED void ch_read(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend(*chan(io,k));}')
        # A channel write as the AGC's channel logic takes it: channel 13's
        # bits 12-14 set the HANDRUPT traps and read back 0; a write to
        # channel 33 only resets its latched bits 11-15 to 1.
        c.append('static MAYBE_UNUSED void ch_out(cm_native_io *io,unsigned k,agc_word v){')
        c.append(' if(k==013u){if(v&04000u)io->trap31a=1;if(v&010000u)io->trap31b=1;if(v&020000u)io->trap32=1;v&=043777u;}')
        c.append(' if(k==033u)v=(agc_word)(*chan(io,k)|076000u);')
        if 0o7 in self.channels:
            # Channel 7 keeps only bits 5-7 (superbank select).
            c.append(' if(k==07u)v&=0160u;')
        c.append(' *chan(io,k)=(agc_word)(v&077777u);')
        c.append('}')
        c.append('static MAYBE_UNUSED void ch_write(apollo_overlay_state *s,cm_native_io *io,unsigned k){ch_out(io,k,(agc_word)(oc16(W[0])&077777u));}')
        c.append('static MAYBE_UNUSED void ch_rand(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend((agc_word)(oc16(W[0])&*chan(io,k)));}')
        c.append('static MAYBE_UNUSED void ch_wand(apollo_overlay_state *s,cm_native_io *io,unsigned k){agc_word v=(agc_word)(oc16(W[0])&*chan(io,k));W[0]=agc_extend(v);ch_out(io,k,v);}')
        c.append('static MAYBE_UNUSED void ch_ror(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend((agc_word)(oc16(W[0])|*chan(io,k)));}')
        c.append('static MAYBE_UNUSED void ch_wor(apollo_overlay_state *s,cm_native_io *io,unsigned k){agc_word v=(agc_word)(oc16(W[0])|*chan(io,k));W[0]=agc_extend(v);ch_out(io,k,v);}')
        c.append('static MAYBE_UNUSED void ch_rxor(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend((agc_word)(oc16(W[0])^*chan(io,k)));}')
        return c
