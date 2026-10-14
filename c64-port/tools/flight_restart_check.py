#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Hardware restarts mid-scenario (docs/FLIGHT-RESTART.md): a GOJAM
(FLIGHT_GOJAM, tools/flight_gojam.h) at a clean sample point of a flight
scenario, then the AGC's own restart (GOPROG, the phase-table check,
STARTSUB/STARTSB2, each active group's restart) and the resumed program,
every native routine in lockstep with yaAGC (tools/flight_host_run.c).

Host (always): each scenario in RESTARTS runs twice, without and with the
GOJAM, plus native snapshots (FLIGHT_SNAPSHOT) just before the GOJAM, just
after it and at the end. It fails if the restart run stops early, adds a
routine yaAGC does not match, takes any restart point outside a routine
table, or
if the restart does not count itself (REDOCTR), keep the interrupted Z in
RSBBQ, end in the same major mode with the same alarms, or (unscaled) adds
sample differences over its run without the restart.

--vice (needs make flight-store and VICE): from the state just after the
GOJAM, platform/c64/restart_check.c runs the flight resident on the C64
store, first the restart burst with CIA1 interrupts off to the next idle
pass through ADVAN (or the first point the store does not hold), and its
erasable image must equal yaAGC's from the same state with its clock frozen
(tools/flight_restart_reference.c); then the resume, with CIA1 interrupts on
for --resume-cs centiseconds of AGC time, reported (the store does not yet
hold all of P63).

--vice --gojam-module (make flight-gojam-vice) instead starts from the state
just before the GOJAM and applies it on the C64, as the resident's trigger
will: the GOJAM module (0x01E5, platform/c64/resident_gojam_module.c)
through rk_run_module, whose image and device records must equal the
post-GOJAM snapshot's; then apollo_ov2_restart_keep_pinned and the burst,
whose image must equal yaAGC's as above.

Writes build/flight-restart/ (logs, snapshots, summary.json, or
summary-gojam-module.json).

  flight_restart_check.py [--host-cc CC] [--skip-index] [--only NAME,...]
                          [--vice [--resume-cs CS | --gojam-module]]
