#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze every final PDVL GDT/2 state word after independent equality."""
import hashlib
from pathlib import Path
from frozen_words import c_header, encode_rows

root = Path(__file__).resolve().parents[1]
data = (root / 'build/gravity-reference.snapshots').read_bytes()
assert data == (root / 'build/gravity-native.snapshots').read_bytes()
rows = [list(map(int, row.split())) for row in data.decode().splitlines()]
assert len(rows) == 5218 and all(len(row) == 102 for row in rows)
selected = [row for row in rows if row[0] < 147 and row[2] == 1]
assert len(selected) == 156
packed = encode_rows([row[3:] for row in selected])
text = ('/* SPDX-License-Identifier: MIT\n'
        ' * GENERATED from original assembly using pinned hardware DV by tools/freeze_gravity.py.\n'
        ' * Full snapshot SHA256: ' + hashlib.sha256(data).hexdigest() + '\n'
        ' * Lossless storage: every decoded uint16_t is compared exactly.\n'
        ' */\n#ifndef GRAVITY_VECTORS_H\n#define GRAVITY_VECTORS_H\n')
text += c_header(packed, 'gravity')
text += '#endif\n'
(root / 'tests/gravity_vectors.h').write_text(text)
print('Frozen 156 complete final 99-word PDVL states: '
      f'{156 * 99 * 2} -> {packed.storage_bytes} bytes; exact lossless roundtrip')
