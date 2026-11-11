#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The x86_64 flight program of the readable routines.

The faithful port's x86 program (c64-port/docs/X86-FLIGHT.md) is its step
and main (c64-port/platform/x86) over every routine table of c64-port/src.
This builds the same step and main over the readable objects alone
(readable/build/x86-<m>/), into readable/build/x86-flight/: they define
every table the port's index names (c64-port/build/flight-store/
<m>-host.inc), under its name. Of the port's own objects only the
computer under the routines is linked (MACHINE below), and the build fails
if the link map names any other (stands_alone). The scenario suite
(c64-port/tools/x86_flight_scenarios.py) then flies that program against
the harness, which still runs the faithful port: every row must end with
the harness's erasable memory, word for word.

  x86_readable.py build [--host-cc CC]
  x86_readable.py scenarios [--quick] [--jobs N] [NAME ...]

Run by `make x86-flight`, `make x86-scenarios-quick` and `make
x86-scenarios` from readable/. Needs the port's toolchain
(c64-port/tools/setup_cloud.sh) for the harness's yaAGC.
"""
import os, re, subprocess, sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
sys.path.insert(0, str(PORT / 'tools'))
import x86_flight_build as xb  # noqa: E402

# The computer under the routines: the objects of c64-port/src the readable
# program links. None is a translation of the listing's code.
MACHINE = (
    # Where a word of fixed memory is (bank and address to its place in
    # the rope), the search of a routine table, and the marker the step
    # keeps DANZIG's place in its index with (a table of one routine that
    # refuses every state).
    'agc_transfer.o',
    # One's complement words: negate, to and from an int.
    'agc_word.o',
    # The DSKY, the device: its relays and lights as channels 10 and 11
    # set them.
    'dsky_display.o',
    # The channels a routine's device record carries, taken from the
    # hardware record before a step and given back after it.
    'flight_io.o',
    # The gyro and CDU output drives, pulse by pulse.
    'resident_cdu.o',
    # The timers (TIME1 to TIME6) and the entry to an interrupt.
    'resident_rupt.o',
    # The rope: the assembled program, read as data.
    'rope_image_host.o',
)
# A symbol each readable subsystem defines, which must be in the program.
READABLE_SYMBOLS = ('novac', 'bankcall', 'phschng2', 'getpart2', 'newops', 'whimper', 'spsin', 'gentran',
                    'dodowntm', 'slap1', 'v37xeq', 'interpret_listing', 'keyrupt1', 'go_lead_in',
                    't4jump', 'imumon', 'goextvb', 'showsum', 'imufinek', 'lodnntab', 'one_pipa',
                    'e_bkcall', 'charin', 'sbandant', 'p3xorp7x', 'r31call', 'endp76', 'v82call',
                    'vndsply_proceed', 'selfchk')
# And each mission's own: R63 is the LM's alone; GIMBAL_LOCK_AVOIDANCE's
# table has each mission's name, and the CM's KALCMANU_STEERING and ANGLFIND
# have routines the LM's have not (STOPRATE, SECAD's basic code), as has
# its SYSTEM_TEST_STANDARD_LEAD_INS (COMPVER). The IMU performance test
# (GEOIMUTT) is the LM's alone. R60 has each mission's name (R60LEM,
# R60CSM). The radar sampling loop (RADSAMP) is the LM's alone, and each
# mission's P20 has labels the other's has not (the LM's PROG20A, the CM's
# PIKUP20). Each mission's P30 has labels the other's has not (the LM's
# P30N33, the CM's P30PHSI). Each mission's P32 has a P32STRT, the LM's in
# basic code (p32strt alone), the CM's in interpretive code too
# (P32STRT_CODE). The LM's P31 is GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc's
# (p31_after_p20flgon), the CM's its P30-P37.agc's. Both missions have
# P21 (prog21). Each mission's R31 has labels the other's has not (the
# LM's REV83, the CM's R34LOS). The LM's P76 starts with interpretive
# code (P76_CODE), the CM's does not. The CM's R30 updates the splash
# error in P00 and P11 (candel), the LM's does not. BURN, BABY, BURN is the
# LM's alone (killtsk2), as is the NSTEER of its P40-P47 and THE_LUNAR_LANDING
# (ddumcalc) and THROTTLE_CONTROL_ROUTINES (massmult) and P66's RODCOMP
# (rodcomp, LUNAR_LANDING_GUIDANCE_EQUATIONS); the CM's P40-P47
# starts at P40CSM (p40csm). LANDING_ANALOG_DISPLAYS is the LM's alone
# (landisp), as is FINDCDUW--GUIDAP_INTERFACE (fltrsub). Each mission's
# P52 is readable (prog52). Each mission's
# LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES is its own (EPHEMERIDES_CODE).
# The CM's SERVICER207 has the entry's half-second PIPA reading (quikread).
# The CM's REENTRY_CONTROL starts entry (startent_after_exit), and its
# CM_BODY_ATTITUDE updates the attitude angles (cm_atup). The CM's P37
# is its own (rtevn_after_exit), as is its TVC DAP (tvcdapon, its
# half-second task, tvcexec, its mass properties, massprop, its
# restarts, redotvc, its pitch and yaw DAPs, pitchdap, its stroke test,
# strktsti, and its roll DAP, rolldap), and its RCS DAP's MYSUBS (smallmp).
# The CM's IMU calibration and alignment starts at IMUTEST (imutest), and
# its P11 has the Saturn takeover (satstick).
MISSION_SYMBOLS = {'lm': ('v89recl', 'kalcman3', 'NOGIMLOC_CODE', 'newdelhi', 'geoimutt', 'r60lem',
                          'radsamp', 'prog20a', 'p30n33', 'p32strt',
                          'p31_after_p20flgon', 'prog21', 'rev83', 'P76_CODE', 'killtsk2',
                          'nsteer', 'ddumcalc', 'massmult', 'rodcomp', 'abrtjask', 'p12lm',
                          'atmag', 'rereadac', 'landisp', 'fltrsub', 'prog52', 'EPHEMERIDES_CODE'),
                   'cm': ('WCALC_CODE', 'newdelhi', 'stoprate', 'secad_after_exit', 'compver', 'r60csm',
                          'pikup20', 'p30phsi', 'P32STRT_CODE', 'prog21', 'r34los_after_exit', 'candel',
                          'rtrn', 'p40csm', 'EPHEMERIDES_CODE', 'prog52', 'p61', 'quikread',
                          'startent_after_exit', 'cm_atup', 'rtevn_after_exit', 'tvcdapon',
                          'tvcexec', 'massprop', 'redotvc', 'pitchdap', 'strktsti', 'rolldap',
                          'smallmp', 'imutest', 'satstick')}
xb.OUT = READABLE / 'build/x86-flight'


def readable_objects(m):
    objects = sorted((READABLE / f'build/x86-{m}').rglob('*.o'))
    if not objects:
        sys.exit(f'x86_readable: no readable objects in build/x86-{m} (run make from readable/)')
    return [str(o) for o in objects]


def stands_alone(exe, link_map, m, platform):
    """The link check: every object the link map names is a readable one
    (build/x86-<m>/), the port's step or main (platform), one of MACHINE,
    or the C library's. Any other object of the repository, which is to
    say any faithful routine, fails the build."""
    repository = READABLE.parent
    allowed = {str((xb.STORE / 'host' / name).resolve()) for name in MACHINE}
    allowed.update(str(Path(o).resolve()) for o in platform)
    ours = str((READABLE / f'build/x86-{m}').resolve()) + os.sep
    machine, others = set(), set()
    for name in set(re.findall(r'(/[^\s()]+\.(?:o|a))\b', link_map.read_text())):
        path = str(Path(name).resolve())
        if not path.startswith(str(repository) + os.sep) or path.startswith(ours):
            continue
        (machine if path in allowed else others).add(path)
    if others:
        sys.exit(f'x86_readable: {exe.name} links objects that are not the readable program\'s:\n  '
                 + '\n  '.join(sorted(os.path.relpath(o, repository) for o in others)))
    if len(machine) != len(allowed):
        sys.exit(f'x86_readable: {link_map.name} does not name the objects {exe.name} was '
                 'linked from')


def build(cc, m, mode, certified_only=False):
    """x86_flight_build.build, linking the readable routines for mission m
    and, of the port's objects, MACHINE alone."""
    define, suffix = xb.MODES[mode]
    suffix += '-certified' if certified_only else ''
    flags = xb.flags(m, define, certified_only)
    xb.OUT.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in ('flight_dispatch', 'flight_main'):
        o = xb.OUT / f'{source}_{m}{suffix}.o'
        subprocess.run([cc, *flags, '-c', f'platform/x86/{source}.c', '-o', str(o)], cwd=PORT, check=True)
        objects.append(str(o))
    machine = [str(xb.STORE / 'host' / name) for name in MACHINE]
    exe = xb.OUT / f'x86-flight-{m}{suffix}'
    link_map = xb.OUT / f'x86-flight-{m}{suffix}.map'
    subprocess.run([cc, *objects, *machine, *readable_objects(m), *xb.LINK, '-Wl,--gc-sections',
                    f'-Wl,-Map={link_map}', '-lm', '-o', str(exe)], cwd=PORT, check=True)
    stands_alone(exe, link_map, m, objects)
    # The readable routines must be the ones linked: theirs are present, and
    # the faithful kernel's instruction helpers (k_go and the rest) absent,
    # as is every name the earlier builds kept a faithful routine under.
    symbols = subprocess.run(['nm', str(exe)], capture_output=True, text=True, check=True).stdout.split('\n')
    names = {line.split()[-1] for line in symbols if line.strip()}
    if (not names.issuperset(READABLE_SYMBOLS + MISSION_SYMBOLS[m]) or 'k_go' in names
            or any(name.startswith('faithful_') for name in names)):
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
