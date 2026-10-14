#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check the flight build of the step-machine files whose chains stop at
the interpreter (AGC_FLIGHT_INTERPRETER) against the original program.

Under the native-interpreter decision the flight store runs interpretive
code through the interpreter module, so a flight build of these files
(-DAGC_FLIGHT_INTERPRETER) ends a chain where it reaches NEWOPS instead of
running the compiled interpretive orders. For each file this builds its
host snapshot with the flag, and its yaAGC reference driver with the
file's leave test widened to stop at NEWOPS too (the reference's own TU is
compiled with -D<leaves>=<leaves>_flight; a shim defines that as NEWOPS or
the file's test). Every case the flight build runs must be byte-identical
to the original program stopped there; a refused case is counted (the
snapshot prints a refusal only with the image unchanged).

  flight_interp_check.py [FILE ...]   (default: all of FILES)
"""
import importlib
import pathlib
import subprocess
import sys

R = pathlib.Path(__file__).resolve().parents[1]
B = R / 'build/flight-interp'
NEWOPS = 0o6070  # Luminary099
# file stem: (freeze module, leave test, mk-style name)
FILES = {
    'lm_ags_init': ('lm_ags_init_freeze', 'ags_leaves', 'lm-ags-init'),
    'lm_burn_baby': ('lm_burn_baby_freeze', 'lmbb_leaves', 'lm-burn-baby'),
    'lm_lunar_landing': ('lm_lunar_landing_freeze', 'lld_leaves', 'lm-lunar-landing'),
    'lm_imu_performance': ('lm_imu_performance_freeze', 'ipt_leaves', 'lm-imu-performance'),
}
REFUSED = 2


def run(cmd, out=None):
    if out is None:
        subprocess.run(cmd, cwd=R, check=True)
        return None
    with open(out, 'w') as f:
        subprocess.run(cmd, cwd=R, check=True, stdout=f)
    return out


def check(stem):
    mod_name, leaves, name = FILES[stem]
    sys.path.insert(0, str(R / 'tools'))
    fz = importlib.import_module(mod_name)
    ref_core = [c for c in fz.CORE if c not in fz.NATIVE]
    for c in ref_core:
        if c.endswith('.o') and not pathlib.Path(c).exists():
            run([sys.executable, 'tools/prepare_no_imu_device_engine.py' if 'no_imu_device' in c
                 else 'tools/prepare_hardware_dv.py'])
    native = B / f'{name}-native'
    run(['cc', *fz.FLAGS, '-Wall', '-Wextra', '-Werror', '-DAGC_FLIGHT_INTERPRETER', *fz.NATIVE,
         f'platform/host/{stem}_snapshot.c', '-o', str(native)])
    shim = B / f'{name}-leaves.c'
    shim.write_text('#include "agc_word.h"\n'
                    f'int {leaves}(agc_word,agc_word,agc_word,agc_word,agc_word);\n'
                    f'int {leaves}_flight(agc_word z,agc_word fb,agc_word sb,agc_word loc,agc_word bs)'
                    f'{{return z==0{NEWOPS:o}u||{leaves}(z,fb,sb,loc,bs);}}\n')
    ref_o = B / f'{name}-reference.o'
    run(['cc', *fz.FLAGS, '-w', f'-D{leaves}={leaves}_flight', '-c', f'tools/{stem}_reference.c', '-o', str(ref_o)])
    reference = B / f'{name}-reference'
    run(['cc', *fz.FLAGS, '-w', str(ref_o), str(shim), *fz.CORE, '-o', str(reference)])
    nat = run([str(native)], B / f'{name}-native.txt')
    ref = run([str(reference), str(fz.ROPE)], B / f'{name}-oracle.txt')
    cases = refused = 0
    with open(nat) as fn, open(ref) as fr:
        for i, (a, b) in enumerate(zip(fn, fr)):
            r = a.split(maxsplit=2)
            assert int(r[0]) == i
            cases += 1
            if int(r[1]) == REFUSED:
                refused += 1
            elif a != b:
                sys.exit(f'FAIL: {name} flight case {i}: native differs from the original program stopped at NEWOPS')
    assert cases and sum(1 for _ in open(ref)) == cases, f'{name}: case counts differ'
    print(f'PASS: {name} flight build: {cases - refused} of {cases} cases equal the original program '
          f'stopped at its leave points or NEWOPS ({refused} refused unchanged)')
    for p in (nat, ref):
        pathlib.Path(p).unlink()


def main():
    B.mkdir(parents=True, exist_ok=True)
    for stem in sys.argv[1:] or FILES:
        check(stem)


if __name__ == '__main__':
    main()
