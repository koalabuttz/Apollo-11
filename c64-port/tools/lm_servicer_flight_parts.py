#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check the two flight parts of src/lm_servicer.c against the original
program (docs/LM-SERVICER-FLIGHT-PARTS.md).

A flight build of SERVICER (AGC_FLIGHT_INTERPRETER: interpretive code is the
native interpreter's) does not fit one 16 KB job slot, so it is linked as two
modules, LMSV_FLIGHT_PART 1 and 2, cut at LMSV_PART_CUT. For each part this
builds the host snapshot with the part's flags and runs the oracle
(tools/lm_servicer_reference.c) with a segment file: the part's basic key
slots, a stop before every basic word of the other part and before every
order pair (NEWOPS with LOC there). Every case the part runs must be
byte-identical to the original program stopped there. A refused case leaves
the image unchanged; those the whole flight build (one module) does not
refuse are runs that cross the cut at a word that is not a key, which the
other part's module could not take over, and are counted separately.

  lm_servicer_flight_parts.py
"""
import re
import subprocess
import sys

sys.path.insert(0, str(__import__('pathlib').Path(__file__).resolve().parent))
import lm_servicer_freeze as fz  # noqa: E402

R, B = fz.R, fz.B / 'flight-parts'


def part_cut():
    """(cut, keys) from src/lm_servicer.c, checked against basic_entries."""
    src = (R / 'src/lm_servicer.c').read_text()
    m = re.search(r'#define LMSV_PART_CUT PH\((\d\d),(0[0-7]+)\).*\n#define LMSV_PART_KEYS (\d+)u', src)
    cut, n = int(m.group(1), 8) * 1024 + int(m.group(2), 8) - 0o2000, int(m.group(3))
    body = src[src.index('basic_entries[]={'):src.index('interp_entries[]={')]
    keys = [int(b, 8) * 1024 + int(a, 8) - 0o2000 for b, a in re.findall(r'[RX](\d\d)\((0[0-7]+)\)', body)]
    assert keys == sorted(keys) and keys[n] == cut, 'LMSV_PART_KEYS is not the index of LMSV_PART_CUT'
    return cut


def crossings(cut):
    """Every transfer in basic() that crosses the cut (a TC or TCF target, a
    TC's return word, a to() target, a fall-through into the next word) must
    land on a basic key."""
    src = (R / 'src/lm_servicer.c').read_text()
    i = src.index('static int basic(')
    body = src[i:src.index('\n}\n', i)]
    keys = {ph(b, a) for b, a in re.findall(r'[RX](\d\d)\((0[0-7]+)\)',
                                            src[src.index('basic_entries[]={'):src.index('interp_entries[]={')])}
    side = lambda p: p < cut
    bad, at, prev = [], None, None
    for m in re.finditer(r'AT\((\d\d),(0[0-7]+)\)|TC\((0[0-7]+),(\d\d),(0[0-7]+)\)|GOTO\((\d\d),(0[0-7]+)\)'
                         r'|to\(s,(0[0-7]+)u?\)', body):
        g = m.groups()
        if g[0]:
            here = (g[0], ph(g[0], g[1]))
            if at and side(at[1]) != side(here[1]) and here[1] not in keys \
                    and not re.search(r'return[^;]*;\s*(/\*.*?\*/\s*)*$', body[prev:m.start()], re.S):
                bad.append(f'fall-through {g[0]},{g[1]}')
            at, prev = here, m.end()
            continue
        if g[2]:
            t = ph(g[3], g[4])
            if side(t) != side(at[1]):
                bad += [what for k, what in ((t, f'TC target {g[3]},{g[4]}'),
                                              (ph(at[0], g[2]), f'TC return {at[0]},{g[2]}')) if k not in keys]
            continue
        t, what = (ph(g[5], g[6]), f'GOTO {g[5]},{g[6]}') if g[5] else (ph(at[0], g[7]), f'to() {at[0]},{g[7]}')
        if side(t) != side(at[1]) and t not in keys:
            bad.append(what)
    if bad:
        sys.exit(f'FAIL: transfers across the cut that land on no key: {bad}')
    print('PASS: every transfer across the cut lands on a basic key')


def ph(b, a):
    return int(b, 8) * 1024 + int(a, 8) - 0o2000


def run(cmd):
    return subprocess.run(cmd, cwd=R, check=True, capture_output=True).stdout.decode().splitlines()


def build(exe, *defines):
    subprocess.run(['cc', *fz.FLAGS, '-Wall', '-Wextra', '-Werror', '-DAGC_FLIGHT_INTERPRETER', *defines,
                    *fz.NATIVE, 'platform/host/lm_servicer_snapshot.c', '-o', str(exe)], cwd=R, check=True)
    return {int(l.split()[0]): l for l in run([str(exe)])}


def main():
    B.mkdir(parents=True, exist_ok=True)
    if not (fz.B / 'agc_engine_hardware_dv.o').exists():
        subprocess.run([sys.executable, 'tools/prepare_hardware_dv.py'], cwd=R, check=True)
    subprocess.run(['cc', *fz.FLAGS, '-w', 'tools/lm_servicer_reference.c', *fz.CORE,
                    '-o', str(B / 'reference')], cwd=R, check=True)
    cut = part_cut()
    crossings(cut)
    of, slots = fz.groups()
    names = fz.keys()
    whole = build(B / 'native-whole')
    for p in (1, 2):
        mine = p == 1 and (lambda ph: ph < cut) or (lambda ph: ph >= cut)
        own = [i for i, k in enumerate(names) if ('B', int(k[:2], 8) * 1024 + int(k[3:], 8) - 0o2000) in of
               and mine(int(k[:2], 8) * 1024 + int(k[3:], 8) - 0o2000)]
        seg = B / f'part{p}.segment'
        seg.write_text(' '.join(map(str, own)) + '\n' +
                       ''.join(f'{k} {ph}\n' for (k, ph) in sorted(of) if k == 'I' or not mine(ph)))
        ref = run([str(B / 'reference'), str(fz.ROPE), str(seg)])
        nat = build(B / f'native-part{p}', f'-DLMSV_FLIGHT_PART={p}')
        assert len(ref) == len(nat), f'part {p}: case counts differ'
        refused = crossing = 0
        for b in ref:
            n = int(b.split()[0])
            a = nat[n]
            if int(a.split()[1]) == fz.REFUSED:
                refused += 1
                crossing += int(whole[n].split()[1]) != fz.REFUSED
            elif a != b:
                sys.exit(f'FAIL: flight part {p} case {n}: native differs from the original program')
        print(f'PASS: lm_servicer flight part {p} ({len(own)} keys): {len(ref) - refused} of {len(ref)} cases equal '
              f'the original program stopped at the cut or NEWOPS; {refused} refused unchanged, '
              f'{crossing} of them at a non-key word across the cut')


if __name__ == '__main__':
    main()
