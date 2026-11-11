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
if the link loads any other file (stands_alone; tests/x86_link_check.py
tries the check on links it must fail). The step is compiled with the
port's index, and with its operand sets less their words
(own_operand_sets): the readable routines read those from the rope image.
The scenario suite
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
# program links, each with the functions and data it defines for other
# files. None is a translation of a routine of the listing. A function
# added to one of them fails the build until it is named here, where a
# reviewer sees it.
MACHINE = {
    # Where a word of fixed memory is (bank and address to its place in
    # the rope), the search of a routine table and of an operand set, and
    # the marker the step keeps DANZIG's place in its index with (a table
    # of one routine that refuses every state).
    'agc_transfer.o': ('agc_cm_danzig_marker', 'agc_erasable_point', 'agc_fixed_physical',
                       'agc_lm_danzig_marker', 'agc_resume_exact', 'agc_site_lookup',
                       'agc_transfer_next', 'agc_transfer_next_paged', 'agc_transfer_operand',
                       'agc_transfer_point', 'agc_transfer_resolve', 'agc_transfer_run',
                       'agc_transfer_run_at'),
    # One's complement words: negate, to and from an int.
    'agc_word.o': ('agc_add_register', 'agc_add_word', 'agc_classify', 'agc_compare',
                   'agc_correct', 'agc_diminished_abs', 'agc_extend', 'agc_from_int', 'agc_negate',
                   'agc_overflow', 'agc_to_int'),
    # The DSKY, the device: its relays and lights as channels 10 and 11
    # set them.
    'dsky_display.o': ('dsky_display_advance', 'dsky_display_flash_visible',
                       'dsky_display_output', 'dsky_display_render', 'dsky_display_reset'),
    # The channels a routine's device record carries, taken from the
    # hardware record before a step and given back after it.
    'flight_io.o': ('flight_io_enter', 'flight_io_leave'),
    # The gyro and CDU output drives, pulse by pulse.
    'resident_cdu.o': ('rk_cdu', 'rk_cdu_count', 'rk_cdu_drive'),
    # The timers (TIME1 to TIME6) and the entry to an interrupt.
    'resident_rupt.o': ('rk_hand_due', 'rk_rupt_take', 'rk_timer_phase', 'rk_timer_tick'),
    # The rope: the assembled program, read as data.
    'rope_image_host.o': ('apollo_rope_load', 'apollo_rope_read'),
}
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
    """The readable objects of mission m: one for each source the Makefile
    names (make x86-objects-<m>), not whatever lies under build/."""
    environment = {k: v for k, v in os.environ.items() if k not in ('MAKEFLAGS', 'MAKELEVEL', 'MFLAGS')}
    listed = subprocess.run(['make', '-s', '--no-print-directory', f'x86-objects-{m}'], cwd=READABLE,
                            env=environment, capture_output=True, text=True, check=True).stdout.split()
    objects = [str(READABLE / o) for o in listed]
    missing = [o for o in objects if not Path(o).is_file()]
    if not objects or missing:
        sys.exit(f'x86_readable: {len(missing) or "all"} readable objects of build/x86-{m} are not '
                 'built (run make x86-flight from readable/)')
    return objects


def toolchain_directories(cc):
    """Where cc keeps the C library and its start files."""
    out = subprocess.run([cc, '-print-search-dirs'], capture_output=True, text=True, check=True).stdout
    line = next((l for l in out.splitlines() if l.startswith('libraries:')), '')
    return sorted({str(Path(d).resolve()) for d in line.split('=', 1)[-1].split(':') if d})


def link_problems(map_text, cwd, expected, toolchain):
    """The link check. The link map lists every file the linker loaded (its
    LOAD lines), as the path the link named it by, from the directory cwd.
    Each must be one of expected (the readable objects, the step and main,
    MACHINE) or a file of the toolchain's (the C library, under one of the
    toolchain directories), and each of expected must be there. Returns
    what is wrong, a line each; nothing if the program stands alone."""
    expected = {str(Path(o).resolve()) for o in expected}
    seen, problems = set(), []
    for line in map_text.splitlines():
        if not line.startswith('LOAD '):
            continue
        path = str((Path(cwd) / line[len('LOAD '):].strip()).resolve())
        if path in expected:
            seen.add(path)
        elif not any(path.startswith(d + os.sep) for d in toolchain):
            problems.append(f'loads {path}, which is not the readable program\'s')
    problems += [f'does not load {path}' for path in sorted(expected - seen)]
    return problems


