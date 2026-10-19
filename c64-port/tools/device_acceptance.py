#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's device_adapter_tests evidence
(docs/PORT_SCOPE.md, integrated_applications).

  device_acceptance.py lm|cm      # runs everything it needs at HEAD
  device_acceptance.py lm|cm [--host SUMMARY.json] [--run RUN.json ...] [--log KEY=PATH ...]

Every input role PORT_SCOPE names (time, IMU/CDU/PIPA, optics, radar,
discretes, DSKY keys, uplink) and every output (jets and engine, CDU and
gyro drives, displays, downlink) gets one row: the AGC interface, the host
adapter and the C64 adapter (each a file and a symbol, checked to exist at
this commit), the host check (flight scenarios that drive the role, from
tools/flight_host_differential.py's summary, "clean" when every native
routine matched yaAGC, erasable memory and output channels) and the C64
check (tools/flight_scenario_vice.py --replay windows whose host inputs
touch the role, "matches" when the C64 ended every checkpoint in the host's
state), plus the unit-level targets. A role with no matching C64 window is
a gap, and the row says what closes it.

With no --host, it runs the host differential; with no --run, the
program's WINDOWS on the C64 in VICE. Writes, all from the same commit:

  docs/acceptance/<Program>-device_adapter_tests.md
  evidence/<m>-acceptance-devices/*.json   (each run and the host summary, tree-stored
                                           with tools/evidence.py add --tree)

Result FAIL (exit 1) when a cited adapter or make target is missing, a
host scenario it cites is not clean, or a role has no matching C64 window;
the doc is still written. A window that differs from the host is an open
item, not a failure by itself (its difference is triaged on the shared
replay mismatch list).
"""
import argparse
import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
# include/resident_rupt.h: RK_RUPT_T6 = 1, T5, T3, T4, KEY1, KEY2, UP, DOWN, RADAR, HAND.
RUPT = {name: 1 << (i + 1) for i, name in enumerate(('T6', 'T5', 'T3', 'T4', 'KEY1', 'KEY2', 'UP', 'DOWN', 'RADAR', 'HAND'))}
FHR = 'tools/flight_host_run.c'
DPC = 'platform/c64/dap_pass_check.c'
RF = 'platform/c64/resident_flight.c'
RRU = 'src/resident_rupt.c'


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def git(*args):
    return subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True).stdout.strip()


def inputs(r):
    return r.get('replay_inputs') or {}


def unit(m, *names):
    """The make targets of routine-level checks: the host differential and
    the C64 (VICE) run of each milestone."""
    pre = f'{m}-' if m else ''
    return [t for n in names for t in (f'{pre}{n}-differential', f'c64-{pre}{n}')]


def make_targets():
    text = '\n'.join(p.read_text() for p in [ROOT / 'Makefile', *sorted((ROOT / 'tools').glob('**/*.mk'))])
    return {t for line in re.findall(r'^([a-z0-9][a-z0-9_. -]*):', text, re.M) for t in line.split()}


def any_window(r):
    return True


def erasable(*addrs):
    return lambda r: bool(set(inputs(r).get('erasable', [])) & {f'{a:04o}' for a in addrs})


def channels(*chs):
    return lambda r: bool(set(inputs(r).get('channels', [])) & {f'{c:o}' for c in chs})


def requests(*names):
    return lambda r: bool(inputs(r).get('requests', 0) & sum(RUPT[n] for n in names))


def pipas(r):
    return bool(inputs(r).get('pipas'))


def either(*preds):
    return lambda r: any(p(r) for p in preds)


def role(key, name, direction, agc, host, c64, env, window, units, gap=None, closes=None, logs=()):
    """host/c64: [(file, symbol regex, what)]; env: FLIGHT_* inputs (or 'keys')
    a host scenario must use to drive the role (None: every scenario);
    window: which replay windows exercise it (outputs: every window compares
    them)."""
    return dict(key=key, name=name, dir=direction, agc=agc, host=host, c64=c64, env=env, window=window,
                units=units, gap=gap, closes=closes, logs=logs)


