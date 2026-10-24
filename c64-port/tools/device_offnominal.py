#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The "Off-nominal inputs" section of tools/device_acceptance.py
(device_adapter_tests, acceptance item 8).

Each row is a nominal flight scenario (tools/flight_host_differential.py,
FLIGHT) with one device input gone wrong: a radar's data good lost, a
garbled uplink word, IMU FAIL, the IMU caged, the middle gimbal near lock,
an Executive overload. On the host every native routine must match yaAGC
with nothing bridged, and the AGC must answer the bad input as the flight
software says it should: an alarm code in FAILREG, a flag or lamp bit, a
count, or a display. The checks read the words the host run prints with
FLIGHT_WATCH (each named by its symbol, looked up in the program's assembly
listing) and the DSKY it prints with FLIGHT_DSKY. Each row also names a C64
window (tools/flight_scenario_vice.py --replay) over the bad input.
"""
import os
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402

PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
STORE = ROOT / 'build/flight-store'


def alarm(code, t0, t1):
    """code: one alarm, or a tuple of alarms any of which answers."""
    codes = code if isinstance(code, tuple) else (code,)
    return ('alarm', codes, t0, t1, f"alarm {' or '.join(f'{c:05o}' for c in codes)} in FAILREG between {t0} and {t1} cs")


def bit(sym, mask, on, t0, t1, what):
    return ('word', sym, mask, mask if on else 0, t0, t1, what)


def dsky(pattern, t0, t1, what):
    return ('dsky', pattern, t0, t1, what)


def final(sym, value, what):
    return ('final', sym, 0o77777, value, what)


def quiet(sym, t0, t1, what):
    return ('quiet', sym, t0, t1, what)


# DSPTAB+11D carries the lamp relay bits: bit 4 NO ATT, bit 6 GIMBAL LOCK.
NO_ATT, GIMBAL_LOCK = 0o10, 0o40
# Per row: scenario, owner, the input gone wrong, the AGC's answer, its
# checks, and the C64 window (snap cs, run cs). The scenarios and their
# inputs are in tools/flight_host_differential.py (FLIGHT).
ROWS = {
    'lm': [
        dict(scenario='lm-rendezvous-rr-dropout', owner='D2',
             failure='RR data good lost from 180 s to 210 s, over R22\'s third mark (`FLIGHT_RR_DROP`)',
             response='The range read fails (alarm 521, RADMODES data fail); R22 drops the lock and R21 flashes '
                      'V50N72 again; after PRO the marks go on and P34 solves TPI as before',
             checks=[alarm(0o521, 18000, 21000),
                     dsky(r'P20 V50 N72', 21000, 21500, 'R21 flashes V50N72 again before the PRO at 215 s')],
             window=(19900, 600)),
        dict(scenario='lm-p63-lr-dropout', owner='F',
             failure='Landing radar data good lost from 880 s to 950 s in P63 braking (`FLIGHT_LR_DROP`)',
             response='R12\'s range reads fail during the dropout and pass again after it (no V57, so nothing '
                      'is incorporated)',
             checks=[bit('RADMODES', 0o20, True, 88000, 88400, 'R12 marks the range read failed (RADMODES bit 5)'),
                     bit('RADMODES', 0o20, False, 95000, 95400, 'and good again after the dropout')],
             window=(87900, 600)),
        dict(scenario='lm-uplink-reject', owner='D2',
             failure='Uplink: a V71 block of 25 components (over the limit of 20), then a garbled word at 50 s '
                     '(`FLIGHT_UPLINK`)',
             response='P27 rejects the block (V21N01 again); UPRUPT\'s redundancy test locks the uplink out '
                      '(FLAGWRD7 bit 4) until ERROR RESET at 65 s; V37E00E then takes P00',
             checks=[dsky(r'P27 V21 N01 R1 +R2', 2100, 2200, 'P27 asks for the first component again'),
                     bit('FLAGWRD7', 0o10, True, 5000, 5100, 'the garbled word sets the lockout (FLAGWRD7 bit 4)'),
                     bit('FLAGWRD7', 0o10, False, 6500, 6600, 'ERROR RESET clears it'),
                     dsky(r'^P00 ', 7100, 9000, 'P00 after V37E00E')],
             window=(1900, 800)),
        dict(scenario='lm-p52-imu-fail', owner='D1b',
             failure='IMU FAIL (channel 30 bit 13) from 30 s to 45 s, over P52\'s first star (`FLIGHT_CHAN`)',
             response='IMUMON records it in IMODES30 and stores alarm 7777; P52 goes on and ends in V37',
             checks=[alarm(0o7777, 3000, 3100),
                     bit('IMODES30', 0o10000, False, 3000, 3100, 'IMODES30 bit 13 records the failure'),
                     bit('IMODES30', 0o10000, True, 4500, 4600, 'and clears when it ends')],
             window=(2900, 700)),
        dict(scenario='lm-p52-imu-cage', owner='D1b',
             failure='IMU caged (channel 30 bit 11) from 30 s to 35 s (`FLIGHT_CHAN`)',
             response='IMUCAGE zeroes the CDUs and lights NO ATT; the marks are made against the caged platform '
                      'and P52 still ends in V37',
             checks=[bit('DSPTAB', NO_ATT, True, 3000, 3100, 'NO ATT lights (DSPTAB+11D bit 4)'),
                     bit('DSPTAB', NO_ATT, False, 3500, 3600, 'and goes out after the cage')],
             window=(2900, 700)),
        dict(scenario='lm-p63-overload-nomon', owner='RK',
             failure='420 MCT a centisecond stolen from 185 s to 240 s in P63 (`FLIGHT_STEAL`), as the RR CDU '
                     'counts did on Apollo 11 (without lm-p63-overload\'s V16N68 monitor, whose PASTEVB the host '
                     'still bridges)',
             response='SERVICER falls behind: the Executive runs out of core sets or VAC areas (alarm 1201 or '
                      '1202, which one depends on the job mix), each a BAILOUT and a restart through the '
                      'phase tables; P63 flies on when the steal ends',
             checks=[alarm((0o1201, 0o1202), 18500, 24000)],
             window=(19000, 600)),
    ],
    'cm': [
        dict(scenario='cm-p20-vhf-dropout', owner='D2',
             failure='VHF ranging data good lost from 180 s to 190 s, over R22\'s second range (`FLIGHT_RR_DROP`)',
             response='VHFREAD takes BADEND: no N49 and no mark for that range; the next ranges incorporate on '
                      'their own (VHFCNT 3 by 320 s, one fewer than cm-p20)',
             checks=[quiet('VHFCNT', 18000, 25000, 'no VHF mark from 180 s to 250 s (cm-p20 marks its second '
                                                         'range at 204 s)'),
                     final('VHFCNT', 3, 'the next ranges mark on their own: VHFCNT 3 by 320 s')],
             window=(17900, 600)),
        dict(scenario='cm-uplink-reject', owner='D2',
             failure='Uplink: a V71 block of 25 components (over the limit of 20), then a garbled word at 50 s '
                     '(`FLIGHT_UPLINK`)',
             response='P27 rejects the block (V21N01 again); UPRUPT locks the uplink out (FLAGWRD7 bit 4) until '
                      'ERROR RESET at 65 s; V37E00E then takes P00',
             checks=[dsky(r'P27 V21 N01 R1 +R2', 2100, 2200, 'P27 asks for the first component again'),
                     bit('FLAGWRD7', 0o10, True, 5000, 5100, 'the garbled word sets the lockout (FLAGWRD7 bit 4)'),
                     bit('FLAGWRD7', 0o10, False, 6500, 6600, 'ERROR RESET clears it'),
                     dsky(r'^P00 ', 7100, 9000, 'P00 after V37E00E')],
             window=(1900, 800)),
        dict(scenario='cm-p52-imu-fail', owner='D1b',
             failure='IMU FAIL from 60 s to 70 s, on V04N06 and V50N25 (`FLIGHT_CHAN`)',
             response='IMUMON records it in IMODES30 and stores alarm 7777; P52 goes on and ends in P00',
             checks=[alarm(0o7777, 6000, 6100),
                     bit('IMODES30', 0o10000, False, 6000, 6100, 'IMODES30 bit 13 records the failure'),
                     bit('IMODES30', 0o10000, True, 7000, 7100, 'and clears when it ends')],
             window=(5900, 600)),
        dict(scenario='cm-p52-imu-cage', owner='D1b',
             failure='IMU caged from 60 s to 65 s, on V04N06 (`FLIGHT_CHAN`)',
             response='NO ATT lights; the marks are made against the caged platform and P52 ends in P00',
             checks=[bit('DSPTAB', NO_ATT, True, 6000, 6100, 'NO ATT lights (DSPTAB+11D bit 4)'),
                     bit('DSPTAB', NO_ATT, False, 6500, 6600, 'and goes out after the cage')],
             window=(5900, 700)),
        dict(scenario='cm-p52-gimbal-lock', owner='D1b',
             failure='Middle gimbal (CDUZ) at 72 degrees from 30 s to 36 s, then 86 degrees at 190 s '
                     '(`FLIGHT_POKE`)',
             response='GLOCKMON lights GIMBAL LOCK and puts it out when the angle comes back; past 85 degrees it '
                      'coarse aligns the platform and NO ATT lights with GIMBAL LOCK',
             checks=[bit('DSPTAB', GIMBAL_LOCK, True, 3000, 3100, 'GIMBAL LOCK lights (DSPTAB+11D bit 6)'),
                     bit('DSPTAB', GIMBAL_LOCK, False, 3600, 3700, 'and goes out at 36 s'),
                     bit('DSPTAB', GIMBAL_LOCK | NO_ATT, True, 19000, 19100,
                         'past 85 degrees: GIMBAL LOCK and NO ATT (coarse align)')],
             window=(2900, 800)),
        dict(scenario='cm-entry-overload', owner='RK',
             failure='600 MCT a centisecond stolen from 210 s to 270 s, over P64 and P67 (`FLIGHT_STEAL`)',
             response='Alarm 1201 and restarts through the phase tables; the entry still reaches P67',
             checks=[alarm(0o1201, 21000, 27000)],
             window=(20900, 600)),
    ],
}


def symbols(m):
    """Unswitched erasable symbols from the program's assembly listing."""
    out = {}
    lst = ROOT / 'build/fixed-data' / PROGRAM[m] / 'assembly.lst'
    for line in lst.read_text(errors='replace').splitlines():
        x = re.match(r'^\d+,\d+:\s+(?:\S+\s+)?([0-7]{4})\s+(\w+)\s+(?:ERASE|EQUALS|=)(?:\s|$)', line)
        if x and x[2] not in out:
            out[x[2]] = int(x[1], 8)
    return out


