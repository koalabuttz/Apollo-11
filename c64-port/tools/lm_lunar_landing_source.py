#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source check for lm-lunar-landing.

Reads the host assembly listing of Luminary099
(build/fixed-data/Luminary099/assembly.lst) and its rope, and checks
src/lm_lunar_landing.c against them:
  - every interpretive case (one per order-pair or store word) carries the
    file's words as its octal literals, in order, from its LOC on (an order
    pair named twice in a row is the PUSHUP word of two pushdown operands);
  - every basic AT() label is an instruction word of the file, and its
    code holds, after the file's #defines, the instruction's erasable
    address, its branch target, the channel it names (chan12, chan33), the
    interrupt state INHINT or RELINT leaves, or, for a fixed operand, the
    rope word there;
  - every word of the file is owned: an AT label, a word of an
    interpretive case, the word after EXTEND, a site operand in the shared
    PHASCHNG table (src/lm_phase_sites.inc) or INTER-BANK table
    (src/lm_interbank_sites.inc), each checked against the rope, or a
    constant the code reads (B32DATA, checked against the rope, or a K_*
    constant);
  - B23DATA equals the rope at the interpretive constants UNITZ-HI6ZEROS;
  - the K_* constants equal the rope words at their symbols and the X_*
    addresses equal their symbols.
