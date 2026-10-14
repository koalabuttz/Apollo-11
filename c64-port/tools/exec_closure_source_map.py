#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-file source maps for EXECUTIVE.agc and WAITLIST.agc of Luminary099
and Comanche055 (milestone exec-closure).

Every assembled word of each file is listed with its source line, its kind
(instruction or constant) and the native routine milestones whose oracle run
executed it. The executed-words files come from the milestones' own
differentials run under tools/oracle_trace (`make exec-closure-trace`):
build/exec-closure-executed-<mission>-<milestone>.txt, plus the RESUME family
from build/rupt-lead-ins-executed-<mission>.txt. Fails unless every
instruction word was executed by at least one milestone's oracle run, apart
from the words named in UNREACHED with the reason no producer reaches them.
Writes build/exec-closure-source-map-<mission>-<file>.json.
"""
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
MISSIONS = {'lm': 'Luminary099', 'cm': 'Comanche055'}
FILES = ['EXECUTIVE.agc', 'WAITLIST.agc']
MILESTONES = {
    'exec-services': {'lm': ['src/lm_exec_routines.c'], 'cm': ['src/cm_exec_routines.c']},
    'exec-jobs': {'lm': ['src/lm_exec_jobs.c', 'src/lm_exec_tasks.c'], 'cm': ['src/cm_exec_jobs.c']},
    'exec-idle': {'lm': ['src/lm_exec_idle.c'], 'cm': ['src/cm_exec_idle.c']},
    'rupt-lead-ins': {'lm': ['src/rupt_lead_ins.c'], 'cm': ['src/rupt_lead_ins.c']},
}
LINE = re.compile(r'^\d+,(\d+):\s+(?:(\d\d),)?(\d{4})\s+(\d{5})(?: (\d{5}))?\s')
DATA = {'-CCS', '-2CADR', 'OCT', 'DEC', '2DEC', 'CADR', '2CADR', 'GENADR', 'ADRES', 'ECADR', 'OCTAL', '2OCT', 'BBCON', '2BCADR',
        'FCADR', 'REMADR', 'DNPTR', 'DNCHAN', '1DNADR', '2DNADR', '3DNADR', '4DNADR', '5DNADR', '6DNADR'}
NEWJOB = ("The original's own CCSHOLE trap on CCS NEWJOB at SPECTEST: outside CHANJOB the Executive keeps "
          "NEWJOB -0 (no job waiting) or a core-set location (positive), so this class never reaches it")
PRIO = ("The original's own CCSHOLE trap for a +0 PRIORITY register: a core set's priority is -0 (free), "
        "positive (active) or negative (asleep), never +0")
BUF1 = ("The original's own CCSHOLE trap on CCS BUF +1 after the scan: BUF +1 holds -0 or the complement of the "
        "highest active priority, never a positive value or +0")
NOOP = "Marked CAN'T GET HERE in the source: the CCS class before it cannot occur"
# (mission, file, line) -> why no producer reaches the instruction word.
UNREACHED = {
    ('lm', 'EXECUTIVE.agc', 182): NEWJOB, ('lm', 'EXECUTIVE.agc', 183): NEWJOB,
    **{('lm', 'EXECUTIVE.agc', n): PRIO for n in (390, 395, 405, 410, 415, 421)},
    ('lm', 'EXECUTIVE.agc', 428): BUF1, ('lm', 'EXECUTIVE.agc', 429): BUF1,
    ('cm', 'EXECUTIVE.agc', 182): NEWJOB,
    **{('cm', 'EXECUTIVE.agc', n): PRIO for n in (387, 392, 402, 407, 412)},
    ('cm', 'EXECUTIVE.agc', 419): BUF1, ('cm', 'EXECUTIVE.agc', 420): BUF1,
    ('lm', 'WAITLIST.agc', 326): NOOP + ' (WTABORT: TC FILLED does not return)',
    ('lm', 'WAITLIST.agc', 527): NOOP + ' (LONGCYCL: LONGTIME +1 is never +0 after DAS of -DPBIT14)',
    ('cm', 'WAITLIST.agc', 525): NOOP + ' (LONGCYCL: LONGTIME +1 is never +0 after DAS of -DPBIT14)',
}
# Words assembled as instructions that the program only reads as data.
READ_AS_DATA = {
    ('lm', 'WAITLIST.agc', 168): 'DELAYEX: VARDELAY reads it with CAF DELAYEX into WAITEXIT (an address word for DTCB)',
    ('cm', 'WAITLIST.agc', 182): 'DELAYEX: VARDELAY reads it with CAF DELAYEX into WAITEXIT (an address word for DTCB)',
}
# Paths not yet native: the file is not complete until they are.
PENDING = {}


def executed(m):
    out = {}
    for ms in MILESTONES:
        p = R / (f'build/rupt-lead-ins-executed-{m}.txt' if ms == 'rupt-lead-ins'
                 else f'build/exec-closure-executed-{m}-{ms}.txt')
        out[ms] = {int(x) for x in p.read_text().split()}
    return out


def physical(bank, addr):
    return int(bank, 8) * 1024 + addr - 0o2000 if bank else 2048 + addr - 0o4000


def main():
    failures, pending, summary = [], [], []
    for m, name in MISSIONS.items():
        ran = executed(m)
        lst = (R / f'build/fixed-data/{name}/assembly.lst').read_text(errors='replace').splitlines()
        for fname in FILES:
            src = f'{name}/{fname}'
            text = (R.parent / src).read_text().splitlines()
            a = next(i for i, l in enumerate(lst) if f': ${fname}' in l)
            b = next(i for i, l in enumerate(lst) if f'(End of include-file {fname}' in l)
            out, cursor, label = [], 0, None
            for l in lst[a:b]:
                g = LINE.match(l)
                if not g:
                    continue
                bank, addr = g.group(2), int(g.group(3), 8)
                listed = l.split('\t')[0].split()[3 + (1 if g.group(5) else 0):]
                listed = [x for x in listed if not re.fullmatch(r'B[-+]?\d+', x)]

                def same(k):
                    lab = not text[k][:1].isspace()
                    return [x for x in text[k].split('#')[0].split()[lab:] if not re.fullmatch(r'B[-+]?\d+', x)] \
                        == listed[lab:]
                k = next((k for k in range(cursor, len(text)) if text[k].strip()
                          and not text[k].lstrip().startswith('#') and same(k)), None)
                assert k is not None, (src, listed, l)
                cursor = k + 1
                source = text[k].split('#')[0].split()
                if not text[k][:1].isspace():
                    label = source[0]
                ws = [g.group(4)] + ([g.group(5)] if g.group(5) else [])
                kind = 'constant' if DATA & set(source) else 'instruction'
                phys = [physical(bank, addr + i) for i in range(len(ws))]
                owners = sorted(ms for ms, s in ran.items() if s & set(phys))
                entry = {'line': k + 1, 'address': f'{bank},{addr:04o}' if bank else f'FF,{addr:04o}',
                         'label': label, 'source': ' '.join(source), 'words': ws, 'kind': kind,
                         'executed_by': owners,
                         'native': sorted({p for ms in owners for p in MILESTONES[ms][m]})}
                key = (m, fname, k + 1)
                if key in READ_AS_DATA:
                    kind = entry['kind'] = 'constant'
                    entry['read_as_data'] = READ_AS_DATA[key]
                if kind == 'instruction' and not owners:
                    if key in UNREACHED:
                        entry['unreached'] = UNREACHED[key]
                    elif key in PENDING:
                        entry['pending'] = PENDING[key]
                        pending.append(f'{src}:{k + 1} {PENDING[key]}')
                    else:
                        failures.append(f'{src}:{k + 1} {entry["address"]} {label}: {" ".join(source)}')
                out.append(entry)
            instr = [e for e in out if e['kind'] == 'instruction']
            doc = {'source_path': src, 'scope': 'whole file',
                   'source_sha256': hashlib.sha256((R.parent / src).read_bytes()).hexdigest(),
                   'rope_sha256': hashlib.sha256((R / f'build/fixed-data/{name}/MAIN.agc.bin').read_bytes()).hexdigest(),
                   'milestones': {ms: v[m] for ms, v in MILESTONES.items()},
                   'source_words': sum(len(e['words']) for e in out), 'source_lines_with_words': len(out),
                   'instruction_lines': len(instr),
                   'instruction_lines_executed_by_oracle': sum(bool(e['executed_by']) for e in instr),
                   'unreached_instruction_lines': sum('unreached' in e for e in instr),
                   'pending_instruction_lines': sum('pending' in e for e in instr),
                   'line_map': out}
            summary.append({k: doc[k] for k in ('source_path', 'source_sha256', 'source_words', 'instruction_lines',
                                                 'instruction_lines_executed_by_oracle', 'unreached_instruction_lines',
                                                 'pending_instruction_lines')})
            stem = fname.split('.')[0].lower()
            (R / f'build/exec-closure-source-map-{m}-{stem}.json').write_text(json.dumps(doc, indent=1) + '\n')
            print(f"{src}: {len(out)} assembled lines, {doc['source_words']} words, {len(instr)} instruction lines, "
                  f"{doc['instruction_lines_executed_by_oracle']} executed by the milestones' oracle runs")
    manifests = {f'{m}-{ms}' if ms != 'rupt-lead-ins' else ms: None for m in MISSIONS for ms in MILESTONES}
    for e in manifests:
        path = R / f'evidence/{e}/SHA256.json'
        run = json.loads((R / f'evidence/{e}/{e}-proof.json').read_text())
        manifests[e] = {'manifest': str(path.relative_to(R)), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                        'oracle_cases': {k: {'mission': v['mission'], 'rope_sha256': v['rope_sha256'], 'cases': v['cases']}
                                         for k, v in ([('run', run)] if 'cases' in run else run.items())
                                         if isinstance(v, dict) and 'cases' in v}}
    # waitlist-entries compares the WAITLIST entries other files reach (T3RUPT, DLY2 -1, NOQBRSM +1, and
    # TASKOVER's RESUME check from yaAGC's own T3RUPT) with yaAGC; its words are already in the traced runs above.
    wle = R / 'evidence/waitlist-entries/SHA256.json'
    wle_run = json.loads((R / 'evidence/waitlist-entries/waitlist-entries-proof.json').read_text())
    manifests['waitlist-entries'] = {'manifest': str(wle.relative_to(R)), 'sha256': hashlib.sha256(wle.read_bytes()).hexdigest(),
                                     'oracle_cases': {'run': {'missions': sorted(wle_run['ropes']), 'ropes_sha256': wle_run['ropes'],
                                                              'cases': wle_run['cases']}},
                                     'taskover_superbank': wle_run['taskover_superbank']['result'],
                                     'traced': False}
    proof = {'files': summary, 'milestone_evidence': manifests,
             'method': 'each milestone differential rerun with tools/oracle_trace linked into its yaAGC reference; '
                       'a word is executed when the original program ran it in that comparison (each run ends where '
                       'the native routine ended, so the word is inside a native routine\'s compared path)'}
    (R / 'build/exec-closure-proof.json').write_text(json.dumps(proof, indent=1) + '\n')
    assert not failures, 'instructions no oracle run executed:\n' + '\n'.join(failures)
    print('PASS: every instruction word of EXECUTIVE.agc and WAITLIST.agc (both missions) is executed by a native '
          'routine milestone\'s oracle run, apart from the named unreached words' +
          (''.join(f'\nPENDING {p}' for p in pending) if pending else ''))


if __name__ == '__main__':
    main()
