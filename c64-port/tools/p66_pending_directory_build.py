#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build a metadata-only check for the separate pending-flight disk."""
import argparse
import json
from pathlib import Path
import subprocess
from overlay_build import crc16
ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--cc', default='mos-c64-clang')
args = parser.parse_args()
modules = [record for record in json.loads((ROOT / 'build' /
    'p66-pending-flight-modules.json').read_text()) if record['id'] != 148]
assert len(modules) == 118 and len({m['id'] for m in modules}) == 118
raw = bytearray()
for record in sorted(modules, key=lambda m: m['id']):
    raw += bytes([record['id']]) + record['byte_count'].to_bytes(2, 'little') + \
        record['crc16'].to_bytes(2, 'little') + record['filename'].encode() + b'\0'
subprocess.run([args.cc, '-Ibuild/p66-pending-flight', '-Iinclude', '-Iplatform/c64',
    '-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror',
    '-DNATIVE_REGISTRY_EXPECTED_COUNT=118', '-DNATIVE_REGISTRY_EXPECTED_CRC=%d' % crc16(raw),
    '-T', 'platform/c64/overlay_resident.ld', '-Wl,-Map,build/p66-pending-directory.map',
    'src/overlay_runtime.c', 'src/native_module_registry.c', 'platform/c64/overlay_platform.c',
    'platform/c64/native_registry_directory_diagnostic.c', '-o',
    'build/p66-pending-directory.prg'], cwd=ROOT, check=True)
print('Built118-record directory proof; pending-flight disk untouched', flush=True)
