#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Flight-scenario interpretive code in VICE (docs/INTERPRETER-RUNTIME.md,
milestone interp-flight): the compact interpreter run through the flight
resident from the whole-program flight store, from states yaAGC reaches in
flight scenarios, each result checked against yaAGC.

1. tools/interp_flight_capture.c (host, links yaAGC) runs the scenarios and
   keeps each first arrival at an interpreter entry point (distinct entry,
   LOC and BANKSET) as a case; up to --cases of them, evenly spread.
2. platform/c64/interp_flight_check.c, linked with the flight resident,
   boots on build/flight-store/<m>.reu with the cases streamed into the REU
   above the store, runs resident steps from each case while Z stays at an
   interpreter entry point, and reports the exit Z, FB, a digest and status.
3. tools/interp_flight_capture.c check: yaAGC from each case, interrupts off
   as on the C64, must reach the exit Z under the exit FB with the C64's
   digest.

--flowed runs flowed states instead (not mission flight): for each
FLIGHT_INTERP_STOP module (tools/flight_store_build.py), whose flight module
stops where its code reaches interpretive code,
tools/interp_flight_flowed.c runs yaAGC from its milestone's cases and keeps
every first arrival inside the file at an interpreter entry point. Those the
verification interpreter (tools/interp_flight_verify.c) refuses, or whose
result yaAGC does not reach, run outside the interpreter's contract and are
left out (counted in the report); the rest all run, as many per VICE boot as the REU stream holds (at most --cases).

Needs make flight-store first. Writes build/interp-flight/<m>-report.json
(<m>-flowed-report.json); --evidence writes both missions' rows
(evidence/interpreter/interp-flight-vice.json, interp-flight-flowed-vice.json).

  interp_flight_test.py [--missions lm,cm] [--cases N] [--flowed [--flowed-states N]]
