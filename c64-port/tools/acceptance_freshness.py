#!/usr/bin/env python3
"""Says whether each program's whole-program acceptance still describes the
C64 build it would ship.

Each program's bootable acceptance doc (docs/acceptance/<program>-
bootable_c64_application.md) lists the sha256 of the files the C64 runs:
the REU store, the flight resident, the loader and the disk images (for
Luminary099 also the check disk, test-*.d81, which is built from its store). This
tool hashes the same files under build/ (run `make flight-disk` first) and
reports each program as FRESH when every built one matches (the store,
resident and loader must be built), or STALE with the files that moved. It
prints the checkout's commit so a build/ older than the checkout is visible. A STALE
program's acceptance was recorded against an older build, so it needs a
re-acceptance run at the current commit before anyone quotes it for the
shipped build.

port_coverage.py --require-complete checks that acceptance exists; this
checks that it is current. It also requires the six category docs of each
program to name one commit and PASS, and that commit to be an ancestor of the
checkout's HEAD. --require-fresh exits 1 on any STALE or
MIXED program or missing build file. port-ci runs it (job acceptance-fresh,
1541 and 1581 sets) on every PR and port-branch push that changes more than
prose or evidence, so a PR that moves an accepted program's bytes must
carry that program's re-acceptance (CONTRIBUTING-port.md, "After
acceptance").

docs/acceptance/pending-reacceptance.json lists programs the base already
knows are stale: the PR that made them so, and the sha256 of each file that
moved at that point. A listed program reports STALE without failing only
while every moved file still has its pinned hash, so any further byte move
fails as usual. The re-acceptance PR deletes the entry; an entry can only be
added in a reviewed PR.

Bytes are not the only thing an acceptance rests on: the program map and
the scoring in program_acceptance.py decide what the integrated doc
credits. So each program's integrated doc is also re-scored from the
evidence it cites with the current map and tool (program_acceptance.py
--rescore), and so is each recovery and restart doc, re-judged with
restart_acceptance.py's verdicts (#969 B8). A doc whose counts or result
would change is CRITERIA STALE and fails like a byte move (#942 B2). A
pending entry with a "criteria" reason reports it without failing, the
same way: a string covers every re-scored doc, a dict maps each category
it covers to its reason.
"""
import json
import argparse
import hashlib
import pathlib
import re
import subprocess
import sys
import time

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import program_acceptance  # noqa: E402
import restart_acceptance  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parent.parent
PROGRAMS = {'Luminary099': 'lm', 'Comanche055': 'cm'}
PENDING = ROOT / 'docs/acceptance/pending-reacceptance.json'
CATEGORIES = ('bootable_c64_application', 'device_adapter_tests', 'integrated_program_tests',
              'memory_and_overlay_validation', 'native_output_tests', 'recovery_and_restart_tests')
ROW = re.compile(r'^\| `([^`]+)` \| `([0-9a-f]{64})` \|$')


def payload(m):
    """The files whose bytes the C64 runs, as the bootable doc names them."""
    return [f'flight-store/{m}.reu', f'flight-disk/{m}-resident.prg', f'flight-disk/{m}-loader.prg']


def recorded(program):
    doc = ROOT / 'docs/acceptance' / f'{program}-bootable_c64_application.md'
    text = doc.read_text()
    commit = re.search(r'at commit `([0-9a-f]+)`', text)
    rows = {}
    for line in text.splitlines():
        match = ROW.match(line.strip())
        if match:
            rows[match.group(1)] = match.group(2)
    return (commit.group(1) if commit else '?'), rows


def check(program, m, build):
    """Every file the doc lists is compared when it is built; the store,
    resident and loader must be. Disk images exist only after the runs that
    make them (the 1541 sets come from --vice), so an unbuilt image is
    reported as unchecked rather than missing."""
    commit, rows = recorded(program)
    moved, missing, unchecked = [], [], []
    for name in payload(m):
        if name not in rows:
            missing.append(f'{name} (not in the doc)')
    for name, want in rows.items():
        path = build / name
        if not path.exists() and name in payload(m):
            missing.append(f'{name} (not built)')
        elif not path.exists():
            unchecked.append(name)
        elif hashlib.sha256(path.read_bytes()).hexdigest() != want:
            moved.append(name)
    return commit, moved, missing, unchecked


def docs_agree(program):
    """The six category docs must come from one acceptance run: each names
    the same commit and says PASS. Re-citing only the bootable doc's hash
    table would otherwise make a program FRESH without re-running the rest."""
    seen = {}
    for c in CATEGORIES:
        doc = ROOT / 'docs/acceptance' / f'{program}-{c}.md'
        head = doc.read_text()[:400] if doc.exists() else ''
        commit = re.search(r'at commit `([0-9a-f]+)`', head)
        seen[c] = (commit.group(1) if commit else 'missing') + ('' if 'Result: **PASS' in head else ' (not PASS)')
    agree = len(set(seen.values())) == 1 and not next(iter(seen.values())).endswith(')') and 'missing' not in seen.values()
    if agree:
        commit = next(iter(seen.values()))
        if not is_ancestor(commit):
            seen = {c: f'{v} (not an ancestor of HEAD)' for c, v in seen.items()}
            agree = False
    return agree, seen


