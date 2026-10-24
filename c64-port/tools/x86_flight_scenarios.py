#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The native x86_64 flight program flying the host differential's flight
scenarios with its own vehicle models (platform/x86/vehicle_models.h),
compared with the host differential flying the same scenario
(tools/flight_host_run.c).

The two runs share the routines and the models but not the clock schedule:
the x86 program charges each point the median cost of
docs/flight-routine-mct.json, the harness's work clock (FLIGHT_SCALED=W)
each table's weight. So the reference is the harness on W and the
yardstick is the harness against itself: the same scenario on its other
clock (FLIGHT_SCALED=1,1,2, yaAGC's own cycles per routine). Near the end
of each scenario (the first sample point at or after 1,000 cs before it)
the x86 program passes when
  - its major mode equals the harness's;
  - its state vector is within SPREAD_K times the harness's own spread of
    the harness, plus RN_FLOOR / VN_FLOOR for the per-routine charges the
    spread cannot see (the largest such case measured: CM P40, VN 0.13 m/s
    against a spread of 0.004);
  - its erasable image (061-3777) differs from the harness's in at most
    WORDS_K times as many words as the harness's two clocks differ in,
    plus WORDS_PLUS. This is what sees programs that do not move the
    state vector: P52 and P57 with their marks left out differ in about
    200 words against a spread of 6 and 18.
With the rendezvous radar (FLIGHT_RR), when the harness's two clocks
leave the state vector at different times (PIPTIME), the state vector
check is against the truth instead. Marks move the state vector and its time (PIPTIME) by
amounts that depend on when each mark falls, so two good runs can hold
state vectors hundreds of kilometres apart, each correct for its own
time (LM P78: the harness at PIPTIME 19,234 cs, the x86 program at
41,786). So each run's own state vector is compared with the true orbit
FLIGHT_RR gives at that run's PIPTIME, and the x86 program's error must be
within SPREAD_K times the larger of the harness's two errors, plus
RN_FLOOR / VN_FLOOR. That check alone does not see the radar:
with FLIGHT_RR both vehicles' permanent state blocks (BLOCKS), the states
the marks update, are checked too, each against its vehicle's true orbit
at the block's own time (TET). Their errors may exceed the larger of the
harness's two by at most RN_FLOOR / VN_FLOOR (no SPREAD_K: a target 300 m
off moves the LM's own block by 270-300 m, inside 4 times the harness's
error), and their times may be at most TET_BEHIND behind the earlier of
the harness's: with the radar off, the target moved 300 m or 5 km, the
marks or the incorporations (V33E) left out one block or the other is
hundreds of metres off or thousands of centiseconds behind.
KNOWN lists scenarios outside the RN/VN bounds, or with an alarm code the
harness did not raise, with the reason and an envelope of their own: such
a run is reported and does not fail when it reaches the compare point,
exits 0, has the same major mode, stays within the word limit and within
the envelope (alarm codes included, and with 'radar' more landing radar
readings unmatched; never the restart count or the TC trap); anything
else fails. Results go to
build/x86-flight/scenarios/summary.json.

  x86_flight_scenarios.py [--host-cc CC] [--skip-build] [--drop-keys CHARS] [NAME ...]

