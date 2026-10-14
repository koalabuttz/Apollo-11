#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Executed-instruction closure for a whole-file interp_compile milestone.

  python3 tools/interp_executed_closure.py <milestone> [--unreached ADDR=REASON ...]

Reads the milestone's source map (build/<m>-source-map.json, from
tools/interp_routine_differential.py) and the words the original program
executed in that differential's yaAGC oracle run (build/<m>-executed.txt,
written by tools/oracle_trace when the differential runs with it first on
PATH). Every assembled word of the file that is not data must have been
executed, or be named with a reason on the command line. Interpretive code
(a word the source map gives to an interpretive routine, not basic_run) is
not executed but read by the interpreter: with build/<m>-read.txt (the
fixed words the oracle read, AGC_READ_WORDS) such a word counts when it was
read. Writes build/<m>-closure.json.
"""
import argparse
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
DATA = {'DEC', '2DEC', 'OCT', '2OCT', 'CADR', '2CADR', 'GENADR', 'ADRES', 'FCADR', 'BBCON', 'DEC*', '2DEC*',
        'REMADR', 'ECADR', 'BNKSUM', 'VN', '2FCADR', 'OCTAL', 'MM', 'NV'}


def physical(address):
    bank, offset = (int(x, 8) for x in address.split(','))
    return bank * 1024 + offset - 0o2000


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('milestone')
    ap.add_argument('--unreached', nargs='*', default=[], metavar='ADDR=REASON')
    a = ap.parse_args()
    m = a.milestone
    smap = json.loads((R / f'build/{m}-source-map.json').read_text())
    executed = {int(x) for x in (R / f'build/{m}-executed.txt').read_text().split()}
    read_path = R / f'build/{m}-read.txt'
    read = {int(x) for x in read_path.read_text().split()} if read_path.exists() else set()
    unreached = dict(u.split('=', 1) for u in a.unreached)
    out = {'milestone': m, 'tool': 'tools/interp_executed_closure.py',
           'source_map_sha256': hashlib.sha256((R / f'build/{m}-source-map.json').read_bytes()).hexdigest(),
           'oracle': 'yaAGC (tools/oracle_trace) in the milestone\'s differential', 'files': {}}
    bad = []
    for path, rec in smap.items():
        rows, counts = [], {'words': 0, 'data': 0, 'instructions': 0, 'executed': 0, 'interpreted': 0, 'unreached': 0, 'core': 0}
        data = False
        for w in rec['map']:
            op = (w['source'].split() + ['', ''])[1] if not w['source'].startswith(' ') else (w['source'].split() + [''])[0]
            # (+1): the second word of a two-word constant (2CADR, 2DEC) is data too.
            data = op.upper() in DATA or 'constant' in w or (w['source'].strip() == '(+1)' and data)
            p = physical(w['address'])
            ran = p in executed
            interp = w.get('function') not in (None, 'basic_run')
            interpreted = interp and p in read
            counts['words'] += 1
            counts['data' if data else 'instructions'] += 1
            core = w.get('routine') == 'interpreter core'
            if not data and core:
                # (INTERPRETER's words the compact core runs, not this
                # milestone's routines: the core's own evidence covers them.)
                counts['core'] += 1
            elif not data:
                if ran:
                    counts['executed'] += 1
                elif interpreted:
                    counts['interpreted'] += 1
                elif w['address'] in unreached:
                    counts['unreached'] += 1
                else:
                    bad.append(f'{path} {w["address"]} {w["source"].strip()}')
            rows.append({'address': w['address'], 'source': w['source'].strip(), 'data': data, 'executed': ran,
                         **({'interpreted': True} if interpreted else {}), **({'core': True} if core else {}),
                         **({'unreached': unreached[w['address']]} if w['address'] in unreached else {})})
        out['files'][path] = {**counts, 'map': rows}
    if bad:
        raise SystemExit('FAIL: not executed by the oracle and not named unreached:\n  ' + '\n  '.join(bad))
    (R / f'build/{m}-closure.json').write_text(json.dumps(out, indent=2) + '\n')
    for path, f in out['files'].items():
        print(f'PASS: {path}: {f["words"]} words, {f["instructions"]} instructions, {f["executed"]} executed by the '
              f'oracle, {f["interpreted"]} interpreted, {f["unreached"]} named unreached, {f["data"]} data'
              + (f', {f["core"]} run by the interpreter core' if f['core'] else ''))


if __name__ == '__main__':
    main()
