#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's memory_and_overlay_validation evidence
(docs/PORT_SCOPE.md, integrated_applications).

  memory_acceptance.py lm|cm [--run RUN.json ...]

With no --run it runs the program's DEFAULT_RUNS windows itself
(tools/flight_scenario_vice.py, about 15 min each program in VICE).

Reads the built store (make flight-store), the resident sizes
(tools/resident_flight_build.py) and one or more flight_scenario_vice.py
results, then writes, all from the same commit:

  docs/acceptance/<Program>-memory_and_overlay_validation.md
  evidence/<m>-acceptance-memory/<scenario>-<snap_cs>.json   (each run, tree-stored
                                         with tools/evidence.py add --tree)

The doc lists the REU layout, each build-time check with its result, the
resident's free bytes against the floor, and each run's runtime REU bound
check (every REU byte the run changed and every DMA it made, inside the
declared layout: tools/reu_layout.py). A run counts only when it covered
its whole window and VICE armed the DMA trace; each run also starts a
control (the first window with the store left out of the layout) that must
FAIL, so a trace that sees nothing cannot pass. The rope's REU base is
checked against what the C64 code reads (include/rope_image.h), and the C64
RAM side against the linker scripts and include/overlay_v2.h: each slot's
origin and size, its largest module, the I/O area at $D000-$DFFF, the
vectors at $FFFA and the end of RAM. It exits 1, and says why, when any
check fails; the doc is still written, so the failure is on record.
"""
import argparse
import hashlib
import json
import os
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402
import reu_layout  # noqa: E402

CC = os.environ.get('CC', 'cc')
PLATFORM = ROOT / 'platform/c64'
OV2 = ROOT / 'include/overlay_v2.h'
# Slot class -> (linker script, overlay_v2.h ADDRESS / BYTES names). Class 2
# is the fixed extension in the RAM under I/O (platform/c64/interp_ext.ld).
SLOTS = {0: ('overlay_v2_slot_job.ld', 'JOB'), 1: ('overlay_v2_slot_task.ld', 'TASK'),
         2: ('interp_ext.ld', 'FIXED'), 3: ('overlay_v2_slot_pinned.ld', 'PINNED'),
         4: ('overlay_v2_slot_pinned_task.ld', 'TASK')}
SLOT_NAMES = {0: 'job', 1: 'task', 2: 'fixed (under I/O)', 3: 'pinned', 4: 'pinned task'}
IO = (0xD000, 0xE000)        # VIC, SID, CIA, colour RAM: only class 2 sits under it
VECTORS = 0xFFFA             # NMI, RESET, IRQ vectors to $FFFF
RAM_END = 0x10000
RESIDENT_LD = 'resident_flight.ld'
# Where each resident region must sit (resident_flight.ld's header, docs/
# OVERLAY_ABI_V2.md): (lowest start, highest end). The DSKY runs from
# $0200-$03FF and keeps its data in the screen's last rows; the screen
# itself ($0400-$07E7) and the sprite pointers ($07F8) stay clear.
RESIDENT_AT = {'low': (0x0200, 0x0400), 'screen_end': (0x06A8, 0x07F8), 'ram': (0x0800, 0x6F00),
               'services': (0x6F00, 0x7000), 'image': (0x7000, 0x8000)}
SERVICES = 0x6F00            # the service jump table every module calls (OVERLAY_ABI_V2.md)
# A control run: the program's first default window, short, with the store
# left out of the layout. Every run reads modules from the store, so the
# trace must log DMAs outside; one that logs none was never watching.
CONTROL_CS, CONTROL_DROP = 200, 'store'

PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
RESIDENT_FLOOR = 2048        # /mnt/project-files/pin-split/resident-recovery.md rule
STORE = ROOT / 'build/flight-store'
# The windows each program's doc checks when no --run is given: a powered
# phase, a DSKY/uplink window and a long landing or entry window, each with
# --replay so the replay rows and states are in the REU too.
DEFAULT_RUNS = {
    'lm': [('lm-p63-descent-thrust', 20010, 200, 50), ('lm-uplink-reject', 1900, 800, 50),
           ('lm-landing-ascent-nassp', 140000, 2000, 200), ('lm-landing-ascent-nassp', 128000, 2000, 200)],
    'cm': [('cm-entry', 29000, 400, 50), ('cm-tli', 31000, 300, 50), ('cm-p52', 5400, 300, 50)],
}
SIZES = ROOT / 'build/resident-flight/sizes.json'


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()



def build_checks(m):
    """[(name, ok, detail)]"""
    out = []
    manifest = json.loads((STORE / f'{m}-manifest.json').read_text())
    report = json.loads((STORE / f'report-{m}.json').read_text())
    try:
        regions = reu_layout.layout(m, manifest)
        out.append(('declared regions do not overlap', True, f'{len(regions)} regions (tools/reu_layout.py layout)'))
    except SystemExit as e:
        regions = []
        out.append(('declared regions do not overlap', False, str(e)))
    image = STORE / f'{m}.reu'
    same = image.exists() and sha(image) == manifest['image_sha256']
    out.append(('store image matches its manifest', same, f"sha256 {manifest['image_sha256'][:16]}..."))
    if regions:
        store_end = manifest['image_bytes']
        top = reu_layout.free_top(m, manifest)
        out.append(('store ends below the next region', store_end <= top,
                    f'store ends {store_end:#x}, next region {top:#x} ({top - store_end} bytes free for replay rows)'))
    out.append(('every key placed in a module', report['unplaced_keys'] == 0, f"unplaced_keys {report['unplaced_keys']}"))
    out.append(('every module links', not report['failed'], f"{report['modules_linked']} linked, failed {report['failed']}"))
    nrk = subprocess.run([sys.executable, 'tools/native_range_keys.py'], cwd=ROOT, capture_output=True, text=True)
    out.append(('every native_ranges return point keyed (#665)', nrk.returncode == 0,
                (nrk.stdout + nrk.stderr).strip().splitlines()[-1] if (nrk.stdout + nrk.stderr).strip() else 'no output'))
    if regions:
        out.append(rope_check(m, manifest, regions))
    ram, ram_checks = ram_layout(manifest)
    out += ram_checks
    sizes = json.loads(SIZES.read_text())[m]
    sizes = dict(sizes, ram=ram)
    out.append((f'resident free bytes at least {RESIDENT_FLOOR}', sizes['free_bytes'] >= RESIDENT_FLOOR,
                f"resident {sizes['resident_bytes']} of {sizes['resident_capacity']} bytes, {sizes['free_bytes']} free"))
    return out, manifest, regions, sizes


def rope_base_c64(m, cc=CC):
    """The rope's REU base the C64 code reads (include/rope_image.h under
    the flight store's RK_LM or RK_CM), or None when the header gives none."""
    name = 'APOLLO_ROPE_REU_LM' if m == 'lm' else 'APOLLO_ROPE_REU_CM'
    try:
        out = subprocess.run([cc, '-E', '-dM', '-Iinclude', f"-DRK_{m.upper()}=1", 'include/rope_image.h'],
                             cwd=ROOT, capture_output=True, text=True).stdout
    except OSError:
        return None
    hit = re.search(rf'^#define {name} \S*?\(?(0x[0-9A-Fa-f]+)', out, re.M)
    return int(hit.group(1), 16) if hit else None


