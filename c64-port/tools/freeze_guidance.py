#!/usr/bin/env python3
"""Freeze full six-word expectations only after independent assembly equality."""
import hashlib, pathlib
root = pathlib.Path(__file__).resolve().parents[1]
data = (root/'build/guidance-reference.snapshots').read_bytes()
assert data == (root/'build/guidance-native.snapshots').read_bytes()
rows = [list(map(int,r.split())) for r in data.decode().splitlines()]
assert len(rows) == 12856 and all(len(r) == 9 for r in rows)
selected = [r for r in rows if r[0] < 547]
assert len(selected) == 568
s = ('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly by tools/freeze_guidance.py.\n'
     ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
     '#ifndef GUIDANCE_VECTORS_H\n#define GUIDANCE_VECTORS_H\n#include <stdint.h>\n'
     'static const uint16_t guidance_reference_words[568][6] = {\n')
for r in selected: s += '    {'+','.join(str(v) for v in r[3:])+'}, /* %u/%u */\n'%tuple(r[:2])
s += '};\n#endif\n'
(root/'tests/guidance_vectors.h').write_text(s)
print('Frozen 568 original-assembly checkpoints, six complete words each')