Scenarios that need a model the x86 program does not have yet (uplink,
stolen time, the gyro torquing and optics output models) are left out. A scenario that starts from the NASSP P63 core
starts the x86 program from the harness's state at that time (X86_START,
made at test time and deleted after the run). GAPS lists those that stop
at a routine not yet certified: they pass only when they stop exactly
there. Needs yaAGC (deps/virtualagc) for the
harness; the x86 program itself links none.
"""
import argparse, json, math, os, re, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402
import x86_flight_build as xb  # noqa: E402

OUT = xb.OUT / 'scenarios'
SPREAD_K, RN_FLOOR, VN_FLOOR = 4.0, 50.0, 0.15
WORDS_K, WORDS_PLUS = 1.5, 10
# Marks move a state block's time (TET) on: the x86 program's may be ahead
# of the harness's (it marks at other times) but at most this far behind
# the earlier of the harness's two (the most measured: CM P20,
# 355 cs). With no marks it stays where the scenario's first integration
# left it (LM 4,704 cs).
TET_BEHIND = 1000
WORDS = range(0o61, 0o4000)
# Since the x86 program charges time as the harness's work clock does
# (fd_work_clock), every scenario is inside its limits; the eleven that
# needed an envelope (P11's liftoff time, the cage at a sample point, the
# V25s' redisplays, the rendezvous marks, the Pinball rows) fly as the
# harness does.
KNOWN = {}
UNSUPPORTED = ('FLIGHT_UPLINK', 'FLIGHT_STEAL', 'FLIGHT_GYRO', 'FLIGHT_OPTICS')
# A FLIGHT_CORE scenario starts from a yaAGC core dump, which the x86
# program never reads: the harness loads it and writes its state at the
# first sample point (FLIGHT_SNAPSHOT=0), and the x86 program starts from
# that header (X86_START). The header is made here, at test time only.
# Scenarios the x86 program cannot fly through yet: it stops at a routine
# whose table is not certified, or at a point no table keys (it has no
# yaAGC bridge). Each is flown and passes only when it stops exactly there:
# exit 1, this stop line (or, with 'point', this stop at any time) and this
# refusal. Any other outcome fails, a full run included, so the entry goes
# when the table is certified or the point keyed. With 'resume', the rest
# is then flown from the harness's state at that centisecond (X86_START)
# and compared as any scenario; with 'then' instead, that resumed run must
# stop at the next gap ('then', a GAPS entry) in the same way.
# Pinball's LM and CM certificates (#674) took the last of them: DECDSP,
# P47, V55 and P65's displays and the EV TC Q point now run natively.
GAPS = {}
# Each mission's MODREG, RN and VN (metres at 2^29, m/cs at 2^7), PIPTIME.
STATE = {'lm': (0o1011, 0o1220, 0o1226), 'cm': (0o1011, 0o1170, 0o1176)}
PIPTIME = {'lm': 0o1234, 'cm': 0o1204}
# FAILREG and the two alarm code registers after it (both missions).
FAILREG = (0o375, 0o376, 0o377)
# REDOCTR, the count of restarts each restart's GOPROG adds to
# (Luminary099 and Comanche055 ERASABLE_ASSIGNMENTS.agc). A row with a
# restart (FLIGHT_GOJAM, or the TC trap's) must count as many as the harness
# on one of its two clocks (they can differ just after the GOJAM):
# a restart that resumes the same program can leave the state vector and
# most words as they were, so this is what sees that the x86 run took it.
REDOCTR = {'lm': 0o320, 'cm': 0o1154}
# The two vehicles' permanent state blocks (RRECTCSM ... and RRECTLEM ...:
# RRECT, VRECT, TET at +014, DELTA +016, NUV +024, RCV +032, VCV +040), at
# the same addresses on both missions. Which one a mark updates is
# VEHUPFLG's choice (the LM's own block by default on the LM, the LM's on
# the CM), so both are checked.
BLOCKS = {'csm': 0o1554, 'lm': 0o1626}
MOON_MU = 4.9028e12


# Three rendezvous rows again, with each V33 a display-driven key: !
# makes it wait for the verb and noun to flash (both programs:
# tools/flight_host_run.c keys_parse, platform/x86/flight_scenario.h), as
# the crew answers the display rather than the clock. Before the x86
# program ran on the work clock these were the rows whose V33s landed on
# a blank display; they check that both programs wait the same way.
WAIT = ('lm-rendezvous-tpm', 'lm-stable-orbit', 'lm-stable-orbit-lm')

# Word limits tighter than the harness's clock spread, for rows the x86
# program flies nearly word for word. The two stable-orbit -wait rows end
# 8 and 17 words apart; with the x86 program ignoring ! (--drop-keys !)
# they end over 500 apart, still inside the spread, so without a cap they
# could not show that both programs wait the same way.
WORD_CAP = {'lm-stable-orbit-wait': 100, 'lm-stable-orbit-lm-wait': 100}


def supported():
    rows = [row for row in fhd.FLIGHT + fhd.RESTART_FLIGHT + fhd.V69_FLIGHT if not any(k in row[4] for k in UNSUPPORTED)]
    return rows + [(f'{name}-wait', m, cs, keys.replace('V33E', '!V33E'), env)
                   for name, m, cs, keys, env in fhd.FLIGHT if name in WAIT]


# The TC trap rows (fhd.V69_FLIGHT: P00, then V69E). The harness takes the
# trap on the work clock and unscaled (yaAGC's own trap), not on its
# 1,1,2 clock, so their spread is the work clock against unscaled. The x86
# program must take the trap too, within SPREAD_K times the harness's two
# trap times apart plus TRAP_CS.
TC_TRAP = {row[0] for row in fhd.V69_FLIGHT}
TRAP_CS = 10
TRAP_LINE = re.compile(r'^TC trap at (\d+) cs', re.M)


def trap_compare(logs, x86_out):
    """The TC trap's time in the harness's two runs and the x86 run:
    (summary, passed, line)."""
    h, o, x = (TRAP_LINE.search(t) for t in (logs[0], logs[1], x86_out))
    if not (h and o):
        return None, False, 'the harness took no TC trap'
    h, o = int(h[1]), int(o[1])
    lim = SPREAD_K * abs(h - o) + TRAP_CS
    if not x:
        return dict(harness=h, other_clock=o, x86=None, limit=lim), False, 'the x86 program took no TC trap'
    x = int(x[1])
    return (dict(harness=h, other_clock=o, x86=x, limit=lim), abs(x - h) <= lim,
            f'TC trap at {x} cs (harness {h}, unscaled {o}; limit {lim} cs apart)')


def kepler(s, dt):
    """The conic state dt seconds after s (platform/x86/radar_models.h's
    rm_kepler)."""
    r0, v2 = math.sqrt(sum(v * v for v in s[:3])), sum(v * v for v in s[3:])
    a = 1.0 / (2.0 / r0 - v2 / MOON_MU)
    sg = sum(s[i] * s[i + 3] for i in range(3)) / math.sqrt(MOON_MU)
    n, sa = math.sqrt(MOON_MU / a ** 3), math.sqrt(a)
    de = n * dt
    for _ in range(50):
        fn = de + sg / sa * (1 - math.cos(de)) - (1 - r0 / a) * math.sin(de) - n * dt
        de -= fn / (1 + sg / sa * math.sin(de) - (1 - r0 / a) * math.cos(de))
    r = a + (r0 - a) * math.cos(de) + sg * sa * math.sin(de)
    f, g = 1 - a / r0 * (1 - math.cos(de)), a * sg / math.sqrt(MOON_MU) * (1 - math.cos(de)) + r0 * math.sqrt(a / MOON_MU) * math.sin(de)
    fd, gd = -math.sqrt(MOON_MU * a) / (r * r0) * math.sin(de), 1 - a / r * (1 - math.cos(de))
    return [f * s[i] + g * s[i + 3] for i in range(3)], [fd * s[i] + gd * s[i + 3] for i in range(3)]


def dp(w, a):
    return ones(w[a]) * 16384 + ones(w[a + 1])


def truth_errors(m, env, w):
    """The run's own state vector against FLIGHT_RR's true orbit at its
    PIPTIME: (RN error m, VN error m/s, PIPTIME cs)."""
    t0, lm, csm = env['FLIGHT_RR'].split(':')
    own = [float(v) for v in (lm if m == 'lm' else csm).split(',')]
    modreg, rn, vn = STATE[m]
    pip = dp(w, PIPTIME[m])
    r, v = kepler(own, (pip - int(t0)) * 0.01)
    return distance(vector(w, rn, 29, 1.0), r), distance(vector(w, vn, 7, 100.0), v), pip


def block_errors(vehicle, env, w):
    """One vehicle's permanent state block (BLOCKS), the state the marks
    update, against FLIGHT_RR's true orbit for that vehicle at the block's
    time TET: (position error m, velocity error m/s, TET cs). In the moon's
    sphere the conic RCV/VCV is at 2^27 m / 2^5 m/cs and the deviation
    DELTA/NUV 2^9 / 2^6 below it (RECTIFY's shifts with PBODY 2)."""
    t0, lm, csm = env['FLIGHT_RR'].split(':')
    true = [float(v) for v in (lm if vehicle == 'lm' else csm).split(',')]
    o = BLOCKS[vehicle]
    tet = dp(w, o + 0o14)
    r, v = kepler(true, (tet - int(t0)) * 0.01)
    pos = [dp(w, o + 0o32 + 2 * i) * 2.0 ** -1 + dp(w, o + 0o16 + 2 * i) * 2.0 ** -10 for i in range(3)]
    vel = [(dp(w, o + 0o40 + 2 * i) * 2.0 ** -23 + dp(w, o + 0o24 + 2 * i) * 2.0 ** -29) * 100.0 for i in range(3)]
    return distance(pos, r), distance(vel, v), tet


def ones(v):
    return -((~v) & 0o37777) if v & 0o40000 else v


def vector(w, a, scale, k):
    return [(ones(w[a + 2 * i]) * 16384 + ones(w[a + 2 * i + 1])) * 2.0 ** (scale - 28) * k for i in range(3)]


def harness_state(m, cs, keys, env, at, clock='W', out=None, log=None):
    """The harness's native erasable image at the first sample point at or
    after centisecond at, on the given clock (FLIGHT_SCALED; None: unscaled,
    yaAGC's own clock): (centisecond, 2048 words), or None when it wrote no
    snapshot. With a list out, the run also watches the landing radar's
    words (LR_ENV) and appends its output; with a list log, it appends its
    output only."""
    snap = OUT / f'snap-{os.getpid()}.h'
    run_env = {**os.environ, **env, **(LR_ENV if out is not None else {}), 'FLIGHT_SNAPSHOT': f'{at},{snap}'}
    run_env.pop('FLIGHT_SCALED', None)
    if clock is not None:
        run_env['FLIGHT_SCALED'] = clock
    p = subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []),
                       cwd=ROOT, env=run_env, capture_output=True, text=True, check=False)
    if out is not None:
        out.append(p.stdout)
    if log is not None:
        log.append(p.stdout)
    if not snap.exists():
        return None
    text = snap.read_text()
    snap.unlink()
    words = [int(x.rstrip('u')) for x in re.search(r'snap_words\[2048\][^{]*\{([^}]*)\}', text)[1].split(',')]
    return int(re.search(r'SNAP_CS (\d+)', text)[1]), words


