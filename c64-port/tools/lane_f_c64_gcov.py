#!/usr/bin/env python3
"""Whole-file C64 branch check for a Lane F module (from the second
certificate reviewer's independent check, #167).

Builds the module's linked src/ files with gcc --coverage (not Lane F's clang
trace-pc-guard), runs (a) every host case in the full build and (b) for each
C64 PRG/group build only that build's rows from tests/<stem>_c64_vectors.h,
runs gcov -b per object, and lists every branch outcome (file, line, branch
index) the host cases take that no C64 row takes.

  python3 tools/lane_f_c64_gcov.py <milestone> [outdir]     (from c64-port/)

Exits non-zero when an outcome or line is missing; writes
build/<milestone>-c64-gcov.json.
"""
import json, pathlib, re, subprocess, sys, shutil
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import lane_f_c64_rows as L

m = sys.argv[1]
out = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else L.B / f'{m}-gcov').resolve()
stem, mac, sym, slots, budget, srcvar, run = L.MODULES[m]
srcs = L.mk_var(m, srcvar)
groups = [None]
macro = None
if m in L.GROUPS:
    macro = L.GROUPS[m][0]
elif m in L.SEGMENTS:
    macro = L.SEGMENTS[m][0]
if macro:
    txt = (L.R / f'tests/{stem}_c64_vectors.h').read_text()
    groups = sorted({int(g) for g in re.findall(rf'#if {macro}==(\d+)', txt)})

DRV = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "%(stem)s_cases.h"
#include "lm_flight_io.h"
#ifdef ROWS
#include "%(stem)s_c64_vectors.h"
#endif
int main(void){apollo_overlay_state s;lm_flight_io io;uint16_t steps;unsigned i,n;(void)io;(void)steps;
#ifdef ROWS
 for(i=0;i<%(mac)s_C64_COUNT;++i){n=%(sym)s_c64_case[i];
#else
 for(i=0;i<%(mac)s_CASES;++i){n=i;
#endif
  (void)RUN(&s,&io,(uint16_t)n,&steps);}
 return 0;}
'''


def build_run(tag, defs):
    d = out / tag
    if d.exists():
        shutil.rmtree(d)
    d.mkdir(parents=True)
    (d / 'drv.c').write_text(DRV % dict(stem=stem, mac=mac, sym=sym))
    runx = run.format(s=sym)
    base = ['gcc', '-std=gnu99', '-O0', '-w', '-I', str(L.R / 'include'), '-I', str(L.R / 'tests'), '-I', str(L.R / 'src'),
            '-DP66_NATIVE_DISPATCH_ONLY', f'-DRUN(s,io,n,steps)={runx}', *defs]
    objs = []
    for src in srcs:
        o = d / (pathlib.Path(src).stem + '.o')
        cov = ['--coverage'] if src.startswith('src/') else []
        subprocess.run(base + cov + ['-c', str(L.R / src), '-o', str(o)], cwd=d, check=True)
        objs.append(str(o))
    subprocess.run(base + ['-c', str(d / 'drv.c'), '-o', str(d / 'drv.o')], cwd=d, check=True)
    link = ['gcc', '--coverage', *objs, str(d / 'drv.o'), '-lm', '-o', str(d / 'run')]
    r = subprocess.run(link, cwd=d, capture_output=True, text=True)
    dead = sorted(set(re.findall(r"undefined reference to `(\w+)'", r.stderr)))
    if r.returncode:
        if not dead:
            sys.exit(r.stderr)
        (d / 'stubs.c').write_text('#include <stdlib.h>\n' + ''.join(f'void {f}(void){{abort();}}\n' for f in dead))
        subprocess.run(['gcc', '-w', '-c', str(d / 'stubs.c'), '-o', str(d / 'stubs.o')], check=True)
        subprocess.run(link + [str(d / 'stubs.o')], cwd=d, check=True)
    subprocess.run([str(d / 'run')], cwd=L.R, check=True, timeout=7200)
    taken, lines, nbr = set(), set(), {}
    for src in srcs:
        if not src.startswith('src/'):
            continue
        st = pathlib.Path(src).stem
        g = d / 'g' / st
        g.mkdir(parents=True)
        subprocess.run(['gcov', '-b', '-c', '-o', str(d / (st + '.o')), str(L.R / src)], cwd=g, capture_output=True)
        for f in g.glob('*.gcov'):
            name = f.name[:-5]
            line, bi = None, 0
            for ln in f.read_text(errors='replace').splitlines():
                mm = re.match(r'\s*[^:]+:\s*(\d+):', ln)
                if mm:
                    line, bi = int(mm.group(1)), 0
                    cnt = ln.split(':')[0].strip()
                    if cnt not in ('-', '#####', '=====') and cnt.rstrip('*').isdigit() and int(cnt.rstrip('*')) > 0:
                        lines.add((name, line))
                    continue
                mb = re.match(r'branch\s+(\d+) (taken (\d+)|never executed)', ln)
                if mb and line:
                    nbr[(name, line)] = max(nbr.get((name, line), 0), int(mb.group(1)) + 1)
                    if mb.group(3) and int(mb.group(3)) > 0:
                        taken.add((name, line, int(mb.group(1))))
    return taken, lines, nbr


# Branch outcomes are keyed (file, line, index); a line whose branch count
# differs between the full and a group build (a switch with #if'd arms) is
# compared by line execution instead: each case arm is its own line.
host, hl, hn = build_run('host', [])
c64, cl = set(), set()
for g in groups:
    defs = ['-DROWS'] + ([f'-D{macro}={g}'] if g is not None else [])
    t, l, n = build_run(f'c64-g{g}', defs)
    c64 |= {x for x in t if n.get(x[:2]) == hn.get(x[:2])}
    c64 |= {x for x in host if x[:2] in n and n[x[:2]] != hn.get(x[:2]) and x[:2] in hn}  # judged by lines
    cl |= l
miss = sorted(host - c64)
lmiss = sorted(hl - cl)
print(f'{m}: host branch outcomes {len(host)}, C64 rows take {len(host & c64)}, missing {len(miss)}; host lines {len(hl)}, missing lines {len(lmiss)}')
for x in miss:
    print('  MISSING', *x)
for x in lmiss:
    print('  MISSING-LINE', *x)
(L.B / f'{m}-c64-gcov.json').write_text(json.dumps(
    {'milestone': m, 'method': 'gcc --coverage, gcov -b per object over every src/ file linked: every host case in the '
                               'whole-file build against each C64 build\'s rows (tests/' + stem + '_c64_vectors.h)',
     'host_branch_outcomes': len(host), 'c64_branch_outcomes': len(host & c64), 'missing': [list(x) for x in miss],
     'host_lines': len(hl), 'missing_lines': [list(x) for x in lmiss]}, indent=1) + '\n')
sys.exit(1 if miss or lmiss else 0)
