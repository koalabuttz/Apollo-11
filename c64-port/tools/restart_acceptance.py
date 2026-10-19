#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's recovery_and_restart_tests evidence
(docs/PORT_SCOPE.md, integrated_applications).

  restart_acceptance.py lm|cm

Runs, all at HEAD (make flight-store first; tools/acceptance_bundle.py
does):

- the host restart check (tools/flight_restart_check.py) for every restart
  row of the program: a hardware restart (GOJAM) in flight, the restart
  and the resumed program in lockstep with yaAGC, the major mode after it
  the same as without the restart;
- for each row that is a flight scenario, the restart replayed on the C64
  (tools/flight_scenario_vice.py --replay from the GOJAM's sample point,
  REPLAY_CS centiseconds, a checkpoint every 50 cs): the C64 must end the
  window in the host's state (all erasable and the output channels);

then writes docs/acceptance/<Program>-recovery_and_restart_tests.md and
stores each run's result as evidence/<m>-acceptance-restart/<name>.json
(tools/evidence.py add --tree). The doc maps every phase group the
acceptance bar names to its rows. It exits 1, and says why, when a row
fails or a phase group has no passing row; the doc is still written.
"""
import argparse
import hashlib
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402
import flight_restart_check as frc  # noqa: E402

PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
REPLAY_CS = 300
# The acceptance bar's phase groups (steward, application-acceptance.md)
# and the restart rows that stand for each.
GROUPS = {
    'lm': [('P00', ['lm-p00-restart', 'lm-p00-doi-restart']),
           ('a burn (DOI, P40 on the DPS)', ['lm-doi-burn-restart']),
           ('landing (P63 braking phase)', ['lm-p63-descent-restart']),
           ('ascent (P12)', ['lm-p12-restart']),
           ('rendezvous (P20 radar tracking)', ['lm-rendezvous-restart'])],
    'cm': [('P00', ['cm-p00-restart', 'cm-p00-p40-restart']),
           ('a burn (P40 SPS, TVC DAP)', ['cm-p40-burn-restart']),
           ('entry (P64)', ['cm-entry-restart']),
           ('rendezvous (P20)', ['cm-p20-restart']),
           ('navigation (P22)', ['cm-p22-restart'])],
}
OUT = ROOT / 'build/restart-acceptance'


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def git(*args):
    return subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True).stdout.strip()


def host_rows(m):
    names = [x[0] for x in frc.RESTARTS if x[1] == m]
    r = subprocess.run([sys.executable, 'tools/flight_restart_check.py', '--only', ','.join(names)],
                       cwd=ROOT, capture_output=True, text=True)
    (OUT / f'{m}-host.log').write_text(r.stdout + r.stderr)
    try:
        rows = json.loads((frc.OUT / 'summary.json').read_text())
    except (OSError, ValueError):
        rows = []
    got = {x['scenario']: x for x in rows}
    return {n: got.get(n) for n in names}


def replayable(name):
    try:
        fhd.scenario(name)
        return True
    except KeyError:
        return False


def c64_row(name, gojam):
    """The restart replayed on the C64 from the GOJAM's sample point."""
    result = ROOT / 'build/flight-store/flight-scenario-vice.json'
    result.unlink(missing_ok=True)
    r = subprocess.run([sys.executable, 'tools/flight_scenario_vice.py', '--scenario', name, '--snap-cs', str(gojam),
                        '--cs', str(REPLAY_CS), '--replay', '--replay-every', '50'],
                       cwd=ROOT, capture_output=True, text=True)
    (OUT / f'{name}-c64.log').write_text(r.stdout + r.stderr)
    try:
        return json.loads(result.read_text())
    except (OSError, ValueError):
        return dict(ok=False, failure=(r.stdout + r.stderr).strip().splitlines()[-1:] or ['no result'])


def run(m):
    OUT.mkdir(parents=True, exist_ok=True)
    dest = OUT / m
    dest.mkdir(parents=True, exist_ok=True)
    host = host_rows(m)
    rows, copies = [], []
    for name, h in host.items():
        gojam = next(x[5] for x in frc.RESTARTS if x[0] == name)
        c64 = c64_row(name, gojam) if replayable(name) else None
        row = dict(scenario=name, gojam_cs=gojam, host=h, c64=c64)
        copy = dest / f'{name}.json'
        copy.write_text(json.dumps(row, indent=1) + '\n')
        copies.append(copy)
        rows.append(dict(name=name, gojam=gojam, host_ok=bool(h and h['ok']), host=h, c64=c64,
                         c64_ok=None if c64 is None else bool(c64.get('ok')),
                         evidence=f'evidence/{m}-acceptance-restart/{copy.name}', sha256=sha(copy)))
    # evidence/<id>/ is flat, with its SHA256.json (tools/ci_guard.py).
    subprocess.run([sys.executable, 'tools/evidence.py', 'add', f'{m}-acceptance-restart', *map(str, copies), '--tree'],
                   cwd=ROOT, check=True)
    return rows


