#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check a compiled-routine file's flight parts (spec 'flight_parts',
docs/FLIGHT-PARTS.md) against its whole build.

The C64 flight store links such a file as one module per part
(<SYM>_FLIGHT_PART=j). Each part keeps its own basic words; a run that
reaches another part's word stops there with Z at it, and the transfer
engine goes on in the module that holds it. This builds every part for the
host (its tables renamed), one runner with all the parts' basic tables and
the file's interpretive table, as the flight store's modules together, and
runs the differential's cases (tools/interp_routine_differential.py, which
also checks the whole build against the original program) through it and
through the whole build. Every case must end the same way: Z, the 2048
erasable words and the interrupt state. A case the parts refuse that the
whole build runs is a hand-off at a word no part keys, and fails the check.

  flight_parts_check.py <spec> [--cases N]
"""
import argparse
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import interp_routine_differential as IRD  # noqa: E402
import interp_specs  # noqa: E402

R, B = IRD.R, IRD.B


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('spec')
    ap.add_argument('--cases', type=int, default=64)
    a = ap.parse_args()
    spec = interp_specs.SPECS[a.spec]
    (m,) = spec['missions']
    sym, base = spec['symbol'][m], spec['base'][m]
    S, n = sym.upper(), max(spec.get('flight_part_map') or [len(spec['flight_parts']) + 1])
    # The differential writes the cases and the whole build's runner, and
    # checks the whole build against the original program.
    subprocess.run([sys.executable, 'tools/interp_routine_differential.py', a.spec, '--cases', str(a.cases)],
                   cwd=R, check=True, stdout=subprocess.DEVNULL)
    cases, whole = os.path.join(B, '%s-cases.bin' % a.spec), os.path.join(B, '%s-native' % a.spec)
    out = os.path.join(B, 'flight-parts', a.spec)
    os.makedirs(out, exist_ok=True)
    names = ('basic', 'interpretive', 'points', 'cadr_words', 'externals')
    steps = '-DIRC_MAX_STEPS=%d' % (4 * spec.get('max_steps', 200))
    objs = []
    for j in range(1, n + 1):
        o = os.path.join(out, 'part%d.o' % j)
        subprocess.run(['cc', *IRD.FLAGS, steps, '-D%s_FLIGHT_PART=%d' % (S, j),
                        *('-D%s_%s=%s_%s_p%d' % (sym, x, sym, x, j) for x in names),
                        '-c', 'src/%s.c' % base, '-o', o], cwd=R, check=True)
        objs.append(o)
    target = os.path.join(out, 'target.c')
    with open(target, 'w') as f:
        f.write('#include "agc_transfer.h"\n#include "%s.h"\n' % base)
        f.write(''.join('extern const agc_routine_table %s_basic_p%d,%s_interpretive_p%d;\n' % (sym, j, sym, j)
                        for j in range(1, n + 1)))
        f.write('const agc_native_program *irc_program(uint8_t cm);\n')
        # (The fields of the spec's own target, tests/<file>_target.c: its
        # interpretive table count and NEWOPS, and the hardware record.)
        own = open(os.path.join(R, 'tests', '%s_target.c' % base)).read()
        f.write('static agc_routine_table parts[%d];\n%s' % (n, 'static agc_hardware hw;\n' if '&hw' in own else ''))
        icount = re.search(r'&%s_interpretive,(\d)' % sym, own).group(1)
        f.write('static agc_native_program prog={parts,%d,&%s_interpretive_p1,%s,0,0,'
                '(uint16_t)(2048u+0%ou-04000u),0164,0165,0,0,%s};\n'
                % (n, sym, icount, 0o6070 if m == 'lm' else 0o6037, '&hw' if '&hw' in own else '0'))
        # A 'fixed_rope' spec's target loads the rope image its routines
        # read their callers' words from (as tests/<file>_target.c does).
        rope = re.search(r'#define IRC_ROPE "([^"]+)"', own) if 'apollo_rope_load' in own else None
        if rope:
            f.write('#include "rope_image.h"\nstatic int rope_loaded;\n')
        f.write('const agc_native_program *irc_program(uint8_t m){(void)m;\n%s%s return &prog;}\n' % (
            ' if(!rope_loaded)rope_loaded=apollo_rope_load(%du,"%s")?1:-1;\n' % (int(m == 'cm'), rope.group(1))
            if rope else '', ''.join(' parts[%d]=%s_basic_p%d;\n' % (j - 1, sym, j) for j in range(1, n + 1))))
    runner = os.path.join(out, 'native-parts')
    subprocess.run(['cc', *IRD.FLAGS, steps, *IRD.cut_flags(spec), *objs, *spec.get('native_sources', []),
                    *IRD.common(spec), target, 'tools/interp_routine_native.c', '-o', runner], cwd=R, check=True)
    w = subprocess.run([whole, cases], cwd=R, check=True, capture_output=True, text=True).stdout.splitlines()
    p = subprocess.run([runner, cases], cwd=R, check=True, capture_output=True, text=True).stdout.splitlines()
    assert len(w) == len(p), 'case counts differ'
    bad = [i for i, (x, y) in enumerate(zip(w, p)) if x != y]
    if bad:
        for i in bad[:5]:
            print('case %d: whole %s / parts %s' % (i, w[i][:60], p[i][:60]))
        sys.exit('FAIL: %d of %d %s cases end differently in the flight parts' % (len(bad), len(w), a.spec))
    print('PASS: %d %s cases end the same in its %d flight parts chained as in the whole build' % (len(w), a.spec, n))


if __name__ == '__main__':
    main()
