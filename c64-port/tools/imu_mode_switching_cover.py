#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""C64 rows of imu-mode-switching by outcome, and their branch-outcome check.

select_rows() (used by tools/imu_mode_switching_freeze.py) builds
tools/imu_mode_switching_cover.c with gcc trace-pc and with clang edge coverage
on src/imu_mode_switching.c and returns the cases that, per mission, take
every edge (of either build), routine, call site and outcome kind the host
stream takes.

Run as a script, it checks build/imu-mode-switching-c64-rows.txt (written by
the freeze: the cases the C64 diagnostics run) with gcc gcov at -O0: for each
mission, every branch outcome of src/imu_mode_switching.c the host stream
takes in that mission must be taken by the mission's C64 rows. Writes
build/imu-mode-switching-c64-cover.json."""
import json, pathlib, re, shutil, subprocess, sys

R = pathlib.Path(__file__).resolve().parents[1]
MAIN = 'src/imu_mode_switching.c'
REST = ['src/agc_image_ops.c', 'src/lm_interbank_routines.c', 'src/lm_interbank.c', 'src/cm_interbank_routines.c',
        'src/cm_interbank_complete.c', 'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c',
        'src/agc_hardware_divide.c', 'tests/imu_mode_switching_cases.c', 'tools/imu_mode_switching_cover.c']
# Every linked native source is measured, not only the file's own.
MEASURED = [MAIN] + [x for x in REST if x.startswith('src/')]
UNMEASURED = [x for x in REST if not x.startswith('src/')]
FLAGS = ['-std=c99', '-Iinclude', '-Itests', '-DP66_NATIVE_DISPATCH_ONLY']
SRC = (R / MAIN).read_text().splitlines()
MISSIONS = ('Luminary099', 'Comanche055')


def select_rows():
    # gcc's trace-pc does not split critical edges (a switch case's break) and
    # clang's edge coverage folds constant ternaries into selects, so each
    # misses some gcov outcomes; the union of both selections takes them all.
    rows = set()
    for cc, cov in (('gcc', 'trace-pc'), ('clang', 'edge,trace-pc')):
        exe = R / f'build/imu-mode-switching-select-{cc}'
        objs = []
        for src in MEASURED:
            o = f'build/ims-select-{cc}-{pathlib.Path(src).stem}.o'
            subprocess.run([cc, *FLAGS, '-O0', f'-fsanitize-coverage={cov}', '-c', src, '-o', o], cwd=R, check=True)
            objs.append(o)
        subprocess.run(['gcc', *FLAGS, '-O2', *objs, *UNMEASURED, '-o', str(exe)], cwd=R, check=True)
        p = subprocess.run([str(exe), 'select'], cwd=R, check=True, capture_output=True, text=True)
        assert p.stderr.strip().splitlines()[-1].endswith(' 0 items left'), p.stderr
        rows |= {int(x) for x in p.stdout.split()}
    return sorted(rows)


def taken(name, mission, arg):
    d = R / 'build/cover' / name
    if d.exists():
        shutil.rmtree(d)
    d.mkdir(parents=True)
    objs = []
    for src in MEASURED:
        o = str(d / (pathlib.Path(src).stem + '.o'))
        subprocess.run(['gcc', *FLAGS, '-O0', '--coverage', '-c', str(R / src), '-o', o], cwd=R, check=True)
        objs.append(o)
    subprocess.run(['gcc', *FLAGS, '-O2', *objs, *UNMEASURED, '--coverage', '-o', str(d / 'run')],
                   cwd=R, check=True)
    subprocess.run([str(d / 'run'), 'replay', str(mission), arg], cwd=R, check=True)
    out, total, never = {}, 0, []
    for src in MEASURED:
        subprocess.run(['gcov', '-b', '-c', '-o', str(d), str(R / src)], cwd=d, check=True, stdout=subprocess.DEVNULL)
        text = (R / src).read_text().splitlines()
        line = None
        for l in (d / (pathlib.Path(src).name + '.gcov')).read_text().splitlines():
            m = re.match(r'\s*(?:[-#=0-9*]+):\s*(\d+):', l)
            if m:
                line = int(m.group(1))
                continue
            m = re.match(r'branch\s+(\d+)\s+(taken (\d+)|never executed)', l)
            if m:
                total += 1
                if m.group(3) and int(m.group(3)) > 0:
                    out.setdefault((src, line), set()).add(int(m.group(1)))
                else:
                    never.append(f'{src}:{line}:{m.group(1)} {text[line - 1].strip()[:100]}')
    return out, total, never


def main():
    rows = R / 'build/imu-mode-switching-c64-rows.txt'
    count = lambda t: sum(len(v) for v in t.values())
    result = dict(sources=MEASURED, measure='gcc gcov -O0 branch outcomes per mission over every linked native source, host build',
                  c64_rows=len(rows.read_text().split()), missions={})
    fails = []
    for cm, name in enumerate(MISSIONS):
        host, total, never = taken(f'ims-host-{cm}', cm, '-')
        c64, _, _ = taken(f'ims-c64-{cm}', cm, str(rows))
        missing = sorted((l, b) for l, v in host.items() for b in v if b not in c64.get(l, ()))
        result['missions'][name] = dict(outcomes=total, host_stream_taken=count(host), c64_rows_taken=count(c64),
                                        host_only=[f'{l[0]}:{l[1]}:{b}' for l, b in missing], never_taken_on_host=never)
        if missing:
            fails.append(f'{name}: {len(missing)} ({result["missions"][name]["host_only"]})')
    (R / 'build/imu-mode-switching-c64-cover.json').write_text(json.dumps(result, indent=1) + '\n')
    if fails:
        raise SystemExit('FAIL C64 rows miss branch outcomes the host stream takes: ' + '; '.join(fails))
    print('PASS C64 rows take every branch outcome the host stream takes, per mission: ' + ', '.join(
        f'{n} {m["c64_rows_taken"]} of {m["outcomes"]} ({m["host_stream_taken"]} on the host)'
        for n, m in result['missions'].items()))


if __name__ == '__main__':
    sys.exit(main())