Writes build/lm-lunar-landing-source-map.json.
"""
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import rope_word  # noqa: E402

FILE = 'THE_LUNAR_LANDING.agc'
LST = R / 'build/fixed-data/Luminary099/assembly.lst'
ROPE = R / 'build/fixed-data/Luminary099/MAIN.agc.bin'
SRC = R / 'src/lm_lunar_landing.c'
SHARED = (R / 'src/lm_phase_sites.inc', R / 'src/lm_interbank_sites.inc')
WORD = re.compile(r'^\d{6},\d{6}: (\d\d),(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
K_SYMBOL = {'V06N43': 'V06N43*'}
DATA = ('CADR', 'ADRES', '2CADR', 'GENADR', 'DEC', 'OCT', 'ECADR', '2DEC', 'VN', 'BBCON', '2OCT', 'FCADR')
OPS = set('''TC TCF CA CAF CAE CS CCS AD ADS SU MASK TS XCH LXCH QXCH DXCH DCA DCS DAS INDEX EXTEND INHINT
 RELINT BZF BZMF INCR AUG DIM MP DV READ WRITE RAND ROR RXOR WAND WOR'''.split())
QUARTER = {'CCS', 'TS', 'XCH', 'LXCH', 'QXCH', 'ADS', 'INCR', 'AUG', 'DIM', 'DXCH', 'DAS'}
DOUBLE = {'DXCH', 'DAS', 'DCA', 'DCS'}
BRANCH = {'TC', 'TCF', 'BZF', 'BZMF'}
CHANNEL = {'RAND', 'WAND', 'READ', 'WRITE', 'ROR', 'WOR', 'RXOR'}
DATA_SPAN = (0o32, 0o3253, 0o3262)   # B32DATA


def physical(bank, addr):
    return 2048 + addr - 0o4000 if addr >= 0o4000 else bank * 1024 + addr - 0o2000


def bank_of(addr):
    return 0o31 if addr < 0o2776 else 0o32


def listing():
    words, on, symbols = {}, False, {}
    text_all = LST.read_text(errors='replace')
    for t in text_all.splitlines():
        if 'Filename:' in t:
            on = FILE in t
            continue
        if on:
            m = WORD.match(t)
            if m and not m.group(5).lstrip().startswith('#'):
                bank, addr = int(m.group(1), 8), int(m.group(2), 8)
                if bank not in (0o31, 0o32):
                    continue
                text = m.group(5).split('\t#')[0].split()
                # A label is the first field when the second is an opcode.
                if len(text) > 1 and (text[1] in OPS or text[1] in DATA) and text[0] not in OPS:
                    text = text[1:]
                p = physical(bank, addr)
                words[p] = (int(m.group(3), 8), ' '.join(text), bank, addr)
                if m.group(4):
                    words[p + 1] = (int(m.group(4), 8), ' '.join(text) + ' (second word)', bank, addr + 1)
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+((?:\d\d,|E\d,)?\d{4})', text_all):
        symbols[m.group(2)] = (m.group(1), m.group(3))
    return words, symbols


def sym_physical(value):
    if ',' in value:
        b, a = value.split(',')
        return int(b, 8) * 1024 + int(a, 8) - 0o2000
    return 2048 + int(value, 8) - 0o4000


def literals(code, defines):
    for name, val in defines.items():
        code = re.sub(rf'\b{name}\b', val, code)
    vals = set()
    for x in re.findall(r'\b(0[0-7]*|[1-9][0-9]*)u?\b', code):
        vals.add(int(x, 8) if x.startswith('0') else int(x))
    for x in re.findall(r'L(0[0-7]+)', code):
        vals.add(int(x, 8))
    return vals


def main():
    rope = ROPE.read_bytes()
    words, symbols = listing()
    src = SRC.read_text()
    defines = {n: v for n, v in re.findall(r'#define (\w+) (0[0-7]*)u', src)}
    fails, owned = [], {}
    # Interpretive cases.
    body = src[src.index('static int interpretive('):src.index('/* ---- Basic helpers')]
    for loc, code in re.findall(r'case (0[0-7]+):(.*)', body):
        loc = int(loc, 8)
        p = physical(bank_of(loc), loc)
        lits = [int(x, 8) for x in re.findall(r'\b(0[0-7]{5})\b', code)]
        lits = [v for i, v in enumerate(lits) if not (i and v == lits[i - 1] and v & 0o40000)]
        for i, v in enumerate(lits):
            w = rope_word(rope, p + i)
            if w != v:
                fails.append(f'interpretive case {loc:05o}: literal {i} {v:05o} != rope {w:05o}')
            owned[p + i] = f'interpretive case {loc:05o}'
        if not lits:
            if 'exit_' not in code or rope_word(rope, p) != 0o77776:
                fails.append(f'interpretive case {loc:05o}: no words')
            owned[p] = f'interpretive case {loc:05o} (EXIT)'
    # Basic labels and their operands.
    basic = src[src.index('static int basic('):src.index('/* The file\'s words')]
    parts = re.split(r'\bAT\((0[0-7]+)\);', basic)
    for a, code in zip(parts[1::2], parts[2::2]):
        a = int(a, 8)
        bank = bank_of(a)
        p = physical(bank, a)
        if p not in words:
            fails.append(f'AT({a:05o}) is not a word of the file')
            continue
        owned.setdefault(p, f'basic AT({a:05o})')
        w, text, _, _ = words[p]
        op = text.split()[0]
        if op == 'EXTEND':
            continue
        if op == 'INHINT' or op == 'RELINT':
            if f'interrupt_enabled={1 if op == "RELINT" else 0};' not in code:
                fails.append(f'AT({a:05o}) {op}: interrupt state not set')
            continue
        if op in CHANNEL:
            if f'chan{w & 0o777:o}' not in code:
                fails.append(f'AT({a:05o}) {text}: channel {w & 0o777:o} not in its code')
            continue
        k = w & 0o1777 if op in QUARTER else w & 0o7777
        if op in DOUBLE:
            k -= 1
        lits = literals(code, defines)
        if op in BRANCH:
            need, what = k, 'branch target'
        elif k >= 0o2000:
            need, what = rope_word(rope, physical(bank, k)), 'fixed operand word'
        else:
            need, what = k, 'erasable address'
        if need not in lits:
            fails.append(f'AT({a:05o}) {text}: {what} {need:05o} not in its code')
    # Shared site operands and data.
    sites = {}
    for path in SHARED:
        for p, v in re.findall(r'\{(\d+)u,\s*(0[0-7]+)u?\}', path.read_text()):
            sites[int(p)] = (int(v, 8), path.name)
    data = [int(x, 8) for x in re.search(r'B32DATA\[\w*\]=\{(.*?)\}', src, re.S).group(1).replace('u', '').split(',')]
    db, d0, d1 = DATA_SPAN
    if len(data) != d1 - d0 + 1:
        fails.append(f'B32DATA has {len(data)} words, not {d1 - d0 + 1}')
    for i, v in enumerate(data):
        if rope_word(rope, physical(db, d0) + i) != v:
            fails.append(f'B32DATA[{i}] {v:05o} != rope {rope_word(rope, physical(db, d0) + i):05o}')
    # The interpretive constants UNITZ .. HI6ZEROS (bank 23 02513-02526).
    b23 = [int(x, 8) for x in re.search(r'B23DATA\[\w*\]=\{(.*?)\}', src, re.S).group(1).replace('u', '').split(',')]
    if len(b23) != 12 or [sym_physical(symbols[n][1]) for n in ('UNITZ', 'UNITY', 'UNITX', 'ZEROVECS', 'HI6ZEROS')] != \
            [physical(0o23, a) for a in (0o2513, 0o2515, 0o2517, 0o2521, 0o2521)]:
        fails.append('B23DATA: span or symbols differ')
    for i, v in enumerate(b23):
        if rope_word(rope, physical(0o23, 0o2513) + i) != v:
            fails.append(f'B23DATA[{i}] {v:05o} != rope {rope_word(rope, physical(0o23, 0o2513) + i):05o}')
    kvals = {int(v, 8) for n, v in re.findall(r'#define K_(\w+) (0[0-7]*|0)u', src)}
    for p, (w, text, bank, addr) in sorted(words.items()):
        if p in owned:
            continue
        op = text.split()[0] if text else ''
        prev = words.get(p - 1, (0, '', 0, 0))[1].split()
        if prev and prev[0] == 'EXTEND':
            owned[p] = 'with EXTEND'
        elif p in sites and op in ('OCT', 'CADR'):
            if sites[p][0] != rope_word(rope, p):
                fails.append(f'site {bank:02o},{addr:04o}: {sites[p][0]:05o} != rope {rope_word(rope, p):05o}')
            owned[p] = f'src/{sites[p][1]}'
        elif bank == db and d0 <= addr <= d1:
            owned[p] = 'B32DATA'
        elif op == 'VN' and w in kvals:
            owned[p] = 'K_ constant'
        else:
            fails.append(f'unowned word {bank:02o},{addr:04o} {w:05o} {text}')
    for name, val in re.findall(r'#define K_(\w+) (0[0-7]*|0)u', src):
        sym = K_SYMBOL.get(name, name)
        if sym not in symbols:
            fails.append(f'K_{name}: no symbol {sym}')
            continue
        w = rope_word(rope, sym_physical(symbols[sym][1]))
        if w != int(val, 8):
            fails.append(f'K_{name} {int(val, 8):05o} != rope {w:05o} at {symbols[sym][1]}')
    for name, val in re.findall(r'#define X_(\w+) (0[0-7]+)u', src):
        if name not in symbols or symbols[name][1] != f'{int(val, 8):04o}':
            fails.append(f'X_{name} {val}: symbol {symbols.get(name)}')
    out = {'file': FILE, 'words': len(words),
           'owners': {f'{p // 1024:02o},{p % 1024 + 0o2000:04o}': o for p, o in sorted(owned.items())}}
    (R / 'build/lm-lunar-landing-source-map.json').write_text(json.dumps(out, indent=1) + '\n')
    if fails:
        print('\n'.join(fails[:60]))
        print(f'FAIL: {len(fails)} lm-lunar-landing source problems')
        return 1
    print(f'PASS: {len(words)} words of {FILE} owned; operands, sites and constants match the rope')
    return 0


if __name__ == '__main__':
    sys.exit(main())
