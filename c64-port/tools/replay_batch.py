#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run a list of C64 replay windows (tools/flight_scenario_vice.py) as a
batch that resumes: every window writes its own log, a window whose log is
finished on the same tree with the same command is skipped, and the batch
stops starting windows once its time budget is spent (each run still starts
at least one window). Rerun the same command to go on where it stopped,
after a time limit, a lost container or a crash. Run one batch per tree
(windows share build/flight-store); the lock on OUT/.lock only catches a
second batch on the same OUT. The runner holds the lock too, so it stays
held until the runner is gone even if the batch itself is killed.

The window list has one window per line, '#' starts a comment:

  SCENARIO SNAP_CS CS [more flight_scenario_vice.py options]

  lm-p63-descent 17500 300
  lm-rendezvous 8000 1000 --replay-outputs

Options after '--' on the command line go to every window; --replay is
always added. A window's log is OUT/SCENARIO-SNAP+CS.log (written to a
.part file and renamed when the run ends). It starts with '$ command',
'# tree ID' (the commit, plus a hash of uncommitted tracked changes) and
'# attempt N', and ends with 'EXIT rc wall Ns' ('EXIT timeout ...' when
--window-timeout-min stopped it).

A window's status comes from its exit code and every line of its log:
  match         rc 0, no line with FAIL, and the pass line ("... in the
                host's state", not "matches the host at no point"; under
                --chain K > 1, the chain line "every window matches the
                host and every seam holds")
  match-masked  a match that ran with a mask, skip or unpaired state
                ("skipped (temporary", "masked_", "(with skips)", "states
                with no partner"): not a full match
  FAIL          a line with FAIL anywhere (window, seam, chain, REU bounds,
                outputs), whatever the order; the first one is quoted
  crash         rc not 0 and no FAIL line (a traceback, VICE died)
  timeout       stopped by --window-timeout-min
  no verdict    rc 0 but no pass line, or a pass line with no compared
                point
  stale         a finished log from another tree or command (rerun)
  not run       no log yet
crash, timeout and no verdict rerun on resume, up to 3 attempts on the same
tree and command; after that they count as finished with that status.

OUT/_summary.txt has a header '# N of M windows finished [INCOMPLETE]:
counts[; MIXED: ...]' and one row per listed window:
'tag|status|rc|commit|store sha256|detail'. --reuse-any keeps finished logs
from another tree or command (they show their own status, and MIXED flags
them when the stamps differ).

  replay_batch.py --windows FILE --out DIR [--budget-min 75]
                  [--window-timeout-min M] [-- OPTIONS]

The budget only stops new windows from starting: a window already running
finishes (or hits --window-timeout-min), so budget plus one window must fit
any outer job limit.

Exit 0 when every window finished (match or not), 3 when windows are left
(budget spent, or a window to retry; rerun to resume), 1 for a bad window
list or a held lock."""
import argparse
import collections
import fcntl
import hashlib
import os
import pathlib
import re
import shlex
import signal
import socket
import subprocess
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
RUNNER = [sys.executable, 'tools/flight_scenario_vice.py']
STAMP = re.compile(r': commit (\S+)( with local changes)?; store \S+ sha256 ([0-9a-f]+)')
MASKS = ('skipped (temporary', 'masked_', '(with skips)', 'states with no partner')
PASS = "in the host's state"
NO_POINT = 'matches the host at no point'
CHAIN_PASS = 'every window matches the host and every seam holds'
SIGNALS = (signal.SIGTERM, signal.SIGINT, signal.SIGHUP)
RETRY = ('crash', 'timeout', 'no verdict')
TRIES = 3
ORDER = ['match', 'match-masked', 'FAIL', 'crash', 'timeout', 'no verdict', 'stale', 'not run']


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


def chain_k(cmd):
    """The --chain K of a command line (1 without one)."""
    f = shlex.split(cmd or '')
    for i, x in enumerate(f):
        if x == '--chain' and i + 1 < len(f) and f[i + 1].isdigit():
            return int(f[i + 1])
        if x.startswith('--chain=') and x[8:].isdigit():
            return int(x[8:])
    return 1


def read(log):
    """Parse a log: dict with cmd, tree, attempt, rc (int, 'timeout' or
    None if cut off), commit, store, status, detail."""
    lines = log.read_text(errors='replace').splitlines()
    r = {'cmd': None, 'tree': None, 'attempt': 1, 'rc': None, 'commit': '', 'store': '', 'detail': ''}
    if lines and lines[0].startswith('$ '):
        r['cmd'] = lines.pop(0)[2:]
    for _ in range(2):
        if not (lines and lines[0].startswith(('# tree ', '# attempt '))):
            break
        k, _, v = lines.pop(0)[2:].partition(' ')
        if k == 'tree':
            r['tree'] = v
        elif k == 'attempt' and v.isdigit():
            r['attempt'] = int(v)
    body = lines
    if body and body[-1].startswith('EXIT '):
        f = body.pop().split()
        r['rc'] = 'timeout' if f[1:2] == ['timeout'] else int(f[1]) if len(f) > 1 and f[1].lstrip('-').isdigit() else None
    fail = masked = None
    stores = set()
    passes, chained = [], None
    for line in body:
        m = STAMP.search(line)
        if m:
            r['commit'] = m.group(1) + (' with local changes' if m.group(2) else '')
            stores.add(m.group(3))
        if fail is None and 'FAIL' in line:
            fail = line.strip()
        if masked is None and any(k in line for k in MASKS):
            masked = line.strip()
        if PASS in line and NO_POINT not in line:
            passes.append(line.strip())
        if CHAIN_PASS in line:
            chained = line.strip()
    r['store'] = ','.join(sorted(stores))
    k = chain_k(r['cmd'])
    verdict = chained if k > 1 else (passes[-1] if passes else None)
    if r['rc'] is None:
        r['status'], r['detail'] = 'crash', 'log has no EXIT line'
    elif r['rc'] == 'timeout':
        r['status'], r['detail'] = 'timeout', (body[-1].strip() if body else '')
    elif fail is not None:
        r['status'], r['detail'] = 'FAIL', fail
    elif r['rc'] != 0:
        r['status'], r['detail'] = 'crash', (body[-1].strip() if body else 'no output')
    elif verdict is None:
        r['status'] = 'no verdict'
        r['detail'] = f'rc 0 but no {"chain " if k > 1 else ""}pass line'
    elif masked is not None:
        r['status'], r['detail'] = 'match-masked', masked
    else:
        r['status'], r['detail'] = 'match', verdict
    return r


def current(r, cmd, cur, reuse_any):
    """The log belongs to this tree and command (or --reuse-any)."""
    return reuse_any or (r['cmd'] == cmd and r['tree'] == cur)


def done(r):
    """Finished: a verdict, or a retry status that used up its tries."""
    return r['status'] not in RETRY or r['attempt'] >= TRIES


def row(out, tag, cmd, cur, reuse_any):
    log = out / f'{tag}.log'
    if not log.exists():
        return None
    r = read(log)
    if not current(r, cmd, cur, reuse_any):
        r['status'] = 'stale'
        r['detail'] = f"from tree {r['tree'] or '?'}" + (' with another command' if r['cmd'] != cmd else '')
    return r


def summary(out, todo, cur, reuse_any):
    """Write OUT/_summary.txt; return (finished, header). Only current logs
    count as finished."""
    rows, counts, commits, stores, fin = [], collections.Counter(), set(), set(), 0
    for tag, cmd in todo:
        r = row(out, tag, cmd, cur, reuse_any)
        if r is None:
            rows.append(f'{tag}|not run||||')
            counts['not run'] += 1
            continue
        counts[r['status']] += 1
        if r['status'] != 'stale':
            fin += done(r)
            if r['commit']:
                commits.add(r['commit'])
            if r['store']:
                stores.update(r['store'].split(','))
        rc = '' if r['rc'] is None else str(r['rc'])
        rows.append(f"{tag}|{r['status']}|{rc}|{r['commit']}|{r['store']}|{r['detail']}")
    head = f'# {fin} of {len(todo)} windows finished'
    if fin < len(todo):
        head += ' INCOMPLETE'
    head += ': ' + ', '.join(f'{counts[k]} {k}' for k in ORDER if counts[k])
    if len(commits) > 1 or len(stores) > 1:
        head += f'; MIXED: {len(commits)} commits, {len(stores)} store images'
    tmp = out / '_summary.txt.part'
    tmp.write_text(head + '\n' + ''.join(r + '\n' for r in rows))
    tmp.replace(out / '_summary.txt')
    return fin, head


def lock(out):
    """Hold an flock on OUT/.lock for the whole batch (the kernel drops it
    when the process dies, so a dead batch never leaves it held)."""
    fd = os.open(out / '.lock', os.O_RDWR | os.O_CREAT, 0o644)
    try:
        fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        who = os.read(fd, 200).decode(errors='replace').strip() or '?'
        os.close(fd)
        sys.stderr.write(f'replay_batch: {out / ".lock"} is held by {who}; one batch per OUT\n')
        return None
    os.ftruncate(fd, 0)
    os.write(fd, f'{os.getpid()} {socket.gethostname()}'.encode())
    return fd


class Stop(Exception):
    pass


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
                   help='start no window after this many minutes (default 75); a running window finishes')
    p.add_argument('--window-timeout-min', type=float, default=0.0,
                   help='stop a window after this many minutes (default 0: no limit)')
    p.add_argument('--reuse-any', action='store_true',
                   help='keep every finished log, even from another tree or command')
    p.add_argument('--runner', default=None, help=argparse.SUPPRESS)  # tests
    p.add_argument('--head', default=None, help=argparse.SUPPRESS)    # tests
    a = p.parse_args(argv)
    runner = shlex.split(a.runner) if a.runner else RUNNER
    todo = [(tag, args, shlex.join(runner + args + extra)) for tag, args in windows(a.windows)]
    a.out.mkdir(parents=True, exist_ok=True)
    held = lock(a.out)
    if held is None:
        return 1
    child = None

    def stop(sig, _frame):
        raise Stop(sig)
    for s in SIGNALS:
        signal.signal(s, stop)
    try:
        for stale in a.out.glob('*.log.part'):
            stale.unlink()
        cur = a.head if a.head is not None else tree()
        plan = [(tag, cmd) for tag, _, cmd in todo]
        start, ran = time.monotonic(), 0
        for tag, args, cmd in todo:
            log = a.out / f'{tag}.log'
            attempt = 1
            if log.exists():
                r = read(log)
                if current(r, cmd, cur, a.reuse_any):
                    if done(r):
                        continue
                    attempt = r['attempt'] + 1
            # Each run starts at least one window, so a rerun always makes progress.
            if ran and time.monotonic() - start > a.budget_min * 60:
                _, head = summary(a.out, plan, cur, a.reuse_any)
                print(f'replay_batch: budget spent; {head[2:]}; rerun the same command to resume', flush=True)
                return 3
            part = log.with_suffix('.log.part')
            t = time.monotonic()
            with part.open('w') as f:
                f.write(f'$ {cmd}\n# tree {cur}\n# attempt {attempt}\n')
                f.flush()
                # Own session, so a stop of the batch can stop the whole run;
                # the runner inherits the lock fd, so OUT stays locked while it
                # runs; signals wait until child is set.
                old = signal.pthread_sigmask(signal.SIG_BLOCK, SIGNALS)
                try:
                    child = subprocess.Popen(runner + args + extra, cwd=ROOT, stdout=f, stderr=subprocess.STDOUT,
                                             start_new_session=True, pass_fds=(held,),
                                             preexec_fn=lambda: signal.pthread_sigmask(signal.SIG_SETMASK, old))
                finally:
                    signal.pthread_sigmask(signal.SIG_SETMASK, old)
                try:
                    rc = child.wait(timeout=a.window_timeout_min * 60 or None)
                except subprocess.TimeoutExpired:
                    os.killpg(child.pid, signal.SIGKILL)
                    child.wait()
                    rc = 'timeout'
                child = None
                f.write(f'EXIT {rc} wall {time.monotonic() - t:.0f}s\n')
            part.replace(log)
            ran += 1
            summary(a.out, plan, cur, a.reuse_any)
            r = read(log)
            print(f"{tag}|{r['status']}|{r['detail']}", flush=True)
        fin, head = summary(a.out, plan, cur, a.reuse_any)
        left = '' if fin == len(todo) else '; rerun the same command to retry what is left'
        print(f'replay_batch: {head[2:]}{left}; summary in {a.out / "_summary.txt"}', flush=True)
        return 0 if fin == len(todo) else 3
    except Stop as e:
        sys.stderr.write(f'replay_batch: stopped by signal {e.args[0]}; rerun the same command to resume\n')
        return 128 + e.args[0]
    finally:
        if child is not None and child.poll() is None:
            try:
                os.killpg(child.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            child.wait()
        os.close(held)


if __name__ == '__main__':
    sys.exit(main())
