#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's native_output_tests evidence
(docs/port-coverage.json, integrated_applications).

  native_output_acceptance.py lm|cm                runs the downlink check and the map's C64 windows, then writes
  native_output_acceptance.py lm|cm --from-results [--replay-dir DIR] [--run RUN.json ...] [--downlink DIR] [--no-evidence]
                                                   writes from results already on disk
  native_output_acceptance.py lm|cm --windows   print the C64 --replay runs the doc needs
  native_output_acceptance.py --stamp-downlink  after tools/flight_downlink_check.py: record the
                                                commit its summary.json belongs to

Reads, all of which must come from the same commit (as
tools/program_acceptance.py: a result from an older commit counts when
the port's sources have not changed since):

  docs/acceptance/<Program>-native-output-map.json   one row per output the program drives:
                                                     the DSKY, the downlink lists, and the
                                                     output channels (RCS jets, engine, lamps,
                                                     CDU, gimbal and radar drives)
  build/flight-downlink/summary.json                 tools/flight_downlink_check.py --vice
  C64 --replay results                               tools/flight_scenario_vice.py --replay
                                                     (build/flight-store/dap-pass, plus --run)

and writes:

  docs/acceptance/<Program>-native_output_tests.md
  evidence/<m>-acceptance-outputs/...               (tools/evidence.py add --tree)

Rows by kind:

  downlink   every named host restart scenario's word stream matches yaAGC's in
             lockstep and one named C64 window takes DOWNRUPTs whose channel
             34/35 words all equal yaAGC's.
  channels   one named C64 --replay window matches the host at a checkpoint
             (the compare covers the erasable words and channels 5, 6, 11-14,
             34 and 35), and one of the row's channels changes in the host's
             states up to that checkpoint, so the window drove it.
  dsky       pending until the DSKY sweep's result exists (the row's note
             names its owner).