"""
import argparse, json, os, re, struct, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402
import flight_host_differential as fhd  # noqa: E402

OUT = ROOT / 'build/flight-restart'
STORE = ROOT / 'build/flight-store'
# The hardware restart module (platform/c64/resident_flight.h RK_MODULE_GOJAM).
GOJAM_MODULE = 0x01E5
PAL_HZ = 985248
# (name, mission, centiseconds, keys, inputs, GOJAM centisecond, FLIGHT_SCALED
# schedule or None for unscaled with samples, run on the C64 with --vice).
# lm-p63-descent-restart (FLIGHT in tools/flight_host_differential.py):
# lm-p63-descent-thrust restarted at 200 s, in the braking phase 54 s after ignition with groups
# 2 (phase 21) and 5 (phase 3) active; it must land back in P63 with the
# LLGE flying. lm-dap-hold-restart: the DAP holding attitude, restarted at
# 20 s, under the overrun schedule (four times slower, each span's cost
# jittered). cm-entry-restart: cm-entry restarted at 250 s, in P64, ending
# in P67 as without it. lm-p00-restart, cm-p00-restart: P00 idle restarted
# at 25 s, unscaled, sampled against a free-running yaAGC.
_P63 = next(x for x in fhd.FLIGHT if x[0] == 'lm-p63-descent-restart')
RESTARTS = [
    (_P63[0], _P63[1], _P63[2], _P63[3], {k: v for k, v in _P63[4].items() if k != 'FLIGHT_GOJAM'},
     int(_P63[4]['FLIGHT_GOJAM']), '1,1,2', True),
    ('lm-dap-hold-restart', 'lm', 3000, '', fhd.DAP, 2000, '4,1,2,1', True),
    ('cm-entry-restart', 'cm', 32000, fhd.ENTRY_KEYS, fhd.ENTRY, 25000, '1,1,2', False),
    ('lm-p00-restart', 'lm', 4000, '', {}, 2500, None, False),
    ('cm-p00-restart', 'cm', 4000, '', {}, 2500, None, False),
]
# Unscaled, the samples a restart may add over its run without one. LM P00:
# after the restart STARTSB1 has DAPIDLER call 1/ACCS, and natively AOSTASK's
# routine from GETAOSUV+6 (20,3250) runs about 1.2 cs as one call, so the
# T4RUPT due meanwhile waits for its end, where yaAGC takes it at INVERT
# (the routine matches yaAGC run from its own start). From then on T4RUPT
# runs a centisecond late natively, and one sample in each 120 ms T4RUPT
# cycle differs in DSRUPTSW (with the display words at the first ones).
SAMPLE_SLACK = {'lm-p00-restart': 128}
# The files whose points a restart runs (FRESH_START_AND_RESTART, RESTARTS,
# PHASE_TABLE_MAINTENANCE): none may be bridged.
RESTART_FILES = ('FRESH_START_AND_RESTART.agc', 'RESTARTS_ROUTINE.agc', 'PHASE_TABLE_MAINTENANCE.agc')
LISTING = {'lm': ROOT / 'build/fixed-data/Luminary099/assembly.lst', 'cm': ROOT / 'build/fixed-data/Comanche055/assembly.lst'}
ADVAN = {'lm': 0o3214, 'cm': 0o3231}


def symbols(m):
    """Erasable symbol -> address, from the listing (as tools/flight_scenarios/lm_p63.py)."""
    syms = {}
    for line in open(LISTING[m], errors='replace'):
        for x in re.finditer(r'\d+,E:\s+(\S+)\s+(\S+)', line):
            s = x.group(2)
            try:
                syms[x.group(1)] = int(s[1:].split(',')[0]) * 0o400 + int(s.split(',')[1], 8) - 0o1400 if ',' in s else int(s, 8)
            except ValueError:
                pass
    return syms


def snap_words(path):
    t = path.read_text()
    return [int(x.rstrip('u')) for x in re.search(r'snap_words\[2048\] SNAP_DATA=\{([^}]*)\}', t)[1].split(',')]


def run(exe, m, cs, keys, env, tag):
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT, capture_output=True,
                       text=True, env={**os.environ, **env})
    (OUT / f'{tag}.log').write_text(r.stdout + r.stderr)
    return r


def snapshot(exe, m, cs, keys, env, tag):
    path = OUT / f'{tag}.h'
    path.unlink(missing_ok=True)
    run(exe, m, cs + 1, keys, {**env, 'FLIGHT_SNAPSHOT': f'{cs},{path}'}, tag)
    if not path.exists():
        sys.exit(f'FAIL: no snapshot {tag} (see {OUT}/{tag}.log)')
    return path


def parse(text, m, words, lanes):
    n = re.search(r'(\d+) native routines \((\d+) yaAGC instructions of time\), (\d+) bridged', text)
    s = re.search(r'(\d+) samples: (\d+) equal, (\d+) differ', text)
    t = re.search(r'AGC time (\d+) cs', text)
    return dict(routines=int(n[1]) if n else None, bridged=int(n[3]) if n else None,
                samples=int(s[1]) if s else 0, equal=int(s[2]) if s else 0,
                agc_cs=int(t[1]) if t else None, gojams=re.findall(r'^GOJAM at (\d+) cs \(Q (\d+)\)', text, re.M),
                mismatches=sorted(set(re.findall(r'^  (?:basic |interp) (\d\d,\d{4}) (\S+): \d+ times', text, re.M))),
                holes=[flight_holes.describe(h, words, lanes, m) for h in flight_holes.HOLE.finditer(text)])


def host(exe, scenario, words, lanes):
    name, m, cs, keys, env, at, sched, _ = scenario
    env = {**env, **({'FLIGHT_SCALED': sched} if sched else {})}
    jam = {**env, 'FLIGHT_GOJAM': str(at)}
    base = parse(run(exe[m], m, cs, keys, env, f'{name}-base').stdout, m, words[m], lanes)
    r = run(exe[m], m, cs, keys, jam, name)
    got = parse(r.stdout, m, words[m], lanes)
    sym = symbols(m)
    pre_path = snapshot(exe[m], m, at, keys, env, f'{name}-pre')
    pre = snap_words(pre_path)
    post = snapshot(exe[m], m, at, keys, jam, f'{name}-gojam')
    end = snap_words(snapshot(exe[m], m, cs, keys, jam, f'{name}-end'))
    end_base = snap_words(snapshot(exe[m], m, cs, keys, env, f'{name}-base-end'))
    failreg = [sym['FAILREG'] + k for k in range(3)]
    phases = {f'PHASE{g}': sym[f'PHASE{g}'] for g in range(1, 7)}
    row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, inputs=env, gojam_cs=at, schedule=sched, exit=r.returncode,
               native_routines=got['routines'], bridged_instructions=got['bridged'], agc_cs=got['agc_cs'],
               samples=got['samples'], samples_equal=got['equal'], base_samples_equal=base['equal'],
               gojam=got['gojams'], routine_mismatches=[f'{p} {t}' for p, t in got['mismatches']],
               new_mismatches=[f'{p} {t}' for p, t in got['mismatches'] if (p, t) not in base['mismatches']],
               new_holes=[h for h in got['holes'] if (h['point'], h['kind']) not in {(b['point'], b['kind']) for b in base['holes']}],
               restart_holes=[h for h in got['holes'] if h['file'] in RESTART_FILES],
               redoctr=[pre[sym['REDOCTR']], end[sym['REDOCTR']]],
               rsbbq=[f'{pre[5]:05o}', f"{end[sym['RSBBQ'] + 1]:05o}"],
               modreg=[f"{end_base[sym['MODREG']]:05o}", f"{end[sym['MODREG']]:05o}"],
               failreg=[[f'{end_base[a]:05o}' for a in failreg], [f'{end[a]:05o}' for a in failreg]],
               active_phases_before={k: f'{pre[a]:05o}' for k, a in phases.items() if pre[a] not in (0, 0o77777)},
               active_phases_end={k: f'{end[a]:05o}' for k, a in phases.items() if end[a] not in (0, 0o77777)},
               end_words_differing_from_base=sum(1 for i in range(0o20, 2048) if not 0o24 <= i <= 0o31 and end[i] != end_base[i]),
               gojam_snapshot=str(post.relative_to(ROOT)), pre_snapshot=str(pre_path.relative_to(ROOT)))
    bad = []
    if r.returncode or len(got['gojams']) != 1:
        bad.append(f"exit {r.returncode}, {len(got['gojams'])} GOJAMs")
    if sched and (got['agc_cs'] or 0) < cs:
        bad.append(f"stopped at {got['agc_cs']} cs")
    # A point a later path reaches that the run without the restart does not
    # (new_holes) is bridged by yaAGC itself: reported, not a failure.
    if row['new_mismatches'] or row['restart_holes']:
        bad.append('new mismatches, or restart points no routine holds')
    if row['redoctr'][1] != row['redoctr'][0] + 1:
        bad.append(f"REDOCTR {row['redoctr']}")
    if row['rsbbq'][0] != row['rsbbq'][1]:
        bad.append(f"RSBBQ+1 {row['rsbbq'][1]}, Z was {row['rsbbq'][0]}")
    if row['modreg'][0] != row['modreg'][1] or row['failreg'][0] != row['failreg'][1]:
        bad.append(f"major mode {row['modreg']} or alarms {row['failreg']} differ from the run without the restart")
    if not sched and got['samples'] - got['equal'] > base['samples'] - base['equal'] + SAMPLE_SLACK.get(name, 0):
        bad.append(f"{got['samples'] - got['equal']} samples differ")
    row['ok'] = not bad
    row['failures'] = bad
    return row


def c64_link(cc, m, header, tag, cs=None, defines=()):
    from resident_flight_build import COMMON, MISSION, resident_sources
    out = OUT / f'{tag}.prg'
    sources = resident_sources(m)
    if '-DRS_GOJAM_MODULE' in defines:
        # restart_check.c includes resident_flight.c, for rk_run_module.
        sources = [s for s in sources if s != 'platform/c64/resident_flight.c']
    subprocess.run([str(c) for c in [
        cc, *COMMON, *MISSION[m], f'-DRS_SNAP="{header}"', *([f'-DRS_CS={cs}u'] if cs else []), *defines,
        '-T', 'platform/c64/resident_flight.ld', f'-Wl,-Map,{out}.map',
        'platform/c64/restart_check.c', *sources, STORE / 'overlay_v2_services.S', '-o', out]],
        cwd=ROOT, check=True)
    return out


def c64_run(a, m, prg, tag, pinned):
    rom = a.rom_dir.resolve()
    work = OUT / f'vice-{tag}'
    work.mkdir(parents=True, exist_ok=True)
    sys_at = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
    mail, image = work / 'mailbox.bin', work / 'image.bin'
    mail.unlink(missing_ok=True)
    image.unlink(missing_ok=True)
    # Boot's DMA of the class-3 module to $C000 is the first store there.
    (work / 'vice.mon').write_text(f'load "{prg}" 0\nwatch store c000\n' + ('ignore 1 1\n' if pinned else '')
                                   + f'g {int(sys_at[1]):04x}\nbsave "{mail}" 0 c000 c0ff\n'
                                   f'bsave "{image}" 0 7000 7fff\nquit\n')
    reu = work / 'image.reu'
    reu.write_bytes((STORE / f'{m}.reu').read_bytes())
    env = dict(os.environ, XDG_CACHE_HOME=str(work / 'cache'), XDG_CONFIG_HOME=str(work / 'config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(env[key]).mkdir(parents=True, exist_ok=True)
    cmd = [a.vice_bin, '-default', '-console', '-pal', '-warp', '+sound',
           '-kernal', str(rom / 'kernal-901227-03.bin'), '-basic', str(rom / 'basic-901226-01.bin'),
           '-chargen', str(rom / 'chargen-901225-01.bin'), '-directory', f'{a.data_dir.resolve()}:{rom.parent}',
           '-initbreak', 'ready', '-moncommands', str(work / 'vice.mon'), '-monlogname', str(work / 'monitor.log'),
           '-monlog', '-limitcycles', str(a.cycles), '-reu', '-reusize', str(reu.stat().st_size // 1024), '-reuimage', str(reu)]
    with (work / 'vice.log').open('w') as log:
        subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, timeout=a.timeout, env=env)
    if not mail.exists() or mail.read_bytes()[0] != 0xa5:
        return dict(ok=False, error=f'no mailbox (cycle limit or crash; see {work}/vice.log)')
    d = mail.read_bytes()
    steps, start, end, z, fb, q, newjob, status, sb, isr, ie, idle = struct.unpack_from('<I6H5B', d, 0x10)
    words = list(struct.unpack('<2048H', image.read_bytes())) if image.exists() else None
    devices = list(struct.unpack_from(f'<{len(DEVICE_FIELDS)}H', d, 0x40))
    return dict(ok=True, boot_failed=d[5] == 1, failed=d[1], fail_class=d[5], devices=dict(zip(DEVICE_FIELDS, devices)),
                steps=steps, start_cs=start, end_cs=end, z=f'{z:05o}', fb=f'{fb:05o}',
                q=f'{q:05o}', newjob=f'{newjob:05o}', status=status, superbank=f'{sb:03o}', in_interrupt=isr,
                interrupt_enabled=ie, idle=bool(idle), words=words)


# platform/c64/restart_check.c rs_devices (RS_GOJAM_MODULE): rk_hardware in
# SNAP_HW's order, rk_flight in SNAP_FLIGHT's, then rk_device and the run.
HW_FIELDS = ('channel11 in_interrupt channel5 channel6 channel13 channel15 channel10 channel32 channel12 channel14 '
             'channel30 channel33 channel31 channel3 channel4').split()
FLIGHT_FIELDS = ('superbank interrupt_enabled channel16 channel34 channel35 trap31a trap31b trap32 executive_alarm '
                 'waitlist_alarm warning').split()
DEVICE_FIELDS = ([f'hw.{f}' for f in HW_FIELDS] + [f'flight.{f}' for f in FLIGHT_FIELDS]
                 + ['io.superbank', 'io.interrupt_enabled', 'run_status', 'env_status', 'rk_pending',
                    'loaded_job', 'loaded_task', 'fault'])


def snap_records(path):
    """A snapshot header's SNAP_IO, SNAP_HW and SNAP_FLIGHT fields, as rs_devices names them."""
    t = path.read_text()
    out = {}
    for macro, prefix in (('SNAP_IO', 'io'), ('SNAP_HW', 'hw'), ('SNAP_FLIGHT', 'flight')):
        body = re.search(rf'#define {macro}\((\w+)\) ([^\n]*)', t)
        for field, value in re.findall(r'\(\w+\)\.(\w+)=(\d+)u;', body[2]):
            out[f'{prefix}.{field}'] = int(value)
    return out


