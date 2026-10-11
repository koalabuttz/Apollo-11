#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run make targets in parallel for CI, fetching skipped legacy evidence on demand.

Lean checkouts (tools/ci_checkout.sh) leave legacy evidence blobs over 1 MB out
of the tree. When a target fails because one of them is missing, this pulls
that blob from git (or a release-stored file through tools/evidence.py) and
reruns the target, so the suite needs only the evidence it actually reads.

  python3 tools/ci_run.py [-j N] [--log-dir build/ci-logs] TARGET...
  python3 tools/ci_run.py --group test        # every prerequisite of `make test`
"""
import argparse
import concurrent.futures
import json
import os
import pathlib
import re
import subprocess
import sys
import threading
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
MISSING = re.compile(r"(?:No such file or directory|cannot open|not found)[^\n]*?(evidence/[A-Za-z0-9._/+-]+)"
                     r"|(evidence/[A-Za-z0-9._/+-]+)[^\n]*?(?:No such file or directory|not found)")
BUILD_OUTPUT = re.compile(r"No such file or directory: '[^']*?/c64-port/(build/[^']+)'|"
                          r"cmp: (build/[^:]+): No such file")
LOCK = threading.Lock()


def group_targets(group):
    sys.path.insert(0, str(ROOT / 'tools'))
    import ci_affected
    rules = ci_affected.database()
    core = {'test': ['test-core'], 'sanitize': ['sanitize-core']}.get(group, [])
    return core + sorted(t for t in rules.get(group, (set(), ''))[0] if t != 'all')


def restore(path):
    """Materialize one evidence file skipped by the lean checkout."""
    target = ROOT / path
    if target.exists():
        return False
    parts = pathlib.PurePosixPath(path).parts
    if len(parts) == 3 and (ROOT / 'evidence' / parts[1] / 'SHA256.json').is_file():
        result = subprocess.run([sys.executable, 'tools/evidence.py', 'fetch', parts[1]], cwd=ROOT)
        return result.returncode == 0 and target.exists()
    blob = subprocess.run(['git', 'cat-file', 'blob', f'HEAD:c64-port/{path}'], cwd=ROOT,
                          stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    if blob.returncode:
        return False
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(blob.stdout)
    for sibling in siblings(path):
        restore(sibling)
    return True


def siblings(path):
    """Skipped files that differ from `path` only in their numbers.

    Chunked evidence (`foo-source-11.snapshots.bin.xz`) is read part by part,
    so one missing chunk means the rest are missing too; fetch them together
    instead of one failing rerun per chunk."""
    sparse = ROOT.parent / '.git' / 'info' / 'sparse-checkout'
    if not sparse.is_file():
        return []
    shape = re.compile(re.sub(r'\\d+|\d+', r'\\d+', re.escape(path)) + '$')
    found = []
    for line in sparse.read_text().splitlines():
        if line.startswith('!/c64-port/'):
            other = line[len('!/c64-port/'):]
            if other != path and shape.match(other) and not (ROOT / other).exists():
                found.append(other)
    return found


def produce(path, target, log_dir, make_args, attempts, stack=()):
    """Recreate a build/ input that the old serial loop left behind.

    Legacy targets read streams written by the oracle differentials
    (`make differential`), by their own module's differential target, or by
    another target's recipe. Try those producers in that order."""
    if (ROOT / path).exists():
        return True
    rules = ci_rules()
    candidates = []
    if path in rules:
        candidates.append(('make', path))
    for name, (_, recipe) in rules.items():
        if name != target and re.search(r'(>|-o)\s*' + re.escape(path) + r'(\s|$)', recipe):
            candidates.append(('make', name))
    stem = re.sub(r'(-(native|reference|sanitize|live|target))?(\.snapshots(\.gz)?|\.json|\.trace)?$', '',
                  pathlib.PurePosixPath(path).name)
    script = ROOT / 'tools' / (stem.replace('-', '_') + '_differential.py')
    if script.is_file():
        candidates.append(('script', script))
    module = target
    for prefix in ('c64-', 'sanitize-', 'test-'):
        module = module[len(prefix):] if module.startswith(prefix) else module
    for name in (f'differential-{module}', f'test-{module}'):
        if name in rules and name != target:
            candidates.append(('make', name))
    for kind, what in candidates:
        print(f'  {target}: producing {path} via {getattr(what, "name", what)}', flush=True)
        if kind == 'make':
            if what in stack:
                continue
            run(what, log_dir, make_args, attempts, stack)
        else:
            text = what.read_text(errors='replace')
            argv = [str(ROOT / 'deps/fixture')] if 'sys.argv[1]' in text else []
            log = log_dir / (what.stem + '.log')
            proc = subprocess.run([sys.executable, str(what), *argv], cwd=ROOT, stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT, text=True)
            log.write_text(proc.stdout)
        if (ROOT / path).exists():
            return True
    return False


