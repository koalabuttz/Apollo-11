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
import hashlib
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


def snapshot(exe, name, cs, tag, replay=0, every=0, chain=1, downrupt=False):
    """replay: the window's centiseconds; the host run then goes on through
    the window and writes its inputs (-snap.h.replay) and its end state
    (-snap.h.end, and every `every` cs before), tools/flight_host_run.c
    FLIGHT_REPLAY. chain: that many windows back to back, the later ones'
    snapshots in -snap.h-1, -snap.h-2, ... (with .replay, .end and .seam)."""
    scen = fhd.scenario(name)
    _, m, _, keys, env = scen
    header = dp.WORK / f'{tag}-snap.h'
    header.unlink(missing_ok=True)
    for n in range(chain):
        for x in ('', '.replay', '.end', '.seam', '.after', '.after-replay', '.outputs'):
            if n or x:
                pathlib.Path(f'{header}{f"-{n}" if n else ""}{x}').unlink(missing_ok=True)
    # The host takes the snapshot at the first point at or after cs with no
    # interrupt pending (a long job, such as PICAPAR, can hold one off for
    # over 100 cs), then stops; running to cs + 1000 leaves room for that.
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs + 1000 * chain + replay * chain), keys], cwd=ROOT,
                       env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SNAPSHOT': f'{cs},{header}',
                            **({'FLIGHT_DOWNRUPT': '1'} if downrupt else {}),
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
                'channel 35', 'superbank', 'channel 10', 'interrupts enabled', 'radar gate']
# tools/flight_host_run.c rp_row: centiseconds, kind, where (bit 0; bits
# 1-2 the work clock's step in the centisecond), address, value, work.
ROW = struct.Struct('<HBBHHH')
# A snapshot more than this far past --snap-cs is reported as slid.
SLIDE_CS = 100


def compared(i):
    """tools/flight_host_run.c compared(): not the central registers, RUPTREG1-4
    or the clock counters (024-031, the work clock's scalers)."""
    return i >= 0o20 and not 0o70 <= i <= 0o73 and not 0o24 <= i <= 0o31


def shown(x, v):
    """A compared value as printed: centiseconds in decimal, words in octal."""
    return f'{v} cs' if x == 'centiseconds' else f'{v:05o}'


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
    for cs, kind, where, addr, value, work in ROW.iter_unpack(rows):
        if kind in (0, 1):
            out.append((cs, 2 if kind == 0 else 1, where, IMAGE + 2 * addr, value, work))
        elif kind == 3:
            out.append((cs, 3, where, pending, value, work))
        elif addr == 0o377:
            out.append((cs, 4, where, RK_HARDWARE + RADAR_GATE, value, work))
        else:
            at = [RK_HARDWARE + HW[addr]] if addr in HW else [RK_FLIGHT_IO + FLIGHT[addr]] if addr in FLIGHT else []
            at += [device + DEVICE[m][addr]] if addr in DEVICE[m] else []
            if not at:
                sys.exit(f'FAIL: no C64 copy of channel {addr:o} for the replay')
            out += [(cs, 2, where, x, value, work) for x in at]
    return b''.join(ROW.pack(*x) for x in out)


def rows_before(rows, cs):
    """How many of the rows (ROW records) come before centisecond cs."""
    return sum(1 for x in ROW.iter_unpack(rows) if x[0] < cs)


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
        # Channel 10 (the last DSKY relay word) and RELINT/INHINT: the
        # mission's io record has interrupt_enabled right after superbank
        # (p66_bias_io, cm_native_io), 0 or 1 on the host.
        ch += [word(io + HW[0o10]), int(word(dev + DEVICE[m]['superbank'] + 2) != 0), b[io + RADAR_GATE]]
        out.append((cs, (*words, *ch)))
    return out


# --replay-outputs: one output channel change (tools/flight_host_run.c
# rp_outputs PATH.outputs, dap_pass_check.c DP_OUTPUTS): centiseconds since
# the snapshot, channel, value.
OUTPUT = struct.Struct('<HHH')


def outputs_cap(host):
    """The C64's room for its output list: twice the host's changes and
    some, so a C64 that drives a channel more often still shows where."""
    return 2 * len(host) + 64


def outputs_compare(host, c64, until, slack=0):
    """The first difference between two output lists up to centisecond
    `until` (both ended by then), in order: the same channel and value, at
    most `slack` cs apart. None when they match."""
    show = lambda x: None if x is None else dict(cs=x[0], channel=f'{x[1]:o}', value=f'{x[2]:05o}')  # noqa: E731
    for i in range(max(len(host), len(c64))):
        h = host[i] if i < len(host) else None
        c = c64[i] if i < len(c64) else None
        if (h is None or h[0] > until) and (c is None or c[0] > until):
            return None
        if h is None or c is None or h[1:] != c[1:] or abs(h[0] - c[0]) > slack:
            return dict(index=i, host=show(h), c64=show(c))
    return None


# The C64's window clock is TIME1's low 14 bits (dap_pass_check.c replay_cs).
WINDOW_MAX = 0o37777


