#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build and execute the native gimbal-complete entry stages under actual stock-ROM C64."""
import argparse
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def target(args,mission,short):
    build = ROOT / 'build'
    prg = build / f'gimbal-complete-{short}.prg'
    sources=['src/agc_word.c','src/agc_multiword.c','src/guidance_shift.c',
             'src/guidance_product.c','src/guidance_sum.c','src/guidance_trig.c',
             'src/guidance_dot.c','src/guidance_dp_divide.c','src/delcomp.c',
             'src/gimbal_stage.c','src/gimbal_complete.c','tests/delcomp_cases.c',
             'tests/gimbal_complete_cases.c','tests/frozen_words.c',
             'platform/c64/gimbal_complete_diagnostic.c']
    subprocess.run([args.cc,'-Os','-std=c99','-Wall','-Wextra','-Werror',
                    '-Iinclude','-Itests',f'-DGIMBAL_COMPLETE_TARGET_CM={mission}',
                    *sources,f'-Wl,-Map=build/gimbal-complete-{short}.map','-o',str(prg)],
                   cwd=ROOT,check=True)
    image = prg.read_bytes()
    assert int.from_bytes(image[:2], 'little') == 0x801
    end = 0x801 + len(image) - 2
    map_text = (build / f'gimbal-complete-{short}.map').read_text()
    bss_end = int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__bss_end =',
                            map_text, re.M)[1], 16)
    assert max(end, bss_end) < 0xc000, 'diagnostic overlaps mailbox'
    start = re.search(rb'\x9e\s*(\d+)\x00', image[:32])
    assert start
    mailbox = build / f'gimbal-complete-{short}-mailbox.bin'
    mailbox.unlink(missing_ok=True)
    monitor = build / f'gimbal-complete-{short}-vice.mon'
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
               '-monlogname', str(build / f'gimbal-complete-{short}-monitor.log'), '-monlog',
               '-limitcycles', '1500000000']
    env = os.environ.copy()
    env.setdefault('XDG_CACHE_HOME', str(build / 'vice-cache'))
    env.setdefault('XDG_CONFIG_HOME', str(build / 'vice-config'))
    for name in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(env[name]).mkdir(parents=True, exist_ok=True)
    with (build / f'gimbal-complete-{short}-vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                       timeout=180, env=env, check=True)
    assert mailbox.read_bytes() == bytes((0xa5, 80, 0)), mailbox.read_bytes().hex()
    print(f'PASS: stock-ROM PAL C64 executes complete {short.upper()} gimbal; 80 full canonical source states and all file constants')
    print('Every physical word and source SUPER/IRQ result compared exactly; caller continuation remains external')
    print(f'Native PRG {len(image)} bytes; load end ${end:04x}; BSS end ${bss_end:04x}; mailbox {mailbox.read_bytes().hex()}')


def main():
    p=argparse.ArgumentParser()
    p.add_argument('--cc',default='mos-c64-clang')
    p.add_argument('--vice',default='x64sc')
    p.add_argument('--rom-dir',type=Path,required=True)
    p.add_argument('--data-dir',type=Path)
    args=p.parse_args()
    for mission,short in enumerate(['lm','cm']):target(args,mission,short)

if __name__ == '__main__':main()
