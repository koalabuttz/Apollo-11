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
)
# A symbol each readable subsystem defines, which must be in the program.
READABLE_SYMBOLS = ('novac', 'bankcall', 'phschng2', 'getpart2', 'newops', 'whimper', 'spsin', 'gentran',
                    'dodowntm', 'slap1', 'v37xeq', 'interpret_listing')
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
    if not names.issuperset(READABLE_SYMBOLS) or 'k_go' in names:
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
