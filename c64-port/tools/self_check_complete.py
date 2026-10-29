#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Completion certificate records for AGC_BLOCK_TWO_SELF-CHECK (both missions).

One milestone, self-check, translates the file in Luminary099 and Comanche055.
For each mission this tool writes one record per certificate category to
evidence/{lm,cm}-self-check-complete/, each a small manifest naming the
milestone records it rests on with their sha256:

  <id>-source-map.json   docs/SELF-CHECK.md's source map and the word check:
      every assembled word of the file is translated (an instruction with its
      native label in src/self_check_body.inc, or data compiled from the
      listing), with per-word detail in <id>-line-map.json
  <id>-comparison.json   the yaAGC proof record; the oracle is rerun on the
      unmodified rope with the coverage trace (tools/cert_trace.h), random-state
      and whole-rope cases, and must print exactly the frozen snapshots; every
      instruction must be executed by those runs but the two words no state
      reaches (tools/self_check_freeze.py UNREACHABLE)
  <id>-host.json         the host and sanitizer log
  <id>-c64.json          the VICE log, with the PRG sha256s it names
  <id>-c64-cover.json    the mission's c64_branch_outcomes records (random-state
      and whole-rope, evidence/self-check/self-check-<m>[-rope]-c64-cover.json,
      tools/a2_c64_rows.py): no host branch arc the C64 rows miss, every
      entry slot run on the C64, every linked source unchanged since the cover

It fails on any hash mismatch, untranslated word, snapshot difference,
unexecuted instruction or failed check.

  python3 tools/self_check_complete.py           # rerun the oracle, write records
  python3 tools/self_check_complete.py --check   # also fail if a record would change
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
import a2_c64_rows  # noqa: E402

MILESTONE = 'self-check'
# mission index, name, source file, certificate id, words below Comanche055's
# (the listing's file name; the Luminary099 tree spells it SELF_CHECK, REPO_SOURCE)
MISSIONS = [(0, 'Luminary099', 'AGC_BLOCK_TWO_SELF-CHECK.agc', 'lm-self-check-complete', 2),
            (1, 'Comanche055', 'AGC_BLOCK_TWO_SELF-CHECK.agc', 'cm-self-check-complete', 0)]
REPO_SOURCE = {0: 'AGC_BLOCK_TWO_SELF_CHECK.agc', 1: 'AGC_BLOCK_TWO_SELF-CHECK.agc'}
DATA_OPS = {'OCT', 'DEC', '2DEC', 'CADR', '2CADR', 'GENADR', 'ADRES', 'ECADR', 'FCADR', '2FCADR', 'BBCON', 'BBCON*',
            'VN', 'MM', 'OCTAL', '2OCT', 'REMADR', 'NV'}
BODY = 'src/self_check_body.inc'
NATIVE = {0: 'src/lm_self_check.c', 1: 'src/cm_self_check.c'}
IDLE = {0: 'src/lm_self_check_idle.c', 1: 'src/cm_self_check_idle.c'}
OPERANDS = {0: 'src/lm_self_check_operands.inc', 1: 'src/cm_self_check_operands.inc'}


def sha(path):
    return hashlib.sha256((R / path).read_bytes()).hexdigest()


def freeze_module():
    spec = importlib.util.spec_from_file_location('self_check_freeze', R / 'tools/self_check_freeze.py')
    freeze = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(freeze)
    return freeze


def run_oracle(freeze, mission, rope):
    """The traced oracle's two snapshots' sha256 and the fixed words it executed."""
    trace = R / 'build/self-check-complete.trace'
    exe = R / 'build/self-check-complete-reference'
    subprocess.run(['cc', '-std=c99', '-O2', f'-DCERT_TRACE={trace}', '-Iinclude', '-Itests', '-Isrc', '-Itools',
                    '-I' + str(freeze.E), 'tools/self_check_reference.c', *freeze.CORE, '-o', str(exe)], cwd=R, check=True)
    shas, seen = [], set()
    for mode in ([], ['rope']):
        out = subprocess.run([str(exe), str(mission), str(rope), *mode], cwd=R, check=True, capture_output=True).stdout
        shas.append(hashlib.sha256(out).hexdigest())
        seen |= {int(x) for x in trace.read_text().split()}
    return shas, seen


