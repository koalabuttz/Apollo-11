#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Two facts the CM certificates rest on (#25, #969 B13), checked against
the yaYUL listings and ropes. The result goes to build/cm-trig-rope-check/result.json;
record it with tools/evidence.py add cm-trig-rope-check build/cm-trig-rope-check/result.json --tree.

1. CM interpretive SIN/COS runs the LM P66 trig kernel
   (src/cm_powered_trig.c, src/sband_trig.c). That kernel is shared
   faithfully: both programs' SN1 evaluate the same Hastings polynomial
   (the TC POLY table words are equal), and the two words the native code
   writes for the CM, POLISH 03557 and TEM1 (POLYRET) 03572, are the CM
   call's own POLY table address and return address, so every CM SIN/COS
   leaves exactly these values. They are not register debris.
2. The CM SINGLE_PRECISION_SUBROUTINES and MYSUBS proofs were run on the
   indentation-repaired fixture rope (deps/cm-trig-fixture, 2ba31de9...).
   It differs from the byte-original rope (6e3150ae...) only in words
   outside both files' code: the check lists each differing word with its
   listing line and fails if any falls inside either file.

  cm_trig_rope_check.py   (needs build/fixed-data/*/assembly.lst and deps/)
"""
import hashlib
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
LST = {m: ROOT / f'build/fixed-data/{p}/assembly.lst' for m, p in (('lm', 'Luminary099'), ('cm', 'Comanche055'))}
ORIGINAL = ROOT / 'evidence/fixed-data-cm-original.rope.bin'
FIXTURE = ROOT / 'deps/cm-trig-fixture/MAIN.agc.bin'
OUT = ROOT / 'build/cm-trig-rope-check/result.json'
SHA = {'original': '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061',
       'fixture': '2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'}
LINE = re.compile(r'^\d+,\d+: +(?:(\d\d),)?([0-7]{4}) +([0-7]{5})(?: ([0-7]{5}))?(.*)$')
NATIVE = ('src/cm_powered_trig.c', 'src/sband_trig.c')


def listing(m):
    """[(bank or None, address, words, None, rest, include file, line)]."""
    rows, current = [], None
    for text in LST[m].read_text(errors='replace').splitlines():
        include = re.search(r'\$(\S+\.agc)', text)
        if include:
            current = include[1]
        if text.startswith('(End of include-file'):
            current = None
        x = LINE.match(text)
        if x:
            words = [int(x[3], 8)] + ([int(x[4], 8)] if x[4] else [])
            rows.append((x[1], int(x[2], 8), words, None, x[5], current, text))
    return rows


def sn1_poly(m):
    """(call address, table words, return address) of SN1's TC POLY."""
    rows = listing(m)
    start = next(i for i, r in enumerate(rows) if re.search(r'[0-7]{5} +SN1\s', r[6]))
    call = next(i for i in range(start, start + 10) if re.search(r'\sTC +POLY\s', rows[i][6]))
    table, i = list(rows[call + 1][2]), call + 2  # DEC N B-14, then the 2DEC coefficients
    while len(rows[i][2]) == 2:
        table += rows[i][2]
        i += 1
    return rows[call][1], table, rows[call][1] + 1 + len(table)


def main():
    out, bad = {}, []
    (lm_call, lm_table, _), (cm_call, cm_table, cm_ret) = sn1_poly('lm'), sn1_poly('cm')
    out['sn1_poly'] = dict(lm_call=f'{lm_call:04o}', cm_call=f'{cm_call:04o}', same_table=lm_table == cm_table,
                           table=[f'{w:05o}' for w in cm_table], cm_polish=f'{cm_call + 1:05o}',
                           cm_polyret=f'{cm_ret:05o}')
    if lm_table != cm_table:
        bad.append('the LM and CM SN1 POLY tables differ: the shared kernel is not the CM polynomial')
    for path in NATIVE:
        text = (ROOT / path).read_text()
        for name, value in (('POLISH', cm_call + 1), ('POLYRET', cm_ret)):
            if not re.search(rf'\b0{value:o}u?\b', text):
                bad.append(f'{path} does not write the CM {name} {value:05o}')
    original, fixture = ORIGINAL.read_bytes(), FIXTURE.read_bytes()
    for name, data in (('original', original), ('fixture', fixture)):
        if hashlib.sha256(data).hexdigest() != SHA[name]:
            bad.append(f'the {name} rope is not {SHA[name][:8]}')
    rows = {(int(r[0], 8) if r[0] else None, r[1]): r for r in listing('cm')}
    diffs = []
    for i in range(0, min(len(original), len(fixture)), 2):
        if original[i:i + 2] != fixture[i:i + 2]:
            bank, offset = divmod(i // 2, 1024)
            r = rows.get((bank, 0o2000 + offset))
            diffs.append(dict(address=f'{bank:02o},{0o2000 + offset:04o}', original=original[i:i + 2].hex(),
                              fixture=fixture[i:i + 2].hex(), file=r[5] if r else None,
                              listing=r[6].strip() if r else 'no listing line (bank checksum word)'))
    out['rope_differences'] = diffs
    for d in diffs:
        if d['file'] in ('SINGLE_PRECISION_SUBROUTINES.agc', 'MYSUBS.agc'):
            bad.append(f"the fixture rope differs inside {d['file']} at {d['address']}")
    out['ok'] = not bad
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(out, indent=1) + '\n')
    for x in bad:
        print('FAIL:', x)
    print(f"{OUT.relative_to(ROOT)}: {'PASS' if not bad else 'FAIL'}; SN1 POLY table same on both: "
          f"{lm_table == cm_table}; CM POLISH {cm_call + 1:05o} POLYRET {cm_ret:05o}; "
          f"{len(diffs)} rope words differ: {', '.join(d['address'] for d in diffs)}")
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
