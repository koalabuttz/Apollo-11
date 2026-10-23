#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write one program's device_adapter_tests evidence
(docs/PORT_SCOPE.md, integrated_applications).

  device_acceptance.py lm|cm      # runs everything it needs at HEAD
  device_acceptance.py lm|cm [--host SUMMARY.json] [--run RUN.json ...]

Every input role the program reads (time, IMU CDUs and PIPAs, hand
controller, optics or AOT marks, radars, discretes, DSKY keys, uplink) and
every output it drives (jets and engine, thrust, CDU and gyro drives,
radar and optics drives, displays and meters, downlink) gets one row:

  adapters   the AGC interface, the host adapter and the C64 adapter, each a
             file and a symbol that must be defined there at this commit (a
             function, a label or a #define, outside comments and strings).
             On the C64 the replay row injector is the adapter of every
             input role in a window: the host's inputs reach the image as
             rows.
  host       every whole-program host flight scenario that drives the role
             (selected by the FLIGHT_* inputs or keys it is given, from
             tools/flight_host_differential.py's summary, scaled time x1)
             must be in the summary, from this commit, and clean: exit 0
             and every native routine matched yaAGC.
  C64        one of the program's WINDOWS (tools/flight_scenario_vice.py
             --replay on stock C64 + REU in VICE) that matches the host and
             exercises the role: for an input, its host rows touch the
             role's words, channel bits or interrupt requests; for an
             output, the role's channel bits or words changed in the host's
             states over the window (replay_outputs); a bit that is only
             held counts only as an enable gate.

A window matches when the run is ok with status 0, every REU DMA stayed
inside the layout, it ended with every input row applied, its end and
every checkpoint were paired and compared with no skips and no
differences, its last compared checkpoint is its end, it covered at least
the window's centiseconds, and it is from this commit.

Result FAIL (exit 1), the doc still written, when an adapter or make
target is missing, a host scenario is missing, stale or not clean, a role
has no host scenario or no matching C64 window, a role has a blocking
open item (a model that does not run), or a run is not one of the
program's windows. Writes, all from the same commit:

  docs/acceptance/<Program>-device_adapter_tests.md
  evidence/<m>-acceptance-devices/*.json   (each run and the program's host rows,
                                           tree-stored with tools/evidence.py add --tree)
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
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402

PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
# include/resident_rupt.h: RK_RUPT_T6 = 1, T5, T3, T4, KEY1, KEY2, UP, DOWN, RADAR, HAND.
RUPT = {name: 1 << (i + 1) for i, name in enumerate(('T6', 'T5', 'T3', 'T4', 'KEY1', 'KEY2', 'UP', 'DOWN', 'RADAR', 'HAND'))}
FHR = 'tools/flight_host_run.c'
DPC = 'platform/c64/dap_pass_check.c'
RF = 'platform/c64/resident_flight.c'
RRU = 'src/resident_rupt.c'

# The C64 replay windows a bare run takes (scenario, snapshot cs, window cs,
# checkpoint every cs), each chosen for the roles whose inputs or outputs
# move in it.
WINDOWS = {
    'lm': [('lm-p63-descent-thrust', 17500, 120, 120),   # PIPAs and THRUST under descent thrust
           ('lm-landing-ascent-nassp', 140000, 150, 50),   # touchdown, engine arm, PROCEED
           ('lm-uplink-reject', 1900, 800, 100),            # uplink words (UPRUPT) and DSKY keys
           ('lm-p52-realign', 4200, 500, 100),              # AOT marks (channel 16, KEYRUPT2)
           ('lm-p57', 15950, 100, 25),                      # lunar-surface AOT mark
           ('lm-p63-lr-update', 61000, 50, 10)],            # P63 descent: jets, landing displays
    'cm': [('cm-p52', 9950, 300, 100),                      # sextant marks, channel 33
           ('cm-tei', 4450, 300, 100),                      # RCS DAP: jets, channels and requests
           ('cm-tei', 52000, 300, 50),                      # TEI coast: jets and TIME6
           ('cm-p40', 70660, 100, 50)],                     # SPS burn
}


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def unit(m, *names):
    """The make targets of routine-level checks: the host differential and
    the C64 (VICE) run of each milestone."""
    pre = f'{m}-' if m else ''
    return [t for n in names for t in (f'{pre}{n}-differential', f'c64-{pre}{n}')]


def make_targets():
    text = '\n'.join(p.read_text() for p in [ROOT / 'Makefile', *sorted((ROOT / 'tools').glob('**/*.mk'))])
    return {t for line in re.findall(r'^([a-z0-9][a-z0-9_. -]*):', text, re.M) for t in line.split()}


# What a window's host rows touched (flight_scenario_vice.py replay_inputs)
# and what its outputs did (replay_outputs).
def inputs(r):
    return r.get('replay_inputs') or {}


def outputs(r):
    return r.get('replay_outputs') or {}


def any_window(r):
    return True


def erasable(*addrs):
    return lambda r: bool(set(inputs(r).get('erasable', [])) & {f'{a:04o}' for a in addrs})


def pipas(r):
    return bool(inputs(r).get('pipas'))


def requests(*names):
    return lambda r: bool(inputs(r).get('requests', 0) & sum(RUPT[n] for n in names))


def channel(ch, mask=0o77777, how='channel_changed'):
    """A host row of channel ch changed (or, how='channel_set', set; 'channel_clear',
    cleared) one of mask's bits."""
    return lambda r: bool(inputs(r).get(how, {}).get(f'{ch:o}', 0) & mask)


def held(ch, mask=0o77777):
    """Output channel ch (an END_CHANNELS name's number) had one of mask's
    bits set in the host's states over the window: an enable, not a drive."""
    return lambda r: bool(outputs(r).get('channel_set', {}).get(f'channel {ch:o}', 0) & mask)


def steady(ch, mask):
    """Output channel ch had all of mask's bits set from the window's start
    to its end, never changing: an enable held through the window."""
    return lambda r: (outputs(r).get('channel_set', {}).get(f'channel {ch:o}', 0) & mask == mask
                      and not outputs(r).get('channel_changed', {}).get(f'channel {ch:o}', 0) & mask)


def never(pred):
    return lambda r: not pred(r)


def moved(ch, mask=0o77777):
    """Output channel ch changed one of mask's bits over the window."""
    return lambda r: bool(outputs(r).get('channel_changed', {}).get(f'channel {ch:o}', 0) & mask)


def words(*addrs):
    """An output word changed over the window (a value that only sits there
    shows nothing was driven)."""
    want = {f'{a:04o}' for a in addrs}
    return lambda r: bool(want & set(outputs(r).get('changed', [])))


def scenario_env(*names, without=()):
    """The window's scenario runs with one of these FLIGHT_* inputs (and none of without)."""
    return lambda r: bool(set(r.get('_env', {})) & set(names)) and not set(r.get('_env', {})) & set(without)


def either(*preds):
    return lambda r: any(p(r) for p in preds)


def both(*preds):
    return lambda r: all(p(r) for p in preds)


def uses(*names):
    """Host scenarios given one of these FLIGHT_* inputs."""
    return lambda env, keys: bool(set(env) & set(names))


def chan_input(*chs):
    """Host scenarios whose FLIGHT_CHAN changes one of these channels (or starts it, FLIGHT_CHnn)."""
    return lambda env, keys: (any(re.search(rf':0?{c:o}=', env.get('FLIGHT_CHAN', '')) for c in chs)
                              or any(f'FLIGHT_CH{c:o}' in env for c in chs))


def pressed(chars):
    """Host scenarios whose key string presses one of chars."""
    return lambda env, keys: bool(re.search(f'[{chars}]', keys or ''))


def role(key, name, direction, agc, host, c64, env, window, units, gap=None, closes=None, blocking=False,
         elsewhere=None, how=None):
    """host/c64: [(file, symbol, what)], each symbol defined in file; env:
    which host scenarios drive the role, f(env, keys) (None: every one);
    window: which replay windows exercise it, f(run); gap: an open item;
    blocking: the gap keeps the role from being covered at all; elsewhere:
    the acceptance doc that covers the role instead; how: what the window
    predicate looks for, for the doc."""
    return dict(key=key, name=name, dir=direction, agc=agc, host=host, c64=c64, env=env, window=window,
                units=units, gap=gap, closes=closes, blocking=blocking, elsewhere=elsewhere, how=how)


def roles(m):
    lm = m == 'lm'
    rows = (DPC, 'replay_rows', 'the replay row injector: the host\'s input rows (kinds 0-3) into the image, '
            'rk_hardware, rk_device and rk_pending at their sample points')
    end = (DPC, 'replay_state', 'writes the C64\'s state at each checkpoint, compared with the host\'s')
    host_end = (FHR, 'rp_end', 'writes the host\'s state at each checkpoint: erasable words and the output channels')
    radar_read = either(erasable(0o46), channel(0o377, how='channel_set'), requests('RADAR'))
    out = [
        role('time', 'Elapsed time', 'in', 'TIME1-TIME6, SCALER1/2 (channels 3, 4), T3RUPT-T6RUPT',
             [(RRU, 'rk_timer_phase', 'the timer model, linked into the host run (FLIGHT_SCALED schedule)')],
             [(RF, 'rk_rupts_due', 'charges each step\'s AGC time and steps the same rk_timer_phase'),
              (RRU, 'rk_timer_phase', 'TIME1-6, the scalers, the T6 counter and the radar gate')],
             None, any_window, ['test-resident-rupt', 'vice-resident-flight'],
             how='every window: the C64 runs its own timers and the compare includes TIME1-6',
             gap='TIME5 a count apart is a temporary replay skip (a window with it does not match)',
             closes='RK\'s work-clock fixes; then drop TIME5_PHASE from tools/flight_scenario_vice.py'),
        role('imu', 'IMU: CDUs and PIPAs', 'in', 'CDUX-CDUZ (032-034), PIPAX-PIPAZ (037-041)',
             [(FHR, 'engine_due' if lm else 'sps_due', 'engine model: PIPA counts along the thrust'),
              (FHR, 'rcs_due' if lm else 'aero_due', 'RCS translation (LM) or aerodynamic drag (CM) into the PIPAs')]
             + ([] if lm else [(FHR, 'imu_due', 'FLIGHT_IMU: channel 12 zero and coarse align move CDUX-Z'),
                               (FHR, 'saturn_due', 'FLIGHT_SATURN boost'), (FHR, 'sivb_due', 'FLIGHT_SIVB TLI burn')]),
             [rows], uses('FLIGHT_ENGINE', 'FLIGHT_RCS', 'FLIGHT_POKE') if lm else
             uses('FLIGHT_SPS', 'FLIGHT_AERO', 'FLIGHT_SATURN', 'FLIGHT_SIVB', 'FLIGHT_IMU'),
             either(pipas, erasable(0o32, 0o33, 0o34)), unit(m, 't4rupt-imu'),
             how='PIPA counts, or CDUX-Z words, in the host rows',
             gap='no attitude dynamics from the jets: the models hold CDUX-Z at the commanded attitude' + (
                 ' (the descent engine moves nothing)' if lm else ' (only the SPS gimbal turns the CM, in pitch and yaw)'),
             blocking=True,
             closes='Lane B: a rigid-body model fed by the jets and engine, on both sides'),
    ]
    if lm:
        out += [
            role('rhc', 'Hand controller (RHC)', 'in', 'P-RHCCTR, Q-RHCCTR, R-RHCCTR (042-044), HANDRUPT, '
                 'channel 13 bit 8 (RHC counter enable)', [], [rows], uses('FLIGHT_RHC'),
                 either(erasable(0o42, 0o43, 0o44), requests('HAND')), unit(m, 'p-axis'),
                 how='RHC counter words or HANDRUPT in the host rows',
                 gap='no hand controller model on the host and no scenario drives the RHC counters or HANDRUPT',
                 blocking=True, closes='an RHC model (counts into 042-044, HANDRUPT) and a P66 or attitude-hold window'),
            role('aot', 'AOT marks', 'in', 'channel 16 bits 3-5 (X MARK, Y MARK, MARK REJECT), KEYRUPT2',
                 [(FHR, 'key_due', 'keys x, y and j: channel 16 bits 3, 4 and 5 with KEYRUPT2'),
                  (FHR, 'MARK_KEY', 'the mark keys\' flag')],
                 [rows], pressed('xyj'), channel(0o16, 0o34, 'channel_set'), unit(m, 'aotmark'),
                 how='a channel 16 row with bit 3, 4 or 5 set',
                 gap='the C64 flight disk has no MARK key (resident_dsky.S has no channel 16 path); in a window '
                     'the mark arrives as a replay row',
                 closes='a channel 16 key in rk_dsky_keys'),
            role('rod', 'Rate-of-descent switch', 'in', 'channel 16 bits 6-7 (ROD +/-), MARKRUPT (KEYRUPT2) in P66',
                 [(FHR, 'key_due', 'keys M and J: channel 16 bits 6 and 7 with KEYRUPT2')],
                 [rows], pressed('MJ'), channel(0o16, 0o140, 'channel_set'), unit(m, 'lunar-landing'),
                 how='a channel 16 row with bit 6 or 7 set'),
            role('rr', 'Rendezvous radar', 'in', 'RNRAD (046) with channel 13 selecting the RR, channel 33 bit 4 '
                 '(RR data good), RADARUPT',
                 [('tools/flight_radar.h', 'rr_due', 'RR model: line of sight, range and range rate, data good, '
                   'FLIGHT_RR_DROP')],
                 [(RRU, 'radar', 'the radar read cycle and RADARUPT on the AGC clock'), rows],
                 uses('FLIGHT_RR', 'FLIGHT_RR_DROP'),
                 either(channel(0o33, 0o10, 'channel_clear'), both(radar_read, scenario_env('FLIGHT_RR', without=('FLIGHT_LR',)))),
                 unit(m, 't4rupt-rr', 'radar-leadin-routines', 'p20-p25'),
                 how='RR data good (channel 33 bit 4 clear) in a host row, or radar reads in a scenario that runs the RR only'),
            role('rr-cdus', 'RR CDUs', 'in', 'CDUT/CDUS (035/036, RR trunnion and shaft)',
                 [('tools/flight_radar.h', 'rr_due', 'RR model: trunnion and shaft angles of the line of sight')],
                 [rows], uses('FLIGHT_RR', 'FLIGHT_RR_DROP'), erasable(0o35, 0o36), unit(m, 't4rupt-rr'),
                 how='CDUT or CDUS words in the host rows'),
            role('lr', 'Landing radar', 'in', 'RNRAD (046) with channel 13 selecting the LR, channel 33 bits 5-9 '
                 '(LR data good, range scale, antenna position), RADARUPT',
                 [('tools/flight_lr.h', 'lr_due', 'LR antenna position'),
                  ('tools/flight_lr.h', 'lrm_due', 'LR model: altitude and velocity beams, data good, FLIGHT_LR_DROP')],
                 [(RRU, 'radar', 'the radar read cycle and RADARUPT on the AGC clock'), rows],
                 uses('FLIGHT_LR', 'FLIGHT_LR_DROP'),
                 either(channel(0o33, 0o220, 'channel_clear'), channel(0o33, 0o540),
                        both(radar_read, scenario_env('FLIGHT_LR', without=('FLIGHT_RR',)))),
                 unit(m, 'radar-leadin-routines', 'lunar-landing'),
                 how='LR data good (channel 33 bit 5 or 8 clear) or range scale or antenna bits in a host row, or radar '
                     'reads in a scenario that runs the LR only'),
        ]
    else:
        out += [
            role('optics', 'Sextant marks', 'in', 'CDUS/CDUT (035/036, sextant), channel 16 bits 6-7 (MARK, MARK '
                 'REJECT), KEYRUPT2',
                 [(FHR, 'load_parse', 'FLIGHT_LOAD: star and sextant angles from tools/flight_scenarios/*.load'),
                  (FHR, 'key_due', 'keys M and J: channel 16 bits 6 and 7 with KEYRUPT2')],
                 [rows], pressed('MJ'), either(channel(0o16, 0o140, 'channel_set'), erasable(0o35, 0o36)),
                 unit(m, 'sxtmark', 't4rupt-optics'),
                 how='a channel 16 row with bit 6 or 7 set, or sextant CDU words in the host rows',
                 gap='the C64 flight disk has no MARK key (resident_dsky.S has no channel 16 path); in a window '
                     'the mark arrives as a replay row',
                 closes='a channel 16 key in rk_dsky_keys'),
            role('vhf', 'VHF ranging', 'in', 'RNRAD (046), channel 33 bit 4 (VHF data good), RADARUPT',
                 [('tools/flight_radar.h', 'rr_due', 'VHF model: range, data good, FLIGHT_RR_DROP')],
                 [(RRU, 'radar', 'the radar read cycle and RADARUPT on the AGC clock'), rows],
                 uses('FLIGHT_RR', 'FLIGHT_RR_DROP'), either(radar_read, channel(0o33, 0o10, 'channel_clear')),
                 unit(m, 'p20-p25'), how='RNRAD, the radar gate or RADARUPT in the host rows, or VHF data good'),
        ]
    out += [
        role('discretes', 'Discretes', 'in', 'channels 30-32 (IMU, abort, mode switches, hand controller discretes, '
             'PROCEED), channel 33 bits 10-15 (PIPA fail, warnings, oscillator, uplink and downlink too fast)',
             [(FHR, 'chan_due', 'FLIGHT_CHAN changes at sample points (FLIGHT_CH30-33: start values)')],
             [rows, ('platform/c64/resident_dsky.S', 'PRO', 'the flight disk\'s PRO key: channel 32 bit 14')],
             chan_input(0o30, 0o31, 0o32, 0o33), either(channel(0o30), channel(0o31), channel(0o32), channel(0o33, 0o77000)),
             unit(m, 't4rupt-imu', 'rcsmon') if lm else unit(m, 't4rupt-imu', 'rcs-dap'),
             how='a channel 30, 31 or 32 row that changes a bit, or a channel 33 row that changes bits 10-15'),
        role('keys', 'DSKY keys', 'in', 'channel 15 and KEYRUPT1',
             [(FHR, 'key_due', 'the scenario\'s key string, pressed at its times')],
             [rows, ('platform/c64/resident_dsky.S', 'rk_dsky_keys', 'the flight disk\'s keyboard scan: channel 15 '
                     'and KEYRUPT1 (not in a window: there the keys are replay rows)')],
             pressed('0-9A-Z+-'), either(channel(0o15, how='channel_set'), requests('KEY1')),
             unit(None, 'keyrupt-uprupt') + ['flight-disk-vice'],
             how='a channel 15 row or KEYRUPT1 in the host rows',
             gap='the DSKY noun x verb sweep on the C64 screen is Lane C\'s', closes='Lane C\'s DSKY sweep check'),
        role('uplink', 'Uplink', 'in', 'INLINK (045), UPRUPT',
             [(FHR, 'uplink_due', 'FLIGHT_UPLINK: triple-redundant INLINK words and UPRUPT, 20 cs apart')],
             [rows], uses('FLIGHT_UPLINK'), either(erasable(0o45), requests('UP')),
             unit(None, 'keyrupt-uprupt') + unit(m, 'update'), how='INLINK words or UPRUPT in the host rows'),
        role('jets', 'RCS jets and engine on/off' if lm else 'RCS jets and SPS on/off', 'out',
             'channels 5 and 6 (jets), channel 11 bits 13-14 (engine on, off)' if lm else
             'channels 5 and 6 (jets), channel 11 bit 13 (SPS on)',
             [host_end, (FHR, 'rcs_due', 'the jets\' translation into the PIPAs')] if lm else [host_end],
             [end], uses('FLIGHT_ENGINE', 'FLIGHT_RCS') if lm else uses('FLIGHT_SPS', 'FLIGHT_AERO'),
             either(moved(5), moved(6), moved(0o11, 0o30000 if lm else 0o10000)),
             unit(m, 't6rupt', 'tjet-law', 'q-r-axis') if lm else unit(m, 'jet-select', 'rcs-dap', 'tvc-roll'),
             how='channel 5 or 6, or the engine bits of channel 11, changed in the host\'s states',
             gap='the jets drive no attitude dynamics (IMU row)', closes='Lane B\'s rigid-body model'),
    ]
    if lm:
        out += [
            role('thrust', 'Descent engine throttle', 'out', 'THRUST (055), channel 14 bit 4 (thrust drive)',
                 [host_end, (FHR, 'engine_due', 'reads the throttle command (PSEUDO55/THRUST) into the engine model')],
                 [end], uses('FLIGHT_ENGINE'), either(words(0o55), moved(0o14, 0o10)),
                 unit(m, 'throttle'), how='THRUST, or channel 14 bit 4, changed in the host\'s states'),
            role('rr-drive', 'RR antenna drive', 'out', 'CDUTCMD/CDUSCMD (053/054), channel 14 bits 11-12',
                 [host_end, ('tools/flight_radar.h', 'rr_due', 'takes the drive: empties CDUTCMD/CDUSCMD when '
                             'channel 14 bit 12 or 11 is set')],
                 [rows, end], uses('FLIGHT_RR', 'FLIGHT_RR_DROP'),
                 both(either(words(0o53, 0o54), moved(0o14, 0o6000)), never(held(0o12, 0o200))),
                 unit(m, 't4rupt-rr'),
                 how='CDUTCMD or CDUSCMD, or channel 14 bit 11 or 12, changed with channel 12 bit 8 (cross pointers) '
                     'clear throughout',
                 gap='the host empties the drive counters at once, not at the pulse rate'),
            role('landing-displays', 'Landing analog displays', 'out', 'ALTM (060), channel 14 bits 2-3 (altitude '
                 'and rate meter), channel 12 cross pointers (LANDING_ANALOG_DISPLAYS)',
                 [host_end], [end], uses('FLIGHT_ENGINE', 'FLIGHT_LR'),
                 either(words(0o60), moved(0o14, 0o6),
                        both(either(words(0o53, 0o54), moved(0o14, 0o6000)), steady(0o12, 0o200))),
                 unit(m, 'landing-displays'),
                 how='ALTM, or channel 14 bit 2 or 3, changed; or the cross pointers: CDUTCMD/CDUSCMD or channel 14 '
                     'bit 11 or 12 changed with channel 12 bit 8 set throughout the window',
                 gap='no meter model on either side: ALTM and the drive bits are compared as words only'),
        ]
    else:
        out += [
            role('optics-drive', 'Optics drive', 'out', 'OPTXCMD/OPTYCMD (CDUTCMD/CDUSCMD, 053/054), channel 14 '
                 'bits 11-12', [host_end, (FHR, 'drives', 'the output drives once a period after the timers: rk_cdu_drive (src/resident_cdu.c)')],
                 [end, ('platform/c64/resident_cdu.S', 'rk_cdu_drive6502', 'the 6502 output drives: 053/054 pulses')],
                 uses('FLIGHT_LOAD'), both(either(words(0o53, 0o54), moved(0o14, 0o6000)), never(held(0o12, 0o200))),
                 unit(m, 'sxtmark'), how='OPTXCMD or OPTYCMD, or channel 14 bit 11 or 12, changed with TVC off (channel 12 bit 8 clear)',
                 gap='no optics model: the drives send the pulses, but CDUS/CDUT never follow on either side '
                     '(FLIGHT_LOAD sets them)',
                 blocking=True, closes='an optics drive model on both sides'),
        ]
    out += [
        role('iss', 'ISS mode bits', 'out', 'channel 12 bits 4-6 (coarse align, zero the ICDUs, error counters) and '
             '15 (ISS turn-on delay complete)',
             [host_end] + ([] if lm else [(FHR, 'imu_due', 'FLIGHT_IMU: zero and coarse align move CDUX-Z')]),
             [end], None if lm else uses('FLIGHT_IMU'), moved(0o12, 0o40070), unit(m, 't4rupt-imu'),
             how='channel 12 bit 4, 5, 6 or 15 changed in the host\'s states',
             gap='no ISS model on the LM: zeroing and coarse align move nothing' if lm else
             'fine align and gyro torquing move nothing (imu_due: coarse align is perfect)', blocking=lm,
             closes='an ISS model (zero, coarse align) on the LM, as imu_due on the CM' if lm else None),
    ]
    if lm:
        out += [
            role('trim', 'DPS gimbal trim', 'out', 'channel 12 bits 9-12 (pitch and roll trim, plus and minus)',
                 [host_end], [end], uses('FLIGHT_ENGINE'), moved(0o12, 0o7400), unit(m, 'trim-gimbal'),
                 how='a trim bit of channel 12 changed in the host\'s states',
                 gap='no gimbal model: the engine thrusts along a fixed axis, so trim moves nothing', blocking=True,
                 closes='Lane B: a DPS gimbal in the engine model'),
        ]
    else:
        out += [
            role('tvc', 'SPS thrust vector control', 'out', 'TVCPITCH/TVCYAW (CDUTCMD/CDUSCMD, 053/054) with channel '
                 '12 bit 8 (TVC enable)',
                 [host_end, (FHR, 'drives', 'the output drives once a period after the timers: rk_cdu_drive (src/resident_cdu.c)'), (FHR, 'sps_due', 'the bell follows the optics error counters while channel 12 '
                                       'bit 8 is set, and the thrust turns the CM in pitch and yaw')],
                 [end, ('platform/c64/resident_cdu.S', 'rk_cdu_drive6502', 'the 6502 output drives: 053/054 pulses')],
                 uses('FLIGHT_SPS'),
                 both(words(0o53, 0o54), steady(0o12, 0o200)), unit(m, 'tvc-roll'),
                 how='TVCPITCH or TVCYAW changed with channel 12 bit 8 set throughout the window',
                 gap='roll does not move under TVC (no RCS jet model)'),
        ]
    out += [
        role('drives', 'IMU CDU drives', 'out', 'CDUXCMD-CDUZCMD (050-052), channel 14 bits 13-15',
             [host_end, (FHR, 'drives', 'the output drives once a period after the timers: rk_cdu_drive (src/resident_cdu.c)')],
             [end, ('platform/c64/resident_cdu.S', 'rk_cdu_drive6502', 'the 6502 output drives once a period after '
                    'the timers, checked against src/resident_cdu.c and yaAGC (make c64-resident-cdu-asm)')],
             None, either(words(0o50, 0o51, 0o52), moved(0o14, 0o70000)), ['test-resident-cdu', 'c64-resident-cdu-asm'],
             how='a CDU drive word, or a drive bit of channel 14, changed in the host\'s states'),
        role('gyro', 'Gyro torquing', 'out', 'GYROCMD (047), channel 14 bits 6-10 (gyro enable, select, sign, activity)',
             [host_end], [end], None, either(words(0o47), moved(0o14, 0o1740)), [],
             how='GYROCMD, or a gyro bit of channel 14, changed in the host\'s states',
             gap='no gyro torque model: GYROCMD pulses move nothing on either side (fine align, gyro compensation)',
             blocking=True, closes='a gyro torque model that turns the platform'),
        role('displays', 'DSKY displays and lamps', 'out', 'channel 10 (relay words), channel 11 lamps',
             [host_end, (FHR, 'dsky_trace', 'FLIGHT_DSKY: prints the DSKY as the relay words set it')],
             [end, ('platform/c64/resident_dsky.S', 'rk_dsky_idle', 'the flight disk draws the DSKY from the relay '
                    'words and channel 11')],
             None, either(moved(0o10), moved(0o11, 0o1176)), unit(None, 't4rupt-core') + ['flight-disk-vice'],
             how='channel 10, or a lamp bit of channel 11, changed in the host\'s states',
             gap='the on-screen DSKY sweep is Lane C\'s', closes='Lane C\'s DSKY sweep check'),
        role('downlink', 'Downlink', 'out', 'channels 34 and 35, DOWNRUPT', [], [], None, any_window, [],
             elsewhere=f'docs/acceptance/{PROGRAM[m]}-native_output_tests.md'),
    ]
    return out


# A symbol's definition: a function (its name at the start of a declaration,
# after any type words, attributes or section macros such as LOW_TEXT("x"),
# then its parameters and body), an assembler label or a #define.
def _definition(name):
    n = re.escape(name)
    return re.compile(rf'^[ \t]*(?:(?:__attribute__\(\([^()]*(?:\([^()]*\))?[^()]*\)\)|[A-Za-z_]\w*(?:\([^()]*\))?)[ \t\*]+)*'
                      rf'{n}[ \t]*\([^;{{}}]*\)\s*\{{|^[ \t]*{n}:|^[ \t]*#[ \t]*define[ \t]+{n}\b', re.M)


def code(path):
    """A source file's text without comments and string or character literals."""
    text = path.read_text(errors='replace')
    text = re.sub(r'/\*.*?\*/', lambda x: '\n' * x[0].count('\n'), text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    if path.suffix in ('.S', '.s'):
        text = re.sub(r';[^\n]*', '', text)
    return re.sub(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])+\'', '""', text)


