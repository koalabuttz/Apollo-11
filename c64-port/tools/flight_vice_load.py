#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run the flight-store load check in VICE (docs/FLIGHT-STORE.md): the flight
resident boots on the whole-program store (build/flight-store/<m>.reu),
resolves every resolver record and times each flight module's load from the
REU (platform/c64/flight_load_check.c). Checks the standard $c000 mailbox
and writes the timings to build/flight-store/vice-load.json.

Each direct module's point also runs on the host (#426): flight-host-run-<m>
FLIGHT_POINTS runs it from the boot state the check leaves in memory, on the
tables the module's wrapper (build/flight-store/<m>/<name>.c) names, for the
C64's step count, and the image and channels sum, the steps and the status
must match the C64's. A module whose tables are not direct (an io record of
its own) runs its main instead and is counted, not compared.

With --snap, every point starts from a host flight snapshot instead of the
boot image (tools/flight_host_run.c FLIGHT_SNAPSHOT, as the #644 replay
windows start): the check is relinked with -DFL_SNAP and its timings go to
build/flight-store/vice-load-snap.json. A scenario's job and interrupt state
lets more routines run than the boot state, where most refuse at entry.

  flight_vice_load.py [--missions lm,cm] [--no-host] [--snap lm=SCENARIO:CS,cm=SCENARIO:CS]
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
# fl_timing (flight_load_check.c): load, cached, direct, main, then fl_point
# (sum, steps, status: code in bits 0-3, FL_POINT_FULL, FL_POINT_NONE,
# FL_POINT_MAIN, step bit 8).
TIMING = struct.Struct('<IHHHHBB')
POINT_FULL, POINT_NONE, POINT_MAIN = 0x10, 0x20, 0x40
STATUS = {0: 'ok', 1: 'out of scope', 2: 'bailout pending', 3: 'idle', 4: 'unmapped', 5: 'invalid action',
          6: 'status 6', 7: 'status 7', 8: '1201', 9: '1202', 15: 'other'}


def _number(text):
    """A C literal from a wrapper: 076000u is octal, 2280u decimal."""
    text = text.rstrip('uU')
    return int(text, 8) if len(text) > 1 and text[0] == '0' else int(text)


def wrapper(path):
    """The tables a module's wrapper names (flight_table_module.c), as
    FLIGHT_POINTS lines after its P line, or None when the module's tables
    are not direct (FS_IO_KIND: run through its main, never rk_direct)."""
    d = dict(re.findall(r'^#define (FS_[A-Z_]+)(?:\(X\))?[ \t]*(.*)$', path.read_text(), re.M))
    if 'FS_IO_KIND' in d:
        return None
    lines = []
    keys = [_number(k) for k in d.get('FS_KEYS', '').split(',') if k.strip()]
    if 'FS_PICK' in d:
        picks = re.findall(r'X\((\w+),(\d+)\)', d['FS_PICK'])
    elif 'FS_CHUNK' in d:
        picks = [(d['FS_CHUNK_TABLE'].strip(), i) for i in re.findall(r'X\((\d+)\)', d['FS_CHUNK'])]
    else:
        picks = None
    if picks is not None:
        if keys and len(keys) != len(picks):
            raise SystemExit(f'{path}: FS_KEYS names {len(keys)} entries, the pick {len(picks)}')
        lines += [f'K {t} {i}' + (f' {keys[n]}' if keys else '') for n, (t, i) in enumerate(picks)]
    else:
        lines += [f'B {t}' for t in re.findall(r'X\((\w+)\)', d.get('FS_BASIC', ''))]
    if 'FS_HAS_INTERP' in d:
        lines += ['H'] + [f'I {t}' for t in re.findall(r'X\((\w+)\)', d.get('FS_INTERP', ''))]
    lines += [f'S {_number(a)} {_number(w):o}'
              for a, w in re.findall(r'\{\s*(\w+)\s*,\s*(\w+)\s*\}', d.get('FS_SITES', ''))]
    fixed = 2 if 'FS_ROPE_DATA' in d else 1 if 'FS_FIXED_DATA' in d else 0
    # Weight 0 (RK_MCT_IDLE): the CM self-check idle loop's module.
    return fixed, lines, _number(d.get('FS_MCT', '1u').strip()) == 0


def point(status, steps):
    """A recorded fl_point's status code, flags and step count."""
    return status & 0x0f, status & (POINT_FULL | POINT_NONE | POINT_MAIN), steps | (status & 0x80) << 1


def host_build(cc, missions):
    """flight-host-run-<m> on this checkout's host index (flight_host_differential.py build)."""
    import flight_host_differential as fhd
    engine = fhd.frozen_engine(cc)
    return {m: fhd.build(cc, m, engine) for m in missions}