def with_start(m, cs, keys, env, at=0):
    """env for the x86 program: with X86_START, the harness's state at its
    first sample point at or after centisecond at (work clock), when the
    scenario starts from a core dump or at is given (a GAPS resume)."""
    if 'FLIGHT_CORE' not in env and not at:
        return env
    start = OUT / f'start-{m}-{os.getpid()}.h'
    start.unlink(missing_ok=True)
    subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []),
                   cwd=ROOT, env={**os.environ, **env, 'FLIGHT_SCALED': 'W', 'FLIGHT_SNAPSHOT': f'{at},{start}'},
                   capture_output=True, check=False)
    return {**env, 'X86_START': str(start)}


def x86_state(m, keys, env, at, lr=False):
    """The x86 program's erasable image at its first sample point at or after centisecond at:
    (run, centisecond, 2048 words), centisecond None when it wrote none. lr:
    the run also watches the landing radar's words (LR_ENV, X86_WATCH)."""
    samples = OUT / f'samples-{os.getpid()}.txt'
    watch = {'X86_WATCH': LR_ENV['FLIGHT_WATCH'], 'FLIGHT_LR_TRACE': '1'} if lr else {}
    r = subprocess.run([str(xb.OUT / f'x86-flight-{m}'), fhd.ROPE[m], str(at + 1000)] + ([keys] if keys else []), cwd=ROOT,
                       env={**os.environ, **env, **watch, 'X86_SPEED': '0', 'X86_SAMPLES': str(samples),
                            'X86_SAMPLES_FROM': str(at)}, capture_output=True, text=True)
    line = samples.read_text().split('\n', 1)[0].split() if samples.exists() else []
    if samples.exists():
        samples.unlink()
    if len(line) != 2049:
        return r, None, None
    return r, int(line[0]), [int(v, 8) for v in line[1:]]