def adapter_ok(path, symbol):
    f = ROOT / path
    return f.exists() and _definition(symbol).search(code(f)) is not None


def host_rows(m, summary, env, commit):
    """[(scenario, state, detail)] for the scaled-x1 flight scenarios of m
    that drive the role (env(inputs, keys); None: all). state: 'clean',
    'NOT CLEAN', 'MISSING' or 'STALE'."""
    import flight_host_differential as fhd  # noqa: E402
    by = {r['scenario']: r for r in summary}
    rows = []
    for name, mm, cs, keys, inp in fhd.FLIGHT:
        if mm != m or (env is not None and not env(inp, keys)):
            continue
        r = by.get(f'{name}-scaled-x1')
        if r is None:
            rows.append((name, 'MISSING', 'not in the host summary'))
            continue
        un = r.get('unregistered')
        un = len(un) if isinstance(un, list) else (un or 0)
        detail = (f"{r.get('native_routines')} native routines, {len(r['routine_mismatches'])} did not match yaAGC, "
                  f'{un} unregistered or refused points (yaAGC ran them)')
        state = ('STALE' if r.get('commit') != commit else
                 'clean' if r['exit'] == 0 and not r['routine_mismatches'] else 'NOT CLEAN')
        rows.append((name, state, detail + (f", from {r.get('commit')}" if state == 'STALE' else '')))
    return rows


