#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source maps for Luminary099 and Comanche055 KEYRUPT_UPRUPT.agc
(milestone keyrupt-uprupt).

Lists every assembled word of each file from its listing with its source
line and whether the original program executed it in the keyrupt-uprupt
oracle run (build/keyrupt-uprupt-executed-{lm,cm}.txt, written by
tools/keyrupt_uprupt_freeze.py). Fails unless every instruction word was
executed (except Luminary's trailing CS XDSPBIT at 04,3400, which nothing
reaches), and unless the bank, base address and LODSAMPT, NOVAC and RESUME
targets built into src/keyrupt_uprupt.c equal the assembled words. Writes
build/keyrupt-uprupt-source-map-{lm,cm}.json.
"""
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
MISSIONS = {'lm': 'Luminary099', 'cm': 'Comanche055'}
NATIVE = ['src/keyrupt_uprupt.c', 'include/keyrupt_uprupt.h']
LINE = re.compile(r'^\d+,(\d+):\s+(\d\d),(\d{4})\s+(\d{5})(?: (\d{5}))?\s')
UNREACHED = {'lm': {0o3400: 'CS XDSPBIT after ELRCODE: no instruction or CADR reaches it (UPPART2, the next '
                            'label, is at 04,3401); it has no native counterpart'}}
DATA = ('OCT', '2CADR')


def main():
    native_text = (R / NATIVE[0]).read_text()
    for m, name in MISSIONS.items():
        src = f'{name}/KEYRUPT_UPRUPT.agc'
        text = (R.parent / src).read_text().splitlines()
        lst = (R / f'build/fixed-data/{name}/assembly.lst').read_text(errors='replace').splitlines()
        a = next(i for i, l in enumerate(lst) if '$KEYRUPT,_UPRUPT.agc' in l)   # the listing names the file KEYRUPT,_UPRUPT
        b = next(i for i, l in enumerate(lst) if '(End of include-file KEYRUPT,_UPRUPT.agc' in l)
        executed = {int(x) for x in (R / f'build/keyrupt-uprupt-executed-{m}.txt').read_text().split()}
        words, out, missing, bank, cursor = {}, [], [], None, 0
        for l in lst[a:b]:
            g = LINE.match(l)
            if not g:
                continue
            n, bank, addr = int(g.group(1)), int(g.group(2), 8), int(g.group(3), 8)
            # The repository's comments differ from the listing's source, so
            # source lines are matched in order (labels may differ: this file
            # reads UPCK where the listing reads UPOK; nothing refers to either).
            listed = l.split('\t')[0].split()[3 + (1 if g.group(5) else 0):]
            def same(k):
                lab = not text[k][:1].isspace()
                return text[k].split('#')[0].split()[lab:] == listed[lab:]
            k = next(k for k in range(cursor, len(text)) if text[k].strip() and not text[k].lstrip().startswith('#') and same(k))
            cursor, n = k + 1, k + 1
            source = text[k].split('#')[0].split()
            labelled = not text[k][:1].isspace()
            assert source[labelled:] == listed[labelled:], (src, n, text[n - 1], l)
            ws = [g.group(4)] + ([g.group(5)] if g.group(5) else [])
            for i, w in enumerate(ws):
                words[addr + i] = int(w, 8)
            kind = 'constant' if any(d in source for d in DATA) else 'instruction'
            ran = bank * 1024 + (addr - 0o2000) in executed
            entry = {'line': n, 'address': f'{bank:02o},{addr:04o}', 'source': ' '.join(source), 'words': ws, 'kind': kind,
                     'native': NATIVE[0], 'executed_by_oracle': ran}
            if addr in UNREACHED.get(m, {}):
                entry['unreached'] = UNREACHED[m][addr]
            elif kind == 'instruction' and not ran:
                missing.append(n)
            out.append(entry)
        assert not missing, f'{src}: instructions never executed by the oracle: lines {missing}'
        site = re.search(r'%s_site=\{0?(\d+)u,0?(\d+)u,0?(\d+)u,0?(\d+)u,0?(\d+)u\}' % m, native_text)
        fb, base, lodsampt, novac, resume = (int(x, 8) for x in site.groups())
        assert fb >> 10 == bank and min(words) == base, (m, oct(fb), oct(base))
        assert words[base + 3] & 0o7777 == lodsampt and words[base + 0o14] & 0o7777 == novac, m
        assert words[base + 0o22] & 0o7777 == resume, m
        doc = {'source_path': src, 'source_sha256': hashlib.sha256((R.parent / src).read_bytes()).hexdigest(),
               'rope_sha256': hashlib.sha256((R / f'build/fixed-data/{name}/MAIN.agc.bin').read_bytes()).hexdigest(),
               'native': {p: hashlib.sha256((R / p).read_bytes()).hexdigest() for p in NATIVE},
               'source_words': sum(len(e['words']) for e in out), 'source_lines_with_words': len(out),
               'instructions_executed_by_oracle': sum(e['executed_by_oracle'] for e in out if e['kind'] == 'instruction'),
               'line_map': out}
        (R / f'build/keyrupt-uprupt-source-map-{m}.json').write_text(json.dumps(doc, indent=1) + '\n')
        print(f"PASS: {len(out)} assembled lines of {src} ({doc['source_words']} words); every reachable instruction "
              f"executed by the original program in the keyrupt-uprupt oracle run; native bank and targets equal the "
              f"assembled words")


if __name__ == '__main__':
    main()
