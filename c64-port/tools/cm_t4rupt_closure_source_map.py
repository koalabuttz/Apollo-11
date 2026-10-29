#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-file source map of Comanche055 T4RUPT_PROGRAM.agc (milestone
cm-t4rupt-chain).

Every assembled word of the file is listed with its source line, its kind
(instruction or constant) and the native routine milestones whose oracle
run executed it: t4rupt-core (build/t4rupt-core-executed-cm.txt),
cm-t4rupt-imu and the whole-interrupt chain (build/cm-t4rupt-<id>-executed.txt,
written by each milestone's freeze) and cm-t4rupt-optics
(build/cm-t4rupt-optics-executed.txt: its differential rerun under
tools/oracle_trace, `make cm-t4rupt-closure-trace`).
Fails unless every instruction word was executed by at least one of them.
Constants that the code could only run as instructions on a path no
producer reaches are named with t4rupt-core's reasons. Writes
build/cm-t4rupt-closure-source-map.json and build/cm-t4rupt-closure-proof.json.
"""
import hashlib
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from t4rupt_core_source_map import UNREACHED  # noqa: E402

SRC = 'Comanche055/T4RUPT_PROGRAM.agc'
MILESTONES = {
    't4rupt-core': (['src/t4rupt_core.c'], 'build/t4rupt-core-executed-cm.txt', 'evidence/t4rupt-core/t4rupt-core-proof.json'),
    'cm-t4rupt-imu': (['src/cm_t4rupt_imu.c'], 'build/cm-t4rupt-imu-executed.txt', 'evidence/cm-t4rupt-imu/cm-t4rupt-imu-proof.json'),
    'cm-t4rupt-optics': (['src/cm_t4rupt_optics.c'], 'build/cm-t4rupt-optics-executed.txt',
                         'evidence/cm-t4rupt-optics/cm-t4rupt-optics-proof.json'),
    'cm-t4rupt-chain': (['src/t4rupt_core.c', 'src/cm_t4rupt_imu.c', 'src/cm_t4rupt_optics.c'],
                        'build/cm-t4rupt-chain-executed.txt', 'evidence/cm-t4rupt-chain/cm-t4rupt-chain-proof.json'),
}
LINE = re.compile(r'^\d+,(\d+):\s+(?:(\d\d),)?(\d{4})\s+(\d{5})(?: (\d{5}))?\s')
# The checked-in scan's spellings that differ from the pinned Virtual AGC
# listing (docs: notes on upstream transcription; the source marks them).
ERRATA = {'ZOPFIN1': 'ZOPFINI'}
# Instruction words no state reaches, with the reason.
UNREACHED_INSTRUCTIONS = {
    ('10', 0o2110): 'the positive branch of CCS A after MASK BIT15 and RXOR LCHAN of two MASK BIT15 values: A is +0 '
                    'or 140000 (negative), never positive, so TCF CMDSETUP here is never taken'}
DATA = {'OCT', 'DEC', '2DEC', 'CADR', '2CADR', 'GENADR', 'ADRES', 'ECADR', 'BBCON', 'FCADR'}


def physical(bank, addr):
    return int(bank, 8) * 1024 + addr - 0o2000 if bank else 2048 + addr - 0o4000


def main():
    ran = {ms: {int(x) for x in (R / v[1]).read_text().split()} for ms, v in MILESTONES.items()}
    lst = (R / 'build/fixed-data/Comanche055/assembly.lst').read_text(errors='replace').splitlines()
    text = (R.parent / SRC).read_text().splitlines()
    a = next(i for i, l in enumerate(lst) if ': $T4RUPT_PROGRAM.agc' in l)
    b = next(i for i, l in enumerate(lst) if '(End of include-file T4RUPT_PROGRAM.agc' in l)
    out, cursor, label, failures = [], 0, None, []
    for l in lst[a:b]:
        g = LINE.match(l)
        if not g:
            continue
        bank, addr = g.group(2), int(g.group(3), 8)
        listed = [x for x in l.split('\t')[0].split()[3 + (1 if g.group(5) else 0):] if not re.fullmatch(r'B[-+]?\d+', x)]

        def same(k):
            lab = not text[k][:1].isspace()
            return [ERRATA.get(x, x) for x in text[k].split('#')[0].split()[lab:]
                    if not re.fullmatch(r'B[-+]?\d+', x)] == listed[lab:]
        k = next((k for k in range(cursor, len(text)) if text[k].strip()
                  and not text[k].lstrip().startswith('#') and same(k)), None)
        assert k is not None, (listed, l)
        cursor = k + 1
        source = text[k].split('#')[0].split()
        if not text[k][:1].isspace():
            label = source[0]
        ws = [g.group(4)] + ([g.group(5)] if g.group(5) else [])
        kind = 'constant' if DATA & set(source) else 'instruction'
        phys = {physical(bank, addr + i) for i in range(len(ws))}
        owners = sorted(ms for ms, s in ran.items() if s & phys)
        entry = {'line': k + 1, 'address': f'{bank},{addr:04o}' if bank else f'FF,{addr:04o}', 'label': label,
                 'source': ' '.join(source), 'words': ws, 'kind': kind, 'executed_by': owners,
                 'native': sorted({p for ms in owners for p in MILESTONES[ms][0]})}
        why = UNREACHED.get(addr) if bank else None
        why = why or (UNREACHED['OCT37737'] if 'OCT37737' in source and kind == 'constant' and bank else None)
        if why and not owners:
            entry['unreached_as_instruction'] = why
        if kind == 'instruction' and not owners and (bank, addr) in UNREACHED_INSTRUCTIONS:
            entry['unreached_instruction'] = UNREACHED_INSTRUCTIONS[(bank, addr)]
        elif kind == 'instruction' and not owners:
            failures.append(f'{SRC}:{k + 1} {entry["address"]} {label}: {" ".join(source)}')
        out.append(entry)
    instr = [e for e in out if e['kind'] == 'instruction']
    doc = {'source_path': SRC, 'scope': 'whole file',
           'source_sha256': hashlib.sha256((R.parent / SRC).read_bytes()).hexdigest(),
           'rope_sha256': hashlib.sha256((R / 'build/fixed-data/Comanche055/MAIN.agc.bin').read_bytes()).hexdigest(),
           'milestones': {ms: v[0] for ms, v in MILESTONES.items()},
           'source_words': sum(len(e['words']) for e in out), 'source_lines_with_words': len(out),
           'instruction_lines': len(instr),
           'instruction_lines_executed_by_oracle': sum(bool(e['executed_by']) for e in instr),
           'unreached_instruction_lines': sum('unreached_instruction' in e for e in instr),
           'line_map': out}
    (R / 'build/cm-t4rupt-closure-source-map.json').write_text(json.dumps(doc, indent=1) + '\n')
    proofs = {}
    for ms, v in MILESTONES.items():
        p = json.loads((R / v[2]).read_text())
        part = p.get('cm', p)
        rope = part['rope_sha256']
        if isinstance(rope, list):  # cm-t4rupt-optics: one per mission of 'missions'
            rope = rope[part['missions'].index('Comanche055')]
        proofs[ms] = {'proof': v[2], 'sha256': hashlib.sha256((R / v[2]).read_bytes()).hexdigest(),
                      'cases': part['cases'], 'rope_sha256': rope,
                      'snapshot_sha256': part.get('snapshot_sha256') or part['oracle_sha256']}
    proof = {'source_path': SRC, 'milestones': proofs,
             'boundary': 'each milestone compares its routines entry by entry with the original program in yaAGC; '
                         'cm-t4rupt-chain runs whole T4RUPT interrupts through agc_transfer_run over all three '
                         'routine tables and compares them with the original from T4RUPT to the first exit from the file',
             'instruction_lines': len(instr),
             'instruction_lines_executed_by_oracle': doc['instruction_lines_executed_by_oracle'],
             'unreached_instruction_lines': doc['unreached_instruction_lines']}
    (R / 'build/cm-t4rupt-closure-proof.json').write_text(json.dumps(proof, indent=1) + '\n')
    assert not failures, 'instructions no oracle run executed:\n' + '\n'.join(failures)
    print(f"PASS: {len(out)} assembled lines of {SRC} ({doc['source_words']} words, {len(instr)} instruction lines); "
          f"every instruction executed by a native routine milestone's oracle run but "
          f"{doc['unreached_instruction_lines']} named unreachable")


if __name__ == '__main__':
    main()
