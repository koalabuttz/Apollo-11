#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check one part of a milestone's 65536-entry oracle stream on the host.

The pinned yaAGC oracle (tools/<m>_reference.c, unchanged Luminary099) and
a fresh strict or ASAN/UBSAN native producer (tools/<m>_snapshot.c) both
run entries [first, limit) of part K of N. Every row must be equal, and the
part's bytes must have the SHA-256 that evidence/<m>/<m>-source-stream-parts.json
pins for it. That file is recorded (--record N) from the whole oracle stream,
which must equal the decoded_sha256 of evidence/<m>/<m>-source-stream.json,
so the N parts together check every byte the whole-stream identity does.

  python3 tools/oracle_stream_part.py lm-conic-lambert --part 2/4 [--sanitize]
  python3 tools/oracle_stream_part.py lm-conic-lambert --record 4
"""
import argparse, hashlib, importlib, json, os, pathlib, shlex, struct, subprocess, sys, threading

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
ROPE = 'deps/lm-rtb-fixture/MAIN.agc.bin'
YAAGC = 'deps/virtualagc/yaAGC'

p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
p.add_argument('milestone')
mode = p.add_mutually_exclusive_group(required=True)
mode.add_argument('--part', metavar='K/N')
mode.add_argument('--record', type=int, metavar='N')
p.add_argument('--sanitize', action='store_true')
p.add_argument('--cc', default='cc')
a = p.parse_args()
m, base = a.milestone, a.milestone.replace('-', '_')
pin = json.loads((R / f'evidence/{m}/{m}-source-stream.json').read_text())
parts_path = R / f'evidence/{m}/{m}-source-stream-parts.json'
states, row = pin['states'], pin['decoded_bytes'] // pin['states']
assert row * states == pin['decoded_bytes']


def ranges(n):
    return [(k * states // n, (k + 1) * states // n) for k in range(n)]


def build_oracle(tag):
    if not (R / 'build/agc_engine_hardware_dv.o').exists():
        subprocess.run([sys.executable, 'tools/prepare_hardware_dv.py'], cwd=R, check=True)
    out = f'build/{m}-oracle-{tag}'
    subprocess.run(['gcc', '-std=c99', '-O2', '-Iinclude', '-Itests', f'-I{YAAGC}', '-Itools', f'tools/{base}_reference.c',
                    f'tests/{base}_cases.c', f'tests/{base}_debris.c', 'tests/cm_executive_cases.c', 'src/agc_word.c',
                    'build/agc_engine_hardware_dv.o', f'{YAAGC}/agc_engine_init.o', f'{YAAGC}/rfopen.o', '-lm', '-o', out],
                   cwd=R, check=True)
    return out


def build_native(tag):
    core = importlib.import_module(f'{base}_core').CORE
    flags = shlex.split(a.cc) + ['-std=c99', '-O1' if a.sanitize else '-O2', '-Wall', '-Wextra', '-Werror', '-Wconversion',
                                 '-pedantic', '-Iinclude', '-Itests']
    if a.sanitize:
        flags += ['-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    out = f'build/{m}-native-{tag}' + ('-sanitize' if a.sanitize else '')
    subprocess.run(flags + [f'tools/{base}_snapshot.c', *core, '-lm', '-o', out], cwd=R, check=True)
    return out


def start(cmd, first, limit):
    return subprocess.Popen(cmd + [str(limit), str(first)], cwd=R, stdout=subprocess.PIPE,
                            env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


if a.record:
    # Every part's oracle runs at once; the whole stream is hashed in order.
    oracle = build_oracle('record')
    spans = ranges(a.record)
    data = [None] * len(spans)

    def read(i, first, limit):
        proc = start([oracle, ROPE], first, limit)
        data[i] = proc.stdout.read()
        if proc.wait() != 0:
            data[i] = None
    threads = [threading.Thread(target=read, args=(i, *s)) for i, s in enumerate(spans)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    whole = hashlib.sha256()
    out = []
    for (first, limit), blob in zip(spans, data):
        if blob is None or len(blob) != (limit - first) * row:
            sys.exit(f'FAIL oracle entries {first}-{limit} short or failed')
        whole.update(blob)
        out.append({'first': first, 'limit': limit, 'sha256': hashlib.sha256(blob).hexdigest()})
    if whole.hexdigest() != pin['decoded_sha256']:
        sys.exit('FAIL whole oracle stream differs from the pinned decoded_sha256')
    parts_path.write_text(json.dumps({'decoded_sha256': pin['decoded_sha256'], 'row_bytes': row, 'parts': out},
                                     indent=2) + '\n')
    print(f'Recorded {len(out)} parts of {states} oracle entries, whole stream {whole.hexdigest()}')
    sys.exit(0)

k, n = (int(x) for x in a.part.split('/'))
parts = json.loads(parts_path.read_text())
if parts['decoded_sha256'] != pin['decoded_sha256'] or parts['row_bytes'] != row or len(parts['parts']) != n:
    sys.exit(f'FAIL {parts_path.name} is not {n} parts of the pinned stream (re-record it: --record {n})')
part = parts['parts'][k - 1]
first, limit = part['first'], part['limit']
assert (first, limit) == ranges(n)[k - 1]
tag = f'part{k}'
ref = start([build_oracle(tag), ROPE], first, limit)
nat = start([build_native(tag)], first, limit)
h = hashlib.sha256()
try:
    for entry in range(first, limit):
        x, y = ref.stdout.read(row), nat.stdout.read(row)
        if len(x) != row or x != y:
            if len(x) == row == len(y):
                words = row // 2
                X, Y = struct.unpack(f'<{words}H', x), struct.unpack(f'<{words}H', y)
                sys.exit(f'FAIL entry {entry} ' + str([(f'{i - 5:04o}', f'{v:06o}', f'{Y[i]:06o}')
                                                        for i, v in enumerate(X) if v != Y[i]][:25]))
            sys.exit(f'FAIL entry {entry} short row')
        h.update(x)
    if ref.stdout.read(1) or nat.stdout.read(1) or ref.wait() or nat.wait():
        sys.exit('FAIL producer exit or trailing bytes')
finally:
    for proc in (ref, nat):
        if proc.poll() is None:
            proc.kill()
            proc.wait()
if h.hexdigest() != part['sha256']:
    sys.exit(f'FAIL oracle entries {first}-{limit} differ from the pinned part SHA256')
print(f'PASS part {k}/{n}: original entries {first}-{limit - 1} oracle equal '
      + ('ASAN/UBSAN' if a.sanitize else 'strict') + f' native, SHA256 {h.hexdigest()}', flush=True)
