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

Cases: --cases ci (a fixed sample: CI_CASES; the CI gate for Pinball and
display changes), --cases all (Lane C's tools/dsky_sweep_cases.py cases(),
about 2,400), or --keys K1,K2 (named key strings). Keys are
tools/flight_host_run.c's, plus Q for PRO (channel 32 bit 14, which is no
key code: the host gets it as a FLIGHT_CHAN press). A case passes when the
C64 ends its window at the host's sample point in the host's state. A case
fails before VICE when its host run bridges a point (a stop on the C64: no
yaAGC there) or has a native routine yaAGC did not match after the snapshot
(the host itself is wrong there), naming the point or the routine.

Each failure gets a class and a signature from the tool, not by hand:
clock (only the centiseconds and TIME1-TIME6 differ, each by one count the
same way, TIME2 only with TIME1),
difference (any other word or channel), stop, host-miss or run. --known
FILE lists the failures not fixed yet with their signatures; the run fails
on a failure not listed, a listed one that now matches, one whose signature
changed, and a listed one of a run mission the run did not exercise.
--write-known FILE writes that list from the run (keeping the reasons).

Acceptance item 6 is met only by --cases all on both missions with no
failure at all, clock class included, committed as evidence
(--evidence DIR: dsky-sweep.json with the store, rope and program hashes).

  dsky_sweep_vice.py [--missions lm,cm] [--cases ci|all] [--keys V16N36E,...] [--batch N] [--jobs N]
                    [--known FILE] [--write-known FILE] [--evidence DIR]

