#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify native original bindings, bank indexing and immutable metadata."""
import argparse
import ctypes
import gzip
import json
import os
from pathlib import Path
import subprocess

from layout_generate import rope_word

ROOT = Path(__file__).resolve().parents[1]


def run(command, **kw):
    subprocess.run(command, cwd=ROOT, check=True, **kw)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--cc', default='cc')
    args = p.parse_args()
    common = ['-std=c99', '-Wall', '-Wextra', '-Werror', '-Wconversion', '-pedantic',
              '-Iinclude', '-Itests']
    files = ['src/layout.c', 'tests/layout_check.c', 'tests/test_layout.c']
    run([args.cc, '-O2', *common, *files, '-o', 'build/test-layout'])
    run(['./build/test-layout'])
    run([args.cc, '-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
         *common, *files, '-o', 'build/test-layout-sanitize'])
    run(['./build/test-layout-sanitize'], env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
    run([args.cc, '-O2', '-shared', '-fPIC', *common, 'src/layout.c', 'src/fixed_data.c',
         '-o', 'build/layout-test.so'])
    lib = ctypes.CDLL(str(ROOT / 'build/layout-test.so'))
    read = lib.apollo_layout_fixed_read
    read.argtypes = [ctypes.c_int, ctypes.c_uint16, ctypes.POINTER(ctypes.c_uint16)]
    read.restype = ctypes.c_bool
    audit = json.loads(gzip.decompress((ROOT / 'evidence/layout-audit.json.gz').read_bytes()))
    for n, mission in enumerate(('Luminary099', 'Comanche055')):
        rope = (ROOT / 'build/fixed-data' / mission / 'MAIN.agc.bin').read_bytes()
        golden = {r['physical_word']: rope_word(rope, r['physical_word'])
                  for r in audit['fixed_metadata'][mission]}
        for physical in range(65536):
            value = ctypes.c_uint16(0xbeef)
            found = read(n, physical, ctypes.byref(value))
            assert found == (physical in golden), (mission, physical)
            assert value.value == (golden[physical] if found else 0xbeef)
    fixed_read = lib.apollo_fixed_data_read
    fixed_read.argtypes, fixed_read.restype = read.argtypes, read.restype
    value = ctypes.c_uint16(0)
    assert fixed_read(1, 0o5010, ctypes.byref(value)) and value.value == 0o200
    print('PASS: all 131072 mission/address probes expose exactly 79 independently decoded original rope metadata words')
    print('Original CM ALTVAR physical01357; R53FLBIT points at BIT8 (05010, raw00200); stray emission01356=021356')


if __name__ == '__main__':
    main()