# The other points' table in the load check's copy of the store
# (flight_load_check.c FL_POINTS_REU, up to the scratch).
POINTS_REU, POINTS_END, JOB_BYTES = 0x1F3000, 0x1F8000, 0x4000


def point_table(m, listed):
    """Every resolver record of each direct, whole module other than the one
    the check already runs (its first): one list of (bank, loc) per module
    in fl_modules order."""
    paths = {x['id']: ROOT / x['path'] for x in json.loads((OUT / f'{m}-manifest.json').read_text())['modules']}
    records = json.loads((OUT / f'{m}-resolver.json').read_text())
    out = []
    for ident, _, bank, loc in listed:
        p = paths.get(ident)
        if p is None or re.search(r'-part\d+$', p.stem) or wrapper(p.with_suffix('.c')) is None:
            out.append([])
            continue
        out.append(sorted({(r['bank'], r['loc']) for r in records if r['module'] == ident} - {(bank, loc)}))
    return out


def write_points(reu, table):
    """The table into the store copy (bytes, changed in place); the point count."""
    total = sum(len(x) for x in table)
    blob = struct.pack(f'<H{len(table)}H', total, *map(len, table)) + b''.join(
        struct.pack('<HH', b, lc) for x in table for b, lc in x)
    if POINTS_REU + len(blob) + 4 * total > POINTS_END or 4 * total > JOB_BYTES:
        raise SystemExit(f'{total} points do not fit the load check\'s table')
    if any(reu[POINTS_REU:POINTS_END]):
        raise SystemExit('the store uses the load check\'s points area (0x1f3000-0x1f7fff)')
    reu[POINTS_REU:POINTS_REU + len(blob)] = blob
    return total


