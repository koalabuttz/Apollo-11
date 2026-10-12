#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run a Lane C suite through the native routines and through the unchanged
original program in Virtual AGC, and require identical complete results.

  python3 tools/lanec/diff.py SUITE [--native build/lanec-native-sanitize]

Writes build/lanec-SUITE-{native,oracle}.txt (complete states, one case per
line), build/lanec-SUITE-{native,oracle}.crc (per-case CRC-32, the compact
evidence form, same hash as the C64 diagnostic) and build/lanec-SUITE-diff.json. The cases are deterministic,
so the full streams are reproducible from the sources at any time.
"""
import argparse, hashlib, json, pathlib, subprocess, sys, zlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROPES = {'lm': ('deps/lm-powered-fixture/MAIN.agc.bin',
                '1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e'),
         'cm': ('deps/cm-powered-fixture/MAIN.agc.bin',
                '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061')}

p = argparse.ArgumentParser()
p.add_argument('suite')
p.add_argument('--native', default='build/lanec-native')
p.add_argument('--oracle', default='build/lanec-oracle')
a = p.parse_args()
for path, digest in ROPES.values():
    if hashlib.sha256((ROOT / path).read_bytes()).hexdigest() != digest:
        sys.exit(f'FAIL: {path} is not the pinned rope')
subprocess.run([sys.executable, 'tools/lanec/symbols.py', '--check'], cwd=ROOT, check=True)
native = subprocess.run([a.native, a.suite], cwd=ROOT, capture_output=True, check=True).stdout
oracle = subprocess.run([a.oracle, a.suite, ROPES['lm'][0], ROPES['cm'][0]], cwd=ROOT,
                        capture_output=True, check=True).stdout
rows_n, rows_o = native.splitlines(), oracle.splitlines()
bad = [i for i, (x, y) in enumerate(zip(rows_n, rows_o)) if x != y]
for i in bad[:10]:
    xs, ys = rows_n[i].split(), rows_o[i].split()
    print('DIFF case', xs[0], [(oct(k - 1) if k <= 2048 else f'io{k - 2049:o}', xs[k], ys[k])
                               for k in range(1, len(xs)) if xs[k] != ys[k]][:12])
tag = pathlib.Path(a.native).name.replace('lanec-native', '') or ''
for name, data in (('native' + tag, native), ('oracle', oracle)):
    (ROOT / f'build/lanec-{a.suite}-{name}.txt').write_bytes(data)
    (ROOT / f'build/lanec-{a.suite}-{name}.crc').write_text(''.join(
        f"{row.split()[0].decode()} {zlib.crc32(b''.join(int(x).to_bytes(2, 'little') for x in row.split()[1:])):08x}\n"
        for row in data.splitlines()))
summary = dict(suite=a.suite, cases=len(rows_o), differing=len(bad) + abs(len(rows_n) - len(rows_o)),
               native_sha256=hashlib.sha256(native).hexdigest(), oracle_sha256=hashlib.sha256(oracle).hexdigest())
(ROOT / f'build/lanec-{a.suite}-diff{tag}.json').write_text(json.dumps(summary, indent=2) + '\n')
print(json.dumps(summary))
if summary['differing'] or not rows_o:
    sys.exit('FAIL: native and original results differ')
print(f'PASS: {len(rows_o)} complete states (2048 erasable words, A/L/Q/EB/FB/Z/BB, channels 0-77, '
      'superbank, IRQ) identical to the original program in Virtual AGC')