def pair_states(host_s, c64_s, ended, after=None, after_rows=()):
    """Host and C64 states to compare, paired by their centiseconds: each
    side writes a state at its own first sample point at or after a mark,
    so one side can have a state the other has none for. Checkpoints (before
    the host's end) pair at the same centisecond. With after (the .after
    states the host writes past its end, tools/flight_host_run.c
    RP_AFTER_CS), the C64 runs on to the last of them, writing a state at
    its first sample point at or after each (main), and the end is the
    first centisecond at or after the host's end where both sides have a
    state: the host's end itself, or a later point when the C64 was busy
    there. It does not pair when a host input the C64 was not given comes
    before it (after_rows: those inputs' centiseconds; main gives the C64
    every .after-replay row, so none), or when the two sides share no such
    point; the host's end and the C64's first state from there are then
    unpaired. With after None (a chain window before the last, whose end
    is the next one's seam) the two last states always pair. Returns
    (pairs, unpaired, end): unpaired as dict(side, cs); end None when no
    ends were compared, else dict(shift_cs, paired), shift_cs the paired
    point (or the C64's first state from the host's end) less the host's
    end."""
    h, c = list(host_s), list(c64_s)
    pairs, unpaired, end, ends, i, j = [], [], None, [], 0, 0
    if h and c and ended:
        he = h.pop()
        if after is None:
            ce = c.pop()
            ends, end = [(he, ce)], dict(shift_cs=ce[0] - he[0], paired=True)
        else:
            # The states from the host's end on are the end's candidates.
            host_end = [he] + [x for x in after if x[0] > he[0]]
            c64_end = [x for x in c if x[0] >= he[0]]
            c = [x for x in c if x[0] < he[0]]
            by_cs = {x[0]: x for x in host_end}
            common = [x for x in c64_end if x[0] in by_cs]
            if common and not any(cs < common[0][0] for cs in after_rows):
                ce = common[0]
                ends, end = [(by_cs[ce[0]], ce)], dict(shift_cs=ce[0] - he[0], paired=True)
            elif c64_end:
                at = common[0][0] if common else c64_end[0][0]
                end = dict(shift_cs=at - he[0], paired=False,
                           inputs_between=bool(common), host_has_cs=bool(common))
                unpaired.append(dict(side='host', cs=he[0], end=True))
                unpaired.append(dict(side='C64', cs=c64_end[0][0], end=True))
            else:
                h.append(he)
    while i < len(h) and j < len(c):
        if h[i][0] == c[j][0]:
            pairs.append((h[i], c[j]))
            i, j = i + 1, j + 1
        elif h[i][0] < c[j][0]:
            unpaired.append(dict(side='host', cs=h[i][0]))
            i += 1
        else:
            unpaired.append(dict(side='C64', cs=c[j][0]))
            j += 1
    unpaired += [dict(side='host', cs=x[0]) for x in h[i:]] + [dict(side='C64', cs=x[0]) for x in c[j:]]
    return pairs + ends, unpaired, end


def replay_verdict(host_s, c64_s, ended, after=None, after_rows=(), skip=None, m=None, exact=False):
    """pair_states, then each pair compared in order up to the first that
    differs: dict(pairs, unpaired, end, diff, at_cs, matched, skipped,
    masked). A window matches only when diff is empty, it ended and its end
    paired. masked[DEAD_SUPERBANK] counts the pairs where a dead superbank
    was masked (m), masked[DEAD_SUPERBANK_AT] lists each as (host cs, host
    superbank, C64 superbank)."""
    pairs, unpaired, end = pair_states(host_s, c64_s, ended, after, after_rows)
    diff, at_cs, matched, skipped, masked = [], None, [], set(), {}
    for (hc, hw), (cc, cw) in pairs:
        n = masked.get(DEAD_SUPERBANK, 0)
        diff = replay_compare(hw, cw, skip, skipped, m, masked, exact)
        if masked.get(DEAD_SUPERBANK, 0) > n:
            masked.setdefault(DEAD_SUPERBANK_AT, []).append((hc, hw[SUPERBANK], cw[SUPERBANK]))
        if hc != cc:
            diff.insert(0, ('centiseconds', hc, cc))
        if diff:
            at_cs = hc
            break
        matched.append(hc)
    return dict(pairs=pairs, unpaired=unpaired, end=end, diff=diff, at_cs=at_cs, matched=matched, skipped=skipped,
                masked=masked)


def states(data, n):
    """The first n states of an end-state file: (centiseconds, words)."""
    k = STATE // 2 - 1
    return [(lambda v: (v[0], v[1:]))(struct.unpack_from(f'<{k + 1}H', data, i * STATE)) for i in range(n)]


# TEMPORARY (steward, 10-18): on the CM the C64 charges the self-check idle
# pass a job ends into as module work, where the host charges it as idle
# (the rest of the period), so the C64's schedule can sit a tick off the
# host's (RK: FS_IDLE honouring on the CM idle+interbank set). Until that
# lands, CM windows skip what a tick moves: DSPCNT and DSRUPTSW (T4RUPT's
# display cycle) and DSPTAB's sent bit (a word and its complement are the
# same digits, sent or not). Every result that skips anything says so.
SCHEDULE_SKIP = {'cm': dict(words={0o776: 'DSPCNT', 0o1302: 'DSRUPTSW'}, sent=range(0o1023, 0o1037))}


