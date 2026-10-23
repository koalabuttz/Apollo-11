#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""EXIT-return keys of the FLIGHT_INTERP_STOP files (docs/FLIGHT-INTERP-STOP.md).

On the C64 the native interpreter runs these files' interpretive code. Its
EXIT leaves Z at the basic word after the EXIT order word and its operand
words, and the flight program looks that word up as a basic key. In the host
build the file's step machine runs on across EXIT in one call, so the word
had no key of its own, and a C64 flight run stopped there UNMAPPED (LM P63:
33,2223 after GETABVAL).

This derives every such word from the Luminary099 listing (each EXIT in the
file: an order word that is EXIT alone, 077776, resumes at the next word; a
pair whose second order is EXIT resumes after its first order's operand
word) and, for each file that opts in (OPT_IN), checks that its basic table
(<stem>_routines, built with -DAGC_FLIGHT_INTERPRETER as the flight store
builds it) keys every one, in ascending order. Files not opted in are
reported with the words they leave unkeyed.

  flight_exit_returns.py            check the opted-in files, report the rest
  flight_exit_returns.py --list     print every file's EXIT-return words
"""
import argparse
import pathlib
import re
import subprocess
import sys
import tempfile

R = pathlib.Path(__file__).resolve().parents[1]
LST = R / 'build/fixed-data/Luminary099/assembly.lst'
WORD = re.compile(r'^\d{6},\d{6}: (?:(\d\d),|   )(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
# stem: the listing files it translates.
FILES = {'lm_servicer': ('SERVICER.agc',),
         'lm_ags_init': ('AGS_INITIALIZATION.agc',),
         'lm_lunar_landing': ('THE_LUNAR_LANDING.agc',),
         'lm_burn_baby': ('BURN_BABY_BURN--MASTER_IGNITION_ROUTINE.agc',),
         'lm_imu_performance': ('IMU_PERFORMANCE_TEST_2.agc', 'IMU_PERFORMANCE_TESTS_4.agc')}
# Files whose basic tables key their EXIT returns (checked).
OPT_IN = {'lm_servicer', 'lm_imu_performance', 'lm_ags_init', 'lm_lunar_landing', 'lm_burn_baby'}
# Interpretive orders without an operand word: a pair "X EXIT" with one of
# these resumes at the next word.
UNARY = set('ABVAL ABS ARCCOS ARCSIN COS DCOMP DSQ PUSH ROUND RVQ SIN SQRT STADR UNIT VCOMP VDEF VSQ'.split())


def physical(bank, addr):
    return bank * 1024 + addr - 0o2000 if addr >= 0o2000 else addr


def name(p):
    return f'{p:05o}' if p < 4096 else f'{p // 1024:02o},{p % 1024 + 0o2000:04o}'


def exit_returns(files):
    """Sorted physical addresses of the basic words EXITs in files resume at.
    Fails when a named file is not in the listing, so a wrong name cannot
    read as "no EXIT returns"."""
    words, order, on, seen = {}, [], False, set()
    for t in LST.read_text(errors='replace').splitlines():
        if 'Filename:' in t:
            on = [f for f in files if f in t]
            seen.update(on)
            continue
        m = WORD.match(t)
        if not on or not m or m.group(5).lstrip().startswith('#'):
            continue
        p = physical(int(m.group(1), 8) if m.group(1) else 0, int(m.group(2), 8))
        words[p] = (int(m.group(3), 8), m.group(5).split('\t#')[0].split())
        order.append(p)
    out = set()
    for p in order:
        word, tok = words[p]
        if 'EXIT' not in tok:
            continue
        if word == 0o77776:
            out.add(p + 1)
            continue
        first = tok[tok.index('EXIT') - 1] if tok.index('EXIT') else None
        if first is None:
            continue
        out.add(p + (1 if first in UNARY else 2))
    if set(files) - seen:
        sys.exit(f'FAIL: not in {LST.name}: {" ".join(sorted(set(files) - seen))}')
    return sorted(out)


def table_keys(stem):
    """The keys of src/<stem>.c's basic table <stem>_routines, flight build."""
    with tempfile.TemporaryDirectory() as d:
        main = pathlib.Path(d) / 'keys.c'
        main.write_text('#include <stdio.h>\n#include "agc_transfer.h"\n'
                        f'extern const agc_routine_table {stem}_routines;\n'
                        f'int main(void){{unsigned i;for(i=0;i<{stem}_routines.count;++i)'
                        f'printf("%u\\n",(unsigned){stem}_routines.entries[i].physical);return 0;}}\n')
        exe = pathlib.Path(d) / 'keys'
        subprocess.run(['cc', '-std=c99', '-O0', '-Iinclude', '-Itests', '-DAGC_FLIGHT_INTERPRETER', main, R / 'src' / f'{stem}.c',
                        '-Wl,--unresolved-symbols=ignore-all', '-o', exe], cwd=R, check=True)
        keys = [int(k) for k in subprocess.run([exe], capture_output=True, text=True, check=True).stdout.split()]
    if any(a >= b for a, b in zip(keys, keys[1:])):
        sys.exit(f'FAIL: {stem}_routines keys do not ascend')
    return set(keys)


def main():
    p = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    p.add_argument('--list', action='store_true')
    a = p.parse_args()
    bad = []
    for stem, files in FILES.items():
        want = exit_returns(files)
        if a.list:
            print(f'{stem}: {" ".join(name(w) for w in want) or "none"}')
            continue
        missing = [w for w in want if w not in table_keys(stem)]
        if stem in OPT_IN:
            if missing:
                bad.append(f'{stem}: EXIT returns not keyed: {" ".join(map(name, missing))}')
            else:
                print(f'PASS: {stem} keys all {len(want)} EXIT returns')
        elif missing:
            print(f'NOTE: {stem} (not opted in) leaves {len(missing)} EXIT returns unkeyed: '
                  f'{" ".join(map(name, missing))}')
    if bad:
        sys.exit('FAIL: ' + '; '.join(bad))


if __name__ == '__main__':
    main()
