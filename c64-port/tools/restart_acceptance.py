#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's recovery_and_restart_tests evidence
(docs/PORT_SCOPE.md, integrated_applications).

  restart_acceptance.py lm|cm

Builds what it needs (make flight-store build/agc_engine_hardware_dv.o),
then runs, all at one clean commit (flight_holes.source_commit):

- the host restart check (tools/flight_restart_check.py) for every restart
  row of the program: a hardware restart (GOJAM) in flight, the restart
  and the resumed program in lockstep with yaAGC, the major mode after it
  the same as without the restart;
- for each row that is a flight scenario, the recovery replayed on the C64
  (tools/flight_scenario_vice.py --replay --replay-exact --strict-start,
  from the host's GOJAM point, REPLAY_CS centiseconds, a checkpoint every
  REPLAY_EVERY cs): the C64 must start at the GOJAM point, end the window in
  the host's state with no compare skipped, and keep every REU DMA inside
  the program's layout;
- for the rows CI runs that way (make flight-gojam-vice), the GOJAM applied
  on the C64 itself (flight_restart_check.py --vice --gojam-module): the
  GOJAM module's image against the post-GOJAM snapshot, then the restart
  burst against yaAGC.

A result counts only when this run wrote it, at this commit, and the run
that wrote it did not fail without saying so. Each row's results, with
their commit and the log, store image, manifest and PRG sha256, are stored
as evidence/<m>-acceptance-restart/<name>.json with the logs
(tools/evidence.py add --tree), then
docs/acceptance/<Program>-recovery_and_restart_tests.md is written. It
exits 1, and says why, when a row fails, a phase group has no row that
passes on the host and the C64, a restart row is in no group, or the tree
is dirty; the doc is still written.
"""
import argparse
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402
import flight_host_differential as fhd  # noqa: E402
import flight_restart_check as frc  # noqa: E402

PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
REPLAY_CS = 300
REPLAY_EVERY = 50
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
# Restart rows in the doc that stand for no phase group. A restart row in
# neither (a new row of flight_restart_check.RESTARTS) fails the doc until
# it is placed here or in GROUPS.
EXTRA = {'lm': ['lm-dap-hold-restart', 'lm-v69-restart'], 'cm': ['cm-p52-restart', 'cm-v69-restart']}
# The rows whose GOJAM is also applied on the C64 (make flight-gojam-vice).
GOJAM_ON_C64 = {'lm': ['lm-p63-descent-restart', 'lm-dap-hold-restart', 'lm-v69-restart'],
                'cm': ['cm-entry-restart', 'cm-p52-restart', 'cm-v69-restart']}
OUT = ROOT / 'build/restart-acceptance'
DOCS = ROOT / 'docs/acceptance'
EVIDENCE = ROOT / 'evidence'
STORE = ROOT / 'build/flight-store'
VICE_RESULT = STORE / 'flight-scenario-vice.json'
PRG = STORE / 'dap-pass'
# Result fields too bulky for the evidence copy (per-step records, profile).
BULKY = ('records', 'profile', 'watched', 'by_request', 'words')
TREE_LIMIT = 256 * 1024
# The files a C64 replay row must hash (c64_replay).
HASHES = ('log_sha256', 'store_sha256', 'manifest_sha256', 'prg_sha256')


def sha(path):
    path = pathlib.Path(path)
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None


def source_commit():
    return flight_holes.source_commit()


def tool(args, log):
    """Run one of the port's tools from c64-port, its output to log; its exit code."""
    r = subprocess.run([sys.executable, *args], cwd=ROOT, capture_output=True, text=True)
    log.write_text(r.stdout + r.stderr)
    return r.returncode


def prerequisites():
    return subprocess.run(['make', '-s', 'flight-store', 'build/agc_engine_hardware_dv.o'], cwd=ROOT).returncode


def replayable(name):
    try:
        fhd.scenario(name)
        return True
    except KeyError:
        return False


