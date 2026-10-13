#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Count flight-typical edges in lane A's kernel case corpora.

For each milestone, builds platform/host/lane_a_edge_scan.c with that
milestone's sources and counts, over the inputs its cases start from: TIME3
within 8 cs of overflow, TIME6 1-3 counts from its end, Waitlist tasks due
together or 1 cs apart, a full Waitlist, NEWJOB +0 and -0, equal busy
priorities, and no free core set or VAC area. Every case is compared with
yaAGC by the milestone's own differential; this shows which edges they hold.

  python3 tools/lane_a_edge_survey.py [--out build/kernel-edges.json]
"""
import argparse
import json
import os
import pathlib
import subprocess

R = pathlib.Path(__file__).resolve().parents[1]
M = {'lm-exec-jobs': ('LM_EXJ', 'LM_EXJ_SRC'), 'lm-exec-services': ('LM_EXS', 'LM_EXS_SRC'),
     'lm-exec-idle': ('LM_EXI', 'LM_EXI_SRC'), 'cm-exec-services': ('CM_EXS', 'CM_EXS_SRC'),
     'cm-exec-jobs': ('CM_EXJ', 'CM_EXJ_SRC'), 'cm-exec-idle': ('CM_EXI', 'CM_EXI_SRC'),
     'lm-t6rupt': ('LM_T6', 'LM_T6_SRC')}


def make_var(expr):
    return subprocess.run(['make', '-s', '--no-print-directory', '--eval', f'print-lane-a-var: ; @echo {expr}',
                           'print-lane-a-var'], cwd=R, check=True, capture_output=True, text=True,
                          env=dict(os.environ, SKIP_EVIDENCE_RESTORE='1')).stdout.split()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--out', default='build/kernel-edges.json')
    a = ap.parse_args()
    cpp = [f for f in make_var('$(CPPFLAGS)') if f.startswith(('-I', '-D'))]
    out = {}
    for name, (key, var) in M.items():
        src = sorted(set(s for s in make_var(f'$({var})') if s.endswith('.c')))
        exe = R / f'build/edge-scan-{name}'
        subprocess.run(['cc', '-O1', '-w', *cpp, '-DP66_NATIVE_DISPATCH_ONLY', '-ffunction-sections',
                        '-Wl,--gc-sections', f'-DM_{key}', *src, 'platform/host/lane_a_edge_scan.c', '-o', str(exe)],
                       cwd=R, check=True)
        out[name] = json.loads(subprocess.run([str(exe)], cwd=R, check=True, capture_output=True, text=True).stdout)
        print(name, out[name])
    (R / a.out).write_text(json.dumps(out, indent=2) + '\n')


if __name__ == '__main__':
    main()
