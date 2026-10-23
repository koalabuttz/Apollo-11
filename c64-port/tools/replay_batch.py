#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run a list of C64 replay windows (tools/flight_scenario_vice.py) as a
batch that resumes: every window writes its own log, a window whose log is
finished on the same commit with the same command is skipped, and the batch
stops starting windows once its time budget is spent (each run still starts
at least one window). Rerun the same command to go on where it stopped,
after a time limit, a lost container or a crash. Run one batch per tree:
OUT/.lock stops a second batch on the same OUT.

The window list has one window per line, '#' starts a comment:

  SCENARIO SNAP_CS CS [more flight_scenario_vice.py options]

  lm-p63-descent 17500 300
  lm-rendezvous 8000 1000 --replay-outputs

Options after '--' on the command line go to every window; --replay is
always added. A window's log is OUT/SCENARIO-SNAP+CS.log (written to a
.part file and renamed when the run ends), starts with '$ command' and
then '# tree ID' (the commit, plus a hash of uncommitted tracked changes),
and ends with 'EXIT rc wall Ns'.

A window's status comes from its exit code and every line of its log:
  match         rc 0 and no line with FAIL
  match-masked  a match that ran with a mask or skip ("skipped (temporary",
                "masked_", "(with skips)"): not a full match
  FAIL          a line with FAIL anywhere (window, seam, chain, REU bounds,
                outputs), whatever the order; the first one is quoted
  crash         rc not 0 and no FAIL line (a traceback, VICE died); rerun
                on resume
  cut off       a .part log with no EXIT line (rerun)
  not run       no log yet

OUT/_summary.txt has a header '# N of M windows finished [INCOMPLETE]:
counts[; MIXED: ...]' and one row per listed window:
'tag|status|rc|commit|store sha256|detail'. A finished log is reused only
when its command and tree id match this run, so a resume on a changed
tree reruns every window;
--reuse-any keeps every finished log.

  replay_batch.py --windows FILE --out DIR [--budget-min 75] [-- OPTIONS]

