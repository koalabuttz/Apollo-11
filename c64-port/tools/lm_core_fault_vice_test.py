#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exercise canonical Executive admission on stock C64 ROMs."""
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
    prg = build / 'lm-core-fault.prg'
    mapping = (build / 'lm-core-fault.map').read_text()
    for section in ('.text', '.rodata', '.data', '.bss', '.noinit'):
        entries = re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+' + re.escape(section) + r'$', mapping, re.M)
        assert len(entries) == 1 and sum(int(x, 16) for x in entries[0]) <= 0x7000, section
    for section in ('.zp.data', '.zp.bss'):
        entries = re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+' + re.escape(section) + r'$', mapping, re.M)
        assert len(entries) == 1 and int(entries[0], 16) == 0, section
    match = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    assert match
    mail = build / 'lm-core-fault-vice-mailbox.bin'
    mail.unlink(missing_ok=True)
    (build/'lm-core-fault-vice-monitor.log').unlink(missing_ok=True)
    monitor = build / 'lm-core-fault-vice.mon'
    monitor.write_text(f'fill 0100 01f1 cd\nfill c800 cfff cd\nload "{prg}" 0\n'
        f'watch store c080\nignore 1 1\ng {int(match[1]):04x}\n'
        'm c080 c085\n'
        'm c000 c003\n'
        f'bsave "{mail}" 0 c080 c085\n'
        f'bsave "{build}/lm-core-fault-vice-shared.bin" 0 7000 7fff\n'
        f'bsave "{build}/lm-core-fault-vice-stack.bin" 0 c800 cfff\n'
        f'bsave "{build}/lm-core-fault-vice-hardware-stack.bin" 0 0100 01f1\nscreen\nquit\n')
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
        '-kernal', str(rom / 'kernal-901227-03.bin'),
        '-basic', str(rom / 'basic-901226-01.bin'),
        '-chargen', str(rom / 'chargen-901225-01.bin'),
        '-directory', str(args.data_dir.resolve()) + ':' + str(rom.parent),
        '+drive8truedrive', '-virtualdev8', '-iecdevice8', '-device8', '1',
        '-drive8type','1581','-8', str(build / 'lm-core-fault.d81'),
        '-initbreak', 'ready', '-moncommands', str(monitor),
        '-monlogname', str(build / 'lm-core-fault-vice-monitor.log'), '-monlog',
        '-limitcycles', '16000000000']
    environment = dict(os.environ, XDG_CACHE_HOME=str(build / 'vice-cache'),
                       XDG_CONFIG_HOME=str(build / 'vice-config'))
    with (build / 'lm-core-fault-vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                       check=True, timeout=2400, env=environment)
    assert mail.exists(), 'VICE did not reach overlay completion mailbox'
    assert mail.read_bytes() == bytes([0xa5,0,0,64,0,2]), mail.read_bytes().hex()
    stack = (build / 'lm-core-fault-vice-stack.bin').read_bytes()
    changed = [i for i, byte in enumerate(stack) if byte != 0xcd]
    extent = 2048 - min(changed) if changed else 0
    assert extent < 2048, 'resident software stack exhausted'
    hardware=(build/'lm-core-fault-vice-hardware-stack.bin').read_bytes()
    used=[i for i,v in enumerate(hardware)if v!=0xcd]
    hardware_extent=len(hardware)-min(used)if used else 0
    assert hardware_extent<len(hardware),'hardware stack exhausted'
    print('PASS: stock PAL C64 ROMs + D81 virtual IEC; resident fixed source bridges and32 frozen input/check disk modules')
    print('64 full raw reserved-code handoff states; execution beyond TC7/TC014 remains external')
    print('64 frozen admission/wake entries; allSUPER forms, caller I/$01 restored; full canonical image and explicit I/O')
    print('Mailbox:', mail.read_bytes().hex(), 'softwarestack measured extent:', extent,'hardwarestack extent:',hardware_extent)
    print('VICE command:', ' '.join(command))


if __name__ == '__main__':
    main()