def not_matched(r, asked, commit):
    """Why a window result is not a match ([] when it is)."""
    why = []
    if r.get('error'):
        return [f"no result: {' '.join(r['error'])}"]
    if r.get('ok') is not True:
        why.append('ok is not true')
    if r.get('status') != 0:
        why.append(f"status {r.get('status')}")
    if (r.get('reu_bounds') or {}).get('ok') is not True:
        why.append(f"REU DMA outside the layout ({(r.get('reu_bounds') or {}).get('dma_outside')})")
    if not r.get('replay_ended'):
        why.append('did not end')
    if r.get('replay_applied') != r.get('replay_rows'):
        why.append(f"applied {r.get('replay_applied')} of {r.get('replay_rows')} input rows")
    if r.get('replay_end_paired') is not True:
        why.append('its end was not paired with the host\'s')
    if r.get('replay_unpaired_count'):
        why.append(f"{r['replay_unpaired_count']} checkpoints not compared")
    if r.get('replay_skipped'):
        why.append('skipped ' + '; '.join(r['replay_skipped']))
    if r.get('replay_differences') != 0:
        why.append('differs: ' + first_diff(r))
    done = r.get('replay_matched_cs') or []
    if not done:
        why.append('no checkpoint compared')
    elif done[-1] != r.get('requested_cs'):
        why.append(f"last compared checkpoint {done[-1]} cs, not its end {r.get('requested_cs')} cs")
    if asked is None:
        why.append('not one of the program\'s windows')
    elif (r.get('requested_cs') or 0) < asked:
        why.append(f"covered {r.get('requested_cs')} of {asked} cs")
    if r.get('commit') != commit:
        why.append(f"from {r.get('commit')}, not {commit}")
    return why


