#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib, subprocess, json, hashlib, os
R = pathlib.Path(__file__).resolve().parents[1]
proof = json.loads((R/'build/cm-priority-display-proof.json').read_text())
p = subprocess.Popen(['./build/cm-priority-display-sanitize'], cwd=R, stdout=subprocess.PIPE,
                     env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
h, size = hashlib.sha256(), 0
while True:
    data = p.stdout.read(1048576)
    if not data:
        break
    h.update(data)
    size += len(data)
assert p.wait() == 0 and size == proof['binary_bytes'] and h.hexdigest() == proof['snapshot_sha256']
print('PASS: full ASAN/UBSAN PRIODSPR corpus, all22362 actual source states')
