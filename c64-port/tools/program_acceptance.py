#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's integrated_program_tests evidence
(docs/port-coverage.json, integrated_applications).

  program_acceptance.py lm|cm      run what has no result at HEAD (the host differential, each
                                   C64 window), then write the doc and record the evidence
  program_acceptance.py lm|cm --no-run [--host DIR] [--replay-dir DIR] [--run RUN.json ...] [--no-evidence]
                                   only collect the results already made (cheap)
  program_acceptance.py lm|cm --windows    print the C64 --replay runs the doc needs
  program_acceptance.py lm|cm --rescore    re-score the committed doc's stored evidence with
                                   this tool and the current map; exit 1 when its counts
                                   or result differ from the doc's (acceptance_freshness.py)

Reads, all of which must come from the same commit:

  docs/acceptance/<Program>-program-map.json   one row per coverage-map entry (status,
                                               the scenarios that run it) and the
                                               C64 replay windows to run:
                                               [scenario, start cs, cs[, chain K[, every]]]
  <host>/summary.json and its logs             tools/flight_host_differential.py
                                               (default build/flight-store/differential)
  C64 --replay results                         tools/flight_scenario_vice.py --replay
                                               (every <replay-dir>/*.json with replay rows,
                                               default build/flight-store/dap-pass, plus --run)

and writes:

  docs/acceptance/<Program>-integrated_program_tests.md
  evidence/<m>-acceptance-program/host-<scenario>.json      (tools/evidence.py add --tree)
  evidence/<m>-acceptance-program/replay-<scenario>-<cs>.json

For each row: the scenarios' host result against yaAGC (routines that did
not match, points the host bridged) and their C64 --replay windows (matches
host, differs with its class, or stops). A result from another commit is
listed as stale and counts as not run (a result from an
older commit counts only when nothing a run depends on, SOURCES below,
changed since). The doc is rewritten on every run; with --no-run that is
cheap (no flight runs), so the auditor can refresh it after each rerun. It exits 1 unless every row is mapped or not exercised (with a
note giving the reason), every mapped row's scenarios match yaAGC, and one
of the C64 windows a mapped row names (its `windows`: the windows its event
falls in; no other window counts for it) matches the host at one checkpoint
or more.
"""
import argparse
import hashlib
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402

PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
# The replay's timer words: a first difference in one of these is the clock
# class (replay-mismatches.md), not a data path.
TIMERS = {'centiseconds', 'TIME1', 'TIME2', 'TIME3', 'TIME4', 'TIME5', 'TIME6'}
# Result fields too bulky for the evidence copy (per-step records, profile).
BULKY = ('records', 'profile', 'watched', 'by_request')


# The paths a flight result may depend on: the whole port (sources, tools,
# docs/*.json with the layout and weight tables the store reads, and the
# program maps this tool reads) and the AGC sources, less what cannot
# change a run: prose (the generated docs included), stored evidence and
# this tool's own files. A result from an older commit still counts only
# when none of these changed since it.
SOURCES = ['.', ':(top)Luminary099', ':(top)Comanche055',
           ':!*.md', ':!evidence',
           ':!tools/program_acceptance.py', ':!tests/test_program_acceptance.py', ':!tools/mk/program-acceptance.mk']


# A C64 replay does not read the program maps (the host run does: it watches
# each row's reach points), so a map edit leaves replays current.
REPLAY_SOURCES = SOURCES + [':!docs/acceptance/*-program-map.json']


def same_sources(run, head, sources=SOURCES):
    """True when a result made at commit `run` holds for `head`."""
    if run == head:
        return True
    if not run or '(dirty)' in run or '(dirty)' in head:
        return False
    r = subprocess.run(['git', 'diff', '--quiet', run, head.split()[0], '--', *sources], cwd=ROOT,
                       capture_output=True)
    return r.returncode == 0


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def host_result(row):
    """(clean, text) for one flight_host_differential.py summary row."""
    if row is None:
        return False, 'not run'
    bridged = [h for h in row.get('unregistered', []) if not h.get('core_start')]
    if row['exit']:
        return False, f"FAILED (exit {row['exit']})"
    # The C64 is compared with the chunk-tracking host (flight_scenario_vice.py
    # builds it from the store's chunk lists), so the host rows must come from
    # that host too: a host-only build (no chunk lists) runs another schedule
    # of job-slot loads and work clock (#926 F3, #942 B6).
    if not row.get('chunked'):
        return False, 'host ran without the chunk lists (run after a current full store build)'
    import flight_host_differential as fhd  # noqa: E402
    charge, text = fhd.charge_state(row)
    if charge == 'over':
        return False, text
    misses = len(row.get('routine_mismatches', []))
    if misses or bridged:
        where = ', '.join(f"{h['label']} ({h['file']}, lane {h['lane']})" for h in bridged[:3])
        return False, (f'{misses} routines differ' if misses else '0 routines differ') + \
            (f', {len(bridged)} points bridged: {where}' if bridged else '')
    return True, 'matches yaAGC (0 routines differ, 0 points bridged)' + (f'; {text}' if charge else '')


def replay_result(r, bounds_here=True):
    """(matches, short text) for one flight_scenario_vice.py --replay result
    (a window, or a --chain run's summary). A chain's windows carry no REU
    bounds of their own (the summary has them): bounds_here False."""
    if r.get('windows') is not None:
        k = r.get('chain', len(r['windows']))
        bounds = r.get('reu_bounds') or {}
        if not (bounds.get('ok') and not bounds.get('dma_outside') and bounds.get('dma_trace_armed')):
            return False, f'chain of {k}: REU bounds not checked inside the layout'
        if r.get('ok') and len(r['windows']) == k and all(w.get('replay_matched_cs') and not w.get('replay_skipped')
                                                           and not w.get('masked_dead_superbank')
                                                           and w.get('replay_unpaired_count') is not None
                                                           and w['replay_unpaired_count']
                                                           == (w.get('replay_busy_apart_count') or 0)
                                                           for w in r['windows']):
            return True, f'matches host (chain of {k} windows, every seam holds)'
        for n, w in enumerate(r['windows']):
            if w.get('seam'):
                return False, f"chain of {k}: seam {w['seam']} fails"
            if w.get('replay_skipped'):   # words left out of the compare (SCHEDULE_SKIP): not a full match
                return False, f"chain of {k}: window {n} matches host except skipped {', '.join(w['replay_skipped'])}"
            ok, text = replay_result(w, False)
            if not ok:
                return False, f'chain of {k}: window {n} {text}'
        return False, f"chain of {k}: ran {len(r['windows'])} windows"
    if r.get('status') not in (0, None):
        return False, f"stops at {r.get('at', '?')}"
    if r.get('short'):
        return False, f"ends early ({r.get('agc_cs')} of {r.get('requested_cs')} cs)"
    bounds = r.get('reu_bounds') if bounds_here else dict(ok=True, dma_trace_armed=True)
    if not bounds:                       # #766: no record is not a pass
        return False, 'no REU bounds result'
    if not bounds.get('ok') or bounds.get('dma_outside'):
        return False, 'REU access outside the layout'
    if not bounds.get('dma_trace_armed'):
        return False, 'REU DMA trace not armed'
    shift = r.get('replay_end_shift_cs')
    late = f', end compared {shift} cs late' if shift else ''
    if r.get('replay_ended') and r.get('replay_end_paired') is False and not r.get('replay_differences'):
        return False, f'end not compared (the C64 ends {shift} cs after the host, with no host state to match)'
    if r.get('replay_differences') == 0 and r.get('replay_ended'):
        if not r.get('replay_matched_cs'):
            return False, 'no checkpoint compared'
        if r.get('ok') is False:          # the run failed for a reason this tool does not name
            return False, f"run failed ({r.get('error') or 'ok is false'})"
        if r.get('replay_skipped'):       # words left out of the compare (SCHEDULE_SKIP): not a full match
            return False, f"matches host except skipped {', '.join(r['replay_skipped'])}"
        if r.get('replay_unpaired_count') is None:
            return False, 'no record of unpaired checkpoints'
        # #766: a checkpoint with no partner was not compared, and fails. A
        # pair where both sides are busy at different places (#892) is a
        # counted note: the next checkpoint or the end still compares.
        busy = r.get('replay_busy_apart_count') or 0
        lone = r['replay_unpaired_count'] - busy
        if lone > 0:
            return False, f"{lone} checkpoints not compared"
        # #837: a masked superbank (flight_scenario_vice.DEAD_SUPERBANK) is a
        # word left out of the compare, like a skipped one: not a full match.
        dead = r.get('masked_dead_superbank')
        if dead:
            return False, f'matches host except {dead} dead superbanks masked at ADVAN (run --replay-exact)'
        notes = [x for x in (f'{busy} checkpoints busy at different places, not compared' if busy else '', late[2:])
                 if x]
        return True, 'matches host' + (f" ({', '.join(notes)})" if notes else '')
    if not r.get('replay_differences') and r.get('replay_ended') is False:
        return False, 'did not reach the next checkpoint'
    first = (r.get('replay_first') or [{}])[0]
    at = first.get('at', '?')
    cls = 'clock' if at in TIMERS else 'data'
    when = r.get('replay_differ_at_cs')
    return False, (f"differs ({cls}: first at {at}, host {first.get('host')} C64 {first.get('c64')}"
                   + (f', by {when} cs' if when is not None else '') + late + ')')


def body_result(row, host):
    """(reached, text) for a row's `reach` points ([[point, label], ...],
    the body its entry leads to): each must be reached in one of its
    scenarios' host runs (flight_host_differential.py's `arrivals`, every
    centisecond yaAGC ran it; `reached`, the first, in older runs), or the
    row is not run by them, only its refusal. A point two rows share in one
    scenario takes a third field, [point, label, cs]: this row's body runs
    at or after that centisecond. A row whose entry cannot refuse names why
    in `no_reach` instead. None when the row names neither."""
    if not row.get('reach'):
        return (True, 'no body check: ' + row['no_reach']) if row.get('no_reach') else None
    missing = []
    for point, label, *after in row['reach']:
        runs = [host[s] for s in row['scenarios'] if s in host]
        times = [t for r in runs for t in (r['arrivals'].get(point, []) if 'arrivals' in r
                                            else [r.get('reached', {}).get(point)]) if t is not None]
        if not any(t >= after[0] for t in times) if after else not times:
            checked = any(point in r.get('arrivals', r.get('reached', {})) for r in runs)
            missing.append(f"{label} ({point}{f', at or after {after[0]} cs' if after else ''}"
                           f"{'' if checked else ', not checked'})")
    if missing:
        return False, 'body not reached: ' + ', '.join(missing)
    return True, 'body reached: ' + ', '.join(label for _, label, *_ in row['reach'])


def load_host(host_dir, commit):
    """{scenario: summary row}, {scenario: stale commit}."""
    path = host_dir / 'summary.json'
    rows, stale = {}, {}
    if not path.exists():
        return rows, stale
    for row in json.loads(path.read_text()):
        name = row['scenario']
        if not name.endswith('-scaled-x1') or row.get('schedule') != '1,1,2':
            continue
        name = name[:-len('-scaled-x1')]
        log = host_dir / f"{row['scenario']}.log"
        row = dict(row, log_sha256=sha(log) if log.exists() else None)
        if not same_sources(row.get('commit'), commit):
            stale[name] = row.get('commit') or 'unknown'
        else:
            rows[name] = row
    return rows, stale


def load_replays(paths, m, commit):
    """{scenario: [result]} (sorted by window start), [(file, commit)] stale."""
    out, stale = {}, []
    for path in paths:
        try:
            r = json.loads(pathlib.Path(path).read_text())
        except (OSError, ValueError):
            continue
        if not isinstance(r, dict) or 'replay_rows' not in r or not r.get('scenario', '').startswith(f'{m}-'):
            continue
        if 'chain_window' in r:          # one window of a --chain run: its summary counts
            continue
        if not same_sources(r.get('commit'), commit, REPLAY_SOURCES):
            stale.append((pathlib.Path(path).name, r.get('commit') or 'unknown'))
            continue
        out.setdefault(r['scenario'], []).append(r)
    for runs in out.values():
        runs.sort(key=lambda r: r.get('snap_cs') or 0)
    return out, stale


def window_key(w):
    """(scenario, start cs, chain K) of one of a row's windows: [scenario, start cs], or
    [scenario, start cs, cs, K] for a chain (the replay_windows form)."""
    return w[0], w[1], w[3] if len(w) > 3 else 1


def run_key(r):
    """(scenario, start cs, chain K) of a --replay result or chain summary."""
    return r.get('scenario'), r.get('snap_cs'), r.get('chain') or 1


def window_span(r):
    """(first cs, last cs) of host time a --replay result or chain summary
    covers: from where its snapshot landed (snap_at, later than snap_cs when
    the snapshot slid) to that plus the cs it ran."""
    start = r.get('snap_at', r.get('snap_cs')) or 0
    if r.get('windows') is not None:
        ran = sum(w.get('requested_cs') or r.get('requested_cs') or 0 for w in r['windows'])
    else:
        ran = max(r.get('requested_cs') or 0, r.get('replay_end_cs') or 0)
    return start, start + ran


def body_in_window(row, host, runs):
    """(True, text) when each of the row's reach points arrives on the host
    inside a matching C64 window of the same scenario (at or after its third
    field, if any), so the window runs the row's body and not only some other
    stretch of the scenario (#925 A1). A row with no reach point fails: even
    an entry that cannot refuse (no_reach) names a point for its window to
    hold."""
    if not row.get('reach'):
        return False, 'no reach point to find in a C64 window'
    missing = []
    for point, label, *after in row['reach']:
        ok = False
        for s, rs in runs.items():
            r = host.get(s) or {}
            times = [t for t in (r['arrivals'].get(point, []) if 'arrivals' in r
                                 else [r.get('reached', {}).get(point)]) if t is not None and (not after or t >= after[0])]
            ok = ok or any(lo <= t <= hi for run, match, _ in rs if match
                           for lo, hi in [window_span(run)] for t in times)
        if not ok:
            missing.append(f'{label} ({point})')
    if missing:
        return False, 'no matching C64 window covers ' + ', '.join(missing)
    return True, 'a matching C64 window covers ' + ', '.join(label for _, label, *_ in row['reach'])


def assess(rows, host, replays, stale_host=None):
    """Per-row verdicts and the counts."""
    out, stale_host = [], stale_host or {}
    for row in rows:
        h = {s: (False, 'stale (another commit)') if s not in host and s in stale_host
             else host_result(host.get(s)) for s in row['scenarios']}
        named = {(s, snap, k) for s, snap, k in map(window_key, row.get('windows', []))}
        c = {s: [(r, *replay_result(r)) for r in replays.get(s, []) if run_key(r) in named] for s in row['scenarios']}
        mapped = row['status'] == 'mapped'
        unexplained = row['status'] == 'not exercised' and not (row.get('note') or '').strip()
        body = body_result(row, host)
        if body:
            h = {**h, 'body': body}
        host_clean = mapped and bool(h) and all(ok for ok, _ in h.values())
        covered = body_in_window(row, host, c)
        c64_match = mapped and any(match for runs in c.values() for _, match, _ in runs) and covered[0]
        out.append(dict(row=row, host=h, c64=c, host_clean=host_clean, c64_match=c64_match, unexplained=unexplained,
                        covered=covered))
    counts = dict(rows=len(out),
                  mapped=sum(a['row']['status'] == 'mapped' for a in out),
                  not_exercised=sum(a['row']['status'] == 'not exercised' for a in out),
                  tbd=sum(a['row']['status'] not in ('mapped', 'not exercised') for a in out),
                  host_clean=sum(a['host_clean'] for a in out),
                  c64_match=sum(a['c64_match'] for a in out),
                  unexplained=sum(a['unexplained'] for a in out))
    counts['ok'] = (counts['tbd'] == 0 and counts['unexplained'] == 0 and counts['host_clean'] == counts['mapped']
                    and counts['c64_match'] == counts['mapped'])
    return out, counts


def cell(text):
    return str(text).replace('|', '/').replace('\n', ' ')


def write_evidence(m, host, replays, dest, store):
    """Copy each used result into dest and record it under <m>-acceptance-program."""
    dest.mkdir(parents=True, exist_ok=True)
    for old in dest.glob('*.json'):
        old.unlink()
    files = {}
    for name, row in sorted(host.items()):
        p = dest / f'host-{name}.json'
        p.write_text(json.dumps(row, indent=1) + '\n')
        files[('host', name)] = p
    for name, runs in sorted(replays.items()):
        for r in runs:
            p = dest / f"replay-{name}-{r.get('snap_cs')}{'-chain' + str(r['chain']) if r.get('chain') else ''}.json"
            p.write_text(json.dumps({k: v for k, v in r.items() if k not in BULKY}, indent=1) + '\n')
            files[('replay', name, r.get('snap_cs'), r.get('chain'))] = p
    if files and store:
        subprocess.run([sys.executable, 'tools/evidence.py', 'add', f'{m}-acceptance-program',
                        *map(str, files.values()), '--tree'], cwd=ROOT, check=True)
    return {k: (f'evidence/{m}-acceptance-program/{p.name}', sha(p)) for k, p in files.items()}


def write_doc(m, assessed, counts, host, replays, stale_host, stale_runs, evidence, commit, out, windows=(), note=None,
              store_sha=None):
    p = PROGRAM[m]
    ev = lambda key: (f"`{evidence[key][0]}` (sha256 `{evidence[key][1]}`)" if key in evidence  # noqa: E731
                      else '(not stored)')
    lines = [f'# {p}: integrated program tests', '',
             f'Generated by `tools/program_acceptance.py {m}` at commit `{commit}`. '
             f"Result: **{'PASS' if counts['ok'] else 'FAIL'}**.", '',
             f"The C64 windows run the store `flight-store/{m}.reu` (sha256 `{store_sha}`)." if store_sha
             else f"The C64 store `flight-store/{m}.reu` is not built at this commit.", '',
             *([f'**Note:** {note}', ''] if note else []),
             'This is the `integrated_program_tests` evidence of the '
             f'`integrated_applications.{p}` entry (docs/port-coverage.json). It takes every major mode and '
             f'routine {p} uses (`docs/acceptance/{p}-program-map.json`, one row per coverage-map entry) and '
             'shows, for each mapped row, the flight scenarios that run it: their whole-program host run '
             'against yaAGC (`tools/flight_host_differential.py`, scaled time x1) and their C64 windows in '
             'VICE (`tools/flight_scenario_vice.py --replay`: the host\'s inputs replayed on the C64 and its '
             'state compared with the host\'s at each checkpoint).', '',
             'It passes when every row is mapped or not exercised (with its reason), every mapped row\'s '
             'scenarios match yaAGC with no bridged point (and reach the body the row names, its `reach`), and one of the C64 windows the row names (the '
             'windows its event falls in) matches the host at one checkpoint or more. A result from a commit with other port sources or AGC sources (anything but prose and '
             'stored evidence) '
             'is stale and counts as not run.', '',
             '## Counts', '',
             '| Rows | mapped | not exercised | not exercised, no reason | TBD | mapped, host matches yaAGC | '
             'mapped, a named C64 window matches host |',
             '|---|---|---|---|---|---|---|',
             f"| {counts['rows']} | {counts['mapped']} | {counts['not_exercised']} | {counts['unexplained']} | {counts['tbd']} | "
             f"{counts['host_clean']} of {counts['mapped']} | {counts['c64_match']} of {counts['mapped']} |", '',
             '## Rows', '',
             '| Row | Entry | Status | Scenarios | Host vs yaAGC | C64 replay | Note |',
             '|---|---|---|---|---|---|---|']
    section = None
    for a in assessed:
        row = a['row']
        if row['section'] != section:
            section = row['section']
            lines.append(f'| **{cell(section)}** | | | | | | |')
        if row['status'] != 'mapped':
            lines.append(f"| {cell(row['row'])} | {cell(row['entry'])} | {cell(row['status'])} | | | | "
                         f"{cell(row.get('note') or ('no reason given (fails)' if a['unexplained'] else ''))} |")
            continue
        hosts = '; '.join(f"{s}: {t}" for s, (_, t) in a['host'].items())
        c64 = '; '.join(f"{s} {r.get('snap_cs')}: {t}" for s, runs in a['c64'].items() for r, _, t in runs) or \
            ('not run' if row.get('windows') else 'no window named')
        if a['c64']:
            c64 += '; ' + a['covered'][1]
        lines.append(f"| {cell(row['row'])} | {cell(row['entry'])} | mapped | {', '.join(row['scenarios'])} | "
                     f"{cell(hosts)} | {cell(c64)} | {cell(row.get('note') or '')} |")
    used = sorted({s for a in assessed for s in a['row']['scenarios']})
    lines += ['', '## Scenarios', '',
              'Each mapped row\'s scenarios, run whole on the host at this commit (exit, routines yaAGC did '
              'not match, points the host bridged; core-start points of a mid-flight core are not counted).', '',
              '| Scenario | Centiseconds | Native routines run | Instructions bridged (core-start points included) | Result | Evidence |',
              '|---|---|---|---|---|---|']
    for s in used:
        row = host.get(s)
        if row is None:
            lines.append(f"| {s} | | | | {'stale (another commit)' if s in stale_host else 'not run'} | |")
            continue
        lines.append(f"| {s} | {row['centiseconds']} | {row.get('native_routines') or 0:,} | "
                     f"{row.get('bridged_instructions') or 0:,} | {cell(host_result(row)[1])} | {ev(('host', s))} |")
    lines += ['', '## C64 replay windows', '',
              'Each window starts from the host\'s state at its start (a snapshot), applies the host\'s inputs '
              '(PIPA counts, channels, erasable writes, interrupt requests) at the host\'s times, and compares '
              'the C64\'s state with the host\'s every few centiseconds. "clock" means the first difference is '
              'in a timer (the AGC-time charge class in /mnt/project-files/flight-scenarios/replay-mismatches.md); '
              '"data" means another word differs first.', '',
              '| Scenario | Start cs | Window cs | Result | Checkpoints matched | C64 speed | Evidence |',
              '|---|---|---|---|---|---|---|']
    windows, plan = [(s, r) for s in used for r in replays.get(s, [])], windows
    for s, r in windows:
        k = r.get('chain')
        parts = r.get('windows') or [r]
        lines.append(f"| {s} | {r.get('snap_at', r.get('snap_cs'))} | "
                     f"{r.get('requested_cs')}{f' x {k}' if k else ''} | {cell(replay_result(r)[1])} | "
                     f"{sum(len(w.get('replay_matched_cs') or []) for w in parts)} | "
                     f"{parts[0].get('wall_per_agc') or ''}x | {ev(('replay', s, r.get('snap_cs'), k))} |")
    ran = {(s, r.get('snap_cs'), r.get('chain') or 1) for s, r in windows}
    for w in sorted(map(tuple, plan)):
        s, snap, cs, k = (*w, 1)[:4]
        if (s, snap, k) not in ran:
            lines.append(f"| {s} | {snap} | {cs}{f' x {k}' if k > 1 else ''} | not run at this commit | | | |")
    if not windows and not plan:
        lines.append('| (none at this commit) | | | | | | |')
    if stale_host or stale_runs:
        lines += ['', '## Stale inputs (from another commit, not counted)', '']
        lines += [f'- host {s}' for s in sorted(stale_host) if s in used]
        lines += [f'- replay {f}' for f, _ in sorted(stale_runs)]
    lines += ['', '## Reproduce', '', '```sh', 'make flight-store',
              f'python3 tools/program_acceptance.py {m}            # runs the host differential and each C64 window',
              f'python3 tools/program_acceptance.py {m} --no-run   # rewrites this doc from the results already made',
              '```', '']
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text('\n'.join(lines))


COUNTS = re.compile(r'^\| (\d+) \| (\d+) \| (\d+) \| (\d+) \| (\d+) \| (\d+) of \d+ \| (\d+) of \d+ \|$', re.M)
CITED = re.compile(r'`(evidence/[a-z]+-acceptance-program/((?:host|replay)-[^`]+\.json))` \(sha256 `([0-9a-f]{64})`\)')


def rescore(m, map_path=None):
    """(same, text): re-score the committed integrated doc from the evidence it
    cites, with this tool and the current map. A map edit or a scoring change
    that would turn the doc's counts or result leaves the store bytes alone,
    so freshness by bytes alone would call such a doc current (#942 B2)."""
    p = PROGRAM[m]
    doc = (ROOT / f'docs/acceptance/{p}-integrated_program_tests.md').read_text()
    claimed = COUNTS.search(doc)
    if not claimed:
        return False, 'no counts table in the doc'
    claimed = dict(zip(('rows', 'mapped', 'not_exercised', 'unexplained', 'tbd', 'host_clean', 'c64_match'),
                       map(int, claimed.groups())))
    claimed['ok'] = 'Result: **PASS**' in doc[:600]
    host, replays, bad = {}, {}, []
    for path, name, want in CITED.findall(doc):
        f = ROOT / path
        if not f.exists() or sha(f) != want:
            bad.append(name)
            continue
        r = json.loads(f.read_text())
        if name.startswith('host-'):
            host[r['scenario'][:-len('-scaled-x1')] if r['scenario'].endswith('-scaled-x1') else r['scenario']] = r
        else:
            replays.setdefault(r['scenario'], []).append(r)
    if bad:
        return False, f"{len(bad)} cited evidence files missing or changed (run tools/evidence.py fetch --coverage): " \
            + ', '.join(bad[:3])
    for runs in replays.values():
        runs.sort(key=lambda r: r.get('snap_cs') or 0)
    plan = json.loads((map_path or ROOT / f'docs/acceptance/{p}-program-map.json').read_text())
    _, counts = assess(plan['rows'], host, replays)
    diff = [f'{k} {claimed[k]} -> {counts[k]}' for k in claimed if claimed[k] != counts[k]]
    if diff:
        return False, 're-scored with this map and tool: ' + ', '.join(diff)
    return True, f"re-scores to the doc's counts ({counts['c64_match']} of {counts['mapped']} mapped rows on the C64)"


def window_cmd(w):
    """The flight_scenario_vice.py --replay run for a map window
    [scenario, start cs, cs(, chain K(, every))]."""
    s, snap, cs, k, every = list(w) + [1, 100][len(w) - 3:]
    return [sys.executable, 'tools/flight_scenario_vice.py', '--scenario', s, '--snap-cs', str(snap), '--cs', str(cs),
            '--replay', '--replay-every', str(every)] + (['--chain', str(k)] if k > 1 else [])


def run_missing(m, rows, windows, host, replays):
    """Run what the doc needs and has no result for at this commit: the
    host differential (when a mapped scenario has no host result) and each
    map window with no C64 result. True when anything ran."""
    used = {s for row in rows if row['status'] == 'mapped' for s in row['scenarios']}
    ran = False
    if used - set(host):
        print(f'{m}: host results missing for {len(used - set(host))} scenarios; running the host differential',
              flush=True)
        subprocess.run([sys.executable, 'tools/flight_host_differential.py'], cwd=ROOT)
        ran = True
    have = {run_key(r) for runs in replays.values() for r in runs}
    for w in windows:
        if (w[0], w[1], w[3] if len(w) > 3 else 1) not in have:
            print(f'{m}: C64 window {w[0]} {w[1]}+{w[2]}', flush=True)
            subprocess.run(window_cmd(w), cwd=ROOT)
            ran = True
    return ran


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', choices=sorted(PROGRAM))
    ap.add_argument('--host', type=pathlib.Path, default=ROOT / 'build/flight-store/differential')
    ap.add_argument('--replay-dir', type=pathlib.Path, default=ROOT / 'build/flight-store/dap-pass')
    ap.add_argument('--run', action='append', default=[], help='a flight_scenario_vice.py --replay result JSON')
    ap.add_argument('--map', type=pathlib.Path, help='default docs/acceptance/<Program>-program-map.json')
    ap.add_argument('--out', type=pathlib.Path, help='default docs/acceptance/<Program>-integrated_program_tests.md')
    ap.add_argument('--commit', help='the commit the inputs must come from (default HEAD)')
    ap.add_argument('--no-evidence', action='store_true', help='do not record the results with tools/evidence.py')
    ap.add_argument('--note', help='a line printed under the result, such as which fixes the data predates')
    ap.add_argument('--no-run', action='store_true',
                    help='only collect results already made (default: first run the host differential and '
                         'each C64 window with no result at this commit)')
    ap.add_argument('--windows', action='store_true', help="print the map's C64 --replay runs and exit")
    ap.add_argument('--rescore', action='store_true',
                    help="re-score the committed doc's cited evidence with this tool and the map; exit 1 on a change")
    a = ap.parse_args(argv)
    m, p = a.mission, PROGRAM[a.mission]
    if a.rescore:
        same, text = rescore(m, a.map)
        print(f'{p} integrated_program_tests: {text}')
        return 0 if same else 1
    plan = json.loads((a.map or ROOT / f'docs/acceptance/{p}-program-map.json').read_text())
    rows, windows = plan['rows'], plan.get('replay_windows', [])
    if a.windows:
        for w in windows:
            print(' '.join(['python3', *window_cmd(w)[1:]]))
        return 0
    commit = a.commit or flight_holes.source_commit()
    host, stale_host = load_host(a.host, commit)
    replays, stale_runs = load_replays(sorted(a.replay_dir.glob('*.json')) + a.run, m, commit)
    if not a.no_run and not a.commit:
        if run_missing(m, rows, windows, host, replays):
            host, stale_host = load_host(a.host, commit)
            replays, stale_runs = load_replays(sorted(a.replay_dir.glob('*.json')) + a.run, m, commit)
    assessed, counts = assess(rows, host, replays, stale_host)
    used = {s for row in rows for s in row['scenarios']}
    evidence = write_evidence(m, {s: r for s, r in host.items() if s in used},
                              {s: r for s, r in replays.items() if s in used},
                              ROOT / 'build/program-acceptance' / m, not a.no_evidence)
    out = a.out or ROOT / f'docs/acceptance/{p}-integrated_program_tests.md'
    reu = ROOT / f'build/flight-store/{m}.reu'
    write_doc(m, assessed, counts, host, replays, stale_host, stale_runs, evidence, commit, out, windows, a.note,
              sha(reu) if reu.exists() else None)
    print(f"{out.relative_to(ROOT) if ROOT in out.parents else out}: {'PASS' if counts['ok'] else 'FAIL'}: "
          f"{counts['rows']} rows, {counts['mapped']} mapped ({counts['host_clean']} match yaAGC on the host, "
          f"{counts['c64_match']} with a C64 window matching the host), {counts['not_exercised']} not exercised, "
          f"{counts['tbd']} TBD; {len(stale_host)} stale host results, {len(stale_runs)} stale replays")
    return 0 if counts['ok'] else 1


if __name__ == '__main__':
    sys.exit(main())
