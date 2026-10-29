#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Reproducible strict/sanitizer/original-source full-image qualification."""
import argparse
import hashlib
import lzma
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def run(command, **kw):
    subprocess.run(command, cwd=ROOT, check=True, **kw)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--cc', default='cc')
    args = p.parse_args()
    common = ['-std=c99', '-Wall', '-Wextra', '-Werror', '-Wconversion', '-pedantic',
              '-Iinclude', '-Itests']
    native = ['src/agc_word.c', 'src/fixed_data.c', 'src/noun_reader.c',
              'tests/noun_reader_cases.c']
    run([args.cc, '-O2', *common, *native, 'tests/test_noun_reader.c',
         '-o', 'build/test-noun-reader'])
    run(['./build/test-noun-reader'])
    run([args.cc, '-O2', *common, *native, 'tools/noun_reader_snapshot.c',
         '-o', 'build/noun-reader-native'])
    for name, extra in [('native', []), ('input', ['--input'])]:
        with (ROOT / f'build/noun-reader-{name}.snapshots').open('wb') as out:
            run(['./build/noun-reader-native', *extra], stdout=out)
    # Existing independently built GPL oracle objects, never target dependencies.
    oracle = ROOT / 'deps/virtualagc/yaAGC'
    objects = [oracle / name for name in ('agc_engine.o', 'agc_engine_init.o', 'rfopen.o')]
    assert all(path.exists() for path in objects), 'build reference engine first with tools/prepare_reference.py'
    run([args.cc, '-std=c99', '-O2', '-Iinclude', '-Itests', '-Ideps/virtualagc/yaAGC',
         'tools/noun_reader_reference.c', 'tests/noun_reader_cases.c', 'tests/scenarios.c',
         *(str(path) for path in objects), '-o', 'build/noun-reader-reference'])
    for short, mission in [('lm', 'Luminary099'), ('cm', 'Comanche055')]:
        with (ROOT / f'build/noun-reader-{short}.snapshots').open('wb') as out:
            run(['./build/noun-reader-reference', f'build/fixed-data/{mission}/MAIN.agc.bin'], stdout=out)
    original = ((ROOT / 'build/noun-reader-lm.snapshots').read_bytes() +
                (ROOT / 'build/noun-reader-cm.snapshots').read_bytes())
    actual = (ROOT / 'build/noun-reader-native.snapshots').read_bytes()
    assert len(original) == 58368 * 2052 * 2
    if original != actual:
        i = next(i for i, (a, b) in enumerate(zip(original, actual)) if a != b)
        raise AssertionError(('full-state mismatch', i // 4104, i % 4104 // 2))
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
    sanitize = ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    run([args.cc, *sanitize, *common, *native, 'tests/test_noun_reader.c',
         '-o', 'build/test-noun-reader-sanitize'])
    run(['./build/test-noun-reader-sanitize'], env=env)
    run([args.cc, *sanitize, *common, *native, 'tools/noun_reader_snapshot.c',
         '-o', 'build/noun-reader-snapshot-sanitize'])
    with (ROOT / 'build/noun-reader-sanitize.snapshots').open('wb') as out:
        run(['./build/noun-reader-snapshot-sanitize'], stdout=out, env=env)
    assert (ROOT / 'build/noun-reader-sanitize.snapshots').read_bytes() == original
    for name, contents in [('reference', original), ('native', actual)]:
        (ROOT / f'evidence/noun-reader-{name}.snapshots.xz').write_bytes(lzma.compress(contents))
    print('PASS: 58368 native and sanitized complete 2048-word images match independently assembled original LM/CM source')
    print('Snapshot little-endian words: case, success, 2048 physical words, SUPER, IRQ')
    print('Reference/native SHA256:', hashlib.sha256(original).hexdigest())
    run(['python3', 'tools/freeze_noun_reader.py'])


if __name__ == '__main__':
    main()
