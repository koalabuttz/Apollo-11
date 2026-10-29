#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exercise bounded disk loading and returned native C calls on stock C64 ROMs."""
import argparse
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--vice', default='x64sc')

    p.add_argument('--rom-dir', required=True, type=Path)
    p.add_argument('--data-dir', required=True, type=Path)
    args = p.parse_args()
    build = ROOT / 'build'
    rom = args.rom_dir.resolve()
    prg = build / 'cm-admission-floor.prg'
    mapping = (build / 'cm-admission-floor.map').read_text()
    for section in ('.text', '.rodata', '.data', '.bss', '.noinit'):
        entries = re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+' + re.escape(section) + r'$', mapping, re.M)
        assert len(entries) == 1 and sum(int(x, 16) for x in entries[0]) <= 0x7000, section
    for section in ('.zp.data', '.zp.bss'):
        entries = re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+' + re.escape(section) + r'$', mapping, re.M)
        assert len(entries) == 1 and int(entries[0], 16) == 0, section
    match = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    assert match
    mail = build / 'cm-admission-floor-vice-mailbox.bin'
    mail.unlink(missing_ok=True)
    (build/'cm-admission-floor-vice-monitor.log').unlink(missing_ok=True)
    monitor = build / 'cm-admission-floor-vice.mon'
    monitor.write_text(f'fill c800 cfff cd\nload "{prg}" 0\n'
        f'watch store c080\nignore 1 1\ng {int(match[1]):04x}\n'
        'm c080 c084\n'
        'm c000 c025\n'
        f'bsave "{mail}" 0 c080 c084\n'
        f'bsave "{build}/cm-admission-floor-vice-shared.bin" 0 7000 7fff\n'
        f'bsave "{build}/cm-admission-floor-vice-stack.bin" 0 c800 cfff\nscreen\nquit\n')
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
        '-kernal', str(rom / 'kernal-901227-03.bin'),
        '-basic', str(rom / 'basic-901226-01.bin'),
        '-chargen', str(rom / 'chargen-901225-01.bin'),
        '-directory', str(args.data_dir.resolve()) + ':' + str(rom.parent),
        '+drive8truedrive', '-virtualdev8', '-iecdevice8', '-device8', '1',
        '-8', str(build / 'cm-admission-floor.d64'),
        '-initbreak', 'ready', '-moncommands', str(monitor),
        '-monlogname', str(build / 'cm-admission-floor-vice-monitor.log'), '-monlog',
        '-limitcycles', '16000000000']
    environment = dict(os.environ, XDG_CACHE_HOME=str(build / 'vice-cache'),
                       XDG_CONFIG_HOME=str(build / 'vice-config'))
    with (build / 'cm-admission-floor-vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                       check=True, timeout=600, env=environment)
    assert mail.exists(), 'VICE did not reach overlay completion mailbox'
    assert mail.read_bytes() == bytes([0xa5,0,0,64,0]), mail.read_bytes().hex()
    stack = (build / 'cm-admission-floor-vice-stack.bin').read_bytes()
    changed = [i for i, byte in enumerate(stack) if byte != 0xcd]
    extent = 2048 - min(changed) if changed else 0
    assert extent < 2048, 'resident software stack exhausted'
    expected=(build/'cm-admission-floor-final-source-sample.bin').read_bytes()
    shared=(build/'cm-admission-floor-vice-shared.bin').read_bytes()
    assert len(shared)==4096 and shared==expected, 'Final whole canonical image differs from original PRIOCHNG source'
    print('PASS: stock PAL C64, actual bare priority+0 admission source paths, both incoming I, original complete image preserved at each source boundary')
    print('All core+0 unavailable positions, all four admission forms and actual resource BAILOUT handoffs:64 source checkpoints, two full2061field samples and finalwhole2048word image')
    print('Mailbox:', mail.read_bytes().hex(), 'softwarestack measured extent:', extent)
    print('VICE command:', ' '.join(command))


if __name__ == '__main__':
    main()
