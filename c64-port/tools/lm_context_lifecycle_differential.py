#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Stream complete canonical lifecycle states against original source."""
import argparse
import hashlib
import itertools
import lzma
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
CORE = ['src/agc_word.c', 'src/lm_context_lifecycle.c',
        'tests/throttle_bridge_cases.c', 'tests/lm_priority_cases.c',
        'tests/lm_sleep_cases.c', 'tests/lm_context_lifecycle_cases.c']


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--retained', action='store_true')
    args = parser.parse_args()
    flags = ['cc', '-std=c99', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests',
             '-O1' if args.sanitize else '-O2']
    if args.sanitize:
        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
    binary = 'build/lm-context-lifecycle-' + ('sanitize' if args.sanitize else 'native')
    subprocess.run(flags + CORE + ['tools/lm_context_lifecycle_snapshot.c', '-o', binary],
                   cwd=ROOT, check=True)
    archive = ROOT / ('evidence' if args.retained else 'build') / \
        'lm-context-lifecycle-reference.snapshots.xz'
    if not args.sanitize and not args.retained:
        engine = ROOT / 'deps/virtualagc/yaAGC'
        subprocess.run(flags + ['-I' + str(engine)] + CORE +
            ['tools/lm_context_lifecycle_reference.c', 'tests/scenarios.c',
             'src/executive.c', str(engine / 'agc_engine.o'),
             str(engine / 'agc_engine_init.o'), str(engine / 'rfopen.o'),
             '-o', 'build/lm-context-lifecycle-reference'], cwd=ROOT, check=True)
        with lzma.open(archive, 'wb', preset=1) as output:
            process = subprocess.Popen(['build/lm-context-lifecycle-reference',
                'deps/fixture/MAIN.agc.bin'], cwd=ROOT, stdout=subprocess.PIPE)
            for line in process.stdout:
                output.write(line)
            assert process.wait() == 0
    errors = []
    digest = hashlib.sha256()
    with lzma.open(archive, 'rb') as expected:
        process = subprocess.Popen([binary], cwd=ROOT, stdout=subprocess.PIPE,
            env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
        for count, (left, right) in enumerate(itertools.zip_longest(expected, process.stdout), 1):
            if left:
                digest.update(left)
            if left != right and len(errors) < 16:
                a = list(map(int, left.split())) if left else []
                b = list(map(int, right.split())) if right else []
                errors.append((count, [(oct(i-5), oct(x), oct(y))
                    for i, (x, y) in enumerate(zip(a, b)) if x != y], a[:5], b[:5]))
        status = process.wait()
    if errors or status:
        raise SystemExit('source mismatch %r; native exit%s' % (errors, status))
    assert count == 32768
    print('PASS:32768 complete original mixed BASIC/INTRSM/idle lifecycle '
          'states; SHA256', digest.hexdigest())


if __name__ == '__main__':
    main()
