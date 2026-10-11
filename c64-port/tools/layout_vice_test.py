#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Execute native original aliases against the actual canonical C64 RAM image."""
import argparse
import gzip
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--cc', default='mos-c64-clang')
    p.add_argument('--vice', default='x64sc')
    p.add_argument('--rom-dir', type=Path, required=True)
    p.add_argument('--data-dir', type=Path)
    args = p.parse_args()
    build = ROOT / 'build'
    prg = build / 'layout.prg'
    subprocess.run([args.cc, '-Os', '-std=c99', '-Wall', '-Wextra', '-Werror',
                    '-Iinclude', '-Itests', 'src/layout.c', 'tests/layout_check.c',
                    'platform/c64/layout_diagnostic.c', '-Wl,-Map=build/layout.map',
                    '-o', str(prg)], cwd=ROOT, check=True)
    image = prg.read_bytes()
    assert int.from_bytes(image[:2], 'little') == 0x801
    end = 0x801 + len(image) - 2
    assert end <= 0x7000, ('diagnostic overlaps canonical image', end)
    start = re.search(rb'\x9e\s*(\d+)\x00', image[:32])
    assert start
    audit = json.loads(gzip.decompress((ROOT / 'evidence/layout-audit.json.gz').read_bytes()))
    count = len(audit['bindings'])
    assert count == 4232
    paths = {name: build / ('layout-' + name + '.bin') for name in ('mailbox', 'bindings', 'image')}
    for path in paths.values():
        path.unlink(missing_ok=True)
    monitor = build / 'layout-vice.mon'
    monitor.write_text(f'load "{prg}" 0\nwatch store c000\nignore 1 1\n'
                       f'g {int(start[1]):04x}\nbank ram\n'
                       f'bsave "{paths["mailbox"]}" 0 c000 c002\n'
                       f'bsave "{paths["bindings"]}" 0 8000 {0x8000 + count * 2 - 1:04x}\n'
                       f'bsave "{paths["image"]}" 0 7000 7fff\nquit\n')
    rom = args.rom_dir.resolve()
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
               '-kernal', str(rom / 'kernal-901227-03.bin'),
               '-basic', str(rom / 'basic-901226-01.bin'),
               '-chargen', str(rom / 'chargen-901225-01.bin'),
               '-directory', (str(args.data_dir.resolve()) + ':' if args.data_dir else '') + str(rom.parent),
               '-initbreak', 'ready', '-moncommands', str(monitor),
               '-monlogname', str(build / 'layout-monitor.log'), '-monlog',
               '-limitcycles', '100000000']
    env = os.environ.copy()
    env.setdefault('XDG_CACHE_HOME', str(build / 'vice-cache'))
    env.setdefault('XDG_CONFIG_HOME', str(build / 'vice-config'))
    for name in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(env[name]).mkdir(parents=True, exist_ok=True)
    with (build / 'layout-vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                       timeout=60, env=env, check=True)
    assert paths['mailbox'].read_bytes() == bytes((0xa5, count & 255, count >> 8))
    native = paths['bindings'].read_bytes()
    assert len(native) == count * 2
    for row, binding in enumerate(audit['bindings']):
        assert int.from_bytes(native[row*2:row*2+2], 'little') == binding['value'], binding
    native = paths['image'].read_bytes()
    assert len(native) == 4096
    for i in range(2048):
        expected = 0o21356 if i == 0o1356 else (47 * i + 0x3412) & 0o77777
        assert int.from_bytes(native[i*2:i*2+2], 'little') == expected, i
    print('PASS: stock-ROM PAL C64 proves 4232 native original bindings and alias RAM reads/writes against sole canonical image at $7000')
    print('All 8192 retained-bank address contexts and 79 original fixed metadata reads; complete 2048-word final image independently compared')
    print(f'PRG {len(image)} bytes; loaded end ${end:04x}; mailbox {paths["mailbox"].read_bytes().hex()}')


if __name__ == '__main__':
    main()
