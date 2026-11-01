#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The readable Pinball through the port's own Pinball tests.

The port checks PINBALL_GAME_BUTTONS_AND_LIGHTS through its routine
tables (c64-port/tests/lanec_pin_transfer_cases.c): every routine of the
tables from its own entry, and every genuine BANKCALL or POSTJUMP caller
outside the file, chained through the routines that follow until the
chain leaves the file, against the original program run live in Virtual
AGC (c64-port/tools/lanec/diff.py). Every routine of those tables runs
through the trampoline lanec_pin_run_lm or lanec_pin_run_cm, which the
readable binding (src/binding/pinball_routines.c) replaces.

This links the port's test runner (build/lanec-native, every lane C suite
and tools/lanec/native.c) with the readable objects given on the command
line in place of the trampoline, which it keeps as
faithful_lanec_pin_run_<m> for the entries not yet readable, into
build/lanec-native-readable. Then, for each suite:

  - every chain the program takes must end in the original program's
    complete state (diff.py, with the readable runner);
  - every chain the faithful port takes the readable program must take
    too (the faithful runner's rows against the readable runner's; see
    SUITES).

  pinball_transfer.py READABLE_LM.o READABLE_CM.o
"""
import os, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
CC = os.environ.get('CC', 'cc')
NATIVE = READABLE / 'build/lanec-native-readable'
ORACLE = READABLE / 'build/lanec-oracle-readable'
STEPS = READABLE / 'build/pinball-steps'
TRAMPOLINE = 'src/lanec_pin_trampoline.c'
SUITE_LIST = 'tests/lanec_suites.c'
# The suites that run the tables: whether they compare with the original
# program (--refusals) or are all out-of-domain (--refusals-only), and
# whether the readable program must take every chain the faithful port
# takes. pin-transfer-c64 registers one module's entries at a time, as a
# C64 program does, so a chain the readable code hands to a routine that
# module does not register (a TC POSTJUMP the module runs on through) ends
# there: its chains are compared, not counted.
# pin-stirred's chains taken must not fall below this (a readable routine
# refusing what it should take).
STIRRED_FLOOR = 7700
SUITES = (('pin-transfer', '--refusals', True), ('pin-transfer-c64', '--refusals', False),
          ('pin-decdsp3', '--refusals', True), ('pin-refuse', '--refusals-only', False))


def port_make(*targets, eval_text=None):
    command = ['make', '-s', '-C', str(PORT), '--no-print-directory', 'SKIP_EVIDENCE_RESTORE=1']
    if eval_text:
        command += ['--eval', eval_text]
    return subprocess.run(command + list(targets), check=True, capture_output=bool(eval_text),
                          text=True).stdout


def port_value(name):
    return port_make('print', eval_text=f'print: ; @echo $({name})').split()


def compile_all(flags, units):
    """Each (source, extra flags) to an object in build/lanec-port/, when
    the source is newer than it; the objects."""
    out = READABLE / 'build/lanec-port'
    out.mkdir(parents=True, exist_ok=True)

    def one(unit):
        source, extra = unit
        name = (source.replace('../', '').replace('/', '_') + ''.join(extra).replace('=', '_')
                .replace('-', '_').replace('/', '_'))[:180]
        o = out / (name + '.o')
        if not o.exists() or o.stat().st_mtime < (PORT / source).stat().st_mtime:
            subprocess.run([CC, '-O2', *flags, *extra, '-c', source, '-o', str(o)], cwd=PORT, check=True)
        return str(o)
    with ThreadPoolExecutor(os.cpu_count() or 1) as pool:
        return list(pool.map(one, units))


def build(objects):
    port_make('build/lanec-native', 'build/lanec-oracle')
    flags = port_value('LANEC_FLAGS')
    sources = port_value('LANEC_CORE') + sorted(set(port_value('LANEC_SUITE_SRC')))
    if TRAMPOLINE not in sources or SUITE_LIST not in sources:
        sys.exit(f'pinball_transfer: the port runner has no {TRAMPOLINE} or {SUITE_LIST}')
    yaagc = port_value('LANEC_YAAGC')[0]
    renamed = {TRAMPOLINE: ['-Dlanec_pin_run_lm=faithful_lanec_pin_run_lm',
                            '-Dlanec_pin_run_cm=faithful_lanec_pin_run_cm'],
               SUITE_LIST: ['-Dlanec_find_suite=port_lanec_find_suite']}
    stirred = str(READABLE / 'tests/pinball_stirred_cases.c')
    steps = str(READABLE / 'tests/pinball_steps.c')
    units = [(s, renamed.get(s, [])) for s in sources] + [(stirred, ['-Wconversion', '-pedantic']),
                                                         (steps, ['-Wconversion', '-pedantic']),
                                                         ('tools/lanec/native.c', []),
                                                         ('tools/lanec/oracle.c', [f'-I{yaagc}'])]
    compiled = dict(zip([u[0] for u in units], compile_all(flags, units)))
    common = [compiled[s] for s in sources if s != TRAMPOLINE] + [compiled[stirred]]
    NATIVE.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([CC, '-O2', *flags, *common, compiled[TRAMPOLINE], compiled['tools/lanec/native.c'],
                    *objects, '-o', str(NATIVE)], cwd=PORT, check=True)
    # The step check (tests/pinball_steps.c): the readable trampoline as
    # readable_lanec_pin_run_<m>, beside the faithful one.
    steps_objects = []
    for o in objects:
        renamed_o = READABLE / 'build' / ('steps-' + Path(o).name)
        subprocess.run(['objcopy', *(f'--redefine-sym=lanec_pin_run_{m}=readable_lanec_pin_run_{m}' for m in ('lm', 'cm')),
                        o, str(renamed_o)], check=True)
        steps_objects.append(str(renamed_o))
    subprocess.run([CC, '-O2', *flags, *common, compiled[TRAMPOLINE], compiled[steps], *steps_objects,
                    '-o', str(STEPS)], cwd=PORT, check=True)
    # The oracle: the port's, with pin-stirred (the port's own trampoline;
    # the oracle runs the original program).
    port_trampoline = compile_all(flags, [(TRAMPOLINE, [])])[0]
    subprocess.run([CC, '-O2', *flags, *common, port_trampoline, compiled['tools/lanec/oracle.c'],
                    'build/agc_engine_hardware_dv.o', f'{yaagc}/agc_engine_init.o', f'{yaagc}/rfopen.o',
                    '-o', str(ORACLE)], cwd=PORT, check=True)


def rows(suite, tag):
    text = (PORT / f'build/lanec-{suite}-native{tag}.txt').read_text().splitlines()
    return [line.split()[-1] != '1' for line in text]


def main():
    objects = [str(Path(o).resolve()) for o in sys.argv[1:]]
    if len(objects) != 2:
        sys.exit(__doc__)
    build(objects)
    bad = 0
    for suite, mode, counted in SUITES:
        for native in ('build/lanec-native', str(NATIVE)):
            subprocess.run([sys.executable, 'tools/lanec/diff.py', suite, mode, '--native', native],
                           cwd=PORT, check=True)
        if not counted:
            continue
        faithful, readable = rows(suite, ''), rows(suite, '-readable')
        lost = [i for i, (f, r) in enumerate(zip(faithful, readable)) if f and not r]
        print(f"{'PASS' if not lost and len(faithful) == len(readable) else 'FAIL'}: {suite}: "
              f"{sum(readable)} chains taken with the readable Pinball, {sum(faithful)} by the faithful port"
              + (f'; refused here, taken there: cases {lost[:10]}' if lost else ''))
        bad |= bool(lost) or len(faithful) != len(readable)
    # pin-stirred (tests/pinball_stirred_cases.c), on this runner and oracle.
    subprocess.run([sys.executable, 'tools/lanec/diff.py', 'pin-stirred', '--refusals', '--native', str(NATIVE),
                    '--oracle', str(ORACLE)], cwd=PORT, check=True)
    taken = sum(rows('pin-stirred', '-readable'))
    print(f"{'PASS' if taken >= STIRRED_FLOOR else 'FAIL'}: pin-stirred: {taken} chains taken with the readable "
          f'Pinball (at least {STIRRED_FLOOR})')
    bad |= taken < STIRRED_FLOOR
    # Step for step as the faithful port (tests/pinball_steps.c).
    bad |= subprocess.run([str(STEPS)], cwd=PORT).returncode != 0
    return bad


if __name__ == '__main__':
    sys.exit(main())
