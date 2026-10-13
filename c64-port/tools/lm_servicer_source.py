#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source check for lm-servicer.

Reads the host assembly listing of Luminary099
(build/fixed-data/Luminary099/assembly.lst) and its rope, and checks
src/lm_servicer.c against them:
  - every interpretive case IC(b,a) (one per order pair or store word)
    carries the file's words as its octal literals, in order, from its
    address on (a word named twice in a row is a pushdown operand or the
    STORE word STADR consumes);
  - every basic AT(b,a) or FF(a) label is an instruction word of the file,
    and its code holds, after the file's #defines and constant arrays, the
    instruction's erasable address, its branch target, the channel it names
    (chan11, chan12, chan32, chan33), the interrupt state INHINT or RELINT
    leaves, or, for a fixed operand, the rope word there;
  - every word of the file is owned: an AT label, a word of an interpretive
    case, the word after EXTEND, a fixed operand an instruction reads, a
    word of the interpretive fixed-operand tables (FIXED, each checked
    against the rope), or a site operand in a shared table (PHASCHNG,
    INTER-BANK, executive, ALARM) or src/lm_servicer_flag_sites.inc, each
    checked against the rope;
  - the K_* constants equal the rope words at their symbols and the X_*
    addresses equal their symbols.
Writes build/lm-servicer-source-map.json.
"""
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import rope_word  # noqa: E402

FILE = 'SERVICER.agc'
LST = R / 'build/fixed-data/Luminary099/assembly.lst'
ROPE = R / 'build/fixed-data/Luminary099/MAIN.agc.bin'
SRC = R / 'src/lm_servicer.c'
FLAG_SITES = R / 'src/lm_servicer_flag_sites.inc'
SHARED = (R / 'src/lm_phase_sites.inc', R / 'src/lm_interbank_sites.inc', R / 'src/lm_exec_sites.inc',
          R / 'src/lm_alarm_sites.inc', FLAG_SITES)
WORD = re.compile(r'^\d{6},\d{6}: (?:(\d\d),|   )(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
K_SYMBOL = {'ZERO': 'ZERO', 'K_NEG0': 'NEG0'}
DATA = ('CADR', 'ADRES', '2CADR', 'GENADR', 'DEC', 'OCT', 'ECADR', '2DEC', 'VN', 'BBCON', '2OCT', 'FCADR')
OPS = set('''TC TCF CA CAF CAE CS CCS AD ADS SU MASK TS XCH LXCH QXCH DXCH DCA DCS DAS INDEX EXTEND INHINT
 RELINT BZF BZMF INCR AUG DIM MP DV READ WRITE RAND ROR RXOR WAND WOR DTCB NOOP COM ZL'''.split())
QUARTER = {'CCS', 'TS', 'XCH', 'LXCH', 'QXCH', 'ADS', 'INCR', 'AUG', 'DIM', 'DXCH', 'DAS', 'DV'}
DOUBLE = {'DXCH', 'DAS', 'DCA', 'DCS'}
BRANCH = {'TC', 'TCF', 'BZF', 'BZMF'}
CHANNEL = {'RAND', 'WAND', 'READ', 'WRITE', 'ROR', 'WOR', 'RXOR'}
NOOPERAND = {'INHINT', 'RELINT', 'EXTEND', 'NOOP', 'COM', 'ZL', 'DTCB', 'DOUBLE'}
def physical(bank, addr):
    return 2048 + addr - 0o4000 if addr >= 0o4000 else bank * 1024 + addr - 0o2000


# Words whose operand the code names another way: DXCH Z is DTCB (dtcb);
# CCS A's +0 outcome runs DCA APSVEX's word without its EXTEND, as CA
# APSVEX +1 (K_DPSVEX, checked against the rope below).
EXEMPT = {physical(0o33, 0o2413): ('DXCH Z',), physical(0o33, 0o2243): ('DCA APSVEX',)}


def num(x):
    return int(x, 8) if x.startswith('0') else int(x)


def name(p):
    return f'{p:05o}' if p < 4096 else f'{p // 1024:02o},{p % 1024 + 0o2000:04o}'


def listing():
    words, on, symbols = {}, False, {}
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
        if on:
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
    for nm, val in defines.items():
        code = re.sub(rf'\b{nm}\b', val, code)
    vals = set()
    for x, y in re.findall(r'\b(0[0-7]*|[1-9][0-9]*)u?\s*\+\s*(0[0-7]*|[1-9][0-9]*)u\b', code):
        vals.add(num(x) + num(y))
    for x in re.findall(r'\b(0[0-7]*|[1-9][0-9]*)u?\b', code):
        vals.add(int(x, 8) if x.startswith('0') else int(x))
    for x in re.findall(r'L\d\d_(0[0-7]+)|LFF_(0[0-7]+)', code):
        vals.add(int(x[0] or x[1], 8))
    return vals


def main():
    rope = ROPE.read_bytes()
    words, symbols = listing()
    src = SRC.read_text()
    defines = {n: v for n, v in re.findall(r'#define (\w+) (0[0-7]*|[1-9])u\b', src)}
    # Constant arrays read as fixed operands: their first word.
    arrays = {n: re.findall(r'(0[0-7]*|\d+)u', v) for n, v in
              re.findall(r'static const agc_word (?:__attribute__\(\(unused\)\) )?(\w+)\[2\][^=]*=\{([^}]*)\}', src)}
    for n, v in arrays.items():
        defines[n] = ' '.join(v)
    # The interpretive fixed-operand tables a basic DCA indexes (ALTCRIT).
    for n, v in re.findall(r'static const agc_word (?:__attribute__\(\(unused\)\) )?(FX\w+)\[\w+\]=\{(.*?)\};', src, re.S):
        defines[n] = ' '.join(re.findall(r'(0[0-7]*|\d+)u?', v)[:2])
    # Helpers that name their operand: intpret is TC INTPRET, tcq is TC Q,
    # tca TC A, tclradret TC LRADRET.
    defines.update({'intpret': defines['X_INTPRET'], 'tcq': '2', 'tca': '0', 'tclradret': defines['LRADRET']})
    fails, owned = [], {}
    # Interpretive cases.
    body = src[src.index('static int interpretive('):src.index('/* ---- Basic helpers')]
    pieces = re.split(r'\bIC\((\d\d),(0[0-7]+)\);', body)
    for i in range(1, len(pieces), 3):
        bank, loc, code = int(pieces[i], 8), int(pieces[i + 1], 8), pieces[i + 2]
        p = physical(bank, loc)
        lits = [int(x, 8) for x in re.findall(r'\b(0[0-7]{5})\b', code)]
        lits = [v for j, v in enumerate(lits) if not (j and v == lits[j - 1])]
        for j, v in enumerate(lits):
            w = rope_word(rope, p + j)
            if w != v:
                fails.append(f'interpretive case {name(p)}: literal {j} {v:05o} != rope {w:05o}')
            owned[p + j] = f'interpretive case {name(p)}'
        if not lits:
            if 'exit_' not in code or rope_word(rope, p) != 0o77776:
                fails.append(f'interpretive case {name(p)}: no words')
            owned[p] = f'interpretive case {name(p)} (EXIT)'
    # Basic labels and their operands.
    basic = src[src.index('static int basic('):src.index('/* The file\'s words')]
    # The C64 group lines and segment-end labels name no instruction.
    basic = re.sub(r'^#.*\n|^ L\d\d_0[0-7]+: __attribute__.*\n', '', basic, flags=re.M)
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
    # The interpretive fixed operands.
    for b, a, n, nm in re.findall(r'\{PHX\(0(\d\d),(0[0-7]+)\),(0[0-7]*),(\w+)\}', src):
        vals = [int(x, 8) if x != '0' else 0 for x in re.findall(r'(0[0-7]*|\d+)u?', re.search(
            rf'static const agc_word (?:__attribute__\(\(unused\)\) )?{nm}\[\w+\]=\{{(.*?)\}};', src, re.S).group(1))]
        if len(vals) != int(n, 8):
            fails.append(f'{nm} has {len(vals)} words, not {int(n, 8)}')
        for j, v in enumerate(vals):
            q = physical(int(b, 8), int(a, 8) + j)
            if rope_word(rope, q) != v:
                fails.append(f'{nm}[{j}] {v:05o} != rope {rope_word(rope, q):05o}')
            owned.setdefault(q, f'fixed operand table {nm}')
    # Site tables.
    sites = {}
    for path in SHARED:
        for p, v in re.findall(r'\{(\d+)u,\s*(0[0-7]+)u?\}', path.read_text()):
            sites[num(p)] = (num(v), path.name)
    flags = [(num(p), num(v)) for p, v in re.findall(r'\{(\d+)u,\s*(0[0-7]*)u?\}', FLAG_SITES.read_text())]
    n_flags = int(re.search(r'#define LMSV_FLAG_SITES (\d+)u', (R / 'include/lm_servicer.h').read_text()).group(1))
    if n_flags != len(flags) or [p for p, _ in flags] != sorted(p for p, _ in flags):
        fails.append(f'LMSV_FLAG_SITES {n_flags} or order differs from lm_servicer_flag_sites.inc ({len(flags)})')
    for p, v in flags:
        prev = words.get(p - 1, (0, ''))[1]
        if not re.match(r'TC (UPFLAG|DOWNFLAG)$', prev):
            fails.append(f'flag site {name(p)} does not follow a TC UPFLAG/DOWNFLAG ({prev})')
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
    for nm, val, sym in re.findall(r'#define K_(\w+) (0[0-7]*)u\s*/\* (.*?) \*/', src):
        if nm == 'DPSVEX':
            continue
        if sym not in symbols:
            fails.append(f'K_{nm}: no symbol {sym}')
            continue
        w = rope_word(rope, sym_physical(symbols[sym][1]))
        if w != int(val, 8):
            fails.append(f'K_{nm} {int(val, 8):05o} != rope {w:05o} at {symbols[sym][1]}')
    if rope_word(rope, physical(0o33, 0o2001)) != int(defines['K_DPSVEX'], 8):
        fails.append('K_DPSVEX is not APSVEX +1')
    for nm, val, sym in re.findall(r'#define X_(\w+) (0[0-7]+)u\s*/\* (.*?) \*/', src):
        sym, off = (sym.split('+')[0].strip(), int(sym.split('+')[1])) if '+' in sym else (sym, 0)
        got = symbols.get(sym, ('', '-'))[1].split(',')[-1]
        if got == '-' or int(got, 8) + off != int(val, 8):
            fails.append(f'X_{nm} {val}: symbol {sym} {symbols.get(sym)}')
    out = {'file': FILE, 'words': len(words), 'owners': {name(p): o for p, o in sorted(owned.items())}}
    (R / 'build/lm-servicer-source-map.json').write_text(json.dumps(out, indent=1) + '\n')
    if fails:
        print('\n'.join(fails[:80]))
        print(f'FAIL: {len(fails)} lm-servicer source problems')
        return 1
    print(f'PASS: {len(words)} words of {FILE} owned; operands, sites and constants match the rope')
    return 0


if __name__ == '__main__':
    sys.exit(main())
