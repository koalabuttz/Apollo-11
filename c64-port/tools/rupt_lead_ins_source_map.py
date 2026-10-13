#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source maps for Luminary099 and Comanche055 INTERRUPT_LEAD_INS.agc
(milestone rupt-lead-ins).

Lists every assembled word of each file from its listing with its source
line, the native routine that translates it and whether the original
program executed it in the rupt-lead-ins oracle run
(build/rupt-lead-ins-executed-{lm,cm}.txt, written by
tools/rupt_lead_ins_freeze.py). Fails unless every instruction word was
executed, and unless the BBCON constants and TCF handlers built into
src/rupt_lead_ins.c equal the assembled words. Writes
build/rupt-lead-ins-source-map-{lm,cm}.json.
"""
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
MISSIONS = {'lm': 'Luminary099', 'cm': 'Comanche055'}
NATIVE = ['src/rupt_lead_ins.c', 'include/rupt_lead_ins.h']
LINE = re.compile(r'^\d+,(\d+):\s+(\d{4})\s+(\d{5})(?: (\d{5}))?\s')
LEAD = 'src/rupt_lead_ins.c'
# Words other milestones translate and compare against the original program.
OTHER = {'lm': {(0o4004, 0o4007): 'src/lm_t6rupt.c (T6RUPT vector; milestone lm-t6rupt)',
                (0o4014, 0o4017): 'src/lm_exec_tasks.c (T3RUPT vector; milestone lm-exec-jobs)',
                (0o4055, 0o4056): 'src/lm_t6rupt.c (T6ADR, read by DCA T6ADR; milestone lm-t6rupt)',
                (0o4057, 0o4057): 'src/lm_exec_tasks.c (T3RPTBB, read by CAF T3RPTBB; milestone lm-exec-jobs)'},
         'cm': {(0o4014, 0o4017): 'src/cm_exec_routines.c (T3RUPT vector; milestone cm-exec-services)',
                (0o4055, 0o4055): 'src/cm_exec_routines.c (T3RPTBB, read by CAF T3RPTBB; milestone cm-exec-services)'}}
# Constants nothing reads: Comanche's T5RPTBB (its vector branches with TCF T5RUPT).
UNREAD = {'cm': {0o4064}}


def native_tables(text, mission):
    body = re.search(r'%s_banked\[7\]=\{(.*?)\};' % mission, text, re.S).group(1)
    return [tuple(int(x, 8) for x in t) for t in re.findall(r'\{0?(\d+)u,0?(\d+)u,0?(\d+)u\}', body)]


def owner(mission, addr):
    for (a, b), who in OTHER[mission].items():
        if a <= addr <= b:
            return who
    return LEAD


def main():
    native_text = (R / LEAD).read_text()
    for m, name in MISSIONS.items():
        src = f'{name}/INTERRUPT_LEAD_INS.agc'
        text = (R.parent / src).read_text().splitlines()
        lst = (R / f'build/fixed-data/{name}/assembly.lst').read_text(errors='replace').splitlines()
        a = next(i for i, l in enumerate(lst) if '$INTERRUPT_LEAD_INS.agc' in l)
        b = next(i for i, l in enumerate(lst) if '(End of include-file INTERRUPT_LEAD_INS.agc' in l)
        executed = {int(x) for x in (R / f'build/rupt-lead-ins-executed-{m}.txt').read_text().split()}
        words, out, missing = {}, [], []
        for l in lst[a:b]:
            g = LINE.match(l)
            if not g:
                continue
            n, addr = int(g.group(1)), int(g.group(2), 8)
            source = text[n - 1].split('#')[0].split()
            assert source and source[-1] in l.split('\t')[0], (src, n, text[n - 1], l)
            ws = [g.group(3)] + ([g.group(4)] if g.group(4) else [])
            for i, w in enumerate(ws):
                words[addr + i] = int(w, 8)
            kind = 'constant' if 'BBCON' in source or '2CADR' in source else 'instruction'
            phys = 2048 + (addr - 0o4000)
            ran = phys in executed
            if kind == 'instruction' and not ran:
                missing.append(n)
            entry = {'line': n, 'address': f'FF,{addr:04o}', 'source': ' '.join(source), 'words': ws, 'kind': kind,
                     'native': owner(m, addr), 'executed_by_oracle': ran}
            if addr in UNREAD.get(m, ()):
                entry['unread'] = 'no instruction reads this constant; it has no native counterpart'
            out.append(entry)
        assert not missing, f'{src}: instructions never executed by the oracle: lines {missing}'
        # The native routines' built-in BBCONs and handlers are the assembled words.
        for vec, bbcon, handler in native_tables(native_text, m):
            caf = words[vec + 1]
            assert words[caf & 0o7777] == bbcon, (m, oct(vec), oct(words[caf & 0o7777]), oct(bbcon))
            assert words[vec + 3] & 0o7777 == handler, (m, oct(vec), oct(words[vec + 3]), oct(handler))
        gobb = {'lm': (0o12103, 0o2667), 'cm': (0o12063, 0o2526)}[m]
        assert words[words[0o4001] & 0o7777] == gobb[0] and words[0o4003] & 0o7777 == gobb[1], m
        assert f'{gobb[0]:o}u,0{gobb[1]:o}u' in native_text, m
        doc = {'source_path': src, 'source_sha256': hashlib.sha256((R.parent / src).read_bytes()).hexdigest(),
               'rope_sha256': hashlib.sha256((R / f'build/fixed-data/{name}/MAIN.agc.bin').read_bytes()).hexdigest(),
               'native': {p: hashlib.sha256((R / p).read_bytes()).hexdigest() for p in NATIVE},
               'source_words': sum(len(e['words']) for e in out), 'source_lines_with_words': len(out),
               'instructions_executed_by_oracle': sum(e['executed_by_oracle'] for e in out if e['kind'] == 'instruction'),
               'line_map': out}
        (R / f'build/rupt-lead-ins-source-map-{m}.json').write_text(json.dumps(doc, indent=1) + '\n')
        print(f"PASS: {len(out)} assembled lines of {src} ({doc['source_words']} words); every instruction executed by the "
              f"original program in the rupt-lead-ins oracle run; native BBCONs and handlers equal the assembled words")


if __name__ == '__main__':
    main()