def c64_gojam(a, row, m):
    """The GOJAM on the C64 (flight-gojam-vice): from the -pre snapshot, the
    GOJAM module through rk_run_module; its image and device records must be
    the post-GOJAM snapshot's. Then, with apollo_ov2_restart_keep_pinned, the
    burst, against yaAGC from the post-GOJAM state."""
    pre, post = ROOT / row['pre_snapshot'], ROOT / row['gojam_snapshot']
    manifest = json.loads((STORE / f'{m}-manifest.json').read_text())
    pinned = any(x['slot'] == 3 for x in manifest['modules'])
    module = next((x for x in manifest['modules'] if x['id'] == GOJAM_MODULE), None)
    name = row['scenario']
    res = dict(module=None if module is None else dict(id=f'{GOJAM_MODULE:#06x}', slot=module['slot'], bytes=module['bytes']))
    if module is None:
        res['ok'] = False
        return res
    tag = f'{name}-gojam-module'
    g = c64_run(a, m, c64_link(a.cc, m, pre, tag, defines=('-DRS_GOJAM_MODULE', '-DRS_GOJAM_ONLY')), tag, pinned)
    if not g['ok'] or g['boot_failed'] or g['failed']:
        res.update(gojam={k: v for k, v in g.items() if k != 'words'}, ok=False)
        return res
    want_words, want = snap_words(post), snap_records(post)
    dev = g['devices']
    word_diff = [i for i in range(2048) if g['words'][i] != want_words[i]]
    field_diff = {k: [dev[k], v] for k, v in want.items() if k in dev and dev[k] != v}
    res['gojam'] = dict(run_status=dev['run_status'], env_status=dev['env_status'], q=f'{g["words"][2]:05o}',
                        z=f'{g["words"][5]:05o}', words_equal=2048 - len(word_diff),
                        words_differing=[f'{i:04o} {g["words"][i]:05o}/{want_words[i]:05o}' for i in word_diff[:16]],
                        fields_compared=sum(1 for k in want if k in dev), fields_differing=field_diff)
    tag = f'{name}-gojam-burst'
    b = c64_run(a, m, c64_link(a.cc, m, pre, tag, defines=('-DRS_GOJAM_MODULE',)), tag, pinned)
    res['burst'] = {k: v for k, v in b.items() if k != 'words'}
    if not b['ok'] or b['boot_failed'] or b['fail_class'] == 3:
        res['ok'] = False
        return res
    yw, r = reference(a, m, name, post, b)
    diff = [i for i in range(2048) if b['words'][i] != yw[i]]
    res['burst'].update(reference=r.stdout.strip(), reference_arrived=r.returncode == 0,
                        words_differing=[f'{i:04o} {b["words"][i]:05o}/{yw[i]:05o}' for i in diff[:16]],
                        words_equal=2048 - len(diff))
    bd = b['devices']
    res['ok'] = (not word_diff and not field_diff and r.returncode == 0 and not diff
                 and bd['loaded_job'] == 0 and bd['loaded_task'] == 0 and bd['fault'] == 0)
    return res


