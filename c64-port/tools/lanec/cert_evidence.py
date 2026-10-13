#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Evidence for a Lane C full-file certificate, rebuilt from the sources.

  python3 tools/lanec/cert_evidence.py PREFIX OUTDIR SUITE... [--refusals SUITE...]
                                       [--refusals-only SUITE...] [--wholefile SCRIPT] [--c64 SUITE=TARGET...]

For every suite it runs tools/lanec/diff.py in the strict and the ASan/UBSan
builds (with --refusals for the suites named there; a --refusals-only suite
must be refused unchanged in every case and is not run in the oracle) and counts the accepted
cases per mission (build/lanec-native --entries). It then runs the whole-file
source map SCRIPT into OUTDIR and every named C64 diagnostic in VICE
(make vice-TARGET). Writes OUTDIR/PREFIX-comparison.json,
OUTDIR/PREFIX-checks.log and OUTDIR/PREFIX-c64-results.json.
"""
import argparse, collections, hashlib, json, os, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser()
p.add_argument('prefix')
p.add_argument('outdir')
p.add_argument('suites', nargs='+')
p.add_argument('--refusals', nargs='*', default=[])
p.add_argument('--refusals-only', nargs='*', default=[])
p.add_argument('--wholefile')
p.add_argument('--c64', nargs='*', default=[])
a = p.parse_args()
out = ROOT / a.outdir
out.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, SKIP_EVIDENCE_RESTORE='1', ASAN_OPTIONS='detect_leaks=0',
           PATH=f'{ROOT}/deps/llvm-mos/bin:{os.environ["PATH"]}')


def run(cmd):
    r = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    if r.returncode:
        sys.exit(f'FAIL: {" ".join(cmd)}\n{r.stdout}{r.stderr}')
    return r.stdout


run(['make', '-s', 'build/lanec-native', 'build/lanec-native-sanitize', 'build/lanec-oracle'])
log, suites = [], {}
for suite in a.suites + a.refusals_only:
    extra = ['--refusals'] if suite in a.refusals else ['--refusals-only'] if suite in a.refusals_only else []
    sanitize = run([sys.executable, 'tools/lanec/diff.py', suite, *extra, '--native', 'build/lanec-native-sanitize'])
    strict = run([sys.executable, 'tools/lanec/diff.py', suite, *extra])
    log += [f'{suite} strict: {strict.strip().splitlines()[-1]}', f'{suite} sanitize: {sanitize.strip().splitlines()[-1]}']
    summary = json.loads((ROOT / f'build/lanec-{suite}-diff.json').read_text())
    if suite in a.refusals_only:
        suites[suite] = summary
        continue
    summary['oracle_crc_sha256'] = hashlib.sha256((ROOT / f'build/lanec-{suite}-oracle.crc').read_bytes()).hexdigest()
    missions = collections.Counter(line.split()[0] for line in run(['build/lanec-native', '--entries', suite]).splitlines())
    summary['accepted_cases_by_mission'] = dict(sorted(missions.items()))
    suites[suite] = summary
    for f in ROOT.glob(f'build/lanec-{suite}-*.txt'):   # the per-case results (the CRCs stay)
        f.unlink()
if a.wholefile:
    log += [line for line in run([sys.executable, a.wholefile, a.outdir]).splitlines() if line.startswith('PASS')]
comparison = {
    'method': 'tools/lanec/diff.py: every case run through the native routines and through the unchanged original '
              'program (pinned ropes) in Virtual AGC; complete results compared (2048 erasable words, '
              'A/L/Q/EB/FB/Z/BB, channels 0-77, superbank, interrupt state). With --refusals, a case the native '
              'routine refuses must leave the image and I/O unchanged and is counted instead of compared.',
    'suites': suites,
    'total_cases': sum(s['cases'] for s in suites.values()),
    'total_differing': sum(s.get('differing', 0) for s in suites.values()),
    'total_refused_unchanged': sum(s.get('refused_unchanged', 0) for s in suites.values()),
}
(out / f'{a.prefix}-comparison.json').write_text(json.dumps(comparison, indent=1) + '\n')
results, prgs = [], {}
for item in a.c64:
    suite, target = item.split('=')
    run(['make', '-s', f'c64-{target}'])
    lines = run(['make', '-s', f'vice-{target}']).strip().splitlines()
    line = ([x for x in lines if x.startswith(('PASS', 'FAIL'))] or lines)[-1]
    if not line.startswith('PASS'):
        sys.exit(f'FAIL: {line}')
    results.append(line)
    log.append(f'{suite} C64: {line}')
    # a target that runs several programs (tools/lanec/vice_many.py) prints each one's PASS line
    for name in [m.group(1) for m in map(re.compile(r'PASS: VICE PAL (\S+\.prg):').match, lines) if m] or [f'{target}.prg']:
        prgs[name] = hashlib.sha256((ROOT / f'build/{name}').read_bytes()).hexdigest()
if a.c64:
    (out / f'{a.prefix}-c64-results.json').write_text(json.dumps(
        {'toolchain': 'llvm-mos SDK (deps/llvm-mos), VICE x64sc PAL stock ROMs (tools/vice_run.py)',
         'results': results, 'prg_sha256': prgs}, indent=1) + '\n')
(out / f'{a.prefix}-checks.log').write_text('\n'.join(log) + '\n')
print('\n'.join(log))
