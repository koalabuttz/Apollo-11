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
KNOWN lists scenarios outside the RN/VN bounds with the reason and an
envelope of their own: such a run is reported and does not fail when it
reaches the compare point, exits 0, has the same major mode, stays within
the word limit and within the envelope; anything else fails. Results go to
build/x86-flight/scenarios/summary.json.

  x86_flight_scenarios.py [--host-cc CC] [--skip-build] [--drop-keys CHARS] [NAME ...]

Scenarios that need a model the x86 program does not have yet (the
landing radar, the NASSP P63 core, uplink, stolen time) are left out, and
GAPS lists those that stop at a routine not yet certified: they pass only
when they stop exactly there. Needs yaAGC (deps/virtualagc) for the
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
# The gyrocompass loop polls the liftoff discrete, so when the AGC sees
# liftoff depends on its clock schedule: TLIFTOFF is 10,005 cs on the
# harness's work clock, 10,022 on its other clock and 10,038 on the x86
# program's (FLIGHT_SATURN lifts off at 10,000). P11 navigates from the
# time it saw, so the offset is in the state vector from liftoff on
# (149 m at 10,051 cs against a spread of 74 m) and grows to orbit.
KNOWN = {'cm-p11': dict(reason='the x86 program sees liftoff 33 cs after the harness (TLIFTOFF 10038 against 10005; '
                                "the harness's other clock: 10022), and P11 navigates from that time",
                         rn=600.0, vn=1.0),   # measured 449 m, 0.73 m/s
         # The cage (channel 30 at 3,000 cs) reaches the DAP at the x86
         # program's sample point 3,000 and at the harness's 3,002 (busy at
         # 3,000 and 3,001 on both its clocks), so the DAP counts one trap
         # fewer (NPTRAPS, NQTRAPS) and its jet words differ after the CDU
         # jump at 5,800. The state vector is the same.
         'lm-p52-imu-cage': dict(reason="the cage reaches the DAP at the x86 program's sample point 3,000 cs and the "
                                        "harness's 3,002, so the DAP's trap counts and jet words differ",
                                 rn=50.0, vn=0.15, words=32)}   # measured 26 words against 25
UNSUPPORTED = ('FLIGHT_UPLINK', 'FLIGHT_STEAL')
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
# and compared as any scenario.
DECDSP = dict(reason="LM P33's decimal display (DECDSP, 41,2523) is in Pinball, whose LM certificate is pending "
                     "(Lane C)",
              stop='no routine at basic 41,2523 (Z 02523 FB 62000) at 2508 cs',
              refusal='basic  41,2523: 1 arrivals, 1 slots, refused by lanec_pin_lm_routines')
# BURN_BABY_BURN's EXIT at 36,2236 (P63, MM 63 at TIG-35) leaves Z at
# 36,2237, a basic word lm_burn_baby's flight build does not key: since
# the FLIGHT_INTERP_STOP files build as flight builds (#739), the
# interpreter runs that EXIT, and tools/flight_exit_returns.py misses the
# word (it names the listing file 'BURN,_BABY,_BURN_--...'). The resume
# point is after the stop and before any engine or radar starts.
BURN_EXIT = dict(reason="lm_burn_baby keys no EXIT return at 36,2237: tools/flight_exit_returns.py names BURN_BABY_BURN's "
                        "listing file wrongly (sent to the steward)",
                 point='no routine at basic 36,2237 (Z 02237 FB 74000)', refusal='basic  36,2237: 1 arrivals, 1 slots\n',
                 resume=20000)
P47_PINBALL = dict(reason="LM P47's display (Pinball, 41,3321) is in Pinball, whose LM certificate is pending (Lane C)",
                   stop='no routine at basic 41,3321 (Z 03321 FB 62000) at 4120 cs',
                   refusal='basic  41,3321: 1 arrivals, 1 slots, refused by lanec_pin_lm_routines')
GAPS = {'lm-cdh': DECDSP, 'lm-cdh-csm': DECDSP, 'lm-p47-nassp': P47_PINBALL,
        **{n: BURN_EXIT for n in ('lm-p63-descent', 'lm-p63-descent-thrust', 'lm-p63-descent-restart', 'lm-p63-nassp',
                                  'lm-p63-v60-nassp', 'lm-p63-lr-dropout', 'lm-p63-lr-update', 'lm-p63-lr-v58',
                                  'lm-p65-autoland-nassp', 'lm-p67-nassp', 'lm-landing-ascent-nassp',
                                  'lm-abort-p70-nassp', 'lm-abort-p71-nassp')}}
# Each mission's MODREG, RN and VN (metres at 2^29, m/cs at 2^7), PIPTIME.
STATE = {'lm': (0o1011, 0o1220, 0o1226), 'cm': (0o1011, 0o1170, 0o1176)}
PIPTIME = {'lm': 0o1234, 'cm': 0o1204}
# The two vehicles' permanent state blocks (RRECTCSM ... and RRECTLEM ...:
# RRECT, VRECT, TET at +014, DELTA +016, NUV +024, RCV +032, VCV +040), at
# the same addresses on both missions. Which one a mark updates is
# VEHUPFLG's choice (the LM's own block by default on the LM, the LM's on
# the CM), so both are checked.
BLOCKS = {'csm': 0o1554, 'lm': 0o1626}
MOON_MU = 4.9028e12


def supported():
    return [row for row in fhd.FLIGHT if not any(k in row[4] for k in UNSUPPORTED)]


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


