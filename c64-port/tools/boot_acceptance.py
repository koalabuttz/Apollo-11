#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's bootable_c64_application evidence
(docs/PORT_SCOPE.md, integrated_applications).

  boot_acceptance.py lm|cm [--skip RUN,...] [--doc-only]

Boots the program from its disk images in VICE (tools/flight_disk.py, a
stock PAL C64 with a 2 MB REU; true drive emulation but in the 1581-kernal
run), once for each way in, each with the REU bounds trace:

  1581           the D81 set and the fast loader, keys typed into the DSKY's
                 key queue: what CI runs (make flight-disk-vice)
  1581-keyboard  the same set, keys pressed on VICE's window so the C64's
                 own keyboard scan reads them (--xtest, under xvfb-run)
  1541           the D64 set, read with the KERNAL, disk swaps asked for
  1581-kernal    the D81 set without drive code (--slow), VICE's virtual
                 drive serving the KERNAL's reads: the path any drive that
                 is not a 1581 gets

Each run types RSET, V37 ENTR 00 ENTR and passes only with the AGC in P00
(MODREG 0, 00 under PROG on the screen). Then the hardware check disk
(APOLLO TEST) runs once. All from the same commit, it writes:

  docs/acceptance/<Program>-bootable_c64_application.md
  evidence/<m>-acceptance-boot/   each run's result, boot record, VICE log
                                  and the DSKY's screenshot (tree-stored)