def roles(m):
    lm = m == 'lm'
    replay_c64 = (DPC, r'static void replay_rows\(', 'replay rows: kind 0/1/2/3 into the image, rk_hardware, rk_device, rk_pending')
    out = [
        role('time', 'Elapsed time', 'in', 'TIME1-TIME6, SCALER1/2 (channels 3, 4), T3RUPT-T6RUPT',
             [(RRU, r'void rk_timer_phase\(', 'the timer model, linked into the host run (FLIGHT_SCALED schedule)')],
             [(RF, r'void rk_rupts_due\(', 'charges each step\'s AGC time and steps the same rk_timer_phase'),
              (RRU, r'void rk_timer_phase\(', 'TIME1-6, scalers, T6 counter, radar gate')],
             None, any_window, ['test-resident-rupt', 'vice-resident-flight'],
             gap='TIME5 a count apart is a temporary replay skip; timer-phase rows on replay-mismatches.md are RK\'s',
             closes='RK\'s FS_PICK and T5 phase fixes; drop TIME5_PHASE from tools/flight_scenario_vice.py'),
        role('imu', 'IMU: CDUs and PIPAs', 'in', 'CDUX-CDUZ (032-034), PIPAX-PIPAZ (037-041), channel 30 IMU bits',
             [(FHR, r'static void engine_due\(' if lm else r'static void sps_due\(', 'engine model: PIPA counts along the thrust'),
              (FHR, r'static void rcs_due\(' if lm else r'static void aero_due\(', 'RCS translation (LM) or aerodynamic drag (CM) into the PIPAs')]
             + ([] if lm else [(FHR, r'static void imu_due\(', 'FLIGHT_IMU: channel 12 zero and coarse align move CDUX-Z'),
                               (FHR, r'static void saturn_due\(', 'FLIGHT_SATURN boost'), (FHR, r'static void sivb_due\(', 'FLIGHT_SIVB TLI burn')]),
             [replay_c64], ['FLIGHT_ENGINE', 'FLIGHT_RCS', 'FLIGHT_POKE'] if lm else ['FLIGHT_SPS', 'FLIGHT_AERO', 'FLIGHT_SATURN', 'FLIGHT_SIVB', 'FLIGHT_IMU'],
             either(pipas, erasable(0o32, 0o33, 0o34)),
             unit(m, 't4rupt-imu'),
             gap='no attitude dynamics: the models hold CDUX-Z at the commanded attitude (CDU drive row)',
             closes='Lane B: rk_cdu_drive on both sides and a rigid-body model fed by the jets and engine'),
        role('optics', 'Optics and marks' if not lm else 'AOT marks', 'in',
             'CDUS/CDUT (035/036, sextant), channel 16 MARK/REJECT, KEYRUPT2' if not lm else 'channel 16 X/Y/REJECT marks, KEYRUPT2',
             [(FHR, r'FLIGHT_LOAD', 'star and sextant loads (tools/flight_scenarios/*.load)'),
              (FHR, r'#define MARK_KEY', 'keys M/J (CM) or x/y/j (LM): channel 16 bits and KEYRUPT2')],
             [replay_c64], ['keys'] if lm else ['FLIGHT_LOAD', 'keys'],
             either(channels(0o16), requests('KEY2'), erasable(0o35, 0o36)),
             unit(m, 'aotmark') if lm else unit(m, 'sxtmark', 't4rupt-optics'),
             gap='no C64 key for MARK or MARK REJECT (resident_dsky.S has no channel 16 path); no mark window replayed',
             closes='a channel 16 key in rk_dsky_keys, and a replayed mark window (' + ('lm-p57 or lm-p52-realign' if lm else 'cm-p52 or cm-p23') + ')'),
        role('radar', 'Rendezvous and landing radar' if lm else 'VHF ranging', 'in',
             'RNRAD (046), channel 13 select bits, channel 33 data good, RADARUPT' if lm else 'RNRAD (046), channel 13 VHF bits, channel 33 bit 2, RADARUPT',
             [('tools/flight_radar.h', r'static void rr_due\(', 'RR (LM) or VHF (CM) model: line of sight, range, data good, FLIGHT_RR_DROP')]
             + ([('tools/flight_lr.h', r'static void lr_due\(', 'LR model: altitude and velocity beams, FLIGHT_LR_DROP, antenna position 2')] if lm else []),
             [(RRU, r'void radar\(', 'radar gate: the read cycle and RADARUPT on the AGC clock'), replay_c64],
             ['FLIGHT_RR', 'FLIGHT_LR', 'FLIGHT_RR_DROP', 'FLIGHT_LR_DROP'] if lm else ['FLIGHT_RR', 'FLIGHT_RR_DROP'],
             either(channels(0o377), erasable(0o46), requests('RADAR')),
             unit(m, 't4rupt-rr', 'radar-leadin-routines', 'p20-p25') if lm else unit(m, 'p20-p25'),
             gap=None, closes=None),
        role('discretes', 'Discretes', 'in', 'channels 30-33 (IMU, abort, mode switches, hand controllers, PROCEED, radar and uplink flip-flops)',
             [(FHR, r'FLIGHT_CHAN=', 'FLIGHT_CH30-33 start values and FLIGHT_CHAN changes at sample points')],
             [replay_c64, ('platform/c64/resident_dsky.c', r'PRO', 'PROCEED key: channel 32 bit 14')],
             ['FLIGHT_CHAN', 'FLIGHT_CH30', 'FLIGHT_CH31', 'FLIGHT_CH32', 'FLIGHT_CH33', 'FLIGHT_STAGE'],
             channels(0o30, 0o31, 0o32, 0o33),
             unit(m, 't4rupt-imu', 'rcsmon') if lm else unit(m, 't4rupt-imu', 'rcs-dap'),
             gap='IMU fail, no-att and cage are covered at routine level only (random channel 30 vectors); off-nominal scenarios are D1b\'s',
             closes='D1b\'s IMU fail and no-att scenarios and their replay windows'),
        role('keys', 'DSKY keys', 'in', 'channel 15 and KEYRUPT1, channel 32 bit 14 (PROCEED)',
             [(FHR, r'KEYS', 'the scenario\'s key string, pressed at its times')],
             [('platform/c64/resident_dsky.S', r'rk_dsky_keys:', 'C64 keyboard scan: channel 15 and KEYRUPT1'),
              (DPC, r'press_due', 'scripted keys in a replay window (--keys)')],
             ['keys'], either(channels(0o15), requests('KEY1')),
             unit(None, 'keyrupt-uprupt') + ['flight-disk-vice'],
             gap='the DSKY noun x verb display sweep on the C64 is Lane C\'s (Pinball)',
             closes='Lane C\'s DSKY sweep check'),
        role('uplink', 'Uplink', 'in', 'INLINK (045), UPRUPT, channel 11 bit 3 (UPLINK ACTY), channel 33 bits 10-11',
             [(FHR, r'static int uplink_due\(', 'FLIGHT_UPLINK: triple-redundant INLINK words and UPRUPT, 20 cs apart')],
             [replay_c64], ['FLIGHT_UPLINK'], either(erasable(0o45), requests('UP')),
             unit(None, 'keyrupt-uprupt') + unit(m, 'update'),
             gap=None if lm else 'no CM uplink scenario: on cm-p40 with FLIGHT_UPLINK the C64 matches the host until Pinball\'s V16N36 stop (Lane C)',
             closes=None if lm else 'Lane C\'s V16N36 fix, then a CM uplink scenario and window'),
        role('jets', 'RCS jets and engine' if lm else 'RCS jets, SPS and TVC', 'out',
             'channels 5 and 6 (jets), channel 11 engine on/off, THRUST (055)' if lm else 'channels 5 and 6 (jets), channel 11 SPS on, TVC commands (CDUSCMD/CDUTCMD)',
             [(FHR, r'static void rp_end\(', 'host end state: channels 5, 6, 11-14, 34, 35, superbank and the erasable words')],
             [(RF, r'rk_hardware', 'latched in rk_hardware (and rk_device on the CM), compared at every checkpoint')],
             ['FLIGHT_ENGINE', 'FLIGHT_RCS'] if lm else ['FLIGHT_SPS'], any_window,
             unit(m, 't6rupt', 'tjet-law', 'q-r-axis') if lm else unit(m, 'jet-select', 'rcs-dap', 'tvc-roll'),
             gap='outputs are compared, but drive no attitude dynamics (perfect attitude models)',
             closes='Lane B\'s rigid-body model (IMU row)'),
        role('drives', 'CDU and gyro drives', 'out', 'channel 14, CDUXCMD-CDUZCMD (050-052), GYROCMD (047)' + ('' if lm else ', CDUSCMD/CDUTCMD (TVC)'),
             [(FHR, r'static void rp_end\(', 'compared as channel 14 and erasable words only')],
             [('src/resident_cdu.c', r'rk_cdu_drive', 'the drive model exists but is not linked into the flight resident')],
             None, any_window, ['test-resident-cdu'],
             gap='the drive counters never drain on either side in flight runs (yaAGC\'s drives are frozen under scaled time)',
             closes='Lane B: link rk_cdu_drive and the gyro drive on both sides; then the IMU row gets real CDU motion'),
        role('displays', 'Displays and lamps', 'out', 'DSPTAB (relay words), channel 10, channel 11 lamps',
             [(FHR, r'FLIGHT_DSKY', 'prints DSPTAB as the DSKY shows it')],
             [('platform/c64/resident_dsky.c', r'DSPTAB', 'the C64 screen draws DSPTAB and channel 11')],
             None, any_window, unit(None, 't4rupt-core') + ['flight-disk-vice'],
             gap='channel 10 strobes are not compared (DSPTAB is); the on-screen sweep is Lane C\'s',
             closes='Lane C\'s DSKY sweep check'),
        role('downlink', 'Downlink', 'out', 'channels 34 and 35, DOWNRUPT, channel 13 bit 7',
             [('tools/flight_downlink.h', r'FLIGHT_DOWNRUPT', 'DOWNRUPT every 20 ms of AGC time, host only')],
             [('platform/c64/downlink_check.c', r'rk_pending', 'raises DOWNRUPT on the C64 and stashes channels 34/35')],
             ['FLIGHT_DOWNRUPT'], requests('DOWN'), ['flight-downlink-check', 'flight-downlink-vice'],
             gap='DOWNRUPT is off in the flight scenarios, so the replay windows never carry it' + ('' if lm else '; the VICE downlink windows are LM only'),
             closes='flight-downlink-vice evidence attached here (--log downlink=...)' + ('' if lm else ' and a CM window in tools/flight_downlink_check.py')),
    ]
    return out


