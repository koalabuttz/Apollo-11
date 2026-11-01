#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The x86_64 flight program with the readable routines.

The faithful port's x86 program (c64-port/docs/X86-FLIGHT.md) links every
routine table of c64-port/src; this builds it with the readable objects
(readable/build/x86-<m>/) in place of the faithful ones that define the
same tables (REPLACED below), into readable/build/x86-flight/. The
scenario suite (c64-port/tools/x86_flight_scenarios.py) then flies that
program against the harness, which still runs the faithful port: every
row must end with the harness's erasable memory, word for word.

  x86_readable.py build [--host-cc CC]
  x86_readable.py scenarios [--quick] [--jobs N] [NAME ...]

Run by `make x86-flight`, `make x86-scenarios-quick` and `make
x86-scenarios` from readable/. Needs the port's toolchain
(c64-port/tools/setup_cloud.sh) for the harness's yaAGC.
"""
import os, subprocess, sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
sys.path.insert(0, str(PORT / 'tools'))
import x86_flight_build as xb  # noqa: E402

# The faithful objects whose routine tables the readable objects define.
REPLACED = (
    # The Executive and WAITLIST (src/executive, src/waitlist).
    'lm_kernel.o', 'cm_kernel.o',
    # INTER-BANK_COMMUNICATION (src/interbank). lm_interbank.o and
    # cm_interbank_complete.o stay: other faithful routines call them.
    'lm_interbank_routines.o', 'lm_interbank_idle.o', 'lm_interbank_dap.o',
    'cm_interbank_routines.o', 'cm_interbank_idle.o', 'cm_interbank_postjump.o',
    # PHASE_TABLE_MAINTENANCE (src/phase_table).
    'lm_phase_table.o', 'cm_phase_table.o',
    # RESTARTS_ROUTINE and RESTART_TABLES (src/restart). cm_restart_select.o
    # stays: the faithful restart path (cm_restart.o) calls it.
    'lm_restart_routines.o', 'lm_restart_select.o', 'cm_restart_routines.o',
    # INTERPRETER.agc (src/interpreter): the interpreter core and the
    # compiled basic routines. The faithful order bodies (lm_interp_orders.o,
    # lm_interp_steps.o and the CM's) stay: the compiled interpretive code
    # of other files calls them, as does the verification interpreter
    # (lm_interp_runtime.o), the holder of last resort.
    'lm_interp_core.o', 'cm_interp_core.o', 'lm_interpreter.o', 'cm_interpreter.o',
    # ALARM_AND_ABORT.agc (src/alarm). The CM's cm_alarm_record.o and the
    # other split files stay: they are the C64's, and nothing on x86 calls
    # them.
    'lm_alarm.o', 'cm_alarm.o',
    # SINGLE_PRECISION_SUBROUTINES.agc (src/single_precision): the routine
    # tables. lm_single_trig.o, cm_single_return.o and cm_quictrig.o stay:
    # other faithful routines call them.
    'sp_routines.o',
    # SERVICE_ROUTINES.agc (src/service): the routine tables.
    # service_routines.o stays: other faithful routines call it.
    'service_transfer.o',
    # DOWN-TELEMETRY_PROGRAM.agc (src/downtelemetry).
    'lm_downtelemetry.o', 'cm_downtelemetry.o',
    # FRESH_START_AND_RESTART.agc, bank 05 and GOTOPOOH (src/fresh_start).
    'lm_fresh_start.o', 'cm_fresh_start.o',
    # FRESH_START_AND_RESTART.agc, verb 37 and P00 (src/fresh_start/v37.c).
    'lm_v37.o', 'cm_v37.o',
    # FRESH_START_AND_RESTART.agc, STATEUP's interpretive code
    # (src/fresh_start/stateup.c, run by the readable interpreter).
    'lm_stateup.o', 'cm_stateup.o',
    # KEYRUPT_UPRUPT.agc (src/keyrupt): both missions' tables.
    'keyrupt_uprupt.o',
    # INTERRUPT_LEAD_INS.agc (src/lead_ins), WAITLIST.agc's RESUME exits
    # (src/waitlist) and the LM's T6-RUPT_PROGRAMS.agc (src/t6rupt). The
    # flight program takes DOWNRUPT's vector and RESUME from the
    # down-telemetry tables (lm_dntm_rupt.o, cm_dntm_rupt.o): the readable
    # binding defines those too.
    'rupt_lead_ins.o', 'lm_t6rupt.o', 'lm_dntm_rupt.o', 'cm_dntm_rupt.o',
    # T4RUPT_PROGRAM.agc: the display and dispatch core, the IMU monitor,
    # the LM's radar checks and the CM's optics (src/t4rupt).
    't4rupt_core.o', 'lm_t4rupt_imu.o', 'cm_t4rupt_imu.o', 'lm_t4rupt_rr.o', 'cm_t4rupt_optics.o',
    # RCS_FAILURE_MONITOR.agc, the LM's (src/rcs_failure_monitor).
    'lm_rcsmon.o',
    # AGS_INITIALIZATION.agc, the LM's R47 (src/ags_initialization).
    'lm_ags_init.o',
    # AOTMARK.agc, the LM's sighting marks (src/aotmark).
    'lm_aotmark.o',
    # SXTMARK.agc, the CM's sighting marks (src/sxtmark).
    'cm_sxtmark.o',
    # EXTENDED_VERBS.agc, part by part (src/extended_verbs/): each the
    # module the port's tables (lanec_ev_routines.o) call. Part 1, the
    # front end; part 2, the short verbs; part 3, alignment and radar
    # designate; part 4, the LM's radar test verbs; part 5, the DAP data
    # load; part 6, the clock update and ATTCK2; part 7, the tracking and
    # state vector verbs; part 8, the LM's interpretive words of those
    # verbs. The CM's file is all readable (part 9): its interpretive words
    # and the four fan words the parts' tables do not list take the place
    # of the port's compiled copy of the file, under its tables' names.
    'lanec_ev_front.o',
    'lanec_ev_short.o',
    'lanec_ev_align.o',
    'lanec_ev_radar.o',
    'lanec_ev_dap.o',
    'lanec_ev_time.o',
    'lanec_ev_track.o',
    'lanec_ev_lmi.o',
    'cm_extended_verbs_file.o',
    # PINBALL_NOUN_TABLES.agc: the readers PINBALL's faithful routines call
    # (apollo_noun_reader, src/pinball_noun_tables). fixed_data.o stays: the
    # faithful CM display code reads the faithful tables.
    'noun_reader.o',
    # LEM_GEOMETRY.agc and CSM_GEOMETRY.agc (src/geometry).
    'lm_geometry.o',
    'cm_geometry.o',
    # IMU_COMPENSATION_PACKAGE.agc, both missions' tables (src/imu_compensation).
    'imu_compensation.o',
    # R63.agc, the LM's V89 final attitude (src/r63).
    'lm_r63.o',
    # ATTITUDE_MANEUVER_ROUTINE.agc and the CM's ANGLFIND.agc, KALCMANU's start
    # (src/attitude_maneuver).
    'lm_attitude_maneuver.o',
    'cm_attitude_maneuver.o',
    # GIMBAL_LOCK_AVOIDANCE.agc, both missions' LOCSKIRT (src/gimbal_lock).
    'lm_gimbal_lock.o',
    'cm_gimbal_lock.o',
    # KALCMANU_STEERING.agc, both missions' maneuver steering (src/kalcmanu_steering).
    'lm_kalcmanu_steering.o',
    'cm_kalcmanu_steering.o',
    # SYSTEM_TEST_STANDARD_LEAD_INS.agc, both missions' lead-ins for erasable
    # programs, with the INTERPRETER entries they reach (src/standard_lead).
    'standard_lead_routines.o',
)
# A symbol each readable subsystem defines, which must be in the program.
READABLE_SYMBOLS = ('novac', 'bankcall', 'phschng2', 'getpart2', 'newops', 'whimper', 'spsin', 'gentran',
                    'dodowntm', 'slap1', 'v37xeq', 'interpret_listing', 'keyrupt1', 'go_lead_in',
                    't4jump', 'imumon', 'goextvb', 'showsum', 'imufinek', 'lodnntab', 'one_pipa',
                    'e_bkcall')
# And each mission's own: R63 is the LM's alone; GIMBAL_LOCK_AVOIDANCE's
# table has each mission's name, and the CM's KALCMANU_STEERING and ANGLFIND
# have routines the LM's have not (STOPRATE, SECAD's basic code), as has
# its SYSTEM_TEST_STANDARD_LEAD_INS (COMPVER).
MISSION_SYMBOLS = {'lm': ('v89recl', 'kalcman3', 'NOGIMLOC_CODE', 'newdelhi'), 'cm': ('WCALC_CODE', 'newdelhi', 'stoprate', 'secad_after_exit', 'compver')}
xb.OUT = READABLE / 'build/x86-flight'


def readable_objects(m):
    objects = sorted((READABLE / f'build/x86-{m}').rglob('*.o'))
    if not objects:
        sys.exit(f'x86_readable: no readable objects in build/x86-{m} (run make from readable/)')
    return [str(o) for o in objects]


def build(cc, m, mode, certified_only=False):
    """x86_flight_build.build, linking the readable kernel for mission m."""
    define, suffix = xb.MODES[mode]
    suffix += '-certified' if certified_only else ''
    flags = xb.flags(m, define, certified_only)
    xb.OUT.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in ('flight_dispatch', 'flight_main'):
        o = xb.OUT / f'{source}_{m}{suffix}.o'
        subprocess.run([cc, *flags, '-c', f'platform/x86/{source}.c', '-o', str(o)], cwd=PORT, check=True)
        objects.append(str(o))
    host = [str(p) for p in sorted((xb.STORE / 'host').glob('*.o')) if p.name not in REPLACED]
    exe = xb.OUT / f'x86-flight-{m}{suffix}'
    subprocess.run([cc, *objects, *host, *readable_objects(m), *xb.LINK, '-Wl,--gc-sections', '-lm',
                    '-o', str(exe)], cwd=PORT, check=True)
    # The readable routines must be the ones linked: theirs are present, and
    # the faithful kernel's instruction helpers (k_go and the rest) absent.
    symbols = subprocess.run(['nm', str(exe)], capture_output=True, text=True, check=True).stdout.split('\n')
    names = {line.split()[-1] for line in symbols if line.strip()}
    if not names.issuperset(READABLE_SYMBOLS + MISSION_SYMBOLS[m]) or 'k_go' in names:
        sys.exit(f'x86_readable: {exe.name} does not link the readable routines alone')
    return exe


xb.build = build


def main():
    args = sys.argv[1:]
    if not args or args[0] not in ('build', 'scenarios'):
        sys.exit(__doc__)
    os.chdir(PORT)
    if args[0] == 'build':
        cc = args[args.index('--host-cc') + 1] if '--host-cc' in args else os.environ.get('CC', 'cc')
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', cc],
                       cwd=PORT, check=True)
        for m in ('lm', 'cm'):
            for mode in ('compiled', 'core'):
                print(f'built {build(cc, m, mode)}')
        return 0
    import x86_flight_scenarios as xs
    xs.OUT = xb.OUT / 'scenarios'
    rest = args[1:]
    if '--jobs' not in rest and '-j' not in rest:
        rest = ['--jobs', str(os.cpu_count() or 1)] + rest
    sys.argv = ['x86_flight_scenarios.py'] + rest
    return xs.main()


if __name__ == '__main__':
    sys.exit(main())