def rope_check(m, manifest, regions, read=rope_base_c64):
    """(name, ok, detail): the manifest's rope base, the layout's rope region
    and the address the C64 code reads (read(m)) all agree."""
    c64 = read(m)
    built = int(manifest['rope_reu_base'], 0)
    region = next((r[1] for r in regions if r[0] == 'rope'), None)
    ok = c64 is not None and built == c64 and region == c64
    shown = f'{c64:#x}' if c64 is not None else 'not found'
    return ('rope base matches what the C64 reads', ok,
            f"manifest {built:#x}, layout {region:#x}, include/rope_image.h {shown}" if region is not None
            else f'manifest {built:#x}, no rope region, include/rope_image.h {shown}')


def ld_memory(path):
    """{region: (origin, length)} from a linker script's MEMORY blocks."""
    text = re.sub(r'/\*.*?\*/', '', pathlib.Path(path).read_text(), flags=re.S)
    return {n: (int(o, 16), int(n_, 16)) for n, o, n_ in
            re.findall(r'(\w+)\s*\([rwx]+\)\s*:\s*ORIGIN\s*=\s*(0x[0-9A-Fa-f]+)\s*,\s*LENGTH\s*=\s*(0x[0-9A-Fa-f]+)',
                       text)}


def ov2_define(name, header=OV2):
    hit = re.search(rf'#define APOLLO_OV2_{name}\s+(0x[0-9A-Fa-f]+)u', pathlib.Path(header).read_text())
    return int(hit.group(1), 16) if hit else None


