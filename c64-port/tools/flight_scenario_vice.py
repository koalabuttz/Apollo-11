#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""A flight scenario on the C64 (docs/FLIGHT-STORE.md, Flight scenarios):
the host flight run (tools/flight_host_run.c, scaled time x1 with the
scenario's inputs and keys) writes the native state at SNAP_CS
(FLIGHT_SNAPSHOT); platform/c64/dap_pass_check.c boots the LM flight
resident on the whole-program flight store in VICE, loads that state and
runs resident steps with CIA1 interrupts on for --cs centiseconds of AGC
time, recording cycles, steps and module loads per AGC interrupt (as
tools/dap_pass_vice.py does for the DAP). The C64 run has no engine model:
it continues from the snapshot as the AGC would with the PIPAs still. With
--keys it presses the scenario's DSKY keys that fall in the window, at the
times the host run presses them; without, it presses none. Other scheduled
inputs (FLIGHT_CHAN, FLIGHT_POKE/LOAD, the radar model) are not replayed.
With --replay the C64 instead gets every input the host run gives the AGC
in the window (tools/flight_host_run.c FLIGHT_REPLAY: model PIPA counts,
keys, channels, pokes, radar), and the run fails unless the C64 ends the
window in the host's state (erasable words and output channels; the first
difference is printed).

First lists the points the host run to the end of the C64 window bridges
to yaAGC (unregistered, or refused by their table), with file, label and
owning lane (tools/flight_holes.py): each is a stop on the C64.
--stop-on-holes fails there instead of running VICE.

Prints wall cycles per AGC second, the AGC time reached, the step status
(a refused or untranslated point stops the run: there is no yaAGC to
bridge to on the C64) and per-interrupt costs; writes
build/flight-store/flight-scenario-vice.json.

  flight_scenario_vice.py [--scenario lm-p63-descent-thrust] [--snap-cs CS] [--cs N] [--stop-on-holes] [--profile]
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
import dap_pass_vice as dp  # noqa: E402
import flight_holes  # noqa: E402
import flight_host_differential as fhd  # noqa: E402
import reu_layout  # noqa: E402


def snapshot(exe, name, cs, tag, replay=0, every=0, chain=1):
    """replay: the window's centiseconds; the host run then goes on through
    the window and writes its inputs (-snap.h.replay) and its end state
    (-snap.h.end, and every `every` cs before), tools/flight_host_run.c
    FLIGHT_REPLAY. chain: that many windows back to back, the later ones'
    snapshots in -snap.h-1, -snap.h-2, ... (with .replay, .end and .seam)."""
    scen = {s[0]: s for s in fhd.FLIGHT}[name]
    _, m, _, keys, env = scen
    header = dp.WORK / f'{tag}-snap.h'
    header.unlink(missing_ok=True)
    for n in range(chain):
        for x in ('', '.replay', '.end', '.seam'):
            if n or x:
                pathlib.Path(f'{header}{f"-{n}" if n else ""}{x}').unlink(missing_ok=True)
    # The host takes the snapshot at the first point at or after cs with no
    # interrupt pending (a long job, such as PICAPAR, can hold one off for
    # over 100 cs), then stops; running to cs + 1000 leaves room for that.
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs + 1000 * chain + replay * chain), keys], cwd=ROOT,
                       env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SNAPSHOT': f'{cs},{header}',
                            **({'FLIGHT_REPLAY': f'{replay},0,{every},{chain}'}
                               if replay else {})},
                       capture_output=True, text=True)
    (dp.WORK / f'{tag}-host.log').write_text(r.stdout + r.stderr)
    if not header.exists():
        sys.exit(f'FAIL: no snapshot for {name} (see {dp.WORK}/{tag}-host.log)')
    if replay and (r.returncode or not pathlib.Path(f'{header}{f"-{chain - 1}" if chain > 1 else ""}.end').exists()):
        sys.exit(f'FAIL: the host run did not reach the end of the {replay} cs window (see {dp.WORK}/{tag}-host.log)')
    return header


# The output channels after the erasable words in the end state
# (tools/flight_host_run.c rp_end, platform/c64/dap_pass_check.c replay_end).
END_CHANNELS = ['channel 5', 'channel 6', 'channel 11', 'channel 12', 'channel 13', 'channel 14', 'channel 34',
                'channel 35', 'superbank']
