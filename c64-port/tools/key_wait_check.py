#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Checks the harness's display-driven keys (! in the keys string,
tools/flight_host_run.c keys_parse) change nothing for a scenario without one.

Builds tools/flight_host_run.c as it is and as it was at --base (default: the
merge base with origin/main), runs every host-differential
scenario on both (SCENARIOS unscaled; SCALED, FLIGHT, RESTART_FLIGHT and
V69_FLIGHT on their clocks and on the work clock), and a C64 replay window
from three of them (FLIGHT_SNAPSHOT + FLIGHT_REPLAY), and requires the same
output byte for byte: stdout, stderr, exit code and the replay files.

Then it checks the ! does something: lm-stable-orbit with its V33s as !V33
must give other output than without the !s (and still exit 0).

Needs the flight store (tools/flight_store_build.py --host-only) and git
history for --base. Not in CI (about 15 minutes on 4 cores).
"""
import argparse, os, re, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402

OUT = ROOT / 'build/key-wait-check'
REPLAY = ('lm-p63-descent', 'cm-p40', 'lm-rendezvous')
REPLAY_CS = 300


def build(cc, m, src, tag, engine):
    """flight-host-run built from src (a path): OUT/flight-host-run-<m>-<tag>."""
    obj, exe = OUT / f'flight_host_run_{m}_{tag}.o', OUT / f'flight-host-run-{m}-{tag}'
    store = ROOT / 'build/flight-store'
    subprocess.run([cc, '-std=c99', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', '-Itools',
                    f'-I{fhd.YAAGC}', f'-DRK_{m.upper()}=1', f'-DFLIGHT_HOST_TABLES="{store / f"{m}-host.inc"}"',
                    f'-DFLIGHT_HOST_SITES="{store / f"{m}-sites.inc"}"', '-c', str(src), '-o', str(obj)],
                   cwd=ROOT, check=True)
    objs = sorted(str(p) for p in (fhd.OUT / 'host').glob('*.o'))
    subprocess.run([cc, str(obj), *objs, str(engine), str(fhd.YAAGC / 'agc_engine_init.o'), str(fhd.YAAGC / 'rfopen.o'),
                    '-Wl,--gc-sections', '-lm', '-o', str(exe)], cwd=ROOT, check=True)
    return exe


def runs():
    """(name, mission, cs, keys, env) for every scenario the check runs."""
    out = [(f'{m}-{keys or "p00"}', m, cs, keys, {}) for m, cs, keys in fhd.SCENARIOS]
    out += [(f'{m}-{keys}-{tag}', m, cs, keys, {**env, 'FLIGHT_SCALED': sched})
            for m, cs, keys, env, sched, tag in fhd.SCALED]
    for name, m, cs, keys, env in fhd.FLIGHT + fhd.RESTART_FLIGHT + fhd.V69_FLIGHT:
        out += [(f'{name}-x1', m, cs, keys, {**env, 'FLIGHT_SCALED': '1,1,2'}),
                (f'{name}-work', m, cs, keys, {**env, 'FLIGHT_SCALED': 'W'})]
    for name in REPLAY:
        _, m, cs, keys, env = fhd.scenario(name)
        out.append((f'{name}-replay', m, cs, keys, {**env, 'FLIGHT_SCALED': 'W', 'FLIGHT_REPLAY': str(REPLAY_CS)}))
    return out


def run(exe, name, m, cs, keys, env, tmp):
    """stdout, stderr, exit code and (for a replay) the files it wrote."""
    env = dict(env)
    snap = Path(tmp) / f'{name}.h'
    if 'FLIGHT_REPLAY' in env:
        env['FLIGHT_SNAPSHOT'] = f'{cs // 2},{snap}'
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                       env={**os.environ, **env}, capture_output=True)
    files = {p.name: p.read_bytes() for p in sorted(Path(tmp).glob(f'{name}.h*'))}
    return r.stdout, r.stderr, r.returncode, files


def compare(exes, row):
    name, m = row[0], row[1]
    with tempfile.TemporaryDirectory() as a, tempfile.TemporaryDirectory() as b:
        got = [run(exes[tag][m], *row, tmp) for tag, tmp in (('base', a), ('head', b))]
    return name, got[0] == got[1], got[1][2]


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--base', help='git ref of the harness to compare with (default: merge base with '
                                  'origin/main)')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    a = p.parse_args()
    base = a.base or subprocess.run(['git', 'merge-base', 'HEAD', 'origin/main'], cwd=ROOT,
                                    capture_output=True, text=True, check=True).stdout.strip()
    OUT.mkdir(parents=True, exist_ok=True)
    src = OUT / 'flight_host_run_base.c'
    src.write_bytes(subprocess.run(['git', 'show', f'{base}:c64-port/tools/flight_host_run.c'], cwd=ROOT,
                                   capture_output=True, check=True).stdout)
    engine = fhd.frozen_engine(a.host_cc)
    exes = {tag: {m: build(a.host_cc, m, s, tag, engine) for m in ('lm', 'cm')}
            for tag, s in (('base', src), ('head', ROOT / 'tools/flight_host_run.c'))}
    rows = runs()
    failed = 0
    with ThreadPoolExecutor(a.jobs) as pool:
        for name, same, code in pool.map(lambda r: compare(exes, r), rows):
            failed += not same
            print(f'{name}: ' + ('same output' if same else 'OUTPUT DIFFERS') + (f' (exit {code})' if code else ''),
                  flush=True)
    print(f'{len(rows) - failed} of {len(rows)} runs give the same output as {base[:12]}')
    # The ! itself: the same run with its V33s waiting for the display must
    # differ (the harness presses them later).
    name, m, cs, keys, env = fhd.scenario('lm-stable-orbit')
    env = {**env, 'FLIGHT_SCALED': '1,1,2'}
    with tempfile.TemporaryDirectory() as tmp:
        plain = run(exes['head'][m], name, m, cs, keys, env, tmp)
        wait = run(exes['head'][m], name, m, cs, keys.replace('V33E', '!V33E'), env, tmp)
    waited = plain[0] != wait[0] and wait[2] == 0
    print(f"{name} with !V33E: {'its output changes' if waited else 'NO CHANGE (the ! did nothing)'}")
    return 1 if failed or not waited else 0


if __name__ == '__main__':
    sys.exit(main())
