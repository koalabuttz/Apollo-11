#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source check for lm-imu-performance.

Reads the fresh host assembly listing of Luminary099
(build/fixed-data/Luminary099/assembly.lst) and its rope, and checks
src/lm_imu_performance.c against them:
  - every interpretive case (one per order-pair or store word) carries the
    file's words as its octal literals, in order, from its LOC on;
  - every basic AT() label is an instruction word of the two files, and
    every word of the files is owned: an AT label, a word of an
    interpretive case, or an operand or constant word listed below;
  - the K_* constants equal the rope words at their symbols, the X_*
    addresses equal their symbols, and the B37DATA, OMEGMS and DPPOSMAX
    tables equal the rope.
Writes build/lm-imu-performance-source-map.json.
"""
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import rope_word  # noqa: E402

FILES = ('IMU_PERFORMANCE_TEST_2.agc', 'IMU_PERFORMANCE_TESTS_4.agc')
LST = R / 'build/fixed-data/Luminary099/assembly.lst'
ROPE = R / 'build/fixed-data/Luminary099/MAIN.agc.bin'
SRC = R / 'src/lm_imu_performance.c'
WORD = re.compile(r'^\d{6},\d{6}: (?:(\d\d),)?(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
# K_* whose symbol is not the name after K_.
K_SYMBOL = {'3990DEC': '3990DEC', '77DECML': '77DECML'}
X_ERASABLE = {'ATIGINC': 'E7,1400'}
DATA = ('CADR', 'ADRES', '2CADR', 'GENADR', 'DEC', 'OCT', 'ECADR', '2DEC', 'VN', 'BBCON', '2OCT', 'MM')
OPS = set('''TC TCF CA CAF CAE CS CCS AD ADS SU MASK TS XCH LXCH QXCH DXCH DCA DCS DAS INDEX EXTEND INHINT
 RELINT BZF BZMF INCR AUG DIM MP DV READ WRITE RAND ROR RXOR WAND WOR EXIT DLOAD SLOAD STORE STODL STOVL STCALL
 CALL RTB'''.split())
# Words compiled into the code under another word's label.
INSIDE = {0o2106: 'TC LONGCALL, compiled with AT(02104) and the CCS NDXCTR -0 slot'}


def physical(bank, addr):
    return 2048 + addr - 0o4000 if bank is None else bank * 1024 + addr - 0o2000


def listing():
    lines = LST.read_text(errors='replace').splitlines()
    words, on, symbols = {}, False, {}
    for t in lines:
        if 'Filename:' in t:
            on = any(f in t for f in FILES)
            continue
        if on:
            m = WORD.match(t)
            if m and not m.group(5).lstrip().startswith('#'):
                bank = int(m.group(1), 8) if m.group(1) else None
                addr = int(m.group(2), 8)
                text = m.group(5).split('\t#')[0].split()
                if len(text) > 1 and text[0] not in OPS and text[0] not in DATA:
                    text = text[1:]   # label
                words[physical(bank, addr)] = (int(m.group(3), 8), ' '.join(text))
                if m.group(4):
                    words[physical(bank, addr) + 1] = (int(m.group(4), 8), ' '.join(text) + ' (second word)')
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+((?:\d\d,|E\d,)?\d{4})', LST.read_text(errors='replace')):
        symbols[m.group(2)] = (m.group(1), m.group(3))
    return words, symbols


def sym_physical(value):
    if ',' in value:
        b, a = value.split(',')
        return int(b, 8) * 1024 + int(a, 8) - 0o2000
    return 2048 + int(value, 8) - 0o4000


def main():
    rope = ROPE.read_bytes()
    words, symbols = listing()
    src = SRC.read_text()
    fails = []
    owned = {}
    # Interpretive cases.
    body = src[src.index('static int interpretive('):src.index('/* ---- Basic helpers')]
    cases = re.findall(r'case (0[0-7]+):(.*)', body)
    for loc, code in cases:
        p = 0o37 * 1024 + int(loc, 8) - 0o2000
        lits = [int(x, 8) for x in re.findall(r'\b(0[0-7]{5})\b', code)]
        # DSU STADR: the address test reads the STADR store word as a
        # pushdown marker, then STADR stores through it; one word, two uses.
        lits = [v for i, v in enumerate(lits) if not (i and v == lits[i - 1] and 'stadr' in code)]
        for i, v in enumerate(lits):
            w = rope_word(rope, p + i)
            if w != v:
                fails.append(f'interpretive case {loc}: literal {i} {v:05o} != rope {w:05o}')
            owned[p + i] = f'interpretive case {loc}'
        if not lits:
            if 'exit_' not in code or rope_word(rope, p) != 0o77776:
                fails.append(f'interpretive case {loc}: no words')
            owned[p] = f'interpretive case {loc} (EXIT)'
    # Basic labels.
    basic = src[src.index('static int basic('):src.index('/* Is Z (or LOC at NEWOPS)')]
    for a in re.findall(r'AT\((0[0-7]+)\)', basic):
        a = int(a, 8)
        p = 2048 + a - 0o4000 if a >= 0o4000 else 0o37 * 1024 + a - 0o2000
        if a >= 0o4000:
            owned[p] = 'FINETIME (INTER-BANK_COMMUNICATION)'
            continue
        if p not in words:
            fails.append(f'AT({a:05o}) is not a word of the files')
        owned.setdefault(p, f'basic AT({a:05o})')
    # Words the code reads as operands or data (TC/CADR operands, 2CADR,
    # ADRES, EXTEND's second word, constants).
    for p, (w, text) in sorted(words.items()):
        if p in owned:
            continue
        op = text.split()[0] if text else ''
        a = p - 0o37 * 1024 + 0o2000
        if a in INSIDE:
            owned[p] = INSIDE[a]
            continue
        if op in DATA or text.endswith('(second word)'):
            owned[p] = f'operand/constant ({text})'
            continue
        # The word after an INDEX or EXTEND is covered by the label on the
        # INDEX or EXTEND.
        prev = words.get(p - 1, (0, ''))[1].split()
        if prev and prev[0] in ('INDEX', 'EXTEND'):
            owned[p] = f'with {prev[0]} at {p - 1:o}'
            continue
        fails.append(f'unowned word 37,{(p - 0o37 * 1024 + 0o2000):04o} {w:05o} {text}')
    # Constants.
    for name, val in re.findall(r'#define K_(\w+) (0[0-7]*|0)u', src):
        sym = K_SYMBOL.get(name, name)
        if sym not in symbols:
            fails.append(f'K_{name}: no symbol {sym}')
            continue
        w = rope_word(rope, sym_physical(symbols[sym][1]))
        if w != int(val, 8):
            fails.append(f'K_{name} {int(val, 8):05o} != rope {w:05o} at {symbols[sym][1]}')
    for name, val in re.findall(r'#define X_(\w+) (0[0-7]+)u', src):
        if name in X_ERASABLE:
            if symbols.get(name, ('', ''))[1] != X_ERASABLE[name]:
                fails.append(f'X_{name}: symbol {symbols.get(name)}')
            continue
        if name not in symbols or symbols[name][1] != f'{int(val, 8):04o}':
            fails.append(f'X_{name} {val}: symbol {symbols.get(name)}')
    for table, phys in (('B37DATA', 0o37 * 1024 + 0o3055 - 0o2000), ('OMEGMS', 0o37 * 1024),
                        ('DPPOSMAX', 0o23 * 1024 + 0o2527 - 0o2000)):
        vals = [int(x, 8) for x in re.search(table + r'\[\w*\]=\{(.*?)\}', src, re.S).group(1).replace('u', '').split(',')]
        for i, v in enumerate(vals):
            if rope_word(rope, phys + i) != v:
                fails.append(f'{table}[{i}] {v:05o} != rope {rope_word(rope, phys + i):05o}')
    out = {'files': list(FILES), 'words': len(words),
           'owners': {f'{p // 1024:02o},{p % 1024 + 0o2000:04o}' if p >= 4096 else f'{p - 2048 + 0o4000:04o}': o
                      for p, o in sorted(owned.items())}}
    (R / 'build/lm-imu-performance-source-map.json').write_text(json.dumps(out, indent=1) + '\n')
    if fails:
        print('\n'.join(fails[:60]))
        print(f'FAIL: {len(fails)} lm-imu-performance source problems')
        return 1
    print(f'PASS: {len(words)} words of {" and ".join(FILES)} owned; constants match the rope')
    return 0


if __name__ == '__main__':
    sys.exit(main())
