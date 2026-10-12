#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source check for lm-burn-baby.

Reads the host assembly listing of Luminary099
(build/fixed-data/Luminary099/assembly.lst) and its rope, and checks
src/lm_burn_baby.c against them:
  - every interpretive case (one per order-pair or store word) carries the
    file's words as its octal literals, in order, from its LOC on (an order
    pair named twice in a row is the PUSHUP word of two pushdown operands);
  - every basic AT(b,a) or FF(a) label is an instruction word of the file,
    and its code holds, after the file's #defines and constant arrays, the
    instruction's erasable address, its branch target, the channel it names
    (chan11, chan14, chan30), the interrupt state INHINT or RELINT leaves,
    or, for a fixed operand, the rope word there;
  - every word of the file is owned: an AT or FF label, a word of an
    interpretive case, the word after EXTEND, a fixed operand an
    instruction reads, a word of TABLE (the ignition tables, checked against
    the rope), a site operand in a shared table (PHASCHNG, INTER-BANK,
    executive, ALARM) or in src/lm_burn_baby_sites.inc, each checked against
    the rope, or a WHICH producer word (P40ADRES .. P42ADRES) naming one of
    the six tables which() accepts;
  - lm_burn_baby_sites.inc holds exactly the CADR after every TC KILLTASK
    in Luminary099;
  - the K_* constants equal the rope words at their symbols and the X_*
    addresses equal their symbols.