ROW = struct.Struct('<HBBHH')
# A snapshot more than this far past --snap-cs is reported as slid.
SLIDE_CS = 100


def compared(i):
    """tools/flight_host_run.c compared(): not the central registers, RUPTREG1-4
    or the clock counters (024-031, the work clock's scalers)."""
    return i >= 0o20 and not 0o70 <= i <= 0o73 and not 0o24 <= i <= 0o31


def replay_place(manifest, m, image, size):
    """An REU address for size bytes of replay rows and end state: the top
    of the zeros between the store and the next declared region (the
    Pinball trampoline save below the rope, tools/reu_layout.py), which the
    run must not touch."""
    top = reu_layout.free_top(m, manifest)
    at = (top - size) & ~0xff
    if at < 0 or any(image[at:top]):
        sys.exit(f'FAIL: no {size} zero bytes below the {m} rope at 0x{top:x} for the replay rows')
    return at


STATE = 2 * (1 + 2048 + len(END_CHANNELS))
# The C64's devices (platform/c64/resident_flight.h): rk_hardware
# (agc_hardware, include/agc_transfer.h) at RK_HARDWARE and rk_flight
# (flight_io, include/flight_io.h) at RK_FLIGHT_IO, byte offsets by field;
# rk_device is the mission's rk_io (LM p66_bias_io, CM cm_native_io).
RK_HARDWARE, RK_FLIGHT_IO, RK_RESTART_LAMP = 0x6fbe, 0x6fdd, 0x6ff5
HW = {0o11: 0, 0o5: 4, 0o6: 6, 0o13: 8, 0o15: 10, 0o10: 12, 0o32: 14, 0o12: 16, 0o14: 18, 0o30: 20, 0o33: 22,
      0o31: 24, 0o3: 26, 0o4: 28}
RADAR_GATE = 30
FLIGHT = {'superbank': 0, 0o16: 4, 0o34: 6, 0o35: 8}
DEVICE = {'lm': {'superbank': 0}, 'cm': {0o11: 0, 0o12: 2, 0o13: 4, 0o14: 6, 0o15: 8, 0o16: 10, 'superbank': 12}}
C64_STATE = 2 + 2 * 2048 + (RK_RESTART_LAMP - RK_HARDWARE) + 32


def map_symbol(prg, name):
    """A symbol's address in a link's lld map."""
    for line in pathlib.Path(f'{prg}.map').read_text().splitlines():
        p = line.split()
        if len(p) == 5 and p[4] == name:
            return int(p[0], 16)
    sys.exit(f'FAIL: no {name} in {prg}.map')


IMAGE = 0x7000   # the canonical image (APOLLO_SHARED_ADDRESS, resident_flight.ld)


def c64_rows(rows, m, device, pending):
    """The host's rows for the C64 (dap_pass_check.c DP_REPLAY), each with
    a C64 address: an erasable word (kind 0) becomes a store (2) into the
    image, a PIPA count (1) stays a count, a channel (2) a store to each C64
    copy of it, the radar gate a byte store (4), and interrupt requests (3)
    an OR into rk_pending."""
    out = []
    for cs, kind, where, addr, value in ROW.iter_unpack(rows):
        if kind in (0, 1):
            out.append((cs, 2 if kind == 0 else 1, where, IMAGE + 2 * addr, value))
        elif kind == 3:
            out.append((cs, 3, where, pending, value))
        elif addr == 0o377:
            out.append((cs, 4, where, RK_HARDWARE + RADAR_GATE, value))
        else:
            at = [RK_HARDWARE + HW[addr]] if addr in HW else [RK_FLIGHT_IO + FLIGHT[addr]] if addr in FLIGHT else []
            at += [device + DEVICE[m][addr]] if addr in DEVICE[m] else []
            if not at:
                sys.exit(f'FAIL: no C64 copy of channel {addr:o} for the replay')
            out += [(cs, 2, where, x, value) for x in at]
    return b''.join(ROW.pack(*x) for x in out)


