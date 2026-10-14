#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""MAIN.agc of both missions: the include manifest, checked against the assembly.

MAIN.agc holds no instruction, constant or declaration: only the $-include
lines that make the program, in assembly order. For each mission this script
checks that
- every include of the checked-in MAIN.agc names a checked-in source file
  (two Luminary099 includes use names the scan's files do not have: ERRATA),
  and every other checked-in .agc is included exactly once;
- the pinned assembled program (build/fixed-data/<mission>/MAIN.agc, the
  Virtual AGC MAIN that yaYUL assembles into the oracle rope) includes the
  same files in the same order, under its own file names (ALIAS);
- in yaYUL's listing, MAIN.agc itself emits no word, every emitted fixed word
  is attributed to exactly one include, and the words equal the rope.
It writes build/main-assembly-<lm|cm>.json: per include, in order, the
checked-in file, the assembler's file name, the fixed words it emits (count,
banks, runs, a 32-bit sum); and the native
manifest src/main_manifest.c (--check: fail if that file is out of date).
"""
import argparse
import hashlib
import json
import re
import subprocess
from pathlib import Path

R = Path(__file__).resolve().parents[1]
TOP = R.parent
MISSIONS = {'lm': 'Luminary099', 'cm': 'Comanche055'}
# Checked-in MAIN.agc include -> checked-in file, where the scan's include
# line names a file the transcription saved under another name.
ERRATA = {'Luminary099': {'LAMBERT_AIMPOINT_GUIDANCE.agc': 'GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc',
                          'TRIM_GIMBAL_CNTROL_SYSTEM.agc': 'TRIM_GIMBAL_CONTROL_SYSTEM.agc'},
          'Comanche055': {}}
# Checked-in file -> the pinned assembly's name for it, where the two names
# differ beyond punctuation (the rest match once reduced to letters and digits).
ALIAS = {'TAGS_FOR_RELATIVE_SETLOC.agc': 'TAGS_FOR_RELATIVE_SETLOC_AND_BLANK_BANK_CARDS.agc',
         'GROUND_TRACKING_DETERMINATION_PROGRAM.agc': 'GROUND_TRACKING_DETERMINATION_PROGRAM_-_P21.agc',
         'STABLE_ORBIT.agc': 'STABLE_ORBIT_-_P38-P39.agc',
         'P34-35_P74-75.agc': 'P34-P35,_P74-P75.agc',
         'IMU_PERFORMANCE_TEST_2.agc': 'IMU_PERFORMANCE_TESTS_2.agc',
         'INTERPRETIVE_CONSTANT.agc': 'INTERPRETIVE_CONSTANTS.agc',
         'Q_R-AXIS_RCS_AUTOPILOT.agc': 'Q,R-AXES_RCS_AUTOPILOT.agc',
         'RT8_OP_CODES.agc': 'RTB_OP_CODES.agc',
         'P30-P37.agc': 'P30,P37.agc',
         'R60_62.agc': 'R60,R62.agc',
         'GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc': 'LAMBERT_AIMPOINT_GUIDANCE.agc'}


def includes(path):
    return re.findall(r'^\$(\S+)', path.read_text(), re.M)


def key(name):
    return re.sub(r'[^A-Z0-9]', '', name.upper())


def fixed_address(text):
    m = re.fullmatch(r'([0-7]{2}),([0-7]{4})', text)
    if m:
        bank, addr = int(m[1], 8), int(m[2], 8)
        assert bank < 36 and 0o2000 <= addr < 0o4000, text
        return bank * 1024 + addr - 0o2000
    if re.fullmatch('[0-7]{4}', text) and 0o4000 <= int(text, 8) < 0o10000:
        return int(text, 8)
    return None


def rope_word(rope, physical):
    bank, offset = divmod(physical, 1024)
    pos = 2 * ((bank ^ 2 if bank < 4 else bank) * 1024 + offset)
    return int.from_bytes(rope[pos:pos + 2], 'big') >> 1


def emitted(listing, rope):
    """{assembler file name: [(physical, word)]} from yaYUL's listing."""
    out, current = {'MAIN.agc': []}, 'MAIN.agc'
    for text in listing.splitlines():
        if text.startswith('(End of include-file '):
            current = 'MAIN.agc'
            continue
        m = re.match(r'^\d{6},\d{6}: (.*)$', text)
        if not m:
            continue
        if m[1].startswith('$'):
            current = m[1][1:].split()[0]
            assert current not in out, current
            out[current] = []
            continue
        words = [int(w, 8) for w in text[33:44].split() if re.fullmatch('[0-7]{5}', w)]
        physical = fixed_address(text[14:23].strip())
        if words and physical is not None:
            for k, w in enumerate(words):
                assert rope_word(rope, physical + k) == w, (current, text)
                out[current].append((physical + k, w))
    return out


