#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source maps for the T4RUPT core of Luminary099 and Comanche055
T4RUPT_PROGRAM.agc (milestone t4rupt-core).

Lists every assembled word from T4RUPT to the end of SYNCT4/QUIKOFF (bank
06,2000-2171 LM, 06,2000-2173 CM) and the RELTAB table it reads, with the
source line and whether the original program executed it in the
t4rupt-core oracle run (build/t4rupt-core-executed-{lm,cm}.txt, written by
tools/t4rupt_core_freeze.py). Fails unless every instruction word was
executed but the ones named in UNREACHED, and unless the bank, DSRUPTSW,
IMODES33, T4JUMP targets and Q+1 built into src/t4rupt_core.c equal the
assembled words. Writes build/t4rupt-core-source-map-{lm,cm}.json.
"""
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
MISSIONS = {'lm': 'Luminary099', 'cm': 'Comanche055'}
NATIVE = ['src/t4rupt_core.c', 'include/t4rupt_core.h']
LINE = re.compile(r'^\d+,(\d+):\s+(?:(\d\d),)?(\d{4})\s+(\d{5})(?: (\d{5}))?\s')
# Constants the code executes as instructions only on paths no producer reaches.
UNREACHED = {0o2037: 'TABLNTH (OCT 12) runs as TC 012 only for a -0 display word or a negative DSPCNT, which '
                     'nothing stores (fresh start writes -BIT12, DSPIN complemented codes)',
             0o2041: '120MRUPT (DEC 16372) runs as CA 7764 only when DSPSCAN finds DSRUPTEM positive; '
                     'DSPOUTSB sets it to -0 and the scan only stores +0 there',
             'OCT37737': 'OCT37737 runs as CA 7737 only when SYNCT4 finds DSRUPTSW +0, which adding BIT9 to a '
                         'negative DSRUPTSW never yields (it reaches -0 or a positive count)'}
DATA = ('OCT', '2CADR', 'CADR', 'DEC')


def main():
    native_text = (R / NATIVE[0]).read_text()
    for m, name in MISSIONS.items():
        src = f'{name}/T4RUPT_PROGRAM.agc'
        text = (R.parent / src).read_text().splitlines()
        lst = (R / f'build/fixed-data/{name}/assembly.lst').read_text(errors='replace').splitlines()
        a = next(i for i, l in enumerate(lst) if '$T4RUPT_PROGRAM.agc' in l)
        b = next(i for i, l in enumerate(lst) if '(End of include-file T4RUPT_PROGRAM.agc' in l)
        executed = {int(x) for x in (R / f'build/t4rupt-core-executed-{m}.txt').read_text().split()}
        end = 0o2171 if m == 'lm' else 0o2173
        words, out, missing, cursor = {}, [], [], 0
        for l in lst[a:b]:
            g = LINE.match(l)
            if not g:
                continue
            n, bank, addr = int(g.group(1)), g.group(2), int(g.group(3), 8)
            if bank is None and not 0o4000 <= addr < 0o4200 or bank is not None and (bank != '06' or addr > end):
                continue
            listed = l.split('\t')[0].split()[3 + (1 if g.group(5) else 0):]
            # The listing's source spells 120MRUPT's scale (B-14); this file does not.
            listed = [x for x in listed if not re.fullmatch(r'B[-+]?\d+', x)]
            def same(k):
                lab = not text[k][:1].isspace()
                return text[k].split('#')[0].split()[lab:] == listed[lab:]
            k = next((k for k in range(cursor, len(text)) if text[k].strip() and not text[k].lstrip().startswith("#") and same(k)), None)
            assert k is not None, (src, listed, l)
            cursor, n = k + 1, k + 1
            source = text[k].split('#')[0].split()
            ws = [g.group(4)] + ([g.group(5)] if g.group(5) else [])
            for i, w in enumerate(ws):
                words[(bank, addr + i)] = int(w, 8)
            kind = 'constant' if any(d in source for d in DATA) else 'instruction'
            phys = 6 * 1024 + addr - 0o2000 if bank else 2048 + addr - 0o4000
            ran = phys in executed
            entry = {'line': n, 'address': f'{bank},{addr:04o}' if bank else f'FF,{addr:04o}', 'source': ' '.join(source),
                     'words': ws, 'kind': kind, 'native': NATIVE[0], 'executed_by_oracle': ran}
            why = UNREACHED.get(addr) or (UNREACHED['OCT37737'] if 'OCT37737' in source else None)
            if bank and why and not ran:
                entry['unreached'] = why
            elif bank and kind == 'instruction' and not ran:
                missing.append(n)
            out.append(entry)
        assert not missing, f'{src}: instructions never executed by the oracle: lines {missing}'
        site = re.search(r'%s_site=\{0?(\d+)u,0?(\d+)u,0?(\d+)u,0?(\d+)u,\{0?(\d+)u,0?(\d+)u,0?(\d+)u,0?(\d+)u\},(\d+)u,0?(\d+)u\}' % m, native_text)
        dsr, im33, novac, resume, j0, j1, j2, j3, shift, qp1 = (int(x, 8) for x in site.groups())
        w = lambda z: words[('06', z)]
        assert w(0o2003) & 0o1777 == dsr and w(0o2075) & 0o7777 == im33, m
        assert w(0o2113) & 0o7777 == novac and w(0o2151 + shift) & 0o7777 == resume, m
        assert w(0o2062) & 0o7777 == qp1, m
        assert [w(0o2120 + i) & 0o7777 for i in range(1, 4)] == [j1, j2, j3] and (m == 'cm' or w(0o2120) & 0o7777 == j0), m
        assert m == 'lm' or w(0o2130) & 0o7777 == j0, m
        reltab = [int(x, 8) for x in re.findall(r'0?(\d+)u', re.search(r'reltab\[12\]=\{([^}]*)\}', native_text).group(1))]
        rt = sorted((k[1], v) for k, v in words.items() if k[0] is None)
        assert [v for _, v in rt] == reltab, m
        doc = {'source_path': src, 'scope': 'T4RUPT through SYNCT4/QUIKOFF and RELTAB',
               'source_sha256': hashlib.sha256((R.parent / src).read_bytes()).hexdigest(),
               'rope_sha256': hashlib.sha256((R / f'build/fixed-data/{name}/MAIN.agc.bin').read_bytes()).hexdigest(),
               'native': {p: hashlib.sha256((R / p).read_bytes()).hexdigest() for p in NATIVE},
               'source_words': sum(len(e['words']) for e in out), 'source_lines_with_words': len(out),
               'instructions_executed_by_oracle': sum(e['executed_by_oracle'] for e in out if e['kind'] == 'instruction'),
               'line_map': out}
        (R / f'build/t4rupt-core-source-map-{m}.json').write_text(json.dumps(doc, indent=1) + '\n')
        print(f"PASS: {len(out)} assembled lines of the {src} core ({doc['source_words']} words); every instruction but "
              f"the unreached ones executed by the original program in the t4rupt-core oracle run; native constants and "
              f"targets equal the assembled words")


if __name__ == '__main__':
    main()