Needs make flight-store (both missions' stores and host runs), the yaAGC
objects and VICE, as flight_scenario_vice.py. Writes
build/flight-store/dsky-sweep/dsky-sweep.json.
"""
import argparse
import concurrent.futures
import hashlib
import json
import os
import pathlib
import re
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
# V32-V37, V16 + V32, KEY REL, RSET and CLR, PROCEED, machine address
# display and increment (N01 then N15), a fixed memory display, the octal
# component verbs V12, V13, V15, V23 and V24, and the decimal and DP3OUTSF
# display rows of #674 (V06N20, V06N21, V05N20; V06N49 on the LM).
CI_CASES = ['V01N36E', 'V04N36E', 'V05N09E', 'V06N36E', 'V06N43E', 'V02N65E', 'V03N20E',
            'V11N36E', 'V12N36E', 'V13N36E', 'V14N43E', 'V15N36E', 'V16N36E', 'V16N65E', 'V17N20E',
            'V21N01E00010E', 'V23N36E00003E', 'V24N36E00001E00002E', 'V25N36E00001E00002E00003E', 'V22N20E',
            'V06N20E', 'V06N21E', 'V05N20E', 'lm:V06N49E',
            'V32E', 'V33E', 'V34E', 'V35E', 'V36E', 'V37E00E',
            'V16N36EV32E', 'V16N36EK', 'V16N36EKR', 'V05N09EC', 'V16N36EQ', 'V05N09EQ',
            'V01N01E01000EV01N15E', 'V27N02E02000E']
# The C64's sweep copy (dap_pass_check.c sweep_snap): RK_HARDWARE to the
# image's end, then rk_device (at most 64 bytes; the 64 bytes of slack
# after SNAP in run_batch).
IMAGE_END = 0x8000
SNAP = IMAGE_END - fsv.RK_HARDWARE
DEVICE_MAX = 64
TIMER_NAMES = {'centiseconds', *fsv.TIMERS.values()}
MISS = re.compile(r'^  (interp|basic ) (\d+,\d+) (\S+): (\d+) times;', re.M)
PRO_BIT = 0o40000  # channel 32 bit 14, 0 while PRO is pressed
PRO_HOLD = 30      # cs PRO stays down
CASE = struct.Struct('<IHHIBBH')   # dap_pass_check.c dp_case


def ci_cases(missions):
    return [(m, k.split(':', 1)[-1]) for m in missions for k in CI_CASES if ':' not in k or k.split(':')[0] == m]


def host_keys(keys, key_cs, channel32):
    """The host's key argument and FLIGHT_CHAN for a sweep key string: each
    Q (PRO) takes a key's slot as channel 32 bit 14 down for PRO_HOLD cs."""
    out, chan, at = f'@{key_cs}', [], key_cs
    for k in keys:
        if k == 'Q':
            chan += [f'{at}:32={channel32 & ~PRO_BIT & 0o77777:o}', f'{at + PRO_HOLD}:32={channel32:o}']
            at += fsv.KEY_GAP
            out += f'@{at}'
        else:
            out += k
            at += fsv.KEY_GAP
    return out, ','.join(chan)


def all_cases():
    try:
        import dsky_sweep_cases
    except ImportError:
        sys.exit('FAIL: --cases all needs tools/dsky_sweep_cases.py (Lane C, #674)')
    full = getattr(dsky_sweep_cases, 'all_cases', None)
    return [(c[0], c[1] + (f'|{c[3]}' if len(c) > 3 and c[3] else '')) for c in (full or dsky_sweep_cases.cases)()]


def case_chan(chan, key_cs):
    """A case's own FLIGHT_CHAN (Lane C's pro_cases: CS:CH=VALUE, timed as
    a run whose keys start at fsv.KEY_AT) moved to the sweep's key time."""
    out = []
    for x in filter(None, chan.split(',')):
        t, rest = x.split(':', 1)
        out.append((int(t) - fsv.KEY_AT + key_cs, rest))
    return out


def bridged(text):
    """{point: arrivals} for the points a host run bridges (flight_holes.HOLE)."""
    return {f'{h[1].strip()} {h[2]},{h[3]}' + (f" refused by {h['refused']}" if h['refused'] else ''): int(h['arrivals'])
            for h in flight_holes.HOLE.finditer(text)}


def misses(text):
    """{routine: times} for the native routines yaAGC did not match in a
    host run (tools/flight_host_run.c lockstep)."""
    found = {f'{x[1].strip()} {x[2]} {x[3]}': int(x[4]) for x in MISS.finditer(text)}
    n = re.search(r'native routines yaAGC did not match \((\d+)\)', text)
    if not n:
        sys.exit('FAIL: no "native routines yaAGC did not match" line in a host run')
    if int(n[1]) != len(found):
        sys.exit(f'FAIL: a host run reports {n[1]} yaAGC misses but lists {len(found)}')
    return found


def host_start(exe, m, a):
    """The points the host run bridges and the routines yaAGC did not match
    up to the snapshot: before any C64 window, so not this sweep's."""
    text = host_holes(exe, m, a.snap_cs, '')
    return bridged(text), misses(text)


def host_holes(exe, m, cs, keys, chan=''):
    """The host run's report to cs (a run with a snapshot or a replay stops
    without one)."""
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs), keys], cwd=ROOT, capture_output=True, text=True,
                       env={**os.environ, 'FLIGHT_SCALED': '1,1,2', **({'FLIGHT_CHAN': chan} if chan else {})})
    if r.returncode:
        sys.exit(f'FAIL: the {m} host run to {cs} cs with keys {keys!r} failed:\n' + (r.stdout + r.stderr)[-2000:])
    return r.stdout