def adapter_ok(path, pattern):
    f = ROOT / path
    return f.exists() and re.search(pattern, f.read_text(errors='replace')) is not None


def host_rows(m, summary, env):
    """[(scenario, clean, detail)] for the scaled-x1 flight scenarios of m that use env."""
    sys.path.insert(0, str(ROOT / 'tools'))
    import flight_host_differential as fhd  # noqa: E402
    by = {r['scenario']: r for r in summary}
    rows = []
    for name, mm, cs, keys, inp in fhd.FLIGHT:
        if mm != m:
            continue
        if env is not None and not (set(inp) & set(env) or ('keys' in env and keys)):
            continue
        r = by.get(f'{name}-scaled-x1')
        if r is None:
            rows.append((name, None, 'not in the summary'))
            continue
        clean = r['exit'] == 0 and not r['routine_mismatches']
        rows.append((name, clean, f"{r.get('native_routines')} routines, {len(r['routine_mismatches'])} yaAGC did not match"))
    return rows


def first_diff(r):
    """Where a differing window first left the host, in words."""
    f = r.get('replay_first') or []
    at = r.get('replay_differ_at_cs')
    w = ', '.join(f"{d['at']} host {d['host']} C64 {d['c64']}" for d in f[:2])
    return f"{r.get('replay_differences')} words by {at} cs" + (f' ({w})' if w else '')