def ram_layout(manifest, platform=PLATFORM, header=OV2):
    """(rows, checks). rows: [(what, start, end, largest module or None)] of
    the C64 RAM the program uses; checks: [(name, ok, detail)]."""
    rows, checks = [], []
    largest = {}
    for mod in manifest['modules']:
        largest[mod['slot']] = max(largest.get(mod['slot'], 0), mod['bytes'])
    res = ld_memory(platform / RESIDENT_LD)
    for name, (floor, ceil) in RESIDENT_AT.items():
        if name not in res:
            checks.append((f'resident {name} declared', False, f'no {name} region in {RESIDENT_LD}'))
            continue
        lo, n = res[name]
        rows.append((f'resident {name}', lo, lo + n, None))
        checks.append((f'resident {name} in ${floor:04x}-${ceil - 1:04x}', floor <= lo and lo + n <= ceil,
                       f'${lo:04x}-${lo + n - 1:04x}'))
    if 'services' in res:
        checks.append(('service table at $6F00', res['services'][0] == SERVICES, f"${res['services'][0]:04x}"))
    resident_end = max((r[2] for r in rows), default=0)
    checks.append(('resident ends below the job slot', bool(rows) and resident_end <= ov2_define('JOB_ADDRESS', header),
                   f'resident ends ${resident_end - 1:04x} ({RESIDENT_LD})'))
    for cls in sorted(set(largest) | {0, 1, 2, 3}):
        ld, key = SLOTS.get(cls, (None, None))
        if ld is None:
            checks.append((f'class {cls} slot is known', False, f'modules in unknown slot class {cls}'))
            continue
        mem = ld_memory(platform / ld)
        lo, n = next(iter(mem.values())) if len(mem) == 1 else (None, None)
        want = (ov2_define(f'{key}_ADDRESS', header), ov2_define(f'{key}_BYTES', header))
        if cls == 3:
            want = (ov2_define('PINNED_ADDRESS', header), ov2_define('PINNED_BYTES', header))
        what = f'class {cls} {SLOT_NAMES[cls]} slot'
        checks.append((f'{what}: linker script matches include/overlay_v2.h', lo is not None and (lo, n) == want,
                       f'{ld} {lo:#06x}+{n:#x}, overlay_v2.h {want[0]:#06x}+{want[1]:#x}' if lo is not None and None not in want
                       else f'{ld}: {len(mem)} MEMORY regions, overlay_v2.h {want}'))
        if lo is None:
            continue
        hi = lo + n
        big = largest.get(cls)
        rows.append((what, lo, hi, big))
        if big is not None:
            checks.append((f'{what}: largest module fits', big <= n, f'{big:,} of {n:,} bytes'))
        under_io = lo < IO[1] and hi > IO[0]
        checks.append((f'{what}: I/O area $D000-$DFFF only for class 2', under_io == (cls == 2),
                       f'${lo:04x}-${hi - 1:04x}'))
        checks.append((f'{what}: below the vectors and RAM end', hi <= VECTORS and hi <= RAM_END,
                       f'ends ${hi - 1:04x}, vectors at ${VECTORS:04x}'))
    # Nothing may overlap, resident or slot, except class 4 replacing class 1
    # (the same region, one module kept for the run). The resident's boot-only
    # code in the job slot is not a row: the first module load replaces it.
    spans = list(rows)
    for i, a in enumerate(spans):
        for b in spans[i + 1:]:
            same_task = {a[0][:7], b[0][:7]} == {'class 1', 'class 4'}
            if a[1] < b[2] and b[1] < a[2] and not same_task:
                checks.append(('no two areas overlap', False, f'{a[0]} ${a[1]:04x}-${a[2] - 1:04x} and '
                               f'{b[0]} ${b[1]:04x}-${b[2] - 1:04x}'))
    if not any(c[0] == 'no two areas overlap' for c in checks):
        checks.append(('no two areas overlap', True, f'{len(spans)} areas (class 1 and 4 share the task region)'))
    return sorted(rows, key=lambda r: r[1]), checks


