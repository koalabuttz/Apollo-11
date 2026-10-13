#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Choose the C64 rows of a Lane F module by outcome.

Runs every host case of the module with its path counters on and every
src/ file it links built with clang -fsanitize-coverage=edge,trace-pc-guard
(tools/lane_f_path_trace.c), then picks the C64 rows: first, greedily, cases
until every entry slot, outcome kind, counted path and control-flow edge the
host cases take is taken by some row, pruning rows the others make
redundant; then evenly spread cases (a stride of 97, which shares no factor
with any module's entry count) up to the module's row budget. A module built
as several PRGs or segment groups is picked per build, from the cases that
build runs as the whole module does. Writes tests/<module>_c64_vectors.h
(the case numbers and their frozen digests from tests/<module>_vectors.h)
and build/<milestone>-c64-cover.json (per slot, outcome kind and path or
edge: host cases and C64 rows taking it). Fails if a host path, edge, slot
or outcome kind is missing from the rows.

  python3 tools/lane_f_c64_rows.py lm-p-axis
"""
import json
import pathlib
import re
import subprocess
import sys

R = pathlib.Path(__file__).resolve().parents[1]
B = R / 'build'
STRIDE = 97
# The host clang (deps/llvm-mos/bin, often first on PATH, has its own clang).
CLANG = '/usr/bin/clang'
# milestone: (file stem, macro prefix, symbol prefix, entry slots, row budget,
#             compile flags, run expression)
IO_RUN = '{s}_run(s,io,n,steps)'
MODULES = {
    'lm-p-axis': ('lm_p_axis', 'LM_PA', 'lm_pa', 16, 128, 'LM_PA_SRC', IO_RUN),
    'lm-q-r-axis': ('lm_q_r_axis', 'LM_QR', 'lm_qr', 32, 128, 'LM_QR_SRC', IO_RUN),
    'lm-throttle': ('lm_throttle', 'LM_THR', 'lm_thr', 4, 256, 'LM_THR_SRC', IO_RUN),
    'lm-tjet-law': ('lm_tjet_law', 'LM_TJ', 'lm_tj', 3, 256, 'LM_TJ_SRC', '{s}_run(s,n,steps)'),
    'lm-dapidler': ('lm_dapidler', 'LM_DI', 'lm_di', 9, 192, 'LM_DI_SRC', IO_RUN),
    'lm-trim-gimbal': ('lm_trim_gimbal', 'LM_TG', 'lm_tg', 4, 256, 'LM_TG_SRC', IO_RUN),
    'lm-landing-displays': ('lm_landing_displays', 'LM_LAD', 'lm_lad', 8, 128, 'LM_LAD_SRC', IO_RUN),
    'lm-ags-init': ('lm_ags_init', 'AGS', 'ags', 39, 128, 'LM_AGS_SRC', '{s}_run(s,n,steps)'),
    'lm-lunar-landing': ('lm_lunar_landing', 'LLD', 'lld', 90, 120, 'LM_LLD_SRC', '{s}_run(s,n,0)'),
    'lm-burn-baby': ('lm_burn_baby', 'LMBB', 'lmbb', 174, 224, 'LM_BB_SRC', '{s}_run(s,n,0)'),
    'lm-servicer': ('lm_servicer', 'LMSV', 'lmsv', 315, 224, 'LM_SV_SRC', '{s}_run(s,n,0)'),
}



def burn_baby_group(slot):
    """lm-burn-baby's PRG of a routine slot: 2 for the bank-17 and bank-35
    keys, 1 for the rest (platform/c64/lm_burn_baby_diagnostic.c)."""
    src = (R / 'src/lm_burn_baby.c').read_text()
    body = re.search(r'lmbb_routine_keys\[LMBB_ROUTINES\]=\{(.*?)\};', src, re.S).group(1)
    banks = re.findall(r'K(\d\d|FF)\(', body)
    return 2 if banks[slot] in ('17', '35') else 1


def lunar_landing_group(slot):
    """lm-lunar-landing's PRG of a routine slot: 2 for DDUMCALC (32,3102)
    and the order pairs src/lm_lunar_landing.c keeps under LLD_G2, 1 for
    the rest (platform/c64/lm_lunar_landing_diagnostic.c)."""
    src = (R / 'src/lm_lunar_landing.c').read_text()
    body = re.search(r'lld_routine_keys\[LLD_ROUTINES\]=\{(.*?)\};', src, re.S).group(1)
    b, a = re.findall(r'B(3[12])\((0[0-7]+)\)', body)[slot]
    a = int(a, 8)
    heavy = b == '32' and (a == 0o3102 or 0o3104 <= a <= 0o3143 or 0o3163 <= a <= 0o3201 or 0o3215 <= a <= 0o3222)
    return 2 if heavy else 1


# Modules built as several PRGs: (group macro, slot -> group). Each PRG gets
# only its own rows.
GROUPS = {'lm-burn-baby': ('LMBB_C64_GROUP', burn_baby_group),
          'lm-lunar-landing': ('LLD_C64_GROUP', lunar_landing_group)}


def servicer_slots():
    """lm-servicer's C64 segment group of each key slot: the SV_G(n) block
    of src/lm_servicer.c's routine tables holding the key's entry."""
    src = (R / 'src/lm_servicer.c').read_text()
    of, g = {}, None
    for name in ('basic_entries', 'interp_entries'):
        body = re.search(rf'{name}\[\]=\{{\n(.*?)\n\}};', src, re.S).group(1)
        for line in body.splitlines():
            m = re.match(r'#if SV_G\((\d+)\)$', line)
            if m:
                g = int(m.group(1))
            for b, a in re.findall(r'R(\d\d)\((0[0-7]+)\)', line):
                of[f'{b},{a}'] = g
    body = re.search(r'lmsv_routine_keys\[LMSV_ROUTINES\]=\{(.*?)\};', src, re.S).group(1)
    return [of[f'{b},{a}'] for b, a in re.findall(r'K(\d\d)\((0[0-7]+)\)', body)]


# Modules built as C64 segment groups (src/lm_servicer.c): each PRG holds
# part of the code and its run of a case ends before the first instruction
# outside it. Per group: (group macro, slot -> group, digest file of the
# group's segment cases from the module's freeze). Each group's rows must
# take every slot, outcome, branch and edge that group's host build
# takes on its cases, and the groups' rows together every branch of the
# whole-file host cases.
SEGMENTS = {'lm-servicer': ('LMSV_C64_GROUP', servicer_slots, 'lm-servicer-g{}-digests.txt')}


def mk_var(milestone, name):
    text = (R / f'tools/mk/{milestone}.mk').read_text()
    return re.search(rf'^{name}\s*=\s*(.*)$', text, re.M).group(1).split()


def table(ctype, name, values, fmt):
    out = [f'static const {ctype} {name}[{len(values)}]={{']
    for i in range(0, len(values), 8):
        out.append(''.join(fmt.format(v) for v in values[i:i + 8]))
    out.append('};')
    return out


def trace(milestone, npaths, extra=(), tag=''):
    """Every host case of the module (built with `extra`): (case, kind,
    counted paths and control-flow edges of every linked source taken)."""
    stem, mac, sym, slots, budget, srcvar, run = MODULES[milestone]
    exe = B / f'{milestone}-path-trace{tag}'
    defs = [f'-DTRACE_HDR="{stem}_cases.h"', f'-DTRACE_PATHS={sym}_paths', f'-DTRACE_NPATHS={npaths}u',
            f'-DTRACE_CASES={mac}_CASES', f'-DTRACE_RUN(s,io,n,steps)={run.format(s=sym)}',
            f'-D{mac}_COUNT_PATHS', '-DP66_NATIVE_DISPATCH_ONLY', '-ffunction-sections',
            '-fdata-sections', '-Wl,--gc-sections', *extra]
    # Every linked source with clang edge coverage: every branch outcome the
    # host cases take in any of them must be taken by a C64 row
    # (tools/lane_f_path_trace.c).
    objs = [src for src in mk_var(milestone, srcvar) if not src.startswith('src/')]
    for src in mk_var(milestone, srcvar):
        if not src.startswith('src/'):
            continue
        obj = B / f'{milestone}-path-trace{tag}-{pathlib.Path(src).stem}.o'
        subprocess.run([CLANG, '-std=c99', '-O0', '-w', '-Iinclude', '-Itests', '-Isrc', *defs,
                        '-fsanitize-coverage=edge,trace-pc-guard', '-c', src, '-o', str(obj)], cwd=R, check=True)
        objs.append(str(obj))
    # (tests/ sources, the case generators, stay uninstrumented.) At -O0 the
    # calls -DP66_NATIVE_DISPATCH_ONLY makes dead stay in the objects: their
    # targets get stubs that abort, so a call that is reached after all fails.
    link = [CLANG, '-std=c99', '-O2', '-w', '-Iinclude', '-Itests', '-Isrc', *defs, *objs,
            'tools/lane_f_path_trace.c', '-o', str(exe)]
    stub = B / f'{milestone}-path-trace{tag}-stubs.c'
    stub.write_text('')
    first = subprocess.run(link, cwd=R, capture_output=True, text=True)
    dead = sorted(set(re.findall(r"undefined reference to `(\w+)'", first.stderr)))
    if first.returncode and not dead:
        sys.exit(first.stderr)
    if dead:
        stub.write_text('#include <stdlib.h>\n' + ''.join(f'void {f}(void);void {f}(void){{abort();}}\n' for f in dead))
        subprocess.run(link + [str(stub)], cwd=R, check=True)
    rows = []
    for line in subprocess.run([str(exe)], cwd=R, check=True, capture_output=True, text=True).stdout.splitlines():
        v = list(map(int, line.split()))
        rows.append((v[0], v[1], set(v[2:])))
    return rows


def cover(rows, slots):
    """Cases taking every slot, kind and branch of `rows`: a greedy cover,
    then (latest pick first) each row dropped whose elements the others
    still take."""
    def elements(r):
        return {('slot', r[0] % slots), ('kind', r[1])} | {('path', p) for p in r[2]}
    need = set().union(*(elements(r) for r in rows))
    chosen, have = [], set()
    while have != need:
        best = max(rows, key=lambda r: (len(elements(r) - have), -r[0]))
        chosen.append(best)
        have |= elements(best)
    count = {}
    for r in chosen:
        for e in elements(r):
            count[e] = count.get(e, 0) + 1
    for r in reversed(list(chosen)):
        if all(count[e] > 1 for e in elements(r)):
            chosen.remove(r)
            for e in elements(r):
                count[e] -= 1
    return [r[0] for r in chosen]


def pick(rows, slots, budget, cases):
    """Greedy cover of every slot, kind and branch of `rows`, then a stride
    fill (over the cases in `rows`) up to `budget`."""
    chosen = cover(rows, slots)
    picked, mine, i = set(chosen), {r[0] for r in rows}, 0
    while len(picked) < min(budget, len(mine)):
        if i * STRIDE % cases in mine:
            picked.add(i * STRIDE % cases)
        i += 1
    return chosen, sorted(picked)


def segments(milestone, npaths, cases, full, out):
    """The C64 rows of a module built as segment groups (SEGMENTS)."""
    stem, mac, sym, slots, budget, srcvar, run = MODULES[milestone]
    macro, slot_groups, digest_file = SEGMENTS[milestone]
    of = slot_groups()
    host = {p for r in full for p in r[2] if p < npaths}
    c64, report, total = set(), {}, 0
    for g in sorted(set(of)):
        rows = [r for r in trace(milestone, npaths, (f'-D{macro}={g}',), f'-g{g}') if of[r[0] % slots] == g]
        share = max(1, budget * sum(1 for x in of if x == g) // slots)
        chosen, picked = pick(rows, slots, share, cases)
        digests = {}
        for line in (B / digest_file.format(g)).read_text().split('\n'):
            if line:
                n, d = line.split()
                digests[int(n)] = int(d, 16)
        assert {r[0] for r in rows} == set(digests), f'{milestone} group {g}: digests and cases differ'
        out += [f'#if {macro}=={g}', f'#define {mac}_C64_COUNT {len(picked)}u',
                *table('uint16_t', f'{sym}_c64_case', picked, '{}u,'),
                *table('uint32_t', f'{sym}_c64', [digests[n] for n in picked], '0x{:08x}ul,'), '#endif']
        taken = {p for r in rows if r[0] in picked for p in r[2]}
        c64 |= {p for p in taken if p < npaths}
        report[g] = {'host_cases': len(rows), 'c64_rows': len(picked), 'cover_rows': len(chosen),
                    'rows': picked, 'branches_and_edges_host': len({p for r in rows for p in r[2]}),
                    'branches_and_edges_c64': len(taken)}
        total += len(picked)
    missing = sorted(host - c64)
    assert not missing, f'{milestone}: the C64 groups miss whole-file host branches {missing}'
    (R / f'tests/{stem}_c64_vectors.h').write_text('\n'.join(out) + '\n')
    (B / f'{milestone}-c64-cover.json').write_text(json.dumps(
        {'milestone': milestone, 'host_cases': cases, 'c64_rows': total, 'groups': report,
         'whole_file_branches_host': len(host), 'whole_file_branches_c64': len(host & c64),
         'host_branches_missing_on_c64': missing,
         'paths': f'0-{npaths - 1} are the module\'s path counters; higher numbers are the control-flow edges of '
                  f'every linked source of each group build (clang -O0 -fsanitize-coverage=edge,trace-pc-guard)'}, indent=1) + '\n')
    print(f'{milestone}: {total} C64 rows in {len(report)} segment groups (each takes every entry, outcome, branch '
          f'and edge of every linked source its group\'s host cases take; together the {len(host)} branches of the whole file)')


def grouped(milestone, npaths, cases, full, out):
    """The C64 rows of a module built as several PRGs by entry slot (GROUPS).
    A case can be a row of its slot's PRG only when that PRG's build of the
    module runs it as the whole module does (a run that reaches another
    PRG's code is refused there); each PRG's rows take every slot, outcome,
    branch and edge its runnable cases take, and the PRGs' rows together
    every counted branch of the whole-module host cases."""
    stem, mac, sym, slots, budget, srcvar, run = MODULES[milestone]
    macro, group = GROUPS[milestone]
    of = {k: group(k) for k in range(slots)}
    kind = {r[0]: r[1] for r in full}
    vec = (R / f'tests/{stem}_vectors.h').read_text()
    digests = [int(h, 16) for h in re.findall(r'0x([0-9a-f]{8})ul', vec)]
    assert len(digests) == cases
    host = {p for r in full for p in r[2] if p < npaths}
    c64, report, total, crossing = set(), {}, 0, 0
    for g in sorted(set(of.values())):
        mine = [r for r in trace(milestone, npaths, (f'-D{macro}={g}',), f'-g{g}') if of[r[0] % slots] == g]
        rows = [r for r in mine if r[1] == kind[r[0]]]
        crossing += len(mine) - len(rows)
        share = max(1, budget * sum(1 for x in of.values() if x == g) // slots)
        chosen, picked = pick(rows, slots, share, cases)
        out += [f'#if {macro}=={g}', f'#define {mac}_C64_COUNT {len(picked)}u',
                *table('uint16_t', f'{sym}_c64_case', picked, '{}u,'),
                *table('uint32_t', f'{sym}_c64', [digests[n] for n in picked], '0x{:08x}ul,'), '#endif']
        taken = {p for r in rows if r[0] in picked for p in r[2]}
        c64 |= {p for p in taken if p < npaths}
        report[g] = {'host_cases': len(mine), 'runnable_in_prg': len(rows), 'c64_rows': len(picked),
                     'cover_rows': len(chosen), 'rows': picked,
                     'branches_and_edges_runnable': len({p for r in rows for p in r[2]}),
                     'branches_and_edges_c64': len(taken)}
        total += len(picked)
    missing = sorted(host - c64)
    assert not missing, f'{milestone}: the C64 PRGs miss whole-module host branches {missing}'
    (R / f'tests/{stem}_c64_vectors.h').write_text('\n'.join(out) + '\n')
    (B / f'{milestone}-c64-cover.json').write_text(json.dumps(
        {'milestone': milestone, 'host_cases': cases, 'c64_rows': total, 'prgs': report,
         'cases_reaching_another_prg': crossing,
         'whole_module_branches_host': len(host), 'whole_module_branches_c64': len(host & c64),
         'host_branches_missing_on_c64': missing,
         'paths': f'0-{npaths - 1} are the module\'s path counters; higher numbers are the control-flow edges of '
                  f'every linked source of each PRG build (clang -O0 -fsanitize-coverage=edge,trace-pc-guard)'},
        indent=1) + '\n')
    print(f'{milestone}: {total} C64 rows in {len(report)} PRGs (each takes every entry, outcome, branch and edge '
          f'of every linked source its runnable cases take; together the {len(host)} branches of the whole module; '
          f'{crossing} cases reach another PRG\'s code)')


def main(milestone):
    stem, mac, sym, slots, budget, srcvar, run = MODULES[milestone]
    test = (R / f'tests/test_{stem}.c').read_text()
    npaths = int(re.search(r'#define PATHS (\d+)u', test).group(1))
    cases = int(re.search(rf'#define {mac}_CASES (\d+)u', (R / f'tests/{stem}_cases.h').read_text()).group(1))
    rows = trace(milestone, npaths)
    assert [r[0] for r in rows] == list(range(cases))
    if milestone in SEGMENTS:
        out = [f'/* SPDX-License-Identifier: MIT; generated by tools/lane_f_c64_rows.py: the C64 rows',
               f' * of each segment group of src/{stem}.c and their digests (the group\'s run of the',
               ' * case to its segment end, frozen from the oracle), chosen so that each group\'s rows',
               ' * take every entry, outcome, branch and linked-source edge its host cases take (build/' + milestone +
               '-c64-cover.json), then', f' * spread with a stride of {STRIDE}. */']
        return segments(milestone, npaths, cases, rows, out)
    if milestone in GROUPS:
        out = [f'/* SPDX-License-Identifier: MIT; generated by tools/lane_f_c64_rows.py: the C64 rows',
               f' * of tests/{stem}_vectors.h per PRG, chosen so that each PRG\'s rows take every entry,',
               ' * outcome, branch and edge its PRG runs as the whole module does (build/' + milestone +
               '-c64-cover.json), then', f' * spread with a stride of {STRIDE}. */']
        return grouped(milestone, npaths, cases, rows, out)

    chosen = cover(rows, slots)
    picked = set(chosen)
    i = 0
    while len(picked) < budget:
        picked.add(i * STRIDE % cases)
        i += 1
    picked = sorted(picked)
    vec = (R / f'tests/{stem}_vectors.h').read_text()
    digests = [int(h, 16) for h in re.findall(r'0x([0-9a-f]{8})ul', vec)]
    assert len(digests) == cases
    counters = npaths
    npaths = max([p for r in rows for p in r[2]] + [counters - 1]) + 1
    host_paths = [0] * npaths
    c64_paths = [0] * npaths
    for r in rows:
        for p in r[2]:
            host_paths[p] += 1
            if r[0] in picked:
                c64_paths[p] += 1
    missing = [p for p in range(npaths) if host_paths[p] and not c64_paths[p]]
    assert not missing, f'{milestone}: C64 rows miss host branches {missing}'
    edges = sum(1 for p in range(counters, npaths) if host_paths[p])
    out = [f'/* SPDX-License-Identifier: MIT; generated by tools/lane_f_c64_rows.py: the C64 rows',
           f' * of tests/{stem}_vectors.h, chosen so that every entry, outcome and branch the',
           ' * host cases take is taken on the C64 (build/' + milestone + '-c64-cover.json), then',
           f' * spread with a stride of {STRIDE}. */',
           ]
    out += [f'#define {mac}_C64_COUNT {len(picked)}u',
            *table('uint16_t', f'{sym}_c64_case', picked, '{}u,'),
            *table('uint32_t', f'{sym}_c64', [digests[n] for n in picked], '0x{:08x}ul,')]
    (R / f'tests/{stem}_c64_vectors.h').write_text('\n'.join(out) + '\n')
    slot_rows = [sum(1 for n in picked if n % slots == k) for k in range(slots)]
    kinds = {f'kind {k}': [sum(1 for r in rows if r[1] == k), sum(1 for r in rows if r[1] == k and r[0] in picked)]
             for k in sorted({r[1] for r in rows})}
    report = {'milestone': milestone, 'host_cases': cases, 'c64_rows': len(picked),
             'cover_rows': len(chosen), 'stride_fill': STRIDE,
             'rows': picked,
             'entry_slots_host_c64': [[cases // slots + (1 if k < cases % slots else 0), slot_rows[k]]
                                      for k in range(slots)],
             'outcomes_host_c64': kinds,
             'branches_host_c64': {str(p): [host_paths[p], c64_paths[p]] for p in range(npaths)},
             'host_branches_missing_on_c64': missing,
             'paths': f'0-{counters - 1} are the module\'s path counters; {counters}-{npaths - 1} are the control-flow '
                      f'edges of every linked source (clang -O0 -fsanitize-coverage=edge,trace-pc-guard) the host cases take ({edges})'}
    (B / f'{milestone}-c64-cover.json').write_text(json.dumps(report, indent=1) + '\n')
    print(f'{milestone}: {len(picked)} C64 rows ({len(chosen)} cover every entry, outcome, the '
          f'{sum(1 for h in host_paths[:counters] if h)} counted paths and the {edges} edges of every linked source the host takes)')


if __name__ == '__main__':
    main(sys.argv[1])
