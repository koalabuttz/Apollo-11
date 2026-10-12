#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Certificate evidence for Luminary099 FRESH_START_AND_RESTART.agc.

The file is translated by three milestones: lm-fresh-start (bank 05 and
fixed-fixed), lm-v37 (bank 04) and lm-stateup (bank 13, interpretive). This
tool reruns the three host-only yaAGC oracles on the unmodified rope with
the coverage trace (tools/cert_trace.h), checks each run prints exactly the
snapshot its milestone froze, and writes to evidence/lm-fresh-start-cert/:

  lm-fresh-start-source-map.json   every assembled word of the file: kind,
                                   owning milestone, native location, and
                                   whether the oracle runs executed it
  lm-fresh-start-comparison.json   the three oracle comparisons, combined

  python3 tools/lm_fresh_start_cert.py
"""
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

OUT = R / 'evidence/lm-fresh-start-cert'
MISSION = 'Luminary099'
SOURCE = 'FRESH_START_AND_RESTART.agc'
# milestone -> (freeze module, reference, native file)
MILESTONES = {
    'lm-fresh-start': ('lm_fresh_start_freeze', 'tools/lm_fresh_start_reference.c', 'src/lm_fresh_start.c'),
    'lm-v37': ('lm_v37_freeze', 'tools/lm_v37_reference.c', 'src/lm_v37.c'),
    'lm-stateup': ('lm_stateup_freeze', 'tools/lm_stateup_reference.c', 'src/lm_stateup.c'),
}
DATA_OPS = {'OCT', 'DEC', '2DEC', 'CADR', '2CADR', 'GENADR', 'ADRES', 'ECADR', 'FCADR', '2FCADR', 'BBCON',
            'VN', 'MM', 'OCTAL', '2OCT', 'REMADR', 'DNCHAN', 'DNPTR', '1DNADR', '2DNADR', '3DNADR',
            '4DNADR', '5DNADR', '6DNADR', 'NV'}
# Written only if STARTSW is patched to TCF STARTSIM; the rope has TCF SKIPSIM.
PATCH_ONLY = range(0o2452, 0o2456)
# R00's interpretive CALL INTSTALL; EXIT (bank 04, after TC INTPRET) is lm-stateup's.
R00_INTERPRETIVE = range(0o2144, 0o2147)


def owner(address):
    bank, _, offset = address.partition(',')
    if not offset:
        return 'lm-fresh-start'
    if bank == '04' and int(offset, 8) in R00_INTERPRETIVE:
        return 'lm-stateup'
    return {'05': 'lm-fresh-start', '04': 'lm-v37', '13': 'lm-stateup'}[bank]


def physical(address):
    bank, _, offset = address.partition(',')
    if not offset:
        z = int(bank, 8)
        return (z >> 10) * 1024 + (z & 0o1777)
    return int(bank, 8) * 1024 + int(offset, 8) - 0o2000


def labels(native):
    text = (R / native).read_text()
    return sorted(int(m, 8) for m in re.findall(r'^L([0-7]+):', text, re.M))


def run_oracle(name):
    module, reference, _ = MILESTONES[name]
    spec = importlib.util.spec_from_file_location(module, R / 'tools' / f'{module}.py')
    freeze = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(freeze)
    trace = R / f'build/{name}-cert.trace'
    exe = R / f'build/{name}-cert-reference'
    subprocess.run(['cc', '-std=c99', '-O2', f'-DCERT_TRACE={trace}', '-Iinclude', '-Itests', '-Isrc', '-Itools',
                    '-I' + str(freeze.E), reference, *freeze.CORE, '-o', str(exe)], cwd=R, check=True)
    out = subprocess.run([str(exe), str(freeze.ROPE)], cwd=R, check=True, capture_output=True).stdout
    proof_path = R / f'evidence/{name}/{name}-proof.json'
    proof = json.loads(proof_path.read_text())
    got = hashlib.sha256(out).hexdigest()
    if got != proof['snapshot_sha256']:
        raise SystemExit(f'{name}: traced oracle output {got} differs from the frozen snapshot')
    seen = {int(x) for x in trace.read_text().split()}
    return proof, proof_path, got, seen


def main():
    base = R / 'build/fixed-data' / MISSION
    listing = (base / 'assembly.lst').read_text(errors='replace')
    rope = base / 'MAIN.agc.bin'
    rows = [r for r in listing_lines(listing) if r[0] == SOURCE and r[3]]
    suites, seen = {}, set()
    for name in MILESTONES:
        proof, proof_path, got, trace = run_oracle(name)
        seen |= trace
        suites[name] = {'proof': str(proof_path.relative_to(R)),
                        'proof_sha256': hashlib.sha256(proof_path.read_bytes()).hexdigest(),
                        'cases': proof['cases'], 'differing': 0,
                        'oracle_snapshot_sha256': got, 'native_snapshot_sha256': proof['snapshot_sha256'],
                        'entries': proof['entries'], 'boundary': proof['boundary']}
    native_labels = {n: labels(MILESTONES[n][2]) for n in ('lm-fresh-start', 'lm-v37')}
    stateup = sorted(int(m, 8) for m in re.findall(r'case (0[0-7]+)u:', (R / 'src/lm_stateup.c').read_text()))
    line_map, unexecuted = [], []
    prefix = None   # the EXTEND/INDEX word whose instruction this one completes
    for path, line, address, words, label, op, opnd in rows:
        who = owner(address)
        z = int(address.partition(',')[2] or address, 8)
        ph = physical(address)
        entry = {'line': line, 'address': address, 'source': ' '.join(x for x in (label, op, opnd) if x),
                 'words': [f'{w:05o}' for w in words], 'owner': who}
        if who == 'lm-stateup':
            bank = address.partition(',')[0]
            pair = max(p for p in stateup if p <= z and (p >= 0o2144) == (bank == '04'))
            entry['kind'] = 'interpretive pair' if pair == z else 'interpretive operand'
            entry['native'] = f'src/lm_stateup.c (case 0{pair:o})'
            entry['executed_by_oracle'] = physical(f'{bank},{pair:04o}') in seen
        elif op in DATA_OPS or (op == '' and opnd == ''):
            entry['kind'] = 'data'
            entry['native'] = 'compiled operand/constant (tools/file_operands_generate.py, checked against the rope)'
            entry['executed_by_oracle'] = None
        else:
            native = MILESTONES[who][2]
            at = [x for x in native_labels[who] if x <= z]
            entry['kind'] = 'instruction'
            entry['native'] = f'{native} (L{at[-1]:o})' if at else native
            # yaAGC runs the instruction after EXTEND or INDEX in the same
            # step (ExtraCode/IndexValue set), so the trace sees its prefix.
            entry['executed_by_oracle'] = ph in seen or (prefix is not None and prefix in seen)
            if who == 'lm-fresh-start' and address.startswith('05,') and z in PATCH_ONLY:
                entry['native'] = 'not translated: STARTSIM runs only if STARTSW is patched'
                entry['kind'] = 'patch-only instruction'
        if entry['kind'] == 'instruction' and op in ('EXTEND', 'INDEX'):
            prefix = ph if prefix is None or prefix not in seen else prefix
        else:
            prefix = None
        if entry['executed_by_oracle'] is False:
            unexecuted.append(entry)
        line_map.append(entry)
    source = R / 'deps/virtualagc' / MISSION / SOURCE
    natives = sorted({MILESTONES[n][2] for n in MILESTONES} | {'src/lm_fresh_start_operands.inc', 'src/lm_v37_operands.inc',
                                                              'src/lm_interp_steps.c', 'src/lm_interp_orders.c'})
    smap = {'source_path': f'{MISSION}/{SOURCE}',
            'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest() if source.exists() else None,
            'rope_sha256': hashlib.sha256(rope.read_bytes()).hexdigest(),
            'native': {n: hashlib.sha256((R / n).read_bytes()).hexdigest() for n in natives},
            'milestones': {n: f'docs/milestones/{n}.md' for n in MILESTONES},
            'source_words': sum(len(r[3]) for r in rows), 'source_lines_with_words': len(rows),
            'instructions': sum(1 for e in line_map if e['kind'] in ('instruction', 'interpretive pair')),
            'instructions_not_executed': [f"{e['address']} {e['source']}" for e in unexecuted],
            'line_map': line_map}
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'lm-fresh-start-source-map.json').write_text(json.dumps(smap, indent=1) + '\n')
    comparison = {'method': 'each milestone\'s cases run through the native routines and through the unmodified '
                            'Luminary099 rope in yaAGC (host only, tools/<milestone>_reference.c rebuilt with the '
                            'coverage trace); the oracle output must equal the snapshot the milestone froze, which '
                            'its freeze script checked byte-identical to the native output',
                  'source_path': f'{MISSION}/{SOURCE}', 'rope_sha256': smap['rope_sha256'], 'suites': suites,
                  'cases': sum(s['cases'] for s in suites.values()), 'differing': 0}
    (OUT / 'lm-fresh-start-comparison.json').write_text(json.dumps(comparison, indent=1) + '\n')
    print(f"PASS: {comparison['cases']} cases in 3 suites equal the frozen snapshots; "
          f"{smap['instructions']} instructions, {len(unexecuted)} not executed")
    for e in unexecuted:
        print('  not executed:', e['address'], e['source'], '->', e['native'])


if __name__ == '__main__':
    main()