def first_diff(r):
    """Where a differing window first left the host, in words."""
    f = r.get('replay_first') or []
    at = r.get('replay_differ_at_cs')
    w = ', '.join(f"{d['at']} host {d['host']} C64 {d['c64']}" for d in f[:2])
    return f"{r.get('replay_differences')} words by {at} cs" + (f' ({w})' if w else '')


def assess(m, table, runs, host, commit):
    """[(role, status, detail lines)], [failures]."""
    bad, out = [], []
    targets = make_targets()
    for t in table:
        if t['elsewhere']:
            here = (ROOT / t['elsewhere']).exists()
            if not here:
                bad.append(f"{t['key']}: {t['elsewhere']} is not in the tree")
            out.append((t, 'in ' + pathlib.Path(t['elsewhere']).name if here else 'GAP', [], [], [], []))
            continue
        ads = [(side, f, s, w, adapter_ok(f, s)) for side in ('host', 'c64') for f, s, w in t[side]]
        bad += [f"{t['key']}: {f} defines no {s}" for _, f, s, _, ok in ads if not ok]
        hrows = host_rows(m, host, t['env'], commit)
        bad += [f"{t['key']}: host scenario {n} {state} ({d})" for n, state, d in hrows if state != 'clean']
        if not hrows:
            bad.append(f"{t['key']}: no host scenario drives it")
        if not any(side == 'host' for side, *_ in ads):
            bad.append(f"{t['key']}: no host adapter")
        bad += [f"{t['key']}: no make target {u}" for u in t['units'] if u not in targets]
        wins = [r for r in runs if t['window'](r)]
        matched = [r for r in wins if not r['_why']]
        if t['blocking']:
            status = 'GAP'
            bad.append(f"{t['key']}: {t['gap']}")
        elif not matched:
            status = 'GAP'
            bad.append(f"{t['key']}: no C64 window that exercises it matches the host")
        else:
            status = 'covered, with open items' if t['gap'] else 'covered'
        out.append((t, status, ads, hrows, wins, matched))
    return out, bad