def restart_check(names, commit, log, gojam_module=False):
    """{name: row} from one flight_restart_check.py run of these rows, each
    stamped with the commit and the log. A row the run did not write is
    {ok: False, failures: [why]}; only a summary this run wrote counts."""
    summary = frc.OUT / ('summary-gojam-module.json' if gojam_module else 'summary.json')
    summary.unlink(missing_ok=True)
    args = ['tools/flight_restart_check.py', '--only', ','.join(names)]
    if gojam_module:
        args[1:1] = ['--vice', '--gojam-module']
    code = tool(args, log)
    try:
        got = {x['scenario']: x for x in json.loads(summary.read_text())}
    except (OSError, ValueError):
        got = {}
    # It exits 1 when a row fails; a failed run with every row passing
    # (or none written) says nothing about the rows.
    unexplained = code != 0 and all(x.get('ok') for x in got.values())
    out = {}
    for n in names:
        row = got.get(n)
        if row is None:
            row = dict(scenario=n, ok=False, failures=[f'flight_restart_check.py exited {code} without this row '
                                                       f'(see {log.name})'])
        elif unexplained:
            row = dict(row, ok=False, failures=[f'flight_restart_check.py exited {code} (see {log.name})'])
        out[n] = dict(row, commit=commit, log=log.name, log_sha256=sha(log))
    return out


def gojam_point(h):
    """The centisecond the host's GOJAM landed on: the asked one moves to the
    next clean sample point (lm-rendezvous 20000 -> 20007)."""
    g = (h or {}).get('gojam') or []
    return int(g[0][0]) if g and g[0] else None


def c64_replay(name, at, log):
    """The recovery replayed on the C64 from the host's GOJAM point."""
    VICE_RESULT.unlink(missing_ok=True)
    code = tool(['tools/flight_scenario_vice.py', '--scenario', name, '--snap-cs', str(at), '--cs', str(REPLAY_CS),
                 '--replay', '--replay-every', str(REPLAY_EVERY), '--replay-exact', '--strict-start'], log)
    try:
        r = json.loads(VICE_RESULT.read_text())
    except (OSError, ValueError):
        tail = log.read_text().strip().splitlines()[-1:] if log.exists() else []
        r = dict(ok=False, failure=tail or [f'exited {code} without a result'])
    m = fhd.scenario(name)[1]
    r = {k: v for k, v in r.items() if k not in BULKY}
    r.update(exit=code, log=log.name, log_sha256=sha(log), store_sha256=sha(STORE / f'{m}.reu'),
             manifest_sha256=sha(STORE / f'{m}-manifest.json'), prg_sha256=sha(PRG / f'{name}-{at}.prg'))
    return r


def host_verdict(h, commit):
    """(ok, text) for one host restart row."""
    if not h:
        return False, 'no result'
    if 'routine_mismatches' not in h:
        return False, 'FAILED: ' + '; '.join(h.get('failures') or ['no result'])
    text = (f"{len(h['routine_mismatches'])} yaAGC mismatches ({len(h['new_mismatches'])} new); "
            f"major mode {h['modreg'][1]} (without the restart {h['modreg'][0]}); "
            f"REDOCTR {h['redoctr'][0]} to {h['redoctr'][1]}")
    bad = []
    if h.get('commit') != commit:
        bad.append('result from another commit')
    if not h.get('ok'):
        bad += h.get('failures') or ['failed']
    # Lockstep with yaAGC means no mismatch at all, not only none new, and
    # no point after the restart that yaAGC had to bridge (#762).
    if h['routine_mismatches']:
        bad.append(f"{len(h['routine_mismatches'])} yaAGC mismatches")
    if h.get('new_holes'):
        bad.append(f"{len(h['new_holes'])} points bridged after the restart (new_holes)")
    if 'new_holes' not in h:
        bad.append('no record of points bridged after the restart')
    return (False, text + '; FAILED: ' + '; '.join(bad)) if bad else (True, text)