def log_record(name, kind, errors):
    log = f'evidence/{name}/{name}-{kind}.log'
    text = (R / log).read_text()
    found = re.findall(r'(\d+) checks, (\d+) failures', text)
    passes = [l for l in text.splitlines() if l.startswith('PASS')]
    ok = bool(found or passes) and not any(int(f) for _, f in found)
    record = {'record': log, 'sha256': sha(log), 'pass_lines': passes,
              'checks': [int(c) for c, _ in found], 'failures': sum(int(f) for _, f in found), 'result': 'PASS' if ok else 'FAIL'}
    if kind == 'vice':
        record['prg_sha256'] = a2_c64_rows.prg_hashes(text)
        if not record['prg_sha256']:
            errors.append(f'{log}: names no PRG sha256')
    if json.loads((R / f'evidence/{name}/SHA256.json').read_text())['files'][f'{name}-{kind}.log']['sha256'] != record['sha256']:
        errors.append(f'{log}: sha256 differs from evidence/{name}/SHA256.json')
    if not ok:
        errors.append(f'{log}: a check failed or no checks found')
    return record


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    freeze = freeze_module()
    proof_path = f'evidence/{MILESTONE}/{MILESTONE}-proof.json'
    proof = json.loads((R / proof_path).read_text())
    errors, stale, summary = [], [], []
    if json.loads((R / f'evidence/{MILESTONE}/SHA256.json').read_text())['files'][f'{MILESTONE}-proof.json']['sha256'] != sha(proof_path):
        errors.append(f'{proof_path}: sha256 differs from evidence/{MILESTONE}/SHA256.json')
    labels = sorted(int(m, 8) for m in re.findall(r'^L([0-7]+):', (R / BODY).read_text(), re.M))
    host = log_record(MILESTONE, 'host', errors)
    c64 = log_record(MILESTONE, 'vice', errors)
    outputs = {}
    for mission, name, source, cert, d in MISSIONS:
        base = R / 'build/fixed-data' / name
        rope = base / 'MAIN.agc.bin'
        rope_sha = hashlib.sha256(rope.read_bytes()).hexdigest()
        record = proof['missions'][name]
        if record['rope_sha256'] != rope_sha:
            errors.append(f'{name}: proof rope differs')
        shas, seen = run_oracle(freeze, mission, rope)
        frozen = [record['snapshot_sha256'], record['snapshot_rope_sha256']]
        if shas != frozen:
            errors.append(f'{name}: traced oracle output {shas} differs from the proof snapshots {frozen}')
        rows = [r for r in listing_lines((base / 'assembly.lst').read_text(errors='replace')) if r[0] == source and r[3]]
        line_map, unexecuted, untranslated, prefix = [], [], [], None
        for path, line, address, words, label, op, opnd in rows:
            bank, _, offset = address.partition(',')
            z = int(offset, 8)
            ph = int(bank, 8) * 1024 + z - 0o2000
            cm = z + d
            entry = {'line': line, 'address': address, 'source': ' '.join(x for x in (label, op, opnd) if x),
                     'words': [f'{w:05o}' for w in words]}
            if bank != '43' or not 0o3234 <= cm <= 0o3725:
                untranslated.append(address)
                continue
            if op in DATA_OPS:
                entry.update(kind='data', native=f'compiled from the listing ({OPERANDS[mission]}, checked against the rope)', executed_by_oracle=None)
                prefix = None
            else:
                at = [x for x in labels if x <= cm]
                entry.update(kind='instruction', native=f'{BODY} (L{at[-1]:o}, Comanche055 numbering)' if at else BODY)
                executed = ph in seen or (prefix is not None and prefix in seen)
                if not executed and cm in freeze.UNREACHABLE:
                    entry['executed_by_oracle'] = False
                    entry['unreachable'] = freeze.UNREACHABLE[cm]
                else:
                    entry['executed_by_oracle'] = executed
                    if not executed:
                        unexecuted.append(f"{address} {entry['source']}")
                prefix = ph if op in ('EXTEND', 'INDEX', 'NDX') and (prefix is None or prefix not in seen) else None if op not in ('EXTEND', 'INDEX', 'NDX') else prefix
            line_map.append(entry)
        total = sum(len(r[3]) for r in rows)
        if untranslated:
            errors.append(f'{name}: words outside bank 43 3234-3725: {untranslated[:8]}')
        if unexecuted:
            errors.append(f'{name}: instructions the oracle runs never execute: {unexecuted}')
        sourcefile = R / 'deps/virtualagc' / name / source
        instructions = sum(1 for e in line_map if e['kind'] == 'instruction')
        outputs[cert] = {
            f'{cert}-line-map.json': {'source_path': f'{name}/{REPO_SOURCE[mission]}', 'rope_sha256': rope_sha, 'line_map': line_map},
            f'{cert}-source-map.json': {
                'category': 'source_to_native_map', 'source_path': f'{name}/{REPO_SOURCE[mission]}',
                'source_sha256': hashlib.sha256(sourcefile.read_bytes()).hexdigest(), 'rope_sha256': rope_sha,
                'aggregates': {MILESTONE: {'record': 'docs/SELF-CHECK.md', 'sha256': sha('docs/SELF-CHECK.md'),
                                           'native': [NATIVE[mission], IDLE[mission], BODY, OPERANDS[mission], 'src/rope_image_host.c', 'platform/c64/rope_image_reu.c'],
                                           'native_sha256': {p: sha(p) for p in (NATIVE[mission], IDLE[mission], BODY, OPERANDS[mission])},
                                           'owns': ['43,3232-3723' if mission == 0 else '43,3234-3725']}},
                'word_check': {'source_words': total, 'translated_words': total - len(untranslated), 'excluded_words': 0,
                               'result': 'PASS' if not untranslated else 'FAIL'},
                'execution_check': {'instructions': instructions, 'not_executed': unexecuted,
                                    'unreachable': {f'{w - d:04o}': why for w, why in freeze.UNREACHABLE.items()},
                                    'result': 'PASS' if not unexecuted else 'FAIL'},
                'line_map': {'record': f'evidence/{cert}/{cert}-line-map.json'}},
            f'{cert}-comparison.json': {
                'category': 'independent_assembly_comparison', 'source_path': f'{name}/{REPO_SOURCE[mission]}', 'rope_sha256': rope_sha,
                'method': 'the milestone\'s random-state and whole-rope cases through the native routines and through the '
                          'unmodified rope in yaAGC (host only); tools/self_check_freeze.py checked the outputs byte-identical '
                          'and froze the snapshots; this tool reran the oracle with the coverage trace and compared',
                'aggregates': {MILESTONE: {'record': proof_path, 'sha256': sha(proof_path), 'cases': record['cases'],
                                           'cases_rope': record['cases_rope'], 'snapshot_sha256': frozen, 'rerun_snapshot_sha256': shas,
                                           'result': 'PASS' if shas == frozen else 'FAIL'}},
                'result': 'PASS' if shas == frozen else 'FAIL'},
            f'{cert}-host.json': {'category': 'host_and_sanitizer_results', 'aggregates': {MILESTONE: host}, 'result': host['result']},
            f'{cert}-c64.json': {'category': 'c64_execution_results', 'aggregates': {MILESTONE: c64}, 'result': c64['result']},
            f'{cert}-c64-cover.json': a2_c64_rows.cover_aggregate(
                {f'{MILESTONE}-{cert[:2]}{tag}': f'evidence/{MILESTONE}/{MILESTONE}-{cert[:2]}{tag}-c64-cover.json'
                 for tag in ('', '-rope')}, errors)}
        summary.append(f'{name}: {total} words translated, {instructions} instructions, all executed but '
                       f'{len(freeze.UNREACHABLE)} unreachable; {record["cases"]} + {record["cases_rope"]} oracle cases equal the snapshots')
    if errors:
        raise SystemExit('FAIL:\n  ' + '\n  '.join(errors))
    for cert, manifests in outputs.items():
        out = R / 'evidence' / cert
        out.mkdir(parents=True, exist_ok=True)
        for name, doc in manifests.items():
            text = json.dumps(doc, indent=1) + '\n'
            path = out / name
            if args.check and (not path.exists() or path.read_text() != text):
                stale.append(f'{cert}/{name}')
            elif not args.check:
                path.write_text(text)
    if stale:
        raise SystemExit(f'FAIL: stale records {stale}; run tools/self_check_complete.py')
    print('PASS: ' + '; '.join(summary) + '; host and C64 logs pass and match their manifests')


if __name__ == '__main__':
    main()