def write_doc(m, assessed, bad, runs, host_cite, store, commit):
    p = PROGRAM[m]
    store_line = (f"The C64 windows run the store `flight-store/{m}.reu` (sha256 `{store}`), each with its own "
                  'resident PRG linked by tools/flight_scenario_vice.py.' if store else
                  f'The C64 store `flight-store/{m}.reu` is not built at this commit.')
    lines = [f'# {p}: device adapter tests', '',
             f'Generated by `tools/device_acceptance.py {m}` at commit `{commit}`. '
             f"Result: **{'FAIL' if bad else 'PASS'}**", '',
             'This is the `device_adapter_tests` evidence of the '
             f'`integrated_applications.{p}` entry (docs/PORT_SCOPE.md): each input role the program reads and each '
             'output it drives, the adapter that carries it on the host and on the C64, and the checks that '
             'exercise it.', '',
             '- **Host check:** the whole-program host differential (tools/flight_host_differential.py, scaled time '
             'x1). Every flight scenario that drives the role must be in the summary, from this commit, and clean: '
             'exit 0 and every native routine left yaAGC\'s erasable memory and output channels. Scenarios are '
             'picked by the FLIGHT_* inputs or keys they are given, not by what the run then drove. Unregistered or '
             'refused points (routines yaAGC ran for the native side) are counted, not failed.'
             + (f' Host summary (this program\'s rows): {host_cite}.' if host_cite else ''),
             '- **C64 check:** a `tools/flight_scenario_vice.py --replay` window on stock C64 + 2 MB REU in VICE. '
             'The C64 starts from the host\'s snapshot, takes the host\'s inputs as rows (the replay row injector is '
             'the C64 adapter of every input role in a window), and must end every checkpoint in the host\'s state: '
             'erasable words, TIME1-6, output channels 5, 6, 10-14, 34, 35 and the superbank. A window matches only '
             'when the run is ok with status 0, every REU DMA stayed in the layout, every input row was applied, '
             'the end and every checkpoint were paired and compared with no skips and no differences, the last '
             'compared checkpoint is the end, and the run covers the window and is from this commit. '
             'A window exercises an input role when its host rows touch the role; an output role when the role\'s '
             'channel bits or words changed in the host\'s states over the window (a held bit counts only as an enable '
             'gate, such as TVC or cross-pointer enable).', '',
             store_line, '',
             '## Summary', '', '| Role | In/out | Adapters | Host check | C64 check | Status |', '|---|---|---|---|---|---|']
    detail = []
    for t, status, ads, hrows, wins, matched in assessed:
        if t['elsewhere']:
            lines.append(f"| {t['name']} | {t['dir']} | see `{t['elsewhere']}` | | | {status} |")
            detail += [f"### {t['name']} (output)", '', f"AGC interface: {t['agc']}.", '',
                       f"Covered by `{t['elsewhere']}` (native_output_tests, Lane E): the host's word stream against "
                       'yaAGC and a C64 window that takes DOWNRUPTs. That doc\'s result is its own; this row does not '
                       'count it.', '']
            continue
        missing = sum(1 for a in ads if not a[4])
        lines.append(f"| {t['name']} | {t['dir']} | {'all defined' if not missing else f'MISSING {missing}'} | "
                     f"{sum(1 for _, s, _ in hrows if s == 'clean')} of {len(hrows)} scenarios clean | "
                     f"{len(matched)} of {len(wins)} windows match | {status} |")
        d = [f"### {t['name']} ({'input' if t['dir'] == 'in' else 'output'})", '', f"AGC interface: {t['agc']}.", '',
             '| Side | Adapter | Symbol | What it does | Defined |', '|---|---|---|---|---|']
        d += [f"| {side} | `{f}` | `{s}` | {w} | {'yes' if ok else 'NO'} |" for side, f, s, w, ok in ads] or \
            ['| host | none | | | NO |']
        d += ['', 'Host scenarios that drive it:', '']
        d += [f'- {n}: {state} ({x})' for n, state, x in hrows] or ['- none']
        d += ['', f"C64 windows that exercise it ({t['how']}):", '']
        d += [f"- {r['scenario']} {r['snap_cs']}+{r.get('requested_cs')}: "
              + ('matches the host at ' + ', '.join(map(str, r.get('replay_matched_cs') or [])) + ' cs'
                 if not r['_why'] else 'no match: ' + '; '.join(r['_why'])) + f" ({r['_evidence']})" for r in wins] \
            or ['- none']
        if t['units']:
            d += ['', 'Routine-level checks (host differential against yaAGC, then the same cases on the C64 in VICE): '
                  + ', '.join(f'`make {u}`' for u in t['units']) + '.']
        if t['gap'] or not matched:
            d += ['', f"**{'Blocking' if t['blocking'] else 'Open'}:** {t['gap'] or 'no matching C64 window exercises it yet'}."
                  + (f" Closes it: {t['closes']}." if t['closes'] else '')]
        detail += d + ['']
    lines += ['', 'Status: covered = adapters defined, every host scenario that drives it clean, and a matching C64 '
              'window that exercises it; "with open items" names what the row still lacks, none of it blocking; '
              'GAP = no matching window, or a blocking open item (a model that does not run).', '']
    if bad:
        lines += ['## Failures', ''] + [f'- {b}' for b in bad] + ['']
    lines += ['## Roles', ''] + detail
    lines += ['## Runs', '', 'A window that differs is not a device failure by itself: each difference is triaged on the '
              'shared replay mismatch list (clock and charge classes, DAP-rate words), and only the device or channel '
              'class is this doc\'s. A role counts as covered only through windows that match.', '',
              '| Scenario | Start cs | cs | Every cs | Result | Evidence |', '|---|---|---|---|---|---|']
    for r in runs:
        lines.append(f"| {r['scenario']} | {r['snap_cs']} | {r.get('requested_cs')} | {r.get('replay_every', '')} | "
                     f"{'matches' if not r['_why'] else '; '.join(r['_why'])} | {r['_evidence']} |")
    lines += ['', '## Reproduce', '', '```sh', 'make flight-store',
              f'python3 tools/device_acceptance.py {m}   # host differential + the windows above',
              '# one window: python3 tools/flight_scenario_vice.py --scenario <scenario> --snap-cs <cs> --cs <n> --replay --replay-every <every>',
              '```', '']
    path = ROOT / 'docs/acceptance' / f'{p}-device_adapter_tests.md'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines))
    return path


