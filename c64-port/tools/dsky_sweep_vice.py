#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The DSKY sweep on the C64 (acceptance item 6, docs/acceptance): DSKY key
sequences from a fresh start, each run on the C64 flight resident in VICE
with the host run's inputs (flight_scenario_vice.py --replay), and each
C64 end state compared with the host's: erasable words, timers and output
channels, so every display the DSKY shows (DSPTAB, channel 10 relay words
in the erasable) and every load the keys make.

Every case starts from one snapshot: the host flight run from a fresh
start, snapshotted at --snap-cs (1000 cs), is the same whatever keys come
after, because the keys go in from --key-cs on ('@1100' in the key string,
tools/flight_host_run.c keys_parse). Per case the host runs on from the
snapshot (FLIGHT_REPLAY) for the keys plus --settle-cs, and writes its
input rows and its end state.

On the C64 one VICE run takes a batch of cases (platform/c64/dap_pass_check.c
DP_SWEEP): it boots once, loads the snapshot, runs case 0's window with its
rows, stashes its end state, reloads the snapshot from the REU and runs the
next case. The case table, the rows, the snapshot copy and the end states
sit in the zeros below the Pinball save (tools/reu_layout.py), and the run's
REU bound check covers them.

Cases: --cases ci (a fixed sample: CI_CASES, both missions; the CI gate for
Pinball and display changes), --cases all (Lane C's
tools/dsky_sweep_cases.py cases(), about 2,400), or --keys K1,K2 (named key
strings). A case passes when the C64 ends its window at the host's sample
point in the host's state. A case whose host run bridges a point (a stop on
the C64: no yaAGC there) fails before VICE, with the point.

  dsky_sweep_vice.py [--missions lm,cm] [--cases ci|all] [--keys V16N36E,...] [--batch N] [--jobs N]