"""
import argparse
import json
import os
import pathlib
import re
import struct
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from resident_flight_build import COMMON, MISSION, resident_sources  # noqa: E402

STORE = ROOT / 'build/flight-store'
OUT = ROOT / 'build/interp-flight'
YAAGC = ROOT / 'deps/virtualagc/yaAGC'
ROPE = {'lm': ROOT / 'evidence/fixed-data-lm-original.rope.bin', 'cm': ROOT / 'evidence/fixed-data-cm-original.rope.bin'}
# P00 from Fresh Start: the NOVAC fresh-start jobs, then V37E00E's state
# vector integration, all interpretive (V37E12E/V37E06E/V37E63E reach no
# entry the first 30 s of P00 does not).
SCENARIOS = {'lm': [('V37E00E', 3000, 'V37E00E', None)], 'cm': [('V37E00E', 3000, 'V37E00E', None)]}
# --p63: Apollo 11 about ten minutes before PDI (tools/scenarios/README.md),
# then V37E63E and through ignition as the core dump's readme keys it:
# P63's ignition-time iteration, PRO (V33E) past V06N61, ENTR past the three
# V50N25 checklists and V50N18, then the countdown, SERVICER from TIG-35 s,
# ignition and the engine-fail branch. A space waits half a second.
P63_KEYS = 'V37E63E' + ' ' * 40 + 'V33E' + (' ' * 10 + 'E') * 4
SCENARIOS_P63 = {'lm': [('P63', 70000, P63_KEYS, 'tools/scenarios/luminary099-p63-ignition.core')], 'cm': []}
# The interpreter's fixed entry points that the store sends to $0A01
# (tools/flight_store_build.py INTERP_POINTS but SWRETURN, which a basic
# table holds; tools/interp_flight_capture.c points[]).
POINTS = {'lm': [0o6070, 0o6061, 0o6063, 0o6106, 0o6460, 0o6144, 0o6463, 0o6466, 0o6252, 0o6646, 0o6037],
          'cm': [0o6037, 0o6030, 0o6032, 0o6055, 0o6427, 0o6113, 0o6432, 0o6435, 0o6221, 0o6615, 0o6006]}
NAMES = ['NEWOPS', 'DANZIG', 'NOIBNKSW', 'DIRADRES', 'DODLOAD', 'DODLOADX', 'DOVLOAD', 'DOVLOADX', 'B15BITADR',
         'GOTO', 'INTPRET']
CASE_BYTES = 4104
# The FLIGHT_INTERP_STOP modules' case sources: tools/interp_flight_flowed.c's
# FLOW_<tag> build, linked with tools/<stem>_freeze.py's CORE.
VERIFY_SOURCES = ['src/lm_interp_runtime.c', 'src/lm_interp_orders.c', 'src/lm_interp_steps.c', 'src/agc_image_ops.c',
                  'src/agc_hardware_divide.c', 'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c']
FLOW_TAGS = {'lm_lunar_landing': 'LLD', 'lm_servicer': 'LMSV', 'lm_burn_baby': 'LMBB', 'lm_ags_init': 'AGS',
             'lm_imu_performance': 'IPT'}
# REU: the case image at IF_REF and the stream from IF_STREAM, below the
# rope images (include/rope_image.h, from 0x1c0000) and above the store.
IF_REF = 0x17f000
IF_STREAM = 0x180000
IF_STREAM_END = 0x1c0000
IF_STEPS = 4000
STATUS = {0: 'EXEC_OK'}


def capture_tool(cc, m):
    # yaAGC with every DV through its hardware model (tools/prepare_hardware_dv.py):
    # the stock engine's fast DV reads an L of -0 as -2^14 (docs/DIVIDE.md),
    # which UNIT meets in flight (a vector component's DCS of +0).
    exe, engine = OUT / f'capture-{m}', ROOT / 'build/agc_engine_hardware_dv.o'
    if not engine.exists():
        subprocess.run([sys.executable, 'tools/prepare_hardware_dv.py'], cwd=ROOT, check=True)
    subprocess.run([cc, '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror', f'-I{YAAGC}', f'-DIF_{m.upper()}=1',
                    'tools/interp_flight_capture.c', str(engine), str(YAAGC / 'agc_engine_init.o'),
                    str(YAAGC / 'rfopen.o'), '-lm', '-o', str(exe)], cwd=ROOT, check=True)
    return exe


def capture(exe, m, want, scenarios=SCENARIOS):
    """The scenarios' cases, evenly thinned to want; with their scenario."""
    cases = []
    for name, cs, keys, core in scenarios[m]:
        path = OUT / f'{m}-{name}-all.bin'
        r = subprocess.run([str(exe), 'capture', str(ROPE[m]), str(cs), keys, '100000', str(path),
                            *([str(ROOT / core)] if core else [])], cwd=ROOT, capture_output=True, text=True)
        if r.returncode:
            raise SystemExit(f'{m} {name}: capture failed: {r.stdout[-500:]}{r.stderr}')
        print(f'{m} {name}: {r.stdout.strip().splitlines()[-1]}')
        data = path.read_bytes()
        cases += [(name, data[i:i + CASE_BYTES]) for i in range(0, len(data), CASE_BYTES)]
    if len(cases) > want:
        cases = [cases[i * len(cases) // want] for i in range(want)]
    return cases


def flowed(m, states, capture_exe):
    """Every FLIGHT_INTERP_STOP module's flowed cases inside the interpreter's
    contract, with their source, and the counts left out."""
    import importlib
    from flight_store_build import FLIGHT_INTERP_STOP
    cases = []
    for stem in sorted(FLIGHT_INTERP_STOP[m]):
        if stem not in FLOW_TAGS:
            raise SystemExit(f'{m}: {stem} stops at interpretive code in flight but has no FLOW_TAGS case source')
        freeze = importlib.import_module(f'{stem}_freeze')
        for obj, prep in (('agc_engine_hardware_dv.o', 'prepare_hardware_dv.py'),
                          ('agc_engine_no_imu_device.o', 'prepare_no_imu_device_engine.py')):
            if str(ROOT / 'build' / obj) in map(str, freeze.CORE) and not (ROOT / 'build' / obj).exists():
                subprocess.run([sys.executable, f'tools/{prep}'], cwd=ROOT, check=True)
        exe, path = OUT / f'flowed-{stem}', OUT / f'flowed-{stem}.bin'
        subprocess.run(['cc', *freeze.FLAGS, '-w', f'-DFLOW_{FLOW_TAGS[stem]}=1', 'tools/interp_flight_flowed.c',
                        *freeze.CORE, '-o', str(exe)], cwd=ROOT, check=True)
        r = subprocess.run([str(exe), str(ROPE[m]), str(states), '100000', str(path)], cwd=ROOT,
                           capture_output=True, text=True)
        if r.returncode:
            raise SystemExit(f'{m} {stem}: flowed capture failed: {r.stdout[-500:]}{r.stderr}')
        print(f'{m} flowed {stem}: {r.stdout.strip().splitlines()[-1]}')
        data = path.read_bytes()
        cases += [(f'flowed {stem.replace("_", "-")}', data[i:i + CASE_BYTES]) for i in range(0, len(data), CASE_BYTES)]
    # The verification interpreter from each case; keep the cases whose
    # result yaAGC reaches (the rest run outside the contract).
    verify = OUT / 'flowed-verify'
    subprocess.run(['cc', '-std=gnu99', '-O2', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests',
                    '-Dexit=interp_flight_verify_exit', 'tools/interp_flight_verify.c', *VERIFY_SOURCES,
                    '-o', str(verify)], cwd=ROOT, check=True)
    every, results = OUT / f'{m}-flowed-all.bin', OUT / f'{m}-flowed-verify.bin'
    every.write_bytes(b''.join(c for _, c in cases))
    subprocess.run([str(verify), str(ROPE[m]), str(every), str(results)], cwd=ROOT, check=True, capture_output=True)
    chk = subprocess.run([str(capture_exe), 'check', str(ROPE[m]), str(every), str(results)], cwd=ROOT,
                         capture_output=True, text=True).stdout.splitlines()
    res = results.read_bytes()
    keep, refused, unreached = [], 0, 0
    for i, c in enumerate(cases):
        status, still = res[12 * i + 10], res[12 * i + 11]
        if status or still:
            refused += 1
        elif ' ok ' not in chk[i]:
            unreached += 1
        else:
            keep.append(c)
    print(f'{m} flowed: {len(keep)} of {len(cases)} cases inside the contract ({refused} refused by the verification '
          f'interpreter, {unreached} whose verification result yaAGC does not reach)')
    return keep, dict(captured=len(cases), refused=refused, unreached=unreached)


def stream(cases):
    """The REU case stream platform/c64/interp_flight_check.c reads: each
    case as the words that differ from the previous one, or whole."""
    out, ref = bytearray(), [0] * 2048
    for _, c in cases:
        words = struct.unpack_from('<2048H', c)
        superbank, entry = struct.unpack_from('<2H', c, 4096)
        diff = [(i, w) for i, w in enumerate(words) if w != ref[i]]
        if 2 * len(diff) > len(words):
            out += struct.pack('<3H', 0xfffe, superbank, entry) + c[:4096]
        else:
            out += struct.pack('<3H', len(diff), superbank, entry)
            for i, w in diff:
                out += struct.pack('<2H', i, w)
        ref = list(words)
    out += struct.pack('<3H', 0xffff, 0, 0)
    return bytes(out)


def link(cc, m, count):
    data = OUT / f'{m}-check.h'
    data.write_text('/* Generated by tools/interp_flight_test.py. */\n'
                    f'#define IF_REF 0x{IF_REF:x}ul\n#define IF_STREAM 0x{IF_STREAM:x}ul\n'
                    f'#define IF_MAX {count}u\n#define IF_STEPS {IF_STEPS}u\n'
                    '#define IF_POINTS {' + ','.join(f'0{p:o}' for p in POINTS[m]) + '}\n')
    out = OUT / f'{m}-check.prg'
    subprocess.run([str(c) for c in [
        cc, *COMMON, *MISSION[m], f'-DIF_DATA="{data}"', '-T', 'platform/c64/resident_flight.ld',
        f'-Wl,-Map,{out}.map', 'platform/c64/interp_flight_check.c', *resident_sources(m),
        STORE / 'overlay_v2_services.S', '-o', out]], cwd=ROOT, check=True)
    return out


def vice(m, prg, reu_bytes, a):
    work = OUT / f'vice-{m}'
    work.mkdir(parents=True, exist_ok=True)
    rom = a.rom_dir.resolve()
    sys_at = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    if sys_at is None:
        raise SystemExit(f'FAIL: {prg.name} has no BASIC SYS header')
    dump = work / 'results.bin'
    dump.unlink(missing_ok=True)
    # $c00f is stored once, last (the pinned module's boot DMA covers $c000).
    (work / 'vice.mon').write_text(f'load "{prg}" 0\nwatch store c00f\n'
                                   f'g {int(sys_at[1]):04x}\nbsave "{dump}" 0 c000 cbff\nquit\n')
    reu = work / 'image.reu'
    reu.write_bytes(reu_bytes)
    cmd = [a.vice, '-default', '-console', '-pal', '-warp', '+sound',
           '-kernal', str(rom / 'kernal-901227-03.bin'), '-basic', str(rom / 'basic-901226-01.bin'),
           '-chargen', str(rom / 'chargen-901225-01.bin'),
           '-directory', f'{a.data_dir.resolve()}:{rom.parent}',
           '-initbreak', 'ready', '-moncommands', str(work / 'vice.mon'),
           '-monlogname', str(work / 'monitor.log'), '-monlog', '-limitcycles', str(a.cycles),
           '-reu', '-reusize', str(len(reu_bytes) // 1024), '-reuimage', str(reu)]
    env = dict(os.environ, XDG_CACHE_HOME=str(work / 'cache'), XDG_CONFIG_HOME=str(work / 'config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        pathlib.Path(env[key]).mkdir(parents=True, exist_ok=True)
    with (work / 'vice.log').open('w') as log:
        subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, timeout=a.timeout, env=env)
    if not dump.exists():
        raise SystemExit(f'FAIL: {m}: no mailbox (cycle limit or crash; see {work}/vice.log)')
    return dump.read_bytes()


def run(m, a, exe):
    left_out = None
    if a.flowed or a.p63:
        if a.flowed:
            every, left_out = flowed(m, a.flowed_states, exe)
        else:
            every = capture(exe, m, a.total, SCENARIOS_P63)
        if not every:
            print(f'{m}: no {"FLIGHT_INTERP_STOP module" if a.flowed else "P63 scenario"}, nothing to run')
            return None
        if a.only:
            lo, hi = (int(x) for x in a.only.split(':'))
            every = every[lo:hi]
        # As many cases per boot as the results area and the REU stream hold.
        batches, size, prev = [[]], 0, None
        for c in every:
            n = len(stream([prev, c] if prev else [c])) - (len(stream([prev])) if prev else 6)
            if len(batches[-1]) == a.cases or IF_STREAM + size + n > IF_STREAM_END:
                batches.append([])
                size, n = 6, len(stream([c]))
            batches[-1].append(c)
            size += n
            prev = c
    else:
        batches = [capture(exe, m, a.cases)]
        if a.only:
            lo, hi = (int(x) for x in a.only.split(':'))
            batches = [batches[0][lo:hi]]
    if a.part:
        # (--part K/N: the boots k with k % N == K - 1, so N CI targets each
        # run a share; the batches, and so each boot's cases, stay the same)
        K, N = (int(x) for x in a.part.split('/'))
        batches = [(k, c) for k, c in enumerate(batches) if k % N == K - 1]
    else:
        batches = list(enumerate(batches))
    if not batches:
        print(f'{m}: no boot in part {a.part}, nothing to run')
        return None
    parts = [boot(m, a, exe, cases, k) for k, cases in batches]
    rows = [r for p in parts for r in p['rows']]
    report = dict(mission=m, ok=all(p['ok'] for p in parts), cases=sum(p['cases'] for p in parts),
                  ran=sum(p['ran'] for p in parts), c64_failures=sum(p['c64_failures'] for p in parts),
                  first_failing_step=next((p['first_failing_step'] for p in parts if p['c64_failures']), 0),
                  boots=len(parts), yaagc_check='; '.join(p['yaagc_check'] for p in parts),
                  scenarios=sorted({r['scenario'].split(' ', 1)[1] for r in rows}) if a.flowed else [k[0] for k in (SCENARIOS_P63 if a.p63 else SCENARIOS)[m]],
                  steps=sum(r['steps'] for r in rows), stream_bytes=max(p['stream_bytes'] for p in parts),
                  **({'left_out': left_out} if a.flowed else {}), rows=rows)
    (OUT / f'{m}{suffix(a)}-report.json').write_text(json.dumps(report, indent=1) + '\n')
    return report


def suffix(a):
    return ('-flowed' if a.flowed else '-p63' if a.p63 else '') + (f'-part{a.part.split("/")[0]}' if a.part else '')


def boot(m, a, exe, cases, k):
    """One VICE boot over cases (batch k): its rows and summary."""
    s = stream(cases)
    if IF_STREAM + len(s) > IF_STREAM_END:
        raise SystemExit(f'{m}: the case stream ({len(s)} bytes) passes {IF_STREAM_END:#x}')
    image = bytearray((STORE / f'{m}.reu').read_bytes())
    if any(image[IF_REF:IF_STREAM_END]):
        raise SystemExit(f'{m}: {m}.reu is not free at {IF_REF:#x}-{IF_STREAM_END:#x}')
    image[IF_STREAM:IF_STREAM + len(s)] = s
    prg = link(a.cc, m, len(cases))
    data = vice(m, prg, bytes(image), a)
    failures, ran = data[1] | data[2] << 8, data[3] | data[4] << 8
    results = data[0x10:0x10 + 12 * ran]
    base = OUT / f'{m}{suffix(a)}{k if suffix(a) else ""}'
    (base.parent / f'{base.name}-cases.bin').write_bytes(b''.join(c for _, c in cases[:ran]))
    (base.parent / f'{base.name}-results.bin').write_bytes(results)
    chk = subprocess.run([str(exe), 'check', str(ROPE[m]), str(base.parent / f'{base.name}-cases.bin'),
                          str(base.parent / f'{base.name}-results.bin')],
                         cwd=ROOT, capture_output=True, text=True)
    lines = chk.stdout.splitlines()
    rows = []
    for i, (keys, c) in enumerate(cases[:ran]):
        digest, z, fb, steps, status, flags = struct.unpack_from('<IHHHBB', results, 12 * i)
        entry = struct.unpack_from('<H', c, 4098)[0]
        rows.append(dict(scenario=keys if a.flowed else f'{m.upper()} {keys}', entry=NAMES[entry], loc=f'{struct.unpack_from("<H", c, 2 * 0o164)[0]:05o}',
                         bankset=f'{struct.unpack_from("<H", c, 2 * 0o165)[0]:05o}', exit_z=f'{z:05o}', exit_fb=f'{fb:05o}',
                         steps=steps, status=status, still_at_entry=bool(flags), digest=f'{digest:08x}',
                         yaagc=lines[i] if i < len(lines) else ''))
    yaagc_ok = chk.returncode == 0
    ok = data[0] == 0xa5 and not failures and ran == len(cases) and yaagc_ok
    return dict(ok=ok, cases=len(cases), ran=ran, c64_failures=failures, first_failing_step=data[5],
                yaagc_check=lines[-1] if lines else chk.stderr, stream_bytes=len(s), rows=rows)


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--missions', default='lm,cm')
    p.add_argument('--cases', type=int, default=240)
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--only', help='LO:HI, run only those of the thinned cases (bisecting)')
    p.add_argument('--flowed', action='store_true',
                   help="the FLIGHT_INTERP_STOP modules' flowed states instead of the flight scenarios")
    p.add_argument('--p63', action='store_true',
                   help='the LM P63 scenario from the Apollo 11 core dump instead of P00 (--total cases, several boots)')
    p.add_argument('--total', type=int, default=1000,
                   help='with --p63: the cases, evenly thinned from every first arrival (default 1000)')
    p.add_argument('--flowed-states', type=int, default=128,
                   help='with --flowed: the states of each routine yaAGC runs from (default 128)')
    p.add_argument('--part', help='K/N (with --flowed or --p63): run only the boots k with k %% N == K - 1, '
                   'for N CI targets (interp-flight-flowed-vice-K); no --evidence')
    p.add_argument('--cycles', type=int, default=3_000_000_000)
    p.add_argument('--timeout', type=int, default=1800)
    p.add_argument('--evidence', type=pathlib.Path,
                   help='write the summary and rows here (evidence/interpreter/interp-flight-vice.json)')
    p.add_argument('--out', type=pathlib.Path, help='output directory (default build/interp-flight)')
    a = p.parse_args()
    global OUT
    OUT = (a.out or OUT).resolve()
    if a.part and (a.evidence or not (a.flowed or a.p63) or not re.fullmatch(r'[1-9]\d*/[1-9]\d*', a.part)
                   or int(a.part.split('/')[0]) > int(a.part.split('/')[1])):
        raise SystemExit('--part K/N (1 <= K <= N) needs --flowed or --p63 and no --evidence')
    if a.cases > 250:
        raise SystemExit('--cases: at most 250 (the results fill $c010-$cbff)')
    OUT.mkdir(parents=True, exist_ok=True)
    failed, evidence = 0, {}
    for m in a.missions.split(','):
        r = run(m, a, capture_tool(a.host_cc, m))
        if r is None:
            continue
        evidence[m] = dict(ok=r['ok'], cases=r['cases'], ran=r['ran'], c64_failures=r['c64_failures'], boots=r['boots'],
                           **({'left_out': r['left_out']} if a.flowed else {}),
                           yaagc_check=r['yaagc_check'], scenarios=r['scenarios'], stream_bytes=r['stream_bytes'],
                           **({'kind': 'flowed states (not mission flight): tools/interp_flight_flowed.c',
                               'flowed_states': a.flowed_states} if a.flowed else
                              {'kind': 'Apollo 11 P63 from NASSP core dump (tools/scenarios/README.md)', 'keys': P63_KEYS,
                               'total': a.total} if a.p63 else {}),
                           columns=(['source'] if a.flowed else []) + ['entry', 'LOC', 'BANKSET', 'exit_Z', 'exit_FB', 'resident_steps', 'status', 'digest'],
                           rows=[([x['scenario']] if a.flowed else []) + [x['entry'], x['loc'], x['bankset'], x['exit_z'], x['exit_fb'], x['steps'], x['status'],
                                  x['digest']] for x in r['rows']])
        bad = [x for x in r['rows'] if x['status'] or x['still_at_entry'] or not x['yaagc'].startswith(f'case ') or ' ok ' not in x['yaagc']]
        entries = sorted({x['entry'] for x in r['rows']})
        if r['ok']:
            print(f"PASS: VICE PAL {m} interp-flight{' flowed states' if a.flowed else ' P63' if a.p63 else ''}: {r['ran']} cases "
                  f"in {r['boots']} boot{'s' if r['boots'] > 1 else ''} from {', '.join(r['scenarios'])}, "
                  f"{r['steps']} resident steps, every exit state reached by yaAGC; entries {', '.join(entries)}")
        else:
            failed += 1
            print(f"FAIL: {m} interp-flight: {r['ran']}/{r['cases']} cases ran, {r['c64_failures']} C64 failures "
                  f"(first step {r['first_failing_step']}), yaAGC: {r['yaagc_check']}")
            for x in bad[:12]:
                print(f"  {x['scenario']} {x['entry']} LOC {x['loc']} BANKSET {x['bankset']}: exit Z {x['exit_z']} "
                      f"FB {x['exit_fb']}, {x['steps']} steps, status {x['status']}"
                      f"{', still at an entry' if x['still_at_entry'] else ''}; {x['yaagc']}")
    if a.evidence:
        a.evidence.write_text(json.dumps(evidence, separators=(',', ':')))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
