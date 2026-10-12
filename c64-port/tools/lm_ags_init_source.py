#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source check for lm-ags-init.

Reads the host assembly listing of Luminary099
(build/fixed-data/Luminary099/assembly.lst) and its rope, and checks
src/lm_ags_init.c against them:
  - every interpretive case (one per order-pair or store word) carries the
    file's words as its octal literals, in order, from its LOC on;
  - every basic AT() label is an instruction word of the file, and its
    code holds, after the file's #defines, the instruction's erasable
    address (the high word K of a double-word order), its branch target,
    or, for a fixed operand, the rope word there;
  - every word of the file is owned: an AT label, a word of an
    interpretive case, the word after EXTEND, a compiled site operand
    (src/lm_ags_init_sites.inc, checked against the rope), a CADR in the
    shared INTER-BANK site table, or a constant the code reads (B32DATA,
    checked against the rope, or a K_* constant);
  - the K_* constants equal the rope words at their symbols and the X_*
    addresses equal their symbols.
Writes build/lm-ags-init-source-map.json.
"""
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import rope_word  # noqa: E402

FILE = 'AGS_INITIALIZATION.agc'
LST = R / 'build/fixed-data/Luminary099/assembly.lst'
ROPE = R / 'build/fixed-data/Luminary099/MAIN.agc.bin'
SRC = R / 'src/lm_ags_init.c'
SITES = R / 'src/lm_ags_init_sites.inc'
INTERBANK = R / 'src/lm_interbank_sites.inc'
BANK = 0o32
WORD = re.compile(r'^\d{6},\d{6}: (?:(\d\d),)?(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
K_SYMBOL = {'ONE': 'ONE', 'TEN': 'TEN', 'BIT6': 'BIT6', 'IMUSEBIT': 'BIT8', 'REFSMBIT': 'BIT13',
            'V06N16': 'V06N16', 'V50N16': 'V50N16', '20SEC': '20SEC'}
DATA = ('CADR', 'ADRES', '2CADR', 'GENADR', 'DEC', 'OCT', 'ECADR', '2DEC', 'VN', 'BBCON', '2OCT')
OPS = set('''TC TCF CA CAF CAE CS CCS AD ADS SU MASK TS XCH LXCH QXCH DXCH DCA DCS DAS INDEX EXTEND INHINT
 RELINT BZF BZMF INCR AUG DIM MP DV READ WRITE RAND ROR RXOR WAND WOR'''.split())
QUARTER = {'CCS', 'TS', 'XCH', 'LXCH', 'QXCH', 'ADS', 'INCR', 'AUG', 'DIM', 'DXCH', 'DAS'}
DOUBLE = {'DXCH', 'DAS', 'DCA', 'DCS'}
BRANCH = {'TC', 'TCF', 'BZF', 'BZMF'}


def physical(addr):
    return 2048 + addr - 0o4000 if addr >= 0o4000 else BANK * 1024 + addr - 0o2000


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
                addr = int(m.group(2), 8)
                text = m.group(5).split('\t#')[0].split()
                if len(text) > 1 and text[0] not in OPS and text[0] not in DATA and not text[0].startswith(('VLOAD', 'STO')):
                    if text[0] not in ('SET', 'RTB', 'CALL', 'STCALL', 'STODL', 'STOVL', 'STORE', 'DSU', 'VXSC',
                                       'VAD', 'MXV', 'VSL8', 'LXA,1', 'SXA,1', 'EXIT'):
                        text = text[1:]   # label
                words[physical(addr)] = (int(m.group(3), 8), ' '.join(text), addr)
                if m.group(4):
                    words[physical(addr) + 1] = (int(m.group(4), 8), ' '.join(text) + ' (second word)', addr + 1)
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
        p = physical(int(loc, 8))
        lits = [int(x, 8) for x in re.findall(r'\b(0[0-7]{5})\b', code)]
        for i, v in enumerate(lits):
            w = rope_word(rope, p + i)
            if w != v:
                fails.append(f'interpretive case {loc}: literal {i} {v:05o} != rope {w:05o}')
            owned[p + i] = f'interpretive case {loc}'
        if not lits:
            if 'exit_' not in code or rope_word(rope, p) != 0o77776:
                fails.append(f'interpretive case {loc}: no words')
            owned[p] = f'interpretive case {loc} (EXIT)'
    # Basic labels and their operands.
    basic = src[src.index('static int basic('):src.index('/* Is Z (or LOC at NEWOPS)')]
    parts = re.split(r'\bAT\((0[0-7]+)\);', basic)
    extend = False
    for a, code in zip(parts[1::2], parts[2::2]):
        a = int(a, 8)
        p = physical(a)
        if p not in words:
            fails.append(f'AT({a:05o}) is not a word of the file')
            continue
        owned.setdefault(p, f'basic AT({a:05o})')
        w, text, _ = words[p]
        op = text.split()[0]
        prev = words.get(p - 1, (0, '', 0))[1].split()
        extend = bool(prev) and prev[0] == 'EXTEND'
        if op == 'EXTEND':
            continue
        if op in QUARTER and not (op == 'CCS' and False):
            k = w & 0o1777
        elif op in ('DCA', 'DCS', 'BZF', 'BZMF') and extend:
            k = w & 0o7777
        else:
            k = w & 0o7777
        if op in DOUBLE:
            k -= 1
        lits = literals(code, defines)
        if op in BRANCH:
            need, what = k, 'branch target'
        elif k >= 0o2000 or (op in ('CAF',) and k >= 0o2000):
            need, what = rope_word(rope, physical(k)), 'fixed operand word'
        else:
            need, what = k, 'erasable address'
        if op == 'CCS' and k == 0:
            need, what = 0, 'register A'
        if need not in lits:
            fails.append(f'AT({a:05o}) {text}: {what} {need:05o} not in its code')
    # Site operands, interbank CADRs and data.
    sites = {int(p): int(v, 8) for p, v in re.findall(r'\{(\d+)u,(0[0-7]+)u\}', SITES.read_text())}
    interbank = {int(p) for p, v in re.findall(r'\{(\d+)u?,\s*(0?\d+)u?\}', INTERBANK.read_text())}
    for p, v in sites.items():
        if rope_word(rope, p) != v:
            fails.append(f'site {p}: {v:05o} != rope {rope_word(rope, p):05o}')
    data = [int(x, 8) for x in re.search(r'B32DATA\[\w*\]=\{(.*?)\}', src, re.S).group(1).replace('u', '').split(',')]
    for i, v in enumerate(data):
        if rope_word(rope, physical(0o2174) + i) != v:
            fails.append(f'B32DATA[{i}] {v:05o} != rope {rope_word(rope, physical(0o2174) + i):05o}')
    kvals = {int(v, 8) for n, v in re.findall(r'#define K_(\w+) (0[0-7]*|0|[1-9]\d*)u', src)}
    for p, (w, text, addr) in sorted(words.items()):
        if p in owned:
            continue
        op = text.split()[0] if text else ''
        prev = words.get(p - 1, (0, '', 0))[1].split()
        if prev and prev[0] == 'EXTEND':
            owned[p] = 'with EXTEND'
        elif p in sites:
            owned[p] = 'src/lm_ags_init_sites.inc'
        elif p in interbank and op == 'CADR':
            owned[p] = 'src/lm_interbank_sites.inc'
        elif 0o2174 <= addr <= 0o2216:
            owned[p] = 'B32DATA'
        elif op == 'VN' and w in kvals:
            owned[p] = 'K_ constant'
        elif op == 'VN':
            owned[p] = 'display code no code reads (V01N14, V50N00A)'
        else:
            fails.append(f'unowned word 32,{addr:04o} {w:05o} {text}')
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
    (R / 'build/lm-ags-init-source-map.json').write_text(json.dumps(out, indent=1) + '\n')
    if fails:
        print('\n'.join(fails[:60]))
        print(f'FAIL: {len(fails)} lm-ags-init source problems')
        return 1
    print(f'PASS: {len(words)} words of {FILE} owned; operands, sites and constants match the rope')
    return 0


if __name__ == '__main__':
    sys.exit(main())