def is_ancestor(commit):
    """The docs' commit must be in this checkout's history: a doc that names
    a commit from another branch (or one made up) describes a build this
    checkout never had. Fails closed: a commit the checkout lacks, or a
    shallow history that cannot show the ancestry, is not an ancestor
    (port-ci fetches the commit history before it runs this)."""
    try:
        return subprocess.run(['git', 'merge-base', '--is-ancestor', commit, 'HEAD'], cwd=ROOT,
                              capture_output=True).returncode == 0
    except OSError:
        return False


def head():
    try:
        out = subprocess.run(['git', 'rev-parse', '--short=12', 'HEAD'], cwd=ROOT, capture_output=True, text=True)
        dirty = subprocess.run(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=ROOT,
                               capture_output=True, text=True).stdout.strip()
        return out.stdout.strip() + (' + local changes' if dirty else '')
    except OSError:
        return '?'


# Each category whose doc is re-judged from the evidence it cites, the
# re-scoring function, and the run that regenerates the doc.
RESCORE = (('integrated_program_tests', program_acceptance.rescore,
            'program_acceptance.py {m} (--no-run when the results exist)'),
           ('recovery_and_restart_tests', restart_acceptance.rescore, 'restart_acceptance.py {m}'))


def criteria(entry, category):
    """A pending entry's "criteria" reason for this category: a string covers
    every re-scored doc, a dict names each category it covers."""
    reason = entry.get('criteria')
    return reason.get(category) if isinstance(reason, dict) else reason


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--build', type=pathlib.Path, default=ROOT / 'build')
    parser.add_argument('--require-fresh', action='store_true')
    args = parser.parse_args()
    bad = False
    built = args.build / 'flight-store' / 'lm.reu'
    print(f'checkout {head()}; compare against the build at that checkout (rebuild with `make flight-disk` if build/ is older)'
          + (f'; build/flight-store/lm.reu written {time.strftime("%Y-%m-%d %H:%M UTC", time.gmtime(built.stat().st_mtime))}'
             if built.exists() else ''))
    pending = json.loads(PENDING.read_text()) if PENDING.exists() else {}
    for program, m in PROGRAMS.items():
        commit, moved, missing, unchecked = check(program, m, args.build)
        agree, seen = docs_agree(program)
        if not agree:
            bad = True
            print(f'{program}: MIXED acceptance docs (each category must come from one run at one commit, PASS, and that commit must be an ancestor of HEAD): '
                  + ', '.join(f'{c} {v}' for c, v in seen.items()))
        elif missing:
            bad = True
            print(f'{program}: UNKNOWN (accepted at {commit}); missing: {", ".join(missing)}')
        elif moved and program in pending and all(
                hashlib.sha256((args.build / n).read_bytes()).hexdigest() == pending[program].get('files', {}).get(n)
                for n in moved):
            print(f'{program}: STALE, pending re-acceptance (since {pending[program].get("since", "?")}; accepted at {commit});'
                  f' changed since, at their pinned hashes: {", ".join(moved)}')
        elif moved:
            bad = True
            print(f'{program}: STALE (accepted at {commit}); changed since: {", ".join(moved)}'
                  + ('; the pending entry pins other bytes, so this is a new move' if program in pending else ''))
        else:
            print(f'{program}: FRESH (accepted at {commit}; every built file the doc lists matches)'
                  + (f'; not built, so not checked: {", ".join(unchecked)}' if unchecked else ''))
        if m == 'lm' and not any(re.fullmatch(r'flight-disk/test-[0-9]+\.d81', n) for n in recorded(program)[1]):
            print(f'{program}: note: the doc lists no check disk (test-*.d81); a bootable doc written by '
                  'tools/boot_acceptance.py since #942 B7 does, and then a moved check disk is STALE')
        for category, rescore, rerun in RESCORE:
            same, text = rescore(m)
            reason = criteria(pending.get(program, {}), category)
            if not same and reason:
                print(f'{program}: CRITERIA STALE, pending re-acceptance ({reason}): the {category} doc {text}')
            elif not same:
                bad = True
                print(f'{program}: CRITERIA STALE: the {category} doc {text}. Re-run {rerun.format(m=m)} in a '
                      're-acceptance PR, or add a "criteria" pending entry.')
    return 1 if bad and args.require_fresh else 0


if __name__ == '__main__':
    sys.exit(main())