def mission_report(mission):
    main = TOP / mission / 'MAIN.agc'
    ours = includes(main)
    errata = ERRATA[mission]
    files = [errata.get(n, n) for n in ours]
    for n in files:
        assert (TOP / mission / n).is_file(), (mission, n)
    assert len(set(files)) == len(files), mission
    others = sorted(p.name for p in (TOP / mission).glob('*.agc') if p.name != 'MAIN.agc')
    assert sorted(files) == others, (mission, sorted(set(others) ^ set(files)))
    work = R / 'build/fixed-data' / mission
    theirs = includes(work / 'MAIN.agc')
    assert len(theirs) == len(files), (mission, len(theirs), len(files))
    for f, t in zip(files, theirs):
        assert ALIAS.get(f, t) == t and (f in ALIAS or key(f) == key(t)), (mission, f, t)
    rope = (work / 'MAIN.agc.bin').read_bytes()
    words = emitted((work / 'assembly.lst').read_text(), rope)
    assert words.pop('MAIN.agc') == [], mission
    assert list(words) == theirs, mission
    seen = {}
    for t, ws in words.items():
        for p, _ in ws:
            assert p not in seen, (mission, t, oct(p), seen.get(p))
            seen[p] = t
    listing = (work / 'assembly.lst').read_text()
    checksums = sorted(fixed_address(f'{b},{a}') for _, b, a in
                       re.findall(r'Bugger word ([0-7]{5}) at ([0-7]{2}),([0-7]{4})\.', listing))
    assert len(checksums) == 36 and not set(checksums) & set(seen), mission
    # (Every include has a coverage entry; its status is not recorded here, so
    # a later certificate flip leaves the source map as it is.)
    coverage = {e['path'] for e in json.loads((R / 'docs/port-coverage.json').read_text())['files']}
    rows = []
    for n, f, t in zip(ours, files, theirs):
        ws = words[t]
        banks = sorted({p // 1024 for p, _ in ws})
        runs = []
        for p, _ in ws:
            if runs and runs[-1][0] + runs[-1][1] == p:
                runs[-1][1] += 1
            else:
                runs.append([p, 1])
        rows.append({'include': n, 'file': f'{mission}/{f}', 'assembler_file': t,
                     'erratum': n != f, 'words': len(ws), 'banks': [f'{b:02o}' for b in banks],
                     'first': f'{ws[0][0]:05o}' if ws else None, 'last': f'{ws[-1][0]:05o}' if ws else None,
                     'sum32': sum(w for _, w in ws) & 0xffffffff,
                     'runs': runs})
        assert f'{mission}/{f}' in coverage, f
    return {'source_path': f'{mission}/MAIN.agc', 'source_sha256': hashlib.sha256(main.read_bytes()).hexdigest(),
            'assembled_main_sha256': hashlib.sha256((work / 'MAIN.agc').read_bytes()).hexdigest(),
            'rope_sha256': hashlib.sha256(rope).hexdigest(), 'includes': len(rows),
            'emitted_fixed_words': len(seen), 'main_emitted_words': 0,
            'errata': {k: v for k, v in errata.items()}, 'bank_checksums': checksums, 'rows': rows}


def manifest_c(reports):
    out = ['/* SPDX-License-Identifier: MIT; generated by tools/main_assembly.py from yaYUL\'s listing of',
           ' * each mission\'s MAIN.agc (include/main_manifest.h, docs/MAIN-ASSEMBLY.md). */',
           '#include "main_manifest.h"']
    tables = []
    for m, rep in reports:
        runs, files = [], []
        for r in rep['rows']:
            files.append(f' {{"{r["include"]}",{len(runs)}u,{len(r["runs"])}u,{r["words"]}u,0x{r["sum32"]:08x}ul}},')
            runs += r['runs']
        out.append(f'static const main_manifest_file {m}_files[{len(files)}]={{')
        out += files
        out.append('};')
        out.append(f'static const main_manifest_run {m}_runs[{len(runs)}]={{')
        out += [''.join(f'{{0{a:o},{c}u}},' for a, c in runs[i:i + 8]) for i in range(0, len(runs), 8)]
        out.append('};')
        out.append(f'static const uint16_t {m}_checksum_at[36]={{')
        out += [''.join(f'0{a:o},' for a in rep['bank_checksums'][i:i + 12]) for i in range(0, 36, 12)]
        out.append('};')
        tables.append(f' {{{m}_files,{len(files)}u,{m}_runs,{len(runs)}u,{m}_checksum_at,{rep["emitted_fixed_words"]}u}},')
    out.append('const main_manifest main_manifests[2]={')
    out += tables
    out.append('};')
    return '\n'.join(out) + '\n'


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--check', action='store_true', help='fail if src/main_manifest.c is not what the listings give')
    ap.add_argument('--proof', action='store_true', help='also run the yaAGC oracle (tools/main_manifest_reference.c)')
    a = ap.parse_args()
    reports = []
    for m in ('lm', 'cm'):
        rep = mission_report(MISSIONS[m])
        reports.append((m, rep))
        (R / f'build/main-assembly-{m}.json').write_text(json.dumps(rep, indent=2) + '\n')
        print(f'PASS: {rep["source_path"]}: {rep["includes"]} includes in the assembled order, '
              f'{rep["emitted_fixed_words"]} fixed words each from one include, none from MAIN.agc'
              + (f'; errata {rep["errata"]}' if rep['errata'] else ''))
    (R / 'build/main-assembly-source-map.json').write_text(json.dumps(
        {'milestone': 'main-assembly', 'tool': 'tools/main_assembly.py', 'native': 'src/main_manifest.c',
         'missions': {MISSIONS[m]: rep for m, rep in reports}}, indent=2) + '\n')
    text, path = manifest_c(reports), R / 'src/main_manifest.c'
    if a.check:
        if not path.exists() or path.read_text() != text:
            raise SystemExit('FAIL: src/main_manifest.c differs from the listings; run tools/main_assembly.py')
        print('PASS: src/main_manifest.c matches the listings')
    else:
        path.write_text(text)
    if a.proof:
        proof(reports)


def proof(reports):
    """build/main-assembly-proof.json: the native manifest against yaAGC's own load of each rope, and
    yaAGC run from each unused TC-to-itself word."""
    e = R / 'deps/virtualagc/yaAGC'
    exe = R / 'build/main-manifest-reference'
    subprocess.run(['cc', '-std=c99', '-O2', '-Iinclude', f'-I{e}', 'tools/main_manifest_reference.c',
                    'src/main_manifest.c', str(e / 'agc_engine.o'), str(e / 'agc_engine_init.o'), str(e / 'rfopen.o'),
                    '-o', str(exe)], cwd=R, check=True)
    ropes = [R / 'build/fixed-data' / MISSIONS[m] / 'MAIN.agc.bin' for m in ('lm', 'cm')]
    run = subprocess.run([str(exe)] + [str(r) for r in ropes], cwd=R, capture_output=True, text=True)
    assert run.returncode == 0 and run.stdout.endswith('\n0 failures\n'), run.stdout + run.stderr
    lines = run.stdout.splitlines()[:2]
    missions = {}
    for (m, rep), line in zip(reports, lines):
        files, loops = map(int, re.match(r'\S+: (\d+) included files .*, (\d+) unused', line).groups())
        assert files == rep['includes']
        missions[MISSIONS[m]] = {'rope_sha256': rep['rope_sha256'], 'source_sha256': rep['source_sha256'],
                                 'cases': files + loops, 'files_equal': files, 'unused_words_run': loops,
                                 'differing': 0, 'result': line}
    out = {'milestone': 'main-assembly', 'oracle': 'tools/main_manifest_reference.c (yaAGC agc_engine, agc_load_binfile)',
           'method': 'yaAGC loads each rope with its own loader; each included file of the native manifest '
                     '(src/main_manifest.c) is compared with the words yaAGC holds at its runs, and yaAGC runs '
                     'from each unused TC-to-itself word in banks 0-37 (four instructions, Z stays)',
           'manifest_sha256': hashlib.sha256((R / 'src/main_manifest.c').read_bytes()).hexdigest(),
           'missions': missions}
    (R / 'build/main-assembly-proof.json').write_text(json.dumps(out, indent=2) + '\n')
    print('PASS: ' + '; '.join(lines))


if __name__ == '__main__':
    main()