def c64_verdict(c, at, commit):
    """(ok, text) for one C64 replay of a restart: it must start at the GOJAM
    point, compare everything, keep the REU inside its layout and end the
    window in the host's state, at this commit. None when there is no replay."""
    if c is None:
        return None, 'not a flight scenario (host only)'
    bad = []
    if 'replay_matched_cs' in c:
        matched = ', '.join(map(str, c['replay_matched_cs'])) or 'no point'
        if c.get('ok'):
            text = f'matches the host at {matched} cs and ends in its state'
        else:
            at_cs = c.get('replay_differ_at_cs')
            text = (f"matches at {matched}; {c.get('replay_differences')} differences"
                    + (f' at {at_cs} cs' if at_cs is not None else ''))
            bad.append('differs from the host' if c.get('replay_differences') else 'did not end the window')
    else:
        text = 'no replay result: ' + ' '.join(c.get('failure') or ['see the log'])
        bad.append('no replay result')
    if 'replay_matched_cs' in c and not c['replay_matched_cs']:
        bad.append('no checkpoint compared')
    if c.get('replay_unpaired'):
        bad.append('checkpoints with no partner: ' + ', '.join(f"{u['side']} {u['cs']} cs" for u in c['replay_unpaired']))
    elif 'replay_matched_cs' in c and c.get('replay_unpaired') is None:
        bad.append('no record of unpaired checkpoints')
    if c.get('commit') != commit:
        bad.append('result from another commit')
    if c.get('snap_at') != at:
        bad.append(f"window starts at {c.get('snap_at')} cs, not at the GOJAM ({at} cs)")
    b = c.get('reu_bounds')
    if not b:
        bad.append('no REU bounds result')
    else:
        inside = b.get('ok') is True and b.get('dma_outside') == 0
        text += f"; REU {'inside the layout' if inside else 'OUTSIDE the layout'}"
        if not inside:
            bad.append(f"{b.get('dma_outside') or 0} DMAs outside the REU layout"
                       + (f", writes in {', '.join(x['region'] for x in b['outside'])}" if b.get('outside') else ''))
    if 'replay_matched_cs' in c and c.get('replay_skipped') is None:
        bad.append('no record of skipped compares')
    elif c.get('replay_skipped'):
        bad.append(f"skipped {', '.join(c['replay_skipped'])} (temporary compare skips)")
    missing = [k for k in HASHES if not c.get(k)]
    if 'replay_matched_cs' in c and missing:
        bad.append(f"no {', '.join(missing)}")
    if c.get('ok') and c.get('exit') not in (0, None):
        bad.append(f"flight_scenario_vice.py exited {c['exit']}")
    if bad:
        return False, text + '; FAILED: ' + '; '.join(bad)
    return True, text


def gojam_verdict(g):
    """(ok, text) for the GOJAM applied on the C64; None when not run."""
    if g is None:
        return None, 'not run'
    c = g.get('c64_gojam')
    if not c:
        return False, 'FAILED: ' + '; '.join(g.get('failures') or ['no C64 result'])
    j, b = c.get('gojam') or {}, c.get('burst') or {}
    text = (f"module {(c.get('module') or {}).get('id')}: {j.get('words_equal')}/2048 words equal the post-GOJAM "
            f"snapshot; burst {b.get('words_equal')}/2048 words equal yaAGC's")
    return (True, text) if c.get('ok') and g.get('ok') else (False, text + '; FAILED')


def run(m, commit):
    dest = OUT / m
    shutil.rmtree(dest, ignore_errors=True)
    dest.mkdir(parents=True)
    names = [x[0] for x in frc.RESTARTS if x[1] == m]
    host = restart_check(names, commit, dest / 'host.log')
    on_c64 = [n for n in GOJAM_ON_C64[m] if n in names]
    gojam = restart_check(on_c64, commit, dest / 'gojam-on-c64.log', gojam_module=True) if on_c64 else {}
    rows = []
    for name in names:
        at = gojam_point(host[name])
        c = None
        if replayable(name):
            c = (c64_replay(name, at, dest / f'{name}-c64.log') if at is not None
                 else dict(ok=False, failure=['no host GOJAM point to start from']))
        rows.append(dict(name=name, gojam=at, asked=next(x[5] for x in frc.RESTARTS if x[0] == name),
                         host=host[name], c64=c, gojam_on_c64=gojam.get(name)))
    return rows


