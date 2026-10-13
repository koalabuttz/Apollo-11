#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source maps for lane A's oracle milestones (service-transfer,
standard-lead-oracle, interbank-oracle).

For each mission, lists every assembled word of the milestone's AGC file from
the mission's listing (build/fixed-data/<mission>/assembly.lst), with its
source line, the registered native routine that translates it (the last
entry of the milestone's routine tables at or before the word, by physical
address) and whether the original program executed it during the
milestone's yaAGC oracle run (build/<m>-executed.txt, written by
tools/<m>_freeze.py through tools/lane_a_executed.h). Instructions the
oracle never executed are listed under `not_executed`, with the reason the
milestone gives for them. Writes build/<m>-source-map.json.

  python3 tools/lane_a_source_map.py <milestone> [--check]

--check fails when an instruction the oracle did not execute has no reason.
"""
import argparse
import collections
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
MISSIONS = ('Luminary099', 'Comanche055')
LINE = re.compile(r'^\d+,(\d+): (..),?(\d{4})?\s*(\d{4})?\s+(\d{5})(?: (\d{5}))?\s+(\S*)\s+(\S+)\s*([^\t]*)')
ENTRY = re.compile(r'\{\s*(\w+)\(([^()]*)\)\s*,\s*(\w+)\s*\}')
DEFINE = re.compile(r'#define (\w+)\(([\w,]+)\) \(uint16_t\)\((.*)\)\s*$', re.M)
INTERP_ORDERS = {'CALL', 'EXIT', 'STCALL', 'GOTO', 'RVQ', 'RTB', 'BHIZ', 'BPL', 'BZE', 'BOV', 'BON', 'BOFF', 'SET',
                 'CLEAR', 'DLOAD', 'VLOAD', 'STORE', 'STODL', 'STOVL', 'SLOAD', 'SSP', 'ITA', 'GOTO'}
LM, CM = (lambda name: not name.startswith('cm_')), (lambda name: name.startswith('cm_'))

M = {
    'standard-lead-oracle': {
        'file': 'SYSTEM_TEST_STANDARD_LEAD_INS.agc',
        'tables': {'Luminary099': [('src/standard_lead_routines.c', LM)],
                   'Comanche055': [('src/standard_lead_routines.c', CM)]},
        'native': ['src/standard_lead_routines.c', 'src/interp_entry.c', 'src/standard_lead.c', 'src/standard_lead_interp.c',
                   'src/standard_lead_wake.c', 'include/standard_lead_routines.h', 'include/standard_lead.h',
                   'include/standard_lead_wake.h'],
        'reasons': {('Comanche055', n): 'executed in milestone cm-lead-tests: its yaAGC oracle runs the entry words '
                    'COMPVER, GTSCPSS1 and REDO at 33,2000-2002 (384 cases, evidence/cm-lead-tests)'
                    for n in (144, 146, 148)} | {
                    ('Comanche055', 149): "REDO's MM 07 operand: data that NEWMODEX reads through Q, not an "
                    'instruction (milestone cm-lead-tests leaves Q at it)'}},
    'interbank-oracle': {
        'file': 'INTER-BANK_COMMUNICATION.agc',
        'tables': {'Luminary099': [('src/lm_interbank_routines.c', lambda n: True)],
                   'Comanche055': [('src/cm_interbank_routines.c', lambda n: True)]},
        'native': ['src/lm_interbank_routines.c', 'src/lm_interbank_idle.c', 'src/lm_interbank.c', 'src/cm_interbank_routines.c', 'src/cm_interbank_idle.c',
                   'src/cm_interbank_complete.c', 'src/lm_interbank_sites.inc', 'src/cm_interbank_sites.inc',
                   'include/lm_interbank_routines.h', 'include/cm_interbank_routines.h'],
        'reasons': {}},
    'service-transfer': {
        'file': 'SERVICE_ROUTINES.agc',
        'tables': {'Luminary099': [('src/service_transfer.c', LM)],
                   'Comanche055': [('src/service_transfer.c', CM)]},
        'native': ['src/service_transfer.c', 'src/service_routines.c', 'src/lm_service_sites.inc',
                   'src/cm_service_sites.inc', 'include/service_transfer.h', 'include/service_routines.h'],
        'reasons': {}},
}


def entries(path, keep):
    """Physical address -> native function for one routine-table source."""
    text = (R / path).read_text()
    macros = {}
    for name, args, body in DEFINE.findall(text):
        body = re.sub(r'(?<![\w])0([0-7]+)u?', lambda m: f'0o{m.group(1)}', body)
        body = re.sub(r'(\d)u\b', r'\1', body)
        macros[name] = (args.split(','), body)
    out = {}
    for macro, args, name in ENTRY.findall(text):
        if macro not in macros or not keep(name):
            continue
        params, body = macros[macro]
        values = [re.sub(r'(\d)u\b', r'\1', a.strip()) for a in args.split(',')]
        values = [f'0o{v[1:]}' if re.fullmatch(r'0[0-7]+', v) else v for v in values]
        expr = body
        for p, v in zip(params, values):
            expr = re.sub(rf'\b{p}\b', f'({v})', expr)
        out[eval(expr, {'__builtins__': {}})] = f'{path} ({name})'  # noqa: S307 (macro arithmetic only)
    return out


def words(mission, source):
    lines = (R / 'build/fixed-data' / mission / 'assembly.lst').read_text(errors='replace').splitlines()
    a = next(i for i, l in enumerate(lines) if f'${source}' in l)
    b = next(i for i, l in enumerate(lines) if f'(End of include-file {source}' in l)
    for l in lines[a:b]:
        m = LINE.match(l.split('\t')[0])
        if not m or not m.group(5):
            continue
        bank, off = (m.group(2).strip() or None), m.group(3) or m.group(4)
        if not off:
            continue
        off = int(off, 8)
        phys = int(bank, 8) * 1024 + (off & 0o1777) if bank else (3 if off >= 0o6000 else 2) * 1024 + (off & 0o1777)
        yield (int(m.group(1)), bank, off, phys, m.group(7), m.group(8), m.group(9).strip(),
               [m.group(5)] + ([m.group(6)] if m.group(6) else []))


def build(milestone):
    cfg = M[milestone]
    executed = collections.defaultdict(set)
    for line in (R / f'build/{milestone}-executed.txt').read_text().splitlines():
        mission, phys = map(int, line.split())
        executed[MISSIONS[mission]].add(phys)
    doc = {'milestone': milestone, 'oracle': 'yaAGC agc_engine from the original ropes, tools/'
           f'{milestone.replace("-", "_")}_reference.c', 'missions': {}}
    for mission in MISSIONS:
        src = f'{mission}/{cfg["file"]}'
        text = (R.parent / src).read_text().splitlines()
        table = {}
        for path, keep in cfg['tables'][mission]:
            table.update(entries(path, keep))
        out, owner, missing, interpretive = [], None, [], False
        cursor = 0
        for _, bank, off, phys, label, op, operand, ws in words(mission, cfg['file']):
            # The listing's line numbers drift from the repository's file, so
            # walk the source in order to the next line with this label and op.
            for n in range(cursor, len(text)):
                code = text[n].split('#')[0]
                tok = code.split()
                if label and tok[:1] and len(tok) > 1 and tok[1] == op and \
                        (tok[0].startswith(label) or label.startswith(tok[0])):
                    break
                if not label and tok[:1] == [op] and code[:1].isspace():
                    break
            else:
                raise SystemExit(f'{src}: no source line for {label} {op} {operand} after line {cursor}')
            cursor, n = n + 1, n + 1
            # The listing puts an unlabelled word's op in the label column; the
            # source line is the authority.
            if not code[:1].isspace():
                label, op, operand = tok[0], tok[1], ' '.join(tok[2:])
            else:
                label, op, operand = '', tok[0], ' '.join(tok[1:])
            src_line = ' '.join(x for x in (label, op, operand) if x)
            owner = table.get(phys, owner)
            kind = 'constant' if op in ('OCT', 'DEC', 'CADR', 'GENADR', 'ADRES', '2CADR', '2DEC', 'BBCON', 'ECADR',
                                         'FCADR', 'REMADR', '2FCADR', '2BCADR', 'OCTAL') else 'instruction'
            ran = phys in executed[mission]
            if interpretive:
                # Interpretive words are data to the basic program: the oracle
                # records an order when the interpreter's fetch point reads it
                # (LOC); an operand runs with the order before it.
                kind = 'interpretive order' if op in INTERP_ORDERS else 'interpretive operand'
                ran = ran or (kind == 'interpretive operand' and bool(out) and out[-1]['executed_by_oracle'])
                interpretive = op != 'EXIT'
            elif op == 'TC' and operand.split()[:1] == ['INTPRET']:
                interpretive = True
            row = {'line': n, 'address': (f'{bank},' if bank else 'FF,') + f'{off:04o}', 'source': src_line,
                   'words': ws, 'kind': kind, 'native': owner or 'none (before the first registered entry)',
                   'executed_by_oracle': ran}
            if kind != 'constant' and not ran:
                reason = cfg['reasons'].get((mission, n))
                if reason:
                    row['not_executed'] = reason
                else:
                    missing.append(n)
            out.append(row)
        doc['missions'][mission] = {
            'source_path': src, 'source_sha256': hashlib.sha256((R.parent / src).read_bytes()).hexdigest(),
            'rope_sha256': hashlib.sha256((R / 'build/fixed-data' / mission / 'MAIN.agc.bin').read_bytes()).hexdigest(),
            'registered_entries': len(table), 'source_words': sum(len(e['words']) for e in out),
            'source_lines_with_words': len(out),
            'instructions': sum(e['kind'] != 'constant' for e in out),
            'instructions_executed_by_oracle': sum(e['executed_by_oracle'] for e in out if e['kind'] != 'constant'),
            'instructions_not_executed_without_reason': missing, 'line_map': out}
    doc['native'] = {p: hashlib.sha256((R / p).read_bytes()).hexdigest() for p in cfg['native']}
    return doc


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('milestone', choices=sorted(M))
    ap.add_argument('--check', action='store_true')
    args = ap.parse_args()
    doc = build(args.milestone)
    (R / f'build/{args.milestone}-source-map.json').write_text(json.dumps(doc, indent=1) + '\n')
    bad = 0
    for mission, d in doc['missions'].items():
        print(f"{args.milestone} {mission}: {d['source_lines_with_words']} assembled lines, "
              f"{d['instructions_executed_by_oracle']}/{d['instructions']} instructions executed by the oracle; "
              f"not executed without a reason: {d['instructions_not_executed_without_reason']}")
        bad += len(d['instructions_not_executed_without_reason'])
    if args.check and bad:
        raise SystemExit(f'FAIL: {bad} instructions never executed by the oracle and given no reason')
    if not bad:
        print(f'PASS: every instruction of {M[args.milestone]["file"]} executed by the oracle or refused with a reason')


if __name__ == '__main__':
    main()