def c64_states(data, n, m):
    """The C64's states (dap_pass_check.c replay_state) in the host's
    layout: (centiseconds, words and END_CHANNELS)."""
    out = []
    for i in range(n):
        b = data[i * C64_STATE:(i + 1) * C64_STATE]
        cs, words = struct.unpack_from('<H', b)[0], struct.unpack_from('<2048H', b, 2)
        io, dev = 2 + 4096, 2 + 4096 + RK_RESTART_LAMP - RK_HARDWARE
        word = lambda at: struct.unpack_from('<H', b, at)[0]  # noqa: E731
        # On the CM, rk_device (cm_native_io) holds channels 11-14: the base
        # tables run on it, and rk_hardware's copies lag (written when a
        # module of another io kind leaves).
        ch = [word(dev + DEVICE[m][c]) if c in DEVICE[m] else word(io + HW[c]) for c in (0o5, 0o6, 0o11, 0o12, 0o13, 0o14)]
        ch += [word(io + RK_FLIGHT_IO - RK_HARDWARE + FLIGHT[c]) for c in (0o34, 0o35)]
        ch.append(word(dev + DEVICE[m]['superbank']))
        out.append((cs, (*words, *ch)))
    return out


def states(data, n):
    """The first n states of an end-state file: (centiseconds, words)."""
    k = STATE // 2 - 1
    return [(lambda v: (v[0], v[1:]))(struct.unpack_from(f'<{k + 1}H', data, i * STATE)) for i in range(n)]


# TEMPORARY (steward, 10-18): on the CM the C64 charges the self-check idle
# pass a job ends into as module work, where the host charges it as idle
# (the rest of the period), so the C64's schedule can sit a tick off the
# host's (RK: FS_IDLE honouring on the CM idle+interbank set). Until that
# lands, CM windows skip what a tick moves: DSPCNT and DSRUPTSW (T4RUPT's
# display cycle), DSPTAB's sent bit (a word and its complement are the same
# digits, sent or not) and a checkpoint one centisecond off (not compared
# at all when its words differ, unless it ends the window). Every result
# that skips anything says so. Remove with the RK fix.
SCHEDULE_SKIP = {'cm': dict(words={0o776: 'DSPCNT', 0o1302: 'DSRUPTSW'}, sent=range(0o1023, 0o1037), cs=1)}


TIMERS = {0o24: 'TIME2', 0o25: 'TIME1', 0o26: 'TIME3', 0o27: 'TIME4', 0o30: 'TIME5', 0o31: 'TIME6'}
# TEMPORARY: at a sample point in the same centisecond the host and the C64
# can sit in different thirds of it (the work clock steps TIME4, TIME1/TIME3
# and TIME5 in turn), so TIME5 can be a count apart while TIME1 agrees: the
# jobs before it took a different work time (the interpretive charge,
# Interpreter #650; the CM idle charge, RK #663). Skipped, and said so,
# until those land.
TIME5_PHASE = 'TIME5 a count apart (a third of a centisecond)'


def replay_compare(h, c, skip=None, skipped=None, apart=False):
    """The differences between a host state and the C64's: (name, host,
    C64) for each compared erasable word, the timers, then each output
    channel. apart: the states are a skipped centisecond apart, so the
    timers are not compared."""
    out = []
    for i, name in TIMERS.items():
        if apart or h[i] == c[i]:
            continue
        if i == 0o30 and h[0o25] == c[0o25] and (h[i] - c[i]) % 0o40000 in (1, 0o37777):
            skipped.add(TIME5_PHASE)
        else:
            out.append((name, h[i], c[i]))
    for i in range(2048):
        if not compared(i) or h[i] == c[i] or (0o37 <= i <= 0o41 and {h[i], c[i]} == {0, 0o77777}):
            continue  # a PIPA's -0 and +0 are the same count
        if skip and i in skip['words']:
            skipped.add(skip['words'][i])
        elif skip and i in skip['sent'] and h[i] == (~c[i] & 0o77777):
            skipped.add('DSPTAB sent bits')
        else:
            out.append((f'{i:04o}', h[i], c[i]))
    return out + [(END_CHANNELS[k], h[2048 + k], c[2048 + k]) for k in range(len(END_CHANNELS)) if h[2048 + k] != c[2048 + k]]


# tools/flight_host_run.c's key schedule (keys_parse, keycode, key_due).
KEY_AT, KEY_GAP = 200, 50
KEYCODE = {'V': 0o21, 'N': 0o37, 'E': 0o34, '0': 0o20, '+': 0o32, '-': 0o33, 'C': 0o36, 'P': 0o22, 'K': 0o31,
           'R': 0o22, 'M': 0o400 | 0o40, 'J': 0o400 | 0o100, 'x': 0o400 | 0o4, 'y': 0o400 | 0o10,
           'j': 0o400 | 0o20}


