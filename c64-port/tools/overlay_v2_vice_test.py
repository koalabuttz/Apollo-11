#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run the ABI 2 REU proof on stock PAL C64 ROMs in VICE and check it.

Needs tools/overlay_v2_target_build.py first. The REU image is attached
read-only through a scratch copy, so the packed store stays unchanged."""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
B = ROOT / 'build/overlay-v2'


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
    reu = B / 'vice-lm.reu'
    shutil.copyfile(B / 'lm.reu', reu)
    mail = B / 'vice-mailbox.bin'
    mail.unlink(missing_ok=True)
    monitor = B / 'vice.mon'
    monitor.write_text(f'fill c800 cfff cd\nload "{prg}" 0\n'
        f'watch store c080\nignore 1 1\ng {int(match[1]):04x}\n'
        f'bsave "{mail}" 0 c080 c09f\n'
        f'bsave "{B}/vice-shared.bin" 0 7000 7fff\n'
        f'bsave "{B}/vice-stack.bin" 0 c800 cfff\nscreen\nquit\n')
    command = [args.vice, '-default', '-console', '-pal', '-warp', '+sound',
        '-kernal', str(rom / 'kernal-901227-03.bin'), '-basic', str(rom / 'basic-901226-01.bin'),
        '-chargen', str(rom / 'chargen-901225-01.bin'),
        '-directory', str(args.data_dir.resolve()) + ':' + str(rom.parent),
        '-reu', '-reusize', '128', '-reuimage', str(reu),
        '-initbreak', 'ready', '-moncommands', str(monitor),
        '-monlogname', str(B / 'vice-monitor.log'), '-monlog', '-limitcycles', '400000000']
    env = dict(os.environ, XDG_CACHE_HOME=str(B / 'vice-cache'), XDG_CONFIG_HOME=str(B / 'vice-config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(env[key]).mkdir(exist_ok=True)
    with (B / 'vice.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=120, env=env)
    assert mail.exists(), 'VICE did not reach the ABI 2 completion mailbox'
    m = mail.read_bytes()
    failures = m[1] | m[2] << 8
    assert m[0] == 0xA5 and m[3] == 2, m.hex()
    assert failures == 0 and m[4] == 0, f'{failures} failures, first at step {m[4]}: {m.hex()}'
    cycles = lambda at: m[at] | m[at + 1] << 8 | m[at + 2] << 16
    shared = (B / 'vice-shared.bin').read_bytes()
    words = [int.from_bytes(shared[i:i + 2], 'little') for i in range(0, 4096, 2)]
    for address, word in enumerate(words):
        expected = {0o300: 12, 0o301: 0x33, 0o303: 2}.get(address, address ^ 0x7376)
        if address != 0o302:
            assert word == expected, (oct(address), word, expected)
    stack = (B / 'vice-stack.bin').read_bytes()
    changed = [i for i, byte in enumerate(stack) if byte != 0xCD]
    modules = json.loads((B / 'lm.json').read_text())['modules']
    sizes = {m_['id']: m_['bytes'] for m_ in modules}
    result = dict(mailbox=m.hex(), irq_count=m[5] | m[6] << 8,
        cold_job_load_cycles=cycles(0x08), cached_job_call_cycles=cycles(0x0B),
        cold_task_load_cycles=cycles(0x13), store_verify_cycles=cycles(0x10),
        job_module_bytes=sizes[0x0F02], task_module_bytes=sizes[0x0F03],
        software_stack_extent=2048 - min(changed) if changed else 0)
    (B / 'vice-result.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS: ABI 2 REU store on stock PAL C64 ROMs, $01=$35, RAM IRQ vectors live')
    for key, value in result.items():
        print(f'  {key}: {value}')


if __name__ == '__main__':
    main()