def run_verdict(r, head=None, store=None):
    """(ok, why): one run counts when it ran at this commit on this store
    (head, store: HEAD's source_commit and the store's sha256, when given),
    covered its window, VICE armed the DMA trace, and nothing was written or
    read outside the layout. A run that leaves the host's state still counts:
    the check is where the C64 reads and writes, not what it computes; the
    doc shows which runs matched."""
    b = r.get('reu_bounds') or {}
    why = []
    if head is not None and r.get('commit') != head:
        why.append(f"run at {r.get('commit')}, not {head}")
    if store is not None and r.get('store_sha256') != store:
        why.append('run on another store')
    want = r.get('requested_cs')
    if r.get('replay'):
        reached = r.get('replay_end_cs') if r.get('replay_ended') else None
    else:
        reached = r.get('agc_cs')
    if want is None or reached is None or reached < want:
        why.append(f'covered {reached if reached is not None else "?"} of {want if want is not None else "?"} cs')
    if not b.get('dma_trace_armed'):
        why.append('DMA trace not armed')
    if not b.get('ok') or b.get('dma_outside') != 0:
        why.append(f"{len(b.get('outside', []))} writes and {b.get('dma_outside')} DMAs outside")
    return not why, '; '.join(why)


def control_verdict(r):
    """(ok, detail): the control run must FAIL with DMAs outside, its trace armed."""
    b = (r or {}).get('reu_bounds') or {}
    ok = bool(b) and b.get('dma_trace_armed') and not b.get('ok') and (b.get('dma_outside') or 0) > 0
    return bool(ok), (f"{b.get('dma_outside')} DMAs outside with the {CONTROL_DROP} left out, trace "
                      f"{'armed' if b.get('dma_trace_armed') else 'NOT armed'}" if b else 'no bounds result')


def vice_run(scenario, snap, cs, extra=()):
    out = STORE / 'flight-scenario-vice.json'
    out.unlink(missing_ok=True)
    subprocess.run([sys.executable, 'tools/flight_scenario_vice.py', '--scenario', scenario, '--snap-cs', str(snap),
                    '--cs', str(cs), *extra], cwd=ROOT, stdout=subprocess.DEVNULL)
    if not out.exists():
        return None
    r = json.loads(out.read_text())
    image = STORE / f"{r.get('mission', '')}.reu"
    r['store_sha256'] = sha(image) if image.exists() else None   # the store this run read
    return r


def control_run(m, dest):
    scenario, snap = DEFAULT_RUNS[m][0][:2]
    print(f'{m}: control {scenario} {snap}+{CONTROL_CS} with the {CONTROL_DROP} left out (must FAIL)', flush=True)
    r = vice_run(scenario, snap, CONTROL_CS, ['--reu-check-drop', CONTROL_DROP])
    dest.mkdir(parents=True, exist_ok=True)
    path = dest / f'control-{scenario}-{snap}.json'
    path.write_text(json.dumps(r, indent=1) + '\n')
    return r, path


def default_runs(m, dest):
    """Run DEFAULT_RUNS[m] and return their result JSONs. A window that
    leaves the host is still a valid bound-check run; only a run that wrote
    no result fails here."""
    dest.mkdir(parents=True, exist_ok=True)
    paths = []
    for scenario, snap, cs, every in DEFAULT_RUNS[m]:
        print(f'{m}: running {scenario} {snap}+{cs} (--replay-every {every})', flush=True)
        r = vice_run(scenario, snap, cs, ['--replay', '--replay-every', str(every)])
        if r is None:
            raise SystemExit(f'FAIL: {scenario} {snap}+{cs} wrote no result')
        path = dest / f'{scenario}-{snap}.json'
        path.write_text(json.dumps(r, indent=1) + '\n')
        paths.append(path)
    return paths


def host_match(r):
    """How a --replay run compared with the host, for the doc."""
    if not r.get('replay'):
        return 'not compared'
    if r.get('replay_differences'):
        return f"no: {r['replay_differences']} differences at {r.get('replay_differ_at_cs')} cs"
    return 'yes' if r.get('replay_ended') else 'no: did not reach the end'


