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

QUARTER = {1: 'CCS', 2: ('DAS', 'LXCH', 'INCR', 'ADS'), 5: ('INDEX', 'DXCH', 'TS', 'XCH')}
EXT_QUARTER = {1: 'DV', 2: ('MSU', 'QXCH', 'AUG', 'DIM'), 6: 'SU'}
CHANNEL_OPS = ('READ', 'WRITE', 'RAND', 'WAND', 'ROR', 'WOR', 'RXOR', 'EDRUPT')
# Erasable-reading orders and their fixed-operand forms.
FIXED_FORM = {'CA': 'caf', 'CS': 'csf', 'AD': 'adf', 'MASK': 'maskf', 'MP': 'mpf', 'SU': 'suf',
              'DCA': 'dcaf', 'DCS': 'dcsf', 'CCS': 'ccsf', 'DV': 'dvf'}
DOUBLE = ('DCA', 'DCS', 'DXCH', 'DAS')


class BasicError(Exception):
    pass


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
        self.L = cp.L
        self.spec = cp.spec
        self.nodes = {}          # (phys, ext) -> list of C statements (or None while walking)
        self.dispatch = set()    # phys reachable by a computed transfer (labels)
        self.entries = set()     # basic starts
        self.fixed_tables = []   # (lo phys, hi phys)
        for lab, (lo, hi) in self.spec.get('fixed_tables', {}).items():
            # A label, or the fixed-fixed logical address (octal) of an
            # EQUALS name such as LIMITS.
            b = self.L.labels[lab] if lab in self.L.labels else 2048 + int(lab, 8) - 0o4000
            self.fixed_tables.append((b + lo, b + hi, lab, lo))
        self.channels = self.spec.get('channels', {})
        if self.channels == 'all':
            # Every channel of the CM io record (include/cm_native_image.h).
            self.channels = {k: 'channel%02o' % k for k in (0o5, 0o6, 0o11, 0o12, 0o13, 0o14, 0o30, 0o31, 0o32, 0o33)}
        self.work = []
        self.words = set()       # every word compiled as basic code
        self.operands = set()    # inline operand words of outside TCs
        self.data_landings = set()
        self.erasable_refs = set()   # erasable addresses the basic code names

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
        if not self.in_file(phys):
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
            if self.is_data(k[0]) and self.where(k[0]) not in self.spec.get('execute_data', ()):
                # Control reaching a constant: the file's flow never does
                # (the paths here are CCS exits and TS skips no admitted
                # state takes); the differential checks none is taken.
                self.data_landings.add(k[0])
                self.nodes[k] = ['/* %s %s: data, never executed */' % (self.where(k[0]), self.text(k[0])),
                                 'return EXEC_INVALID_ACTION;']
                continue
            self.nodes[k] = self.compile(*k)

    def is_data(self, phys):
        ln = self.L.by_phys.get(phys)
        if ln is None:
            return True     # second word of a two-word constant
        return ln.op in self.CONST_OPS

    def outside_tc(self, phys, target_logical):
        """TC to an outside routine: the return is the word after the
        inline operands (constant lines after the TC)."""
        ret = phys + 1
        while True:
            ln = self.L.by_phys.get(ret)
            if ln is None or ln.op not in self.CONST_OPS:
                break
            self.operands.update(range(ret, ret + len(ln.words)))
            ret += len(ln.words)
        name = self.L.label_at(self.phys_of(target_logical, phys)) or ('%04o' % target_logical)
        self.cp.externals[self.phys_of(target_logical, phys)] = name
        if self.in_file(ret):
            self.cp.add_start(ret, 'b', 'return from ' + name)
        return name

    CONST_OPS = ('OCT', 'OCTAL', 'DEC', 'DEC*', '2DEC', '2DEC*', 'ADRES', 'CADR', 'FCADR', 'GENADR',
                 '2CADR', '2FCADR', 'BBCON', 'REMADR', '2OCT', '2BCADR')

    def jump(self, phys, target_logical, call, clean=False):
        """Statements for TC/TCF (call) to a static target."""
        here = phys
        st = []
        if call:
            st.append('ai_tc(s,0%05ou);' % ((self.logical(here) + 1) & 0o7777))
        tp = self.phys_of(target_logical, here)
        if target_logical == 0o2 and call:
            raise BasicError('TC Q handled elsewhere')
        if tp is not None and self.in_file(tp):
            st.append('goto %s;' % self.add(tp, False, clean))
            if call:
                self.dispatch.add(here + 1)
                self.add(here + 1)
            return st
        if tp is not None and self.L.labels.get('INTPRET') == tp and call:
            # TC INTPRET: the interpretive code at the next word.
            st[-1:] = ['ai_tc(s,0%05ou);W[5]=0%04o;intpret(s,io);return EXEC_OK; /* INTPRET */'
                       % ((self.logical(here) + 1) & 0o7777, self.cp.intpret)]
            self.cp.add_start(here + 1, 'i', 'after TC INTPRET')
            return st
        if tp is None:
            raise BasicError('transfer to erasable %04o at %s' % (target_logical, self.where(here)))
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
            st.append(self.index_value(a, phys))
            st.extend(self.indexed(nxt, ext, clean))
            return st
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
        if a is None or op in ('EXTEND', 'INDEX', 'RESUME') or op in CHANNEL_OPS:
            raise BasicError('INDEX of %s at %s' % (op, self.where(phys)))
        st.append('e=(long)0%o+ix;if(e<0||e>0%o)return EXEC_INVALID_ACTION;' % (a, lim))
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
        if op in ('TC', 'TCF'):
            call = op == 'TC'
            if dyn:
                if call:
                    st.append('ai_tc(s,0%05ou);' % ((self.logical(here) + 1) & 0o7777))
                    self.dispatch.add(nxt)
                    self.add(nxt)
                st.append('W[5]=(agc_word)e;goto dispatch;')
                return st
            if a < 0o2000 and call and a not in (3, 4, 5, 6, 7) and not 0o20 <= a <= 0o23:
                # TC A (XXALQ), TC L (XLQ), TC Q (RETURN) or TC to a stored
                # return: the word there runs as an instruction, which the
                # file only ever makes a TC to fixed memory (Q = K+1).
                reg = {0: 'W[0]', 1: 'W[1]', 2: 'W[2]'}.get(a, 'ai_word(s,0%o)' % a)
                st.append('e=(long)(agc_correct(%s)&077777u);' % reg if a < 3 else 'e=(long)%s;' % reg)
                st.append('if(e<02000||e>07777)return EXEC_INVALID_ACTION;')
                st.append('ai_tc(s,0%ou);W[5]=(agc_word)e;goto dispatch;' % (a + 1))
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
        if op in CHANNEL_OPS:
            if a not in self.channels:
                raise BasicError('channel %o at %s' % (a, self.where(here)))
            if op == 'EDRUPT':
                raise BasicError('EDRUPT at %s' % self.where(here))
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
        if op == 'MSU':
            # On an ordinary erasable word, as yaAGC takes it.
            if dyn or a < 0o20 or a >= 0o2000:
                raise BasicError('MSU on %o at %s' % (a, self.where(here)))
            self.uses_msu = True
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
            if op in ('DCA', 'DCS'):
                fp0 = self.phys_of(a - 1, here)
                st.append('{static const agc_word k[2]={0%05ou,0%05ou};ai_%s(s,k);} /* %s */' % (
                    self.L.word(fp0), self.L.word(fp), lo_name, self.L.label_at(fp0) or sym))
            else:
                st.append('ai_%s(s,0%05ou); /* %s */' % (lo_name, self.L.word(fp), sym))
            st.append(cont())
            return st
        if writes and a in (4, 5, 6) or (op == 'DXCH' and a in (4, 5, 6, 7)):
            raise BasicError('%s on %o at %s' % (op, a, self.where(here)))
        st.append(self.operand_stmt(op, a, here, False))
        st.append(cont())
        return st

    # ---- output ----------------------------------------------------------
    def emit(self, sym):
        c = []
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
        c.append('static MAYBE_UNUSED void intpret(apollo_overlay_state *s,cm_native_io *io){p66_bias_io l;')
        c.append(' l.superbank=io->superbank;l.interrupt_enabled=io->interrupt_enabled;cmi_intpret(s,&l);')
        c.append(' io->superbank=l.superbank;io->interrupt_enabled=l.interrupt_enabled;}')
        if self.channels:
            c.extend(self.channel_helpers())
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
        c.append(' if(e>=06000u)p=3u*1024u+e-06000u;else if(e>=04000u)p=2u*1024u+e-04000u;else p=0%ou*1024u+e-02000u;' % self.bank(min(self.entries) if self.entries else self.cp.lo))
        for lo, hi, lab, off in self.fixed_tables:
            c.append(' if(p>=%uu&&p<=%uu){*k=T%o[p-%uu];return 1;}' % (lo, hi, lo, lo))
        c.append(' (void)p;(void)k;return 0;')
        c.append('}')
        if getattr(self, 'uses_msu', False):
            c.append('/* EXTEND MSU on an ordinary erasable word: the two\'s-complement')
            c.append(' * difference of the 15-bit operands, less one when negative. */')
            c.append('static void msu(apollo_overlay_state *s,unsigned k){')
            c.append(' unsigned a=agc_correct(W[0])&077777u,b=~(unsigned)ai_word(s,k)&077777u,d=a+b+1u;')
            c.append(' if(d&040000u)d=(d|0100000u)-1u;')
            c.append(' W[0]=agc_extend((agc_word)(d&077777u));')
            c.append('}')
        c.append('/* Every basic word of the file. */')
        c.append('static MAYBE_UNUSED enum exec_status basic_run(apollo_overlay_state *s,cm_native_io *io,uint16_t at){')
        c.append(' long ix=0,e=0;uint16_t p;')
        c.append(' (void)ix;(void)e;')
        c.append(' switch(at){')
        for p in sorted(self.entries):
            c.append(' case %uu:goto %s;' % (p, self.label(p)))
        c.append(' default:return EXEC_OUT_OF_SCOPE;')
        c.append(' }')
        for k in sorted(self.nodes):
            c.append('%s:' % self.label(*k))
            body = self.nodes[k]
            twin = (k[0], k[1], False)
            if k[2] and twin in self.nodes and self.nodes[twin] == [x.replace(self.label(*k), self.label(*twin)) for x in body]:
                # The same code as the copy reached with A unknown.
                c.append(' goto %s;' % self.label(*twin))
                continue
            for x in body:
                c.append(' ' + x)
        c.append('dispatch:')
        c.append(' p=agc_fixed_physical(W[5],W[4],io->superbank);')
        c.append(' switch(p){')
        for p in sorted(self.dispatch):
            if (p, False, False) in self.nodes:
                c.append(' case %uu:goto %s;' % (p, self.label(p)))
        c.append(' default:return EXEC_OK;')
        c.append(' }')
        c.append('}')
        return c

    def channel_helpers(self):
        c = ['/* Channel orders on the mission\'s output channels, as yaAGC\'s. */']
        c.append('static MAYBE_UNUSED agc_word *chan(cm_native_io *io,unsigned k){')
        c.append(' switch(k){%s default:return 0;}' % ''.join(
            'case 0%o:return &io->%s;' % (k, v) for k, v in sorted(self.channels.items())))
        c.append('}')
        c.append('static MAYBE_UNUSED agc_word oc16(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}')
        c.append('static MAYBE_UNUSED void ch_read(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend(*chan(io,k));}')
        # A channel write as the AGC's channel logic takes it: channel 13's
        # bits 12-14 set the HANDRUPT traps and read back 0; a write to
        # channel 33 only resets its latched bits 11-15 to 1.
        c.append('static MAYBE_UNUSED void ch_out(cm_native_io *io,unsigned k,agc_word v){')
        c.append(' if(k==013u){if(v&04000u)io->trap31a=1;if(v&010000u)io->trap31b=1;if(v&020000u)io->trap32=1;v&=043777u;}')
        c.append(' if(k==033u)v=(agc_word)(*chan(io,k)|076000u);')
        c.append(' *chan(io,k)=(agc_word)(v&077777u);')
        c.append('}')
        c.append('static MAYBE_UNUSED void ch_write(apollo_overlay_state *s,cm_native_io *io,unsigned k){ch_out(io,k,(agc_word)(oc16(W[0])&077777u));}')
        c.append('static MAYBE_UNUSED void ch_rand(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend((agc_word)(oc16(W[0])&*chan(io,k)));}')
        c.append('static MAYBE_UNUSED void ch_wand(apollo_overlay_state *s,cm_native_io *io,unsigned k){agc_word v=(agc_word)(oc16(W[0])&*chan(io,k));W[0]=agc_extend(v);ch_out(io,k,v);}')
        c.append('static MAYBE_UNUSED void ch_ror(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend((agc_word)(oc16(W[0])|*chan(io,k)));}')
        c.append('static MAYBE_UNUSED void ch_wor(apollo_overlay_state *s,cm_native_io *io,unsigned k){agc_word v=(agc_word)(oc16(W[0])|*chan(io,k));W[0]=agc_extend(v);ch_out(io,k,v);}')
        c.append('static MAYBE_UNUSED void ch_rxor(apollo_overlay_state *s,cm_native_io *io,unsigned k){W[0]=agc_extend((agc_word)(oc16(W[0])^*chan(io,k)));}')
        return c
