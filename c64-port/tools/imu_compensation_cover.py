#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Branch-outcome check of the imu-compensation C64 rows.

The C64 diagnostics (platform/c64/imu_compensation_diagnostic.c) run every
37th entry case, every 4th plain Comanche round trip and every 16th plain
Luminary round trip. This builds tools/imu_compensation_cover.c with gcc
--coverage at -O0 on every linked native source (ICP_SRC) and, per mission,
lists the branch outcomes the host stream (every case) takes that those rows
and EXTRA_ROWS do not; `pick` finds extra rows for any such outcome. The
round trips through the Executive do not fit on the C64: outcomes only they
take (NOVAC through BAILOUT, and the transfers into it) are reported, not
required. Writes build/imu-compensation-c64-cover.json and
build/imu-compensation-c64-rows.txt."""
import json, pathlib, re, shutil, subprocess, sys

R = pathlib.Path(__file__).resolve().parents[1]
ICP_SRC = ['src/imu_compensation.c', 'src/agc_image_ops.c', 'src/agc_hardware_divide.c', 'src/lm_interbank_routines.c', 'src/lm_interbank.c', 'src/cm_interbank_routines.c', 'src/cm_interbank_complete.c', 'src/lm_exec_routines.c', 'src/lm_admission_image.c', 'src/lm_waitlist_image.c', 'src/waitlist.c', 'src/executive.c', 'src/p66_native.c', 'src/cm_exec_routines.c', 'src/cm_admission_zero.c', 'src/cm_waitlist_extended.c', 'src/cm_waitlist_internal.c', 'src/cm_task_transfer.c', 'src/cm_native_image.c', 'src/cm_executive.c', 'src/cm_waitlist.c', 'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/imu_compensation_cases.c']
MEASURED = [x for x in ICP_SRC if x.startswith('src/')]
UNMEASURED = [x for x in ICP_SRC if not x.startswith('src/')] + ['tools/imu_compensation_cover.c']
FLAGS = ['-std=c99', '-Iinclude', '-Itests', '-DP66_NATIVE_DISPATCH_ONLY', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections']
MISSIONS = ('Luminary099', 'Comanche055')
ENTRY_CASES, SITES, PLAIN_FIRST, CASES = 20 * 512, 6, 20 * 512 + 6 * 512, 20 * 512 + 6 * 512 + 6 * 128


# Cases picked (python3 tools/imu_compensation_cover.py pick) for the branch
# outcomes the strides miss; the freeze adds them to the C64 diagnostics.
EXTRA_ROWS = [4300, 2727, 13314, 660, 1067, 79, 2431, 1237]


def c64_rows():
    plain = range(PLAIN_FIRST, CASES)
    return EXTRA_ROWS + (list(range(0, ENTRY_CASES, 37)) + [n for n in plain if (n - PLAIN_FIRST) % SITES >= 4][::4]
            + [n for n in plain if (n - PLAIN_FIRST) % SITES < 4][::16])


def build(name):
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
    return d


def measure(d, mission, arg):
    for g in d.glob('*.gcda'):
        g.unlink()
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


def pick(mission, rows, want):
    """Cases the C64 can run (entry and plain) that take the outcomes in want, found by bisection."""
    d = build(f'icp-pick-{mission}')
    f = R / f'build/cover/icp-pick-{mission}.txt'
    def takes(cases, item):
        f.write_text(''.join(f'{n}\n' for n in cases))
        return item[1] in measure(d, mission, str(f))[0].get(item[0], ())
    extra, want = [], set(want)
    while want:
        item = min(want)
        lo = pool()
        assert takes(lo, item), item
        while len(lo) > 1:
            half = lo[:len(lo) // 2]
            lo = half if takes(half, item) else lo[len(lo) // 2:]
        extra.append(lo[0])
        f.write_text(''.join(f'{n}\n' for n in extra))
        got = measure(d, mission, str(f))[0]
        want = {w for w in want if w[1] not in got.get(w[0], ())}
    return extra


def pool():
    """The cases the C64 diagnostics can run: entry cases and plain round trips."""
    return list(range(ENTRY_CASES)) + list(range(PLAIN_FIRST, CASES))


def streams(cm):
    """Outcomes of the whole host stream, of its C64-runnable cases, and of the C64 rows."""
    files = R / 'build/cover'
    files.mkdir(parents=True, exist_ok=True)
    (files / 'icp-pool.txt').write_text(''.join(f'{n}\n' for n in pool()))
    rows = R / 'build/imu-compensation-c64-rows.txt'
    rows.write_text(''.join(f'{n}\n' for n in c64_rows()))
    d = build(f'icp-{cm}')
    host, total, never = measure(d, cm, '-')
    runnable, _, _ = measure(d, cm, str(files / 'icp-pool.txt'))
    c64, _, _ = measure(d, cm, str(rows))
    return host, runnable, c64, total, never


def main():
    count = lambda t: sum(len(v) for v in t.values())
    flat = lambda t: {(l, b) for l, v in t.items() for b in v}
    result = dict(sources=MEASURED, measure='gcc gcov -O0 branch outcomes per mission over every linked native source, host build',
                  c64_rows=len(c64_rows()), missions={})
    fails = []
    for cm, name in enumerate(MISSIONS):
        host, runnable, c64, total, never = streams(cm)
        missing = sorted(flat(runnable) - flat(c64))
        trips_only = sorted(flat(host) - flat(runnable))
        result['missions'][name] = dict(outcomes=total, host_stream_taken=count(host), c64_rows_taken=count(c64),
                                        host_only=[f'{l[0]}:{l[1]}:{b}' for l, b in missing],
                                        executive_round_trips_only=[f'{l[0]}:{l[1]}:{b}' for l, b in trips_only],
                                        never_taken_on_host=never)
        if missing:
            fails.append(f'{name}: {len(missing)} ({result["missions"][name]["host_only"]})')
    (R / 'build/imu-compensation-c64-cover.json').write_text(json.dumps(result, indent=1) + '\n')
    if fails:
        raise SystemExit('FAIL C64 rows miss branch outcomes the host stream takes: ' + '; '.join(fails))
    print('PASS C64 rows take every branch outcome the host stream takes outside the round trips through the '
          'Executive, per mission: ' + ', '.join(
              f'{n} {m["c64_rows_taken"]} of {m["outcomes"]} ({m["host_stream_taken"]} on the host, '
              f'{len(m["executive_round_trips_only"])} only in the round trips through the Executive)'
              for n, m in result['missions'].items()))


if __name__ == '__main__':
    if sys.argv[1:2] == ['pick']:
        for cm in map(int, sys.argv[2:] or (0, 1)):
            host, runnable, c64, _, _ = streams(cm)
            want = [(l, b) for l, v in runnable.items() for b in v if b not in c64.get(l, ())]
            print(MISSIONS[cm], pick(cm, c64_rows(), want))
    else:
        sys.exit(main())
