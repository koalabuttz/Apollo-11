#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Module loads on the C64, by module, from VICE's REU trace
(flight_scenario_vice.py --load-trace).

A VICE monitor checkpoint logs, without stopping, every REU fetch (REU to
C64) of at least MIN_BYTES and prints the controller's registers. Each
fetch whose REU start lies in a store module (the manifest's page and
bytes) counts against that module: a fetch at the module's first byte is
one load, and every fetch's length adds to its bytes. Rope words, header
peeks and the undo stash are shorter or not fetches, so they stay out.
The checkpoint only reads the controller, so the run's C64 cycles are
the same with and without it.

  flight_load_profile.py MONITOR_LOG MANIFEST   (prints the ranking)
"""
import json
import re
import sys

# Checkpoint 5: after flight_scenario_vice.py's REU bound trace (2, 3, 4).
CHECKPOINT = 5
MIN_BYTES = 64
_LEN = '((@io:$df08 * $100) + @io:$df07)'


def monitor(min_bytes=MIN_BYTES):
    """The VICE monitor commands: execute bit set, fetch (with or without
    autoload or the $FF00 trigger), at least min_bytes (a length of 0 is
    64 KB and also counts). VICE wants each && and || between two
    parenthesised terms, and reads bare numbers as hex."""
    cmd = '@io:$df01'
    fetch = f'((({cmd} == $91) || ({cmd} == $81)) || (({cmd} == $b1) || ({cmd} == $a1)))'
    size = f'(({_LEN} >= ${min_bytes:x}) || ({_LEN} == $0))'
    return f'trace store df01 if ({fetch} && {size})\ncommand {CHECKPOINT} "io $df00"'


def fetches(log):
    """[(c64, reu_start, length)] of each fetch checkpoint 5 logged."""
    out = []
    for m in re.finditer(rf'^#{CHECKPOINT} \(Trace store df01\).*\n\.C:[0-9a-f]{{4}}.*\n'
                         r'(?:Executing:.*\n)?RAM Expansion Module:\n>C:df00((?:\s+[0-9a-f]{2}){11})', log, re.M):
        b = [int(x, 16) for x in m.group(1).split()]
        out.append((b[2] | b[3] << 8, b[4] | b[5] << 8 | b[6] << 16, (b[7] | b[8] << 8) or 0x10000))
    return out


def name(module):
    return module['path'].rsplit('/', 1)[-1].removesuffix('.prg')


def profile(log, manifest):
    """dict(fetches, unplaced, modules=[dict(module, id, slot, bytes, loads,
    fetched)] sorted by bytes fetched, most first, pairs=[[from, to, loads]]:
    the module loaded into a slot after the one loaded there before, most
    first)."""
    spans = sorted((x['page'] * 256, x['page'] * 256 + x['bytes'], x) for x in manifest['modules'])
    rows, unplaced, n, last, pairs = {}, 0, 0, {}, {}
    for _, start, length in fetches(log):
        n += 1
        hit = next((x for lo, hi, x in spans if lo <= start < hi), None)
        if hit is None:
            unplaced += 1
            continue
        r = rows.setdefault(hit['id'], dict(module=name(hit), id=f"{hit['id']:04x}", slot=hit['slot'],
                                            bytes=hit['bytes'], loads=0, fetched=0))
        r['fetched'] += length
        if start == hit['page'] * 256:
            r['loads'] += 1
            before = last.get(hit['slot'])
            if before is not None:
                pairs[before, r['module']] = pairs.get((before, r['module']), 0) + 1
            last[hit['slot']] = r['module']
    return dict(fetches=n, unplaced=unplaced, modules=sorted(rows.values(), key=lambda r: -r['fetched']),
                pairs=[[a, b, k] for (a, b), k in sorted(pairs.items(), key=lambda x: -x[1])])


def show(p, top=20):
    total = sum(r['fetched'] for r in p['modules'])
    print(f"module loads: {sum(r['loads'] for r in p['modules'])} loads, {total:,} B fetched "
          f"in {p['fetches']} fetches ({p['unplaced']} outside the store's modules)")
    for r in p['modules'][:top]:
        print(f"  {r['module']} ({r['id']}, slot {r['slot']}, {r['bytes']:,} B): {r['loads']} loads, "
              f"{r['fetched']:,} B ({100 * r['fetched'] / total if total else 0:.1f}%)")
    print('  loads after another module in the slot: ' + ', '.join(f'{a} > {b} {k}' for a, b, k in p['pairs'][:12]))


if __name__ == '__main__':
    show(profile(open(sys.argv[1]).read(), json.load(open(sys.argv[2]))), 40)
