#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run a Lane C suite through the native routines and through the unchanged
original program in Virtual AGC, and require identical complete results.

  python3 tools/lanec/diff.py SUITE [--native build/lanec-native-sanitize]

Writes build/lanec-SUITE-{native,oracle}.txt (complete states, one case per
line), build/lanec-SUITE-{native,oracle}.crc (per-case CRC-32, the compact
evidence form, same hash as the C64 diagnostic) and build/lanec-SUITE-diff.json. The cases are deterministic,
so the full streams are reproducible from the sources at any time.

With --refusals, a case the native routine refuses (status 1, which the
native runner prints only when the image and I/O are unchanged) is counted
as a refusal instead of compared: those are the suite's out-of-domain cases.
At least one case must still be accepted and match. With --refusals-only
(a suite of out-of-domain inputs) the oracle is not run and every case must
be refused with the state unchanged.

With --uncounted the run and its checks are the same, but its verdict lines
do not start with PASS: a CI part that needs this suite's files while
another part of the same run carries the verdict (the CM parts of
vice-lanec-pin-keys-<n>, tools/mk/lanec-pin-keys.mk) keeps the run's check
count (tools/vice_manifest.py) equal to the unsplit run's.
"""
import argparse, hashlib, json, os, pathlib, subprocess, sys, zlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROPES = {'lm': ('deps/lm-powered-fixture/MAIN.agc.bin',
                '1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e'),
         'cm': ('deps/cm-powered-fixture/MAIN.agc.bin',
                '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061')}

p = argparse.ArgumentParser()
p.add_argument('suite')
p.add_argument('--native', default='build/lanec-native')
p.add_argument('--oracle', default='build/lanec-oracle')
p.add_argument('--refusals', action='store_true')
p.add_argument('--refusals-only', action='store_true')
p.add_argument('--uncounted', action='store_true')
a = p.parse_args()
for path, digest in ROPES.values():
    if hashlib.sha256((ROOT / path).read_bytes()).hexdigest() != digest:
        sys.exit(f'FAIL: {path} is not the pinned rope')
subprocess.run([sys.executable, 'tools/lanec/symbols.py', '--check'], cwd=ROOT, check=True,
               stdout=subprocess.DEVNULL if a.uncounted else None)
VERDICT = 'checked (uncounted)' if a.uncounted else 'PASS'
native = subprocess.run([a.native, a.suite], cwd=ROOT, capture_output=True, check=True).stdout
if a.refusals_only:
    rows = native.splitlines()
    accepted = [r.split()[0].decode() for r in rows if r.split()[-1] != b'1']
    summary = dict(suite=a.suite, cases=len(rows), refused_unchanged=len(rows) - len(accepted),
                   native_sha256=hashlib.sha256(native).hexdigest())
    (ROOT / f'build/lanec-{a.suite}-native.txt').write_bytes(native)
    (ROOT / f'build/lanec-{a.suite}-diff.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary))
    if accepted or not rows:
        sys.exit(f'FAIL: cases not refused with the state unchanged: {accepted[:10]}')
    print(f'PASS: {len(rows)} out-of-domain cases refused with the image and I/O unchanged')
    sys.exit(0)
oracle = subprocess.run([a.oracle, a.suite, ROPES['lm'][0], ROPES['cm'][0]], cwd=ROOT,
                        capture_output=True, check=True).stdout
rows_n, rows_o = native.splitlines(), oracle.splitlines()
refused = [i for i, x in enumerate(rows_n) if a.refusals and x.split()[-1] == b'1']
skip = set(refused)
bad = [i for i, (x, y) in enumerate(zip(rows_n, rows_o)) if x != y and i not in skip]
for i in bad[:10]:
    xs, ys = rows_n[i].split(), rows_o[i].split()
    print('DIFF case', xs[0], [(oct(k - 1) if k <= 2048 else f'io{k - 2049:o}', xs[k], ys[k])
                               for k in range(1, len(xs)) if xs[k] != ys[k]][:12])
tag = pathlib.Path(a.native).name.replace('lanec-native', '') or ''
def write_atomic(path, data):
    """Write path whole: CI parts of one suite (vice-lanec-pin-keys-<n>) may
    write and read the same rows at once on one runner."""
    tmp = path.with_name(f'{path.name}.{os.getpid()}.tmp')
    tmp.write_bytes(data)
    os.replace(tmp, path)


for name, data in (('native' + tag, native), ('oracle', oracle)):
    write_atomic(ROOT / f'build/lanec-{a.suite}-{name}.txt', data)
    (ROOT / f'build/lanec-{a.suite}-{name}.crc').write_text(''.join(
        f"{row.split()[0].decode()} {zlib.crc32(b''.join(int(x).to_bytes(2, 'little') for x in row.split()[1:])):08x}\n"
        for row in data.splitlines()))
summary = dict(suite=a.suite, cases=len(rows_o), differing=len(bad) + abs(len(rows_n) - len(rows_o)),
               **({'refused_unchanged': len(refused)} if a.refusals else {}),
               native_sha256=hashlib.sha256(native).hexdigest(), oracle_sha256=hashlib.sha256(oracle).hexdigest())
(ROOT / f'build/lanec-{a.suite}-diff{tag}.json').write_text(json.dumps(summary, indent=2) + '\n')
print(json.dumps(summary))
if summary['differing'] or len(refused) >= len(rows_o):
    sys.exit('FAIL: native and original results differ')
print(f'{VERDICT}: {len(rows_o) - len(refused)} complete states (2048 erasable words, A/L/Q/EB/FB/Z/BB, channels 0-77, '
      'superbank, IRQ) identical to the original program in Virtual AGC'
      + (f'; {len(refused)} out-of-domain cases refused with the state unchanged' if refused else ''))
