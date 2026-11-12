#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The readable x86 program flown against Virtual AGC itself, routine by
routine.

The scenario suite (tests/x86_readable.py scenarios) compares the readable
program with the harness, and the harness runs the faithful port. This
check has no faithful routine in it. It builds the port's lockstep tool
(c64-port/tools/x86_flight_lockstep.c) over the readable objects: the
program's own step runs the mission, and for every routine it runs (and
every slot of the interpreter core) Virtual AGC's engine, yaAGC, is loaded
with the state the routine started in and run on the original rope until
it reaches the routine's continuation. There yaAGC's 2,048 words of
erasable memory (Z aside), its channels and its interrupt state must be
the routine's. A point where they are not is a miss, and a miss fails the
row. So does a row whose counts (centiseconds, routines checked, yaAGC
instructions) are not those of the run of record,
tests/x86_lockstep_record.txt: a check that came to skip routines, or to
stop yaAGC early, would pass every point it still looked at.

The binary this builds links yaAGC, so it is a test and never the readable
program (which must not link it). Of the repository it links the readable
objects, the step, the lockstep tool, the seven machine objects
(x86_readable.MACHINE) and yaAGC's three objects, and nothing else: the
link check of the readable program is run on it too.

  x86_lockstep.py [--quick] [--jobs N] [NAME ...]
      the port's 16 check rows in both interpretive modes (32 rows, named
      <mission>-<row>-<mode>), then the flight scenarios, compiled, from
      GOJAM or from the harness's state at the NASSP core's start point.
      Each row's counts must be the record's. Then the rows left out
      (LEFT_OUT), which must still fail as their reasons say.
      --quick: the check rows and one flight from the core's start point
      (QUICK_FLIGHTS; a few minutes; every row takes about 20 minutes on
      8 processors).
      --record FILE: write the run's rows to FILE in place of comparing
      them (make x86-lockstep-record keeps tests/x86_lockstep_record.txt,
      the run of record; commit it when a row or a count changes, and
      say why).
  x86_lockstep.py --wrong
      the check tried on programs that are wrong: the LM program with one
      erasable word wrong, once; with one channel word wrong; with one
      HANDRUPT trap armed that should not be (tests/x86_lockstep_wrong.c).
      Each time the check must name the routine, and the word.

Run by `make x86-lockstep-quick` and `make x86-lockstep` from readable/
(each runs --wrong too). Needs the port's toolchain
(c64-port/tools/setup_cloud.sh) for yaAGC.
"""
import argparse, concurrent.futures, json, os, re, subprocess, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import x86_readable as xr  # noqa: E402

xb, PORT, READABLE = xr.xb, xr.PORT, xr.READABLE
import flight_host_differential as fhd  # noqa: E402
import x86_flight_check as xc  # noqa: E402
import x86_flight_scenarios as xs  # noqa: E402

OUT = xb.OUT / 'lockstep'
SOURCE = PORT / 'tools/x86_flight_lockstep.c'
RECORD = READABLE / 'tests/x86_lockstep_record.txt'
# The flight scenarios --quick flies with the check rows: one from the
# NASSP core's start point, which the landing's routines arm a HANDRUPT
# trap in (channel 13's bits 12 to 14, which the tool compares as traps).
QUICK_FLIGHTS = ('lm-p63-nassp',)
# A scenario input the x86 program does not read (x86_flight_scenarios:
# the harness's 1,1,2 clock only): two rows that differ in it alone are
# one run.
HARNESS_ONLY = ('FLIGHT_SWAP_MCT',)
# Flight scenarios this check does not count, each with the number of
# points at which it differs and the reason. A full run flies them too,
# and fails if one no longer differs at that many points (then the reason
# no longer holds: fly the row).
LEFT_OUT = {
    'lm-p52-watchman': (2,
        'its job loops in GOTOERS until the night watchman restarts the AGC, and the lockstep '
        'tool cannot follow either. A step of the loop ends where it started (in the readable '
        'interpreter at the GOTO order after 63 hops, with A and the erasable bank as the hops '
        'left them: README, Departures, an endless GOTO chain), and yaAGC, looping inside '
        'GOTOERS, does not come back there. And the tool has no case for the watchman\'s restart '
        '(the step reports it as trap 3; the tool treats trap 2, the TC trap\'s). The row fails '
        'the same way with the faithful routines: measured, 173 steps at 2 points with them, 174 '
        'steps at 2 points with the readable ones, all in the loop or at the restart.'),
}
LINE = re.compile(r'(\d+) cs, (\d+) routines and core slots checked \((\d+) yaAGC instructions\), '
                  r'(\d+) points yaAGC did not match')


def engine(cc):
    """yaAGC's engine with its clock frozen, and the two objects it needs."""
    scaled = fhd.OUT / 'agc_engine_scaled.o'
    if not scaled.exists():
        fhd.frozen_engine(cc)
    return [str(scaled), str(fhd.YAAGC / 'agc_engine_init.o'), str(fhd.YAAGC / 'rfopen.o')]


