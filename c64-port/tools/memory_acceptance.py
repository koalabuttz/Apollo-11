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
declared layout: tools/reu_layout.py). It exits 1, and says why, when any
check fails; the doc is still written, so the failure is on record.
"""
import argparse
import hashlib
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import reu_layout  # noqa: E402

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


def git(*args):
    return subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True).stdout.strip()


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
    sizes = json.loads(SIZES.read_text())[m]
    out.append((f'resident free bytes at least {RESIDENT_FLOOR}', sizes['free_bytes'] >= RESIDENT_FLOOR,
                f"resident {sizes['resident_bytes']} of {sizes['resident_capacity']} bytes, {sizes['free_bytes']} free"))
    return out, manifest, regions, sizes


def default_runs(m, dest):
    """Run DEFAULT_RUNS[m] and return their result JSONs. A window that
    leaves the host is still a valid bound-check run; only a run that wrote
    no result fails here."""
    dest.mkdir(parents=True, exist_ok=True)
    out = STORE / 'flight-scenario-vice.json'
    paths = []
    for scenario, snap, cs, every in DEFAULT_RUNS[m]:
        out.unlink(missing_ok=True)
        print(f'{m}: running {scenario} {snap}+{cs} (--replay-every {every})', flush=True)
        subprocess.run([sys.executable, 'tools/flight_scenario_vice.py', '--scenario', scenario, '--snap-cs', str(snap),
                        '--cs', str(cs), '--replay', '--replay-every', str(every)], cwd=ROOT,
                       stdout=subprocess.DEVNULL)
        if not out.exists():
            raise SystemExit(f'FAIL: {scenario} {snap}+{cs} wrote no result')
        path = dest / f'{scenario}-{snap}.json'
        path.write_bytes(out.read_bytes())
        paths.append(path)
    return paths


def runtime_checks(m, runs, dest):
    rows, copies = [], []
    dest.mkdir(parents=True, exist_ok=True)
    for path in runs:
        r = json.loads(pathlib.Path(path).read_text())
        if r.get('mission', m) != m:
            raise SystemExit(f'{path}: a {r["mission"]} run, not {m}')
        b = r.get('reu_bounds')
        name = f"{r['scenario']}-{r.get('snap_cs', 'x')}{'-replay' if r.get('replay') else ''}.json"
        copy = dest / name
        copy.write_text(json.dumps(r, indent=1) + '\n')
        copies.append(copy)
        ok = bool(b) and b['ok'] and b.get('dma_outside') == 0
        rows.append(dict(scenario=r['scenario'], snap_cs=r.get('snap_cs'), agc_cs=r.get('agc_cs'),
                         replay=r.get('replay'), ok=ok, touched=(b or {}).get('touched', []),
                         outside=len((b or {}).get('outside', [])), dma_outside=(b or {}).get('dma_outside'),
                         dma_late=(b or {}).get('dma_unchecked', 0),
                         evidence=f'evidence/{m}-acceptance-memory/{name}', sha256=sha(copy)))
    if copies:
        # evidence/<id>/ is flat, with its SHA256.json (tools/ci_guard.py).
        subprocess.run([sys.executable, 'tools/evidence.py', 'add', f'{m}-acceptance-memory',
                        *map(str, copies), '--tree'], cwd=ROOT, check=True)
    return rows


def write_doc(m, checks, manifest, regions, sizes, rows, commit):
    p = PROGRAM[m]
    ok = all(c[1] for c in checks) and rows and all(r['ok'] for r in rows)
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
    lines += ['', '## Runtime REU bound check', '',
              'Each run below is a `tools/flight_scenario_vice.py` flight run on the C64 in VICE (stock C64, '
              '2 MB REU). Two checks per run: every REU byte that changed lies in a writable region '
              '(the image before the run against the one VICE wrote back), and every DMA the run made, read or '
              'write, lies inside one declared region (a VICE monitor trace on each $DF01 command). '
              'Either one failing fails the run. VICE now and then evaluates the trace only after a DMA '
              'has run; for those (column "seen late") only the last byte is range-checked, and their '
              'writes are still in the byte compare.', '',
              '| Scenario | Start cs | Run cs | Replay | Regions written | Writes outside | DMAs outside | Seen late | Result | Evidence |',
              '|---|---|---|---|---|---|---|---|---|---|']
    for r in rows:
        lines.append(f"| {r['scenario']} | {r['snap_cs']} | {r['agc_cs']} | {'yes' if r['replay'] else 'no'} | "
                     f"{', '.join(r['touched']) or 'none'} | {r['outside']} | {r['dma_outside']} | {r['dma_late']} | "
                     f"{'pass' if r['ok'] else 'FAIL'} | `{r['evidence']}` (sha256 `{r['sha256'][:16]}...`) |")
    if not rows:
        lines.append('| (no runs given) | | | | | | | | FAIL | |')
    lines += ['', 'The check\'s own test: the same LM P63 window with the scratch region left out of the layout '
              '(`--reu-check-drop scratch`) logs 48 DMAs outside and fails; with the full layout it logs 0 '
              '(docs/FLIGHT-STORE.md).', '',
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
    commit = git('rev-parse', '--short=12', 'HEAD') + (' (dirty)' if git('status', '--porcelain', '--', 'src', 'include', 'platform', 'tools') else '')
    checks, manifest, regions, sizes = build_checks(a.mission)
    runs = a.run or default_runs(a.mission, ROOT / 'build/memory-acceptance' / a.mission / 'runs')
    rows = runtime_checks(a.mission, runs, ROOT / 'build/memory-acceptance' / a.mission)
    path, ok = write_doc(a.mission, checks, manifest, regions, sizes, rows, commit)
    print(f"{path.relative_to(ROOT)}: {'PASS' if ok else 'FAIL'} (sha256 {sha(path)})")
    for name, good, detail in checks:
        if not good:
            print(f'FAIL: {name}: {detail}')
    for r in rows:
        if not r['ok']:
            print(f"FAIL: {r['scenario']} at {r['snap_cs']}: {r['outside']} writes, {r['dma_outside']} DMAs outside")
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
