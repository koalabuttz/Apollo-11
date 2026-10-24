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
import struct
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
        # The INTERPRETER targets name it through irc_rope_path() (next to
        # the source): the yaYUL binary of the spec's mission, by absolute
        # path. Without it the indexed fixed reads (CAF BIT14) refuse.
        rope = None
        if 'apollo_rope_load' in own:
            lit = re.search(r'#define IRC_ROPE "([^"]+)"', own)
            rope = lit.group(1) if lit else os.path.join(
                R, 'build/fixed-data/%s/MAIN.agc.bin' % ('Comanche055' if m == 'cm' else 'Luminary099'))
        if rope:
            f.write('#include "rope_image.h"\nstatic int rope_loaded;\n')
        f.write('const agc_native_program *irc_program(uint8_t m){(void)m;\n%s%s return &prog;}\n' % (
            ' if(!rope_loaded)rope_loaded=apollo_rope_load(%du,"%s")?1:-1;\n' % (int(m == 'cm'), rope)
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
    any_eb(a.spec, base, cases, whole, runner, out)


# An EB the differential's cases never start under (they draw E5 for the
# fast-window files).
OTHER_EB = 0o1000


def any_eb_keys(base):
    """The keys whose per-key entry runs under any EB (basic_sane_any_eb:
    spec 'any_eb_entries', or a routine that sets EB first), as Z and the
    FB the entry checks (None: fixed-fixed, any FB)."""
    src = open(os.path.join(R, 'src', '%s.c' % base)).read()
    keys = []
    for m in re.finditer(r'static enum exec_status r\w+_abi\(.*?\n(.*?)\n}', src, re.S):
        b = re.search(r'basic_sane_any_eb\([^,]*,[^,]*,0(\d+)u\)', m.group(1))
        k = re.search(r'basic_run\(s,\([^)]*\)v,(\d+)u\)', m.group(1))
        if b and k:
            at = int(k.group(1))
            z = 0o4000 + at - 2048 if at // 1024 in (2, 3) else 0o2000 + at % 1024
            keys.append((z, None if b.group(1) == '177777' else int(b.group(1), 8)))
    return keys


def any_eb(spec, base, cases, whole, runner, out):
    """The parts' flight entry must take an any-EB key under an EB other
    than E5, as the whole build's per-key entry does (P06 calls FINETIME +1
    under its own EB). The differential's cases that start at such a key,
    run again under OTHER_EB, must end the same in the parts as in the whole
    build, and the whole build must not refuse them."""
    keys = any_eb_keys(base)
    if not keys:
        return
    d = open(cases, 'rb').read()
    o = 4
    for _ in range(2):
        k, = struct.unpack_from('<H', d, o + 2)
        o += 4 + 4 * k
        for _ in range(2):
            k, = struct.unpack_from('<H', d, o)
            o += 2 + 2 * k
        if struct.unpack_from('<I', d, 0)[0] == 0x33435249:
            k, = struct.unpack_from('<H', d, o)
            o += 2 + 4 * k
    head, (count,), o = d[:o], struct.unpack_from('<I', d, o), o + 4
    picked, seen = [], set()
    for _ in range(count):
        seed, cm, ie, sb, x, ne = struct.unpack_from('<IBBHHH', d, o)
        cut = d[o + 12:o + 14] if ie & 0x80 else b''
        at = o + 12 + len(cut)
        edits = [struct.unpack_from('<2H', d, at + 4 * i) for i in range(ne)]
        o = at + 4 * ne
        e = dict(edits)
        for z, fb in keys:
            if e.get(5) == z and (fb is None or e.get(4) == fb) and 4 in e:
                seen.add(z)
                more = [(3, OTHER_EB), (6, (e[4] & 0o76000) | OTHER_EB >> 8)]
                edits2 = [x for x in edits if x[0] not in (3, 6)] + more
                picked.append(struct.pack('<IBBHHH', seed, cm, ie, sb, x, len(edits2)) + cut
                              + b''.join(struct.pack('<2H', *x) for x in edits2))
    missing = sorted({z for z, _ in keys} - seen)
    if missing:
        sys.exit('FAIL: no %s case starts at the any-EB key(s) %s' % (spec, ', '.join('%05o' % z for z in missing)))
    path = os.path.join(out, 'any-eb-cases.bin')
    with open(path, 'wb') as f:
        f.write(head + struct.pack('<I', len(picked)) + b''.join(picked))
    w = subprocess.run([whole, path], cwd=R, check=True, capture_output=True, text=True).stdout.splitlines()
    p = subprocess.run([runner, path], cwd=R, check=True, capture_output=True, text=True).stdout.splitlines()
    refused = [x for x in w if x.split()[1] == '1']
    if refused:
        sys.exit('FAIL: the whole build refuses %d of %d %s any-EB cases' % (len(refused), len(w), spec))
    bad = [i for i, (x, y) in enumerate(zip(w, p)) if x != y]
    if len(w) != len(p) or bad:
        for i in bad[:5]:
            print('any-EB case %d: whole %s / parts %s' % (i, w[i][:60], p[i][:60]))
        sys.exit('FAIL: %d of %d %s cases at an any-EB key under EB %05o end differently in the flight parts'
                 % (len(bad), len(w), spec, OTHER_EB))
    print('PASS: %d %s cases at its %d any-EB keys under EB %05o end the same in the flight parts'
          % (len(w), spec, len(keys), OTHER_EB))


if __name__ == '__main__':
    main()