def address(sym, table):
    """DSPTAB means DSPTAB+11D, the lamp word."""
    return table[sym] + (11 if sym == 'DSPTAB' else 0)


def host_run(m, row, table, logs):
    _, mission, cs, keys, env = fhd.scenario(row['scenario'])
    assert mission == m
    watched = sorted({address(c[1], table) for c in row['checks'] if c[0] in ('word', 'final', 'quiet')}
                     | {table['FAILREG'] + k for k in range(3)})
    r = subprocess.run([str(STORE / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []),
                       cwd=ROOT, capture_output=True, text=True,
                       env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_DSKY': '1',
                            'FLIGHT_WATCH': ','.join(f'{a:o}' for a in watched)})
    log = logs / f"{row['scenario']}.log"
    log.write_text(r.stdout + r.stderr)
    text = r.stdout
    misses = re.search(r'native routines yaAGC did not match \((\d+)\)', text)
    holes = re.search(r'unregistered or refused points \((\d+)\)', text)
    words = [(int(t), int(a, 8), int(v, 8)) for t, a, v in re.findall(r'^watch (\d+) cs: ([0-7]+) ([0-7]+)$', text, re.M)]
    shown = [(int(t), d) for t, d in re.findall(r'^DSKY (\d+) cs: (.*)$', text, re.M)]
    results = []
    for c in row['checks']:
        if c[0] == 'alarm':
            _, code, t0, t1, what = c
            fail = range(table['FAILREG'], table['FAILREG'] + 3)
            hit = [t for t, a, v in words if a in fail and t0 <= t <= t1 and v & 0o7777 in code]
        elif c[0] == 'word':
            _, sym, mask, want, t0, t1, what = c
            a = address(sym, table)
            hit = [t for t, x, v in words if x == a and t0 <= t <= t1 and v & mask == want]
        elif c[0] == 'quiet':
            _, sym, t0, t1, what = c
            a = address(sym, table)
            changed = [t for t, x, v in words if x == a and t0 <= t <= t1]
            results.append(dict(check=what, ok=not changed, at_cs=changed[0] if changed else None))
            continue
        elif c[0] == 'final':
            _, sym, mask, want, what = c
            a = address(sym, table)
            last = [(t, v) for t, x, v in words if x == a]
            hit = [last[-1][0]] if last and last[-1][1] & mask == want else []
        else:
            _, pattern, t0, t1, what = c
            hit = [t for t, d in shown if t0 <= t <= t1 and re.search(pattern, d)]
        results.append(dict(check=what, ok=bool(hit), at_cs=hit[0] if hit else None))
    return dict(scenario=row['scenario'], exit=r.returncode, mismatches=int(misses[1]) if misses else None,
                bridged=int(holes[1]) if holes else None, checks=results, log=log,
                ok=r.returncode == 0 and misses is not None and holes is not None
                and int(misses[1]) == 0 and int(holes[1]) == 0 and all(x['ok'] for x in results))