TIMERS = {0o24: 'TIME2', 0o25: 'TIME1', 0o26: 'TIME3', 0o27: 'TIME4', 0o30: 'TIME5', 0o31: 'TIME6'}
# TEMPORARY: at a sample point in the same centisecond the host and the C64
# can sit in different thirds of it (the work clock steps TIME4, TIME1/TIME3
# and TIME5 in turn), so TIME5 can be a count apart while TIME1 agrees: the
# jobs before it took a different work time (the interpretive charge,
# Interpreter #650; the CM idle charge, RK #663). Skipped, and said so,
# until those land.
TIME5_PHASE = 'TIME5 a count apart (a third of a centisecond)'


# ADVAN, the Executive's idle loop (01,3214 LM, 01,3231 CM). Through it the
# superbank is dead: ADVAN to SELFCHK sets superbank 4 (SUPDXCHZ, SELFBANK),
# CHECKNJ comes back by POSTJUMP, which keeps it, and every way out writes it
# before anything reads it (SUPDXCHZ to SELFCHK or, by NUDIRECT, to the job).
# So two sides at ADVAN with FB below 060000 read either the last job's
# superbank or SELFBANK's 0100, depending only on whether a self-check pass
# ran before the sample point. Masked only then (one side 0100), and not
# under --replay-exact. Temporary like SCHEDULE_SKIP: it fires only while the
# two sides reach ADVAN with different timing. The seam check (seam_check)
# passes no mission, so it never masks.
ADVAN = {'lm': 0o3214, 'cm': 0o3231}
DEAD_SUPERBANK = 'masked_dead_superbank'
DEAD_SUPERBANK_AT = 'masked_dead_superbank_at'
SELFBANK_SUPERBANK = 0o100
SUPERBANK = 2048 + END_CHANNELS.index('superbank')


def dead_superbank(h, c, m):
    """Both sides at ADVAN in bank 01 (FB below 060000), one side's
    superbank SELFBANK's 0100: that superbank difference is masked."""
    return (m in ADVAN and h[SUPERBANK] != c[SUPERBANK] and SELFBANK_SUPERBANK in (h[SUPERBANK], c[SUPERBANK])
            and h[5] == c[5] == ADVAN[m] and h[4] < 0o60000 and c[4] < 0o60000)


def mask_mission(exact, m):
    """The mission replay_compare masks a dead superbank for: none under
    --replay-exact."""
    return None if exact else m


def replay_compare(h, c, skip=None, skipped=None, m=None, masked=None, exact=False):
    """The differences between a host state and the C64's: (name, host,
    C64) for each compared erasable word, the timers, then each output
    channel. With the mission m, a dead superbank (dead_superbank) is not a
    difference; each one is counted in masked[DEAD_SUPERBANK]. Unless
    exact, TIME5 a count apart is noted in skipped (TIME5_PHASE), not a
    difference."""
    out = []
    for i, name in TIMERS.items():
        if h[i] == c[i]:
            continue
        if not exact and i == 0o30 and h[0o25] == c[0o25] and (h[i] - c[i]) % 0o40000 in (1, 0o37777):
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
    dead = dead_superbank(h, c, m)
    if dead and masked is not None:
        masked[DEAD_SUPERBANK] = masked.get(DEAD_SUPERBANK, 0) + 1
    return out + [(END_CHANNELS[k], h[2048 + k], c[2048 + k]) for k in range(len(END_CHANNELS))
                  if h[2048 + k] != c[2048 + k] and not (dead and 2048 + k == SUPERBANK)]


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
    return keys_header(dp.WORK / f'{name}-keys.h', keys), keys


def keys_header(header, keys):
    """Write the DP_KEYS header for (cs, key code) pairs; returns its path."""
    header.write_text(f'#define DP_NKEYS {len(keys)}u\n'
                      f"static const uint16_t dp_key_cs[{max(len(keys), 1)}]={{{','.join(str(t) for t, _ in keys) or '0'}}};\n"
                      f"static const uint16_t dp_key_code[{max(len(keys), 1)}]={{{','.join(f'0{k:o}' for _, k in keys) or '0'}}};\n")
    return header


# The C64 replay's options (platform/c64/dap_pass_check.c DP_REPLAY and
# resident_flight.c), in one place for this tool and tools/dp_replay_link.py,
# which links the harness with them on every push.
DOWNRUPT_DEFINES = ['-DRK_DOWNRUPT=1']


def replay_defines(at, rows, cs, first, table_at, end_at, states_max, busy, outputs_at, cap):
    """The DP_REPLAY defines: the rows at REU `at`, the window's cs, the
    first checkpoint and the checkpoint table, the end states, the busy
    sample (0: none) and the output list (cap 0: none)."""
    return ['-DDP_REPLAY', f'-DDP_REPLAY_REU=0x{at:x}ul', f'-DDP_REPLAY_ROWS={rows}u',
            f'-DDP_REPLAY_CS={cs}u', f'-DDP_STATE_FIRST={first}u',
            f'-DDP_STATE_TABLE=0x{table_at:x}ul', f'-DDP_END=0x{end_at:x}ul',
            f'-DDP_STATES_MAX={states_max}u', *([f'-DDP_BUSY={int(busy)}u'] if busy else []),
            f'-DDP_OUTPUTS=0x{outputs_at if cap else 0:x}ul', f'-DDP_OUTPUTS_MAX={cap}u']


