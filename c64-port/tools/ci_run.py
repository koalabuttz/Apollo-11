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
import gzip
import json
import lzma
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
                          r"cmp: (build/[^:]+): No such file|"
                          r"cannot open (build/[^:\s]+): No such file|"
                          # generated headers included from build/ (-Ibuild)
                          r"fatal error: '([A-Za-z0-9_.-]+\.h)' file not found")


def missing_build(output):
    """The build/ inputs a failed command names, headers mapped into build/."""
    found = {next(filter(None, m)) for m in BUILD_OUTPUT.findall(output)}
    return {p if p.startswith('build/') else 'build/' + p for p in found}
LOCK = threading.Lock()
# Recipe text that writes the path that follows it.
WRITES = r'(>|-o|-Map[,=])\s*'
TOOL_ARGS = {'--cc': 'MOS_CC', '--mos-cc': 'MOS_CC', '--c1541': 'C1541'}


def tool_args(script, make_args):
    """Toolchain options a producer script accepts, from make args or the environment."""
    values = dict(arg.split('=', 1) for arg in make_args if '=' in arg)
    text = script.read_text(errors='replace')
    argv = []
    for option, variable in TOOL_ARGS.items():
        value = values.get(variable) or os.environ.get(variable)
        if value and f"'{option}'" in text:
            argv += [option, value]
    return argv


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


def evidence_names():
    """Every evidence path, checked out or not: git's index plus release manifests."""
    names = subprocess.run(['git', 'ls-files', 'evidence'], cwd=ROOT, stdout=subprocess.PIPE,
                           text=True).stdout.split()
    for manifest in (ROOT / 'evidence').glob('*/SHA256.json'):
        files = json.loads(manifest.read_text()).get('files', {})
        names += [manifest.parent.relative_to(ROOT).as_posix() + '/' + name for name in files]
    return names


# Earlier attempts kept beside a milestone's final evidence; never expected values.
ARCHIVED = re.compile(r'-(failure|first-|timeout|prequalification)[^/]*/|/build/')


def frozen(path):
    """Copy a build/ input from its frozen copy under evidence/, if there is one.

    Expected values (final-source samples, target results, reference
    snapshots) were frozen into evidence/ when each milestone closed; the
    build/ copy the old serial loop left behind is the same bytes. Build
    products (PRGs, maps, disk images, headers, binaries) are never copied:
    a test must run what the current source builds."""
    name = pathlib.PurePosixPath(path).name
    if pathlib.PurePosixPath(name).suffix in ('', '.prg', '.map', '.d64', '.d81', '.h', '.o', '.elf'):
        return None
    if name.endswith('-modules.json'):
        return None  # a disk manifest: its build script also writes the PRGs it lists
    # build/x.bin matches evidence/**/x.bin; build/dir/x.json needs evidence/**/dir/x.json.
    tail = '/' + path[len('build/'):]
    for suffix, opener in (('', open), ('.gz', gzip.open), ('.xz', lzma.open)):
        for source in sorted((n for n in evidence_names()
                              if n.endswith(tail + suffix) and (tail.count('/') > 1 or n.count('/') <= 2)
                              and not ARCHIVED.search(n)),
                             key=lambda n: (n.count('/'), n)):
            with LOCK:
                restore(source)
            if not (ROOT / source).is_file():
                continue
            with opener(ROOT / source, 'rb') as data:
                (ROOT / path).parent.mkdir(parents=True, exist_ok=True)
                (ROOT / path).write_bytes(data.read())
            return source
    return None


MAKE_VAR = re.compile(r'\$\(if \$\(([A-Z_]+)\),\$\(\1\),[^)]*\)|\$\(([A-Z_]+)\)')


DERIVED = re.compile(r'^([A-Z_]+)\s*:?=\s*\$\(if \$\(filter ([^,]+),\$\(([A-Z_]+)\)\),([^,)]*),([^,)]*)\)\s*$', re.M)


