#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Branch outcomes of src/lm_imu_performance.c (gcc gcov, -O0) taken by the
whole host stream (every case, every routine key) and by the C64 rows
(build/lm-imu-performance-c64-rows.txt from tools/lm_imu_performance_freeze.py:
the rows each C64 part runs, replayed on the host through the part-0 build,
which stops each part's chained steps where the part build does). Every
outcome the host stream takes must also be taken by the C64 rows. Writes
build/lm-imu-performance-c64-cover.json."""
import json, pathlib, re, shutil, subprocess, sys

R = pathlib.Path(__file__).resolve().parents[1]
REST = ['src/sband_trig.c', 'src/guidance_trig.c', 'src/guidance_product.c', 'src/lm_interp_orders.c',
        'src/lm_interp_steps.c', 'src/agc_image_ops.c', 'src/agc_hardware_divide.c', 'src/agc_transfer.c',
        'src/agc_word.c', 'src/agc_multiword.c', 'tests/lm_imu_performance_cases.c', 'tools/lm_imu_performance_cover.c']
SRC = (R / 'src/lm_imu_performance.c').read_text().splitlines()
FLAGS = ['-std=c99', '-Iinclude', '-Itests', '-DP66_NATIVE_DISPATCH_ONLY', '-DIPT_C64_PART=0']


def taken(name, arg):
    d = R / 'build/cover' / name
    if d.exists():
        shutil.rmtree(d)
    d.mkdir(parents=True)
    subprocess.run(['gcc', *FLAGS, '-O0', '--coverage', '-c', str(R / 'src/lm_imu_performance.c'),
                    '-o', str(d / 'lm_imu_performance.o')], cwd=R, check=True)
    subprocess.run(['gcc', *FLAGS, '-O2', '--coverage', str(d / 'lm_imu_performance.o'), *REST,
                    '-o', str(d / 'run')], cwd=R, check=True)
    subprocess.run([str(d / 'run'), 'replay', arg], cwd=R, check=True)
    subprocess.run(['gcov', '-b', '-c', '-o', str(d), str(R / 'src/lm_imu_performance.c')], cwd=d, check=True,
                   stdout=subprocess.DEVNULL)
    out, total, line, never = {}, 0, None, []
    for l in (d / 'lm_imu_performance.c.gcov').read_text().splitlines():
        m = re.match(r'\s*(?:[-#=0-9*]+):\s*(\d+):', l)
        if m:
            line = int(m.group(1))
            continue
        m = re.match(r'branch\s+(\d+)\s+(taken (\d+)|never executed)', l)
        if m:
            total += 1
            if m.group(3) and int(m.group(3)) > 0:
                out.setdefault(line, set()).add(int(m.group(1)))
            else:
                never.append(f'{line}:{m.group(1)} {SRC[line - 1].strip()[:100]}')
    return out, total, never


def main():
    rows = R / 'build/lm-imu-performance-c64-rows.txt'
    host, total, never = taken('ipt-host', '-')
    c64, _, _ = taken('ipt-c64', str(rows))
    count = lambda t: sum(len(v) for v in t.values())
    missing = sorted((l, b) for l, v in host.items() for b in v if b not in c64.get(l, ()))
    result = dict(source='src/lm_imu_performance.c', measure='gcc gcov -O0 branch outcomes, part-0 build',
                  c64_rows=len(rows.read_text().splitlines()), outcomes=total, host_stream_taken=count(host),
                  c64_rows_taken=count(c64), host_only=[f'{l}:{b}' for l, b in missing],
                  never_taken_on_host=never)
    (R / 'build/lm-imu-performance-c64-cover.json').write_text(json.dumps(result, indent=1) + '\n')
    if missing:
        raise SystemExit(f"FAIL C64 rows miss {len(missing)} branch outcomes the host stream takes: {result['host_only']}")
    print(f'PASS C64 rows take every branch outcome the host stream takes: {count(c64)} of {total} outcomes '
          f'({count(host)} on the host)')


if __name__ == '__main__':
    sys.exit(main())