def steal_flags(n, t_from, t_to):
    """RK_STEAL: n stolen cycles a step from TIME2:TIME1 cs t_from to t_to."""
    return [f'-DRK_STEAL={n}u', f'-DRK_STEAL_FROM={t_from}ul', f'-DRK_STEAL_TO={t_to}ul']


def host_holes(exe, name, cs, tag):
    """The points the host run to cs bridges, as flight_holes.describe dicts."""
    _, m, _, keys, env = fhd.scenario(name)
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
    return (*words, *[field(f'channel{c}') for c in (5, 6, 11, 12, 13, 14, 34, 35)], field('superbank'),
            field('channel10'), int(field('interrupt_enabled') != 0), field('radar_gate'))


def snap_channels(header):
    """A snapshot header's channels, {octal channel: value} (0o377 the radar
    gate; tools/flight_host_run.c snapshot)."""
    text = pathlib.Path(header).read_text()
    out = {c: int(v) for c, v in re.findall(r'\(\w\)\.channel(\d+)=(\d+)u', text)}  # names in octal
    gate = re.search(r'\(\w\)\.radar_gate=(\d+)u', text)
    return {**out, **({'377': int(gate[1])} if gate else {})}


def channel_bits(rows, start):
    """The channel bits a window's host input rows set, clear and change
    (tools/device_acceptance.py reads them): per channel in octal, the OR of
    the values, of their complements, and of each row against the
    channel's value before it (the snapshot's, start, for its first row)."""
    seen, out = dict(start), dict(channel_set={}, channel_clear={}, channel_changed={})
    for row in ROW.iter_unpack(rows):
        if row[1] != 2:
            continue
        c, v = f'{row[3]:o}', row[4]
        out['channel_set'][c] = out['channel_set'].get(c, 0) | v
        out['channel_clear'][c] = out['channel_clear'].get(c, 0) | (~v & 0o77777)
        if c in seen:
            out['channel_changed'][c] = out['channel_changed'].get(c, 0) | (v ^ seen[c])
        seen[c] = v
    return out


# The counter and I/O words an output drives (tools/device_acceptance.py):
# RHC counters to ALTM, 042-060.
OUTPUT_WORDS = range(0o42, 0o61)


def outputs_seen(start, host_states):
    """What the program's outputs did over a window, from the host's start
    state and its states at the checkpoints: per output channel (END_CHANNELS
    name) the bits set at any of them and the bits that changed from the
    start, and the output words (OUTPUT_WORDS, octal) that were non-zero or
    changed."""
    every = [start] + [w for _, w in host_states]
    chans = {name: 2048 + k for k, name in enumerate(END_CHANNELS)}
    return dict(channel_set={n: _or(w[i] for w in every) for n, i in chans.items()},
                channel_changed={n: _or(w[i] ^ start[i] for w in every) for n, i in chans.items()},
                nonzero=[f'{a:04o}' for a in OUTPUT_WORDS if any(w[a] for w in every)],
                changed=[f'{a:04o}' for a in OUTPUT_WORDS if any(w[a] != start[a] for w in every)])


def _or(values):
    out = 0
    for v in values:
        out |= v
    return out


def steal_defines(name, start, snap_at):
    """A FLIGHT_STEAL scenario's stolen cycles on the C64 (resident_flight.c
    RK_STEAL): its one T0-T1:N window as TIME2:TIME1 centiseconds. The host
    counts T0 and T1 from its run's start (flight_host_run.c now()); the
    first snapshot, snap_at cs into that run, gives the clock there."""
    spec = fhd.scenario(name)[4].get('FLIGHT_STEAL')
    if not spec:
        return []
    m = re.fullmatch(r'(\d+)-(\d+):(\d+)', spec)
    if not m:
        sys.exit(f'FAIL: {name}: FLIGHT_STEAL={spec}: the C64 replay models one T0-T1:N window')
    t0 = ((start[0o24] & 0o37777) << 14 | (start[0o25] & 0o37777)) - snap_at
    lo, hi, n = (int(x) for x in m.groups())
    return steal_flags(n, t0 + lo, t0 + hi)


def ones_add(w, d):
    """A 15-bit ones'-complement counter word moved by a signed count."""
    v = (w - 0o77777 if w & 0o40000 else w) + (d - 0x10000 if d & 0x8000 else d)
    return v & 0o77777 if v >= 0 else (v + 0o77777) & 0o77777