def reference(a, m, name, header, burst):
    """yaAGC from header, its clock frozen, to where the C64 burst stopped."""
    ref = OUT / f'flight-restart-reference-{name}'
    y = fhd.YAAGC
    subprocess.run([a.host_cc, '-std=c99', '-O1', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', f'-I{y}',
                    f'-DRK_{m.upper()}=1', f'-DRS_SNAP="{header}"', 'tools/flight_restart_reference.c',
                    str(fhd.OUT / 'agc_engine_scaled.o'), str(y / 'agc_engine_init.o'), str(y / 'rfopen.o'), '-lm',
                    '-o', str(ref)], cwd=ROOT, check=True)
    stop = [] if burst['idle'] else [burst['z'], burst['fb'], burst['superbank']]
    r = subprocess.run([str(ref), fhd.ROPE[m], str(OUT / f'{name}-reference.bin'), *stop], cwd=ROOT, capture_output=True, text=True)
    return list(struct.unpack('<2048H', (OUT / f'{name}-reference.bin').read_bytes())), r


def c64(a, row, m):
    header = ROOT / row['gojam_snapshot']
    manifest = json.loads((STORE / f'{m}-manifest.json').read_text())
    pinned = any(x['slot'] == 3 for x in manifest['modules'])
    name = row['scenario']
    # The burst: interrupts off, to the next idle pass through ADVAN.
    burst = c64_run(a, m, c64_link(a.cc, m, header, f'{name}-burst'), f'{name}-burst', pinned)
    res = dict(burst={k: v for k, v in burst.items() if k != 'words'})
    if not burst['ok'] or burst['boot_failed']:
        res['ok'] = False
        return res
    yw, r = reference(a, m, name, header, burst)
    # A, L and Q at 16 bits on both sides.
    diff = [i for i in range(2048) if burst['words'][i] != yw[i]]
    res['burst'].update(reference=r.stdout.strip(), reference_arrived=r.returncode == 0,
                        words_differing=[f'{i:04o} {burst["words"][i]:05o}/{yw[i]:05o}' for i in diff[:16]],
                        words_equal=2048 - len(diff))
    # The resume: interrupts on.
    resume = c64_run(a, m, c64_link(a.cc, m, header, f'{name}-resume', a.resume_cs), f'{name}-resume', pinned)
    res['resume'] = {k: v for k, v in resume.items() if k != 'words'}
    res['ok'] = r.returncode == 0 and not diff
    return res


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-index', action='store_true', help='reuse build/flight-store/<m>-host.inc and host objects')
    p.add_argument('--only', help='comma-separated scenario names')
    p.add_argument('--vice', dest='run_vice', action='store_true', help='also run the restart on the C64 store in VICE')
    p.add_argument('--resume-cs', type=int, default=200, help='AGC centiseconds the C64 resume runs')
    p.add_argument('--gojam-module', action='store_true',
                   help='with --vice: apply the GOJAM on the C64 (the GOJAM module) from the pre-GOJAM state')
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
    words = {m: flight_holes.listing(flight_holes.MISSION[m]) for m in exe}
    lanes = flight_holes.lanes()
    only = set(a.only.split(',')) if a.only else None
    summary, failed = [], 0
    for scenario in RESTARTS:
        if only and scenario[0] not in only:
            continue
        row = host(exe, scenario, words, lanes)
        print(f"{row['scenario']}: GOJAM at {row['gojam'][0][0] if row['gojam'] else '?'} cs, "
              f"{row['native_routines']} native routines, {len(row['routine_mismatches'])} yaAGC did not match "
              f"({len(row['new_mismatches'])} new), REDOCTR {row['redoctr'][0]}->{row['redoctr'][1]}, "
              f"phases {row['active_phases_before']}, major mode {row['modreg'][1]} (without the restart {row['modreg'][0]})"
              + (f", {row['samples_equal']}/{row['samples']} samples equal ({row['base_samples_equal']} without the restart)"
                 if not row['schedule'] else '')
              + ('' if row['ok'] else ' FAILED: ' + '; '.join(row['failures'])))
        if a.run_vice and scenario[7] and a.gojam_module:
            row['c64_gojam'] = c64_gojam(a, row, scenario[1])
            g, b = row['c64_gojam'].get('gojam', {}), row['c64_gojam'].get('burst', {})
            print(f"  C64 GOJAM module {row['c64_gojam'].get('module')}: run status {g.get('run_status')}, Q {g.get('q')} "
                  f"Z {g.get('z')}; {g.get('words_equal')}/2048 words and "
                  f"{g.get('fields_compared', 0) - len(g.get('fields_differing') or {})}/{g.get('fields_compared')} "
                  f"device fields equal the post-GOJAM snapshot" + (f" {g['words_differing']}" if g.get('words_differing') else '')
                  + (f" {g['fields_differing']}" if g.get('fields_differing') else ''))
            print(f"  C64 burst after apollo_ov2_restart_keep_pinned: {b.get('steps')} steps, "
                  f"{'idle at ADVAN' if b.get('idle') else 'stopped'} Z {b.get('z')} FB {b.get('fb')} status {b.get('status')}; "
                  f"yaAGC {b.get('reference')}; {b.get('words_equal')}/2048 words equal"
                  + (f" {b['words_differing']}" if b.get('words_differing') else ''))
            row['ok'] = row['ok'] and row['c64_gojam']['ok']
        elif a.run_vice and scenario[7]:
            row['c64'] = c64(a, row, scenario[1])
            b = row['c64'].get('burst', {})
            rs = row['c64'].get('resume', {})
            print(f"  C64 burst: {b.get('steps')} steps, {'idle at ADVAN' if b.get('idle') else 'stopped'} Z {b.get('z')} "
                  f"FB {b.get('fb')} status {b.get('status')}; yaAGC {b.get('reference')}; {b.get('words_equal')}/2048 words equal"
                  + (f" {b['words_differing']}" if b.get('words_differing') else ''))
            print(f"  C64 resume: {rs.get('steps')} steps over {(rs.get('end_cs', 0) - rs.get('start_cs', 0)) & 0x3fff} cs, "
                  f"status {rs.get('status')} at Z {rs.get('z')} FB {rs.get('fb')}")
            row['ok'] = row['ok'] and row['c64']['ok']
        failed += not row['ok']
        summary.append(row)
    (OUT / ('summary-gojam-module.json' if a.gojam_module else 'summary.json')).write_text(json.dumps(summary, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