A row with `waits_on` is reported as waiting and does not pass. The doc
passes when every row passes or is "not exercised" with a reason.
"""
import argparse
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402
import program_acceptance as pa  # noqa: E402

DOWNLINK = ROOT / 'build/flight-downlink'
WORK = ROOT / 'build/flight-store/dap-pass'
# The output channels of a --replay state, after the 2048 erasable words
# (tools/flight_scenario_vice.py END_CHANNELS).
CHANNELS = [5, 6, 0o11, 0o12, 0o13, 0o14, 0o34, 0o35]
STATE_WORDS = 1 + 2048 + len(CHANNELS) + 1
# This tool's own files do not make a result stale (as program_acceptance.py's).
pa.SOURCES = pa.SOURCES + [':!tools/native_output_acceptance.py', ':!tests/test_native_output_acceptance.py',
                           ':!tools/mk/native-output-acceptance.mk']


def host_states(header):
    """[(cs, {channel: value})]: the snapshot's channels (cs 0), then each
    state the host wrote in the window (its .end file); [] when the files
    are gone."""
    head, end = pathlib.Path(header), pathlib.Path(f'{header}.end')
    if not head.exists() or not end.exists():
        return []
    import flight_scenario_vice as fsv
    out = [(0, dict(zip(CHANNELS, fsv.snap_state(head)[2048:2048 + len(CHANNELS)])))]
    data = end.read_bytes()
    for cs, words in fsv.states(data, len(data) // (2 * STATE_WORDS)):
        out.append((cs, dict(zip(CHANNELS, words[2048:2048 + len(CHANNELS)]))))
    return out


def driven(states, channels, until):
    """{channel: [values]} of the row's channels that change in states at
    or before cs `until`."""
    seen = {}
    for cs, chans in states:
        if cs > until:
            break
        for c in channels:
            seen.setdefault(c, [])
            if chans[c] not in seen[c]:
                seen[c].append(chans[c])
    return {c: v for c, v in seen.items() if len(v) > 1}


def window_states(r):
    """host_states for a --replay result (a chain's windows one after another)."""
    header = WORK / f"{r['scenario']}-{r['snap_cs']}-snap.h"
    if not r.get('chain'):
        return host_states(header)
    out, base = [], 0
    for n in range(r['chain']):
        part = host_states(f'{header}-{n}' if n else header)
        out += [(base + cs, ch) for cs, ch in part]
        base = out[-1][0] if out else base
    return out


def load_downlink(path, commit):
    """(summary or None, stale commit or None)."""
    f = path / 'summary.json'
    if not f.exists():
        return None, None
    s = json.loads(f.read_text())
    stamp = path / 'commit'
    run = s.get('commit') or (stamp.read_text().strip() if stamp.exists() else None)
    if not pa.same_sources(run, commit):
        return None, run or 'unknown'
    return dict(s, commit=run), None


def assess_downlink(row, summary):
    if summary is None:
        return False, 'not run at this commit'
    host = {r['scenario']: r for r in summary.get('host', [])}
    c64 = {r['window']: r for r in summary.get('c64', [])}
    parts, ok = [], True
    for s in row.get('host', []):
        r = host.get(s)
        good = bool(r and r['ok'])
        ok &= good
        parts.append(f"host {s}: " + ('not run' if r is None else
                     f"{r['native_words']} words, {r['span_differ']} differ from yaAGC" + ('' if good else ' (FAILED)')))
    seen = False
    for w in row.get('c64', []):
        r = c64.get(w)
        good = bool(r and r['ok'] and r.get('records') and not r.get('records_differ'))
        seen |= good
        parts.append(f"C64 {w}: " + ('not run' if r is None else
                     f"{r.get('records')} DOWNRUPTs, {r.get('records_differ')} differ"
                     + (', lists ' + ', '.join(x['name'] for x in r.get('list_starts', [])) if r.get('list_starts') else '')))
    return ok and seen, '; '.join(parts)


def assess_channels(row, replays):
    named = {pa.window_key(w) for w in row.get('windows', [])}
    parts, best = [], False
    for s in sorted({w[0] for w in row.get('windows', [])}):
        for r in replays.get(s, []):
            if pa.run_key(r) not in named:
                continue
            match, text = pa.replay_result(r)
            parts_r = r.get('windows') or [r]
            matched = [cs for w in parts_r for cs in (w.get('replay_matched_cs') or [])]
            states = window_states(r)
            moved = driven(states, row['channels'], max(matched)) if matched and states else {}
            if matched and not states:
                text += '; host states gone, rerun to show the channels'
            elif matched:
                text += '; ' + (', '.join(f'ch{c:o} ' + '/'.join(f'{v:05o}' for v in vals[:4])
                                          for c, vals in moved.items()) if moved else
                                'channels unchanged to the last matched checkpoint')
            r.setdefault('outputs_driven', {}).update({f'{c:o}': vals for c, vals in moved.items()})
            best |= bool(matched) and bool(moved) and (match or bool(r.get('replay_matched_cs')))
            parts.append(f"{s} {r.get('snap_cs')}: {text}")
    return best, '; '.join(parts) or ('not run' if row.get('windows') else 'no window named')


def assess(rows, replays, downlink):
    out = []
    for row in rows:
        kind, status = row['kind'], row.get('status', 'mapped')
        if status == 'not exercised':
            ok, text = None, ''
        elif kind == 'downlink':
            ok, text = assess_downlink(row, downlink)
        elif kind == 'channels':
            ok, text = assess_channels(row, replays)
        else:
            ok, text = False, 'pending'
        if row.get('waits_on') and status != 'not exercised':
            ok, text = False, f"waits on {row['waits_on']}; {text}"
        out.append(dict(row=row, ok=ok, text=text,
                        unexplained=status == 'not exercised' and not (row.get('note') or '').strip()))
    counts = dict(rows=len(out), passed=sum(a['ok'] is True for a in out),
                  not_exercised=sum(a['ok'] is None for a in out),
                  waiting=sum(bool(a['row'].get('waits_on')) for a in out),
                  unexplained=sum(a['unexplained'] for a in out))
    counts['ok'] = counts['passed'] + counts['not_exercised'] == counts['rows'] and not counts['unexplained']
    return out, counts


def write_evidence(m, replays, downlink, dest, store):
    dest.mkdir(parents=True, exist_ok=True)
    for old in dest.glob('*.json'):
        old.unlink()
    files = {}
    for name, runs in sorted(replays.items()):
        for r in runs:
            p = dest / f"replay-{name}-{r.get('snap_cs')}{'-chain' + str(r['chain']) if r.get('chain') else ''}.json"
            p.write_text(json.dumps({k: v for k, v in r.items() if k not in pa.BULKY}, indent=1) + '\n')
            files[('replay', name, r.get('snap_cs'), r.get('chain'))] = p
    if downlink is not None:
        p = dest / 'downlink.json'
        keep = lambda rows: [x for x in rows if (x.get('scenario') or x.get('window', '')).startswith(f'{m}-')]  # noqa: E731
        p.write_text(json.dumps(dict(commit=downlink.get('commit'), host=keep(downlink.get('host', [])),
                                     c64=keep(downlink.get('c64', []))), indent=1) + '\n')
        files[('downlink',)] = p
    if files and store:
        subprocess.run([sys.executable, 'tools/evidence.py', 'add', f'{m}-acceptance-outputs',
                        *map(str, files.values()), '--tree'], cwd=ROOT, check=True)
    return {k: (f'evidence/{m}-acceptance-outputs/{p.name}', pa.sha(p)) for k, p in files.items()}


def write_doc(m, assessed, counts, stale_runs, stale_downlink, evidence, commit, out, windows=(), note=None,
              store_sha=None):
    p = pa.PROGRAM[m]
    lines = [f'# {p}: native output tests', '',
             f'Generated by `tools/native_output_acceptance.py {m}` at commit `{commit}`. '
             f"Result: **{'PASS' if counts['ok'] else 'FAIL'}**.", '',
             *([f'**Note:** {note}', ''] if note else []),
             (f'The C64 windows ran on `flight-store/{m}.reu` (sha256 `{store_sha}`).' if store_sha else
              f'`flight-store/{m}.reu` is not built, so no C64 result here holds.'), '',
             'This is the `native_output_tests` evidence of the '
             f'`integrated_applications.{p}` entry (docs/port-coverage.json). It takes each output {p} drives '
             f'(`docs/acceptance/{p}-native-output-map.json`): the DSKY, the downlink lists on channels 34 and 35, '
             'and the output channels (RCS jets on 5 and 6; engine, lamps and alarms on 11; CDU, optics, radar, '
             'gimbal and TVC enables on 12; radar and downlink control on 13; CDU, gyro and thrust drives on 14). '
             'For each it shows the run that compares the native output with the reference:', '',
             '- **Downlink**: `tools/flight_downlink_check.py --vice`. On the host, the native word stream matches '
             'yaAGC\'s in lockstep through each restart scenario; on the C64, DOWNRUPTs taken from a snapshot give the '
             'same channel 34/35 words and word-order bits as yaAGC from the same state.',
             '- **Output channels**: `tools/flight_scenario_vice.py --replay`. The host\'s inputs are replayed on the '
             'C64 and its erasable words and channels 5, 6, 11-14, 34 and 35 are compared with the host\'s at each '
             'checkpoint (the host matches yaAGC on the same inputs: the integrated_program_tests doc). A row counts '
             'a window only when one of its channels changes in the host\'s states up to the last checkpoint the '
             'C64 matched, so the window drove that output.',
             '- **DSKY**: the noun x verb sweep on the C64 build (acceptance bar item 6).', '',
             'A result from a commit with other port sources (src, include, platform, tools, tests) is stale and '
             'counts as not run.', '',
             '## Counts', '',
             '| Rows | pass | not exercised | waiting on a fix | not exercised, no reason |',
             '|---|---|---|---|---|',
             f"| {counts['rows']} | {counts['passed']} | {counts['not_exercised']} | {counts['waiting']} | "
             f"{counts['unexplained']} |", '',
             '## Rows', '',
             '| Output | Kind | Channels | Result | Detail | Note |',
             '|---|---|---|---|---|---|']
    section = None
    for a in assessed:
        row = a['row']
        if row['section'] != section:
            section = row['section']
            lines.append(f'| **{pa.cell(section)}** | | | | | |')
        verdict = {True: 'pass', None: 'not exercised', False: 'waiting' if row.get('waits_on') else
                   ('pending' if row['kind'] == 'dsky' else 'not yet')}[a['ok']]
        chans = ', '.join(f'{c:o}' for c in row.get('channels', [])) or \
            ('34, 35' if row['kind'] == 'downlink' else ('10 (display)' if row['kind'] == 'dsky' else ''))
        lines.append(f"| {pa.cell(row['row'])} | {row['kind']} | {chans} | {verdict} | {pa.cell(a['text'])} | "
                     f"{pa.cell(row.get('note') or '')} |")
    lines += ['', '## Evidence', '']
    lines += [f'- `{path}` (sha256 `{digest[:16]}...`)' for _, (path, digest) in sorted(evidence.items(), key=str)] or \
        ['- none at this commit']
    if stale_runs or stale_downlink:
        lines += ['', '## Stale inputs (not counted)', '']
        lines += [f'- replay {f}: run at `{c}`' for f, c in sorted(stale_runs)
                  if any(f.startswith(f'{w[0]}-') for w in windows)]
        lines += [f'- downlink summary: run at `{stale_downlink}`'] if stale_downlink else []
    lines += ['', '## Reproduce', '', '```sh', 'make flight-store',
              f'python3 tools/native_output_acceptance.py {m}   # runs the downlink check and each C64 window',
              '```', '']
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text('\n'.join(lines))


def window_command(w):
    s, snap, cs, k, every = list(w) + [1, 100][len(w) - 3:]
    return (['tools/flight_scenario_vice.py', '--scenario', s, '--snap-cs', str(snap), '--cs', str(cs), '--replay',
             '--replay-every', str(every)] + (['--chain', str(k)] if k > 1 else []))


def run_needed(m, rows, windows, downlink):
    """The default run (tools/acceptance_bundle.py): the downlink check on the
    map's host scenarios and C64 windows, then each C64 --replay window. A
    failing run is not an error here: its result goes in the doc."""
    only = sorted({x for row in rows if row['kind'] == 'downlink' for x in row.get('host', []) + row.get('c64', [])})
    if only:
        subprocess.run([sys.executable, 'tools/flight_downlink_check.py', '--vice', '--only', ','.join(only)], cwd=ROOT)
        downlink.mkdir(parents=True, exist_ok=True)
        (downlink / 'commit').write_text(flight_holes.source_commit() + '\n')
    for w in windows:
        subprocess.run([sys.executable, *window_command(w)], cwd=ROOT)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', nargs='?', choices=sorted(pa.PROGRAM))
    ap.add_argument('--replay-dir', type=pathlib.Path, default=WORK)
    ap.add_argument('--run', action='append', default=[], help='a flight_scenario_vice.py --replay result JSON')
    ap.add_argument('--downlink', type=pathlib.Path, default=DOWNLINK)
    ap.add_argument('--map', type=pathlib.Path, help='default docs/acceptance/<Program>-native-output-map.json')
    ap.add_argument('--out', type=pathlib.Path, help='default docs/acceptance/<Program>-native_output_tests.md')
    ap.add_argument('--commit', help='the commit the inputs must come from (default HEAD)')
    ap.add_argument('--no-evidence', action='store_true')
    ap.add_argument('--note')
    ap.add_argument('--windows', action='store_true', help="print the map's C64 --replay runs and exit")
    ap.add_argument('--from-results', action='store_true', help='do not run anything: write from the results on disk')
    ap.add_argument('--stamp-downlink', action='store_true',
                    help="write the commit flight_downlink_check.py's summary belongs to")
    a = ap.parse_args(argv)
    if a.stamp_downlink:
        (a.downlink / 'commit').write_text(flight_holes.source_commit() + '\n')
        return 0
    if not a.mission:
        ap.error('mission required')
    m, p = a.mission, pa.PROGRAM[a.mission]
    plan = json.loads((a.map or ROOT / f'docs/acceptance/{p}-native-output-map.json').read_text())
    rows = plan['rows']
    windows = sorted({tuple(w) for row in rows for w in row.get('windows', [])})
    if a.windows:
        for w in windows:
            print(' '.join(['python3', *window_command(w)]))
        return 0
    if not a.from_results:
        run_needed(m, rows, windows, a.downlink)
    commit = a.commit or flight_holes.source_commit()
    replays, stale_runs = pa.load_replays(sorted(a.replay_dir.glob('*.json')) + a.run, m, commit)
    downlink, stale_downlink = load_downlink(a.downlink, commit)
    assessed, counts = assess(rows, replays, downlink)
    used = {w[0] for w in windows}
    evidence = write_evidence(m, {s: r for s, r in replays.items() if s in used}, downlink,
                              ROOT / 'build/native-output-acceptance' / m, not a.no_evidence)
    out = a.out or ROOT / f'docs/acceptance/{p}-native_output_tests.md'
    # The store the windows ran on (acceptance contract rule 9).
    store = ROOT / 'build/flight-store' / f'{m}.reu'
    store_sha = pa.sha(store) if store.exists() else None
    if not store_sha:
        counts['ok'] = False
    write_doc(m, assessed, counts, stale_runs, stale_downlink, evidence, commit, out, windows, a.note, store_sha)
    print(f"{out.relative_to(ROOT) if ROOT in out.parents else out}: {'PASS' if counts['ok'] else 'FAIL'}: "
          f"{counts['rows']} rows, {counts['passed']} pass, {counts['not_exercised']} not exercised, "
          f"{counts['waiting']} waiting on a fix; {len(stale_runs)} stale replays"
          + (f', downlink stale ({stale_downlink})' if stale_downlink else ''))
    return 0 if counts['ok'] else 1


if __name__ == '__main__':
    sys.exit(main())