def harness_state(m, cs, keys, env, at, clock='W'):
    """The harness's native erasable image at the first sample point at or
    after centisecond at, on the given clock (FLIGHT_SCALED): (centisecond,
    2048 words), or None when it wrote no snapshot."""
    snap = OUT / f'snap-{os.getpid()}.h'
    subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                   env={**os.environ, **env, 'FLIGHT_SCALED': clock, 'FLIGHT_SNAPSHOT': f'{at},{snap}'},
                   capture_output=True, check=False)
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


def x86_state(m, keys, env, at):
    """The x86 program's erasable image at its first sample point at or after centisecond at:
    (run, centisecond, 2048 words), centisecond None when it wrote none."""
    samples = OUT / f'samples-{os.getpid()}.txt'
    r = subprocess.run([str(xb.OUT / f'x86-flight-{m}'), fhd.ROPE[m], str(at + 1000)] + ([keys] if keys else []), cwd=ROOT,
                       env={**os.environ, **env, 'X86_SPEED': '0', 'X86_SAMPLES': str(samples),
                            'X86_SAMPLES_FROM': str(at)}, capture_output=True, text=True)
    line = samples.read_text().split('\n', 1)[0].split() if samples.exists() else []
    if samples.exists():
        samples.unlink()
    if len(line) != 2049:
        return r, None, None
    return r, int(line[0]), [int(v, 8) for v in line[1:]]


def distance(a, b):
    return sum((x - y) ** 2 for x, y in zip(a, b)) ** 0.5


def compare(name, m, cs, keys, env, drop='', resume=0):
    """One scenario: (summary row, passed). drop: key letters left out of
    the x86 run only (a check that the comparison sees them)."""
    modreg, rn, vn = STATE[m]
    ref = harness_state(m, cs, keys, env, cs - 1000)
    if ref is None:
        return dict(scenario=name, mission=m, line=f'{name}: the harness wrote no snapshot'), False
    at, hw = ref
    other = harness_state(m, cs, keys, env, at, '1,1,2')
    r, xcs, xw = x86_state(m, ''.join(c for c in keys if c not in drop), with_start(m, cs, keys, env, resume), at)
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
    words_tol = int(WORDS_K * spread_words) + WORDS_PLUS
    truth = None
    h_err, o_err, x_err = (truth_errors(m, env, w) for w in (hw, sw, xw)) if 'FLIGHT_RR' in env else (None,) * 3
    if h_err and h_err[2] != o_err[2]:
        d_rn, s_rn, d_vn, s_vn = x_err[0], max(h_err[0], o_err[0]), x_err[1], max(h_err[1], o_err[1])
        rn_tol, vn_tol = SPREAD_K * s_rn + RN_FLOOR, SPREAD_K * s_vn + VN_FLOOR
        truth = dict(pip_cs=dict(harness=h_err[2], other_clock=o_err[2], x86=x_err[2]),
                     rn_error_m=dict(harness=round(h_err[0], 1), other_clock=round(o_err[0], 1), x86=round(x_err[0], 1)),
                     vn_error_mps=dict(harness=round(h_err[1], 3), other_clock=round(o_err[1], 3),
                                       x86=round(x_err[1], 3)))
    ok = r.returncode == 0 and hw[modreg] == xw[modreg] and d_rn <= rn_tol and d_vn <= vn_tol and words <= words_tol
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
    return dict(scenario=name, mission=m, centisecond=at, x86_exit=r.returncode,
                major_mode=dict(harness=f'{hw[modreg]:02o}', x86=f'{xw[modreg]:02o}'),
                rn_metres=dict(harness=[round(v, 1) for v in state['rn'][0]], x86=[round(v, 1) for v in state['rn'][2]],
                               difference=round(d_rn, 1), spread=round(s_rn, 1), limit=round(rn_tol, 1)),
                vn_mps=dict(harness=[round(v, 2) for v in state['vn'][0]], x86=[round(v, 2) for v in state['vn'][2]],
                            difference=round(d_vn, 3), spread=round(s_vn, 3), limit=round(vn_tol, 3)),
                words=dict(differ=words, spread=spread_words, limit=words_tol), truth=truth, state_blocks=blocks, inputs=env,
                line=line), ok


def fly_gap(name, m, cs, keys, env):
    """A GAPS scenario: (summary row, passed), passed only when the x86
    program stops at the expected routine and nowhere else."""
    g = GAPS[name]
    r = subprocess.run([str(xb.OUT / f'x86-flight-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                       env={**os.environ, **with_start(m, cs, keys, env), 'X86_SPEED': '0'}, capture_output=True,
                       text=True)
    got = r.stderr.strip()
    want = g.get('stop') or g['point'] + ' at N cs'
    stopped = got == g['stop'] if 'stop' in g else re.fullmatch(re.escape(g['point']) + r' at \d+ cs', got)
    ok = r.returncode == 1 and bool(stopped) and g['refusal'] in r.stdout + '\n'
    line = (f"{name}: stops as expected ({got}): {g['reason']}" if ok else
            f"{name}: expected exit 1 with \"{want}\", got exit {r.returncode}: {got[:200] or 'no stop line'}")
    row = dict(scenario=name, mission=m, gap=g['reason'], x86_exit=r.returncode, ok=ok, line=line)
    if ok and g.get('resume'):
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
    inside = (mm['harness'] == mm['x86'] and w['differ'] <= k.get('words', w['limit'])
              and row['rn_metres']['difference'] <= k['rn'] and row['vn_mps']['difference'] <= k['vn'])
    words = f", {k['words']} words" if 'words' in k else ''
    return f"{k['reason']} (envelope RN {k['rn']:.0f} m, VN {k['vn']} m/s{words})" if inside else None


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
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