def seam_check(header, c64_end, m, exact=False, skipped=None):
    """--chain: the differences between a window's start (its snapshot)
    and the C64's end of the window before plus the inputs the host gave
    at that point (.seam rows), as replay_compare reports them."""
    if c64_end is None:
        return [('C64 end state', 0, 0)]
    w = list(c64_end)
    chans = {0o5: 0, 0o6: 1, 0o11: 2, 0o12: 3, 0o13: 4, 0o14: 5, 0o34: 6, 0o35: 7, 0o10: 9, 0o377: 11}
    for _, kind, _, addr, value, _ in ROW.iter_unpack(pathlib.Path(f'{header}.seam').read_bytes()):
        if kind == 0:
            w[addr] = value
        elif kind == 1:
            w[addr] = ones_add(w[addr], value)
        elif kind == 2 and addr in chans:
            w[2048 + chans[addr]] = value
    # A TIME5 phase tolerated at a seam goes into skipped, so the next
    # window reports it (with skips) instead of passing silently.
    return replay_compare(snap_state(header), w, None, set() if skipped is None else skipped, exact=exact)


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
                   help='with --replay, compare without the temporary schedule skip (SCHEDULE_SKIP, CM) and '
                   'without the dead-superbank mask at ADVAN (dead_superbank)')
    p.add_argument('--replay-outputs', action='store_true',
                   help='with --replay, also log every change of output channels 5, 6, 11, 12, 13 and 14 on both sides '
                   'and fail unless the C64 makes the same changes in the same order at the same centiseconds '
                   "(changes the host's device models make, such as the radar's on 12-14, reach the C64 as replay rows, "
                   'so those match by construction; the compare tests the writes the flight code makes)')
    p.add_argument('--outputs-slack', type=int, default=0,
                   help='with --replay-outputs, let a change come up to N cs earlier or later than the host\'s')
    p.add_argument('--replay-every', type=int, default=0,
                   help='with --replay, also compare at the first sample point at or after every N cs, '
                   'to find when the C64 leaves the host')
    p.add_argument('--chain', type=int, default=1,
                   help='with --replay, run that many --cs windows back to back on the host, then each on the C64 from '
                   "the host's state where the one before ended, and fail unless the C64 ends each window in the "
                   "host's state and that state is the next window's start (the seam)")
    p.add_argument('--downrupt', action='store_true',
                   help='DOWNRUPT every 20 ms on both sides (host FLIGHT_DOWNRUPT, C64 RK_DOWNRUPT, as on the disk): '
                        'the window then also compares the downlink, channels 34 and 35 at each checkpoint')
    p.add_argument('--strict-start', action='store_true',
                   help=f'fail when the snapshot slides more than {SLIDE_CS} cs past --snap-cs')
    p.add_argument('--watch', default='', help='up to five erasable words (octal addresses, comma separated) '
                   'printed as the window ends, e.g. 1011 for Comanche MODREG')
    a = p.parse_args()
    dp.WORK.mkdir(parents=True, exist_ok=True)
    m = fhd.scenario(a.scenario)[1]
    # A scenario whose host run has DOWNRUPT on (the uplink-accept rows, for
    # V74's erasable dump) needs it on the C64 too, or the downlink list
    # pointer (CTLIST) only moves on the host.
    # (As the host reads it, tools/flight_downlink.h dl_parse: any value but
    # empty or "0".)
    if fhd.scenario(a.scenario)[4].get('FLIGHT_DOWNRUPT', '0') not in ('', '0') and not a.downrupt:
        print(f'{a.scenario}: the scenario sets FLIGHT_DOWNRUPT, so --downrupt is on')
        a.downrupt = True
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
    # Which source and store image the log's results come from, so a
    # result quoted later can be matched to its build.
    head = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    dirty = subprocess.run(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=ROOT,
                           capture_output=True, text=True).stdout.strip()
    print(f'{a.scenario}: commit {head or "unknown"}{" with local changes" if dirty else ""}; '
          f'store {m}.reu sha256 {hashlib.sha256(reu).hexdigest()}')
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
    header = snapshot(exe, a.scenario, a.snap_cs, tag, a.cs if a.replay else 0, every, a.chain, a.downrupt)
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
    # The C64 counts a window on TIME1's 14 bits (dap_pass_check.c
    # replay_cs): a longer one wraps, and its states land on the store.
    if a.replay:
        for h, _, _ in windows:
            if host_window_cs(h) > WINDOW_MAX:
                sys.exit(f'FAIL: {a.scenario}: the host ran a window {host_window_cs(h)} cs long from {a.snap_cs} cs; '
                         f'the C64 counts at most {WINDOW_MAX} cs')
    # The most states a C64 window writes: one at the first sample point at
    # or after each of the host's checkpoints (at most one per `every` mark
    # before --cs), and the end. The C64 has its
    # own sample points, so it can write a state at a mark the host had none
    # for; its area holds them all and replay_state stops at its end.
    max_states = a.cs // every + 1
    if max_states > 255:
        sys.exit(f'FAIL: {a.scenario}: {max_states} states in a window; the C64 counts at most 255 '
                 '(raise --replay-every)')
    results, c64_end = [], None
    for n, (header, tag, cs) in enumerate(windows):
        seam_skipped = set()
        if n:
            seam = seam_check(header, c64_end, m, a.replay_exact, seam_skipped)
            print(f'{a.scenario}: seam {n}: ' + (f'the C64 end of window {n - 1} is the start of window {n}' if not seam else
                  f'FAIL: the C64 end of window {n - 1} is not the start of window {n}: {len(seam)} differences, first '
                  + ', '.join(f'{x} {shown(x, h)}/{shown(x, c)}' for x, h, c in seam[:6]) + ' (start/C64 end)'))
            if seam:
                results.append(dict(ok=False, seam=n, error=f'seam {n}: {len(seam)} differences'))
                break
        keys, defines, patch, end_at = None, [], [], None
        if a.replay:
            rows = pathlib.Path(f'{header}.replay').read_bytes()
            host_end = pathlib.Path(f'{header}.end').read_bytes()
            # The host's states past its end (the last window only).
            after_path = pathlib.Path(f'{header}.after')
            host_after = (states(after_path.read_bytes(), after_path.stat().st_size // STATE) if after_path.exists()
                          else None)
            # The host's inputs past its end go to the C64 too, after the
            # window's own, so a C64 state past the host's end has every
            # input the host had before it (pair_states).
            extra = pathlib.Path(f'{header}.after-replay').read_bytes() if host_after is not None else b''
            after_rows = []
            n_states = len(host_end) // STATE
            if n_states > max_states:
                sys.exit(f'FAIL: {a.scenario}: the host wrote {n_states} states, more than the {max_states} reserved')
            # The C64 writes its checkpoints and ends where the host did: at
            # its first sample point at or after each of the host's states'
            # centiseconds (a first sample point at or after the asked cs
            # can come a tick before the host's). The checkpoints after the
            # first go in a 2-byte table after the state area, ended by 0xFFFF.
            host_cs = [x[0] for x in states(host_end, n_states)]
            cs, marks = host_cs[-1], host_cs[:-1]
            # Past the host's end the C64 runs on to the host's last .after
            # state, writing a state at each of them too, so a C64 busy at
            # the host's end still ends at a point where the host has one.
            after_cs = [x[0] for x in host_after or () if x[0] > cs]
            c64_cs, marks = (after_cs[-1], marks + [cs] + after_cs[:-1]) if after_cs else (cs, marks)
            states_max = max_states + len(after_cs)
            if states_max > 255:
                sys.exit(f'FAIL: {a.scenario}: {states_max} states in a window; the C64 counts at most 255')
            end_bytes = states_max * C64_STATE
            table = struct.pack(f'<{len(marks[1:]) + 1}H', *marks[1:], 0xffff)
            # --replay-outputs: the C64's output list after the table.
            host_outs = list(OUTPUT.iter_unpack(pathlib.Path(f'{header}.outputs').read_bytes())) if a.replay_outputs else []
            cap = outputs_cap(host_outs) if a.replay_outputs else 0
            # CM channel rows can take two C64 rows each.
            at = replay_place(manifest, m, reu, 2 * len(rows + extra) + end_bytes + len(table) + OUTPUT.size * cap + 256)
            end_at = (at + 2 * len(rows + extra) + 0xff) & ~0xff
            table_at = end_at + end_bytes
            outputs_at = table_at + len(table)
            busy = fhd.scenario(a.scenario)[4].get('FLIGHT_BUSY_SAMPLE')
            # The window ends at the C64's first sample point cs in; DP_CS
            # stops it later if none comes.
            # The C64's rows (their count does not depend on rk_device's address).
            n_c64 = len(c64_rows(rows, m, 0, 0)) // ROW.size
            n_extra = len(c64_rows(extra, m, 0, 0)) // ROW.size
            defines = [*replay_defines(at, n_c64 + n_extra, c64_cs, marks[0] if marks else 0xffff, table_at, end_at,
                                       states_max, busy, outputs_at, cap),
                       *steal_defines(a.scenario, snap_state(windows[0][0]), snap_at)]
            kinds = {}
            # What the window's inputs touch, by kind (tools/device_acceptance.py
            # reads it): erasable words and PIPAs by address, channels (0o377 the
            # radar gate), interrupt requests by RK_RUPT_BIT bit.
            touched = {0: set(), 1: set(), 2: set(), 3: 0}
            for row in ROW.iter_unpack(rows):
                kinds[row[1]] = kinds.get(row[1], 0) + 1
                if row[1] == 3:
                    touched[3] |= row[4]
                elif row[1] in touched:
                    touched[row[1]].add(row[3])
            replay_inputs = dict(counts={str(k): n for k, n in sorted(kinds.items())},
                                 erasable=[f'{a:04o}' for a in sorted(touched[0])],
                                 pipas=[f'{a:04o}' for a in sorted(touched[1])],
                                 channels=[f'{a:o}' for a in sorted(touched[2])], requests=touched[3],
                                 **channel_bits(rows, snap_channels(header)))
            replay_outputs = outputs_seen(snap_state(header), states(host_end, n_states))
            print(f'{a.scenario}: {len(rows) // ROW.size} host input rows ({n_c64} on the C64) in the window at REU 0x{at:x} '
                  f"({kinds.get(1, 0)} PIPA counts, {kinds.get(0, 0)} erasable words, {kinds.get(2, 0)} channels, "
                  f'{kinds.get(3, 0)} interrupt requests)'
                  + (f', then {len(extra) // ROW.size} past the host\'s end' if extra else ''))
        if a.keys:
            keys, pressed = window_keys(tag, fhd.scenario(a.scenario)[3], snap_at, cs)
            print(f'{a.scenario}: {len(pressed)} keys pressed on the C64 at '
                  + ' '.join(f'+{t}:{k:o}' for t, k in pressed))
        prg = dp.link(a.cc, tag, header, cs + 500 if a.replay else cs, scratch, os.environ.get('FSV_STEPS') == '1',
                      m, a.profile, keys, [w for w in a.watch.split(',') if w][:5],
                      defines + (DOWNRUPT_DEFINES if a.downrupt else []))
        if a.replay:
            # rk_device's address comes from the link.
            patch = [(at, c64_rows(rows + extra, m, map_symbol(prg, 'rk_device'), map_symbol(prg, 'rk_pending'))), (table_at, table),
                     (outputs_at, bytes(OUTPUT.size * cap))]
        # The run's declared REU regions (tools/reu_layout.py): VICE logs every
        # DMA, read or write, that is not inside one of them, and afterwards every
        # changed byte must lie in a writable one (not the store, the rope or the
        # replay rows). --reu-check-drop leaves a region out (the check's own test).
        # Each chain window has its own rows, so its own regions.
        regions = [x for x in reu_layout.layout(
            m, manifest, (at, end_at, reu_layout.free_top(m, manifest)) if a.replay else None)
            if x[0] != a.reu_check_drop]
        r = dp.vice(a, tag, prg, pinned, m, patch, [reu_layout.dma_trace(regions)])
        r.update(mission=m, snap_cs=a.snap_cs, replay=bool(a.replay))
        after = dp.WORK / f'vice-{tag}' / 'image.reu'
        if r.get('status') is not None and after.exists():
            before = bytearray(reu)
            for at_, data in patch:
                before[at_:at_ + len(data)] = data
            bounds = reu_layout.report(m, regions, before, after.read_bytes(), a.scenario)
            log = (dp.WORK / f'vice-{tag}' / 'monitor.log').read_text()
            bounds['dma_outside'] = reu_layout.dma_misses(log)
            bounds['dma_trace_armed'] = reu_layout.trace_armed(log)
            bounds['dma_hits'] = reu_layout.dma_hits(log)[:16]
            bounds['dma_unchecked'] = reu_layout.dma_misses(log, (4,))
            if bounds['dma_unchecked']:
                print(f"{a.scenario}: {bounds['dma_unchecked']} DMAs seen only after they ran (last byte checked)")
            if bounds['dma_outside']:
                bounds['ok'] = False
                print(f"FAIL: {a.scenario}: {bounds['dma_outside']} DMAs outside the {m} REU layout")
                for h in bounds['dma_hits']:
                    print(f"  PC {h['pc']} command {h['command']} C64 {h['c64']} REU {h['start']}-{h['end']}")
            else:
                print(f'{a.scenario}: every DMA inside the {m} REU layout')
            r['reu_bounds'] = bounds
            if not bounds['ok']:
                r['ok'] = False
        if a.replay and r.get('status') is not None:
            mail = (dp.WORK / f'vice-{tag}' / 'results.bin').read_bytes()
            applied, end_cs, ended, c64_written = struct.unpack_from('<HHBB', mail, 0x30)
            # Rows past the host's end the C64 applied before its own end:
            # every one before that centisecond (dap_pass_check.c replay_due).
            due_extra = rows_before(c64_rows(extra, m, 0, 0), end_cs) if ended else 0
            applied_extra, applied = max(0, applied - n_c64), min(applied, n_c64)
            r.update(replay_rows=n_c64, replay_applied=applied, replay_end_cs=end_cs, replay_ended=bool(ended),
                     replay_after_rows=applied_extra, replay_inputs=replay_inputs, replay_outputs=replay_outputs)
            # Every state the C64 wrote against the host's at the same point.
            image = (dp.WORK / f'vice-{tag}' / 'image.reu').read_bytes()
            host_s, c64_s = states(host_end, n_states), c64_states(image[end_at:], min(c64_written, states_max), m)
            c64_end = c64_s[-1][1] if c64_s else None
            skip = None if a.replay_exact else SCHEDULE_SKIP.get(m)
            v = replay_verdict(host_s, c64_s, bool(ended), host_after, after_rows, skip, mask_mission(a.replay_exact, m),
                               a.replay_exact)
            unpaired, end, diff, at_cs, matched, skipped = (v[k] for k in ('unpaired', 'end', 'diff', 'at_cs', 'matched',
                                                                         'skipped'))
            skipped = skipped | {f'{x} (at seam {n})' for x in seam_skipped}
            shift = end['shift_cs'] if end else None
            inner = [u for u in unpaired if not u.get('end')]
            r.update(replay_unpaired=unpaired, replay_unpaired_count=len(inner), replay_end_shift_cs=shift,
                     replay_end_paired=bool(end and end['paired']))
            if inner:
                print(f'{a.scenario}: states with no partner, not compared: '
                      + ', '.join(f"{u['side']} {u['cs']} cs" for u in inner))
            if end and end['paired'] and shift:
                print(f"{a.scenario}: the C64 ends {shift} cs after the host's end; compared with the host's state "
                      f"at {cs + shift} cs")
            if skipped:
                print(f"{a.scenario}: skipped (temporary, SCHEDULE_SKIP / TIME5_PHASE): {', '.join(sorted(skipped))}")
            n_dead, dead_at = v['masked'].get(DEAD_SUPERBANK, 0), v['masked'].get(DEAD_SUPERBANK_AT, [])
            if n_dead:
                print(f'{a.scenario}: {DEAD_SUPERBANK}: {n_dead} (superbank differs only at ADVAN, where it is dead): '
                      + ', '.join(f'{x} cs host {hs:04o} C64 {cs:04o}' for x, hs, cs in dead_at))
            r.update(**{DEAD_SUPERBANK: n_dead, DEAD_SUPERBANK_AT: dead_at})
            r.update(replay_matched_cs=matched, replay_differences=len(diff), replay_differ_at_cs=at_cs,
                     replay_skipped=sorted(skipped),
                     replay_first=[dict(at=x, host=shown(x, h), c64=shown(x, c)) for x, h, c in diff[:32]])
            seen = (f"matches the host{' (with skips)' if skipped else ''} at {', '.join(map(str, matched))} cs" if matched
                    else 'matches the host at no point') + (f' ({len(inner)} checkpoints not compared)' if inner else '')
            if a.replay_outputs:
                n_bytes, full = struct.unpack_from('<HB', mail, 0x3a)
                n_outs = n_bytes // OUTPUT.size
                c64_outs = list(OUTPUT.iter_unpack(image[outputs_at:outputs_at + OUTPUT.size * n_outs]))
                first = (dict(index=n_outs, host=None, c64=None, full=True) if full else
                         outputs_compare(host_outs, c64_outs, min(cs, end_cs), a.outputs_slack))
                r.update(outputs_matched=first is None, outputs_first_diff=first,
                         outputs_host=len(host_outs), outputs_c64=n_outs)
                if first is None:
                    print(f'{a.scenario}: the C64 makes the host\'s {len(host_outs)} output channel changes '
                          + (f'within {a.outputs_slack} cs' if a.outputs_slack else 'at the same centiseconds'))
                else:
                    r['ok'] = False
                    print(f'FAIL: {a.scenario}: ' + (f'more than {cap} output channel changes on the C64' if full else
                          f"output channel change {first['index']} differs: host {first['host']} C64 {first['c64']}"))
            if diff:
                r['ok'] = False
                print(f'FAIL: {a.scenario}: the C64 {seen}, then leaves it by {at_cs} cs into the window '
                      f'({applied} of {n_c64} input rows applied): {len(diff)} differences; '
                      f'first at {diff[0][0]} host {shown(*diff[0][:2])} C64 {shown(diff[0][0], diff[0][2])}')
                for x, h, c in diff[1:12]:
                    print(f'  {x} host {shown(x, h)} C64 {shown(x, c)}')
            elif ended and end and not end['paired']:
                r['ok'] = False
                why = 'a host input comes in between' if end['inputs_between'] else 'the host has no state there'
                print(f"FAIL: {a.scenario}: the C64 {seen}, then ends at {cs + shift} cs, {shift} cs after the host's "
                      f"end, and the ends are not compared ({why})")
            elif ended and (applied != n_c64 or applied_extra != due_extra):
                r['ok'] = False
                print(f'FAIL: {a.scenario}: the C64 {seen}, but applied {applied} of the {n_c64} input rows'
                      + (f' and {applied_extra} of the {due_extra} past the host\'s end' if due_extra or applied_extra else ''))
            elif not ended:
                r['ok'] = False
                print(f'FAIL: {a.scenario}: the C64 {seen}, then did not reach a sample point {c64_cs} cs in '
                      f'(stopped at {end_cs} cs, {applied} of {n_c64} input rows applied)')
            else:
                print(f"{a.scenario}: the C64 {seen}: it ends the window ({end_cs} cs, all {applied} input rows"
                      + (f", {shift} cs after the host's end" if shift else '') + ") in the host's state")
        if a.profile:
            r['profile'] = dp.profile(tag)
        r.update(scenario=a.scenario, snap_cs=a.snap_cs, snap_at=snap_at, snap_slid=slid > SLIDE_CS, requested_cs=cs,
                 host_holes=holes, downrupt=a.downrupt, commit=flight_holes.source_commit())
        if a.chain > 1:
            r.update(chain=a.chain, chain_window=n)
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
        # One result for the whole chain (tools/program_acceptance.py): each
        # window's verdict, and the seam that failed if one did.
        first = results[0] if results else {}
        (dp.WORK / f'{a.scenario}-{a.snap_cs}-chain{a.chain}.json').write_text(json.dumps(dict(
            scenario=a.scenario, snap_cs=a.snap_cs, snap_at=first.get('snap_at'), requested_cs=a.cs, chain=a.chain,
            ok=ok, status=next((x.get('status') for x in results if x.get('status')), 0), replay_rows=sum(x.get('replay_rows', 0) for x in results),
            replay_ended=ok, replay_differences=0 if ok else None, commit=flight_holes.source_commit(),
            reu_bounds=dict(ok=all((x.get('reu_bounds') or {}).get('ok', True) for x in results)),
            windows=[{k: x.get(k) for k in ('ok', 'status', 'at', 'seam', 'error', 'replay_differences',
                                             'replay_differ_at_cs', 'replay_first', 'replay_matched_cs', 'replay_end_shift_cs',
                                             'replay_end_paired', 'replay_unpaired_count',
                                             'requested_cs', 'wall_per_agc', 'replay_skipped', 'outputs_matched',
                                             'outputs_first_diff')} for x in results]), indent=1) + '\n')
        print(f"{a.scenario}: chain of {a.chain} windows from {a.snap_cs} cs: " + ('every window matches the host and every seam holds' if ok else f'FAIL at window {len(results) - 1}'))
    return 0 if all(x.get('ok') for x in results) and len(results) == a.chain else 1


if __name__ == '__main__':
    sys.exit(main())