def host_points(m, work, data, modules, listed, host, extra=()):
    """Run each direct module's point on the host (FLIGHT_POINTS) and compare.
    A flight part's module (docs/FLIGHT-PARTS.md, <file>-part<j>) is not
    compared: its keys and its hand-back at another part's word exist only in
    the C64 flight build, and the host runs the file whole.
    A point the C64 ran no step of (the module refused it at entry) must be
    refused on the host too. extra: the other points, (module index, bank,
    loc, fl_point sum, steps, status) each, compared as module index's are.
    Returns (compared, of them refused, not direct, parts, mismatches)."""
    import flight_host_differential as fhd
    paths = {x['id']: ROOT / x['path'] for x in json.loads((OUT / f'{m}-manifest.json').read_text())['modules']}
    (work / 'device.bin').write_bytes(data[0x100:0x200])
    # The C64's record sizes ($c0fe, $c0ff): the host fails on a layout of its own.
    spec, recorded, not_direct, parts = [f'M {work / "image.bin"} {work / "device.bin"} {data[0xfe]} {data[0xff]}'], {}, [], []
    for i, x in enumerate(modules):
        code, flags, steps = point(x['point_status'], x['point_steps'])
        if flags & POINT_NONE:
            continue
        if flags & POINT_MAIN:
            not_direct.append(x['id'])
            continue
        ident, bank, loc = listed[i][0], listed[i][2], listed[i][3]
        if re.search(r'-part\d+$', paths[ident].stem):
            parts.append(x['id'])
            continue
        tables = wrapper(paths[ident].with_suffix('.c'))
        if tables is None:
            raise SystemExit(f'{m} module {x["id"]}: the C64 ran its point directly, its wrapper names an io record')
        fixed, lines, idle = tables
        # rk_direct ends a CM idle-loop run at POSTJUMP (resident_flight.c RK_CM_POSTJUMP).
        stop = int(idle and m == 'cm')
        spec += [f'P {i} {loc:o} {bank & 0o76000:o} {bank & 0o100:o} {steps} {int(bool(flags & POINT_FULL))} {fixed} {stop}',
                 *lines, 'E']
        recorded[i] = (x['point_sum'], steps, code)
        for j, (k, b, lc, psum, psteps, pstatus) in enumerate(extra):
            if k != i:
                continue
            code2, flags2, steps2 = point(pstatus, psteps)
            if flags2 & POINT_NONE:
                continue
            label = len(modules) + j
            spec += [f'P {label} {lc:o} {b & 0o76000:o} {b & 0o100:o} {steps2} {int(bool(flags2 & POINT_FULL))} '
                     f'{fixed} {stop}', *lines, 'E']
            recorded[label] = (psum, steps2, code2)
    (work / 'points.spec').write_text('\n'.join(spec) + '\n')
    r = subprocess.run([str(host), fhd.ROPE[m]], env=dict(os.environ, FLIGHT_POINTS=str(work / 'points.spec')),
                       cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        raise SystemExit(f'{m} host points: {r.stderr.strip()}')
    names = [x['id'] for x in modules] + [f'{modules[k]["id"]} at {b:o},{lc:o}' for k, b, lc, *_ in extra]
    mismatches = compare(recorded, r.stdout, [dict(id=n) for n in names])
    return len(recorded), sum(1 for _, n, _ in recorded.values() if not n), not_direct, parts, mismatches


def compare(recorded, host_out, modules):
    """The points whose host run differs from the C64's: one line each."""
    ran = {}
    for line in host_out.split('\n'):
        if line.strip():
            i, s, n, c = (int(v) for v in line.split())
            ran[i] = (s, n, c)
    out = []
    for i, (s, n, c) in sorted(recorded.items()):
        h = ran.get(i)
        if h is None:
            out.append(f'module {modules[i]["id"]}: no host run')
        elif h != (s, n, c):
            why = [f'{what} {a}/{b}' for what, a, b in
                   (('sum', f'{s:04x}', f'{h[0]:04x}'), ('steps', n, h[1]),
                    ('status', STATUS.get(c, c), STATUS.get(h[2], h[2]))) if a != b]
            out.append(f'module {modules[i]["id"]}: ' + ', '.join(why) + ' (C64/host)')
    return out


def snap_prg(m, a, work):
    """The load check relinked to start from the --snap scenario's host
    snapshot, and the snapshot's header."""
    import flight_scenario_vice as fsv
    import flight_store_build as fsb
    name, cs = a.snaps[m]
    header = fsv.snapshot(a.host_exe[m], name, cs, f'load-check-{m}')
    prg = fsb.load_check_link(a.cc, m, OUT / f'{m}-load-check-snap.prg', [f'-DFL_SNAP="{header.resolve()}"'])
    return ROOT / prg, header


def run(m, a):
    prg = OUT / f'{m}-load-check.prg'
    rom = a.rom_dir.resolve()
    work = ROOT / 'build' / f'vice-{m}-load-check{"-snap" if m in a.snaps else ""}'
    work.mkdir(parents=True, exist_ok=True)
    snap = None
    if m in a.snaps:
        prg, snap = snap_prg(m, a, work)
    sys_at = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    if sys_at is None:
        sys.exit(f'FAIL: {prg.name} has no BASIC SYS header')
    dump, image = work / 'results.bin', work / 'image.bin'
    dump.unlink(missing_ok=True)
    image.unlink(missing_ok=True)
    (work / 'monitor.log').unlink(missing_ok=True)
    commands = work / 'vice.mon'
    # With a pinned module, boot's DMA of it to $C000 is the first store
    # there; the mailbox's done flag is the next.
    pinned = '#define FL_PINNED ' in (OUT / f'{m}-load-check.h').read_text()
    commands.write_text(f'load "{prg}" 0\nwatch store c000\n' + ('ignore 1 1\n' if pinned else '')
                        + f'g {int(sys_at[1]):04x}\n'
                        f'bsave "{dump}" 0 c000 cdff\n'
                        # The boot image the check left at IMAGE, for the host reference.
                        f'bsave "{image}" 0 7000 7fff\nquit\n')
    listed = [(int(a, 16), int(b), _number(c), _number(d)) for a, b, c, d in
              re.findall(r'\{0x([0-9a-f]+)u,(\d+)u,(\w+),(\w+)\}',
                         (OUT / f'{m}-load-check.h').read_text().split('fl_modules', 1)[1])]
    reu = work / 'image.reu'
    store = bytearray((OUT / f'{m}.reu').read_bytes())
    table = point_table(m, listed) if a.host and a.all_points else [[] for _ in listed]
    total = write_points(store, table)
    reu.write_bytes(store)
    pts = work / 'points.bin'
    pts.unlink(missing_ok=True)
    if total:
        commands.write_text(commands.read_text().replace('\nquit\n', f'\nbsave "{pts}" 0 8000 {0x8000 + 4 * total - 1:x}\nquit\n'))
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
    # Module ids and sizes in check order, from the check's data header (listed).
    for i in range(count):
        ident, size = listed[i][0], listed[i][1]
        load, cached, direct, main, psum, psteps, pstatus = TIMING.unpack_from(data, 0x400 + TIMING.size * i)
        load, cached = load - overhead, cached - overhead
        modules.append(dict(id=f'0x{ident:04x}', bytes=size, load_cycles=load, cached_cycles=cached,
                            direct_cycles=direct - overhead if direct else None,
                            main_cycles=main - overhead if direct and main else None,
                            direct_only=bool(direct and not main),
                            direct_differs=i in differ,
                            transfer_cycles=load - cached, cycles_per_byte=round((load - cached) / size, 2),
                            point_sum=psum, point_steps=psteps, point_status=pstatus))
    run_only, at_point, resolve = (v - overhead for v in struct.unpack_from('<3H', data, 0x3f0))
    host = dict(compared=0, refused=0, not_direct=[], parts=[], mismatches=[])
    extra = []
    if total and ok:
        raw = pts.read_bytes() if pts.exists() else b''
        if len(raw) < 4 * total:
            return dict(mission=m, ok=False, error=f'no results for the {total} other points (see {work}/vice.log)')
        flat = [(k, b, lc) for k, x in enumerate(table) for b, lc in x]
        extra = [(k, b, lc, *struct.unpack_from('<HBB', raw, 4 * j)) for j, (k, b, lc) in enumerate(flat)]
    if a.host and ok:
        n, refused, nd, parts, mism = host_points(m, work, data, modules, listed, a.host_exe[m], extra)
        host = dict(compared=n, refused=refused, not_direct=nd, parts=parts, mismatches=mism)
        ok = ok and not mism
    return dict(mission=m, ok=ok, snapshot=dict(scenario=a.snaps[m][0], cs=a.snaps[m][1], header=str(snap))
                if snap else None, failures=failures, checks=checks, first_failing_step=data[5], pinned_module=pinned,
                timer_overhead_cycles=overhead, soft_stack_peak_bytes=struct.unpack_from('<H', data, 0x3f6)[0],
                call_profile=dict(runtime_cached_run=run_only, resident_transfer_point=at_point,
                                  resident_transfer_resolve=resolve),
                host_points=host, modules=modules)


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
    p.add_argument('--no-host', dest='host', action='store_false',
                   help='skip the host reference (flight-host-run-<m>, built from the store build\'s host index)')
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--one-point', dest='all_points', action='store_false',
                   help="run only each module's first record, not every record of it")
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--snap', default='', help='start the points from host snapshots: lm=SCENARIO:CS,cm=SCENARIO:CS '
                   '(tools/flight_host_differential.py scenarios)')
    a = p.parse_args()
    a.snaps = {}
    for item in filter(None, a.snap.split(',')):
        m, _, at = item.partition('=')
        name, _, cs = at.rpartition(':')
        if m not in ('lm', 'cm') or not name or not cs.isdigit():
            p.error(f'--snap {item}: want lm|cm=SCENARIO:CS')
        a.snaps[m] = (name, int(cs))
    if a.snaps and not a.host:
        p.error('--snap needs the host (its snapshot comes from flight-host-run)')
    a.host_exe = host_build(a.host_cc, a.missions.split(',')) if a.host else {}
    results = [run(m, a) for m in a.missions.split(',')]
    out_name = 'vice-load-snap.json' if a.snaps else 'vice-load.json'
    (OUT / out_name).write_text(json.dumps(results, indent=1) + '\n')
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
        (OUT / out_name).write_text(json.dumps(results, indent=1) + '\n')
    failed = 0
    for r in results:
        if 'fit' in r:
            continue
        if not r['ok']:
            failed += 1
            for line in r.get('host_points', {}).get('mismatches', [])[:20]:
                print(f"FAIL: {r['mission']} point differs from the host: {line}")
            print(f"FAIL: {r['mission']} load check: {r.get('error') or {k: v for k, v in r.items() if k != 'modules'}}")
            continue
        mods = r['modules'] or [dict(load_cycles=0, cached_cycles=0)]
        at = f" from {r['snapshot']['scenario']} at {r['snapshot']['cs']} cs" if r.get('snapshot') else ''
        print(f"PASS: VICE PAL {r['mission']} load check{at}: {r['checks']} checks, 0 failures; "
              f"{len(mods)} modules, load and call {min(x['load_cycles'] for x in mods)}-"
              f"{max(x['load_cycles'] for x in mods)} cycles "
              f"({max(x['load_cycles'] for x in mods) * 1000 / PAL_HZ:.1f} ms max), cached call "
              f"{min(x['cached_cycles'] for x in mods)}-{max(x['cached_cycles'] for x in mods)}"
              + (f", direct call saves {min(d)}-{max(d)} cycles over the module main ({len(d)} modules match)"
                 if (d := [x['main_cycles'] - x['direct_cycles'] for x in mods
                           if x.get('main_cycles') is not None]) else '')
              + (f", {len(o)} direct-only modules run their point in {min(o)}-{max(o)} cycles"
                 if (o := [x['direct_cycles'] for x in mods if x.get('direct_only')]) else '')
              + (f"; {r['host_points']['compared']} points match the host ({r['host_points']['refused']} "
                 f"refused at entry on both; not compared: "
                 f"{len(r['host_points']['not_direct'])} run through their main, "
                 f"{len(r['host_points']['parts'])} flight parts)" if a.host else '')
              + f"; soft stack peak {r['soft_stack_peak_bytes']} bytes")
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
