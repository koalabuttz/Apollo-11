#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Reader for the yaYUL assembly listings (build/fixed-data/<program>/
assembly.lst): every assembled line with its source file, address, words,
label, op and operand columns, the fixed-memory words by physical address,
and the program's labels. Used by tools/interp_compile.py and the
compiled-routine differential."""
import os
import re

LISTINGS = {'lm': 'build/fixed-data/Luminary099/assembly.lst',
            'cm': 'build/fixed-data/Comanche055/assembly.lst'}
PREFIX = re.compile(r'^\d{6},\d{6}: ')
BANKED = re.compile(r'^(\d\d),(\d{4})$')
FIXED_FIXED = re.compile(r'^(\d{4})$')
OCTAL5 = re.compile(r'^[0-7]{5}$')


def physical(bank, loc):
    """Physical fixed word (bank*1024+offset) of bank,loc."""
    return bank * 1024 + (loc & 0o1777)


def fixed_fixed(loc):
    return 2048 + (loc - 0o4000)


class Line:
    __slots__ = ('src', 'phys', 'bank', 'loc', 'words', 'label', 'op', 'operand', 'text')

    def __repr__(self):
        return '%s %s %s %s %s' % (self.where(), ' '.join('%05o' % w for w in self.words),
                                   self.label, self.op, self.operand)

    def where(self):
        return '%02o,%04o' % (self.bank, self.loc)


class Listing:
    def __init__(self, path):
        self.lines = []
        self.by_phys = {}
        self.rope = {}
        self.labels = {}
        image = os.path.join(os.path.dirname(path), 'MAIN.agc.bin')
        self.image = open(image, 'rb').read() if os.path.exists(image) else None
        src = 'MAIN.agc'
        for raw in open(path, errors='replace'):
            raw = raw.rstrip('\n')
            if raw.startswith('(End of include-file'):
                src = 'MAIN.agc'
                continue
            if not PREFIX.match(raw):
                continue
            body = raw[15:]
            if body.startswith('$'):
                src = body[1:].split()[0]
                continue
            addr = raw[15:22].strip()
            m = BANKED.match(addr)
            f = FIXED_FIXED.match(addr)
            if m:
                bank, loc = int(m.group(1), 8), int(m.group(2), 8)
            elif f:
                loc = int(f.group(1), 8)
                if loc < 0o4000:
                    continue
                bank = 2 if loc < 0o6000 else 3
            else:
                continue
            words = []
            for a, b in ((33, 38), (39, 44)):
                t = raw[a:b]
                if OCTAL5.match(t):
                    words.append(int(t, 8))
            text = raw[46:].split('\t')[0]
            ln = Line()
            ln.src, ln.bank, ln.loc = src, bank, loc
            ln.phys = physical(bank, loc)
            ln.words = words
            ln.label = raw[46:65].strip()
            ln.op = raw[65:74].strip()
            ln.operand = raw[74:].split('\t')[0].strip() if len(raw) > 74 else ''
            ln.text = text
            if ln.label and ln.op not in ('=', 'EQUALS', 'ERASE') and words:
                self.labels.setdefault(ln.label, ln.phys)
            if words:
                self.lines.append(ln)
                self.by_phys[ln.phys] = ln
                for i, w in enumerate(words):
                    self.rope[ln.phys + i] = w

    def file_lines(self, name):
        return [l for l in self.lines if l.src == name]

    def word(self, phys):
        if phys in self.rope:
            return self.rope[phys]
        return self.image_word(phys)

    def image_word(self, phys):
        """Rope word from MAIN.agc.bin beside the listing, for words the
        listing does not show (filler past a constant that an order reads)."""
        if self.image is None:
            return 0
        bank, off = divmod(phys, 1024)
        if bank < 4:
            bank ^= 2
        o = 2 * (bank * 1024 + off)
        if o + 1 >= len(self.image):
            return 0
        return (self.image[o] << 8 | self.image[o + 1]) >> 1

    def label_at(self, phys):
        for k, v in self.labels.items():
            if v == phys:
                return k
        return None

    def jump_table(self, label, count, first=0):
        """{name: target} for the TCF table at label (entry `first` there)."""
        base = self.labels[label]
        out = {}
        for i in range(count):
            ln = self.by_phys.get(base + i)
            if ln is None or ln.op != 'TCF':
                continue
            out[ln.operand.split()[0]] = ln.words[0] & 0o7777
        return out


def load(mission):
    return Listing(LISTINGS[mission])