def build(cc, m, mode, extra=(), name='x86-flight-lockstep'):
    """The lockstep tool over the readable objects of mission m, into
    build/x86-flight/lockstep/. extra: more link arguments (--wrong)."""
    define, suffix = xb.MODES[mode]
    OUT.mkdir(parents=True, exist_ok=True)
    flags = xr.step_flags(m, define) + [f'-I{fhd.YAAGC}']
    objects = []
    for source, path in (('flight_dispatch', PORT / 'platform/x86/flight_dispatch.c'), ('lockstep', SOURCE)):
        o = OUT / f'{source}_{m}{suffix}.o'
        subprocess.run([cc, *flags, '-c', str(path), '-o', str(o)], cwd=PORT, check=True)
        objects.append(str(o))
    objects += [str(xb.STORE / 'host' / o) for o in xr.MACHINE] + xr.readable_objects(m) + engine(cc)
    exe = OUT / f'{name}-{m}{suffix}'
    link_map = OUT / f'{exe.name}.map'
    subprocess.run([cc, *objects, *extra, *xb.LINK, '-Wl,--gc-sections', f'-Wl,-Map={link_map}', '-lm', '-o',
                    str(exe)], cwd=PORT, check=True)
    xr.stands_alone(exe, link_map, objects + [a for a in extra if a.endswith('.o')], cc)
    return exe


