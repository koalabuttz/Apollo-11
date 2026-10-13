#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pick a lane's C64 rows by the host's branch outcomes (gcov).

A host runner built with --coverage (build()) runs one case at a time;
trace() reads every branch arc the case takes, in every linked source,
with gcov -b -j, as 'file:line branch N'. cover() picks the fewest cases
whose arcs together equal what the whole host corpus takes; check() lists
the arcs a given row list misses. Cases whose .gcda files are byte-equal
share one gcov run.

  c64_row_cover.py pick  --objdir D --cases N [--budget K] [--trace T] -- RUNNER {case}...
  c64_row_cover.py check --objdir D --cases N --rows 0,17,40 [--trace T] -- RUNNER {case}...

The runner gets {case} replaced by the case number; per-case dumps made
elsewhere (DIR/<case>/*.gcda) work with --dumps DIR in place of a runner.
--trace saves the per-case arcs (JSON) and reuses them when present.
Edge-value families for case generators are in tools/edge_cases.py.
"""
import argparse
import concurrent.futures
import glob
import hashlib
import json
import os
import pathlib
import queue
import re
import shutil
import subprocess
import sys
import tempfile

R = pathlib.Path(__file__).resolve().parents[1]


def build(objdir, sources, flags=(), cc='cc', name='runner'):
    """Compile sources (relative to the port root) at -O0 --coverage into objdir; returns the runner."""
    objdir = pathlib.Path(objdir).resolve()
    shutil.rmtree(objdir, ignore_errors=True)
    objdir.mkdir(parents=True)
    exe = objdir / name
    subprocess.run([cc, '-O0', '--coverage', *flags, *sources, '-o', str(exe)], cwd=R, check=True)
    return exe


def arcs(d):
    """Branch arcs taken by the .gcda files in d (with their .gcno beside them)."""
    data = sorted(glob.glob(os.path.join(d, '*.gcda')))
    if not data:
        return frozenset()
    out = subprocess.run(['gcov', '-b', '-j', '-t', '-o', d, *data], cwd=d, check=True,
                         capture_output=True, text=True).stdout
    taken = []
    for doc in out.splitlines():
        doc = json.loads(doc)
        obj = os.path.splitext(os.path.basename(doc.get('data_file', '')))[0]
        for f in doc['files']:
            name = f['file']
            if os.path.isabs(name):
                name = os.path.relpath(name, R)
            for ln in f['lines']:
                for k, b in enumerate(ln['branches']):
                    if b['count']:
                        taken.append((obj, name, ln['line_number'], k))
    return frozenset(arc_keys(taken))


def arc_keys(taken):
    """'file:line branch N' keys of (object, source, line, branch) arcs.

    A .c source built into more than one object (the Comanche wrappers
    #include the Luminary file: cm_interp_orders.c includes
    lm_interp_orders.c) keys each object's arcs as 'file@object:line branch
    N', so a two-mission runner does not merge the missions' arcs. A source
    in one object, and every header, keeps the plain key."""
    objs = {}
    for obj, name, _, _ in taken:
        objs.setdefault(name, set()).add(obj)
    split = {n for n, o in objs.items() if n.endswith('.c') and len(o) > 1}
    return {f"{name}@{obj}:{line} branch {k}" if name in split else f"{name}:{line} branch {k}"
            for obj, name, line, k in taken}


def _worker_dirs(objdir, jobs, tmp):
    q = queue.Queue()
    for k in range(jobs):
        w = os.path.join(tmp, str(k))
        os.mkdir(w)
        for g in glob.glob(os.path.join(objdir, '*.gcno')):
            os.symlink(g, os.path.join(w, os.path.basename(g)))
        q.put(w)
    return q


def _digest(w):
    h = hashlib.sha256()
    for g in sorted(glob.glob(os.path.join(w, '*.gcda'))):
        h.update(open(g, 'rb').read())
    return h.hexdigest()


def trace(objdir, cases, argv=None, dumps=None, jobs=None):
    """{case: frozenset(arcs)}: argv(case) is the command running that one case
    (from the port root), or dumps/<case>/ holds its .gcda files."""
    objdir = str(pathlib.Path(objdir).resolve())
    jobs = jobs or os.cpu_count() or 1
    env = dict(os.environ, GCOV_PREFIX_STRIP=str(objdir.count('/')))
    seen = {}
    with tempfile.TemporaryDirectory() as tmp:
        free = _worker_dirs(objdir, jobs, tmp)

        def one(n):
            w = free.get()
            try:
                for g in glob.glob(os.path.join(w, '*.gcda')):
                    os.remove(g)
                if dumps:
                    for g in glob.glob(os.path.join(dumps, str(n), '*.gcda')):
                        shutil.copy(g, w)
                else:
                    subprocess.run(argv(n), cwd=R, check=True, stdout=subprocess.DEVNULL,
                                   env=dict(env, GCOV_PREFIX=w))
                key = _digest(w)
                if key not in seen:
                    seen[key] = arcs(w)
                return n, seen[key]
            finally:
                free.put(w)
        with concurrent.futures.ThreadPoolExecutor(jobs) as ex:
            return dict(ex.map(one, cases))


def host_trace(objdir, sources, flags, cases, args=('{case}',), jobs=None):
    """build() then trace(): the runner gets args with {case} replaced."""
    exe = str(build(objdir, sources, flags))
    return trace(objdir, cases, lambda n: [exe, *(x.replace('{case}', str(n)) for x in args)], jobs=jobs)


def cover(trace, cost=None, budget=None, seed=()):
    """Greedy set cover: seed plus the fewest cases whose arcs equal the host's
    (ties to the lowest cost(case), then the lowest case). Exits if it needs
    more than budget rows."""
    cost = cost or (lambda n: 0)
    host = frozenset().union(*trace.values())
    chosen = list(seed)
    have = set().union(*(trace[n] for n in chosen))
    while have != host:
        best = min(trace, key=lambda n: (-len(trace[n] - have), cost(n), n))
        chosen.append(best)
        have |= trace[best]
    if budget is not None and len(chosen) > budget:
        sys.exit(f'c64_row_cover: {len(chosen)} rows cover the host arcs, over the budget of {budget}')
    return sorted(chosen)


def missing(trace, rows):
    """Host arcs the rows do not take, sorted by file, line and branch."""
    host = frozenset().union(*trace.values())
    have = frozenset().union(*(trace[n] for n in rows))
    key = lambda a: [int(t) if t.isdigit() else t for t in re.split(r'(\d+)', a)]
    return sorted(host - have, key=key)


def check(trace, rows, what='C64 rows'):
    """Exit listing the missed arcs unless the rows take every host arc."""
    miss = missing(trace, rows)
    if miss:
        sys.exit(f'{what} miss {len(miss)} host branch arcs:\n  ' + '\n  '.join(miss))


def summary(trace, rows):
    """Proof fields for a milestone's proof JSON."""
    host = frozenset().union(*trace.values())
    return {'c64_row_selection': 'tools/c64_row_cover.py: the C64 rows take every branch arc (gcov -b, all '
                                 'linked sources) the host cases take',
            'host_cases_traced': len(trace), 'host_branch_arcs': len(host), 'c64_rows': len(rows),
            'c64_branch_arcs': len(host) - len(missing(trace, rows))}


def save(path, trace):
    pathlib.Path(path).write_text(json.dumps({str(n): sorted(a) for n, a in sorted(trace.items())}) + '\n')


def load(path):
    return {int(n): frozenset(a) for n, a in json.loads(pathlib.Path(path).read_text()).items()}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('mode', choices=('pick', 'check'))
    ap.add_argument('--objdir', required=True, help='where the runner\'s .gcno files are')
    ap.add_argument('--cases', type=int, required=True)
    ap.add_argument('--budget', type=int)
    ap.add_argument('--rows', help='comma-separated case numbers (check)')
    ap.add_argument('--dumps', help='DIR/<case>/*.gcda instead of a runner')
    ap.add_argument('--trace', help='per-case arcs JSON, read if present, else written')
    ap.add_argument('--jobs', type=int)
    ap.add_argument('cmd', nargs='*')
    a = ap.parse_args(argv)
    if a.trace and os.path.exists(a.trace):
        t = load(a.trace)
    else:
        t = trace(a.objdir, range(a.cases), lambda n: [s.replace('{case}', str(n)) for s in a.cmd],
                  a.dumps, a.jobs)
        if a.trace:
            save(a.trace, t)
    host = frozenset().union(*t.values())
    if a.mode == 'pick':
        rows = cover(t, budget=a.budget)
        print(','.join(map(str, rows)))
        print(f'{len(rows)} of {len(t)} cases take all {len(host)} host branch arcs', file=sys.stderr)
    else:
        rows = [int(x) for x in a.rows.split(',') if x]
        check(t, rows)
        print(f'{len(rows)} rows take all {len(host)} host branch arcs')


if __name__ == '__main__':
    main()