It exits 1, and says why, when a run fails; the doc is still written, so
the failure is on record. --doc-only rewrites the doc from the evidence
already in the tree.
"""
import argparse
import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import flight_disk as fdisk  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[1]
PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
OUT = ROOT / 'build/flight-disk'
STAGE = ROOT / 'build/boot-acceptance'
RUNS = {
    '1581': ([], 'D81 set, fast loader', 'DSKY key queue (VICE monitor)'),
    '1581-keyboard': (['--xtest'], 'D81 set, fast loader', "C64 keyboard scan (keys pressed on VICE's window)"),
    '1541': (['--drive', '1541'], 'D64 set, KERNAL load, disk swaps', 'DSKY key queue (VICE monitor)'),
    '1581-kernal': (['--slow'], "D81 set, KERNAL load through VICE's virtual drive (no true drive emulation)",
                    'DSKY key queue (VICE monitor)'),
}
# The source the runs build from: a change here makes the doc stale.
SOURCES = ['src', 'include', 'platform', 'tools', 'tests', 'Makefile']


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def git(*args):
    return subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True).stdout.strip()


def flight_disk(args, xvfb=False):
    """Run flight_disk.py; its report (build/flight-disk/vice.json), or a
    failed one when it wrote none."""
    (OUT / 'vice.json').unlink(missing_ok=True)
    cmd = [sys.executable, 'tools/flight_disk.py', '--vice', *args]
    if xvfb:
        # 24-bit colour: VICE's GTK window takes no keys on Xvfb's default 8-bit screen
        cmd = ['xvfb-run', '-a', '-s', '-screen 0 1024x768x24', *cmd]
    print('+', ' '.join(cmd), flush=True)
    code = subprocess.run(cmd, cwd=ROOT).returncode
    if (OUT / 'vice.json').exists():
        return json.loads((OUT / 'vice.json').read_text())[0]
    return dict(disks=[], bytes_loaded=0, data_bytes=[], loader_bytes=0, resident_bytes=0,
                vice=dict(ok=False, error=f'flight_disk.py exited {code} with no report'))


def run_all(m, skip, stage):
    stage.mkdir(parents=True, exist_ok=True)
    for name, (extra, _, _) in RUNS.items():
        if name in skip:
            continue
        (OUT / f'vice-{m}' / 'vice.log').unlink(missing_ok=True)
        (OUT / f'boot-{m}.json').unlink(missing_ok=True)  # never a stale one from an earlier make
        report = flight_disk(['--missions', m, '--p00', *extra], xvfb='--xtest' in extra)
        (stage / f'{m}-{name}.json').write_text(json.dumps(report, indent=1) + '\n')
        boot = OUT / f'boot-{m}.json'
        if boot.exists():  # removed after each run, so never a stale one
            shutil.copy(boot, stage / f'{m}-{name}-boot.json')
        for log in ('vice', 'monitor'):  # monitor.log: the REU bounds trace
            if (OUT / f'vice-{m}' / f'{log}.log').exists():
                shutil.copy(OUT / f'vice-{m}' / f'{log}.log', stage / f'{m}-{name}-{log}.log')
        shot = OUT / f'vice-{m}' / f'{m}-dsky.png'
        if shot.exists():
            shutil.copy(shot, stage / f'{m}-{name}-dsky.png')
            shot.unlink()
        boot.unlink(missing_ok=True)
    if 'test' not in skip:
        (OUT / 'vice-test' / 'vice.log').unlink(missing_ok=True)
        report = flight_disk(['--missions', 'test'])
        # Bind the check disk to the commit, as the boot records do.
        report['commit'] = git('rev-parse', 'HEAD')
        # the boot records' scope (flight_disk.py boot_evidence)
        report['dirty'] = bool(git('status', '--porcelain', '--untracked-files=no', '--', '.',
                                   ':(exclude)evidence', ':(exclude)docs'))
        files = [OUT / d for d in report.get('disks', [])] + sorted(OUT.glob('test-*.prg'))
        report['sha256'] = {f'{f.parent.name}/{f.name}': sha(f) for f in files if f.exists()}
        (stage / f'{m}-check-disk.json').write_text(json.dumps(report, indent=1) + '\n')
        if (OUT / 'vice-test' / 'vice.log').exists():
            shutil.copy(OUT / 'vice-test' / 'vice.log', stage / f'{m}-check-disk-vice.log')
    files = sorted(str(p) for p in stage.iterdir())
    subprocess.run([sys.executable, 'tools/evidence.py', 'add', f'{m}-acceptance-boot', *files, '--tree'],
                   cwd=ROOT, check=True)


def trim(rows):
    rows = list(rows)
    while rows and not rows[-1].strip():
        rows.pop()
    return rows


def kb(n):
    return f'{n:,}'


def write_doc(m, commit):
    p = PROGRAM[m]
    ev = ROOT / 'evidence' / f'{m}-acceptance-boot'
    rel = lambda f: f'evidence/{m}-acceptance-boot/{f.name}'  # noqa: E731
    digest = lambda f: f"`{rel(f)}` (sha256 `{sha(f)[:16]}...`)"  # noqa: E731
    runs = {n: json.loads((ev / f'{m}-{n}.json').read_text()) for n in RUNS if (ev / f'{m}-{n}.json').exists()}
    boots = {n: json.loads((ev / f'{m}-{n}-boot.json').read_text()) for n in RUNS if (ev / f'{m}-{n}-boot.json').exists()}
    check_path = ev / f'{m}-check-disk.json'
    check = json.loads(check_path.read_text()) if check_path.exists() else None
    # The REU trace is checked again here from each run's monitor log, so
    # the result rests on the log in the tree, not only on the run's JSON.
    traced, trace_errors = {}, {}
    for n in runs:
        log = ev / f'{m}-{n}-monitor.log'
        want = boots.get(n, {}).get('monitor_log_sha256')
        regions = [(x['name'], int(x['start'], 16), int(x['end'], 16), x['writable'])
                   for x in (runs[n]['vice'].get('reu_bounds') or {}).get('regions', [])]
        if not log.exists():
            trace_errors[n] = ['no monitor log']
        elif not want:
            trace_errors[n] = ['the boot record has no monitor_log_sha256']
        elif sha(log) != want:
            trace_errors[n] = ['the monitor log is not the one the boot record hashes']
        elif not regions:
            trace_errors[n] = ['the run records no REU regions']
        else:
            t = fdisk.trace_result(log.read_text(errors='replace'), regions)
            trace_errors[n] = t['trace_errors'] + ([f"{t['dma_outside']} DMAs outside"] if t['dma_outside'] else [])
        traced[n] = not trace_errors[n]
    oks = {n: bool(r['vice'].get('ok')) and bool(r['vice'].get('p00'))
           and bool((r['vice'].get('reu_bounds') or {}).get('ok')) and traced[n] for n, r in runs.items()}
    commits = {b['commit'] for b in boots.values()} | ({check['commit']} if check and check.get('commit') else set())
    dirty = any(b.get('dirty') for b in boots.values()) or bool(check and check.get('dirty'))
    one_commit = len(commits) == 1 and len(boots) == len(RUNS) and bool(check and check.get('commit')) and not dirty
    ok = set(oks) == set(RUNS) and all(oks.values()) and one_commit and bool(check and check['vice'].get('ok'))
    lines = [f'# {p}: bootable C64 application', '',
             f'Generated by `tools/boot_acceptance.py {m}` at commit `{commit}`. '
             f"Result: **{'PASS' if ok else 'FAIL'}**.", '',
             'This is the `bootable_c64_application` evidence of the '
             f'`integrated_applications.{p}` entry (docs/PORT_SCOPE.md). It shows the whole program booting on a '
             'stock C64 from its disk images alone, with nothing else prepared on a PC: the loader fills a '
             '2 MB REU from the disks, the flight resident verifies the store and starts the AGC with the '
             'hardware restart (GOJAM), and the crew\'s keys then select P00 on the DSKY on the C64 screen.', '',
             'Every run below is in VICE (x64sc, PAL, stock 1 MHz C64, 2 MB REU, the Commodore KERNAL, BASIC '
             'and character ROMs), with true drive emulation except in the `1581-kernal` run, where VICE\'s '
             'virtual drive serves the KERNAL\'s reads. Each one types RSET, `V37` ENTR `00` ENTR after the '
             'boot and passes only when the AGC is in P00: MODREG 00000 and `00` under PROG on the screen. '
             'Speed and real time are not acceptance criteria (docs/PORT_SCOPE.md); the times are given to '
             'say what to expect. Users\' steps: docs/RUN-ON-C64.md.', '']
    missing = [n for n in RUNS if n not in boots] + ([] if check and check.get('commit') else ['check disk'])
    if missing:
        lines += [f"Not run, or with no record of its commit: {', '.join(missing)}. Every run is required.", '']
    if len(commits) > 1 or dirty:
        lines += [f"The runs and the check disk come from {len(commits)} commits "
                  f"({', '.join(c[:12] for c in sorted(commits))}){', from a dirty tree' if dirty else ''}: "
                  'stale. Run the tool again from one commit.', '']
    lines += ['## Disk sets', '',
              '| Drive | Images | Disks | Bytes to the REU | Data bytes per disk | Loader | Resident |',
              '|---|---|---|---|---|---|---|']
    for n in ('1581', '1541'):
        r = runs.get(n)
        if r:
            lines.append(f"| {n} | {', '.join(f'`{d}`' for d in r['disks'])} | {len(r['disks'])} | "
                         f"{kb(r['bytes_loaded'])} | {', '.join(kb(x) for x in r['data_bytes'])} | "
                         f"{kb(r['loader_bytes'])} bytes | {kb(r['resident_bytes'])} bytes |")
    lines += ['', 'Each set holds the same bytes for the REU: `build/flight-store/'
              f'{m}.reu` (the module store and the rope image), compressed (tools/fdz.c), and the flight resident. '
              'The loader is the first file on disk 1. Image hashes, from each run\'s boot record:', '',
              '| File | sha256 |', '|---|---|']
    seen = {}
    for n in ('1581', '1541'):
        for f, h in boots.get(n, {}).get('sha256', {}).items():
            seen.setdefault(f, h)
    for f, h in seen.items():
        lines.append(f'| `{f}` | `{h}` |')
    lines += ['', '## Boot to P00', '',
              '| Run | Disks and load | Keys entered through | Load | Store check (rk_boot) | Boot status | '
              'MODREG | FAILREG | P00 | REU bounds | Result | Evidence |', '|---|---|---|---|---|---|---|---|---|---|---|---|']
    for n, (_, how, keys) in RUNS.items():
        r = runs.get(n)
        if not r:
            lines.append(f'| {n} | {how} | {keys} | | | | | | | | not run | |')
            continue
        v = r['vice']
        swaps = f" (disks asked for: {', '.join(map(str, v['swaps']))})" if v.get('swaps') else ''
        files = [ev / f'{m}-{n}.json', ev / f'{m}-{n}-vice.log'] + \
            [f for f in (ev / f'{m}-{n}-monitor.log', ev / f'{m}-{n}-boot.json') if f.exists()]
        rb = v.get('reu_bounds')
        bounds = ('not checked' if not rb else
                  f"inside ({rb.get('dma_unchecked')} DMAs checked at their last byte only)"
                  if rb.get('ok') and traced.get(n) else
                  f"FAIL: {rb.get('dma_outside', 0)} DMAs outside, {len(rb.get('outside', []))} written ranges"
                  + (f"; {'; '.join(trace_errors[n])}" if trace_errors.get(n) else ''))
        lines.append(f"| {n} | {how}{swaps} | {keys} | {v.get('load_s')} s | {v.get('boot_s')} s | "
                     f"{v.get('boot_status')} | {v.get('modreg')} | {' '.join(v.get('failreg') or [])} | "
                     f"{'yes' if v.get('p00') else 'no'} | {bounds} | {'pass' if oks[n] else 'FAIL'} | "
                     f"{'<br>'.join(digest(f) for f in files)} |")
    lines += ['', 'Load is the loader reading and expanding the disks into the REU (from its own CIA count, '
              'PAL cycles); the store check is the flight resident verifying every module header and CRC, the '
              'resolver and the rope\'s site words before the GOJAM. The KERNAL\'s own load of the 6 KB loader '
              'adds about 10 s. A disk asked for twice means the loader did not take the first try: the run attaches the next image and presses RETURN at once, and the loader asks again when the id sector it reads is not the new disk\'s or the read fails, as it would on real hardware; the second try loads. The loader\'s message at each ask is in the run\'s JSON (`swap_prompts`). Boot status 0 means every store check passed. Each boot record '
              f'(`{m}-<run>-boot.json`, written by `flight_disk.py --p00`) binds the commit, the disk images, '
              'the loader and resident PRGs, the REU store image and the VICE log by SHA-256.', '',
              'REU bounds is the check of acceptance item 9 (tools/reu_layout.py, #681) on each boot. The '
              'loader first checks for a 2 MB REU (one byte at the start of each 64 KB bank) and clears all of '
              'it. From then on VICE traces every REU DMA, read or write: the disks\' data, the hand-off\'s '
              'fetch of the resident and the AGC\'s run, and logs any whose range is not inside the program\'s '
              'declared layout plus the resident\'s stage at the top of the REU (`<run>-monitor.log`; '
              'tools/flight_disk.py boot_layout). VICE now and then evaluates a DMA\'s condition only after it ran; '
              'then only its last byte is range-checked (the count is in the table, #681\'s limit as in the '
              'flight runs), and its writes are still covered by the byte check: after the run, every REU byte '
              'that differs from the store image must lie in a writable region. A run passes only with both '
              'clean, the trace\'s three checkpoints set as VICE numbered them and at least one DMA seen, '
              'which this tool checks again from the monitor log. The run\'s JSON lists the regions it '
              'checked against.', '']
    b = boots.get('1581') or next(iter(boots.values()), None)
    if b and b.get('screen'):
        shot = ev / f'{m}-1581-dsky.png'
        lines += ['## The DSKY after P00', '',
                  'The C64 screen (the DSKY) at the end of the 1581 run, as text from screen memory '
                  f"(sha256 of the text `{b['screen_sha256'][:16]}...`):", '', '```text', *trim(b['screen']), '```', '']
        if shot.exists():
            lines += [f'Screenshot: {digest(shot)}.', '']
        lines += ['The digits are what the AGC sent to the DSKY relays (channel 10, through T4RUPT); the lamps '
                  'are channel 11\'s, row 12\'s and the restart flip-flop. Lit lamps show in yellow on the '
                  'screen, so the text above shows every label.', '']
    kbd = runs.get('1581-keyboard')
    lines += ['## Keyboard', '']
    if kbd:
        lines += ['In the `1581-keyboard` run the keys are pressed on VICE\'s window through the X server (XTEST), '
                  'so they reach the AGC the way a user\'s do: the C64\'s keyboard scan reads the CIA matrix 25 '
                  'times a second and queues each DSKY key, and the AGC takes it at its next idle as a KEYRUPT. '
                  f"RSET, `V37` ENTR `00` ENTR reached P00: {'yes' if kbd['vice'].get('p00') else 'no'}. The key map "
                  '(docs/RUN-ON-C64.md, The DSKY): `0`-`9`, `+`, `-`, `V` VERB, `N` NOUN, RETURN ENTR, `C` or '
                  'DEL CLR, `R` RSET, `K` KEY REL, `P` PRO.', '']
    else:
        lines += ['The keyboard run was skipped.', '']
    lines += ['## In CI', '',
              'Every port-ci run whose changes touch a file on the watch list in tools/mk/ci-extra.mk '
              '(`ci-watch-flight-disk-vice`: the disk tool, the loader, the drive code, the decompressor, the '
              'resident\'s main, the REU store code and the REU layout check) runs '
              '`make flight-disk-vice`: the `1581` run above for both programs, and the check disk. It fails '
              'unless both reach P00, the boots stay inside the REU layout and the check passes. A change outside the list (the store\'s modules, the rest of the resident) does not re-run it, so a program\'s flip PR regenerates this doc at its own commit (tools/boot_acceptance.py). The run keeps the boot records '
              '(`flight-disk-boot-<m>.json`), their VICE logs and the disk images it booted in its '
              '`c64…-logs` artifact.', '']
    lines += ['## Hardware check disk' + (" (the LM build's)" if m != 'lm' else ''), '']
    if check and m != 'lm':
        lines += ["There is one check disk, and it is built from the LM's store: it checks the machine (the "
                  'C64, its REU and its drive) rather than one program, so it runs LM code, not Comanche055. It '
                  'is reported here because a C64 that fails it is not fit to run either program. This '
                  "program's own boot is the runs above.", '']
    if check:
        v = check['vice']
        lines += ['The check disk (`test-1.d81`, `LOAD"APOLLO TEST",8`) loads the LM\'s store like the LM disk, '
                  'then measures the video standard, the REU\'s size and DMA speed, runs the LM\'s P63 for '
                  '200 cs from a host snapshot with the host\'s inputs, and compares a CRC of the end state '
                  'against the one VICE gave when the disk was made (docs/RUN-ON-C64.md, Check on real hardware). '
                  f"In VICE: CRC {v.get('check_crc', '').upper()} against {v.get('check_expect', '').upper()}, "
                  f"{'PASS' if v.get('ok') else 'FAIL'}; load {v.get('load_s')} s; at commit "
                  f"`{(check.get('commit') or 'unknown')[:12]}`, with "
                  f"{', '.join(f'`{f}` (sha256 `{h[:16]}...`)' for f, h in check.get('sha256', {}).items()) or 'no image hashes'}. "
                  'The CRC VICE gives is baked into the disk when it is made, so it moves with the store; the '
                  'commit and the image hash tie this result to the boots above. Evidence: '
                  f"{digest(check_path)}, {digest(ev / f'{m}-check-disk-vice.log')}.", '',
                  '```text', *trim(v.get('screen', [])), '```', '',
                  "The `HOST RUN'S CRC ... (NOT YET THE SAME)` line is not a boot failure. The check passes "
                  "when the C64 computes what VICE does; the host run's CRC is shown for reference and differs "
                  'because of the known C64-versus-host mismatch in this P63 window '
                  '(docs/RUN-ON-C64.md, Check on '
                  'real hardware). It becomes the same when that window matches the host.', '']
    else:
        lines += ['Not run.', '']
    lines += ['## Real hardware', '',
              'Not yet run on a real C64. The first try is the check disk above on David\'s C64 Ultimate; its '
              'photographed screen will be added here. It is not a blocker for this category: the acceptance plan '
              'counts real hardware as a bonus.', '',
              '## Reproduce', '', '```sh',
              'make flight-store build/agc_engine_hardware_dv.o build/fixed-data/Luminary099/MAIN.agc.bin '
              'build/fixed-data/Comanche055/MAIN.agc.bin',
              f'python3 tools/boot_acceptance.py {m}      # all runs: about 12 minutes in VICE warp',
              '```', '']
    path = ROOT / 'docs/acceptance' / f'{p}-bootable_c64_application.md'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines))
    return path, ok, oks, check


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', choices=sorted(PROGRAM))
    ap.add_argument('--skip', default='', help=f"runs to leave out: {', '.join(RUNS)}, test")
    ap.add_argument('--doc-only', action='store_true', help='rewrite the doc from the evidence in the tree')
    a = ap.parse_args(argv)
    dirty = git('status', '--porcelain', '--untracked-files=no', '--', *SOURCES)
    commit = git('rev-parse', '--short=12', 'HEAD') + (' (dirty)' if dirty else '')
    if a.doc_only:
        # The runs' commit, from their boot records: the doc reports what ran.
        recs = sorted((ROOT / 'evidence' / f'{a.mission}-acceptance-boot').glob('*-boot.json'))
        ran = {json.loads(r.read_text())['commit'][:12] for r in recs}
        if len(ran) == 1:
            commit = ran.pop()
    else:
        stage = STAGE / a.mission
        shutil.rmtree(stage, ignore_errors=True)
        old = ROOT / 'evidence' / f'{a.mission}-acceptance-boot'
        shutil.rmtree(old, ignore_errors=True)
        run_all(a.mission, set(filter(None, a.skip.split(','))), stage)
    path, ok, oks, check = write_doc(a.mission, commit)
    print(f"{path.relative_to(ROOT)}: {'PASS' if ok else 'FAIL'} (sha256 {sha(path)})")
    for n, good in oks.items():
        if not good:
            print(f'FAIL: {n}: P00 not reached')
    if not check or not check['vice'].get('ok'):
        print('FAIL: the check disk did not pass')
    return 0 if ok else 1


if __name__ == '__main__':
    os.environ.setdefault('SKIP_EVIDENCE_RESTORE', '1')
    sys.exit(main())
