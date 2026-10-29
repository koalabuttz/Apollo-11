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
    prg = build / 'overlay-resident.prg'
    mapping = (build / 'overlay-resident.map').read_text()
    for section in ('.text', '.rodata', '.data', '.bss', '.noinit'):
        entries = re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+' + re.escape(section) + r'$', mapping, re.M)
        assert len(entries) == 1 and sum(int(x, 16) for x in entries[0]) <= 0x7000, section
    for section in ('.zp.data', '.zp.bss'):
        entries = re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+' + re.escape(section) + r'$', mapping, re.M)
        assert len(entries) == 1 and int(entries[0], 16) == 0, section
    match = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    assert match
    mail = build / 'overlay-vice-mailbox.bin'
    mail.unlink(missing_ok=True)
    monitor = build / 'overlay-vice.mon'
    monitor.write_text(f'fill c800 cfff cd\nload "{prg}" 0\n'
        f'watch store c080\nignore 1 1\ng {int(match[1]):04x}\n'
        'm c080 c08a\n'
        'm c090 c093\n'
        f'bsave "{mail}" 0 c080 c08a\n'
        f'bsave "{build}/overlay-vice-shared.bin" 0 7000 7fff\n'
        f'bsave "{build}/overlay-vice-stack.bin" 0 c800 cfff\nscreen\nquit\n')
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
        '-kernal', str(rom / 'kernal-901227-03.bin'),
        '-basic', str(rom / 'basic-901226-01.bin'),
        '-chargen', str(rom / 'chargen-901225-01.bin'),
        '-directory', str(args.data_dir.resolve()) + ':' + str(rom.parent),
        '+drive8truedrive', '-virtualdev8', '-iecdevice8', '-device8', '1',
        '-8', str(build / 'overlay.d64'),
        '-initbreak', 'ready', '-moncommands', str(monitor),
        '-monlogname', str(build / 'overlay-vice-monitor.log'), '-monlog',
        '-limitcycles', '1000000000']
    environment = dict(os.environ, XDG_CACHE_HOME=str(build / 'vice-cache'),
                       XDG_CONFIG_HOME=str(build / 'vice-config'))
    with (build / 'overlay-vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                       check=True, timeout=60, env=environment)
    assert mail.exists(), 'VICE did not reach overlay completion mailbox'
    assert mail.read_bytes() == bytes.fromhex('a50000064e010000000000'), mail.read_bytes().hex()
    memory = (build / 'overlay-vice-shared.bin').read_bytes()
    words = [int.from_bytes(memory[i:i+2], 'little') for i in range(0, len(memory), 2)]
    assert len(words) == 2048
    for address, word in enumerate(words):
        expected = {0o300: 78, 0o301: 1, 0o302: 0}.get(address, address ^ 0x7376)
        assert word == expected, (address, word, expected)
    stack = (build / 'overlay-vice-stack.bin').read_bytes()
    changed = [i for i, byte in enumerate(stack) if byte != 0xcd]
    extent = 2048 - min(changed) if changed else 0
    assert extent < 2048, 'resident software stack exhausted'
    print('PASS: stock PAL C64 ROMs + D64 virtual IEC; two independently linked native payloads')
    print('Real Executive admission/dispatcher -> A/cache-A/B/reloaded-A -> normal native returns -> idle')
    print('All2048 raw sourcewords verified; exact shared aliases; transientBSS reset on swaps; I/D/$01 restored')
    print('Mailbox:', mail.read_bytes().hex(), 'softwarestack measured extent:', extent)
    print('VICE command:', ' '.join(command))


if __name__ == '__main__':
    main()
