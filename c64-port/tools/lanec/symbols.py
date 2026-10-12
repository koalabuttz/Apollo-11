#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate include/lanec_symbols.h from the yaYUL listings of the pinned
mission fixtures, for the symbols named in tools/lanec/symbols.txt.

Addresses are assembler facts about the unchanged original source. Before
reading them, the fixture copies of every Lane C source file are checked
against the original Luminary099/Comanche055 files (whitespace-normalized,
comments kept out; the fixtures only repair indentation).

  python3 tools/lanec/symbols.py          # regenerate
  python3 tools/lanec/symbols.py --check  # fail if the header is stale

symbols.txt rows: "M NAME..." (addresses), "M TABLE LABEL N" (TC/TCF
transfer table), "M WORD NAME..." (assembled word at a label, an equate, or NAME+N).
"""
import pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
MISSIONS = (('LM', 'Luminary099', 'lm-powered-fixture'), ('CM', 'Comanche055', 'cm-powered-fixture'))
LANE_FILES = (('EXTENDED_VERBS.agc', 'EXTENDED_VERBS.agc'),
              ('DISPLAY_INTERFACE_ROUTINES.agc', 'DISPLAY_INTERFACE_ROUTINES.agc'),
              ('PINBALL_GAME_BUTTONS_AND_LIGHTS.agc', 'PINBALL_GAME__BUTTONS_AND_LIGHTS.agc'))
OUT = ROOT / 'include/lanec_symbols.h'


def normalized(path):
    """Code fields only. Transcriber-added SBANK= lines bracketed by
    '## [WORKAROUND]' markers (2009, removed upstream in 2017 because yaYUL
    now assigns superbank bits itself) are not original source and are dropped."""
    rows, lines = [], path.read_text(errors='replace').splitlines()
    for i, line in enumerate(lines):
        code = line.split('#', 1)[0]
        if code.split()[:1] == ['SBANK='] and i and lines[i - 1].startswith('## [WORKAROUND]'):
            continue
        if code.strip():
            rows.append(' '.join(code.split()))
    return rows


def check_sources(program, fixture):
    for original, copy in LANE_FILES:
        a = normalized(ROOT.parent / program / original)
        b = normalized(ROOT / 'deps' / fixture / copy)
        # Known transcription deltas, also accepted by the earlier DSKY
        # milestones (e.g. tools/dsky_nv_update_differential.py): the
        # DSPOCTWO/DSPOCTWD spelling of one label, a duplicate PRIOSAVE
        # equate, and CAF/CA spelling of the same instruction on erasable.
        if original == 'PINBALL_GAME_BUTTONS_AND_LIGHTS.agc':
            a = [x.replace('DSPOCTWO', 'DSPOCTWD') for x in a]
        # yaYUL needs an explicit D on the non-octal relative address +8.
        if program == 'Comanche055' and original == 'EXTENDED_VERBS.agc':
            a = [('BZMF +8D' if x == 'BZMF +8' else x) for x in a]
        if original == 'DISPLAY_INTERFACE_ROUTINES.agc':
            a = [x for x in a if x != 'PRIOSAVE EQUALS R1SAVE']
            if program == 'Comanche055':
                a = [x.replace('CAF EBANKSAV', 'CA EBANKSAV') for x in a]
        if a != b:
            sys.exit(f'FAIL: {fixture}/{copy} differs from {program}/{original}')


def symbols(fixture):
    text = (ROOT / 'deps' / fixture / 'assembly.lst').read_text(errors='replace')
    table = text[text.rindex('Symbol Table'):]
    found = {}
    for kind, name, where in re.findall(r'\d+,([A-Z]):\s+(\S+)\s+((?:[0-9A-Z]+,)?[0-7]+)\s', table):
        if ',' in where:
            bank, addr = where.split(',')
            found[name] = (kind, bank, int(addr, 8))
        else:
            found[name] = (kind, None, int(where, 8))
    return found


def transfer_table(fixture, table, label, count):
    """Source TC/TCF targets of a transfer-vector table, by label name."""
    text = (ROOT / 'deps' / fixture / 'assembly.lst').read_text(errors='replace')
    rows, started = [], False
    for line in text.splitlines():
        m = re.match(r'^\d+,\d+: ([0-7]{2}),([0-7]{4})\s+[0-7]{5}\s+(.*)$', line)
        if not m:
            continue
        fields = m.group(3).split('#', 1)[0].split()
        if fields and fields[0] == label:
            started = True
            fields = fields[1:]
        if started:
            if len(fields) == 3:
                fields = fields[1:]  # a label inside the table (VERB69)
            if fields[0] not in ('TC', 'TCF') or len(fields) != 2:
                sys.exit(f'FAIL: {label} table row is not TC/TCF: {line}')
            kind, bank, addr = table[fields[1]]
            if bank is not None and bank != m.group(1):
                sys.exit(f'FAIL: {label} target {fields[1]} outside bank {m.group(1)}')
            rows.append((fields[0], fields[1], addr))
            if len(rows) == count:
                return rows
    sys.exit(f'FAIL: {label} table too short')


def words(fixture):
    """Assembled word at each fixed-memory location, keyed like the symbol
    table: (bank or None for fixed-fixed, address)."""
    text = (ROOT / 'deps' / fixture / 'assembly.lst').read_text(errors='replace')
    found = {}
    for line in text.splitlines():
        m = re.match(r'^\d+,\d+:\s+(?:([0-7]{2}),)?([0-7]{4})\s+([0-7]{5})\s', line)
        if m:
            found.setdefault((m.group(1), int(m.group(2), 8)), int(m.group(3), 8))
    return found


def word(table, assembled, name):
    """Word at NAME or NAME+N; NAME may be an equate (MRKNVBIT = BIT9)."""
    base, _, offset = name.partition('+')
    if name in table:  # labels such as BIT15+6 contain '+' themselves
        base, offset = name, ''
    if base not in table or (offset and not offset.isdigit()):
        return None
    kind, bank, addr = table[base]
    return assembled.get((bank, addr + int(offset or 0)))


def cname(name):
    return re.sub(r'[^A-Za-z0-9]', lambda m: {'/': '_S_', '+': '_P_', '-': '_M_', '.': '_D_'}.get(m.group(0), '_'), name)


def render():
    wanted = [x.split('#', 1)[0].split() for x in (ROOT / 'tools/lanec/symbols.txt').read_text().splitlines()]
    rows = ['/* SPDX-License-Identifier: MIT */',
            '/* Generated by tools/lanec/symbols.py from the pinned mission listings. */',
            '#ifndef LANEC_SYMBOLS_H', '#define LANEC_SYMBOLS_H']
    for mission, program, fixture in MISSIONS:
        check_sources(program, fixture)
        table = symbols(fixture)
        for entry in wanted:
            if not entry or entry[0] != mission:
                continue
            if entry[1] == 'WORD':
                assembled = words(fixture)
                for name in entry[2:]:
                    value = word(table, assembled, name)
                    if value is None:
                        sys.exit(f'FAIL: {mission} constant {name} not in listing')
                    rows.append(f'#define LANEC_{mission}_{cname(name)}_WORD 0{value:o}u /* {name} */')
                continue
            if entry[1] == 'TABLE':
                rows.append(f'/* {mission} {entry[2]}: source TC/TCF transfer targets */')
                rows.append(f'#define LANEC_{mission}_{cname(entry[2])}_TABLE {{ \\')
                for op, target, addr in transfer_table(fixture, table, entry[2], int(entry[3])):
                    rows.append(f'  {{{1 if op == "TCF" else 0}u, 0{addr:o}u}}, /* {op} {target} */ \\')
                rows.append('}')
                continue
            for name in entry[1:]:
                if name not in table:
                    sys.exit(f'FAIL: {mission} symbol {name} not in listing')
                kind, bank, addr = table[name]
                c = f'LANEC_{mission}_{cname(name)}'
                rows.append(f'#define {c} 0{addr:o}u /* {name} {kind} {bank or "fixed-fixed/unswitched"} */')
                if kind == 'F' and bank is not None:
                    rows.append(f'#define {c}_BANK 0{int(bank, 8):o}u')
    rows.append('#endif')
    return '\n'.join(rows) + '\n'


if __name__ == '__main__':
    text = render()
    if '--check' in sys.argv:
        if OUT.read_text() != text:
            sys.exit('FAIL: include/lanec_symbols.h is stale; run tools/lanec/symbols.py')
        print('PASS: lanec symbols current')
    else:
        OUT.write_text(text)
        print('wrote', OUT.relative_to(ROOT))