def run_window(scenario, snap, cs, every, out):
    """One flight_scenario_vice.py --replay window; its result JSON (the run
    writes it even when the C64 leaves the host's state)."""
    res = ROOT / 'build/flight-store/flight-scenario-vice.json'
    res.unlink(missing_ok=True)
    r = subprocess.run([sys.executable, 'tools/flight_scenario_vice.py', '--scenario', scenario, '--snap-cs', str(snap),
                        '--cs', str(cs), '--replay', '--replay-every', str(every)], cwd=ROOT, capture_output=True, text=True)
    (out / f'{scenario}-{snap}.log').write_text(r.stdout + r.stderr)
    result = out / f'{scenario}-{snap}.json'
    try:
        d = json.loads(res.read_text())
    except (OSError, ValueError):
        d = {}
    if d.get('scenario') != scenario or d.get('snap_cs') != snap:
        text = (r.stdout + r.stderr).strip().splitlines()
        # The first error line says why (a link error before its traceback).
        d = dict(scenario=scenario, snap_cs=snap,
                 error=[next((x.strip() for x in text if 'error:' in x), text[-1] if text else 'no output')])
    d['replay_every'] = every
    result.write_text(json.dumps(d, indent=1) + '\n')
    print(f"{scenario} {snap}+{cs}: {next((x for x in reversed(r.stdout.splitlines()) if 'matches the host' in x or 'FAIL' in x), 'no result')}")
    return result


