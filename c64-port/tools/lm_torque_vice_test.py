#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build and execute the native LM torque routine under actual stock-ROM C64."""
import argparse
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
    prg = build / 'lm-torque.prg'
    subprocess.run([args.cc, '-Os', '-std=c99', '-Wall', '-Wextra', '-Werror',
                    '-Iinclude', '-Itests', 'src/agc_word.c', 'src/agc_multiword.c',
                    'src/lm_torque.c', 'tests/lm_torque_cases.c',
                    'platform/c64/lm_torque_diagnostic.c',
                    '-Wl,-Map=build/lm-torque.map', '-o', str(prg)],
                   cwd=ROOT, check=True)
    image = prg.read_bytes()
    assert int.from_bytes(image[:2], 'little') == 0x801
    end = 0x801 + len(image) - 2
    map_text = (build / 'lm-torque.map').read_text()
    bss_end = int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__bss_end =',
                            map_text, re.M)[1], 16)
    assert max(end, bss_end) < 0xc000, 'diagnostic overlaps mailbox'
    start = re.search(rb'\x9e\s*(\d+)\x00', image[:32])
    assert start
    mailbox = build / 'lm-torque-mailbox.bin'
    mailbox.unlink(missing_ok=True)
    monitor = build / 'lm-torque-vice.mon'
    monitor.write_text(f'load "{prg}" 0\nwatch store c000\nignore 1 1\n'
                       f'g {int(start[1]):04x}\n'
                       f'bsave "{mailbox}" 0 c000 c002\nquit\n')
    rom = args.rom_dir.resolve()
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
               '-kernal', str(rom / 'kernal-901227-03.bin'),
               '-basic', str(rom / 'basic-901226-01.bin'),
               '-chargen', str(rom / 'chargen-901225-01.bin'),
               '-directory', (str(args.data_dir.resolve()) + ':' if args.data_dir else '') + str(rom.parent),
               '-initbreak', 'ready', '-moncommands', str(monitor),
               '-monlogname', str(build / 'lm-torque-monitor.log'), '-monlog',
               '-limitcycles', '2200000000']
    env = os.environ.copy()
    env.setdefault('XDG_CACHE_HOME', str(build / 'vice-cache'))
    env.setdefault('XDG_CONFIG_HOME', str(build / 'vice-config'))
    for name in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(env[name]).mkdir(parents=True, exist_ok=True)
    with (build / 'lm-torque-vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                       timeout=180, env=env, check=True)
    assert mailbox.read_bytes() == bytes((0xa5, 0, 8)), mailbox.read_bytes().hex()
    print('PASS: stock-ROM PAL C64 executes complete LM KALMAN_FILTER; 2048 original source states under all eight EB alias contexts')
    print('Every physical word compared exactly, including unchanged words; SUPER/IRQ preserved')
    print(f'Native PRG {len(image)} bytes; load end ${end:04x}; BSS end ${bss_end:04x}; mailbox {mailbox.read_bytes().hex()}')


if __name__ == '__main__':
    main()
