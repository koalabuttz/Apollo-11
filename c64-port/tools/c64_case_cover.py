#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Choose the C64 rows of a case-table milestone so that they run every
branch outcome of the native routine that the full host case set runs.

select() builds a host-only driver with clang -fsanitize-coverage=edge,
trace-pc-guard on the native source (tools/edge_cover.c records one guard
per control-flow edge), runs the strided rows first and then every other
case, and returns the strided rows plus the fewest further cases (greedy)
that add the edges the strided rows miss. It also returns the edge count,
so the freeze can record that the chosen rows run every edge the host does.
"""
import os
import pathlib
import subprocess

R = pathlib.Path(__file__).resolve().parents[1]
HOST_CLANG = os.environ.get('HOST_CLANG', '/usr/bin/clang' if pathlib.Path('/usr/bin/clang').exists() else 'clang')
SUPPORT = ['src/agc_image_ops.c', 'src/agc_hardware_divide.c', 'src/agc_transfer.c', 'src/agc_word.c',
           'src/agc_multiword.c']


def select(name, prefix, native, cases, header, total, stride):
    """name: milestone id; prefix: case API prefix (lti_digest); native: the
    routine source, or a list of them; cases: the case generator; header: its
    header; total: the case count. Returns (rows, host_edges, row_edges)."""
    out = R / 'build/cover'
    out.mkdir(parents=True, exist_ok=True)
    objs = []
    for src in [native] if isinstance(native, str) else native:
        obj = out / f'{name}-{pathlib.Path(src).stem}.o'
        subprocess.run([HOST_CLANG, '-std=c99', '-O0', '-Iinclude', '-Itests', '-fsanitize-coverage=edge,trace-pc-guard',
                        '-c', src, '-o', str(obj)], cwd=R, check=True)
        objs.append(str(obj))
    drv = out / f'{name}-cover.c'
    drv.write_text(f'''#include <stdio.h>
#include "edge_cover.h"
#include "{header}"
int main(void){{static apollo_overlay_state s;unsigned n;char tag[16];
 for(n=0;n<{total};n+={stride}u){{(void){prefix}_digest(&s,(uint16_t)n);sprintf(tag,"s %u",n);edge_cover_mark(stdout,tag);}}
 for(n=0;n<{total};++n){{if(n%{stride}u==0u)continue;(void){prefix}_digest(&s,(uint16_t)n);sprintf(tag,"x %u",n);edge_cover_mark(stdout,tag);}}
 return 0;}}
''')
    exe = out / f'{name}-cover'
    subprocess.run([HOST_CLANG, '-std=c99', '-O1', '-Iinclude', '-Itests', '-Itools', str(drv), *objs,
                    'tools/edge_cover.c', cases, *SUPPORT, '-o', str(exe)], cwd=R, check=True)
    lines = subprocess.run([str(exe)], cwd=R, check=True, capture_output=True, text=True).stdout.splitlines()
    sets = []
    for line in lines:
        tag, edges = line.split(' | ') if ' | ' in line else (line.rstrip(' |'), '')
        kind, n = tag.split()
        sets.append((kind, int(n), set(map(int, edges.split()))))
    host = set().union(*(e for _, _, e in sets))
    covered = set().union(*(e for k, _, e in sets if k == 's'))
    rows = set(range(0, total, stride))
    rest = [(n, e) for k, n, e in sets if k == 'x']
    while covered != host:
        n, e = max(rest, key=lambda r: len(r[1] - covered))
        rows.add(n)
        covered |= e
    return sorted(rows), len(host), len(covered)
