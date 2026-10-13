#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-file source-to-native map for Luminary099 EXTENDED_VERBS.agc, the
evidence behind its full-file certificate.

  python3 tools/lanec/wholefile_ev.py OUTDIR

It reads every assembled word of the file from the pinned listing and runs
the Lane C extended-verb suites through the oracle with LANEC_TRACE. Each
word gets exactly one owner:
  - an instruction word started by the unchanged original program in Virtual
    AGC under a suite's cases belongs to that suite's milestone (the first
    suite below that started it);
  - an interpretive word read by the original interpreter (INDEX LOC) under
    lanec-ev-lmi's cases belongs to lanec-ev-lmi;
  - a data word right after a call (its argument: CADR, 2CADR, ADRES, ...)
    belongs to the call's owner;
  - any other data word is a constant: a symbols.txt WORD row or a literal
    that a native module reads, or a label whose readers are listed, or a
    word nothing reads;
  - one instruction word is unreachable (UNREACHABLE below, with the reason
    checked against the listing).
The suites' complete results match the native code (tools/lanec/diff.py), so
every owned word is reproduced by its milestone. Each instruction row names
the C function that carries it and, when the word is a transfer-ABI entry
(src/lanec_ev_*_routines.c; interpretive entries keyed by LOC), the routine
it is registered as. Writes
OUTDIR/lanec-ev-lm-source-map.json.
"""
import json, os, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/lanec'))
import symbols  # noqa: E402
import wholefile  # noqa: E402

wholefile.FILE = FILE = 'EXTENDED_VERBS.agc'
# suite -> (milestone, native module), in ownership priority order
SUITES = (('ev-front', 'lanec-ev-front', 'src/lanec_ev_front.c'),
          ('ev-short', 'lanec-ev-short', 'src/lanec_ev_short.c'),
          ('ev-align', 'lanec-ev-align', 'src/lanec_ev_align.c'),
          ('ev-radar', 'lanec-ev-radar', 'src/lanec_ev_radar.c'),
          ('ev-dap', 'lanec-ev-dap', 'src/lanec_ev_dap.c'),
          ('ev-time', 'lanec-ev-time', 'src/lanec_ev_time.c'),
          ('ev-track', 'lanec-ev-track', 'src/lanec_ev_track.c'),
          ('ev-lmi', 'lanec-ev-lmi', 'src/lanec_ev_lmi.c'))
INTERPRETIVE = 'ev-lmi'
DATA = wholefile.DATA | {'REMADR', 'MM'}
# (bank, address): (listing text, the three words before it, reason)
UNREACHABLE = {
    ('43', 0o2661): ('TC ALM/END', ('CS FLGWRD11', 'MASK LRBYBIT', 'CCS A'),
                     'RDRUSECK: CS FLGWRD11, MASK LRBYBIT leaves +0 or bit 16-15 set (LRBYBIT = BIT15), so '
                     'CCS A never takes its positive branch to this TC; the native radar check '
                     '(lanec-ev-radar) has no such path to translate.'),
}


def traces(mission):
    executed, interpreted = {}, {}
    env = dict(os.environ)
    for suite, _, _ in SUITES:
        out = ROOT / f'build/lanec-{suite}-trace.txt'
        env['LANEC_TRACE'] = str(out)
        subprocess.run(['build/lanec-oracle', suite, 'deps/lm-powered-fixture/MAIN.agc.bin',
                        'deps/cm-powered-fixture/MAIN.agc.bin'], cwd=ROOT, env=env, check=True,
                       stdout=subprocess.DEVNULL)
        for line in out.read_text().split('\n'):
            kind, _, where = line.partition(' ')
            if kind not in (mission, mission + 'I'):
                continue
            bank, addr = where.split(',')
            key = (None if bank == '0' else f'{int(bank, 8):02o}', int(addr, 8))
            if kind == mission:
                executed.setdefault(key, suite)
            elif suite == INTERPRETIVE:
                interpreted.setdefault(key, suite)
    return executed, interpreted


def literal_in(label, words):
    """The native module that defines LABEL as the same literal word."""
    for _, _, mod in SUITES:
        m = re.search(rf'#define {re.escape(symbols.cname(label))} (0[0-7]*)u\b', (ROOT / mod).read_text())
        if m and len(words) == 1 and int(m.group(1), 8) == int(words[0], 8):
            return mod
    return None


def build(mission):
    table = symbols.symbols(wholefile.FIXTURE[mission])
    rows = wholefile.listing_words(mission)
    wholefile.original_lines(mission, rows)
    executed, interpreted = traces(mission)
    milestone = {s: (ms, mod) for s, ms, mod in SUITES}
    native = '\n'.join((ROOT / mod).read_text() for _, _, mod in SUITES)
    src = (ROOT.parent / wholefile.ORIGINAL[mission] / FILE).read_text(errors='replace').splitlines()
    read = wholefile.read_addresses(mission, {k: v for k, v in table.items()}, native)
    abi, abi_i = wholefile.abi_entries(mission, 'ev'), wholefile.abi_entries(mission, 'ev', True)
    owners, line_map, label, call_owner = {}, [], None, None
    for i, r in enumerate(rows):
        key = (r['bank'], r['addr'])
        fields = r['text'].split('#', 1)[0].split()
        labelled = not src[r['line'] - 1][:1].isspace()
        if labelled:
            label = fields[0]
        op = next((f for f in fields if f in DATA), None) if fields else None
        if key in executed and not op:
            ms, mod = milestone[executed[key]]
            entry = {'kind': 'instruction', 'owner': ms, 'native': f'{mod}: {wholefile.c_function(mod, label)}',
                     'evidence': f'executed under {executed[key]}'}
            at = wholefile.physical(r['bank'], r['addr'])
            if at in abi:
                entry['abi_routine'] = '{}[{}] -> {}'.format(abi[at][0], at, abi[at][1])
            call_owner = (ms, mod)
        elif key in interpreted and not op:
            ms, mod = milestone[interpreted[key]]
            entry = {'kind': 'interpretive', 'owner': ms, 'native': f'{mod}: {wholefile.c_function(mod, label)}',
                     'evidence': f'read by the interpreter under {interpreted[key]}'}
            at = wholefile.physical(r['bank'], r['addr'])
            if at in abi_i:
                entry['abi_routine'] = '{}[{}] (LOC) -> {}'.format(abi_i[at][0], at, abi_i[at][1])
            call_owner = None
        elif key in UNREACHABLE:
            text, before, why = UNREACHABLE[key]
            got = [' '.join(rows[i - k]['text'].split('#', 1)[0].split()[-2:]) for k in (3, 2, 1)]
            if ' '.join(fields[-2:]) != text or tuple(got) != before:
                sys.exit(f'FAIL: {mission} {r["bank"]},{r["addr"]:04o} is not the unreachable word described')
            entry = {'kind': 'unreachable instruction', 'owner': 'lanec-ev-radar', 'native': 'none (unreachable)',
                     'evidence': why}
            call_owner = None
        elif op and call_owner and not labelled:
            entry = {'kind': 'inline data', 'owner': call_owner[0], 'native': f'{call_owner[1]} (call argument)'}
        elif op:
            name = label if labelled else f'{label} (following word)'
            literal = labelled and literal_in(label, r['words'])
            if key in read:
                how = f'compiled constant {read[key]} (symbols.txt WORD row)'
            elif literal:
                how = f'compiled literal in {literal}'
            elif not labelled:
                sys.exit(f'FAIL: {mission} unlabelled constant {r["bank"]},{r["addr"]:04o} is not read as a WORD row')
            else:
                users = wholefile.readers(mission, label)
                how = ('preserved in the rope; read only by ' + ', '.join(users) if users else
                       'preserved in the rope; no source line of the program refers to it')
            entry = {'kind': 'constant', 'owner': 'constants', 'native': how, 'constant': name}
            call_owner = None
        else:
            sys.exit(f'FAIL: {mission} instruction {r["bank"]},{r["addr"]:04o} "{r["text"].strip()}" '
                     'never executed or interpreted')
        owners[entry['owner']] = owners.get(entry['owner'], 0) + len(r['words'])
        line_map.append(dict(line=r['line'], address=f'{r["bank"] or "FF"},{r["addr"]:04o}',
                             source=' '.join(fields), words=r['words'], **entry))
    total = sum(len(r['words']) for r in rows)
    count = lambda kind: sum(len(e['words']) for e in line_map if e['kind'] == kind)  # noqa: E731
    return {
        'source_path': f'{wholefile.ORIGINAL[mission]}/{FILE}',
        'source_sha256': wholefile.sha(ROOT.parent / wholefile.ORIGINAL[mission] / FILE),
        'rope_sha256': wholefile.ROPE[mission],
        'native': {p: wholefile.sha(ROOT / p) for p in sorted({mod for _, _, mod in SUITES} | {'src/lanec.c'})},
        'source_words': total,
        'source_lines_with_words': len(rows),
        'line_map': line_map,
        'aliases': wholefile.aliases(mission, table, line_map),
        'abi_entries': {'basic': len(abi), 'interpretive': len(abi_i)},
        'whole_file': {'assembled_words': total, 'words_by_owner': dict(sorted(owners.items())),
                       'instruction_words_executed_by_oracle': count('instruction'),
                       'interpretive_words_read_by_oracle': count('interpretive'),
                       'inline_data_words': count('inline data'), 'constant_words': count('constant'),
                       'unreachable_instruction_words': count('unreachable instruction')},
        'note': ('Every assembled word of the file has one owner. Every instruction word but one was started, '
                 'and every interpretive word was read by the interpreter, in the unchanged original program in '
                 'Virtual AGC under at least one case of the Lane C extended-verb suites (oracle LANEC_TRACE), '
                 'and the native modules reproduce those cases\' complete results (2048 erasable words, '
                 'registers, channels, superbank, interrupt state). Call arguments are compiled into the native '
                 'call. Constants are compiled from symbols.txt WORD rows or as literals where native code reads them. Every '
                 'labelled entry and every return into the file from a callee is a transfer-ABI routine (abi_routine; '
                 'tools/lanec/routines.py ev checks this against the listing). The one '
                 'unreachable word is listed with the reason, checked against the listing.'),
    }


if __name__ == '__main__':
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'build/ev-cert')
    out.mkdir(parents=True, exist_ok=True)
    data = build('LM')
    # One line_map row per line keeps the map under the in-tree evidence limit.
    rows = ',\n'.join('  ' + json.dumps(r) for r in data['line_map'])
    text = json.dumps(dict(data, line_map=None), indent=1).replace('"line_map": null', '"line_map": [\n' + rows + '\n ]')
    (out / 'lanec-ev-lm-source-map.json').write_text(text + '\n')
    w = data['whole_file']
    print(f'PASS LM {data["source_path"]}: {w["assembled_words"]} assembled words; '
          f'{w["instruction_words_executed_by_oracle"]} executed, {w["interpretive_words_read_by_oracle"]} '
          f'interpreted, {w["inline_data_words"]} call arguments, {w["constant_words"]} constants, '
          f'{w["unreachable_instruction_words"]} unreachable; owners {w["words_by_owner"]}')