def runtime_checks(m, runs, dest, control=None, head=None, store=None):
    """(rows, control row or None). control: (result, path) of control_run."""
    rows, copies, crow = [], [], None
    dest.mkdir(parents=True, exist_ok=True)
    if control is not None:
        r, path = control
        copy = dest / pathlib.Path(path).name
        copy.write_text(json.dumps(r, indent=1) + '\n')
        copies.append(copy)
        ok, detail = control_verdict(r)
        if r and head is not None and (r.get('commit') != head or r.get('store_sha256') != store):
            ok, detail = False, detail + '; run at another commit or on another store'
        crow = dict(ok=ok, detail=detail, scenario=(r or {}).get('scenario'), snap_cs=(r or {}).get('snap_cs'),
                    agc_cs=(r or {}).get('agc_cs'), evidence=f'evidence/{m}-acceptance-memory/{copy.name}',
                    sha256=sha(copy))
    for path in runs:
        r = json.loads(pathlib.Path(path).read_text())
        if r.get('mission', m) != m:
            raise SystemExit(f'{path}: a {r["mission"]} run, not {m}')
        b = r.get('reu_bounds')
        name = f"{r['scenario']}-{r.get('snap_cs', 'x')}{'-replay' if r.get('replay') else ''}.json"
        copy = dest / name
        copy.write_text(json.dumps(r, indent=1) + '\n')
        copies.append(copy)
        ok, why = run_verdict(r, head, store)
        rows.append(dict(host=host_match(r), scenario=r['scenario'], snap_cs=r.get('snap_cs'), agc_cs=r.get('agc_cs'),
                         requested_cs=r.get('requested_cs'), why=why, armed=bool((b or {}).get('dma_trace_armed')),
                         replay=r.get('replay'), ok=ok, touched=(b or {}).get('touched', []),
                         outside=len((b or {}).get('outside', [])), dma_outside=(b or {}).get('dma_outside'),
                         dma_late=(b or {}).get('dma_unchecked', 0),
                         evidence=f'evidence/{m}-acceptance-memory/{name}', sha256=sha(copy)))
    if copies:
        # evidence/<id>/ is flat, with its SHA256.json (tools/ci_guard.py).
        subprocess.run([sys.executable, 'tools/evidence.py', 'add', f'{m}-acceptance-memory',
                        *map(str, copies), '--tree'], cwd=ROOT, check=True)
    return rows, crow


