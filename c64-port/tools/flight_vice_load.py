#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run the flight-store load check in VICE (docs/FLIGHT-STORE.md): the flight
resident boots on the whole-program store (build/flight-store/<m>.reu),
resolves every resolver record and times each flight module's load from the
REU (platform/c64/flight_load_check.c). Checks the standard $c000 mailbox
and writes the timings to build/flight-store/vice-load.json.

  flight_vice_load.py [--missions lm,cm]
"""
import argparse
import json
import os
import pathlib
import re
import struct
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/flight-store'
PAL_HZ = 985248


def run(m, a):
    prg = OUT / f'{m}-load-check.prg'
    rom = a.rom_dir.resolve()
    work = ROOT / 'build' / f'vice-{m}-load-check'
    work.mkdir(parents=True, exist_ok=True)
    sys_at = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    if sys_at is None:
        sys.exit(f'FAIL: {prg.name} has no BASIC SYS header')
    dump = work / 'results.bin'
    dump.unlink(missing_ok=True)
    (work / 'monitor.log').unlink(missing_ok=True)
    commands = work / 'vice.mon'
    # With a pinned module, boot's DMA of it to $C000 is the first store
    # there; the mailbox's done flag is the next.
    pinned = '#define FL_PINNED ' in (OUT / f'{m}-load-check.h').read_text()
    commands.write_text(f'load "{prg}" 0\nwatch store c000\n' + ('ignore 1 1\n' if pinned else '')
                        + f'g {int(sys_at[1]):04x}\n'
                        f'bsave "{dump}" 0 c000 cbff\nquit\n')
    reu = work / 'image.reu'
    reu.write_bytes((OUT / f'{m}.reu').read_bytes())
    cmd = [a.vice, '-default', '-console', '-pal', '-warp', '+sound',
           '-kernal', str(rom / 'kernal-901227-03.bin'), '-basic', str(rom / 'basic-901226-01.bin'),
           '-chargen', str(rom / 'chargen-901225-01.bin'),
           '-directory', f'{a.data_dir.resolve()}:{rom.parent}',
           '-initbreak', 'ready', '-moncommands', str(commands),
           '-monlogname', str(work / 'monitor.log'), '-monlog', '-limitcycles', str(a.cycles),
           '-reu', '-reusize', str(reu.stat().st_size // 1024), '-reuimage', str(reu)]
    env = dict(os.environ, XDG_CACHE_HOME=str(work / 'cache'), XDG_CONFIG_HOME=str(work / 'config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        pathlib.Path(env[key]).mkdir(parents=True, exist_ok=True)
    with (work / 'vice.log').open('w') as log:
        subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, timeout=a.timeout, env=env)
    if not dump.exists():
        return dict(mission=m, ok=False, error=f'no mailbox (cycle limit or crash; see {work}/vice.log)')
    data = dump.read_bytes()
    failures, checks = data[1] | data[2] << 8, data[3] | data[4] << 8
    ok = data[0] == 0xa5 and not failures and checks > 0
    overhead = struct.unpack_from('<I', data, 0x3f8)[0]
    count = struct.unpack_from('<H', data, 0x3fe)[0]
    differ = set(struct.unpack_from('<8H', data, 0x3e0)[:data[0x3df]])
    modules = []
    # Module ids and sizes in check order, from the check's data header.
    listed = re.findall(r'\{0x([0-9a-f]+)u,(\d+)u,', (OUT / f'{m}-load-check.h').read_text().split('fl_modules', 1)[1])
    for i in range(count):
        ident, size = int(listed[i][0], 16), int(listed[i][1])
        load, cached, direct, main = struct.unpack_from('<IHHH', data, 0x400 + 10 * i)
        load, cached = load - overhead, cached - overhead
        modules.append(dict(id=f'0x{ident:04x}', bytes=size, load_cycles=load, cached_cycles=cached,
                            direct_cycles=direct - overhead if direct else None,
                            main_cycles=main - overhead if direct else None,
                            direct_differs=i in differ,
                            transfer_cycles=load - cached, cycles_per_byte=round((load - cached) / size, 2)))
    run_only, point, resolve = (v - overhead for v in struct.unpack_from('<3H', data, 0x3f0))
    return dict(mission=m, ok=ok, failures=failures, checks=checks, first_failing_step=data[5], pinned_module=pinned,
                timer_overhead_cycles=overhead, soft_stack_peak_bytes=struct.unpack_from('<H', data, 0x3f6)[0],
                call_profile=dict(runtime_cached_run=run_only, resident_transfer_point=point,
                                  resident_transfer_resolve=resolve),
                modules=modules)


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--missions', default='lm,cm')
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--cycles', type=int, default=3_000_000_000)
    p.add_argument('--timeout', type=int, default=1800)
    a = p.parse_args()
    results = [run(m, a) for m in a.missions.split(',')]
    (OUT / 'vice-load.json').write_text(json.dumps(results, indent=1) + '\n')
    # Load cost against module size: a least-squares line over both missions.
    xs = [(x['bytes'], x['transfer_cycles']) for r in results if r['ok'] for x in r['modules']]
    if len(xs) > 2:
        mx, my = sum(b for b, _ in xs) / len(xs), sum(t for _, t in xs) / len(xs)
        slope = sum((b - mx) * (t - my) for b, t in xs) / sum((b - mx) ** 2 for b, _ in xs)
        fit = dict(fixed_cycles=round(my - slope * mx), cycles_per_byte=round(slope, 3),
                   max_residual_cycles=round(max(abs(t - (my - slope * mx) - slope * b) for b, t in xs)))
        print(f"load = {fit['fixed_cycles']} + {fit['cycles_per_byte']} cycles per byte over a cached call "
              f"(max residual {fit['max_residual_cycles']}), {PAL_HZ} cycles per second")
        results.append(dict(fit=fit))
        (OUT / 'vice-load.json').write_text(json.dumps(results, indent=1) + '\n')
    failed = 0
    for r in results:
        if 'fit' in r:
            continue
        if not r['ok']:
            failed += 1
            print(f"FAIL: {r['mission']} load check: {r.get('error') or r}")
            continue
        mods = r['modules'] or [dict(load_cycles=0, cached_cycles=0)]
        print(f"PASS: VICE PAL {r['mission']} load check: {r['checks']} checks, 0 failures; "
              f"{len(mods)} modules, load and call {min(x['load_cycles'] for x in mods)}-"
              f"{max(x['load_cycles'] for x in mods)} cycles "
              f"({max(x['load_cycles'] for x in mods) * 1000 / PAL_HZ:.1f} ms max), cached call "
              f"{min(x['cached_cycles'] for x in mods)}-{max(x['cached_cycles'] for x in mods)}"
              + (f", direct call saves {min(d)}-{max(d)} cycles over the module main ({len(d)} modules match)"
                 if (d := [x['main_cycles'] - x['direct_cycles'] for x in mods
                           if x.get('direct_cycles') is not None]) else '')
              + f"; soft stack peak {r['soft_stack_peak_bytes']} bytes")
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
