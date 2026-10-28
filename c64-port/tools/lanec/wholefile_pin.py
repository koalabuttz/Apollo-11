#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-file source-to-native map for Luminary099 and Comanche055
PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, the evidence behind their full-file
certificates.

  python3 tools/lanec/wholefile_pin.py OUTDIR

It reads every assembled word of the file from the pinned listings and runs
the Lane C Pinball suites (pin-keys, pin-gaps, the pw-* suites of lanec-pw whose
module some Pinball routine still tries, and the pin-head-* suites)
through the oracle with LANEC_TRACE. Each word gets exactly one owner:
  - an instruction word started by the unchanged original program in Virtual
    AGC under a suite's cases belongs to that suite's milestone (pin-keys
    first, then pin-gaps, then the pw-* suites in name order), and names the native module
    that carries it (the module's run wrapper for a pw-* suite);
  - a data word right after a call (its argument: CADR, ADRES, ...) belongs
    to the call's owner;
  - any other data word is a constant: a symbols.txt WORD row or a literal
    that a native module reads, or a label whose readers are listed, or a
    word nothing reads;
  - an instruction word that other code reads as a constant (CONSTANTS) or
    that no path reaches (UNREACHABLE) is listed with the reason, checked
    against the listing.
The suites' complete results match the native code (tools/lanec/diff.py), so
every owned word is reproduced by its milestone. Instruction rows that are
transfer-ABI entries (src/lanec_pin_routines.inc) name their routine. Writes
OUTDIR/lanec-pin-{lm,cm}-source-map.json.
"""
import hashlib, json, os, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/lanec'))
import symbols  # noqa: E402
import wholefile  # noqa: E402

wholefile.FILE = FILE = 'PINBALL_GAME_BUTTONS_AND_LIGHTS.agc'
DATA = wholefile.DATA | {'BBCON', '2FCADR', 'FCADR', 'BBCON*', 'DEC*', '2DEC*'}


def flight_runs():
    """The pw-* runs some Pinball routine tries (src/lanec_pin_routines.inc's
    try-lists and the keypress modules): the others no longer run in the
    program (routines.py pin `dropped` and heads, docs/LANEC-PIN-HEADS.md)."""
    text = '\n'.join(line for line in (ROOT / 'src/lanec_pin_routines.inc').read_text().splitlines()
                     if line.startswith('static const dispatch try'))
    text += '\n'.join(p.read_text() for p in sorted((ROOT / 'src').glob('lanec_pin_keys*.c')))
    return set(re.findall(r'lanec_pw_[a-z0-9_]+_run', text))


def suites():
    """(suite, milestone, native) in ownership priority order."""
    rows = [('pin-keys', 'lanec-pin-keys', 'src/lanec_pin_keys.c'), ('pin-gaps', 'lanec-pin-gaps', 'src/lanec_pin_gaps.c')]
    runs = flight_runs()
    for path in sorted((ROOT / 'tests').glob('lanec_pw_*_cases.c')):
        text = path.read_text()
        name = re.search(r'"(pw-[a-z0-9-]+)"', text).group(1)
        mod = re.search(r'\((src/[a-z0-9_]+\.c)', text)
        run = re.search(r'lanec_pw_[a-z0-9_]+_run', text)
        if run.group(0) in runs:
            rows.append((name, 'lanec-pw', f'{mod.group(1) if mod else "src/lanec_pw_runs.c"} via {run.group(0)}'))
    # the head families: Pinball's words from a point to its next call, where a
    # dropped module ran on through other routines
    for path in sorted((ROOT / 'src').glob('lanec_pin_head_*.c')):
        h = path.stem[len('lanec_pin_head_'):]
        rows.append((f'pin-head-{h}', 'lanec-pin-heads', f'src/{path.name} via lanec_pin_head_{h}_run'))
    # the key labels' cases run the earlier key modules, which own those words first
    return rows + [('pin-key-labels', 'lanec-pin-keys', 'src/lanec_pin_keys.c')]


SUITES = suites()
_DATA = 'never executed: the word before it ends in a transfer or is data, and no code transfers to it'
# (label, listing code fields after the label): (kind, reason). The label is
# the last one at or above the word; both missions unless noted.
CLASSIFIED = {
    # Data entry sets DSPCOUNT from VD1 (19) down to R3D5 (0), so the range
    # below is really 0-14 and 16-19 (#862 item 4). The text is part of the
    # source maps evidence/lanec-pin-cert/SHA256.json pins under both accepted
    # certificates, so it stays until those maps are next regenerated.
    ('INRELTAB', 'TC CCSHOLE'): ('constant instruction',
        'INRELTAB +15: GETINREL and CLEAR read it as data (INDEX DSPCOUNT / A, CAF INRELTAB) only for display '
        'count 15, which no DSPCOUNT reaches (0-14, 16-22); ' + _DATA),
    ('MIXAD', 'TC MIXTEMP'): ('constant instruction',
        'read by AD MIXAD (MIXNOUN builds TC MIXTEMP +n); ' + _DATA),
    ('DISPLACE', 'TC GOQ'): ('constant instruction', 'read by INDEX DECOUNT, CAF DISPLACE (SFRET); ' + _DATA),
    ('DISPLACE', 'TC RIGHT5'): ('constant instruction', 'DISPLACE +1, read by INDEX DECOUNT, CAF DISPLACE; ' + _DATA),
    ('DISPLACE', 'TC LEFT5'): ('constant instruction', 'DISPLACE +2, read by INDEX DECOUNT, CAF DISPLACE; ' + _DATA),
    ('NVSBENDL', 'TC NVSUBEND'): ('constant instruction',
        'read by CS NVSBENDL (DSPALARM) and CAF NVSBENDL; the words before it are data (MONADR); ' + _DATA),
    ('ENDINST', 'TC ENDOFJOB'): ('constant instruction',
        'read by CAF/CA/CS/AD ENDINST in Pinball and by XCHTOEND (DISPLAY_INTERFACE_ROUTINES), which copy it '
        'into a return cell; the word before it is TC JOBSLEEP; ' + _DATA),
    ('TCNOVAC', 'TC NOVAC'): ('constant instruction', 'read by CAF TCNOVAC (copied into a cell); ' + _DATA),
    ('TCWAIT', 'TC WAITLIST'): ('constant instruction', 'read by CAF TCWAIT (VBRQWAIT); ' + _DATA),
    ('TCTSKOVR', 'TC TASKOVER'): ('constant instruction', 'read by CA TCTSKOVR (KILLDEAD in BURN_BABY_BURN); ' + _DATA),
    ('TCFINDVC', 'TC FINDVAC'): ('constant instruction', 'read by CAF TCFINDVC (SETVAC); ' + _DATA),
    ('LEGALTST', 'TC CCSHOLE'): ('unreachable instruction',
        'LEGALTST: AD NEG2, CCS A takes its +0 branch to this TC only for a sum of +0, and the one\'s-complement '
        'adder never gives +0 when one operand (NEG2) is nonzero (INREL 2 gives -0)'),
    ('NEGOPT', 'TC CCSHOLE'): ('unreachable instruction',
        'NEGOPT (Comanche055): AD 20BIAS, CCS A takes its +0 branch to this TC only for a sum of +0, which the '
        'one\'s-complement adder never gives with a nonzero operand (20BIAS)'),
    ('ALL3DEC', 'TC FORCEV25'): ('unreachable instruction',
        'ALL3DEC: CS OCT34BAR, MASK DECBRNCH leaves at most 34 octal, and AD OCT34BAR (-34) gives a negative '
        'sum or -0, so CCS A never takes its positive branch to this first TC FORCEV25 (the second, after '
        'OCT34BAR, is executed)'),
}


def traces(mission):
    executed = {}
    env = dict(os.environ)
    # Keyed by the oracle build, which holds the suites' cases: a stale trace
    # from an older case set would credit words to the wrong module.
    cache = ROOT / 'build/lanec-pinmap' / hashlib.sha256((ROOT / 'build/lanec-oracle').read_bytes()).hexdigest()[:16]
    cache.mkdir(parents=True, exist_ok=True)
    for suite, _, _ in SUITES:
        out = cache / f'{suite}.trace'
        if not out.exists():
            env['LANEC_TRACE'] = str(out)
            subprocess.run(['build/lanec-oracle', suite, 'deps/lm-powered-fixture/MAIN.agc.bin',
                            'deps/cm-powered-fixture/MAIN.agc.bin'], cwd=ROOT, env=env, check=True,
                           stdout=subprocess.DEVNULL)
        for line in out.read_text().split('\n'):
            kind, _, where = line.partition(' ')
            if kind != mission:
                continue
            bank, addr = where.split(',')
            executed.setdefault((None if bank == '0' else f'{int(bank, 8):02o}', int(addr, 8)), suite)
    return executed


def literal_in(label, words):
    """A native module that defines LABEL as the same literal word."""
    for _, _, mod in SUITES:
        path = ROOT / mod.split(' ')[0]
        m = re.search(rf'#define \w*{re.escape(symbols.cname(label))} (0[0-7]*)u\b', path.read_text())
        if m and len(words) == 1 and int(m.group(1), 8) == int(words[0], 8):
            return mod.split(' ')[0]
    return None


def build(mission, survey=False):
    table = symbols.symbols(wholefile.FIXTURE[mission])
    rows = wholefile.listing_words(mission)
    wholefile.original_lines(mission, rows)
    executed = traces(mission)
    milestone = {s: (ms, mod) for s, ms, mod in SUITES}
    native = '\n'.join((ROOT / p).read_text() for p in sorted({m.split(' ')[0] for _, _, m in SUITES}))
    src = (ROOT.parent / wholefile.ORIGINAL[mission] / FILE).read_text(errors='replace').splitlines()
    read = wholefile.read_addresses(mission, table, native)
    abi = wholefile.abi_entries(mission, 'pin')
    owners, line_map, label, call_owner, open_rows = {}, [], None, None, []
    for i, r in enumerate(rows):
        key = (r['bank'], r['addr'])
        fields = r['text'].split('#', 1)[0].split()
        labelled = not src[r['line'] - 1][:1].isspace()
        if labelled:
            label = fields[0]
        op = next((f for f in fields if f in DATA), None) if fields else None
        if key in executed and not op:
            ms, mod = milestone[executed[key]]
            entry = {'kind': 'instruction', 'owner': ms, 'native': mod, 'evidence': f'executed under {executed[key]}'}
            at = wholefile.physical(r['bank'], r['addr'])
            if at in abi:
                entry['abi_routine'] = f'lanec_pin_{mission.lower()}_routines[{at}] -> {abi[at]}'
            call_owner = (ms, mod)
        elif not op and (label, ' '.join(fields[1:] if labelled else fields)) in CLASSIFIED:
            kind, why = CLASSIFIED[(label, ' '.join(fields[1:] if labelled else fields))]
            entry = {'kind': kind, 'owner': 'constants' if kind == 'constant instruction' else 'lanec-pin-gaps',
                     'native': 'none (' + ('read as data' if kind == 'constant instruction' else 'unreachable') + ')',
                     'evidence': why}
            call_owner = None
        elif op and call_owner and not labelled:
            entry = {'kind': 'inline data', 'owner': call_owner[0], 'native': f'{call_owner[1]} (call argument)'}
        elif op:
            name = label if labelled else f'{label} (following word)'
            literal = labelled and literal_in(label, r['words'])
            if key in read:
                how = f'compiled constant {read[key]} (symbols.txt WORD row)'
            else:
                users = wholefile.readers(mission, label) if labelled else []
                how = (f'compiled literal in {literal}' if literal else
                       'preserved in the rope; read only by ' + ', '.join(users) if users else
                       f'preserved in the rope; part of the {label} table' if not labelled else
                       'preserved in the rope; no source line of the program refers to it')
            entry = {'kind': 'constant', 'owner': 'constants', 'native': how, 'constant': name}
            call_owner = None
        else:
            open_rows.append(f'{mission} {r["bank"] or "FF"},{r["addr"]:04o} {label}: {" ".join(fields)}')
            call_owner = None
            continue
        owners[entry['owner']] = owners.get(entry['owner'], 0) + len(r['words'])
        line_map.append(dict(line=r['line'], address=f'{r["bank"] or "FF"},{r["addr"]:04o}',
                             source=' '.join(fields), words=r['words'], **entry))
    if open_rows:
        print('\n'.join(open_rows))
        if not survey:
            sys.exit(f'FAIL: {mission} {len(open_rows)} instruction words never executed and not classified')
    total = sum(len(r['words']) for r in rows)
    count = lambda kind: sum(len(e['words']) for e in line_map if e['kind'] == kind)  # noqa: E731
    return {
        'source_path': f'{wholefile.ORIGINAL[mission]}/{FILE}',
        'source_sha256': wholefile.sha(ROOT.parent / wholefile.ORIGINAL[mission] / FILE),
        'rope_sha256': wholefile.ROPE[mission],
        'native': {p: wholefile.sha(ROOT / p) for p in sorted({m.split(' ')[0] for _, _, m in SUITES} | {'src/lanec.c'})},
        'source_words': total,
        'source_lines_with_words': len(rows),
        'line_map': line_map,
        'aliases': wholefile.aliases(mission, table, line_map),
        'abi_entries': {'basic': len(abi)},
        'whole_file': {'assembled_words': total, 'words_by_owner': dict(sorted(owners.items())),
                       'instruction_words_executed_by_oracle': count('instruction'),
                       'inline_data_words': count('inline data'), 'constant_words': count('constant'),
                       'constant_instruction_words': count('constant instruction'),
                       'unreachable_instruction_words': count('unreachable instruction')},
        'note': ('Every assembled word of the file has one owner. Every instruction word but those listed as read '
                 'as data or unreachable was started in the unchanged original program in Virtual AGC under at '
                 'least one case of the Lane C Pinball suites (oracle LANEC_TRACE), and the native modules '
                 'reproduce those cases\' complete results (2048 erasable words, registers, channels, superbank, '
                 'interrupt state). Call arguments are compiled into the native call. Constants are compiled from '
                 'symbols.txt WORD rows or as literals where native code reads them. Every labelled entry and every '
                 'return into the file from a callee outside it is a transfer-ABI routine (abi_routine; '
                 'tools/lanec/routines.py pin checks this against the listing). The words read as data and the '
                 'unreachable ones are listed with the reason, checked against the listing.'),
    }


if __name__ == '__main__':
    survey = '--survey' in sys.argv
    args = [a for a in sys.argv[1:] if a != '--survey']
    out = pathlib.Path(args[0] if args else 'build/pin-cert')
    out.mkdir(parents=True, exist_ok=True)
    for mission in ('LM', 'CM'):
        data = build(mission, survey)
        rows = ',\n'.join('  ' + json.dumps(r) for r in data['line_map'])
        text = json.dumps(dict(data, line_map=None), indent=1).replace('"line_map": null',
                                                                        '"line_map": [\n' + rows + '\n ]')
        (out / f'lanec-pin-{mission.lower()}-source-map.json').write_text(text + '\n')
        w = data['whole_file']
        print(f'PASS {mission} {data["source_path"]}: {w["assembled_words"]} assembled words; '
              f'{w["instruction_words_executed_by_oracle"]} executed, {w["inline_data_words"]} call arguments, '
              f'{w["constant_words"]} constants, {w["constant_instruction_words"]} instructions read as data, '
              f'{w["unreachable_instruction_words"]} unreachable; owners {w["words_by_owner"]}')