Writes build/lm-burn-baby-source-map.json.
"""
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import rope_word  # noqa: E402

FILE = 'BURN_BABY_BURN--MASTER_IGNITION_ROUTINE.agc'
LST = R / 'build/fixed-data/Luminary099/assembly.lst'
ROPE = R / 'build/fixed-data/Luminary099/MAIN.agc.bin'
SRC = R / 'src/lm_burn_baby.c'
OWN_SITES = R / 'src/lm_burn_baby_sites.inc'
FLAG_SITES = R / 'src/lm_burn_baby_flag_sites.inc'
SHARED = (R / 'src/lm_phase_sites.inc', R / 'src/lm_interbank_sites.inc', R / 'src/lm_exec_sites.inc',
          R / 'src/lm_alarm_sites.inc', OWN_SITES, FLAG_SITES)
WORD = re.compile(r'^\d{6},\d{6}: (?:(\d\d),|   )(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
K_SYMBOL = {'ZERO': 'ZERO', 'K_NEG0': 'NEG0'}
DATA = ('CADR', 'ADRES', '2CADR', 'GENADR', 'DEC', 'OCT', 'ECADR', '2DEC', 'VN', 'BBCON', '2OCT', 'FCADR')
OPS = set('''TC TCF CA CAF CAE CS CCS AD ADS SU MASK TS XCH LXCH QXCH DXCH DCA DCS DAS INDEX EXTEND INHINT
 RELINT BZF BZMF INCR AUG DIM MP DV READ WRITE RAND ROR RXOR WAND WOR DTCB NOOP COM ZL'''.split())
QUARTER = {'CCS', 'TS', 'XCH', 'LXCH', 'QXCH', 'ADS', 'INCR', 'AUG', 'DIM', 'DXCH', 'DAS', 'DV'}
DOUBLE = {'DXCH', 'DAS', 'DCA', 'DCS'}
BRANCH = {'TC', 'TCF', 'BZF', 'BZMF'}
CHANNEL = {'RAND', 'WAND', 'READ', 'WRITE', 'ROR', 'WOR', 'RXOR'}
NOOPERAND = {'INHINT', 'RELINT', 'EXTEND', 'NOOP', 'COM', 'ZL', 'DTCB'}
TABLE_SPAN = (0o36, 0o2022, 0o2125)
PRODUCERS = (0o3133, 0o3135)          # P40ADRES, P41ADRES, P42ADRES (bank 36)
# Operands the code reads through a computed address: KILLTASK's CA 0
# under INDEX Q reads the caller's CADR (lm_burn_baby_sites).
EXEMPT = {0o6033: ('CA 0',)}


def physical(bank, addr):
    return 2048 + addr - 0o4000 if addr >= 0o4000 else bank * 1024 + addr - 0o2000


def name(p):
    return f'{p:05o}' if p < 4096 else f'{p // 1024:02o},{p % 1024 + 0o2000:04o}'


def listing():
    words, on, symbols, killtask = {}, False, {}, []
    text_all = LST.read_text(errors='replace')
    lines = text_all.splitlines()
    for i, t in enumerate(lines):
        if 'Filename:' in t:
            on = FILE in t
            continue
        m = WORD.match(t)
        if not m or m.group(5).lstrip().startswith('#'):
            continue
        bank = int(m.group(1), 8) if m.group(1) else 0
        addr = int(m.group(2), 8)
        text = m.group(5).split('\t#')[0].split()
        if len(text) > 1 and (text[1] in OPS or text[1] in DATA) and text[0] not in OPS:
            text = text[1:]
        if text[:2] == ['TC', 'KILLTASK']:
            n = WORD.match(lines[i + 1])
            killtask.append((physical(int(n.group(1), 8), int(n.group(2), 8)), int(n.group(3), 8)))
        if on:
            p = physical(bank, addr)
            words[p] = (int(m.group(3), 8), ' '.join(text), bank, addr)
            if m.group(4):
                words[p + 1] = (int(m.group(4), 8), ' '.join(text) + ' (second word)', bank, addr + 1)
    for m in re.finditer(r'\d+,([EF]):\s+(\S+)\s+((?:\d\d,|E\d,)?\d{4})', text_all):
        symbols[m.group(2)] = (m.group(1), m.group(3))
    return words, symbols, killtask


def sym_physical(value):
    if ',' in value:
        b, a = value.split(',')
        return int(b, 8) * 1024 + int(a, 8) - 0o2000
    return 2048 + int(value, 8) - 0o4000


def literals(code, defines):
    for nm, val in defines.items():
        code = re.sub(rf'\b{nm}\b', val, code)
    vals = set()
    for x in re.findall(r'\b(0[0-7]*|[1-9][0-9]*)u?\b', code):
        vals.add(int(x, 8) if x.startswith('0') else int(x))
    for x in re.findall(r'L\d\d_(0[0-7]+)|LFF_(0[0-7]+)', code):
        vals.add(int(x[0] or x[1], 8))
    return vals


def main():
    rope = ROPE.read_bytes()
    words, symbols, killtask = listing()
    src = SRC.read_text()
    defines = {n: v for n, v in re.findall(r'#define (\w+) (0[0-7]*|[1-9])u\b', src)}
    # Constant arrays read as fixed operands: their first word.
    arrays = {n: re.findall(r'(0[0-7]*|\d+)u', v) for n, v in
              re.findall(r'static const agc_word (\w+)\[2\][^=]*=\{([^}]*)\}', src)}
    for n, v in arrays.items():
        defines[n] = ' '.join(v)
    # Helpers that name their operand: WHICHW reads WHICH, intpret is TC
    # INTPRET, tcq is TC Q.
    defines.update({'WHICHW': defines['WHICH'], 'intpret': defines['X_INTPRET'], 'tcq': '2'})
    fails, owned = [], {}
    # Interpretive cases (bank 36).
    body = src[src.index('static int interpretive('):src.index('/* ---- Basic helpers')]
    pieces = re.split(r'\bcase (0[0-7]+):', body)
    for loc, code in zip(pieces[1::2], pieces[2::2]):
        loc = int(loc, 8)
        p = physical(0o36, loc)
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
    parts = re.split(r'\b(?:AT\((\d\d),(0[0-7]+)\)|FF\((0[0-7]+)\));', basic)
    labelled = {physical(int(parts[i], 8), int(parts[i + 1], 8)) if parts[i] else physical(0, int(parts[i + 2], 8)): parts[i + 3]
                for i in range(1, len(parts), 4)}
    for i in range(1, len(parts), 4):
        b, a, f, code = parts[i:i + 4]
        bank, a = (int(b, 8), int(a, 8)) if b else (0, int(f, 8))
        p = physical(bank, a)
        if p not in words:
            fails.append(f'label {bank:02o},{a:04o} is not a word of the file')
            continue
        owned.setdefault(p, f'basic {name(p)}')
        w, text, _, _ = words[p]
        op = text.split()[0]
        if op in ('INHINT', 'RELINT'):
            if f'interrupt_enabled={1 if op == "RELINT" else 0};' not in code:
                fails.append(f'{name(p)} {op}: interrupt state not set')
            continue
        if op in NOOPERAND:
            continue
        if text in EXEMPT.get(p, ()):
            continue
        if op in CHANNEL:
            if f'chan{w & 0o777:o}' not in code and not (w & 0o777 == 1 and 'W[1]' in code):
                fails.append(f'{name(p)} {text}: channel {w & 0o777:o} not in its code')
            continue
        k = w & 0o1777 if op in QUARTER else w & 0o7777
        if op in DOUBLE:
            k -= 1
        lits = literals(code, defines)
        if op in ('CA', 'CAF') and k == 5:
            need, what = a + 1, 'Z (the next address)'
        elif op in BRANCH:
            need, what = k, 'branch target'
        elif k >= 0o2000:
            q = physical(bank if k < 0o4000 else 0, k)
            need, what = rope_word(rope, q), 'fixed operand word'
            owned.setdefault(q, f'operand of {name(p)}')
            if op in DOUBLE:
                owned.setdefault(q + 1, f'operand of {name(p)}')
                if rope_word(rope, q + 1) not in lits:
                    fails.append(f'{name(p)} {text}: second operand word not in its code')
        else:
            need, what = k, 'erasable address'
        if need not in lits:
            fails.append(f'{name(p)} {text}: {what} {need:05o} not in its code')
        # An INDEX whose indexed word has no label of its own runs both.
        nxt = words.get(p + 1)
        if op == 'INDEX' and nxt and p + 1 not in labelled:
            nw, ntext = nxt[0], nxt[1]
            nop = ntext.split()[0]
            nk = nw & 0o1777 if nop in QUARTER else nw & 0o7777
            if nop in DOUBLE:
                nk -= 1
            if (nk >= 0o2000 and nop not in BRANCH) or nk not in lits:
                fails.append(f'{name(p + 1)} {ntext} (indexed): operand {nk:05o} not in the INDEX code')
            owned[p + 1] = f'basic {name(p)} (indexed)'
    # TABLE.
    table = [int(x, 8) for x in re.search(r'TABLE\[0104\][^=]*=\{(.*?)\};', src, re.S).group(1).replace('u', '').split(',') if x.strip()]
    tb, t0, t1 = TABLE_SPAN
    if len(table) != t1 - t0 + 1:
        fails.append(f'TABLE has {len(table)} words, not {t1 - t0 + 1}')
    for i, v in enumerate(table):
        q = physical(tb, t0 + i)
        if rope_word(rope, q) != v:
            fails.append(f'TABLE[{i}] {v:05o} != rope {rope_word(rope, q):05o}')
        owned.setdefault(q, 'TABLE')
    # The WHICH producers' words.
    accepted = {int(x, 8) for x in re.findall(r'v!=(0[0-7]+)u', src[src.index('static NOINL int which('):])[:6]}
    for a in range(PRODUCERS[0], PRODUCERS[1] + 1):
        q = physical(0o36, a)
        if rope_word(rope, q) not in accepted:
            fails.append(f'producer word 36,{a:04o} {rope_word(rope, q):05o} not a table which() accepts')
        owned.setdefault(q, 'WHICH producer (P40-P47)')
    # Site tables.
    sites = {}
    for path in SHARED:
        for p, v in re.findall(r'\{(\d+)u,\s*(0[0-7]+)u?\}', path.read_text()):
            sites[int(p)] = (int(v, 8), path.name)
    mine = sorted((int(p), int(v, 8)) for p, v in re.findall(r'\{(\d+)u,\s*(0[0-7]+)u?\}', OWN_SITES.read_text()))
    flags = re.findall(r'\{(\d+)u,\s*(0[0-7]+)u?\}', FLAG_SITES.read_text())
    n_flags = int(re.search(r'#define LMBB_FLAG_SITES (\d+)u', (R / 'include/lm_burn_baby.h').read_text()).group(1))
    if n_flags != len(flags) or [int(p) for p, _ in flags] != sorted(int(p) for p, _ in flags):
        fails.append(f'LMBB_FLAG_SITES {n_flags} or order differs from lm_burn_baby_flag_sites.inc ({len(flags)})')
    for p, v in flags:
        prev = words.get(int(p) - 1, (0, ''))[1]
        if not re.match(r'TC (UPFLAG|DOWNFLAG|INVFLAG)$', prev):
            fails.append(f'flag site {name(int(p))} does not follow a TC UPFLAG/DOWNFLAG/INVFLAG ({prev})')
    if mine != sorted(killtask):
        fails.append(f'lm_burn_baby_sites.inc differs from the TC KILLTASK CADRs {sorted(killtask)}')
    n_sites = int(re.search(r'#define LMBB_SITES (\d+)u', (R / 'include/lm_burn_baby.h').read_text()).group(1))
    if n_sites != len(mine):
        fails.append(f'LMBB_SITES {n_sites} != {len(mine)}')
    for p, v in mine:
        if rope_word(rope, p) != v:
            fails.append(f'KILLTASK site {name(p)}: {v:05o} != rope {rope_word(rope, p):05o}')
    for p, (w, text, bank, addr) in sorted(words.items()):
        if p in owned:
            continue
        op = text.split()[0] if text else ''
        prev = words.get(p - 1, (0, '', 0, 0))[1].split()
        if prev and prev[0] == 'EXTEND':
            owned[p] = 'with EXTEND'
        elif op == 'INDEX' and p + 1 in labelled:
            # Run with the word it indexes, whose code reads the index.
            if (w & 0o7777) not in literals(labelled[p + 1], defines):
                fails.append(f'{name(p)} {text}: index register not read by {name(p + 1)}')
            owned[p] = f'basic {name(p + 1)} (its INDEX)'
        elif p in sites and op in DATA:
            if sites[p][0] != rope_word(rope, p):
                fails.append(f'site {name(p)}: {sites[p][0]:05o} != rope {rope_word(rope, p):05o}')
            owned[p] = f'src/{sites[p][1]}'
        else:
            fails.append(f'unowned word {name(p)} {w:05o} {text}')
    for nm, val in re.findall(r'#define K_(\w+) (0[0-7]*|0)u', src):
        sym = K_SYMBOL.get(nm, nm)
        if sym not in symbols:
            fails.append(f'K_{nm}: no symbol {sym}')
            continue
        w = rope_word(rope, sym_physical(symbols[sym][1]))
        if w != int(val, 8):
            fails.append(f'K_{nm} {int(val, 8):05o} != rope {w:05o} at {symbols[sym][1]}')
    for nm, val in re.findall(r'#define X_(\w+) (0[0-7]+)u', src):
        if nm not in symbols or symbols[nm][1] != f'{int(val, 8):04o}':
            fails.append(f'X_{nm} {val}: symbol {symbols.get(nm)}')
    out = {'file': FILE, 'words': len(words), 'owners': {name(p): o for p, o in sorted(owned.items())}}
    (R / 'build/lm-burn-baby-source-map.json').write_text(json.dumps(out, indent=1) + '\n')
    if fails:
        print('\n'.join(fails[:80]))
        print(f'FAIL: {len(fails)} lm-burn-baby source problems')
        return 1
    print(f'PASS: {len(words)} words of {FILE} owned; operands, sites and constants match the rope')
    return 0


if __name__ == '__main__':
    sys.exit(main())