def fly(exe, m, cs, keys, env):
    """One run of the lockstep: (exit status, its output, seconds)."""
    start = time.time()
    run = subprocess.run([str(exe), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=PORT,
                         capture_output=True, text=True, env={**os.environ, **env})
    return run.returncode, run.stdout + run.stderr, time.time() - start


def row(tag, exe, m, cs, keys, env, trap=False):
    """Fly one row and judge it: (summary row, the line to print)."""
    start = None
    try:
        if 'FLIGHT_CORE' in env:
            env = xs.with_start(m, cs, keys, env)
            start = Path(env['X86_START'])
        status, out, seconds = fly(exe, m, cs, keys, env)
    finally:
        if start is not None:   # the core dump's state is not kept
            start.unlink(missing_ok=True)
    got = LINE.search(out)
    misses = [l.strip() for l in out.splitlines() if l.startswith('  miss ')]
    ok = status == 0 and got is not None and int(got[4]) == 0 and (not trap or 'TC trap at' in out)
    result = dict(row=tag, mission=m, centiseconds=cs, ok=ok, exit=status, seconds=round(seconds),
                  checked=int(got[2]) if got else None, instructions=int(got[3]) if got else None,
                  points_not_matched=int(got[4]) if got else None, misses=misses)
    if got:
        line = (f'{tag}: {int(got[1]):,} cs, {int(got[2]):,} routines and core slots checked against '
                f'Virtual AGC, {got[4]} points differ')
    else:
        line = f'{tag}: no result: ' + ' '.join(out.split())[-200:]
    line = ('ok   ' if ok else 'FAIL ') + line + ''.join(f'\n       {l[:240]}' for l in misses[:6])
    return result, line


def not_modelled_rows():
    """The harness's flight rows gate 2 does not fly, and so neither does
    this check: the x86 program has no model of one of their inputs
    (x86_flight_scenarios.UNSUPPORTED). (row, those inputs) each."""
    return [(row[0], [k for k in xs.UNSUPPORTED if k in row[4]])
            for row in fhd.FLIGHT + fhd.RESTART_FLIGHT + fhd.V69_FLIGHT + fhd.WATCHMAN_FLIGHT
            if any(k in row[4] for k in xs.UNSUPPORTED)]


def recorded(path=RECORD):
    """The run of record: {row: (centiseconds, routines and core slots
    checked, yaAGC instructions)}."""
    rows = {}
    for line in Path(path).read_text().splitlines():
        if line.strip() and not line.startswith('#'):
            name, cs, checked, instructions = line.split()[:4]
            rows[name] = (int(cs), int(checked), int(instructions))
    return rows


def record_problems(summary, record, every_row):
    """Where a run is not the run of record: a row it does not have, a row
    with other counts, and (of a run of every row) a row not flown."""
    problems = []
    for r in summary:
        counts = (r['centiseconds'], r['checked'], r['instructions'])
        if r['row'] not in record:
            problems.append(f'{r["row"]} is not in the record')
        elif counts != record[r['row']]:
            problems.append(f'{r["row"]}: {counts[0]} cs, {counts[1]} routines and core slots, {counts[2]} yaAGC '
                            'instructions, where the record has %d, %d, %d' % record[r['row']])
    if every_row:
        problems += [f'{name} is in the record and was not flown'
                     for name in sorted(set(record) - {r['row'] for r in summary})]
    return problems


def record(path, summary, left_out, twice):
    """The run's rows as a text file that can be committed. left_out: the
    rows of LEFT_OUT as flown, by name; twice: pairs of rows that are one
    run under two names."""
    head = subprocess.run(['git', 'rev-parse', '--short=8', 'HEAD'], cwd=READABLE, capture_output=True,
                          text=True).stdout.strip()
    dirty = subprocess.run(['git', 'status', '--porcelain', '--', str(READABLE), str(PORT / 'tools'),
                            str(PORT / 'platform'), str(PORT / 'src')], cwd=READABLE, capture_output=True,
                           text=True).stdout.strip()
    lines = ['# The readable x86 program against Virtual AGC: the run of record.',
             '# Written by readable/tests/x86_lockstep.py --record (make x86-lockstep-record).',
             f'# Tree: commit {head or "unknown"}' + (', with changes not committed' if dirty else '') + '.',
             f'# Date: {time.strftime("%Y-%m-%d", time.gmtime())}.',
             '# Columns: row, centiseconds flown, routines and core slots checked, yaAGC instructions',
             '# run, points that differ.']
    width = max(len(r['row']) for r in summary)
    for r in summary:
        points = '-' if r['points_not_matched'] is None else r['points_not_matched']
        lines.append(f'{r["row"]:<{width}} {r["centiseconds"]:>7} {r["checked"] or 0:>10} '
                     f'{r["instructions"] or 0:>11} {points:>3}' + ('' if r['ok'] else '  FAILED'))
    lines.append(f'# Total: {len(summary)} rows, {sum(r["checked"] or 0 for r in summary)} routines and core '
                 f'slots, {sum(r["instructions"] or 0 for r in summary)} yaAGC instructions, '
                 f'{sum(r["points_not_matched"] or 0 for r in summary)} points that differ, '
                 f'{sum(not r["ok"] for r in summary)} rows failed.')
    if twice:
        lines.append('# One run under two names (a check row and a scenario, alike in all the x86 program reads), '
                     'counted twice: ' + ', '.join(f'{a} = {b}' for a, b in twice) + '.')
    for name, (points, reason) in LEFT_OUT.items():
        r = left_out.get(name)
        flown = (f' Flown: {r["checked"]} routines and core slots, {r["points_not_matched"]} points that differ.'
                 if r and r['checked'] is not None else '')
        lines.append(f'# Left out: {name}: {reason}{flown}')
    rows = not_modelled_rows()
    lines.append(f'# Not flown, by gate 2 or by this check ({len(rows)} rows of the harness): the x86 program has '
                 'no model of one of their inputs. ' + '; '.join(f'{name} ({", ".join(k)})' for name, k in rows)
                 + '.')
    lines += ['# Not compared with yaAGC by this check, whose clock is frozen and whose interrupt requests are',
              '# cleared at each load: which interrupts are pending, the night watchman, the rupt lock and the',
              '# TC trap (the step\'s own), the HANDRUPT traps\' firing, T6\'s disabling, the timers\' counting',
              '# and the entry to an interrupt. Of channel 10 the last word written is compared, not the word',
              '# of each relay row. Channel 16 is loaded and not compared.']
    Path(path).write_text('\n'.join(lines) + '\n')


# The programs --wrong flies: (the macro tests/x86_lockstep_wrong.c is built
# with, the function wrapped, what is wrong, what the one miss must show,
# whether an erasable word may differ too, whether channel 13 may).
# (WRONG_TRAP's bit is one the check leaves out of channel 13: a check
# that compared more of the channel than it should would show it there.)
WRONG = (
    ('WRONG_TS', 'ts', 'one TS storing its word plus one, once', re.compile(r' 0006 \d{5}/\d{5}'), True, False),
    ('WRONG_CHANNEL', 'wor_channel', 'one WOR leaving bit 5 of channel 13 changed', re.compile(r' ch13 \d{5}/\d{5}'),
     False, True),
    ('WRONG_TRAP', 'wor_channel', 'one WOR leaving bit 13 of channel 13 set, which arms trap 31B',
     re.compile(r' traps 31A 31B 32 \d1\d/\d0\d'), False, False),
)
ERASABLE = re.compile(r' \d{4} \d{5}/\d{5}')
CHANNEL13 = re.compile(r' ch13 ')


def wrong(cc):
    """The check must fail each wrong program, and say where and in what."""
    OUT.mkdir(parents=True, exist_ok=True)
    failures = 0
    for macro, wrapped, what, shows, erasable, channel13 in WRONG:
        o = OUT / f'x86_lockstep_{macro.lower()}.o'
        subprocess.run([cc, '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic', f'-D{macro}', '-c',
                        str(READABLE / 'tests/x86_lockstep_wrong.c'), '-o', str(o)], check=True)
        exe = build(cc, 'lm', 'compiled', extra=[str(o), f'-Wl,--wrap={wrapped}'],
                    name=f'x86-lockstep-{macro.lower().replace("_", "-")}')
        status, out, _ = fly(exe, 'lm', 1000, '', {})
        got = LINE.search(out)
        misses = [l.strip() for l in out.splitlines() if l.startswith('  miss ')]
        ok = (status == 1 and got is not None and int(got[4]) == 1 and len(misses) == 1
              and ': 1 times;' in misses[0] and shows.search(misses[0]) is not None
              and (erasable or not ERASABLE.search(misses[0]))
              and (channel13 or not CHANNEL13.search(misses[0])))
        failures += not ok
        print(f'{"ok  " if ok else "FAIL"} x86-lockstep --wrong: the LM program with {what}: '
              + (f'{got[4]} points differ: {misses[0][:200] if misses else ""}' if got
                 else 'no result: ' + ' '.join(out.split())[-200:]), flush=True)
    return 1 if failures else 0


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--quick', action='store_true', help='the check rows alone')
    p.add_argument('--wrong', action='store_true', help='only the negative: a wrong program must fail')
    p.add_argument('--record', metavar='FILE', help='write the rows of a full run to FILE')
    p.add_argument('--jobs', '-j', type=int, default=os.cpu_count() or 1)
    p.add_argument('names', nargs='*', help='rows (default: all)')
    a = p.parse_args()
    if a.record and (a.quick or a.names or a.wrong):
        sys.exit('x86_lockstep: --record is for a run of every row')
    a.record = a.record and str(Path(a.record).resolve())
    every_row = not a.quick and not a.names
    os.chdir(PORT)
    subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                   cwd=PORT, check=True)
    if a.wrong:
        return wrong(a.host_cc)
    flights = xs.supported()
    unknown = sorted(set(LEFT_OUT) - {r[0] for r in flights})
    if unknown:
        sys.exit(f'x86_lockstep: LEFT_OUT names a row the suite no longer has: {", ".join(unknown)}')
    exe = {(m, mode): build(a.host_cc, m, mode) for m in ('lm', 'cm') for mode in xb.MODES}
    rows = [(f'{m}-{name}-{mode}', exe[m, mode], m, cs, keys, env, name.startswith('v69'))
            for mode in xb.MODES for name, m, cs, keys, env in xc.SCENARIOS]
    rows += [(name, exe[m, 'compiled'], m, cs, keys, env, False) for name, m, cs, keys, env in flights
             if name not in LEFT_OUT and (not a.quick or name in QUICK_FLIGHTS)]
    missing = sorted(set(QUICK_FLIGHTS) - {r[0] for r in rows})
    if missing:
        sys.exit(f'x86_lockstep: QUICK_FLIGHTS names a row the suite no longer has: {", ".join(missing)}')
    # One run under two names: the same program, time, keys and inputs
    # (but an input the x86 program does not read).
    def run(r):
        return r[1:5] + ({k: v for k, v in r[5].items() if k not in HARNESS_ONLY},)
    twice = [(a_[0], b[0]) for i, a_ in enumerate(rows) for b in rows[i + 1:] if run(a_) == run(b)]
    left = [(name, exe[m, 'compiled'], m, cs, keys, env, False) for name, m, cs, keys, env in flights
            if name in LEFT_OUT] if every_row else []
    if a.names:
        rows = [r for r in rows if r[0] in a.names]
        missing = sorted(set(a.names) - {r[0] for r in rows})
        if missing:
            sys.exit(f'x86_lockstep: no such row: {", ".join(missing)}')
    if any('FLIGHT_CORE' in r[5] for r in rows + left):   # the harness gives those rows their start state
        frozen = fhd.frozen_engine(a.host_cc)
        for m in sorted({r[2] for r in rows + left if 'FLIGHT_CORE' in r[5]}):
            fhd.build(a.host_cc, m, frozen)
        xs.OUT = OUT
    summary, failed = [], 0
    with concurrent.futures.ThreadPoolExecutor(max(1, a.jobs)) as pool:
        for result, line in pool.map(lambda r: row(*r), rows):   # in row order
            summary.append(result)
            failed += not result['ok']
            print(line, flush=True)
        left_out = {}
        for result, line in pool.map(lambda r: row(*r), left):
            points, _ = LEFT_OUT[result['row']]
            still = result['exit'] != 0 and result['points_not_matched'] == points
            failed += not still
            left_out[result['row']] = result
            print(f'{"ok  " if still else "FAIL"} left out, {result["row"]}: {result["points_not_matched"]} points '
                  f'differ' + (', as its reason says' if still else f', where LEFT_OUT says {points}: if the row '
                               'now passes, fly it'), flush=True)
    (OUT / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    checked = sum(r['checked'] or 0 for r in summary)
    points = sum(r['points_not_matched'] or 0 for r in summary)
    stale = [] if a.record else record_problems(summary, recorded(), every_row)
    print(f'x86-lockstep: {len(summary)} rows, {checked:,} routines and core slots checked against Virtual AGC, '
          f'{points} points differ, {failed} rows failed'
          + ('' if a.record else f'; {len(stale)} rows not as the run of record (tests/{RECORD.name})'))
    for problem in stale:
        print(f'FAIL the run of record: {problem}')
    if stale:
        print('     If the program or the scenarios changed, fly the record again (make x86-lockstep-record) and '
              'commit it with the reason.')
    if every_row:
        for name, (_, reason) in LEFT_OUT.items():
            print(f'x86-lockstep: left out: {name}: {reason}')
        without = not_modelled_rows()
        print(f'x86-lockstep: not flown, by gate 2 or here ({len(without)} rows the x86 program has no model of an '
              f'input of): {", ".join(name for name, _ in without)}')
    if a.record:
        record(a.record, summary, left_out, twice)
    return 1 if failed or stale else 0


if __name__ == '__main__':
    sys.exit(main())