def key_schedule(keys):
    """(centisecond, code) for each key in a flight_host_run key string; a
    mark key's code is 0400 | its channel 16 bits."""
    out, at, i = [], KEY_AT, 0
    while i < len(keys):
        if keys[i] == '@':
            j = i + 1
            while j < len(keys) and keys[j].isdigit():
                j += 1
            at = max(at, int(keys[i + 1:j]))
            i = j
            continue
        c = keys[i]
        out.append((at, KEYCODE.get(c, ord(c) - ord('0'))))
        at += KEY_GAP
        i += 1
    return out


def window_keys(name, scen_keys, snap_at, cs):
    """The DP_KEYS header for the keys the host run had not pressed by its
    snapshot (at snap_at cs) and that fall in the cs-long window, timed from
    the window's start."""
    keys = [(t - snap_at, k) for t, k in key_schedule(scen_keys) if snap_at < t < snap_at + cs]
    header = dp.WORK / f'{name}-keys.h'
    header.write_text(f'#define DP_NKEYS {len(keys)}u\n'
                      f"static const uint16_t dp_key_cs[{max(len(keys), 1)}]={{{','.join(str(t) for t, _ in keys) or '0'}}};\n"
                      f"static const uint16_t dp_key_code[{max(len(keys), 1)}]={{{','.join(f'0{k:o}' for _, k in keys) or '0'}}};\n")
    return header, keys


def host_holes(exe, name, cs, tag):
    """The points the host run to cs bridges, as flight_holes.describe dicts."""
    _, m, _, keys, env = {s[0]: s for s in fhd.FLIGHT}[name]
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs), keys], cwd=ROOT,
                       env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2'}, capture_output=True, text=True)
    (dp.WORK / f'{tag}-holes.log').write_text(r.stdout + r.stderr)
    words, owners = flight_holes.listing(flight_holes.MISSION[m]), flight_holes.lanes()
    return [flight_holes.describe(h, words, owners, m) for h in flight_holes.HOLE.finditer(r.stdout)]


