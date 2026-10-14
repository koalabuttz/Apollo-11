#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The downlink word stream in flight (docs/FLIGHT-DOWNLINK.md): each
scenario of tools/flight_restart_check.py, its GOJAM included, run with
FLIGHT_DOWNRUPT=1 (tools/flight_downlink.h: a DOWNRUPT every 20 ms of AGC
time on both sides) and the channel 34/35 words logged on both sides
(FLIGHT_DOWNLINK).

A scenario fails if the run stops early, if the native word stream (NAT)
and yaAGC's in lockstep (SPAN) differ in any word, word-order bit or 20 ms
frame, or in length, if (unscaled) the native stream differs from the
free-running reference's (REF) by more than REF_SLACK words, if the run adds
a routine yaAGC does not match to the same run without DOWNRUPT, if a
downlist other than the last or one cut by the GOJAM is not whole, or if the
first words after the GOJAM are not the ID and sync words of the list
DNLSTCOD names (STARTSUB points DNTMGOTO at LDNPHAS1).

--vice (needs make flight-store and VICE): for each window in VICE_WINDOWS,
a native snapshot (FLIGHT_SNAPSHOT, the run with FLIGHT_DOWNRUPT) at an idle
sample point, then platform/c64/downlink_check.c on the C64 store takes
--downrupts DOWNRUPTs from it one after another with CIA1 interrupts off,
and its channel 34/35 words and word-order bits must equal yaAGC's from the
same state with its clock frozen (tools/flight_downlink_reference.c).

Writes build/flight-downlink/ (logs, word streams, summary.json).

  flight_downlink_check.py [--host-cc CC] [--skip-index] [--only NAME,...] [--jobs N] [--vice [--downrupts N]]
