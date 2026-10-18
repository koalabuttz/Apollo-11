#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""DAP pass timings in VICE (platform/c64/dap_pass_check.c): the LM flight
resident on the whole-program flight store, started from a native state the
host flight run wrote mid-flight (tools/flight_host_run.c FLIGHT_SNAPSHOT),
runs with CIA1 interrupts on for DP_CS centiseconds of AGC time. Prints wall
cycles per AGC second and, per AGC interrupt, cycles, steps and module loads,
and writes build/flight-store/dap-pass.json.

Scenarios (Lane F, /mnt/project-files/lane-f/dap-cycle-trace.md): channel 30
with IMU operate and PGNCS control, channel 31 AUTO, so DAPIDLER starts the
DAP; hold samples at SNAP_CS; firing pokes CDUY and CDUZ at SNAP_CS first.

Needs make flight-store (build/flight-store/lm.reu and the services), the
yaAGC objects and VICE (as tools/flight_vice_load.py).

  dap_pass_vice.py [--snap-cs CS] [--cs N] [--scenarios hold,firing]
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
sys.path.insert(0, str(ROOT / 'tools'))
from resident_flight_build import COMMON, MISSION, resident_sources  # noqa: E402
import flight_host_differential  # noqa: E402

OUT = ROOT / 'build/flight-store'
WORK = OUT / 'dap-pass'
PAL_HZ = 985248
SCRATCH_BYTES = 32768
CH30, CH31 = '036377', '057777'
SCENARIOS = {'hold': '', 'firing': '{cs}:033=02000,{cs}:034=077000'}
# platform/c64/resident_flight.c RK_PROF segments, in order.
SEGMENTS = ['check loop', 'rk_rupts', 'trampoline hit', 'trampoline miss', 'direct slot scan',
            'direct slot miss', 'direct hit (lookup + body)', 'direct miss tail', 'resolver lookup',
            'module run (load + main)', 'step return']
RUPTS = {0: 'background', 1: 'T6RUPT', 2: 'T5RUPT', 3: 'T3RUPT', 4: 'T4RUPT', 5: 'KEYRUPT1', 6: 'KEYRUPT2',
         7: 'UPRUPT', 8: 'DOWNRUPT', 9: 'RADARUPT', 10: 'HANDRUPT'}


def snapshot(exe, name, cs):
    header = WORK / f'{name}-snap.h'
    env = dict(os.environ, FLIGHT_CH30=CH30, FLIGHT_CH31=CH31, FLIGHT_SNAPSHOT=f'{cs},{header}')
    if SCENARIOS[name]:
        env['FLIGHT_POKE'] = SCENARIOS[name].format(cs=cs)
    r = subprocess.run([str(exe), flight_host_differential.ROPE['lm'], str(cs + 1)], cwd=ROOT, env=env,
                       capture_output=True, text=True)
    (WORK / f'{name}-host.log').write_text(r.stdout + r.stderr)
    if not header.exists():
        sys.exit(f'FAIL: no snapshot for {name} (see {WORK}/{name}-host.log)')
    return header


def link(cc, name, header, cs, scratch, steps=False, m='lm', profile=False, keys=None, watch=None, defines=()):
    out = WORK / f'{name}.prg'
    subprocess.run([str(c) for c in [
        cc, *COMMON, *MISSION[m], '-DRK_RUPT_TRACE', f'-DDP_SNAP="{header}"', f'-DDP_CS={cs}u',
        f'-DDP_SCRATCH=0x{scratch:x}ul', f'-DDP_SCRATCH_BYTES={SCRATCH_BYTES}ul', *(['-DDP_STEPS'] if steps else []), *(['-DRK_STEP_PROFILE'] if profile else []), *([f'-DDP_KEYS="{keys}"'] if keys else []),
        *([f"-DDP_WATCH={','.join('0' + w for w in watch)}"] if watch else []), *defines, '-T', 'platform/c64/resident_flight.ld', f'-Wl,-Map,{out}.map',
        'platform/c64/dap_pass_check.c', *resident_sources(m), OUT / 'overlay_v2_services.S', '-o', out]],
        cwd=ROOT, check=True)
    return out