def host_window_cs(header):
    """The centiseconds the host ran a replay window: its last state's."""
    data = pathlib.Path(f'{header}.end').read_bytes()
    return states(data, len(data) // STATE)[-1][0]


def snap_state(header):
    """A snapshot header's state in the end-state layout (words, then
    END_CHANNELS; tools/flight_host_run.c snapshot)."""
    text = pathlib.Path(header).read_text()
    words = [int(v) for v in re.search(r'snap_words\[2048\][^{]*\{([^}]*)\}', text)[1].replace('u', '').split(',')]
    field = lambda name: int(re.search(r'\(\w\)\.' + name + r'=(\d+)u', text)[1])  # noqa: E731
    return (*words, *[field(f'channel{c}') for c in (5, 6, 11, 12, 13, 14, 34, 35)], field('superbank'))


def ones_add(w, d):
    """A 15-bit ones'-complement counter word moved by a signed count."""
    v = (w - 0o77777 if w & 0o40000 else w) + (d - 0x10000 if d & 0x8000 else d)
    return v & 0o77777 if v >= 0 else (v + 0o77777) & 0o77777


def seam_check(header, c64_end, m):
    """--chain: the differences between a window's start (its snapshot)
    and the C64's end of the window before plus the inputs the host gave
    at that point (.seam rows), as replay_compare reports them."""
    if c64_end is None:
        return [('C64 end state', 0, 0)]
    w = list(c64_end)
    chans = {0o5: 0, 0o6: 1, 0o11: 2, 0o12: 3, 0o13: 4, 0o14: 5, 0o34: 6, 0o35: 7}
    for _, kind, _, addr, value in ROW.iter_unpack(pathlib.Path(f'{header}.seam').read_bytes()):
        if kind == 0:
            w[addr] = value
        elif kind == 1:
            w[addr] = ones_add(w[addr], value)
        elif kind == 2 and addr in chans:
            w[2048 + chans[addr]] = value
    return replay_compare(snap_state(header), w, None, set())


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--scenario', default='lm-p63-descent-thrust')
    p.add_argument('--snap-cs', type=int, default=17500, help='AGC centisecond the C64 run starts from')
    p.add_argument('--cs', type=int, default=200, help='AGC centiseconds to run on the C64')
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--reu-check-drop', default='', metavar='REGION',
                   help="leave a declared REU region out of the bound check (e.g. scratch): the check's own test")
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--cycles', type=int, default=6_000_000_000)
    p.add_argument('--timeout', type=int, default=3600)
    p.add_argument('--stop-on-holes', action='store_true', help='fail before VICE if the host run bridges any point')
    p.add_argument('--profile', action='store_true', help='cycles per resident step segment (RK_STEP_PROFILE)')
    p.add_argument('--keys', action='store_true',
                   help="press the scenario's DSKY keys that fall in the window on the C64, as the host run does")
    p.add_argument('--replay', action='store_true',
                   help="replay the host run's inputs in the window (PIPA counts from its engine and aero models, "
                   'keys, channel changes, pokes) on the C64, and fail unless the C64 ends the window in the '
                   "host's state (erasable and output channels)")
    p.add_argument('--replay-exact', action='store_true',
                   help='with --replay on the CM, compare without the temporary schedule skip (SCHEDULE_SKIP)')
    p.add_argument('--replay-every', type=int, default=0,
                   help='with --replay, also compare at the first sample point at or after every N cs, '
                   'to find when the C64 leaves the host')
    p.add_argument('--chain', type=int, default=1,
                   help='with --replay, run that many --cs windows back to back on the host, then each on the C64 from '
                   "the host's state where the one before ended, and fail unless the C64 ends each window in the "
                   "host's state and that state is the next window's start (the seam)")
    p.add_argument('--strict-start', action='store_true',
                   help=f'fail when the snapshot slides more than {SLIDE_CS} cs past --snap-cs')
    p.add_argument('--watch', default='', help='up to five erasable words (octal addresses, comma separated) '
                   'printed as the window ends, e.g. 1011 for Comanche MODREG')
    a = p.parse_args()
    dp.WORK.mkdir(parents=True, exist_ok=True)
    m = {s[0]: s for s in fhd.FLIGHT}[a.scenario][1]
    # Each window's files (snapshot, PRG, VICE directory, result) carry its
    # start, so windows of one scenario can run side by side.
    tag = f'{a.scenario}-{a.snap_cs}'
    subprocess.run([sys.executable, 'tools/flight_store_build.py', '--missions', m, '--host-cc', a.host_cc],
                   cwd=ROOT, check=True)
    subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--missions', m,
                    '--host-cc', a.host_cc], cwd=ROOT, check=True)
    exe = fhd.build(a.host_cc, m, fhd.frozen_engine(a.host_cc))
    manifest = json.loads((dp.OUT / f'{m}-manifest.json').read_text())
    pinned = any(mod['slot'] == 3 for mod in manifest['modules'])
    reu = (dp.OUT / f'{m}.reu').read_bytes()
    scratch = len(reu) - dp.SCRATCH_BYTES
    if any(reu[scratch:]):
        sys.exit(f'FAIL: no unused {dp.SCRATCH_BYTES} bytes at the end of {m}.reu')
    holes = host_holes(exe, a.scenario, a.snap_cs + a.cs * a.chain, tag)
    # A core-start point (tools/flight_holes.py CORE_START) is bridged only
    # because the scenario's core starts mid-way through code, before any
    # C64 window: not a stop on the C64.
    stops = [d for d in holes if not d['core_start']]
    print(f'{a.scenario}: the host run to {a.snap_cs + a.cs * a.chain} cs bridges {len(stops)} points, each a stop on the C64'
          + (f' (and {len(holes) - len(stops)} core-start points, not stops)' if len(stops) < len(holes) else ''))
    for d in holes:
        print(f"  {d['kind']:6} {d['point']} {d['label']:14} {d['file']:40} lane {d['lane']:3} {d['arrivals']:>5} arrivals"
              + (f"  refused by {d['refused_by']}" if d['refused_by'] else '')
              + ('  core-start artifact, not a C64 stop' if d['core_start'] else ''))
    if stops and a.stop_on_holes:
        (dp.OUT / 'flight-scenario-vice.json').write_text(json.dumps(dict(ok=False, host_holes=holes), indent=1) + '\n')
        return 1
    if a.chain > 1 and not a.replay:
        sys.exit('--chain needs --replay')
    if a.replay and a.keys:
        sys.exit('--replay replays the keys itself; leave out --keys')
    every = a.replay_every if 0 < a.replay_every < a.cs else a.cs
    header = snapshot(exe, a.scenario, a.snap_cs, tag, a.cs if a.replay else 0, every, a.chain)
    # The snapshot waits for a point with no interrupt pending, so inside a
    # long interpretive job it can start well after --snap-cs (CM 9000 ->
    # 9952), and the window then covers other code than asked for.
    snap_at = int(re.search(r'^snapshot at (\d+) cs', (dp.WORK / f'{tag}-host.log').read_text(), re.M)[1])
    slid = snap_at - a.snap_cs
    print(f'{a.scenario}: snapshot at {snap_at} cs (asked {a.snap_cs}, '
          + (f'slid {slid} cs: the window starts late)' if slid > SLIDE_CS else f'+{slid} cs)'))
    if slid > SLIDE_CS and a.strict_start:
        sys.exit(f'FAIL: {a.scenario}: the snapshot slid {slid} cs past {a.snap_cs} (--strict-start)')
    # --chain: the windows the host run wrote (header-1, header-2, ...),
    # each as long as the host ran it (to its first sample point with no
    # interrupt pending at or after --cs).
    windows = [(header, tag, a.cs)]
    for n in range(1, a.chain):
        windows.append((pathlib.Path(f'{header}-{n}'), f'{tag}-{n}', a.cs))
    if a.chain > 1:
        windows = [(h, t, host_window_cs(h)) for h, t, _ in windows]
    results, c64_end = [], None
    for n, (header, tag, cs) in enumerate(windows):
        if n:
            seam = seam_check(header, c64_end, m)
            print(f'{a.scenario}: seam {n}: ' + (f'the C64 end of window {n - 1} is the start of window {n}' if not seam else
                  f'FAIL: the C64 end of window {n - 1} is not the start of window {n}: {len(seam)} differences, first '
                  + ', '.join(f'{x} {h:05o}/{c:05o}' for x, h, c in seam[:6]) + ' (start/C64 end)'))
            if seam:
                results.append(dict(ok=False, seam=n))
                break
        keys, defines, patch, end_at = None, [], [], None
        if a.replay:
            rows = pathlib.Path(f'{header}.replay').read_bytes()
            host_end = pathlib.Path(f'{header}.end').read_bytes()
            n_states = len(host_end) // STATE
            end_bytes = n_states * C64_STATE
            # CM channel rows can take two C64 rows each.
            at = replay_place(manifest, m, reu, 2 * len(rows) + end_bytes + 256)
            end_at = (at + 2 * len(rows) + 0xff) & ~0xff
            busy = {s[0]: s for s in fhd.FLIGHT}[a.scenario][4].get('FLIGHT_BUSY_SAMPLE')
            # The window ends at the C64's first sample point cs in; DP_CS
            # stops it later if none comes.
            # The C64's rows (their count does not depend on rk_device's address).
            n_c64 = len(c64_rows(rows, m, 0, 0)) // ROW.size
            defines = ['-DDP_REPLAY', f'-DDP_REPLAY_REU=0x{at:x}ul', f'-DDP_REPLAY_ROWS={n_c64}u',
                       f'-DDP_REPLAY_CS={cs}u', f'-DDP_EVERY={every}u', f'-DDP_EVERY_UNTIL={a.cs}u', f'-DDP_END=0x{end_at:x}ul', *([f'-DDP_BUSY={int(busy)}u'] if busy else [])]
            kinds = {}
            for row in ROW.iter_unpack(rows):
                kinds[row[1]] = kinds.get(row[1], 0) + 1
            print(f'{a.scenario}: {len(rows) // ROW.size} host input rows ({n_c64} on the C64) in the window at REU 0x{at:x} '
                  f"({kinds.get(1, 0)} PIPA counts, {kinds.get(0, 0)} erasable words, {kinds.get(2, 0)} channels, "
                  f'{kinds.get(3, 0)} interrupt requests)')
        if a.keys:
            keys, pressed = window_keys(tag, {s[0]: s for s in fhd.FLIGHT}[a.scenario][3], snap_at, cs)
            print(f'{a.scenario}: {len(pressed)} keys pressed on the C64 at '
                  + ' '.join(f'+{t}:{k:o}' for t, k in pressed))
        prg = dp.link(a.cc, tag, header, cs + 500 if a.replay else cs, scratch, os.environ.get('FSV_STEPS') == '1',
                      m, a.profile, keys, [w for w in a.watch.split(',') if w][:5], defines)
        if a.replay:
            # rk_device's address comes from the link.
            patch = [(at, c64_rows(rows, m, map_symbol(prg, 'rk_device'), map_symbol(prg, 'rk_pending')))]
        # The run's declared REU regions (tools/reu_layout.py): VICE logs every
        # DMA, read or write, that is not inside one of them, and afterwards every
        # changed byte must lie in a writable one (not the store, the rope or the
        # replay rows). --reu-check-drop leaves a region out (the check's own test).
        # Each chain window has its own rows, so its own regions.
        regions = [x for x in reu_layout.layout(
            m, manifest, (at, end_at, reu_layout.free_top(m, manifest)) if a.replay else None)
            if x[0] != a.reu_check_drop]
        r = dp.vice(a, tag, prg, pinned, m, patch, [reu_layout.dma_trace(regions)])
        after = dp.WORK / f'vice-{tag}' / 'image.reu'
        if r.get('status') is not None and after.exists():
            before = bytearray(reu)
            for at_, data in patch:
                before[at_:at_ + len(data)] = data
            bounds = reu_layout.report(m, regions, before, after.read_bytes(), a.scenario)
            bounds['dma_outside'] = reu_layout.dma_misses((dp.WORK / f'vice-{tag}' / 'monitor.log').read_text())
            if bounds['dma_outside']:
                bounds['ok'] = False
                print(f"FAIL: {a.scenario}: {bounds['dma_outside']} DMAs outside the {m} REU layout")
            else:
                print(f'{a.scenario}: every DMA inside the {m} REU layout')
            r['reu_bounds'] = bounds
            if not bounds['ok']:
                r['ok'] = False
        if a.replay and r.get('status') is not None:
            mail = (dp.WORK / f'vice-{tag}' / 'results.bin').read_bytes()
            applied, end_cs, ended, c64_written = struct.unpack_from('<HHBB', mail, 0x30)
            r.update(replay_rows=n_c64, replay_applied=applied, replay_end_cs=end_cs, replay_ended=bool(ended))
            # Every state the C64 wrote against the host's at the same point.
            image = (dp.WORK / f'vice-{tag}' / 'image.reu').read_bytes()
            host_s, c64_s = states(host_end, n_states), c64_states(image[end_at:], min(c64_written, n_states), m)
            c64_end = c64_s[-1][1] if c64_s else None
            diff, at_cs, matched, skipped = [], None, [], set()
            skip = None if a.replay_exact else SCHEDULE_SKIP.get(m)
            pairs = list(zip(host_s, c64_s))
            for k, ((hc, hw), (cc, cw)) in enumerate(pairs):
                apart = hc != cc and skip and abs(hc - cc) <= skip['cs']
                diff = replay_compare(hw, cw, skip, skipped, apart)
                if apart:
                    # A tick apart, an interrupt's temporaries can differ
                    # too; such a checkpoint is skipped, the window's end is not.
                    if diff and k < len(pairs) - 1:
                        skipped.add(f'checkpoint at {hc} cs a centisecond apart ({len(diff)} words)')
                        continue
                    skipped.add('checkpoints a centisecond apart')
                elif hc != cc:
                    diff.insert(0, ('centiseconds', hc, cc))
                if diff:
                    at_cs = hc
                    break
                matched.append(hc)
            if skipped:
                print(f"{a.scenario}: skipped (temporary, SCHEDULE_SKIP / TIME5_PHASE): {', '.join(sorted(skipped))}")
            r.update(replay_matched_cs=matched, replay_differences=len(diff), replay_differ_at_cs=at_cs,
                     replay_skipped=sorted(skipped),
                     replay_first=[dict(at=x, host=f'{h:05o}', c64=f'{c:05o}') for x, h, c in diff[:32]])
            seen = (f"matches the host{' (with skips)' if skipped else ''} at {', '.join(map(str, matched))} cs" if matched
                    else 'matches the host at no point')
            if diff:
                r['ok'] = False
                print(f'FAIL: {a.scenario}: the C64 {seen}, then leaves it by {at_cs} cs into the window '
                      f'({applied} of {n_c64} input rows applied): {len(diff)} differences; '
                      f'first at {diff[0][0]} host {diff[0][1]:05o} C64 {diff[0][2]:05o}')
                for x, h, c in diff[1:12]:
                    print(f'  {x} host {h:05o} C64 {c:05o}')
            elif not ended:
                r['ok'] = False
                print(f'FAIL: {a.scenario}: the C64 {seen}, then did not reach a sample point {cs} cs in '
                      f'(stopped at {end_cs} cs, {applied} of {n_c64} input rows applied)')
            else:
                print(f"{a.scenario}: the C64 {seen}: it ends the window ({end_cs} cs, all {applied} input rows) "
                      "in the host's state")
        if a.profile:
            r['profile'] = dp.profile(tag)
        r.update(scenario=a.scenario, snap_cs=a.snap_cs, snap_at=snap_at, snap_slid=slid > SLIDE_CS, requested_cs=cs,
                 host_holes=holes)
        # Status 0 means no step refused; it does not mean the window ran to its
        # end (a harness exit before --cs, like the TIME1 wrap #595 fixed, also
        # reports status 0). A window that stops short of --cs without a refusal
        # fails, so a "no stop" result always covers the whole window.
        if not a.replay and r.get('status') == 0 and r.get('agc_cs') is not None and r['agc_cs'] < cs:
            r.update(ok=False, short=True)
            print(f"FAIL: {a.scenario}: status 0 but the window ended after {r['agc_cs']} of {cs} cs")
        (dp.OUT / 'flight-scenario-vice.json').write_text(json.dumps(r, indent=1) + '\n')
        (dp.WORK / f'{tag}.json').write_text(json.dumps(r, indent=1) + '\n')
        if os.environ.get('FSV_STEPS') == '1':
            # One record per step: the physical bank (superbank applied) and Z
            # (dap_pass_check.c DP_STEPS); fixed-fixed Z as bank 02 or 03.
            print('last steps (bank,Z/status):',
                  ' '.join((f"{x['job_loads']:02o},{x['steps']:04o}" if 0o2000 <= x['steps'] < 0o4000 else
                            f"{x['steps'] >> 10:02o},{0o2000 + x['steps'] % 1024:04o}" if 0o4000 <= x['steps'] < 0o10000 else
                            f"{x['steps']:05o}")
                           + f"/{x['status']}" for x in r.get('records', [])[-12:]))
        if 'by_request' in r:
            print(f"{a.scenario} from {a.snap_cs} cs: {r['wall_cycles']} cycles for {r['agc_cs']} AGC cs "
                  f"({r['wall_per_agc']} x real time), {r['dropped_ticks']} ticks dropped, {r['steps']} steps, "
                  f"status {r['status']} at {r['at']} (Z {r['z']} FB {r['fb']} superbank {r['superbank']})")
            if a.watch:
                print('  at the end: ' + ' '.join(f'{w}={v:05o}' for w, v in zip([w for w in a.watch.split(',') if w], r['watched'])))
            for k, b in sorted(r['by_request'].items()):
                print(f"  {k}: {b['count']} x, mean {b['cycles'] // b['count']} cycles (max {b['max_cycles']}), "
                      f"{b['steps']} steps, {b['job_loads']} job and {b['task_loads']} task loads")
            for x in r.get('profile', []):
                print(f"  {x['segment']}: {x['calls']} x, {x['cycles']} cycles"
                      f" ({x['cycles'] // x['calls'] if x['calls'] else 0} each)")
        else:
            print(f"FAIL: {a.scenario}: {r.get('error')}")
        results.append(r)
        if not r.get('ok'):
            break
    if a.chain > 1:
        ok = len(results) == a.chain and all(x.get('ok') for x in results)
        print(f"{a.scenario}: chain of {a.chain} windows from {a.snap_cs} cs: " + ('every window matches the host and every seam holds' if ok else f'FAIL at window {len(results) - 1}'))
    return 0 if all(x.get('ok') for x in results) and len(results) == a.chain else 1


if __name__ == '__main__':
    sys.exit(main())