Needs make flight-store (both missions' stores and host runs), the yaAGC
objects and VICE, as flight_scenario_vice.py. Writes
build/flight-store/dsky-sweep/dsky-sweep.json.
"""
import argparse
import concurrent.futures
import json
import os
import pathlib
import shutil
import struct
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import dap_pass_vice as dp  # noqa: E402
import flight_holes  # noqa: E402
import flight_host_differential as fhd  # noqa: E402
import flight_scenario_vice as fsv  # noqa: E402
import reu_layout  # noqa: E402

WORK = dp.OUT / 'dsky-sweep'
# The CI sample: each display verb on a time noun, a mixed noun and an
# angle noun, the monitor verbs, the load verbs into a time and an angle,
# V32-V37, PROCEED-less V16 + V32, KEY REL, RSET and CLR, machine address
# display and increment (N01 then N15), and a fixed memory display.
CI_CASES = ['V01N36E', 'V04N36E', 'V05N09E', 'V06N36E', 'V06N43E', 'V02N65E', 'V03N20E',
            'V11N36E', 'V14N43E', 'V16N36E', 'V16N65E', 'V17N20E',
            'V21N01E00010E', 'V25N36E00001E00002E00003E', 'V22N20E',
            'V32E', 'V33E', 'V34E', 'V35E', 'V36E', 'V37E00E',
            'V16N36EV32E', 'V16N36EK', 'V16N36EKR', 'V05N09EC',
            'V01N01E01000EV01N15E', 'V27N02E02000E']
SNAP = 2 * 2048 + (fsv.RK_RESTART_LAMP - fsv.RK_HARDWARE)   # dap_pass_check.c sweep_snap, then rk_device
CASE = struct.Struct('<IHHIBBH')   # dap_pass_check.c dp_case


def all_cases():
    try:
        import dsky_sweep_cases
    except ImportError:
        sys.exit('FAIL: --cases all needs tools/dsky_sweep_cases.py (Lane C, #674)')
    return [(m, k) for m, k, _ in dsky_sweep_cases.cases()]


def bridged(text):
    """{point: arrivals} for the points a host run bridges (flight_holes.HOLE)."""
    return {f'{h[1].strip()} {h[2]},{h[3]}' + (f' refused by {h[6]}' if h[6] else ''): int(h[4])
            for h in flight_holes.HOLE.finditer(text)}


def host_start(exe, m, a):
    """The points the host run bridges up to the snapshot: before any C64
    window, so not stops."""
    return bridged(host_holes(exe, m, a.snap_cs, ''))


def host_holes(exe, m, cs, keys):
    """The host run's report to cs (a run with a snapshot or a replay stops
    without one)."""
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs), keys], cwd=ROOT, capture_output=True, text=True,
                       env={**os.environ, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SWAP_MCT': '0'})
    if r.returncode:
        sys.exit(f'FAIL: the {m} host run to {cs} cs with keys {keys!r} failed:\n' + (r.stdout + r.stderr)[-2000:])
    return r.stdout


def host_case(exe, m, i, keys, a, start):
    """The host run of one case: its snapshot, rows and end state, and the
    points it bridges in the window (flight_holes), each a stop on the C64."""
    header = WORK / f'{m}-{i}-snap.h'
    for x in ('', '.replay', '.end'):
        pathlib.Path(f'{header}{x}').unlink(missing_ok=True)
    window = a.key_cs - a.snap_cs + fsv.KEY_GAP * len(keys) + a.settle_cs
    env = {**os.environ, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SWAP_MCT': '0',
           'FLIGHT_SNAPSHOT': f'{a.snap_cs},{header}', 'FLIGHT_REPLAY': f'{window},0,{window},1'}
    r = subprocess.run([str(exe), fhd.ROPE[m], str(a.snap_cs + 1000 + window), f'@{a.key_cs}{keys}'], cwd=ROOT,
                       env=env, capture_output=True, text=True)
    found = bridged(host_holes(exe, m, a.snap_cs + window, f'@{a.key_cs}{keys}'))
    holes = [f'{x} ({n - start.get(x, 0)} arrivals)' for x, n in found.items() if n > start.get(x, 0)]
    end = pathlib.Path(f'{header}.end')
    if r.returncode or not end.exists():
        return dict(m=m, keys=keys, ok=False, error=f'the host run did not end its window (exit {r.returncode})',
                    log=(r.stdout + r.stderr)[-2000:])
    data = end.read_bytes()
    host_end = fsv.states(data, len(data) // fsv.STATE)[-1]
    return dict(m=m, keys=keys, header=str(header), window=window, holes=holes,
                rows=pathlib.Path(f'{header}.replay').read_bytes(), host_end=host_end, ok=True)


def link(a, m, name, header, table, snap, cs):
    defines = ['-DDP_REPLAY', '-DDP_REPLAY_REU=0ul', '-DDP_REPLAY_ROWS=1u', '-DDP_END=0ul',
               '-DDP_EVERY=037777u', '-DDP_EVERY_UNTIL=037777u', f'-DDP_SWEEP=0x{table:x}ul', f'-DDP_SWEEP_SNAP=0x{snap:x}ul']
    old = dp.WORK
    dp.WORK = WORK
    try:
        return dp.link(a.cc, name, header, cs, len((dp.OUT / f'{m}.reu').read_bytes()) - dp.SCRATCH_BYTES,
                       False, m, False, None, [], defines)
    finally:
        dp.WORK = old


def run_batch(a, m, n, batch, manifest, pinned):
    """One VICE run over a batch of host cases (all with the same snapshot);
    each case's result."""
    name = f'{m}-batch-{n}'
    reu = (dp.OUT / f'{m}.reu').read_bytes()
    top = reu_layout.free_top(m, manifest)
    # The rows (read-only) below; the case table, the snapshot copy and the
    # end states (written by the run) above.
    prg = None
    rows = [len(fsv.c64_rows(c['rows'], m, 0, 0)) for c in batch]
    rows_bytes = (sum(rows) + 0xff) & ~0xff
    writable = 2 + CASE.size * len(batch) + 64 + SNAP + fsv.C64_STATE * len(batch)
    at = fsv.replay_place(manifest, m, reu, rows_bytes + writable + 0x200)
    states_at = at + rows_bytes
    table = states_at
    snap = (table + 2 + CASE.size * len(batch) + 0xff) & ~0xff
    ends = (snap + SNAP + 64 + 0xff) & ~0xff
    if ends + fsv.C64_STATE * len(batch) > top:
        sys.exit(f'FAIL: batch {n} of {len(batch)} cases does not fit below 0x{top:x}; use a smaller --batch')
    cs = max(c['window'] for c in batch) + 500
    prg = link(a, m, name, batch[0]['header'], table, snap, cs)
    device, pending = fsv.map_symbol(prg, 'rk_device'), fsv.map_symbol(prg, 'rk_pending')
    data, tab, row_at = b'', struct.pack('<H', len(batch)), at
    for k, c in enumerate(batch):
        r = fsv.c64_rows(c['rows'], m, device, pending)
        tab += CASE.pack(row_at, len(r) // fsv.ROW.size, c['window'], ends + k * fsv.C64_STATE, 0, 0xff, 0)
        data += r
        row_at += len(r)
    patch = [(at, data), (table, tab)]
    regions = reu_layout.layout(m, manifest, (at, states_at, top))
    old = dp.WORK
    dp.WORK = WORK
    try:
        r = dp.vice(a, name, prg, pinned, m, patch, [reu_layout.dma_trace(regions)])
    finally:
        dp.WORK = old
    work = WORK / f'vice-{name}'
    image = (work / 'image.reu').read_bytes() if (work / 'image.reu').exists() else None
    out, bounds_ok = [], False
    if image is not None and r.get('status') is not None:
        before = bytearray(reu)
        for x, d in patch:
            before[x:x + len(d)] = d
        bounds = reu_layout.report(m, regions, before, image, name)
        bounds_ok = bounds['ok'] and not reu_layout.dma_misses((work / 'monitor.log').read_text())
    for k, c in enumerate(batch):
        res = dict(m=m, keys=c['keys'], window=c['window'], holes=c['holes'])
        if image is None or r.get('status') is None:
            res.update(ok=False, error=r.get('error', 'no VICE result'))
            out.append(res)
            continue
        _, nrows, _, _, ended, status, applied = CASE.unpack_from(image, table + 2 + k * CASE.size)
        res.update(rows=nrows, applied=applied, ended=bool(ended), status=status)
        if status == 0xff:
            res.update(ok=False, error='the batch stopped before this case')
        elif not ended:
            res.update(ok=False, error=f'the C64 stopped (status {status}) before a sample point '
                       f'{c["window"]} cs in ({applied} of {nrows} rows applied)')
        else:
            hc, hw = c['host_end']
            cc, cw = fsv.c64_states(image[ends + k * fsv.C64_STATE:], 1, m)[0]
            skipped = set()
            diff = fsv.replay_compare(hw, cw, fsv.SCHEDULE_SKIP.get(m), skipped)
            if hc != cc:
                diff.insert(0, ('centiseconds', hc, cc))
            res.update(ok=not diff, end_cs=cc, skipped=sorted(skipped), differences=len(diff),
                       first=[dict(at=x, host=f'{h:05o}', c64=f'{v:05o}') for x, h, v in diff[:16]])
        res['ok'] = res['ok'] and bounds_ok
        if not bounds_ok:
            res.setdefault('error', 'REU access outside the declared layout')
        out.append(res)
    if not a.keep:
        shutil.rmtree(work, ignore_errors=True)
    return out, r


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--missions', default='lm,cm')
    p.add_argument('--cases', default='ci', choices=('ci', 'all'))
    p.add_argument('--keys', default='', help='key strings, comma separated, instead of --cases')
    p.add_argument('--every', type=int, default=1, help='with --cases all, every Nth case')
    p.add_argument('--snap-cs', type=int, default=1000)
    p.add_argument('--key-cs', type=int, default=1100, help='the first key, in AGC cs from the fresh start')
    p.add_argument('--settle-cs', type=int, default=150, help='cs the window runs past the last key')
    p.add_argument('--batch', type=int, default=16, help='cases per VICE run')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 1)
    p.add_argument('--skip-build', action='store_true', help='use the built stores and host runs as they are')
    p.add_argument('--keep', action='store_true', help="keep each batch's VICE directory")
    p.add_argument('--known', type=pathlib.Path,
                   help='a JSON list of cases known not to match yet ({"lm V16N36E": "why", ...}): the run fails '
                   'only on a failure not listed, or on a listed case that now matches (take it off the list)')
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--cycles', type=int, default=20_000_000_000)
    p.add_argument('--timeout', type=int, default=7200)
    a = p.parse_args()
    WORK.mkdir(parents=True, exist_ok=True)
    missions = [m for m in a.missions.split(',') if m]
    if a.keys:
        cases = [(m, k) for m in missions for k in a.keys.split(',') if k]
    elif a.cases == 'ci':
        cases = [(m, k) for m in missions for k in CI_CASES]
    else:
        cases = [c for c in all_cases()[::a.every] if c[0] in missions]
    results, runs = [], []
    for m in missions:
        mine = [k for x, k in cases if x == m]
        if not mine:
            continue
        if not a.skip_build:
            subprocess.run([sys.executable, 'tools/flight_store_build.py', '--missions', m, '--host-cc', a.host_cc],
                           cwd=ROOT, check=True)
            subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--missions', m,
                            '--host-cc', a.host_cc], cwd=ROOT, check=True)
        exe = fhd.build(a.host_cc, m, fhd.frozen_engine(a.host_cc))
        manifest = json.loads((dp.OUT / f'{m}-manifest.json').read_text())
        pinned = any(mod['slot'] == 3 for mod in manifest['modules'])
        start = host_start(exe, m, a)
        with concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
            host = list(pool.map(lambda x: host_case(exe, m, *x, a, start), enumerate(mine)))
        good = []
        snap0 = None
        for h in host:
            if not h['ok']:
                results.append(dict(m=m, keys=h['keys'], ok=False, error=h['error']))
                print(f"FAIL: {m} {h['keys']}: {h['error']}")
                continue
            snap = fsv.snap_state(h['header'])
            snap0 = snap0 or snap
            if snap != snap0:
                sys.exit(f"FAIL: {m} {h['keys']}: the host snapshot at {a.snap_cs} cs differs from the first case's "
                         '(the keys must come after it: raise --key-cs)')
            if h['holes']:
                results.append(dict(m=m, keys=h['keys'], ok=False, holes=h['holes'],
                                    error=f"the host run bridges {len(h['holes'])} points, each a stop on the C64"))
                print(f"FAIL: {m} {h['keys']}: the host run bridges {h['holes'][0]}"
                      + (f" and {len(h['holes']) - 1} more" if len(h['holes']) > 1 else ''))
                continue
            good.append(h)
        batches = [good[i:i + a.batch] for i in range(0, len(good), a.batch)]
        print(f'{m}: {len(mine)} cases, {len(good)} with no bridged host point, in {len(batches)} VICE runs')
        with concurrent.futures.ThreadPoolExecutor(min(a.jobs, len(batches) or 1)) as pool:
            for out, r in pool.map(lambda x: run_batch(a, m, x[0], x[1], manifest, pinned), enumerate(batches)):
                runs.append(dict(m=m, wall_cycles=r.get('wall_cycles'), agc_cs=r.get('agc_cs'), error=r.get('error')))
                for res in out:
                    results.append(res)
                    print(f"{'ok  ' if res['ok'] else 'FAIL'} {m} {res['keys']}: "
                          + (f"matches the host at {res['end_cs']} cs" + (f" (skipped: {', '.join(res['skipped'])})"
                                                                       if res['skipped'] else '')
                             if res['ok'] else res.get('error') or
                             f"{res['differences']} differences at {res['end_cs']} cs, first "
                             + ', '.join(f"{x['at']} host {x['host']} C64 {x['c64']}" for x in res['first'][:4])))
    for h in WORK.glob('*-snap.h*'):
        h.unlink()
    ok = sum(1 for r in results if r['ok'])
    (WORK / 'dsky-sweep.json').write_text(json.dumps(dict(passed=ok, cases=len(results), results=results, runs=runs),
                                                     indent=1) + '\n')
    print(f"{'PASS' if ok == len(results) and results else 'FAIL'}: DSKY sweep on the C64: {ok} of {len(results)} "
          'cases end in the host state')
    if not a.known:
        return 0 if ok == len(results) and results else 1
    known = json.loads(a.known.read_text())
    new = [f"{r['m']} {r['keys']}" for r in results if not r['ok'] and f"{r['m']} {r['keys']}" not in known]
    fixed = [f"{r['m']} {r['keys']}" for r in results if r['ok'] and f"{r['m']} {r['keys']}" in known]
    for x in new:
        print(f'FAIL: {x} does not match the host and is not in {a.known.name}')
    for x in fixed:
        print(f'FAIL: {x} now matches the host: take it off {a.known.name}')
    print(f"{'PASS' if results and not new and not fixed else 'FAIL'}: against {a.known.name}: "
          f'{len(results) - ok} known failures, {len(new)} new, {len(fixed)} fixed')
    return 0 if results and not new and not fixed else 1


if __name__ == '__main__':
    sys.exit(main())
