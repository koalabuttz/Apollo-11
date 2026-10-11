#!/usr/bin/env python3
"""Freeze 128 complete original-assembly states, all six upper flag bits."""
import pathlib
root=pathlib.Path(__file__).resolve().parents[1]
rows=[list(map(int,line.split())) for line in (root/'build/phase-reference.snapshots').read_text().splitlines()]
assert len(rows)==49152 and all(len(r)==58 for r in rows)
selected=[rows[(n//2)*768+n%2+2*((n//2)%6)] for n in range(128)]
lines=['/* SPDX-License-Identifier: MIT',' * Frozen unchanged original PHASCHNG / 2PHSCHNG assembly snapshots. */',
 '#ifndef PHASE_VECTORS_H','#define PHASE_VECTORS_H','#include "phase_cases.h"',
 '#define PH_TARGET_COUNT 128','static const agc_word phase_expected[PH_TARGET_COUNT][PH_WORD_COUNT] = {']
lines += [' {'+','.join(str(w) for w in r[2:])+'},' for r in selected]
lines += ['};','#endif','']
(root/'tests/phase_vectors.h').write_text('\n'.join(lines))
print('Frozen 128 complete original assembly states (56 words each)')
