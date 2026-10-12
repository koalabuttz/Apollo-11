#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run a diagnostic PRG in VICE x64sc and check the standard $c000 mailbox.

Mailbox contract for new milestones (see CONTRIBUTING-port.md):
  $c000 = $a5 when the diagnostic finished, $c001/$c002 = failure count (LE),
  $c003/$c004 = number of checks run (LE). Anything else fails.

  python3 tools/vice_run.py build/<id>.prg
Defaults come from deps/env.sh (VICE, VICE_ROM_DIR, VICE_DATA_DIR).
"""
import argparse
import os
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('prg', type=pathlib.Path)
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--cycles', type=int, default=400_000_000)
    p.add_argument('--timeout', type=int, default=300)
    a = p.parse_args()
    prg = a.prg.resolve()
    rom = a.rom_dir.resolve()
    work = ROOT / 'build' / ('vice-' + prg.stem)
    work.mkdir(parents=True, exist_ok=True)
    match = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    if match is None:
        sys.exit('FAIL: PRG has no BASIC SYS header')
    mailbox = work / 'mailbox.bin'
    mailbox.unlink(missing_ok=True)
    (work / 'monitor.log').unlink(missing_ok=True)   # VICE appends; keep one session per run
    commands = work / 'vice.mon'
    commands.write_text(f'load "{prg}" 0\nwatch store c000\ng {int(match[1]):04x}\n'
                        f'm c000 c004\nbsave "{mailbox}" 0 c000 c004\nquit\n')
    cmd = [a.vice, '-default', '-console', '-pal', '-warp', '+sound',
           '-kernal', str(rom / 'kernal-901227-03.bin'), '-basic', str(rom / 'basic-901226-01.bin'),
           '-chargen', str(rom / 'chargen-901225-01.bin'),
           '-directory', f'{a.data_dir.resolve()}:{rom.parent}',
           '-initbreak', 'ready', '-moncommands', str(commands),
           '-monlogname', str(work / 'monitor.log'), '-monlog', '-limitcycles', str(a.cycles)]
    env = dict(os.environ, XDG_CACHE_HOME=str(work / 'cache'), XDG_CONFIG_HOME=str(work / 'config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        pathlib.Path(env[key]).mkdir(parents=True, exist_ok=True)
    with (work / 'vice.log').open('w') as log:
        subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, timeout=a.timeout, env=env)
    if not mailbox.exists():
        sys.exit(f'FAIL: {prg.name}: VICE produced no mailbox (cycle limit or crash; see {work}/vice.log)')
    data = mailbox.read_bytes()
    failures = data[1] | data[2] << 8
    checks = data[3] | data[4] << 8
    if len(data) != 5 or data[0] != 0xa5 or failures or not checks:
        sys.exit(f'FAIL: {prg.name}: mailbox {data.hex()} ({failures} failures, {checks} checks)')
    print(f'PASS: VICE PAL {prg.name}: {checks} checks, 0 failures')
    return 0


if __name__ == '__main__':
    sys.exit(main())
