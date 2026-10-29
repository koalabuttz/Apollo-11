#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run the CM service overlay proof under ABI 2 (REU store) in VICE.

Needs tools/cm_service_overlay_v2_build.py first. This is the ABI 1 proof
(tools/cm_service_overlay_vice_test.py) with the same replay and the same
expected mailbox, but every module comes from the REU store. When
build/cm-service-loop-reference.snapshots exists, the final canonical image
is also compared against original case 320."""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
B = ROOT / 'build/cm-service-overlay-v2'


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=Path, default=os.environ.get('VICE_ROM_DIR'))
    p.add_argument('--data-dir', type=Path, default=os.environ.get('VICE_DATA_DIR'))
    args = p.parse_args()
    if not args.rom_dir or not args.data_dir:
        p.error('--rom-dir and --data-dir (or VICE_ROM_DIR/VICE_DATA_DIR) are required')
    rom = args.rom_dir.resolve()
    prg = B / 'resident.prg'
    match = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    assert match
    reu = B / 'vice-cm.reu'
    shutil.copyfile(B / 'cm.reu', reu)
    mail = B / 'vice-mailbox.bin'
    mail.unlink(missing_ok=True)
    monitor = B / 'vice.mon'
    monitor.write_text(f'fill c800 cfff cd\nload "{prg}" 0\n'
        f'watch store c080\nignore 1 1\ng {int(match[1]):04x}\n'
        f'bsave "{mail}" 0 c080 c088\n'
        f'bsave "{B}/vice-shared.bin" 0 7000 7fff\n'
        f'bsave "{B}/vice-stack.bin" 0 c800 cfff\nscreen\nquit\n')
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
        '-kernal', str(rom / 'kernal-901227-03.bin'), '-basic', str(rom / 'basic-901226-01.bin'),
        '-chargen', str(rom / 'chargen-901225-01.bin'),
        '-directory', str(args.data_dir.resolve()) + ':' + str(rom.parent),
        '-reu', '-reusize', '128', '-reuimage', str(reu),
        '-initbreak', 'ready', '-moncommands', str(monitor),
        '-monlogname', str(B / 'vice-monitor.log'), '-monlog', '-limitcycles', '16000000000']
    env = dict(os.environ, XDG_CACHE_HOME=str(B / 'vice-cache'), XDG_CONFIG_HOME=str(B / 'vice-config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(env[key]).mkdir(exist_ok=True)
    with (B / 'vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600, env=env)
    assert mail.exists(), 'VICE did not reach the CM ABI 2 completion mailbox'
    m = mail.read_bytes()
    failures = m[1] | m[2] << 8
    assert m[0] == 0xA5 and failures == 0 and m[3] == 58 and m[4] == 6, m.hex()
    stack = (B / 'vice-stack.bin').read_bytes()
    changed = [i for i, byte in enumerate(stack) if byte != 0xCD]
    extent = 2048 - min(changed) if changed else 0
    assert extent < 2048, 'resident software stack exhausted'
    result = dict(mailbox=m.hex(), checkpoints=m[3], module_loads=m[5] | m[6] << 8,
                  irq_count=m[7] | m[8] << 8, software_stack_extent=extent)
    print('PASS: CM service overlay proof on ABI 2, modules from the REU store, $01=$35')
    reference = ROOT / 'build/cm-service-loop-reference.snapshots'
    if reference.exists():
        expected = [list(map(int, line.split())) for line in reference.read_text().splitlines()
                    if line.startswith('320 ')][-1]
        shared = (B / 'vice-shared.bin').read_bytes()
        actual = [int.from_bytes(shared[i:i + 2], 'little') for i in range(0, 4096, 2)]
        assert actual == expected[4:2052], 'final canonical image differs from original case 320'
        result['case_320_image'] = 'match'
    else:
        result['case_320_image'] = 'not checked (no reference snapshots)'
    abi1 = ROOT / 'build/cm-service-overlay-vice-shared.bin'
    if abi1.exists():
        assert abi1.read_bytes() == (B / 'vice-shared.bin').read_bytes(), 'final image differs from the ABI 1 run'
        result['abi1_image'] = 'match'
    (B / 'vice-result.json').write_text(json.dumps(result, indent=1) + '\n')
    for key, value in result.items():
        print(f'  {key}: {value}')


if __name__ == '__main__':
    main()