def distance(a, b):
    return sum((x - y) ** 2 for x, y in zip(a, b)) ** 0.5


# The landing radar (FLIGHT_LR rows). Both programs watch what the AGC
# takes from the radar: HMEAS (03654, each altitude reading), VMEAS
# (03652, each velocity reading), DELTAH (03664, the altitude update the
# reading gives) and FLGWRD12's LRALTFLG (0110 bit 5: no LR altitude data,
# which follows the range data-good bit), and print the radar model's
# readings with the true height and HCALC (FLIGHT_LR_TRACE).
LR_ENV = {'FLIGHT_WATCH': '110,3652,3653,3654,3655,3664,3665', 'FLIGHT_WATCH_STEP': '1', 'FLIGHT_LR_TRACE': '1'}
# Each word's readings: (high, low) addresses, and whether its differences
# are relative (to the larger of the value and 1,000 counts) or in counts;
# the floors added to 4 times the harness's own median and 90th percentile.
LR_READINGS = {'HMEAS': (0o3654, 0o3655, True, 0.002, 0.005), 'VMEAS': (0o3652, 0o3653, True, 0.002, 0.005),
               'DELTAH': (0o3664, 0o3665, False, 10, 20)}
LR_MATCH = 100   # a reading matches the other run's nearest within this many cs
LR_UNMATCHED = 4  # unmatched readings allowed beyond the harness's own (one cycle early or late per change)
LR_FLAG_CS = 250  # a flag change may come this much later or earlier, beyond twice the harness's own
LR_HCALC_M = 10.0  # HCALC's error from the true height: metres beyond 4 times the harness's own


def lr_history(text, at):
    """A run's landing radar history up to centisecond at: each word's
    readings [(cs, value)], LRALTFLG's changes, and HCALC minus the true
    height at each altitude reading [(cs, metres)]. The values at the start
    are not readings."""
    words, last, readings, flag = {}, {}, {k: [] for k in LR_READINGS}, []
    errors = []
    for ln in text.splitlines():
        f = ln.split()
        if ln.startswith('watch ') and int(f[1]) <= at:
            t, a, v = int(f[1]), int(f[3], 8), int(f[4], 8)
            first = a not in words
            words[a] = v
            if a == 0o110:
                if not first and (v & 0o20) != (last.get(a, v) & 0o20):
                    flag.append((t, (v >> 4) & 1))
                last[a] = v
            for k, (hi, lo, _, _, _) in LR_READINGS.items():
                if a in (hi, lo) and hi in words and lo in words:
                    dv = ones(words[hi]) * 16384 + ones(words[lo])
                    if k in last and last[k] != dv:
                        if readings[k] and readings[k][-1][0] == t:
                            readings[k][-1] = (t, dv)   # the other half of the same store
                        else:
                            readings[k].append((t, dv))
                    last[k] = dv
        elif ln.startswith('LR ') and f[1].isdigit() and int(f[1]) <= at and ' range ' in ln:
            errors.append((int(f[1]), float(f[12]) - float(f[7])))
    return readings, flag, errors