def submake_writes(line, path, text=''):
    """The VAR=value assignments under which a recipe line writes `path`, or None.

    Per-variant sub-makefile rules name outputs like
    build/x$(if $(SELECTOR),$(SELECTOR),0).map; match those against the
    missing file and pass the variant back as a make variable. A name built
    from a derived variable (TREE = $(if $(filter 1,$(MISSION)),cm,lm), in
    the makefile `text`) is passed back as the variable it derives from."""
    for *_, out in re.findall(WRITES + r'"?(build/(?:\$\((?:[^()]|\([^()]*\))*\)|[^\s"$])+)', line):
        pattern, names, last = '', [], 0
        for var in MAKE_VAR.finditer(out):
            name = var[1] or var[2]
            pattern += re.escape(out[last:var.start()])
            pattern += f'(?P={name})' if name in names else f'(?P<{name}>[A-Za-z0-9_]+)'
            names.append(name)
            last = var.end()
        match = re.fullmatch(pattern + re.escape(out[last:]), path)
        if match:
            assigned = []
            derived = {d[0]: d[1:] for d in DERIVED.findall(text)}
            for name, value in match.groupdict().items():
                if name in derived:
                    when, source, then, other = derived[name]
                    default = re.search(r'^' + source + r'\s*\?=\s*(\S+)', text, re.M)
                    unless = default[1] if default and default[1] != when else '1' if when == '0' else '0'
                    name, value = source, when if value == then else unless if value == other else None
                    if value is None:
                        break
                assigned.append(f'{name}={value}')
            else:
                return assigned
    return None


_TEXT = None


def makefile_text():
    """The Makefile and every tools/*.mk, for derived-variable lookups."""
    global _TEXT
    if _TEXT is None:
        _TEXT = '\n'.join(p.read_text(errors='replace')
                           for p in [ROOT / 'Makefile', *sorted((ROOT / 'tools').glob('*.mk'))])
    return _TEXT