def write_doc(m, table, runs, logs, host_src, commit):
    p = PROGRAM[m]
    bad = []
    lines = [f'# {p}: device adapter tests', '',
             'HEADER', '',
             'This is the `device_adapter_tests` evidence of the '
             f'`integrated_applications.{p}` entry (docs/PORT_SCOPE.md): each input role the program reads and each '
             'output it drives, the adapter that carries it on the host and on the C64, and the checks that '
             'exercise it. The host check is the whole-program host differential (tools/flight_host_differential.py, '
             'scaled time x1): a scenario is clean when every native routine left yaAGC\'s erasable memory and output '
             'channels (5, 6, 10-14). The C64 check is a '
             '`tools/flight_scenario_vice.py --replay` window on stock C64 + 2 MB REU in VICE: the C64 starts from '
             'the host\'s snapshot, takes the host\'s inputs as rows, and must end every checkpoint in the host\'s '
             'state (erasable words, TIME1-6, output channels 5, 6, 11-14, 34, 35 and the superbank; '
             'docs/FLIGHT-STORE.md "matches host"). A window exercises an input role when its rows touch the role\'s '
             'words, channels or interrupt requests; outputs are compared in every window.', '',
             '## Summary', '', '| Role | In/out | Adapters | Host check | C64 check | Status |', '|---|---|---|---|---|---|']
    detail = []
    for t in table:
        ads = [(side, f, s, w, adapter_ok(f, s)) for side in ('host', 'c64') for f, s, w in t[side]]
        missing = [a for a in ads if not a[4]]
        bad += [f"{t['key']}: {a[1]} has no /{a[2]}/" for a in missing]
        hrows = host_rows(m, host_src, t['env']) if host_src is not None else []
        bad += [f"{t['key']}: host scenario {n} not clean ({d})" for n, c, d in hrows if c is False]
        wins = [r for r in runs if t['window'](r)]
        matched = [r for r in wins if r['_match']]
        host_ok = any(c for _, c, _ in hrows)
        status = ('covered' if matched and (host_ok or t['env'] is None) and not t['gap']
                  else 'covered, with open items' if matched else 'GAP')
        if status == 'GAP':
            bad.append(f"{t['key']}: no C64 window that exercises it matches the host")
        lines.append(f"| {t['name']} | {t['dir']} | {'all present' if not missing else 'MISSING ' + str(len(missing))} | "
                     f"{sum(1 for _, c, _ in hrows if c)} of {len(hrows)} scenarios clean | "
                     f"{len(matched)} of {len(wins)} windows match | {status} |")
        d = [f"### {t['name']} ({'input' if t['dir'] == 'in' else 'output'})", '', f"AGC interface: {t['agc']}.", '',
             '| Side | Adapter | Symbol | What it does | Present |', '|---|---|---|---|---|']
        for side, f, s, w, ok in ads:
            d.append(f"| {side} | `{f}` | `{s}` | {w} | {'yes' if ok else 'NO'} |")
        d += ['', 'Host scenarios that drive it' + (' (every scenario)' if t['env'] is None else
                                                  f" ({', '.join(t['env'])})") + ':', '']
        d += [f"- {n}: {'clean' if c else 'not run' if c is None else 'NOT CLEAN'} ({x})" for n, c, x in hrows] or ['- none']
        d += ['', 'C64 replay windows that ' + ('exercise it' if t['dir'] == 'in' else 'compare it') + ':', '']
        d += [f"- {r['scenario']} {r['snap_cs']}+{r.get('requested_cs')}: "
              f"{'matches the host at ' + ', '.join(map(str, r.get('replay_matched_cs') or [])) + ' cs' if r['_match'] else 'differs: ' + first_diff(r)}"
              f"{' (skips: ' + '; '.join(r['replay_skipped']) + ')' if r.get('replay_skipped') else ''}; "
              f"inputs {json.dumps(inputs(r).get('counts', {}))} (`{r['_evidence']}`)" for r in wins] or ['- none']
        targets = make_targets()
        bad += [f"{t['key']}: no make target {u}" for u in t['units'] if u not in targets]
        d += ['', 'Routine-level checks (host differential against yaAGC, then the same cases on the C64 in VICE): '
              + ', '.join(f'`make {u}`' for u in t['units']) + '.']
        for k in t['logs']:
            if k in logs:
                d.append(f"Log: `{logs[k]}`.")
        if t['gap'] or not matched:
            d += ['', f"**Open:** {t['gap'] or 'no C64 window exercises this role yet'}."
                  + (f" Closes it: {t['closes']}." if t['closes'] else '')]
        detail += d + ['']
    lines += ['', 'Status: covered = adapters present, the host check clean, and at least one C64 window that '
              'exercises the role matches the host; "with open items" lists what the row still lacks; GAP = no '
              'matching C64 window exercises it yet.', '', '## Roles', ''] + detail
    lines += ['## Runs', '', 'A window that differs is not a device failure by itself: each difference is triaged on the shared '
              'replay mismatch list (clock and charge classes, DAP-rate words), and only the device or channel class '
              'is this doc\'s. A role counts as covered only through windows that match.', '',
              '| Scenario | Start cs | cs | Every cs | Result | Evidence |', '|---|---|---|---|---|---|']
    for r in runs:
        lines.append(f"| {r['scenario']} | {r['snap_cs']} | {r.get('requested_cs')} | {r.get('replay_every', '')} | "
                     f"{'matches' if r['_match'] else 'differs: ' + first_diff(r)} | `{r['_evidence']}` (sha256 `{r['_sha'][:16]}...`) |")
    lines += ['', '## Reproduce', '', '```sh', f'python3 tools/device_acceptance.py {m}   # host differential + the windows above',
              '# one window: python3 tools/flight_scenario_vice.py --scenario <scenario> --snap-cs <cs> --cs <n> --replay --replay-every <every>',
              '```', '']
    lines[lines.index('HEADER')] = (f'Generated by `tools/device_acceptance.py {m}` at commit `{commit}`. '
                                    f"Result: **{'FAIL' if bad else 'PASS'}**")
    path = ROOT / 'docs/acceptance' / f'{p}-device_adapter_tests.md'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines))
    return path, bad