_RULES = None


def ci_rules():
    global _RULES
    with LOCK:
        if _RULES is None:
            sys.path.insert(0, str(ROOT / 'tools'))
            import ci_affected
            _RULES = ci_affected.database()
    return _RULES


def run(target, log_dir, make_args, attempts, stack=()):
    log = log_dir / (re.sub(r'[^A-Za-z0-9._-]', '_', target) + '.log')
    start = time.monotonic()
    tried = set()
    for _ in range(attempts):
        proc = subprocess.run(['make', '--no-print-directory', 'SKIP_EVIDENCE_RESTORE=1', *make_args, target],
                              cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                              env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
        log.write_text(proc.stdout)
        if proc.returncode == 0:
            return target, True, time.monotonic() - start, ''
        built = sorted({a or b for a, b in BUILD_OUTPUT.findall(proc.stdout)} - tried)
        if built:
            tried.update(built)
            ran = [produce(path, target, log_dir, make_args, attempts, stack + (target,)) for path in built]
            if any(ran):
                continue
        wanted = {(a or b).rstrip("'.,:)") for a, b in MISSING.findall(proc.stdout)} - tried
        tried |= wanted
        with LOCK:
            # Another target may have restored the file meanwhile; that counts.
            fetched = [p for p in sorted(wanted) if restore(p) or (ROOT / p).exists()]
        if not fetched:
            break
        print(f'  {target}: fetched {len(fetched)} skipped evidence file(s), rerunning', flush=True)
    return target, False, time.monotonic() - start, proc.stdout[-3000:]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('targets', nargs='*')
    parser.add_argument('--group', action='append', default=[])
    parser.add_argument('-j', '--jobs', type=int, default=os.cpu_count() or 2)
    parser.add_argument('--log-dir', type=pathlib.Path, default=ROOT / 'build/ci-logs')
    parser.add_argument('--attempts', type=int, default=30)
    parser.add_argument('--make-arg', action='append', default=[])
    parser.add_argument('--summary', type=pathlib.Path)
    args = parser.parse_args()
    targets = list(dict.fromkeys(args.targets + [t for g in args.group for t in group_targets(g)]))
    if not targets:
        print('nothing to run')
        return 0
    args.log_dir.mkdir(parents=True, exist_ok=True)
    (ROOT / 'build').mkdir(exist_ok=True)
    results = []
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        futures = [pool.submit(run, t, args.log_dir, args.make_arg, args.attempts) for t in targets]
        for future in concurrent.futures.as_completed(futures):
            target, ok, seconds, tail = future.result()
            results.append({'target': target, 'ok': ok, 'seconds': round(seconds, 1)})
            print(f"{'PASS' if ok else 'FAIL'} {target} ({seconds:.0f}s)", flush=True)
            if not ok:
                print(tail, flush=True)
    failed = sorted(r['target'] for r in results if not r['ok'])
    if failed:
        # Separate make processes can race on a shared intermediate; a real
        # failure fails again when run alone.
        print('rerunning failed targets one at a time:', ' '.join(failed), flush=True)
        for target in failed:
            _, ok, seconds, tail = run(target, args.log_dir, args.make_arg, args.attempts)
            print(f"{'PASS' if ok else 'FAIL'} {target} (serial, {seconds:.0f}s)", flush=True)
            if not ok:
                print(tail, flush=True)
            for r in results:
                if r['target'] == target:
                    r.update(ok=ok, serial=True)
        failed = sorted(r['target'] for r in results if not r['ok'])
    if args.summary:
        args.summary.write_text(json.dumps({'failed': failed, 'results': results}, indent=1) + '\n')
    print(f'{len(results) - len(failed)}/{len(results)} targets passed' +
          (': failed ' + ' '.join(failed) if failed else ''))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
