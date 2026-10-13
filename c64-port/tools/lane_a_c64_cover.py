#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""C64 rows for lane A's kernel milestones, chosen by host branch outcomes.

Each milestone's C64 diagnostic already runs every STRIDE-th case. This adds
the fewest further cases (tools/c64_row_cover.py) that make the C64 rows take
every branch arc the whole host corpus takes, in every source the C64 program
links (gcov -b on a host build of those sources with the C64 defines). It
writes tests/<m>_c64_rows.h (the extra cases and their native digests, which
the host tests prove equal to yaAGC's) and evidence/<m>/<m>-c64-cover.json.

  python3 tools/lane_a_c64_cover.py lm-t6rupt [--jobs N]
  python3 tools/lane_a_c64_cover.py --check lm-t6rupt   # re-trace, check the rows
"""
import argparse
import json
import os
import pathlib
import re
import subprocess
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
import c64_row_cover  # noqa: E402

# name: (case header, digest(n) as C, [(mission, cases)], sources variable,
#        flags variable or defines, symbol prefix, c64 vectors header, stride,
#        stride counts per space)
M = {
    'lm-exec-jobs': ('lm_exec_jobs_cases.h', 'lm_exj_digest(&image,n)', 'LM_EXJ_SRC', '-DP66_NATIVE_DISPATCH_ONLY',
                     'lm_exj', 'LM_EXJ'),
    'lm-exec-services': ('lm_exec_services_cases.h', 'lm_exs_digest(&image,n)', 'LM_EXS_SRC',
                         '-DP66_NATIVE_DISPATCH_ONLY', 'lm_exs', 'LM_EXS'),
    'lm-exec-idle': ('lm_exec_idle_cases.h', 'lm_exi_digest(&image,n)', 'LM_EXI_SRC', '$(LM_EXI_FLAGS)', 'lm_exi',
                     'LM_EXI'),
    'cm-exec-services': ('cm_exec_services_cases.h', 'cm_exs_digest(&image,n)', 'CM_EXS_SRC', '$(CM_EXS_FLAGS)',
                         'cm_exs', 'CM_EXS'),
    'cm-exec-jobs': ('cm_exec_jobs_cases.h', 'cm_exj_digest(&image,n)', 'CM_EXJ_SRC', '$(CM_EXJ_FLAGS)', 'cm_exj',
                     'CM_EXJ'),
    'cm-exec-idle': ('cm_exec_idle_cases.h', 'cm_exi_digest(&image,n)', 'CM_EXI_SRC', '$(CM_EXI_FLAGS)', 'cm_exi',
                     'CM_EXI'),
    'service-transfer': ('service_transfer_cases.h', 'svt_digest(&image,n)', 'SVT_SRC', '', 'svt', 'SVT'),
    'interbank-oracle': ('interbank_oracle_cases.h', 'ibo_digest(&image,n)', 'IBO_SRC', '', 'ibo', 'IBO'),
    'standard-lead-oracle': ('standard_lead_oracle_cases.h', 'slo_digest(&image,n)', 'SLO_SRC', '', 'slo', 'SLO'),
    'lm-t6rupt': ('lm_t6rupt_cases.h', 'lm_t6_digest(&image,n)', 'LM_T6_SRC', '$(LM_T6_FLAGS)', 'lm_t6', 'LM_T6'),
    # Two missions: case n < LM count is Luminary's case n, else Comanche's n - LM count.
    'rupt-lead-ins': ('rupt_lead_ins_cases.h', 'n<RLI_LM_CASES?rli_digest(0,&image,n):rli_digest(1,&image,'
                      '(uint16_t)(n-RLI_LM_CASES))', 'RLI_SRC', '$(RLI_FLAGS)', 'rli', 'RLI'),
    'keyrupt-uprupt': ('keyrupt_uprupt_cases.h', 'n<KU_CASES?ku_digest(0,&image,n):ku_digest(1,&image,'
                       '(uint16_t)(n-KU_CASES))', 'KU_SRC', '$(KU_FLAGS)', 'ku', 'KU'),
    # Whole T4RUPT interrupts: the diagnostic runs a picked case list (lch_c64_case), not a stride.
    'lm-t4rupt-chain': ('lm_t4rupt_chain_cases.h', 'lch_digest(&image,n)', 'LCH_SRC', '$(LCH_FLAGS)', 'lch', 'LCH'),
}
TWO = {'rupt-lead-ins': ('RLI_LM_CASES', 'RLI_CM_CASES'), 'keyrupt-uprupt': ('KU_CASES', 'KU_CASES')}


def make_var(expr):
    out = subprocess.run(['make', '-s', '--no-print-directory', '--eval', f'print-lane-a-var: ; @echo {expr}',
                          'print-lane-a-var'], cwd=R, check=True, capture_output=True, text=True,
                         env=dict(os.environ, SKIP_EVIDENCE_RESTORE='1')).stdout
    return out.split()


def header_int(path, name):
    m = re.search(rf'#define {name} (\d+)u', (R / path).read_text())
    return int(m.group(1)) if m else None


def stride_rows(name, prefix):
    """The cases the diagnostic's stride loops already run."""
    vec = f'tests/{name.replace("-", "_")}_c64_vectors.h'
    listed = re.search(rf'{M[name][4]}_c64_case\[\d+\]=\{{([^}}]*)\}}', (R / vec).read_text())
    if listed:  # a picked case list instead of a stride
        return None, [int(x) for x in re.findall(r'(\d+)u', listed.group(1))]
    stride = header_int(vec, f'{prefix}_C64_STRIDE')
    if name in TWO:
        lm, cm = (header_int(f'tests/{M[name][0]}', TWO[name][0]), header_int(f'tests/{M[name][0]}', TWO[name][1]))
        lmc, cmc = header_int(vec, f'{prefix}_LM_C64_COUNT'), header_int(vec, f'{prefix}_CM_C64_COUNT')
        return lm + cm, [k * stride for k in range(lmc)] + [lm + k * stride for k in range(cmc)]
    count = header_int(vec, f'{prefix}_C64_COUNT')
    cases = None
    if name == 'lm-exec-jobs':  # three programs, each from its part's first case
        cases = header_int(f'tests/{M[name][0]}', f'{prefix}_CASES')
        return cases, list(range(0, cases, stride))
    return cases, [k * stride for k in range(count)]


def count_expr(name):
    prefix = M[name][5]
    return '+'.join(TWO[name]) if name in TWO else f'{prefix}_CASES'


def runner_source(name):
    hdr, digest = M[name][0], M[name][1]
    return (f'/* generated by tools/lane_a_c64_cover.py */\n#include <stdio.h>\n#include <stdlib.h>\n'
            f'#include "{hdr}"\nstatic apollo_overlay_state image;\n'
            f'int main(int argc,char **argv){{uint16_t n;(void)argc;'
            f'if(argv[1][0]==\'-\'){{printf("%u\\n",(unsigned)({count_expr(name)}));return 0;}}'
            f'n=(uint16_t)atol(argv[1]);'
            f'printf("%08lx\\n",(unsigned long)({digest}));return 0;}}\n')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('milestone', choices=sorted(M))
    ap.add_argument('--jobs', type=int)
    ap.add_argument('--check', action='store_true')
    a = ap.parse_args()
    name = a.milestone
    under = name.replace('-', '_')
    prefix_c, prefix = M[name][4], M[name][5]
    objdir = R / f'build/{name}-cover'
    sources = sorted(set(s for s in make_var(f'$({M[name][2]})') if s.endswith('.c')))
    flags = M[name][3]
    defines = [f for f in make_var(flags) if f.startswith('-D')] if flags.startswith('$') else flags.split()
    defines = [d for d in defines if 'C64' not in d and 'PART' not in d]
    cppflags = [f for f in make_var('$(CPPFLAGS)') if f.startswith(('-I', '-D'))]
    objdir.mkdir(parents=True, exist_ok=True)
    main_c = R / f'build/{name}-cover-main.c'
    main_c.write_text(runner_source(name))
    exe = c64_row_cover.build(objdir, sources + [str(main_c.relative_to(R))], (*cppflags, *defines, '-w', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections'))
    cases, seed = stride_rows(name, prefix)
    total = int(subprocess.run([str(exe), '-n'], cwd=R, check=True, capture_output=True, text=True,
                               env=dict(os.environ, GCOV_PREFIX='/tmp/lane-a-cover-discard')).stdout)
    assert cases in (None, total), (cases, total)
    cases = total
    trace_path = R / f'build/{name}-cover-trace.json'
    if trace_path.exists() and not a.check:
        trace = c64_row_cover.load(trace_path)
    else:
        trace = c64_row_cover.trace(objdir, range(cases), lambda n: [str(exe), str(n)], jobs=a.jobs)
        c64_row_cover.save(trace_path, trace)
    rows_h = R / f'tests/{under}_c64_rows.h'
    if a.check:
        extra = [int(x) for x in re.findall(r'\b(\d+)u', rows_h.read_text().split('_row[')[1].split('}')[0])]
        c64_row_cover.check(trace, sorted(set(seed) | set(extra)))
        print(f'{name}: {len(seed)} stride rows and {len(extra)} cover rows take every host branch arc')
        return
    rows = c64_row_cover.cover(trace, seed=seed)
    extra = sorted(set(rows) - set(seed))
    digests = [subprocess.run([str(exe), str(n)], cwd=R, check=True, capture_output=True, text=True,
                              env=dict(os.environ, GCOV_PREFIX='/tmp/lane-a-cover-discard')).stdout.strip()
               for n in extra]
    body = ',\n'.join(','.join(f'{n}u' for n in extra[i:i + 12]) for i in range(0, len(extra), 12)) or '0u'
    dbody = ',\n'.join(','.join(f'0x{d}ul' for d in digests[i:i + 6]) for i in range(0, len(digests), 6)) or '0ul'
    k = len(extra)
    rows_h.write_text(
        f'/* SPDX-License-Identifier: MIT; generated by tools/lane_a_c64_cover.py: the cases the C64\n'
        f' * diagnostic runs besides its {prefix}_C64 rows, so its rows take every branch arc\n'
        f' * (gcov -b, all linked sources) the {cases} host cases take; native digests, equal to\n'
        f' * yaAGC\'s by the host test. */\n'
        f'#define {prefix}_C64_ROWS {k}u\n'
        f'static const uint16_t {prefix_c}_c64_row[{max(k, 1)}]={{\n{body}}};\n'
        f'static const uint32_t {prefix_c}_c64_row_digest[{max(k, 1)}]={{\n{dbody}}};\n')
    host = frozenset().union(*trace.values())
    record = {
        'milestone': name, 'tool': 'tools/c64_row_cover.py (via tools/lane_a_c64_cover.py)',
        'reproduce': f'python3 tools/lane_a_c64_cover.py --check {name}',
        'linked_sources': sources, 'defines': defines,
        'host_cases': cases, 'host_branch_arcs': len(host),
        'c64_rows': len(seed) + k, 'stride_rows': len(seed), 'cover_rows': k, 'cover_cases': extra,
        'c64_branch_arcs': len(host) - len(c64_row_cover.missing(trace, rows)),
        'host_branches_missing_on_c64': c64_row_cover.missing(trace, rows),
    }
    out = R / f'build/{name}-c64-cover.json'
    out.write_text(json.dumps(record, indent=2) + '\n')
    print(f'{name}: {len(seed)} stride rows + {k} cover rows take all {len(host)} host branch arcs; '
          f'wrote {rows_h.relative_to(R)} and {out.relative_to(R)}')


if __name__ == '__main__':
    main()
