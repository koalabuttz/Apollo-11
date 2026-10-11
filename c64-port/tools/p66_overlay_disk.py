#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pack the actual five bounded P66 modules with their supplied resident PRG."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
from overlay_build import crc16

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--resident', required=True, type=Path)
    p.add_argument('--modules', type=Path, default=ROOT / 'build/overlay-p66-modules.json')
    p.add_argument('--output', type=Path, default=ROOT / 'build/p66-overlay.d64')
    p.add_argument('--c1541', default='c1541')
    args = p.parse_args()
    resident = args.resident.resolve()
    data = resident.read_bytes()
    assert data[:2] == b'\x01\x08' and re.search(rb'\x9e\s*\d+\x00', data[:32]), 'not a bootable resident PRG'
    assert 0x0801 + len(data) - 2 <= 0x7000, 'resident file overlaps shared source RAM'
    manifest = json.loads(args.modules.read_text())
    files = []
    for module in manifest:
        path = args.modules.resolve().parent / module['prg']
        data = path.read_bytes()
        assert hashlib.sha256(data).hexdigest() == module['sha256']
        assert len(data) - 2 == module['byte_count'] and crc16(data[2:]) == module['crc16']
        assert data[:2] == b'\x00\x80' and data[7] == module['id']
        files.append((path, module['filename'].lower()))
    # Formatted D64 has664 data blocks, each carrying254 filebytes. This packing
    # check leaves directory overhead for the six-file demonstration.
    blocks = (resident.stat().st_size + 253) // 254
    blocks += sum((path.stat().st_size + 253) // 254 for path, _ in files)
    assert blocks <= 663, 'native P66 payload set does not fit a35-track D64'
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.unlink(missing_ok=True)
    command = [args.c1541, '-format', 'apollo p66,11', 'd64', str(args.output.resolve()),
               '-write', str(resident), 'p66native']
    for path, name in files:
        command += ['-write', str(path), name]
    build = ROOT / 'build'
    environment = dict(os.environ, XDG_CACHE_HOME=str(build / 'vice-cache'),
                       XDG_CONFIG_HOME=str(build / 'vice-config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(environment[key]).mkdir(parents=True, exist_ok=True)
    subprocess.run(command, check=True, env=environment)
    print('Packed one bootable resident andfive independentlylinked P66 groups:', args.output)
    print('Native PRG data blocks:', blocks, '/664; source state is canonical2048wordRAM')


if __name__ == '__main__':
    main()
