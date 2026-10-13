#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source map for Luminary099 T6-RUPT_PROGRAMS.agc (milestone lm-t6rupt).

Lists every assembled word of the file from the Luminary099 listing with its
source line, the native function that translates it (src/lm_t6rupt.c) and
whether the original program executed it during the lm-t6rupt oracle run
(build/lm-t6rupt-executed.txt, written by tools/lm_t6rupt_freeze.py).
Fails unless every instruction word was executed, except T6JOBCHK +3 (TC
T6JOBCHK +3, reached only by a negative nonzero TIME6, which nothing
produces; the native routines refuse it). Writes
build/lm-t6rupt-source-map.json.
"""
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
SRC = 'Luminary099/T6-RUPT_PROGRAMS.agc'
LST = R / 'build/fixed-data/Luminary099/assembly.lst'
ROPE = R / 'build/fixed-data/Luminary099/MAIN.agc.bin'
NATIVE = ['src/lm_t6rupt.c', 'include/lm_t6rupt_routines.h']
LINE = re.compile(r'^\d+,(\d+): (..),?(\d{4})?\s*(\d{4})?\s+(\d{5})(?: (\d{5}))?\s+(\S*)\s+(\S+)\s*([^\t]*)')
# Native function per label range (source labels in order of address).
OWNERS = [('T6JOBCHK', 't6jobchk (CCS TIME6, TC Q / TC CCSHOLE)'), ('T6JOB', 't6jobchk (T6JOB)'),
          ('ENABLET6', 't6jobchk (ENABLET6)'), ('GOCH56', 't6jobchk (GOCH56: INDEX L; TCF WRITEP -1)'),
          ('-1P', 'fire (CA NEXTP)'), ('WRITEP', 'fire / writex (WRITEP)'), ('-1U', 'fire (CA NEXTU)'),
          ('WRITEU', 'write5 (WRITEU)'), ('-1V', 'fire (CA NEXTV)'), ('WRITEV', 'write5 (WRITEV)'),
          ('00314OCT', 'OCT00314 constant (write5)'), ('DOT6RUPT', 'dot6rupt')]


def main():
    text = (R.parent / SRC).read_text().splitlines()
    lines = LST.read_text(errors='replace').splitlines()
    a = next(i for i, l in enumerate(lines) if '$T6-RUPT_PROGRAMS.agc' in l)
    b = next(i for i, l in enumerate(lines) if '(End of include-file T6-RUPT_PROGRAMS.agc' in l)
    executed = {int(x) for x in (R / 'build/lm-t6rupt-executed.txt').read_text().split()}
    out, owner, missing = [], None, []
    pending = {'CA NEXTP': '-1P', 'CA NEXTU': '-1U', 'CA NEXTV': '-1V'}
    for l in lines[a:b]:
        m = LINE.match(l.split('\t')[0])
        if not m or not m.group(5):
            continue
        bank, off = (m.group(2).strip() or None), m.group(3) or m.group(4)
        if not off:
            continue
        off = int(off, 8)
        phys = (int(bank, 8) * 1024 + (off & 0o1777)) if bank else (3 if off >= 0o6000 else 2) * 1024 + (off & 0o1777)
        label, op, operand = m.group(7), m.group(8), m.group(9).strip()
        src = ' '.join(x for x in (label, op, operand) if x)
        key = f'{op} {operand}'.strip()
        if label:
            owner = dict(OWNERS).get(label, owner)
        elif key in pending:
            owner = dict(OWNERS)[pending[key]]
        n = int(m.group(1)) - 5
        assert text[n - 1].split()[:2] == src.split()[:2] or label == '', (n, text[n - 1], src)
        kind = 'constant' if op == 'OCT' else 'instruction'
        words = [m.group(5)] + ([m.group(6)] if m.group(6) else [])
        ran = phys in executed
        refused = src == 'TC T6JOBCHK +3'
        if kind == 'instruction' and not ran and not refused:
            missing.append(n)
        out.append({'line': n, 'address': (f'{bank},' if bank else 'FF,') + f'{off:04o}', 'source': src,
                    'words': words, 'kind': kind, 'owner': 'lm-t6rupt', 'native': f'src/lm_t6rupt.c ({owner})',
                    'executed_by_oracle': ran, **({'refused': 'negative nonzero TIME6 has no producer; '
                                                   'T6JOBCHK refuses it unchanged'} if refused else {})})
    assert not missing, f'instructions never executed by the oracle: lines {missing}'
    doc = {'source_path': SRC, 'source_sha256': hashlib.sha256((R.parent / SRC).read_bytes()).hexdigest(),
           'rope_sha256': hashlib.sha256(ROPE.read_bytes()).hexdigest(),
           'native': {p: hashlib.sha256((R / p).read_bytes()).hexdigest() for p in NATIVE},
           'source_words': sum(len(e['words']) for e in out), 'source_lines_with_words': len(out),
           'instructions_executed_by_oracle': sum(e['executed_by_oracle'] for e in out if e['kind'] == 'instruction'),
           'line_map': out}
    (R / 'build/lm-t6rupt-source-map.json').write_text(json.dumps(doc, indent=1) + '\n')
    print(f"PASS: {len(out)} assembled lines of {SRC}; every instruction but the refused T6JOBCHK +3 executed by the original program "
          f"in the lm-t6rupt oracle run and translated in src/lm_t6rupt.c")


if __name__ == '__main__':
    main()