def evidence(m, rows):
    """evidence/<m>-acceptance-restart/: one JSON per row and the logs; the
    folder is rewritten, so no row from an older run lingers."""
    ident = f'{m}-acceptance-restart'
    shutil.rmtree(EVIDENCE / ident, ignore_errors=True)
    files, logs = [], set()
    for r in rows:
        p = OUT / m / f"{r['name']}.json"
        p.write_text(json.dumps({k: r[k] for k in ('name', 'gojam', 'asked', 'host', 'c64', 'gojam_on_c64')},
                                indent=1) + '\n')
        r['evidence'], r['sha256'] = f'evidence/{ident}/{p.name}', sha(p)
        files.append(p)
        logs |= {x['log'] for x in (r['host'], r['c64'], r['gojam_on_c64']) if x and x.get('log')}
    files += [OUT / m / x for x in sorted(logs) if (OUT / m / x).is_file() and (OUT / m / x).stat().st_size <= TREE_LIMIT]
    subprocess.run([sys.executable, 'tools/evidence.py', 'add', ident, *map(str, files), '--tree'],
                   cwd=ROOT, check=True)


def assess(m, rows, commit, end_commit, store_sha):
    """Each row's verdicts; [(group, rows, ok)]; the doc's problems (none: PASS)."""
    in_p00 = {n for g, ns in GROUPS[m] if g == 'P00' for n in ns}
    for r in rows:
        r['host_ok'], r['host_text'] = host_verdict(r['host'], commit)
        mode = ((r['host'] or {}).get('modreg') or [None, None])[1]
        if r['host_ok'] and r['name'] not in in_p00 and r['name'] not in EXTRA[m] and mode in ('00000', '77777'):
            # The same major mode with and without the restart proves little
            # when both runs have left the phase (#762: cm-p22-restart).
            r['host_ok'] = False
            r['host_text'] += '; FAILED: ends in P00, outside the phase it stands for'
        r['c64_ok'], r['c64_text'] = c64_verdict(r['c64'], r['gojam'], commit)
        r['gojam_ok'], r['gojam_text'] = gojam_verdict(r['gojam_on_c64'])
        r['ok'] = r['host_ok'] and r['c64_ok'] is not False and r['gojam_ok'] is not False
    by = {r['name']: r for r in rows}
    groups = [(g, [by[n] for n in ns if n in by]) for g, ns in GROUPS[m]]
    groups = [(g, rs, any(r['ok'] and r['c64_ok'] for r in rs)) for g, rs in groups]
    placed = {n for _, ns in GROUPS[m] for n in ns} | set(EXTRA[m])
    problems = []
    if '(dirty)' in commit:
        problems.append('the tree has changes not in the commit: results must come from a clean commit')
    if end_commit != commit:
        problems.append('the tree changed during the run')
    if not store_sha:
        problems.append('no flight store image')
    problems += [f"{r['name']}: the C64 ran another store image" for r in rows
                 if r['c64'] and r['c64'].get('store_sha256') not in (None, store_sha)]
    problems += [f"restart row {r['name']} is in no phase group (add it to GROUPS or EXTRA)"
                 for r in rows if r['name'] not in placed]
    problems += [f"{r['name']}: host {r['host_text']}; C64 {r['c64_text']}"
                 + (f"; GOJAM on the C64 {r['gojam_text']}" if r['gojam_ok'] is False else '')
                 for r in rows if not r['ok']]
    problems += [f'phase group {g}: no row passes on the host and the C64' for g, _, good in groups if not good]
    return groups, problems