"""
import argparse, json, os, re, struct, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402
import flight_restart_check as frc  # noqa: E402

OUT = ROOT / 'build/flight-downlink'
# A downlist is 100 word pairs: ID and sync, then 99 (DOWNLINK_LISTS.agc).
LIST_WORDS = 200
SYNC = 0o77340
# The downlist IDs (yaAGC's ddd-<ID>-Luminary099.tsv and the Colossus 2
# ones for Comanche055): 77777 - DNLSTCOD.
NAMES = {'lm': {0o77777: 'coast and align', 0o77776: 'AGS initialization and update', 0o77775: 'rendezvous/prethrust',
                0o77774: 'orbital maneuvers', 0o77773: 'descent and ascent', 0o77772: 'lunar surface align',
                0o1776: 'erasable dump'},
         'cm': {0o77777: 'coast and align', 0o77776: 'entry and update', 0o77775: 'rendezvous and prethrust',
                0o77774: 'powered', 0o77773: 'program 22', 0o1777: 'erasable dump'}}
# Unscaled, the native side takes an interrupt only at a routine boundary
# (docs/FLIGHT-RESTART.md, the LM P00 samples), so a DOWNRUPT can land in a
# later 20 ms frame than on the free-running reference. Words whose frame
# alone differs are counted apart and allowed up to this many.
REF_SLACK = 0
# C64 windows: (tag, scenario, snapshot centisecond, with the scenario's
# GOJAM). lm-p63-descent: the descent and ascent list in the braking phase,
# before the GOJAM; -after-gojam: the list STARTSUB restarted, 9 cs after the
# GOJAM; lm-p00: the coast and align list in P00.
VICE_WINDOWS = [('lm-p63-descent', 'lm-p63-descent-restart', 19950, False),
                ('lm-p63-descent-after-gojam', 'lm-p63-descent-restart', 20010, True),
                ('lm-p00', 'lm-p00-restart', 2400, False)]


def stream(path):
    s = {'REF': [], 'NAT': [], 'SPAN': [], 'GOJAM': []}
    for line in path.read_text().splitlines():
        f = line.split()
        if f[0] == 'GOJAM':
            s['GOJAM'].append((int(f[1]), int(f[2])))
            continue
        s[f[0]].append(dict(cs=int(f[2]), frame=int(f[3]), ch=int(f[4], 8), value=int(f[5], 8), wo=int(f[6]),
                            bridged=len(f) > 7))
    return s


def lists(words):
    """(start index, ID, length in words) of each list in a stream."""
    starts = [i for i in range(len(words) - 1) if words[i]['ch'] == 0o34 and not words[i]['wo']
              and words[i + 1]['ch'] == 0o35 and words[i + 1]['value'] == SYNC]
    return [(i, words[i]['value'], (starts[k + 1] if k + 1 < len(starts) else len(words)) - i) for k, i in enumerate(starts)]


def differ(a, b, timed=True):
    keys = ('ch', 'value', 'wo') + (('frame',) if timed else ())
    return [i for i in range(min(len(a), len(b))) if any(a[i][k] != b[i][k] for k in keys)]


def check(exe, scenario):
    name, m, cs, keys, env, at, sched, _ = scenario
    env = {**env, **({'FLIGHT_SCALED': sched} if sched else {}), 'FLIGHT_GOJAM': str(at)}
    words = OUT / f'{name}.words'
    words.unlink(missing_ok=True)
    on = frc.run(exe[m], m, cs, keys, {**env, 'FLIGHT_DOWNRUPT': '1', 'FLIGHT_DOWNLINK': str(words)}, name)
    off = frc.run(exe[m], m, cs, keys, env, f'{name}-no-downrupt')
    mis = lambda t: sorted(set(re.findall(r'^  (?:basic |interp) (\d\d,\d{4}) (\S+): \d+ times', t, re.M)))  # noqa: E731
    t = re.search(r'AGC time (\d+) cs', on.stdout)
    s = stream(words) if words.exists() else {'REF': [], 'NAT': [], 'SPAN': [], 'GOJAM': []}
    nat, span, ref = s['NAT'], s['SPAN'], s['REF']
    ls = lists(nat)
    names = NAMES[m]
    gojam = re.findall(r'^GOJAM at (\d+) cs', on.stdout, re.M)
    jam = int(gojam[0]) if gojam else None
    # The list the GOJAM cut, and the first words after it (the native
    # stream's index at the restart from 04000).
    after = s['GOJAM'][0][0] if s['GOJAM'] else None
    cut = [x for x in ls if after is not None and x[0] < after <= x[0] + x[2]]
    first = next((x for x in ls if x[0] == after), None)
    unm = re.search(r'(\d+) words of unmatched spans', on.stdout)
    by_id = {}
    for i, ident, n in ls:
        d = by_id.setdefault(f'{ident:05o}', dict(name=names.get(ident, '?'), lists=0, whole=0, first_cs=nat[i]['cs']))
        d['lists'] += 1
        d['whole'] += n == LIST_WORDS
    partial = [dict(at_cs=nat[i]['cs'], id=f'{ident:05o}', words=n) for k, (i, ident, n) in enumerate(ls)
               if n != LIST_WORDS and k != len(ls) - 1 and (i, ident, n) not in cut]
    ref_diff = differ(nat, ref) if ref else []
    ref_value_diff = differ(nat, ref, timed=False) if ref else []
    row = dict(scenario=name, mission=m, centiseconds=cs, schedule=sched, gojam_cs=jam, exit=on.returncode,
               agc_cs=int(t[1]) if t else None, native_words=len(nat), span_words=len(span), reference_words=len(ref),
               bridged_words=sum(w['bridged'] for w in nat), unmatched_span_words=int(unm[1]) if unm else None,
               span_differ=len(differ(nat, span)) + abs(len(nat) - len(span)),
               reference_differ=len(ref_value_diff) + (abs(len(nat) - len(ref)) if ref else 0),
               reference_frame_only=len(ref_diff) - len(ref_value_diff),
               lists=by_id, partial_lists=partial,
               cut_by_gojam=[dict(id=f'{x[1]:05o}', from_cs=nat[x[0]]['cs'], words=x[2]) for x in cut],
               after_gojam=dict(cs=nat[after]['cs'], words=[f"{nat[after + k]['value']:05o}" for k in range(2)],
                                list=f'{first[1]:05o}' if first else None, list_name=names.get(first[1]) if first else None,
                                list_words=first[2] if first else None) if after is not None and after + 1 < len(nat) else None,
               mismatches=[' '.join(x) for x in mis(on.stdout)],
               new_mismatches=[' '.join(x) for x in mis(on.stdout) if x not in mis(off.stdout)])
    bad = []
    if on.returncode or len(gojam) != 1 or (sched and (row['agc_cs'] or 0) < cs):
        bad.append(f"exit {on.returncode}, {len(gojam)} GOJAMs, AGC time {row['agc_cs']}")
    if not nat or row['span_differ'] or row['unmatched_span_words']:
        bad.append(f"{len(nat)} native words, {row['span_differ']} differ from yaAGC's in lockstep, "
                   f"{row['unmatched_span_words']} in unmatched spans")
    if ref and (row['reference_differ'] or row['reference_frame_only'] > REF_SLACK):
        bad.append(f"{row['reference_differ']} words differ from the reference ({row['reference_frame_only']} in frame only)")
    if row['new_mismatches']:
        bad.append(f"new mismatches {row['new_mismatches']}")
    if partial:
        bad.append(f'lists not whole: {partial[:4]}')
    if not first or row['after_gojam']['words'][1] != f'{SYNC:05o}':
        bad.append(f"after the GOJAM: {row['after_gojam']}")
    row['ok'] = not bad
    row['failures'] = bad
    return row


def c64_window(a, exe, window, scratch, pinned):
    from resident_flight_build import COMMON, MISSION, resident_sources
    tag, scenario, at, jam = window
    name, m, cs, keys, env, gojam, sched, _ = next(x for x in frc.RESTARTS if x[0] == scenario)
    env = {**env, **({'FLIGHT_SCALED': sched} if sched else {}), **({'FLIGHT_GOJAM': str(gojam)} if jam else {}),
           'FLIGHT_DOWNRUPT': '1'}
    header = frc.snapshot(exe[m], m, at, keys, env, f'{tag}-snap')
    n = a.downrupts
    prg = OUT / f'{tag}.prg'
    subprocess.run([str(c) for c in [
        a.cc, *COMMON, *MISSION[m], f'-DDL_SNAP="{header}"', f'-DDL_N={n}u', f'-DDL_SCRATCH=0x{scratch:x}ul',
        '-T', 'platform/c64/resident_flight.ld', f'-Wl,-Map,{prg}.map',
        'platform/c64/downlink_check.c', *resident_sources(m), frc.STORE / 'overlay_v2_services.S', '-o', prg]],
        cwd=ROOT, check=True)
    d, _ = frc.vice_boot(a, m, prg, OUT / f'vice-{tag}', pinned, mail_end=f'{0xc040 + 6 * n - 1:x}')
    row = dict(window=tag, scenario=scenario, snapshot=str(header.relative_to(ROOT)), snapshot_cs=at, downrupts=n)
    if d is None:
        row.update(ok=False, failures=[f'no mailbox (cycle limit or crash; see {OUT}/vice-{tag}/vice.log)'])
        return row
    steps, records, start, z, fb, status, isr = struct.unpack_from('<I4H2B', d, 0x10)
    c64 = [struct.unpack_from('<2H2B', d, 0x40 + 6 * k) for k in range(records)]
    ref_exe = OUT / f'flight-downlink-reference-{tag}'
    y = fhd.YAAGC
    subprocess.run([a.host_cc, '-std=c99', '-O1', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', f'-I{y}',
                    f'-DRK_{m.upper()}=1', f'-DDL_SNAP="{header}"', 'tools/flight_downlink_reference.c',
                    str(fhd.OUT / 'agc_engine_scaled.o'), str(y / 'agc_engine_init.o'), str(y / 'rfopen.o'), '-lm',
                    '-o', str(ref_exe)], cwd=ROOT, check=True)
    r = subprocess.run([str(ref_exe), fhd.ROPE[m], str(n)], cwd=ROOT, capture_output=True, text=True)
    ref = [tuple(int(x, 8) for x in line.split()) for line in r.stdout.splitlines()]
    words = [(c[0], c[1], c[2]) for c in c64]
    diff = [k for k in range(min(len(words), len(ref))) if words[k] != ref[k]]
    starts = [k for k, w in enumerate(words) if not w[2] and w[1] == SYNC]
    row.update(steps=steps, records=records, start_cs=start, z=f'{z:05o}', fb=f'{fb:05o}', status=status, in_interrupt=isr,
               boot_failed=d[5] == 1, reference_records=len(ref), reference=r.stderr.strip(),
               records_differ=len(diff) + abs(len(words) - len(ref)),
               first_differences=[dict(downrupt=k, c64=[f'{x:05o}' for x in words[k]], yaagc=[f'{x:05o}' for x in ref[k]])
                                  for k in diff[:4]],
               list_starts=[dict(downrupt=k, id=f'{words[k][0]:05o}', name=NAMES[m].get(words[k][0], '?')) for k in starts],
               first_words=[f'{w[0]:05o} {w[1]:05o} {w[2]}' for w in words[:2]])
    bad = []
    if d[5] or records != n or r.returncode:
        bad.append(f'{records} of {n} DOWNRUPTs on the C64 (status class {d[5]}, step status {status}), '
                   f'reference exit {r.returncode} {r.stderr.strip()}')
    if row['records_differ']:
        bad.append(f"{row['records_differ']} DOWNRUPTs' words differ from yaAGC's")
    row.update(ok=not bad, failures=bad)
    return row


def vice(a, exe):
    manifest = json.loads((frc.STORE / 'lm-manifest.json').read_text())
    pinned = any(x['slot'] == 3 for x in manifest['modules'])
    reu = (frc.STORE / 'lm.reu').read_bytes()
    scratch = len(reu) - 4096
    if any(reu[scratch:]):
        sys.exit('FAIL: no unused 4096 bytes at the end of lm.reu')
    rows = []
    for w in VICE_WINDOWS:
        row = c64_window(a, exe, w, scratch, pinned)
        print(f"C64 {row['window']}: {row.get('records')} of {row['downrupts']} DOWNRUPTs from {row['snapshot_cs']} cs "
              f"in {row.get('steps')} resident steps, {row.get('records_differ')} differ from yaAGC's; "
              f"first words {row.get('first_words')}; lists started "
              + ', '.join(f"{x['id']} {x['name']} at DOWNRUPT {x['downrupt']}" for x in row.get('list_starts', []))
              + ('' if row['ok'] else ' FAILED: ' + '; '.join(row['failures'])))
        rows.append(row)
    return rows


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-index', action='store_true', help='reuse build/flight-store/<m>-host.inc and host objects')
    p.add_argument('--only', help='comma-separated scenario names')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 1)
    p.add_argument('--vice', dest='run_vice', action='store_true', help='also take DOWNRUPTs on the C64 store in VICE')
    p.add_argument('--c64-only', action='store_true', help='with --vice: skip the host scenarios')
    p.add_argument('--downrupts', type=int, default=100, help='DOWNRUPTs each C64 window takes')
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--vice-bin', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=Path, default=Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=Path, default=Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--cycles', type=int, default=3_000_000_000)
    p.add_argument('--timeout', type=int, default=1800)
    a = p.parse_args()
    if not a.skip_index:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
    engine = fhd.frozen_engine(a.host_cc)
    exe = {m: fhd.build(a.host_cc, m, engine) for m in ('lm', 'cm')}
    OUT.mkdir(parents=True, exist_ok=True)
    frc.OUT = OUT   # frc.run writes its logs here
    only = set(a.only.split(',')) if a.only else None
    todo = [x for x in frc.RESTARTS if (not only or x[0] in only) and not (a.run_vice and a.c64_only)]
    with ThreadPoolExecutor(max(1, a.jobs)) as pool:
        rows = list(pool.map(lambda x: check(exe, x), todo))
    for row in rows:
        ag = row['after_gojam'] or {}
        print(f"{row['scenario']}: {row['native_words']} native words ({row['bridged_words']} bridged), "
              f"{row['span_words']} yaAGC in lockstep, {row['span_differ']} differ"
              + (f"; reference {row['reference_words']}, {row['reference_differ']} differ, "
                 f"{row['reference_frame_only']} in frame only" if row['reference_words'] else '')
              + '; lists ' + ', '.join(f"{k} {v['name']} x{v['lists']} ({v['whole']} whole) from {v['first_cs']} cs"
                                       for k, v in row['lists'].items())
              + f"; GOJAM at {row['gojam_cs']} cs cut {[c['id'] for c in row['cut_by_gojam']]}, "
              f"then at {ag.get('cs')} cs {ag.get('words')} ({ag.get('list')} {ag.get('list_name')}, {ag.get('list_words')} words)"
              + f"; {len(row['mismatches'])} yaAGC did not match ({len(row['new_mismatches'])} new)"
              + ('' if row['ok'] else ' FAILED: ' + '; '.join(row['failures'])))
    c64 = vice(a, exe) if a.run_vice else []
    (OUT / 'summary.json').write_text(json.dumps(dict(host=rows, c64=c64), indent=1) + '\n')
    return 1 if any(not r['ok'] for r in rows + c64) else 0


if __name__ == '__main__':
    sys.exit(main())