def write_doc(m, checks, manifest, regions, sizes, rows, commit, control=None):
    p = PROGRAM[m]
    ok = bool(all(c[1] for c in checks) and rows and all(r['ok'] for r in rows) and control and control['ok'])
    lines = [f'# {p}: memory and overlay validation', '',
             f'Generated by `tools/memory_acceptance.py {m}` at commit `{commit}`. '
             f"Result: **{'PASS' if ok else 'FAIL'}**.", '',
             'This is the `memory_and_overlay_validation` evidence of the '
             f'`integrated_applications.{p}` entry (docs/PORT_SCOPE.md). It shows that the program\'s REU store, '
             'resident and overlays fit the declared memory layout when built, and that flight runs on the C64 '
             'read and write only inside that layout.', '',
             '## REU layout', '',
             'From `tools/reu_layout.py`; each bound comes from the source that owns the region '
             '(docs/FLIGHT-STORE.md). A `--replay` run adds its rows (read only) and states (writable) '
             'between the store and the next region.', '',
             '| Region | Start | End | Bytes | Access |', '|---|---|---|---|---|']
    for name, lo, hi, w in regions:
        lines.append(f"| {name} | `{lo:#08x}` | `{hi:#08x}` | {hi - lo:,} | {'read/write' if w else 'read only'} |")
    image = STORE / f'{m}.reu'
    lines += ['', f"Store: `flight-store/{m}.reu` (sha256 `{sha(image) if image.exists() else 'missing'}`), "
              f"{manifest['image_bytes']:,} bytes, {len(manifest['modules'])} modules, "
              f"{manifest['resolver_records']} resolver records, REU {manifest['reu_size_kb']} KB.", '',
              '## Build-time checks', '', '| Check | Result | Detail |', '|---|---|---|']
    for name, good, detail in checks:
        lines.append(f"| {name} | {'pass' if good else 'FAIL'} | {detail} |")
    lines += ['', '## Resident', '',
              f"The resident (kernel, Executive, WAITLIST, interpreter core and service table, $0801-$6EFF) is "
              f"{sizes['resident_bytes']:,} of {sizes['resident_capacity']:,} bytes, {sizes['free_bytes']:,} free; "
              f'the floor is {RESIDENT_FLOOR:,} free (a PR that adds resident bytes recovers them in the same series). '
              'Overlays load into the job and task slots above it through the service jump table at $6F00.', '',
              '| Component | Bytes |', '|---|---|']
    for comp, n in sizes['components'].items():
        lines.append(f'| {comp} | {n:,} |')
    lines += ['', '## C64 RAM', '',
              'From the linker scripts (platform/c64/*.ld), checked against include/overlay_v2.h; the largest '
              'module of each slot class is from the store manifest. Only the class 2 extension sits in the RAM '
              f'under the I/O area (${IO[0]:04X}-${IO[1] - 1:04X}); nothing reaches the vectors at ${VECTORS:04X}.', '',
              '| Area | Start | End | Largest module |', '|---|---|---|---|']
    for what, lo, hi, big in sizes.get('ram', []):
        lines.append(f"| {what} | `${lo:04X}` | `${hi - 1:04X}` | {f'{big:,} B' if big is not None else ''} |")
    lines += ['', '## Runtime REU bound check', '',
              'Each run below is a `tools/flight_scenario_vice.py` flight run on the C64 in VICE (stock C64, '
              '2 MB REU). A run counts only when it covered its whole window and VICE armed the DMA trace '
              '(all three trace checkpoints set). Two checks per run: every REU byte that changed lies in a writable region '
              '(the image before the run against the one VICE wrote back), and every DMA the run made, read or '
              'write, lies inside one declared region (a VICE monitor trace on each $DF01 command). '
              'Either one failing fails the run. VICE now and then evaluates the trace only after a DMA '
              'has run; for those (column "seen late") only the last byte is range-checked, and their '
              'writes are still in the byte compare.', '',
              'A run that left the host\'s state still counts here (the check is where the C64 reads and writes, '
              'not what it computes); the "Matches host" column says which did.', '',
              '| Scenario | Start cs | Window cs | Covered cs | Trace armed | Matches host | Regions written | Writes outside | DMAs outside | Seen late | Result | Evidence |',
              '|---|---|---|---|---|---|---|---|---|---|---|---|']
    for r in rows:
        lines.append(f"| {r['scenario']} | {r['snap_cs']} | {r.get('requested_cs')} | {r['agc_cs']} | "
                     f"{'yes' if r.get('armed') else 'NO'} | {r.get('host', '')} | "
                     f"{', '.join(r['touched']) or 'none'} | {r['outside']} | {r['dma_outside']} | {r['dma_late']} | "
                     f"{'pass' if r['ok'] else 'FAIL: ' + r.get('why', '')} | `{r['evidence']}` (sha256 `{r['sha256'][:16]}...`) |")
    if not rows:
        lines.append('| (no runs given) | | | | | | | | | | FAIL | |')
    lines += ['', '## Control', '',
              f'The check\'s own test, run each time: the first window above, {CONTROL_CS} cs, with the '
              f'{CONTROL_DROP} left out of the layout (`--reu-check-drop {CONTROL_DROP}`). Every run reads its modules '
              'from the store, so this run must FAIL with DMAs outside; a trace that logs nothing fails here.', '']
    if control:
        lines.append(f"Control: {control['scenario']} from {control['snap_cs']} cs, ran {control['agc_cs']} cs: "
                     f"{control['detail']}: {'it failed as it must (check passes)' if control['ok'] else 'the check FAILS'}. "
                     f"`{control['evidence']}` (sha256 `{control['sha256'][:16]}...`)")
    else:
        lines.append('Control: not run; the check FAILS.')
    lines += ['',
              '## Reproduce', '', '```sh', 'make flight-store', 'python3 tools/resident_flight_build.py',
              f'python3 tools/memory_acceptance.py {m}   # runs its default windows; or --run RESULT.json ...',
              '```', '']
    path = ROOT / 'docs/acceptance' / f'{p}-memory_and_overlay_validation.md'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines))
    return path, ok


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', choices=sorted(PROGRAM))
    ap.add_argument('--run', action='append', default=[],
                    help='a flight_scenario_vice.py result JSON (default: run DEFAULT_RUNS)')
    a = ap.parse_args(argv)
    commit = flight_holes.source_commit()
    checks, manifest, regions, sizes = build_checks(a.mission)
    work = ROOT / 'build/memory-acceptance' / a.mission
    runs = a.run or default_runs(a.mission, work / 'runs')
    control = control_run(a.mission, work / 'runs')
    image = STORE / f'{a.mission}.reu'
    rows, crow = runtime_checks(a.mission, runs, work, control, commit, sha(image) if image.exists() else None)
    path, ok = write_doc(a.mission, checks, manifest, regions, sizes, rows, commit, crow)
    print(f"{path.relative_to(ROOT)}: {'PASS' if ok else 'FAIL'} (sha256 {sha(path)})")
    for name, good, detail in checks:
        if not good:
            print(f'FAIL: {name}: {detail}')
    for r in rows:
        if not r['ok']:
            print(f"FAIL: {r['scenario']} at {r['snap_cs']}: {r['why']}")
    if not crow['ok']:
        print(f"FAIL: control: {crow['detail']}")
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
