#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Completion certificate records for Comanche055 FRESH_START_AND_RESTART.agc.

Three milestones translate the file: cm-fresh-start (bank 05 and
fixed-fixed), cm-v37 (bank 04 basic code and bank 20's VAC5STOR) and
cm-stateup (the interpretive code: bank 13 and R00's two pairs in bank 04). This tool writes one record
per certificate category to evidence/cm-fresh-start-complete/, each a small
manifest naming the per-milestone records it aggregates with their sha256:

  cm-fresh-start-complete-source-map.json   the three milestones' source maps
      (docs) and the word-ownership check: together they own every assembled
      word of the file except the patch-only STARTSIM, with no overlaps
      (per-word detail in cm-fresh-start-complete-line-map.json)
  cm-fresh-start-complete-comparison.json   the three yaAGC proof records;
      each oracle is rerun on the unmodified rope with the coverage trace
      (tools/cert_trace.h) and must print exactly its frozen snapshot, and
      every translated instruction must be executed by those runs
  cm-fresh-start-complete-host.json         the three host and sanitizer logs
  cm-fresh-start-complete-c64.json          the three VICE logs

It fails on any hash mismatch, ownership gap or overlap, oracle snapshot
difference, unexecuted instruction or failed check.

  python3 tools/cm_fresh_start_complete.py           # rerun oracles, write records
  python3 tools/cm_fresh_start_complete.py --check   # also fail if a record would change
"""
import argparse
import hashlib
import importlib.util
import json
import pathlib
import re
import subprocess
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from agc_sites_generate import listing_lines  # noqa: E402

OUT = R / 'evidence/cm-fresh-start-complete'
PREFIX = 'cm-fresh-start-complete'
MISSION = 'Comanche055'
SOURCE = 'FRESH_START_AND_RESTART.agc'
# milestone -> (freeze module, reference, native file, source-map doc)
MILESTONES = {
    'cm-fresh-start': ('cm_fresh_start_freeze', 'tools/cm_fresh_start_reference.c', 'src/cm_fresh_start.c', 'docs/CM-FRESH-START.md'),
    'cm-v37': ('cm_v37_freeze', 'tools/cm_v37_reference.c', 'src/cm_v37.c', 'docs/CM-V37.md'),
    'cm-stateup': ('cm_stateup_freeze', 'tools/cm_stateup_reference.c', 'src/cm_stateup.c', 'docs/CM-STATEUP.md'),
}
# What each milestone's source map declares it owns: (bank or None for
# fixed-fixed, first, last) address ranges, inclusive.
OWNS = {
    'cm-fresh-start': [('05', 0o2342, 0o2344), ('05', 0o2351, 0o3217), (None, 0o4106, 0o4113)],
    'cm-v37': [('04', 0o2000, 0o2130), ('04', 0o2134, 0o2565), ('20', 0o2000, 0o2064)],
    'cm-stateup': [('13', 0o2000, 0o2035), ('04', 0o2131, 0o2133)],
}
# STARTSIM: written only if STARTSW is patched to TCF STARTSIM; the rope has TCF SKIPSIM.
EXCLUDED = [('05', 0o2345, 0o2350)]
DATA_OPS = {'OCT', 'DEC', '2DEC', 'CADR', '2CADR', 'GENADR', 'ADRES', 'ECADR', 'FCADR', '2FCADR', 'BBCON',
            'VN', 'MM', 'OCTAL', '2OCT', 'REMADR', 'NV'}


def sha(path):
    return hashlib.sha256((R / path).read_bytes()).hexdigest()


def split(address):
    bank, _, offset = address.partition(',')
    return (bank, int(offset, 8)) if offset else (None, int(bank, 8))


def physical(bank, z):
    return (z >> 10) * 1024 + (z & 0o1777) if bank is None else int(bank, 8) * 1024 + z - 0o2000


def within(ranges, bank, z):
    return any(b == bank and lo <= z <= hi for b, lo, hi in ranges)


def run_oracle(name):
    module, reference, _, _ = MILESTONES[name]
    spec = importlib.util.spec_from_file_location(module, R / 'tools' / f'{module}.py')
    freeze = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(freeze)
    trace = R / f'build/{name}-complete.trace'
    exe = R / f'build/{name}-complete-reference'
    subprocess.run(['cc', '-std=c99', '-O2', f'-DCERT_TRACE={trace}', '-DCERT_NEWOPS=06037u', '-Iinclude', '-Itests', '-Isrc', '-Itools',
                    '-I' + str(freeze.E), reference, *freeze.CORE, '-o', str(exe)], cwd=R, check=True)
    out = subprocess.run([str(exe), str(freeze.ROPE)], cwd=R, check=True, capture_output=True).stdout
    return hashlib.sha256(out).hexdigest(), {int(x) for x in trace.read_text().split()}


def counts(path, pattern):
    """The PASS lines and the checks/failures of a log."""
    text = (R / path).read_text()
    passes = [l for l in text.splitlines() if l.startswith('PASS')]
    found = re.findall(pattern, text)
    return passes, found


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    errors = []
    base = R / 'build/fixed-data' / MISSION
    rows = [r for r in listing_lines((base / 'assembly.lst').read_text(errors='replace')) if r[0] == SOURCE and r[3]]
    rope_sha = hashlib.sha256((base / 'MAIN.agc.bin').read_bytes()).hexdigest()
    manifests = {}

    # Independent assembly comparison: the three proofs, each oracle rerun with the trace.
    seen, suites = set(), {}
    for name in MILESTONES:
        proof_path = f'evidence/{name}/{name}-proof.json'
        proof = json.loads((R / proof_path).read_text())
        got, trace = run_oracle(name)
        seen |= trace
        if got != proof['snapshot_sha256']:
            errors.append(f'{name}: traced oracle output {got} differs from the proof snapshot')
        if proof['rope_sha256'] != rope_sha:
            errors.append(f'{name}: proof rope differs')
        suites[name] = {'record': proof_path, 'sha256': sha(proof_path), 'cases': proof['cases'],
                        'snapshot_sha256': proof['snapshot_sha256'], 'rerun_snapshot_sha256': got,
                        'result': 'PASS' if got == proof['snapshot_sha256'] else 'FAIL'}

    # Source map: word ownership and execution, per word.
    stateup = sorted(int(m, 8) for m in re.findall(r'case (0[0-7]+)u:', (R / 'src/cm_stateup.c').read_text()))
    labels = {n: sorted(int(m, 8) for m in re.findall(r'^L([0-7]+):', (R / MILESTONES[n][2]).read_text(), re.M))
              for n in ('cm-fresh-start', 'cm-v37')}
    line_map, owned_words, excluded_words, overlaps, gaps, unexecuted = [], 0, 0, [], [], []
    prefix = None
    for path, line, address, words, label, op, opnd in rows:
        bank, z = split(address)
        owners = [n for n, ranges in OWNS.items() if within(ranges, bank, z)]
        entry = {'line': line, 'address': address, 'source': ' '.join(x for x in (label, op, opnd) if x),
                 'words': [f'{w:05o}' for w in words]}
        if within(EXCLUDED, bank, z):
            entry.update(owner=None, kind='excluded (STARTSIM, patch-only)', executed_by_oracle=False)
            excluded_words += len(words)
            if owners:
                overlaps.append(address)
            line_map.append(entry)
            prefix = None
            continue
        if len(owners) != 1:
            (overlaps if owners else gaps).append(address)
            continue
        who = owners[0]
        owned_words += len(words)
        ph = physical(bank, z)
        entry['owner'] = who
        if who == 'cm-stateup':
            pair = max(p for p in stateup if p <= z and (p >= 0o2131) == (bank == '04'))
            entry['kind'] = 'interpretive pair' if pair == z else 'interpretive operand'
            entry['native'] = f'src/cm_stateup.c (case 0{pair:o})'
            entry['executed_by_oracle'] = physical(bank, pair) in seen
            prefix = None
        elif op in DATA_OPS or (op == '' and opnd == ''):
            entry['kind'] = 'data'
            entry['native'] = 'compiled operand/constant table (tools/file_operands_generate.py, checked against the rope)'
            entry['executed_by_oracle'] = None
            prefix = None
        else:
            at = [x for x in labels[who] if x <= z]
            entry['kind'] = 'instruction'
            entry['native'] = f'{MILESTONES[who][2]} (L{at[-1]:o})' if at else MILESTONES[who][2]
            # yaAGC runs the instruction after EXTEND or INDEX in the same
            # step (ExtraCode/IndexValue set), so the trace sees its prefix.
            entry['executed_by_oracle'] = ph in seen or (prefix is not None and prefix in seen)
            if op in ('EXTEND', 'INDEX', 'NDX'):
                prefix = ph if prefix is None or prefix not in seen else prefix
            else:
                prefix = None
        if entry['executed_by_oracle'] is False:
            unexecuted.append(f"{address} {entry['source']}")
        line_map.append(entry)
    total = sum(len(r[3]) for r in rows)
    if gaps:
        errors.append(f'words no milestone owns: {gaps[:8]}')
    if overlaps:
        errors.append(f'words owned twice: {overlaps[:8]}')
    if owned_words + excluded_words != total:
        errors.append(f'{owned_words} owned + {excluded_words} excluded != {total} words')
    if unexecuted:
        errors.append(f'instructions the oracle runs never execute: {unexecuted}')
    line_map_path = f'evidence/{PREFIX}/{PREFIX}-line-map.json'
    line_map_doc = {'source_path': f'{MISSION}/{SOURCE}', 'rope_sha256': rope_sha, 'line_map': line_map}
    manifests[f'{PREFIX}-line-map.json'] = line_map_doc
    source = R / 'deps/virtualagc' / MISSION / SOURCE
    manifests[f'{PREFIX}-source-map.json'] = {
        'category': 'source_to_native_map', 'source_path': f'{MISSION}/{SOURCE}',
        'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(), 'rope_sha256': rope_sha,
        'aggregates': {n: {'record': MILESTONES[n][3], 'sha256': sha(MILESTONES[n][3]),
                           'native': MILESTONES[n][2], 'native_sha256': sha(MILESTONES[n][2]),
                           'owns': [f'{b or "FF"},{lo:04o}-{hi:04o}' for b, lo, hi in OWNS[n]]} for n in MILESTONES},
        'excluded': {'STARTSIM': ['05,2345-2350'], 'why': 'runs only if STARTSW is patched to TCF STARTSIM; the rope has TCF SKIPSIM'},
        'ownership_check': {'source_words': total, 'owned_words': owned_words, 'excluded_words': excluded_words,
                            'gaps': len(gaps), 'overlaps': len(overlaps),
                            'result': 'PASS' if not gaps and not overlaps and owned_words + excluded_words == total else 'FAIL'},
        'execution_check': {'instructions': sum(1 for e in line_map if e['kind'] in ('instruction', 'interpretive pair')),
                            'not_executed': unexecuted, 'result': 'PASS' if not unexecuted else 'FAIL'},
        'line_map': {'record': line_map_path}}
    manifests[f'{PREFIX}-comparison.json'] = {
        'category': 'independent_assembly_comparison', 'source_path': f'{MISSION}/{SOURCE}', 'rope_sha256': rope_sha,
        'method': 'each milestone\'s cases through the native routines and through the unmodified rope in yaAGC '
                  '(host only); its freeze script checked the outputs byte-identical and froze the snapshot; this '
                  'tool reran each oracle with the coverage trace and compared with that snapshot',
        'aggregates': suites, 'cases': sum(s['cases'] for s in suites.values()),
        'result': 'PASS' if all(s['result'] == 'PASS' for s in suites.values()) else 'FAIL'}
    for category, kind, pattern in (('host', 'host', r'(\d+) checks, (\d+) failures'),
                                    ('c64', 'vice', r'(\d+) checks, (\d+) failures')):
        agg = {}
        for name in MILESTONES:
            log = f'evidence/{name}/{name}-{kind}.log'
            passes, found = counts(log, pattern)
            checks = [int(c) for c, f in found]
            failures = [int(f) for c, f in found]
            ok = bool(found) and not any(failures)
            agg[name] = {'record': log, 'sha256': sha(log), 'pass_lines': passes,
                         'checks': checks, 'failures': sum(failures), 'result': 'PASS' if ok else 'FAIL'}
            manifest_sha = json.loads((R / f'evidence/{name}/SHA256.json').read_text())['files'][f'{name}-{kind}.log']['sha256']
            if manifest_sha != agg[name]['sha256']:
                errors.append(f'{log}: sha256 differs from evidence/{name}/SHA256.json')
            if not ok:
                errors.append(f'{log}: a check failed or no checks found')
        manifests[f'{PREFIX}-{category}.json'] = {
            'category': 'host_and_sanitizer_results' if category == 'host' else 'c64_execution_results',
            'aggregates': agg, 'result': 'PASS' if all(a['result'] == 'PASS' for a in agg.values()) else 'FAIL'}
    for name in MILESTONES:
        proof_sha = json.loads((R / f'evidence/{name}/SHA256.json').read_text())['files'][f'{name}-proof.json']['sha256']
        if proof_sha != suites[name]['sha256']:
            errors.append(f'evidence/{name}/{name}-proof.json: sha256 differs from evidence/{name}/SHA256.json')
    if errors:
        raise SystemExit('FAIL:\n  ' + '\n  '.join(errors))
    OUT.mkdir(parents=True, exist_ok=True)
    stale = []
    for name, doc in manifests.items():
        text = json.dumps(doc, indent=1) + '\n'
        path = OUT / name
        if args.check and (not path.exists() or path.read_text() != text):
            stale.append(name)
        elif not args.check:
            path.write_text(text)
    if stale:
        raise SystemExit(f'FAIL: stale records {stale}; run tools/cm_fresh_start_complete.py')
    print(f"PASS: {total} words: {owned_words} owned by exactly one milestone, {excluded_words} STARTSIM excluded; "
          f"{manifests[PREFIX + '-comparison.json']['cases']} oracle cases equal the snapshots; every translated "
          f"instruction executed; host and C64 logs pass and match their manifests")


if __name__ == '__main__':
    main()