def write_doc(m, rows, groups, problems, commit, store_sha):
    p = PROGRAM[m]
    ok = not problems
    lines = [f'# {p}: recovery and restart tests', '',
             f'Generated by `tools/restart_acceptance.py {m}` at commit `{commit}`. '
             f"Result: **{'PASS' if ok else 'FAIL'}**", '',
             f'Flight store image: `flight-store/{m}.reu` sha256 `{store_sha}`', '',
             'This is the `recovery_and_restart_tests` evidence of the '
             f'`integrated_applications.{p}` entry (docs/PORT_SCOPE.md). Each row is a hardware restart (GOJAM) '
             'in flight. On the host (tools/flight_restart_check.py) the restart and the resumed program run in '
             "lockstep with yaAGC to the row's end, and the program ends in the same major mode as the run "
             'without the restart.', '',
             'On the C64 (tools/flight_scenario_vice.py --replay --replay-exact --strict-start, VICE, a stock C64 '
             "with the store in an REU) the window starts at the host's GOJAM point, just after the GOJAM, so it "
             f"replays the recovery: {REPLAY_CS} cs with the host's inputs, a checkpoint every {REPLAY_EVERY} cs. "
             'At each checkpoint the C64 must equal the host in erasable 0020-3777 (less the clock scalers '
             '0024-0031 and RUPTREG1-4, 0070-0073), the timers and the output channels, with no compare skipped, '
             "and every REU DMA must stay inside the program's layout (tools/reu_layout.py). The window covers the "
             "recovery, not the whole resumed phase, which the host covers to the row's end. For the rows "
             'GOJAM_ON_C64 names, the GOJAM is also applied on the C64 itself (the GOJAM module, then the restart '
             'burst against yaAGC). Speed is not a criterion. So the chain is: the host matches yaAGC, and the '
             'C64 matches the host.', '',
             "The inputs (keys, loads, radar and vehicle models) are each scenario's, in "
             'tools/flight_host_differential.py (FLIGHT, RESTART_POINTS) and tools/flight_restart_check.py '
             '(RESTARTS).', '',
             '## Phase groups', '',
             'A group passes when one of its rows passes on the host and on the C64.', '',
             '| Phase group | Rows | Result |', '|---|---|---|']
    for g, rs, good in groups:
        lines.append(f"| {g} | {', '.join(r['name'] for r in rs) or '(none)'} | {'pass' if good else 'FAIL'} |")
    lines += ['', '## Restarts', '',
              '| Row | GOJAM cs (asked) | Host against yaAGC | C64 against the host | GOJAM on the C64 | Result | Evidence |',
              '|---|---|---|---|---|---|---|']
    for r in rows:
        lines.append(f"| {r['name']} | {r['gojam']} ({r['asked']}) | {r['host_text']} | {r['c64_text']} | "
                     f"{r['gojam_text']} | {'pass' if r['ok'] else 'FAIL'} | "
                     f"`{r.get('evidence', '-')}` (sha256 `{(r.get('sha256') or '-')[:16]}...`) |")
    if problems:
        lines += ['', '## Why it fails', ''] + [f'- {x}' for x in problems]
    lines += ['', 'Rows marked host only are restarts that are not flight scenarios (the P00 restarts, sampled '
              'unscaled against a free-running yaAGC, and the older scaled rows of tools/flight_restart_check.py), '
              'so they have no C64 replay; each phase group still needs a C64 row. Each evidence file holds the '
              "row's results with their commit and the sha256 of the log, store image, manifest and PRG."]
    if m == 'cm':
        lines += ['', "Alarm 00120 after cm-p20-restart is expected: STARTSB2 resets OPTMODES, and T4RUPT's "
                  'CONTDRVE raises it as on the AGC.']
    lines += ['', '## Reproduce', '', 'From a clean checkout of the commit above:', '', '```sh',
              f'python3 tools/restart_acceptance.py {m}   # builds the flight store and the yaAGC object first',
              '```', '']
    path = DOCS / f'{p}-recovery_and_restart_tests.md'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines))
    return path, ok


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', choices=sorted(PROGRAM))
    a = ap.parse_args(argv)
    commit = source_commit()
    if prerequisites():
        sys.exit('FAIL: make flight-store build/agc_engine_hardware_dv.o failed')
    rows = run(a.mission, commit)
    evidence(a.mission, rows)
    store_sha = sha(STORE / f'{a.mission}.reu')
    groups, problems = assess(a.mission, rows, commit, source_commit(), store_sha)
    path, ok = write_doc(a.mission, rows, groups, problems, commit, store_sha)
    print(f"{path}: {'PASS' if ok else 'FAIL'} (sha256 {sha(path)})")
    for x in problems:
        print(f'FAIL: {x}')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