Exit 0 when every window finished (match or not), 3 when the budget ran out
first (rerun to resume), 1 for a bad window list or a held lock."""
import argparse
import collections
import hashlib
import os
import pathlib
import re
import shlex
import socket
import subprocess
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
RUNNER = [sys.executable, 'tools/flight_scenario_vice.py']
STAMP = re.compile(r': commit (\S+)( with local changes)?; store \S+ sha256 ([0-9a-f]+)')
MASKS = ('skipped (temporary', 'masked_', '(with skips)')


def windows(path):
    """(tag, args) for each line of the window list."""
    out, seen = [], set()
    for n, line in enumerate(path.read_text().splitlines(), 1):
        f = shlex.split(line, comments=True)
        if not f:
            continue
        if len(f) < 3 or not f[1].isdigit() or not f[2].isdigit():
            sys.exit(f'{path}:{n}: want SCENARIO SNAP_CS CS [options], got {line!r}')
        tag = f'{f[0]}-{f[1]}+{f[2]}'
        if tag in seen:
            sys.exit(f'{path}:{n}: {tag} is listed twice')
        seen.add(tag)
        out.append((tag, ['--scenario', f[0], '--snap-cs', f[1], '--cs', f[2], *f[3:]]))
    return out


def tree():
    """This tree's id: the commit, plus a hash of its uncommitted tracked
    changes when there are any."""
    def git(*a):
        return subprocess.run(['git', *a], cwd=ROOT, capture_output=True).stdout
    c = git('rev-parse', 'HEAD').decode().strip() or 'unknown'
    diff = git('diff', 'HEAD', '--binary')
    return c + (f'+{hashlib.sha256(diff).hexdigest()[:16]}' if diff.strip() else '')


def read(log):
    """Parse a log: dict with cmd, rc (None if cut off), commit, store,
    status, detail."""
    lines = log.read_text(errors='replace').splitlines()
    r = {'cmd': None, 'tree': None, 'rc': None, 'commit': '', 'store': '', 'detail': ''}
    if lines and lines[0].startswith('$ '):
        r['cmd'] = lines.pop(0)[2:]
    if lines and lines[0].startswith('# tree '):
        r['tree'] = lines.pop(0)[7:]
    body = lines
    if body and body[-1].startswith('EXIT '):
        try:
            r['rc'] = int(body[-1].split()[1])
        except (IndexError, ValueError):
            pass
        body = body[:-1]
    fail = masked = None
    stores = set()
    for line in body:
        m = STAMP.search(line)
        if m:
            r['commit'] = m.group(1) + (' with local changes' if m.group(2) else '')
            stores.add(m.group(3))
        if fail is None and 'FAIL' in line:
            fail = line.strip()
        if masked is None and any(k in line for k in MASKS):
            masked = line.strip()
    r['store'] = ','.join(sorted(stores))
    if r['rc'] is None:
        r['status'], r['detail'] = 'cut off', (body[-1].strip() if body else '')
    elif fail is not None:
        r['status'], r['detail'] = 'FAIL', fail
    elif r['rc'] != 0:
        r['status'], r['detail'] = 'crash', (body[-1].strip() if body else 'no output')
    elif masked is not None:
        r['status'], r['detail'] = 'match-masked', masked
    else:
        r['status'] = 'match'
        r['detail'] = next((l.strip() for l in reversed(body) if "in the host's state" in l), '')
    return r


def done(r):
    """A finished window: it ran to the end and did not crash."""
    return r is not None and r['rc'] is not None and r['status'] != 'crash'


def logs(out, tag):
    log = out / f'{tag}.log'
    if log.exists():
        return read(log)
    part = out / f'{tag}.log.part'
    return read(part) if part.exists() else None


def summary(out, tags):
    rows, counts, commits, stores, fin = [], collections.Counter(), set(), set(), 0
    for t in tags:
        r = logs(out, t)
        if r is None:
            rows.append(f'{t}|not run||||')
            counts['not run'] += 1
            continue
        counts[r['status']] += 1
        fin += done(r)
        if r['commit']:
            commits.add(r['commit'])
        if r['store']:
            stores.update(r['store'].split(','))
        rc = '' if r['rc'] is None else str(r['rc'])
        rows.append(f"{t}|{r['status']}|{rc}|{r['commit']}|{r['store']}|{r['detail']}")
    order = ['match', 'match-masked', 'FAIL', 'crash', 'cut off', 'not run']
    head_line = f'# {fin} of {len(tags)} windows finished'
    if fin < len(tags):
        head_line += ' INCOMPLETE'
    head_line += ': ' + ', '.join(f'{counts[k]} {k}' for k in order if counts[k])
    if len(commits) > 1 or len(stores) > 1:
        head_line += f'; MIXED: {len(commits)} commits, {len(stores)} store images'
    tmp = out / '_summary.txt.part'
    tmp.write_text(head_line + '\n' + ''.join(r + '\n' for r in rows))
    tmp.replace(out / '_summary.txt')
    return fin, head_line


def lock(out):
    """Take OUT/.lock ('pid host'); a dead pid on this host is taken over."""
    path, me = out / '.lock', f'{os.getpid()} {socket.gethostname()}'
    for _ in range(2):
        try:
            fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        except FileExistsError:
            try:
                pid, host = path.read_text().split()
                pid = int(pid)
            except (OSError, ValueError):
                pid, host = 0, ''
            if host == socket.gethostname():
                try:
                    os.kill(pid, 0)
                except (ProcessLookupError, ValueError):
                    path.unlink(missing_ok=True)
                    continue
                except PermissionError:
                    pass
            sys.stderr.write(f'replay_batch: {path} is held by {pid} on {host or "?"}; '
                             'one batch per OUT (remove the lock if that run is gone)\n')
            return None
        os.write(fd, me.encode())
        os.close(fd)
        return path
    return None


def main():
    argv = sys.argv[1:]
    extra = argv[argv.index('--') + 1:] if '--' in argv else []
    argv = argv[:argv.index('--')] if '--' in argv else argv
    if '--replay' not in extra:
        extra = ['--replay', *extra]
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--windows', type=pathlib.Path, required=True)
    p.add_argument('--out', type=pathlib.Path, required=True)
    p.add_argument('--budget-min', type=float, default=75.0,
                   help='start no window after this many minutes (default 75, so the last window '
                        'still ends inside a 2 h job limit)')
    p.add_argument('--reuse-any', action='store_true',
                   help='keep every finished log, even from another commit or command')
    p.add_argument('--runner', default=None, help=argparse.SUPPRESS)  # tests
    p.add_argument('--head', default=None, help=argparse.SUPPRESS)    # tests
    a = p.parse_args(argv)
    runner = shlex.split(a.runner) if a.runner else RUNNER
    todo = windows(a.windows)
    a.out.mkdir(parents=True, exist_ok=True)
    held = lock(a.out)
    if held is None:
        return 1
    try:
        for stale in a.out.glob('*.log.part'):
            stale.unlink()
        cur = a.head if a.head is not None else tree()
        tags = [t for t, _ in todo]
        start, ran = time.monotonic(), 0
        for tag, args in todo:
            log = a.out / f'{tag}.log'
            cmd = shlex.join(runner + args + extra)
            if log.exists():
                r = read(log)
                if done(r) and (a.reuse_any or (r['cmd'] == cmd and r['tree'] == cur)):
                    continue
            # Each run starts at least one window, so a rerun always makes progress.
            if ran and time.monotonic() - start > a.budget_min * 60:
                _, line = summary(a.out, tags)
                print(f'replay_batch: budget spent; {line[2:]}; rerun the same command to resume', flush=True)
                return 3
            part = log.with_suffix('.log.part')
            t = time.monotonic()
            with part.open('w') as f:
                f.write('$ ' + cmd + '\n# tree ' + cur + '\n')
                f.flush()
                rc = subprocess.run(runner + args + extra, cwd=ROOT, stdout=f, stderr=subprocess.STDOUT).returncode
                f.write(f'EXIT {rc} wall {time.monotonic() - t:.0f}s\n')
            part.replace(log)
            ran += 1
            summary(a.out, tags)
            r = read(log)
            print(f"{tag}|{r['status']}|{r['detail']}", flush=True)
        fin, line = summary(a.out, tags)
        print(f'replay_batch: {line[2:]}; summary in {a.out / "_summary.txt"}', flush=True)
        return 0 if fin == len(tags) else 3
    finally:
        held.unlink(missing_ok=True)


if __name__ == '__main__':
    sys.exit(main())
