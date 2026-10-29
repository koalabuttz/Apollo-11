#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run actual native C64 table reads, comparing all words with source data."""
import argparse
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
    prg = build / 'fixed-data.prg'
    subprocess.run([args.cc, '-Os', '-std=c99', '-Wall', '-Wextra', '-Werror',
                    '-Iinclude', 'src/fixed_data.c',
                    'platform/c64/fixed_data_diagnostic.c',
                    '-Wl,-Map=build/fixed-data.map', '-o', str(prg)],
                   cwd=ROOT, check=True)
    image = prg.read_bytes()
    assert int.from_bytes(image[:2], 'little') == 0x801
    end = 0x801 + len(image) - 2
    assert end < 0x7000, ('loaded image overlaps output scratch', end)
    start = re.search(rb'\x9e\s*(\d+)\x00', image[:32])
    assert start
    audit = json.loads((build / 'fixed-data/audit.json').read_text())
    count = sum(mission['native_word_count'] for mission in audit['missions'])
    assert count <= 4096
    mailbox, words = build / 'fixed-data-mailbox.bin', build / 'fixed-data-words.bin'
    mailbox.unlink(missing_ok=True)
    words.unlink(missing_ok=True)
    monitor = build / 'fixed-data-vice.mon'
    monitor.write_text(
        f'load "{prg}" 0\nwatch store c000\nignore 1 1\n'
        f'g {int(start[1]):04x}\n'
        f'bsave "{mailbox}" 0 c000 c002\n'
        f'bsave "{words}" 0 7000 {0x7000 + count * 2 - 1:04x}\nquit\n')
    rom = args.rom_dir.resolve()
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
               '-kernal', str(rom / 'kernal-901227-03.bin'),
               '-basic', str(rom / 'basic-901226-01.bin'),
               '-chargen', str(rom / 'chargen-901225-01.bin'),
               '-directory', (str(args.data_dir.resolve()) + ':' if args.data_dir else '') + str(rom.parent),
               '-initbreak', 'ready', '-moncommands', str(monitor),
               '-monlogname', str(build / 'fixed-data-monitor.log'), '-monlog',
               '-limitcycles', '500000000']
    env = os.environ.copy()
    env.setdefault('XDG_CACHE_HOME', str(build / 'vice-cache'))
    env.setdefault('XDG_CONFIG_HOME', str(build / 'vice-config'))
    for name in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(env[name]).mkdir(parents=True, exist_ok=True)
    with (build / 'fixed-data-vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                       timeout=60, env=env, check=True)
    assert mailbox.read_bytes() == bytes((0xa5, count & 255, count >> 8))
    expected = []
    for mission in audit['missions']:
        raw = {}
        for record in mission['native_records']:
            for offset, word in enumerate(record['words']):
                raw[record['physical_fixed_word'] + offset] = word
        expected.extend(raw[address] for address in sorted(raw))
    actual = words.read_bytes()
    assert len(actual) == len(expected) * 2
    for index, word in enumerate(expected):
        assert int.from_bytes(actual[2 * index:2 * index + 2], 'little') == word, index
    print(f'PASS: stock-ROM PAL C64 executes native LM+CM table ABI; all {len(expected)} raw words match source')
    print(f'Native PRG bytes {len(image)}; loaded end 0x{end:04x}; mailbox {mailbox.read_bytes().hex()}')


if __name__ == '__main__':
    main()