def env_of(name):
    """A flight scenario's FLIGHT_* inputs."""
    import flight_host_differential as fhd  # noqa: E402
    try:
        return dict(fhd.scenario(name)[4])
    except KeyError:
        return {}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('mission', choices=sorted(PROGRAM))
    ap.add_argument('--host', help="tools/flight_host_differential.py's summary.json")
    ap.add_argument('--run', action='append', default=[], help='a flight_scenario_vice.py --replay result JSON')
    a = ap.parse_args(argv)
    m = a.mission
    commit = flight_holes.source_commit()
    eid = f'{m}-acceptance-devices'
    dest = ROOT / 'build' / eid  # staged here, then stored flat by tools/evidence.py add --tree
    shutil.rmtree(dest, ignore_errors=True)
    dest.mkdir(parents=True)
    copies = []

    def stored(copy):
        copies.append(copy)
        return f'`evidence/{eid}/{copy.name}` (sha256 `{sha(copy)[:16]}...`)'
    if not a.host:
        # A scenario that differs ends the differential nonzero; its summary
        # still holds every row. No summary means the run itself broke.
        summary = ROOT / 'build/flight-store/differential/summary.json'
        summary.unlink(missing_ok=True)
        subprocess.run([sys.executable, 'tools/flight_host_differential.py'], cwd=ROOT, check=False)
        if not summary.exists():
            sys.exit('tools/flight_host_differential.py wrote no summary')
        a.host = str(summary)
    if not a.run:
        out = ROOT / 'build' / f'{eid}-runs'
        shutil.rmtree(out, ignore_errors=True)
        out.mkdir(parents=True)
        a.run = [str(run_window(*w, out)) for w in WINDOWS[m]]
    host = [r for r in json.loads(pathlib.Path(a.host).read_text()) if r.get('mission') == m]
    copy = dest / 'host-summary.json'
    copy.write_text(json.dumps(host, indent=1) + '\n')
    host_cite = stored(copy)
    asked = {(s, snap): cs for s, snap, cs, _ in WINDOWS[m]}
    bad, runs = [], []
    for path in a.run:
        r = json.loads(pathlib.Path(path).read_text())
        if r.get('mission', m) != m or not r['scenario'].startswith(f'{m}-'):
            bad.append(f"run {path}: {r['scenario']} is not a {PROGRAM[m]} window")
            continue
        copy = dest / f"{r['scenario']}-{r.get('snap_cs', 'x')}.json"
        copy.write_text(json.dumps(r, indent=1) + '\n')
        r['_evidence'] = stored(copy)
        r['_env'] = env_of(r['scenario'])
        r['_why'] = not_matched(r, asked.get((r['scenario'], r.get('snap_cs'))), commit)
        runs.append(r)
    shutil.rmtree(ROOT / 'evidence' / eid, ignore_errors=True)  # this generator's alone: no stale runs
    # evidence/<id>/ is flat, with its SHA256.json (tools/ci_guard.py).
    subprocess.run([sys.executable, 'tools/evidence.py', 'add', eid, *map(str, copies), '--tree'], cwd=ROOT, check=True)
    assessed, failed = assess(m, roles(m), runs, host, commit)
    bad += failed
    store = ROOT / f'build/flight-store/{m}.reu'
    path = write_doc(m, assessed, bad, runs, host_cite, sha(store) if store.exists() else None, commit)
    print(f"{path.relative_to(ROOT)}: {'PASS' if not bad else 'FAIL'} (sha256 {sha(path)})")
    for b in bad:
        print(f'FAIL: {b}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