def vice(a, name, prg, pinned, m='lm', patch=()):
    """patch: (REU address, bytes) written into the run's REU image copy
    (flight_scenario_vice.py --replay: the input rows)."""
    rom = a.rom_dir.resolve()
    work = WORK / f'vice-{name}'
    work.mkdir(parents=True, exist_ok=True)
    sys_at = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    dump = work / 'results.bin'
    dump.unlink(missing_ok=True)
    commands = work / 'vice.mon'
    # Boot's DMA of the class-3 module to $C000 is the first store there.
    commands.write_text(f'load "{prg}" 0\nwatch store c000\n' + ('ignore 1 1\n' if pinned else '')
                        + f'g {int(sys_at[1]):04x}\nbsave "{dump}" 0 c000 cbff\nquit\n')
    reu = work / 'image.reu'
    image = bytearray((OUT / f'{m}.reu').read_bytes())
    for at, data in patch:
        image[at:at + len(data)] = data
    reu.write_bytes(image)
    env = dict(os.environ, XDG_CACHE_HOME=str(work / 'cache'), XDG_CONFIG_HOME=str(work / 'config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        pathlib.Path(env[key]).mkdir(parents=True, exist_ok=True)
    cmd = [a.vice, '-default', '-console', '-pal', '-warp', '+sound',
           '-kernal', str(rom / 'kernal-901227-03.bin'), '-basic', str(rom / 'basic-901226-01.bin'),
           '-chargen', str(rom / 'chargen-901225-01.bin'),
           '-directory', f'{a.data_dir.resolve()}:{rom.parent}',
           '-initbreak', 'ready', '-moncommands', str(commands),
           '-monlogname', str(work / 'monitor.log'), '-monlog', '-limitcycles', str(a.cycles),
           '-reu', '-reusize', str(reu.stat().st_size // 1024), '-reuimage', str(reu), '-reuimagerw']
    with (work / 'vice.log').open('w') as log:
        subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, timeout=a.timeout, env=env)
    if not dump.exists():
        return dict(scenario=name, ok=False, error=f'no mailbox (cycle limit or crash; see {work}/vice.log)')
    data = dump.read_bytes()
    if data[0] != 0xa5:
        return dict(scenario=name, ok=False, error='mailbox not written')
    cycles, dilation, steps, start_cs, end_cs, count, z, fb, superbank, status, lost = struct.unpack_from(
        '<3I6H2B', data, 0x10)
    # The physical bank: FB 030-037 with superbank bit 0100 are banks 040-047.
    bank = (fb >> 10) + (0o10 if fb >= 0o60000 and superbank & 0o100 else 0)
    # Where Z is, as the listing writes it: the switched bank for 02000-03777,
    # bank 02 or 03 for fixed-fixed 04000-07777, else the erasable address.
    if 0o2000 <= z < 0o4000:
        at = f'{bank:02o},{z:04o}'
    elif 0o4000 <= z < 0o10000:
        at = f'{z >> 10:02o},{0o2000 + z % 1024:04o}'
    else:
        at = f'{z:05o}'
    # Every record is in the REU scratch (the store file's last SCRATCH_BYTES),
    # which VICE writes back to image.reu on exit (-reuimagerw); the dump
    # holds the first ones that fit below the profile or the stack canary.
    # lost: the scratch filled, or the image did not come back.
    keep = min(count, ((0xb80 if getattr(a, 'profile', False) else 0xc00) - 0x40) // 10)
    ram = [struct.unpack_from('<4BHI', data, 0x40 + 10 * i) for i in range(keep)]
    image = reu.read_bytes()
    scratch = len(image) - SCRATCH_BYTES
    rows = [struct.unpack_from('<4BHI', image, scratch + 10 * i) for i in range(count)]
    if rows[:keep] != ram:
        rows, lost = ram, lost or count > keep
    records = [dict(zip(('request', 'job_loads', 'task_loads', 'status', 'steps', 'cycles'), x)) for x in rows]
    for r in records:
        r['request'] = RUPTS.get(r['request'], str(r['request']))
    agc_cs = (end_cs - start_cs) & 0x3fff
    by = {}
    for r in records:
        b = by.setdefault(r['request'], dict(count=0, cycles=0, max_cycles=0, steps=0, job_loads=0, task_loads=0))
        b['count'] += 1
        b['cycles'] += r['cycles']
        b['max_cycles'] = max(b['max_cycles'], r['cycles'])
        for k in ('steps', 'job_loads', 'task_loads'):
            b[k] += r[k]
    watched = [struct.unpack_from('<H', data, 6 + 2 * i)[0] for i in range(5)]
    return dict(scenario=name, ok=status == 0 and not lost, watched=watched, status=status, z=f'{z:05o}', fb=f'{fb:05o}',
                superbank=f'{superbank:04o}', bank=f'{bank:02o}', at=at,
                wall_cycles=cycles, agc_cs=agc_cs, dropped_ticks=dilation, steps=steps,
                wall_per_agc=round(cycles / (agc_cs * PAL_HZ / 100), 3) if agc_cs else None,
                by_request=by, records=records, lost=bool(lost))


def profile(name):
    """The RK_STEP_PROFILE segments from a run's mailbox: cycles and calls each."""
    data = (WORK / f'vice-{name}' / 'results.bin').read_bytes()
    return [dict(zip(('segment', 'cycles', 'calls'), (SEGMENTS[i], *struct.unpack_from('<IH', data, 0xb80 + 6 * i))))
            for i in range(len(SEGMENTS))]


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--snap-cs', type=int, default=3000, help='AGC centisecond the run starts from')
    p.add_argument('--cs', type=int, default=100, help='AGC centiseconds to run on the C64')
    p.add_argument('--scenarios', default='hold,firing')
    p.add_argument('--profile', action='store_true', help='cycles per resident step segment (RK_STEP_PROFILE)')
    p.add_argument('--steps', action='store_true', help='one record per step (Z, FB bank, loads) for debugging')
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--cycles', type=int, default=3_000_000_000)
    p.add_argument('--timeout', type=int, default=1800)
    a = p.parse_args()
    WORK.mkdir(parents=True, exist_ok=True)
    subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--missions', 'lm',
                    '--host-cc', a.host_cc], cwd=ROOT, check=True)
    exe = flight_host_differential.build(a.host_cc, 'lm', flight_host_differential.frozen_engine(a.host_cc))
    manifest = json.loads((OUT / 'lm-manifest.json').read_text())
    pinned = any(m['slot'] == 3 for m in manifest['modules'])
    reu = (OUT / 'lm.reu').read_bytes()
    scratch = len(reu) - SCRATCH_BYTES
    if any(reu[scratch:]):
        sys.exit(f'FAIL: no unused {SCRATCH_BYTES} bytes at the end of lm.reu')
    results, failed = [], 0
    for name in a.scenarios.split(','):
        header = snapshot(exe, name, a.snap_cs)
        prg = link(a.cc, name, header, a.cs, scratch, a.steps, profile=a.profile)
        r = vice(a, name, prg, pinned)
        if a.profile and r.get('ok') is not None and (WORK / f'vice-{name}' / 'results.bin').exists():
            r['profile'] = profile(name)
            for x in r['profile']:
                print(f"  {x['segment']}: {x['calls']} x, {x['cycles']} cycles"
                      f" ({x['cycles'] // x['calls'] if x['calls'] else 0} each)")
        r.update(pinned=sorted(f"0x{m['id']:04x} class {m['slot']}" for m in manifest['modules'] if m['slot'] >= 3))
        results.append(r)
        if not r['ok']:
            failed += 1
            print(f"FAIL: {name}: {r.get('error') or 'status %d at %s (Z %s FB %s superbank %s)' % (r['status'], r['at'], r['z'], r['fb'], r['superbank'])}")
        if 'by_request' in r:
            print(f"{name}: {r['wall_cycles']} cycles for {r['agc_cs']} AGC cs ({r['wall_per_agc']} x real time), "
                  f"{r['dropped_ticks']} ticks dropped, {r['steps']} steps")
            for k, b in sorted(r['by_request'].items()):
                print(f"  {k}: {b['count']} x, mean {b['cycles'] // b['count']} cycles (max {b['max_cycles']}), "
                      f"{b['steps']} steps, {b['job_loads']} job and {b['task_loads']} task loads")
    (OUT / 'dap-pass.json').write_text(json.dumps(results, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