def row_ok(r):
    return r['host_ok'] and r['c64_ok'] is not False


def host_cell(h):
    if not h:
        return 'no result'
    return (f"{len(h['routine_mismatches'])} yaAGC mismatches ({len(h['new_mismatches'])} new); "
            f"major mode {h['modreg'][1]} (without the restart {h['modreg'][0]}); REDOCTR {h['redoctr'][0]} to {h['redoctr'][1]}"
            + ('' if h['ok'] else '; FAILED: ' + '; '.join(h['failures'])))


def c64_cell(c):
    if c is None:
        return 'not a flight scenario (host only)'
    if c.get('ok'):
        return f"matches the host at {', '.join(map(str, c.get('replay_matched_cs', [])))} cs and ends in its state"
    if 'replay_matched_cs' in c:
        at = c.get('replay_differ_at_cs')
        return (f"matches at {', '.join(map(str, c['replay_matched_cs'])) or 'no point'}; "
                f"{c.get('replay_differences')} differences{f' at {at} cs' if at is not None else ''}")
    return 'FAILED: ' + ' '.join(c.get('failure', ['see the log']))


def write_doc(m, rows, commit):
    p = PROGRAM[m]
    by = {r['name']: r for r in rows}
    groups = [(g, [by[n] for n in names if n in by]) for g, names in GROUPS[m]]
    group_ok = [(g, rs, any(row_ok(r) and r['c64_ok'] for r in rs)) for g, rs in groups]
    ok = all(row_ok(r) for r in rows) and all(x[2] for x in group_ok)
    lines = [f'# {p}: recovery and restart tests', '',
             f'Generated by `tools/restart_acceptance.py {m}` at commit `{commit}`. '
             f"Result: **{'PASS' if ok else 'FAIL'}**", '',
             'This is the `recovery_and_restart_tests` evidence of the '
             f'`integrated_applications.{p}` entry (docs/PORT_SCOPE.md). Each row is a hardware restart (GOJAM) '
             'in flight. On the host (tools/flight_restart_check.py) the restart and the resumed program run in '
             'lockstep with yaAGC, and the program ends in the same major mode as the run without the restart. '
             'On the C64 (tools/flight_scenario_vice.py --replay, VICE, stock C64 with a 2 MB REU) the window '
             f'starts at the GOJAM\'s sample point, so it replays the recovery, and runs {REPLAY_CS} cs with the '
             'host\'s inputs; the C64 must match the host at its checkpoints (all erasable and the output channels) '
             'and end in its state. So the chain is: the host matches yaAGC, and the C64 matches the host.', '',
             '## Phase groups', '',
             'A group passes when one of its rows passes on the host and on the C64.', '',
             '| Phase group | Rows | Result |', '|---|---|---|']
    for g, rs, good in group_ok:
        lines.append(f"| {g} | {', '.join(r['name'] for r in rs) or '(none)'} | {'pass' if good else 'FAIL'} |")
    lines += ['', '## Restarts', '',
              '| Row | GOJAM cs | Host against yaAGC | C64 against the host | Result | Evidence |',
              '|---|---|---|---|---|---|']
    for r in rows:
        lines.append(f"| {r['name']} | {r['gojam']} | {host_cell(r['host'])} | {c64_cell(r['c64'])} | "
                     f"{'pass' if row_ok(r) else 'FAIL'} | `{r['evidence']}` (sha256 `{r['sha256'][:16]}...`) |")
    lines += ['', 'Rows marked host only are restarts that are not flight scenarios (the P00 restarts, sampled unscaled '
              'against a free-running yaAGC, and the older scaled rows of tools/flight_restart_check.py), so they '
              'have no C64 replay; each phase group still needs a C64 row. Alarm 00120 after '
              'cm-p20-restart is expected: STARTSB2 resets OPTMODES, and T4RUPT\'s CONTDRVE raises it as on the AGC.',
              '', '## Reproduce', '', '```sh', 'make flight-store', f'python3 tools/restart_acceptance.py {m}', '```', '']
    path = ROOT / 'docs/acceptance' / f'{p}-recovery_and_restart_tests.md'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines))
    return path, ok, group_ok


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', choices=sorted(PROGRAM))
    a = ap.parse_args(argv)
    commit = git('rev-parse', '--short=12', 'HEAD') + (' (dirty)' if git('status', '--porcelain', '--', 'src', 'include', 'platform', 'tools') else '')
    rows = run(a.mission)
    path, ok, group_ok = write_doc(a.mission, rows, commit)
    print(f"{path.relative_to(ROOT)}: {'PASS' if ok else 'FAIL'} (sha256 {sha(path)})")
    for r in rows:
        if not row_ok(r):
            print(f"FAIL: {r['name']}: host {host_cell(r['host'])}; C64 {c64_cell(r['c64'])}")
    for g, rs, good in group_ok:
        if not good:
            print(f'FAIL: phase group {g}: no row passes on the host and the C64')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