def machine_problems():
    """Each MACHINE object defines the names listed for it and no other."""
    problems = []
    for name, pinned in MACHINE.items():
        out = subprocess.run(['nm', '--defined-only', '--extern-only', str(xb.STORE / 'host' / name)],
                             capture_output=True, text=True, check=True).stdout
        defined = {line.split()[-1] for line in out.splitlines() if line.strip()}
        if defined != set(pinned):
            problems.append(f'{name} defines {" ".join(sorted(defined ^ set(pinned)))}, which MACHINE '
                            '(tests/x86_readable.py) does not say')
    return problems


def stands_alone(exe, link_map, expected, cc):
    """Fail the build unless exe was linked from expected alone."""
    problems = link_problems(link_map.read_text(), PORT, expected, toolchain_directories(cc))
    problems += machine_problems()
    if problems:
        sys.exit(f'x86_readable: {exe.name} does not stand alone:\n  ' + '\n  '.join(problems))


def own_operand_sets(m):
    """The port's operand sets of mission m (c64-port/build/flight-store/
    <m>-sites.inc) with every word taken out: the addresses alone, each
    with WORD_IN_ROPE (src/binding/binding.h) where its word was. The
    port's generator lifts the sets from the faithful sources: the words
    after the TCs each source's routines serve, address and word. The
    step picks a routine by them (the CM's POSTJUMP is in two tables, at
    two costs: one serves the self-check's idle loop alone, the other the
    rest), so the addresses are part of the port's index, as the tables'
    order and costs are. The words are the rope's, and the readable
    program reads them there (fixed_word_at). Returns the file's path."""
    word = re.compile(r'\{(\d+)u,0?\d+u\}')
    sets = re.compile(r'static const agc_site_operand \w+\[\]=\{((?:\{\w+,0?\d+u\},?)+)\};')
    lines = (xb.STORE / f'{m}-sites.inc').read_text().splitlines()
    out = [f'/* Generated by readable/tests/x86_readable.py from c64-port/build/flight-store/{m}-sites.inc: '
           'the addresses of each operand set, without the words (0100000 in their place: the readable '
           'routines read each word from the rope image). */']
    for line in lines[1:]:
        if not sets.fullmatch(line):
            sys.exit(f'x86_readable: a line of {m}-sites.inc is not an operand set: {line[:60]}')
        out.append(word.sub(r'{\1u,0100000u}', line))
    if len(out) < 2 or not lines[0].startswith('/*'):
        sys.exit(f'x86_readable: {m}-sites.inc is not the file expected')
    text = '\n'.join(out) + '\n'
    if re.search(r',0?[0-7]+u\}', text.replace(',0100000u}', '').replace('{AGC_TRANSFER_NONE,0u}', '')):
        sys.exit(f'x86_readable: a word of {m}-sites.inc was left in the readable program\'s sets')
    path = xb.OUT / f'{m}-operand-addresses.inc'
    if not path.exists() or path.read_text() != text:
        path.write_text(text)
    return path


def step_flags(m, define, certified_only=False):
    """The port's compile flags for the step and main, with
    own_operand_sets in place of the port's sets."""
    flags = xb.flags(m, define, certified_only)
    prefix = '-DFLIGHT_HOST_SITES='
    ours = [i for i, flag in enumerate(flags) if flag.startswith(prefix)]
    if len(ours) != 1:
        sys.exit('x86_readable: the port\'s compile flags do not name its operand sets as expected')
    flags[ours[0]] = f'{prefix}"{own_operand_sets(m)}"'
    if any('-sites.inc' in flag for flag in flags):
        sys.exit('x86_readable: the port\'s operand sets are still named in the compile flags')
    return flags


def build(cc, m, mode, certified_only=False):
    """x86_flight_build.build, linking the readable routines for mission m
    and, of the port's objects, MACHINE alone."""
    define, suffix = xb.MODES[mode]
    suffix += '-certified' if certified_only else ''
    flags = step_flags(m, define, certified_only)
    xb.OUT.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in ('flight_dispatch', 'flight_main'):
        o = xb.OUT / f'{source}_{m}{suffix}.o'
        subprocess.run([cc, *flags, '-c', f'platform/x86/{source}.c', '-o', str(o)], cwd=PORT, check=True)
        objects.append(str(o))
    objects += [str(xb.STORE / 'host' / name) for name in MACHINE]
    objects += readable_objects(m)
    exe = xb.OUT / f'x86-flight-{m}{suffix}'
    link_map = xb.OUT / f'x86-flight-{m}{suffix}.map'
    subprocess.run([cc, *objects, *xb.LINK, '-Wl,--gc-sections', f'-Wl,-Map={link_map}', '-lm', '-o',
                    str(exe)], cwd=PORT, check=True)
    stands_alone(exe, link_map, objects, cc)
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