def host_case(exe, m, i, case, a, start, channel32):
    """The host run of one case: its snapshot, rows and end state, the
    points it bridges in the window (flight_holes), each a stop on the C64,
    and the routines yaAGC did not match in the window."""
    header = WORK / f'{m}-{i}-snap.h'
    for x in ('', '.replay', '.end'):
        pathlib.Path(f'{header}{x}').unlink(missing_ok=True)
    keys, _, own = case.partition('|')
    own = case_chan(own, a.key_cs)
    last = max([a.key_cs + fsv.KEY_GAP * len(keys)] + [t for t, _ in own])
    window = last - a.snap_cs + a.settle_cs
    hkeys, chan = host_keys(keys, a.key_cs, channel32)
    if own:
        # One FLIGHT_CHAN in time order: the Q presses and the case's own.
        both = [(int(x.split(':', 1)[0]), x.split(':', 1)[1]) for x in filter(None, chan.split(','))] + own
        chan = ','.join(f'{t}:{v}' for t, v in sorted(both, key=lambda x: x[0]))
    env = {**os.environ, 'FLIGHT_SCALED': '1,1,2',
           'FLIGHT_SNAPSHOT': f'{a.snap_cs},{header}', 'FLIGHT_REPLAY': f'{window},0,{window},1',
           **({'FLIGHT_CHAN': chan} if chan else {})}
    r = subprocess.run([str(exe), fhd.ROPE[m], str(a.snap_cs + 1000 + window), hkeys], cwd=ROOT,
                       env=env, capture_output=True, text=True)
    text = host_holes(exe, m, a.snap_cs + window, hkeys, chan)
    found, missed = bridged(text), misses(text)
    holes = [f'{x} ({n - start[0].get(x, 0)} arrivals)' for x, n in found.items() if n > start[0].get(x, 0)]
    yaagc = [f'{x} ({n - start[1].get(x, 0)} times)' for x, n in missed.items() if n > start[1].get(x, 0)]
    end = pathlib.Path(f'{header}.end')
    if r.returncode or not end.exists():
        return dict(m=m, keys=case, ok=False, error=f'the host run did not end its window (exit {r.returncode})',
                    log=(r.stdout + r.stderr)[-2000:])
    data = end.read_bytes()
    host_end = fsv.states(data, len(data) // fsv.STATE)[-1]
    return dict(m=m, keys=case, header=str(header), window=window, holes=holes, misses=yaagc,
                rows=pathlib.Path(f'{header}.replay').read_bytes(), host_end=host_end, ok=True)


def counts_apart(name, h, c):
    """The C64's lead over the host in a clock word one count apart (+1 or
    -1; 0 when not): the centiseconds, or a timer (a 15-bit counter word, so
    0 and 077777 are one apart too)."""
    d = c - h if name == 'centiseconds' else {1: 1, 0o77777: -1}.get((c - h) % 0o100000, 0)
    return d if d in (1, -1) else 0


def clock_only(diff):
    """Only the clock one count apart, all one way: the centiseconds and
    timers, each a count ahead (or each behind), TIME2 only with TIME1 (it
    is TIME1's carry)."""
    names = {x[0] for x in diff}
    ways = {counts_apart(*x) if x[0] in TIMER_NAMES else 0 for x in diff}
    return bool(diff) and len(ways) == 1 and 0 not in ways and ('TIME2' not in names or 'TIME1' in names)


def classify(res):
    """A failed case's class and signature: what the known list keys on."""
    if res.get('holes'):
        return 'stop', 'stop: ' + ', '.join(sorted(x.rsplit(' (', 1)[0] for x in res['holes']))
    if res.get('misses'):
        return 'host-miss', 'host-miss: ' + ', '.join(sorted(x.rsplit(' (', 1)[0] for x in res['misses']))
    if 'diff' not in res:
        return 'run', 'run: ' + re.sub(r'\d+ of \d+ rows applied', 'rows', res.get('error', 'no result'))
    names = sorted(x[0] for x in res['diff'])
    if clock_only(res['diff']):
        return 'clock', 'clock: ' + ', '.join(names)
    return 'difference', 'difference: ' + ', '.join(names)


def snap_text(header):
    """A snapshot header as the C64 loads it: every line but the comment
    line (which names the centisecond it was taken at)."""
    return [x for x in pathlib.Path(header).read_text().splitlines() if not x.startswith('/*')]


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def link(a, m, name, header, table, snap, cs):
    defines = ['-DDP_REPLAY', '-DDP_REPLAY_REU=0ul', '-DDP_REPLAY_ROWS=1u', '-DDP_END=0ul',
               '-DDP_STATE_FIRST=0xffffu', '-DDP_STATE_TABLE=0ul', '-DDP_STATES_MAX=1u',
               '-DDP_OUTPUTS=0ul', '-DDP_OUTPUTS_MAX=0u', f'-DDP_SWEEP=0x{table:x}ul', f'-DDP_SWEEP_SNAP=0x{snap:x}ul']
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
    writable = 2 + CASE.size * len(batch) + DEVICE_MAX + SNAP + fsv.C64_STATE * len(batch)
    at = fsv.replay_place(manifest, m, reu, rows_bytes + writable + 0x200)
    states_at = at + rows_bytes
    table = states_at
    snap = (table + 2 + CASE.size * len(batch) + 0xff) & ~0xff
    ends = (snap + SNAP + DEVICE_MAX + 0xff) & ~0xff
    if ends + fsv.C64_STATE * len(batch) > top:
        sys.exit(f'FAIL: batch {n} of {len(batch)} cases does not fit below 0x{top:x}; use a smaller --batch')
    cs = max(c['window'] for c in batch) + 500
    prg = link(a, m, name, batch[0]['header'], table, snap, cs)
    device, pending = fsv.map_symbol(prg, 'rk_device'), fsv.map_symbol(prg, 'rk_pending')
    size = device_size(prg)
    if size > DEVICE_MAX:
        sys.exit(f'FAIL: rk_device is {size} bytes, more than the {DEVICE_MAX} the sweep copy leaves for it')
    data, tab, row_at = b'', struct.pack('<H', len(batch)), at
    for k, c in enumerate(batch):
        r = fsv.c64_rows(c['rows'], m, device, pending)
        tab += CASE.pack(row_at, len(r) // fsv.ROW.size, c['host_end'][0], ends + k * fsv.C64_STATE, 0, 0xff, 0)
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
        res = dict(m=m, keys=c['keys'], window=c['window'], holes=c['holes'], misses=c['misses'])
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
            res.update(ok=not diff, end_cs=cc, skipped=sorted(skipped), differences=len(diff), diff=diff,
                       first=[dict(at=x, host=f'{h:05o}', c64=f'{v:05o}') for x, h, v in diff[:16]])
        res['ok'] = res['ok'] and bounds_ok
        if not bounds_ok:
            res.setdefault('error', 'REU access outside the declared layout')
        out.append(res)
    r['prg_sha256'] = sha(prg) if pathlib.Path(prg).exists() else None
    if not a.keep:
        shutil.rmtree(work, ignore_errors=True)
    return out, r


def device_size(prg):
    """rk_device's size in a link's lld map (the line's size column)."""
    for line in pathlib.Path(f'{prg}.map').read_text().splitlines():
        p = line.split()
        if len(p) == 5 and p[4] == 'rk_device':
            return int(p[2], 16)
    sys.exit(f'FAIL: no rk_device in {prg}.map')


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
                   help='the failures not fixed yet ({"lm V16N36E": {"signature": ..., "why": ...}, ...}): the run '
                   'fails on a failure not listed, a listed one that now matches or whose signature changed, and '
                   'a listed case of a run mission the run did not exercise')
    p.add_argument('--write-known', type=pathlib.Path,
                   help="write the run's failures as a known list (keeping the reasons of --known's entries)")
    p.add_argument('--evidence', type=pathlib.Path, help='also copy dsky-sweep.json here (evidence)')
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
        cases, mode = [(m, k) for m in missions for k in a.keys.split(',') if k], 'keys'
    elif a.cases == 'ci':
        cases, mode = ci_cases(missions), 'ci'
    else:
        cases, mode = [c for c in all_cases()[::a.every] if c[0] in missions], 'all' if a.every == 1 else 'every'
    results, runs, stores = [], [], {}
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
        stores[m] = dict(reu_sha256=sha(dp.OUT / f'{m}.reu'), rope_sha256=sha(fhd.ROPE[m]),
                         manifest_sha256=sha(dp.OUT / f'{m}-manifest.json'))
        start = host_start(exe, m, a)
        channel32 = snap_channel32(exe, m, a)
        with concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
            host = list(pool.map(lambda x: host_case(exe, m, *x, a, start, channel32), enumerate(mine)))
        good = []
        snap0 = None
        for h in host:
            if not h['ok']:
                results.append(dict(m=m, keys=h['keys'], ok=False, error=h['error']))
                print(f"FAIL: {m} {h['keys']}: {h['error']}")
                continue
            # The C64 loads the first case's header for the whole batch: every
            # case's must be the same, byte for byte.
            snap = snap_text(h['header'])
            snap0 = snap0 or snap
            if snap != snap0:
                sys.exit(f"FAIL: {m} {h['keys']}: the host snapshot at {a.snap_cs} cs differs from the first case's "
                         '(the keys must come after it: raise --key-cs)')
            if h['misses']:
                results.append(dict(m=m, keys=h['keys'], ok=False, holes=[], misses=h['misses'],
                                    error=f"{len(h['misses'])} native routines yaAGC did not match on the host"))
                print(f"FAIL: {m} {h['keys']}: yaAGC did not match {h['misses'][0]} on the host"
                      + (f" and {len(h['misses']) - 1} more" if len(h['misses']) > 1 else ''))
                continue
            if h['holes']:
                results.append(dict(m=m, keys=h['keys'], ok=False, holes=h['holes'], misses=[],
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
    for r in results:
        if not r['ok']:
            r['class'], r['signature'] = classify(r)
        if 'diff' in r:
            r['differing'] = [x[0] for x in r.pop('diff')]
    ok = sum(1 for r in results if r['ok'])
    classes = {}
    for r in results:
        if not r['ok']:
            classes[r['class']] = classes.get(r['class'], 0) + 1
    report = dict(mode=mode, missions=missions, every=a.every, snap_cs=a.snap_cs, key_cs=a.key_cs,
                  settle_cs=a.settle_cs, passed=ok, cases=len(results), failed_by_class=classes, stores=stores,
                  item6=mode == 'all' and set(missions) == {'lm', 'cm'} and ok == len(results) and bool(results),
                  results=results, runs=runs)
    (WORK / 'dsky-sweep.json').write_text(json.dumps(report, indent=1) + '\n')
    if a.evidence:
        a.evidence.mkdir(parents=True, exist_ok=True)
        shutil.copy(WORK / 'dsky-sweep.json', a.evidence / 'dsky-sweep.json')
    print(f"{'PASS' if ok == len(results) and results else 'FAIL'}: DSKY sweep on the C64: {ok} of {len(results)} "
          'cases end in the host state' + (f" (failed: {', '.join(f'{n} {c}' for c, n in sorted(classes.items()))})"
                                           if classes else ''))
    known = json.loads(a.known.read_text()) if a.known else {}
    if a.write_known:
        a.write_known.write_text(json.dumps({f"{r['m']} {r['keys']}": dict(
            signature=r['signature'], why=(known.get(f"{r['m']} {r['keys']}") or {}).get('why', 'TODO'))
            for r in sorted(results, key=lambda r: (r['m'], r['keys'])) if not r['ok']}, indent=1) + '\n')
        print(f'wrote {a.write_known}')
    if not a.known:
        return 0 if ok == len(results) and results else 1
    run = {f"{r['m']} {r['keys']}": r for r in results}
    bad = []
    for k, r in run.items():
        if not r['ok'] and k not in known:
            bad.append(f"{k} does not match the host ({r['signature']}) and is not in {a.known.name}")
        elif r['ok'] and k in known:
            bad.append(f'{k} now matches the host: take it off {a.known.name}')
        elif not r['ok'] and known[k]['signature'] != r['signature']:
            bad.append(f"{k} fails differently: {r['signature']} (listed: {known[k]['signature']})")
    for k in known:
        if k.split(' ', 1)[0] in missions and k not in run:
            bad.append(f'{k} is in {a.known.name} but this run did not exercise it')
    for x in bad:
        print(f'FAIL: {x}')
    print(f"{'PASS' if results and not bad else 'FAIL'}: against {a.known.name}: "
          f'{len(results) - ok} known failures, {len(bad)} not as listed')
    return 0 if results and not bad else 1


def snap_channel32(exe, m, a):
    """Channel 32 at the snapshot (the PRO bit's neighbours), from a host
    run to it."""
    header = WORK / f'{m}-ch32-snap.h'
    subprocess.run([str(exe), fhd.ROPE[m], str(a.snap_cs + 200), ''], cwd=ROOT, capture_output=True, text=True,
                   env={**os.environ, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SNAPSHOT': f'{a.snap_cs},{header}'})
    found = re.search(r'\(h\)\.channel32=(\d+)u', header.read_text()) if header.exists() else None
    if not found:
        sys.exit(f'FAIL: no {m} snapshot at {a.snap_cs} cs for channel 32')
    return int(found[1])


if __name__ == '__main__':
    sys.exit(main())
