#!/usr/bin/env python3
"""Freeze 128 complete original-assembly abort cleanup states."""
import pathlib
root=pathlib.Path(__file__).resolve().parents[1]
rows=[list(map(int,line.split())) for line in (root/'build/recovery-reference.snapshots').read_text().splitlines()]
assert len(rows)==49152 and all(len(r)==49 for r in rows)
selected=[rows[n*511 if n<64 else 32768+(n%4)*4096+(n//4-16)*237] for n in range(128)]
lines=['/* SPDX-License-Identifier: MIT',' * Frozen unchanged original abort cleanup assembly snapshots. */',
 '#ifndef RECOVERY_VECTORS_H','#define RECOVERY_VECTORS_H','#include "recovery_cases.h"',
 '#define RECOVERY_TARGET_COUNT 128','static const agc_word recovery_expected[RECOVERY_TARGET_COUNT][RECOVERY_WORD_COUNT] = {']
lines += [' {'+','.join(str(w) for w in r[2:])+'},' for r in selected]
lines += ['};','#endif','']
(root/'tests/recovery_vectors.h').write_text('\n'.join(lines))
print('Frozen 128 complete original assembly states (47 words each)')