def produce(path, target, log_dir, make_args, attempts, stack=()):
    """Recreate a build/ input that the old serial loop left behind.

    Legacy targets read streams written by the oracle differentials
    (`make differential`), by their own module's differential target, or by
    another target's recipe. Try those producers in that order."""
    if (ROOT / path).exists():
        return True
    source = frozen(path)
    if source:
        print(f'  {target}: copied {path} from {source}', flush=True)
        return True
    rules = ci_rules()
    candidates = []
    if path in rules:
        candidates.append(('make', path))
    if path.endswith('.map'):
        # A map linked beside a PRG from a pattern rule (build/x-%.prg):
        # make the PRG some rule depends on, and make links the map with it.
        prg = path[:-len('.map')] + '.prg'
        if prg in rules or any(prg in deps for deps, _ in rules.values()):
            candidates.append(('make', prg))
    for name, (_, recipe) in rules.items():
        if name != target and re.search(WRITES + re.escape(path) + r'(\s|$)', recipe):
            candidates.append(('make', name))
        elif name != target and '$(' in recipe:
            # Included per-variant rules (build/x-phase$(PHASE).map) run
            # with the variables that name the missing file.
            for line in recipe.splitlines():
                assigned = submake_writes(line, path, makefile_text())
                if assigned:
                    candidates.append(('submake', ('Makefile', name, *assigned)))
                    break
    name = pathlib.PurePosixPath(path).name
    stem = re.sub(r'(-(native|reference|sanitize|live|target|inputs))?(\.snapshots|\.json|\.trace)?(\.gz|\.xz)?$', '',
                  name)
    script = ROOT / 'tools' / (stem.replace('-', '_') + '_differential.py')
    scripts = [script] if script.is_file() else []
    scripts += sorted(s for s in (ROOT / 'tools').glob('*_differential.py')
                      if s != script and s.name != 'differential.py'
                      and path[len('build/'):] in s.read_text(errors='replace'))
    candidates += [('script', s) for s in scripts]
    # Sub-makefiles run through `make -f tools/<x>.mk` are not in the main
    # database; find the rule whose recipe writes the file.
    included = set(re.findall(r'^-?include\s+(tools/\S+\.mk)', (ROOT / 'Makefile').read_text(), re.M))
    for mk in sorted((ROOT / 'tools').glob('*.mk')):
        if mk.relative_to(ROOT).as_posix() in included:
            continue
        rule, text = None, mk.read_text(errors='replace')
        for line in text.splitlines():
            head = re.match(r'([A-Za-z0-9_.-]+)\s*:(?!=)', line)
            if head:
                rule = head[1]
            elif rule and line.startswith('\t'):
                assigned = submake_writes(line, path, text)
                if assigned is not None:
                    candidates.append(('submake', (mk.relative_to(ROOT).as_posix(), rule, *assigned)))
                    break
    # Build and freeze scripts that write the file under its literal name
    # (module manifests, frozen samples, PRGs linked with -Map).
    writers = [s for s in sorted((ROOT / 'tools').glob('*.py'))
               if s not in scripts and not s.name.endswith(('_vice_test.py', '_differential.py'))
               and s.name not in ('ci_run.py', 'ci_affected.py', 'legacy_vice.py')
               and re.search(r"^.*['/]" + re.escape(name) + r"['\s].*(write_(text|bytes)|'-o'|-Map)"
                             r"|^.*(write_(text|bytes)|'-o'|-Map).*['/]" + re.escape(name) + r"['\s]",
                             s.read_text(errors='replace'), re.M)]
    # Build scripts named after the file they write under a computed name
    # (lm_interpreter_change_target_build.py writes stem+'-target-modules.json').
    flat = name.replace('-', '_')
    writers += sorted((s for s in (ROOT / 'tools').glob('*_build.py')
                       if s not in writers and '_' in s.name[:-len('_build.py')]
                       and flat.startswith(s.name[:-len('_build.py')] + '_')),
                      key=lambda s: -len(s.name))
    writers.sort(key=lambda s: (not s.name.endswith('_build.py'), not s.name.startswith('freeze'), s.name))
    candidates += [('script', s) for s in writers[:4]]
    module = target
    for prefix in ('c64-', 'sanitize-', 'test-'):
        module = module[len(prefix):] if module.startswith(prefix) else module
    for rule in (f'differential-{module}', f'test-{module}'):
        if rule in rules and rule != target:
            candidates.append(('make', rule))
    for kind, what in candidates:
        if (kind == 'make' and what in stack) or (kind == 'script' and what.name in stack):
            continue
        label = what.name if kind == 'script' else ' '.join(what) if kind == 'submake' else what
        print(f'  {target}: producing {path} via {label}', flush=True)
        if kind == 'make':
            ok = run(what, log_dir, make_args, attempts, stack)[1]
        else:
            if kind == 'script':
                argv = [str(ROOT / 'deps/fixture')] if 'sys.argv[1]' in what.read_text(errors='replace') else []
                argv += tool_args(what, make_args)
                cmd, log = [sys.executable, str(what), *argv], log_dir / (what.stem + '.log')
            else:
                cmd = ['make', '--no-print-directory', 'SKIP_EVIDENCE_RESTORE=1', *make_args, '-f', *what]
                log = log_dir / '-'.join([pathlib.Path(what[0]).stem, *what[1:]]).replace('=', '')
                log = log.with_name(log.name + '.log')
            ok = produce_with(cmd, log, label, log_dir, make_args, attempts, stack)
        if (ROOT / path).exists():
            if ok:
                return True
            (ROOT / path).unlink()  # a failed producer may leave a truncated stream
    return False


def produce_with(cmd, log, label, log_dir, make_args, attempts, stack):
    """Run a producer, first producing any build/ input it turns out to need."""
    tried = set()
    for _ in range(attempts):
        proc = subprocess.run(cmd, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        log.write_text(proc.stdout)
        if proc.returncode == 0:
            return True
        built = sorted(missing_build(proc.stdout) - tried)
        if label in stack or not built:
            return False
        tried.update(built)
        if not any([produce(path, label, log_dir, make_args, attempts, stack + (label,)) for path in built]):
            return False
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
        built = sorted(missing_build(proc.stdout) - tried)
        if built:
            tried.update(built)
            ran = [produce(path, target, log_dir, make_args, attempts, stack + (target,)) for path in built]
            if any(ran):
                continue
        if 'missing/outside evidence file' in proc.stdout and '--coverage' not in tried:
            # The coverage audit names categories, not paths: fetch every
            # release-stored file that a coverage certificate cites.
            tried.add('--coverage')
            with LOCK:
                fetch = subprocess.run([sys.executable, 'tools/evidence.py', 'fetch', '--coverage'], cwd=ROOT)
            if fetch.returncode == 0:
                print(f'  {target}: fetched coverage evidence, rerunning', flush=True)
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