def lr_pairs(a, b):
    """Readings a and b matched by time (each b reading at most once): the
    pairs [(a value, b value)] and how many on either side matched none."""
    pairs, used = [], set()
    for t, v in a:
        near = min((k for k in range(len(b)) if k not in used and abs(b[k][0] - t) <= LR_MATCH),
                   key=lambda k: abs(b[k][0] - t), default=None)
        if near is not None:
            used.add(near)
            pairs.append((v, b[near][1]))
    return pairs, len(a) + len(b) - 2 * len(pairs)


def lr_quantiles(pairs, rel):
    d = sorted(abs(x - w) / (max(abs(w), 1000) if rel else 1) for w, x in pairs)
    return (d[len(d) // 2], d[min(len(d) - 1, int(0.9 * len(d)))]) if d else (0, 0)


def lr_compare(ref, other, x86, at):
    """The landing radar check: (block, passed, line). Every reading the
    harness took has an x86 reading within LR_MATCH cs (and the reverse),
    with no more unmatched ones than the harness's two clocks have plus
    LR_UNMATCHED; the readings' differences (median and 90th percentile)
    within 4 times the harness's own plus a floor; LRALTFLG changes as often
    and at about the same times; HCALC as far from the true height."""
    h, o, x = (lr_history(t, at) for t in (ref, other, x86))
    block, ok, parts = {}, True, []
    for k, (_, _, rel, f50, f90) in LR_READINGS.items():
        hp, h_un = lr_pairs(h[0][k], o[0][k])
        xp, x_un = lr_pairs(h[0][k], x[0][k])
        (h50, h90), (x50, x90) = lr_quantiles(hp, rel), lr_quantiles(xp, rel)
        lim = (SPREAD_K * h50 + f50, SPREAD_K * h90 + f90, h_un + LR_UNMATCHED)
        good = x_un <= lim[2] and x50 <= lim[0] and x90 <= lim[1]
        ok = ok and good
        block[k] = dict(readings=dict(harness=len(h[0][k]), other_clock=len(o[0][k]), x86=len(x[0][k])),
                        unmatched=dict(other_clock=h_un, x86=x_un, limit=lim[2]),
                        median=dict(other_clock=round(h50, 4), x86=round(x50, 4), limit=round(lim[0], 4)),
                        p90=dict(other_clock=round(h90, 4), x86=round(x90, 4), limit=round(lim[1], 4)))
        unit = (lambda v: f'{100 * v:.2f}%') if rel else (lambda v: f'{v:.0f}')
        parts.append(f"{k} {len(x[0][k])} readings ({x_un} unmatched, limit {lim[2]}; median {unit(x50)}, "
                     f"limit {unit(lim[0])}; 90% {unit(x90)}, limit {unit(lim[1])})" + ('' if good else ' FAILED'))
    hf, of, xf = h[1], o[1], x[1]
    lim = [2 * abs(a[0] - b[0]) + LR_FLAG_CS if len(of) == len(hf) else 2 * LR_FLAG_CS for a, b in zip(hf, of)] \
        if len(of) == len(hf) else [2 * LR_FLAG_CS] * len(hf)
    good = [v for _, v in xf] == [v for _, v in hf] and all(abs(a[0] - b[0]) <= c for a, b, c in zip(hf, xf, lim))
    ok = ok and good
    block['LRALTFLG'] = dict(harness=hf, other_clock=of, x86=xf, ok=good)
    parts.append(f"LRALTFLG changes at {', '.join(str(t) for t, _ in xf) or 'none'} cs (the harness's "
                 f"{', '.join(str(t) for t, _ in hf) or 'none'})" + ('' if good else ' FAILED'))
    hp, h_un = lr_pairs(h[2], o[2])
    xp, x_un = lr_pairs(h[2], x[2])
    h_d, x_d = max((abs(a - b) for a, b in hp), default=0.0), max((abs(a - b) for a, b in xp), default=0.0)
    lim = SPREAD_K * h_d + LR_HCALC_M
    good = x_un <= h_un + LR_UNMATCHED and x_d <= lim
    ok = ok and good
    block['hcalc_error_m'] = dict(readings=len(x[2]), unmatched=x_un, unmatched_limit=h_un + LR_UNMATCHED,
                                  other_clock=round(h_d, 1), x86=round(x_d, 1), limit=round(lim, 1))
    parts.append(f"HCALC from the true height within {x_d:.1f} m of the harness's at each altitude reading "
                 f"(limit {lim:.1f}; {x_un} unmatched)" + ('' if good else ' FAILED'))
    return block, ok, 'landing radar: ' + ', '.join(parts)


def compare(name, m, cs, keys, env, drop='', resume=0):
    """One scenario: (summary row, passed). drop: key letters left out of
    the x86 run only (a check that the comparison sees them)."""
    modreg, rn, vn = STATE[m]
    lr = [] if 'FLIGHT_LR' in env else None
    logs = [] if name in TC_TRAP else None
    ref = harness_state(m, cs, keys, env, cs - 1000, out=lr, log=logs)
    if ref is None:
        return dict(scenario=name, mission=m, line=f'{name}: the harness wrote no snapshot'), False
    at, hw = ref
    other = harness_state(m, cs, keys, env, at, None if name in TC_TRAP else '1,1,2', out=lr, log=logs)
    r, xcs, xw = x86_state(m, ''.join(c for c in keys if c not in drop), with_start(m, cs, keys, env, resume), at,
                           lr is not None)
    if other is None or xcs != at:
        why = 'the harness on its other clock wrote no snapshot' if other is None else \
            f'the x86 program stopped before {at} cs (exit {r.returncode}): {r.stderr.strip()[:200]}'
        return dict(scenario=name, mission=m, centisecond=at, line=f'{name}: {why}'), False
    sw = other[1]
    state = {'rn': [vector(w, rn, 29, 1.0) for w in (hw, sw, xw)], 'vn': [vector(w, vn, 7, 100.0) for w in (hw, sw, xw)]}
    d_rn, s_rn = distance(state['rn'][0], state['rn'][2]), distance(state['rn'][0], state['rn'][1])
    d_vn, s_vn = distance(state['vn'][0], state['vn'][2]), distance(state['vn'][0], state['vn'][1])
    words = sum(hw[i] != xw[i] for i in WORDS)
    spread_words = sum(hw[i] != sw[i] for i in WORDS)
    rn_tol, vn_tol = SPREAD_K * s_rn + RN_FLOOR, SPREAD_K * s_vn + VN_FLOOR
    words_tol = min(int(WORDS_K * spread_words) + WORDS_PLUS, WORD_CAP.get(name, 1 << 30))
    truth = None
    h_err, o_err, x_err = (truth_errors(m, env, w) for w in (hw, sw, xw)) if 'FLIGHT_RR' in env else (None,) * 3
    if h_err and h_err[2] != o_err[2]:
        d_rn, s_rn, d_vn, s_vn = x_err[0], max(h_err[0], o_err[0]), x_err[1], max(h_err[1], o_err[1])
        rn_tol, vn_tol = SPREAD_K * s_rn + RN_FLOOR, SPREAD_K * s_vn + VN_FLOOR
        truth = dict(pip_cs=dict(harness=h_err[2], other_clock=o_err[2], x86=x_err[2]),
                     rn_error_m=dict(harness=round(h_err[0], 1), other_clock=round(o_err[0], 1), x86=round(x_err[0], 1)),
                     vn_error_mps=dict(harness=round(h_err[1], 3), other_clock=round(o_err[1], 3),
                                       x86=round(x_err[1], 3)))
    # The alarm codes (FAILREG, +1, +2) wherever the harness's two clocks
    # agree on them: an alarm the harness raised that the x86 run did not
    # (LM R22's 521 in an RR dropout), or the reverse.
    alarms = [a for a in FAILREG if hw[a] == sw[a] and xw[a] != hw[a]]
    ok = (r.returncode == 0 and hw[modreg] == xw[modreg] and d_rn <= rn_tol and d_vn <= vn_tol and words <= words_tol
          and not alarms)
    blocks, block_line = None, ''
    if 'FLIGHT_RR' in env:
        blocks = {}
        for veh in BLOCKS:
            h_o, o_o, x_o = (block_errors(veh, env, w) for w in (hw, sw, xw))
            o_rn_tol = max(h_o[0], o_o[0]) + RN_FLOOR
            o_vn_tol = max(h_o[1], o_o[1]) + VN_FLOOR
            o_tet_min = min(h_o[2], o_o[2]) - TET_BEHIND
            ok = ok and x_o[0] <= o_rn_tol and x_o[1] <= o_vn_tol and x_o[2] >= o_tet_min
            blocks[veh] = dict(tet_cs=dict(harness=h_o[2], other_clock=o_o[2], x86=x_o[2], at_least=o_tet_min),
                               rn_error_m=dict(harness=round(h_o[0], 1), other_clock=round(o_o[0], 1),
                                               x86=round(x_o[0], 1), limit=round(o_rn_tol, 1)),
                               vn_error_mps=dict(harness=round(h_o[1], 3), other_clock=round(o_o[1], 3),
                                                 x86=round(x_o[1], 3), limit=round(o_vn_tol, 3)))
            block_line += (f"; {veh.upper()} block at {x_o[2]} cs (at least {o_tet_min}; the harness's {h_o[2]}, "
                           f"{o_o[2]}), {x_o[0]:.1f} m from the true orbit (limit {o_rn_tol:.0f}), {x_o[1]:.2f} m/s "
                           f"(limit {o_vn_tol:.2f})")
    if truth:
        line = (f"{name}: at {at} cs major mode {hw[modreg]:02o}/{xw[modreg]:02o} (harness/x86), "
                f"RN {d_rn:.1f} m from the true orbit (limit {rn_tol:.0f}), VN {d_vn:.2f} m/s (limit {vn_tol:.2f}), "
                f"{words} words differ (limit {words_tol}; the harness's errors: RN {s_rn:.1f} m, VN {s_vn:.3f} m/s; "
                f"its clocks differ in {spread_words} words)")
    else:
        line = (f"{name}: at {at} cs major mode {hw[modreg]:02o}/{xw[modreg]:02o} (harness/x86), "
                f"RN {d_rn:.1f} m apart (limit {rn_tol:.0f}), VN {d_vn:.2f} m/s (limit {vn_tol:.2f}), "
                f"{words} words differ (limit {words_tol}; the harness's clocks: RN {s_rn:.1f} m, VN {s_vn:.3f} m/s, "
                f"{spread_words} words)")
    line += block_line
    if alarms:
        line += '; alarm codes differ: ' + ', '.join(f'{a:04o} {hw[a]:05o}/{xw[a]:05o}' for a in alarms)
    # Checks a KNOWN envelope never excuses: the restart count and the TC trap.
    checks_ok, restarts = True, None
    if 'FLIGHT_GOJAM' in env or name in TC_TRAP:
        c = REDOCTR[m]
        restarts = dict(harness=hw[c], other_clock=sw[c], x86=xw[c])
        same = xw[c] in (hw[c], sw[c])
        ok, checks_ok = ok and same, checks_ok and same
        line += f'; REDOCTR {hw[c]}/{xw[c]}' + ('' if same else ' (restarts differ)')
    trap = None
    if logs is not None:
        trap, trap_ok, trap_line = trap_compare(logs, r.stdout)
        ok, checks_ok = ok and trap_ok, checks_ok and trap_ok
        line += '; ' + trap_line
    radar = None
    if lr is not None:
        radar, lr_ok, lr_line = lr_compare(lr[0], lr[1], r.stdout, at)
        ok = ok and lr_ok
        line += '; ' + lr_line
    return dict(scenario=name, mission=m, centisecond=at, x86_exit=r.returncode,
                major_mode=dict(harness=f'{hw[modreg]:02o}', x86=f'{xw[modreg]:02o}'),
                rn_metres=dict(harness=[round(v, 1) for v in state['rn'][0]], x86=[round(v, 1) for v in state['rn'][2]],
                               difference=round(d_rn, 1), spread=round(s_rn, 1), limit=round(rn_tol, 1)),
                vn_mps=dict(harness=[round(v, 2) for v in state['vn'][0]], x86=[round(v, 2) for v in state['vn'][2]],
                            difference=round(d_vn, 3), spread=round(s_vn, 3), limit=round(vn_tol, 3)),
                words=dict(differ=words, spread=spread_words, limit=words_tol), truth=truth, state_blocks=blocks,
                alarms=dict(harness=[f'{hw[a]:05o}' for a in FAILREG], x86=[f'{xw[a]:05o}' for a in FAILREG],
                            differ=[f'{a:04o}' for a in alarms]), landing_radar=radar, tc_trap=trap, redoctr=restarts, checks_ok=checks_ok, inputs=env,
                line=line), ok


def fly_gap(name, m, cs, keys, env, g=None, start=0):
    """A GAPS scenario: (summary row, passed), passed only when the x86
    program stops at the expected routine and nowhere else (from the
    harness's state at start, when given)."""
    g = g or GAPS[name]
    r = subprocess.run([str(xb.OUT / f'x86-flight-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                       env={**os.environ, **with_start(m, cs, keys, env, start), 'X86_SPEED': '0'},
                       capture_output=True, text=True)
    got = r.stderr.strip()
    want = g.get('stop') or g['point'] + ' at N cs'
    stopped = got == g['stop'] if 'stop' in g else re.fullmatch(re.escape(g['point']) + r' at \d+ cs', got)
    ok = r.returncode == 1 and bool(stopped) and g['refusal'] in r.stdout + '\n'
    line = (f"{name}: stops as expected ({got}): {g['reason']}" if ok else
            f"{name}: expected exit 1 with \"{want}\", got exit {r.returncode}: {got[:200] or 'no stop line'}")
    row = dict(scenario=name, mission=m, gap=g['reason'], x86_exit=r.returncode, ok=ok, line=line)
    if ok and g.get('then'):
        rest, ok = fly_gap(name, m, cs, keys, env, g['then'], g['resume'])
        row.update(resumed=dict(rest, from_cs=g['resume']), ok=ok)
        row['line'] = line + f"; from the harness's state at {g['resume']} cs: " + rest['line'].split(': ', 1)[1]
    elif ok and g.get('resume'):
        rest, ok = compare(name, m, cs, keys, env, resume=g['resume'])
        row.update(resumed=dict(rest, from_cs=g['resume']), ok=ok)
        row['line'] = line + f"; from the harness's state at {g['resume']} cs: " + rest['line'].split(': ', 1)[1]
    return row, ok


def within_known(name, row):
    """The KNOWN reason when the row stays inside that scenario's envelope
    (and every other check passes), else None."""
    k = KNOWN.get(name)
    if not k or 'words' not in row or row['x86_exit'] != 0:
        return None
    mm, w = row['major_mode'], row['words']
    blocks = all(b['rn_error_m']['x86'] <= b['rn_error_m']['limit'] and b['vn_error_mps']['x86'] <= b['vn_error_mps']['limit']
                 and b['tet_cs']['x86'] >= b['tet_cs']['at_least'] for b in (row.get('state_blocks') or {}).values())
    # Alarm codes may differ only where the envelope names the x86 run's code.
    allowed = k.get('alarms', {})
    alarms = all(allowed.get(int(a, 8)) == int(row['alarms']['x86'][FAILREG.index(int(a, 8))], 8)
                 for a in row['alarms']['differ'])
    rn, vn = k.get('rn', row['rn_metres']['limit']), k.get('vn', row['vn_mps']['limit'])
    inside = (mm['harness'] == mm['x86'] and w['differ'] <= k.get('words', w['limit']) and blocks and alarms
              and row['rn_metres']['difference'] <= rn and row['vn_mps']['difference'] <= vn
              and row['checks_ok'] and radar_within(row['landing_radar'], k.get('radar', {})))
    words = f", {k['words']} words" if 'words' in k else ''
    codes = ''.join(f', {a:04o} {v:05o}' for a, v in allowed.items())
    lr = ''.join(f', {r} {n} unmatched' for r, n in k.get('radar', {}).items())
    return f"{k['reason']} (envelope RN {rn:.0f} m, VN {vn} m/s{words}{codes}{lr})" if inside else None


def radar_within(block, unmatched):
    """The landing radar check passes, except that a reading named in
    unmatched may have up to that many x86 readings with no harness reading
    near them (each one's median and 90th percentile still within limits)."""
    if block is None:
        return True
    readings = all(b['unmatched']['x86'] <= max(b['unmatched']['limit'], unmatched.get(r, 0))
                   and b['median']['x86'] <= b['median']['limit'] and b['p90']['x86'] <= b['p90']['limit']
                   for r, b in block.items() if r in LR_READINGS)
    h = block['hcalc_error_m']
    hcalc = h['x86'] <= h['limit'] and h['unmatched'] <= max(h['unmatched_limit'], unmatched.get('HCALC', 0))
    return readings and hcalc and block['LRALTFLG']['ok']


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-build', action='store_true', help='reuse the built harness and x86 program')
    p.add_argument('--drop-keys', default='', metavar='CHARS',
                   help='leave these key letters out of the x86 run only, e.g. MJxyj (the mark buttons): should fail')
    p.add_argument('names', nargs='*', help='scenarios (default: every one the x86 program supports)')
    a = p.parse_args()
    rows = [r for r in supported() if not a.names or r[0] in a.names]
    if not a.skip_build:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
        engine = fhd.frozen_engine(a.host_cc)
        for m in ('lm', 'cm'):
            fhd.build(a.host_cc, m, engine)
            xb.build(a.host_cc, m, 'compiled')
    OUT.mkdir(parents=True, exist_ok=True)
    summary, failed = [], 0
    try:
        for name, m, cs, keys, env in rows:
            if name in GAPS:
                row, ok = fly_gap(name, m, cs, keys, env)
                failed += not ok
                summary.append(row)
                print(row['line'] + ('' if ok else ' FAILED'), flush=True)
                continue
            row, ok = compare(name, m, cs, keys, env, a.drop_keys)
            known = None if ok else within_known(name, row)
            failed += not ok and not known
            row.update(ok=ok, known=known)
            summary.append(row)
            print(row['line'] + ('' if ok else f' KNOWN: {known}' if known else ' FAILED'), flush=True)
        (OUT / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    finally:
        for start in OUT.glob(f'start-*-{os.getpid()}.h'):   # the core dump's state is not kept
            start.unlink()
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