def held(x):
    if x['ok']:
        return 'yes' + (f", at {x['at_cs']} cs" if x['at_cs'] is not None else '')
    return 'NO' + (f" (changed at {x['at_cs']} cs)" if x['at_cs'] is not None else '')


def windows(m):
    """The rows' C64 windows, as device_acceptance.WINDOWS tuples."""
    return [(r['scenario'], *r['window'], 50) for r in ROWS[m]]


def check(m, dest):
    """Build the host run and check every row; each row's log goes to dest."""
    engine = fhd.frozen_engine(os.environ.get('CC', 'cc'))
    fhd.build(os.environ.get('CC', 'cc'), m, engine)
    table = symbols(m)
    out = []
    for row in ROWS[m]:
        print(f"{m}: off-nominal {row['scenario']} on the host", flush=True)
        out.append(dict(row=row, host=host_run(m, row, table, dest)))
    return out


def section(results, runs, first_diff):
    """(lines, bad): the doc section, and what fails the result. runs are
    device_acceptance's window results (with '_why', empty on a match)."""
    bad, lines = [], ['## Off-nominal inputs', '',
                      'Each row is a nominal flight scenario (tools/flight_host_differential.py, FLIGHT) with one '
                      'device input gone wrong (acceptance item 8). **Host**: every native routine matches yaAGC with '
                      'no point bridged (scaled time x1), and the AGC answers the bad input as the checks below say '
                      '(words read with `FLIGHT_WATCH`, named by symbol from the program\'s assembly listing; the DSKY '
                      'with `FLIGHT_DSKY`). **C64**: the row\'s window over the bad input (see Runs).', '',
                      '| Scenario | Owner | Input gone wrong | AGC response | Host | C64 window |',
                      '|---|---|---|---|---|---|']
    detail = []
    for res in results:
        row, h = res['row'], res['host']
        snap, cs = row['window']
        win = next((r for r in runs if r['scenario'] == row['scenario'] and r.get('snap_cs') == snap), None)
        c64 = ('not run' if win is None else
               'matches the host at ' + ', '.join(map(str, win.get('replay_matched_cs') or [])) + ' cs' if not win['_why']
               else 'differs: ' + first_diff(win) if win.get('replay_differences')
               else 'no match: ' + '; '.join(win['_why']))
        host = ('clean' if h['ok'] else 'NOT CLEAN') + f": {h['mismatches']} mismatches, {h['bridged']} bridged, " \
            f"{sum(x['ok'] for x in h['checks'])} of {len(h['checks'])} response checks"
        lines.append(f"| {row['scenario']} | {row['owner']} | {row['failure']} | {row['response']} | {host} | "
                     f"{snap}+{cs}: {c64} |")
        if win is None:
            bad.append(f"off-nominal {row['scenario']}: no C64 window {snap}+{cs} in the runs")
        if not h['ok']:
            bad.append(f"off-nominal {row['scenario']}: host {h['mismatches']} mismatches, {h['bridged']} bridged, "
                       f"failed checks {[x['check'] for x in h['checks'] if not x['ok']]}")
        detail.append(f"- **{row['scenario']}** ({h['evidence']}): "
                      + ('; '.join(f"{x['check']}: {held(x)}" for x in h['checks']) or 'host match only'))
    lines += ['', 'A window that differs is an open item on the shared replay mismatch list, as in Runs.', '',
              '### Response checks', ''] + detail + ['']
    return lines, bad
