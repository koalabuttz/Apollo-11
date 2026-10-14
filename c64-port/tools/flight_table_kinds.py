#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check the interpretive flag of every table in the host index
(build/flight-store/<m>-host.inc, tools/flight_store_build.py
interpretive_table) against the table's own keys: a table is interpretive
when its keys are interpretive words of the listing (an order or operand
line), basic when they are basic instructions or erasable. The DANZIG
markers are the exception (an interpretive lookup keyed at DANZIG's basic
word). Any table whose flag disagrees with the majority of its keys fails;
so does a table with both kinds of keys in numbers above a stray word.

  flight_table_kinds.py [--host-cc CC]   (after flight_store_build.py --host-only)
"""
import argparse, collections, os, re, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import interp_compile  # noqa: E402
import interp_listing  # noqa: E402

STORE = ROOT / 'build/flight-store'
DUMP = r'''#include <stdio.h>
#include "agc_transfer.h"
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed) extern const agc_routine_table t;
#include TABLES
#undef FLIGHT_TABLE
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed) {unsigned j;for(j=0;j<t.count;++j)printf("%s %d %u\n",#t,i,(unsigned)t.entries[j].physical);}
int main(void){
#include TABLES
return 0;}
'''
# A table may hold a stray word of the other kind (a resume after a TC
# INTPRET, an EXIT's next word) as long as its own kind is the majority.
STRAY = 3


def keys(cc, m, d):
    c = Path(d) / 'dump.c'
    c.write_text(DUMP)
    exe = Path(d) / f'dump-{m}'
    subprocess.run([cc, '-std=c99', '-Iinclude', '-Itests', f'-DRK_{m.upper()}=1', f'-DTABLES="{STORE / f"{m}-host.inc"}"',
                    str(c), *sorted(str(p) for p in (STORE / 'host').glob('*.o')), '-Wl,--gc-sections', '-lm', '-o', str(exe)],
                   cwd=ROOT, check=True)
    out = subprocess.run([str(exe)], capture_output=True, text=True, check=True).stdout
    return [(t, int(i), int(k)) for t, i, k in (line.split() for line in out.splitlines())]


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    a = p.parse_args()
    bad = 0
    for m in ('lm', 'cm'):
        listing = interp_listing.load(m)
        kind = {}
        for ln in listing.lines:
            if ln.phys is None or not ln.words:
                continue
            for i in range(len(ln.words)):
                kind[ln.phys + i] = 'i' if not ln.op or interp_compile.is_interp_op(ln.op) else 'b'
        with tempfile.TemporaryDirectory() as d:
            rows = keys(a.host_cc, m, d)
        flag, counts = {}, collections.defaultdict(collections.Counter)
        for t, i, k in rows:
            flag[t] = i
            counts[t][kind.get(k, 'b')] += 1
        for t, c in counts.items():
            if re.search(r'_marker$', t):
                continue
            want = int(c['i'] > c['b'])
            if flag[t] != want or min(c['i'], c['b']) > STRAY:
                print(f'{m} {t}: flagged {"interpretive" if flag[t] else "basic"}, keys {dict(c)}')
                bad = 1
        print(f'{m}: {len(counts)} tables checked')
    return bad


if __name__ == '__main__':
    sys.exit(main())