# The C64 replay windows a bare run takes (scenario, snapshot cs, window cs,
# checkpoint every cs), chosen so each input role has a window whose rows
# touch it. The longer windows of the same phases drift by a clock tick;
# those rows are on the shared replay mismatch list, not here.
WINDOWS = {
    'lm': [('lm-p63-descent-thrust', 17500, 120, 120),   # PIPAs under descent thrust
           ('lm-landing-ascent-nassp', 140000, 150, 50),   # touchdown, radar gate, engine arm
           ('lm-uplink-reject', 1950, 100, 100),            # UPRUPT rows, KEYRUPT
           ('lm-p52-realign', 4200, 500, 500),              # optics marks, channel 30
           ('lm-p57', 15950, 100, 100),                     # lunar-surface alignment marks
           ('lm-p63-lr-update', 61000, 50, 50)],            # landing radar reads
    'cm': [('cm-p52', 9950, 300, 100),                      # optics marks, ch30/ch33
           ('cm-tei', 4450, 300, 100),                      # RCS DAP, channels and requests
           ('cm-tei', 52000, 300, 50),                      # RCS DAP and TIME6
           ('cm-p40', 70660, 100, 50)],                     # SPS thrust PIPAs
}


def run_window(scenario, snap, cs, every, out):
    """One flight_scenario_vice.py --replay window; its result JSON (the run
    writes it even when the C64 leaves the host's state)."""
    res = ROOT / 'build/flight-store/flight-scenario-vice.json'
    res.unlink(missing_ok=True)
    r = subprocess.run([sys.executable, 'tools/flight_scenario_vice.py', '--scenario', scenario, '--snap-cs', str(snap),
                        '--cs', str(cs), '--replay', '--replay-every', str(every)], cwd=ROOT, capture_output=True, text=True)
    log = out / f'{scenario}-{snap}.log'
    log.write_text(r.stdout + r.stderr)
    result = out / f'{scenario}-{snap}.json'
    try:
        d = json.loads(res.read_text())
    except (OSError, ValueError):
        d = {}
    if d.get('scenario') != scenario or d.get('snap_cs') != snap:
        d = dict(scenario=scenario, snap_cs=snap, requested_cs=cs, error=(r.stdout + r.stderr).strip().splitlines()[-1:])
    d['replay_every'] = every
    result.write_text(json.dumps(d, indent=1) + '\n')
    print(f"{scenario} {snap}+{cs}: {next((x for x in reversed(r.stdout.splitlines()) if 'matches the host' in x or 'FAIL' in x), 'no result')}")
    return result


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', choices=sorted(PROGRAM))
    ap.add_argument('--host', help="tools/flight_host_differential.py's summary.json")
    ap.add_argument('--run', action='append', default=[], help='a flight_scenario_vice.py --replay result JSON')
    ap.add_argument('--log', action='append', default=[], help='KEY=PATH, a check log for a role (copied)')
    a = ap.parse_args(argv)
    commit = git('rev-parse', '--short=12', 'HEAD') + (' (dirty)' if git('status', '--porcelain', '--', 'src', 'include', 'platform', 'tools') else '')
    eid = f'{a.mission}-acceptance-devices'
    dest = ROOT / 'build' / eid  # staged here, then stored flat by tools/evidence.py add --tree
    shutil.rmtree(dest, ignore_errors=True)
    dest.mkdir(parents=True)
    copies = []

    def stored(copy):
        copies.append(copy)
        return f'evidence/{eid}/{copy.name}'
    if not a.host:
        subprocess.run([sys.executable, 'tools/flight_host_differential.py'], cwd=ROOT, check=False)
        a.host = str(ROOT / 'build/flight-store/differential/summary.json')
    if not a.run:
        out = ROOT / 'build' / f'{eid}-runs'
        shutil.rmtree(out, ignore_errors=True)
        out.mkdir(parents=True)
        a.run = [str(run_window(*w, out)) for w in WINDOWS[a.mission]]
    host = None
    if a.host:
        host = json.loads(pathlib.Path(a.host).read_text())
        copy = dest / 'host-summary.json'
        copy.write_text(json.dumps(host, indent=1) + '\n')
        stored(copy)
    runs = []
    for path in a.run:
        r = json.loads(pathlib.Path(path).read_text())
        if r.get('mission', a.mission) != a.mission or r['scenario'].split('-')[0] != a.mission:
            continue
        copy = dest / f"{r['scenario']}-{r.get('snap_cs', 'x')}.json"
        copy.write_text(json.dumps(r, indent=1) + '\n')
        r['_evidence'], r['_sha'] = stored(copy), sha(copy)
        r['_match'] = bool(r.get('replay_ended')) and r.get('replay_differences') == 0 and r.get('replay_applied') == r.get('replay_rows')
        runs.append(r)
    logs = {}
    for kv in a.log:
        k, _, p = kv.partition('=')
        copy = dest / f'{k}-{pathlib.Path(p).name}'
        copy.write_bytes(pathlib.Path(p).read_bytes())
        logs[k] = stored(copy)
    if copies:
        shutil.rmtree(ROOT / 'evidence' / eid, ignore_errors=True)  # this generator's alone: no stale runs
        # evidence/<id>/ is flat, with its SHA256.json (tools/ci_guard.py).
        subprocess.run([sys.executable, 'tools/evidence.py', 'add', eid, *map(str, copies), '--tree'],
                       cwd=ROOT, check=True)
    path, bad = write_doc(a.mission, roles(a.mission), runs, logs, host, commit)
    print(f"{path.relative_to(ROOT)}: {'PASS' if not bad else 'FAIL'} (sha256 {sha(path)})")
    for b in bad:
        print(f'FAIL: {b}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
