#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source map for the IMU monitor of Luminary099 T4RUPT_PROGRAM.agc
(milestone lm-t4rupt-imu).

Lists every assembled word from IMUMON to 90SECS (bank 06,2172-3005) with
the source line and whether the original program executed it in the
lm-t4rupt-imu oracle run (build/lm-t4rupt-imu-executed.txt, written by
tools/lm_t4rupt_imu_freeze.py). Fails unless every instruction word was
executed, and unless each fixed constant and external entry address built
into src/lm_t4rupt_imu.c equals the assembled word or symbol. Writes
build/lm-t4rupt-imu-source-map.json.
"""
import hashlib
import json
import pathlib
import re

R = pathlib.Path(__file__).resolve().parents[1]
NATIVE = ['src/lm_t4rupt_imu.c', 'include/lm_t4rupt_imu.h']
SRC = 'Luminary099/T4RUPT_PROGRAM.agc'
LINE = re.compile(r'^\d+,(\d+):\s+(?:(\d\d),)?(\d{4})\s+(\d{5})(?: (\d{5}))?\s')
FIRST, LAST = 0o2172, 0o3005
DATA = ('OCT', '2CADR', 'CADR', 'DEC')
# Native constant name -> the program's label, where they differ.
LABELS = {'K30RDMSK': '30RDMSK', 'K33RDMSK': 'PRIO16', 'OCT15000': 'PRIO15', 'BITS7_8': 'BITS7&8',
          'BITS4_5': 'BITS4&5', 'BITS6_15': 'BITS6&15', 'MBIT10': '-BIT10', 'K90SECS': '90SECS',
          'K4SECS': '4SECS', 'M70DEGS': '-70DEGS', 'M15DEGS': '-15DEGS'}
EXTERNAL = ('ALARM', 'WAITLIST', 'VARDELAY', 'TASKOVER', 'POSTJUMP', 'IBNKCALL', 'ZEROICDU', 'VARALARM', 'RESUME')


def main():
    native_text = (R / NATIVE[0]).read_text()
    text = (R.parent / SRC).read_text().splitlines()
    lst = (R / 'build/fixed-data/Luminary099/assembly.lst').read_text(errors='replace').splitlines()
    symbols = dict((m.group(1), m.group(2)) for m in re.finditer(r'\d+,[A-Z]:\s+(\S+)\s+((?:\d\d,)?\d{4})\b', '\n'.join(lst)))
    words = {}
    for l in lst:
        g = LINE.match(l)
        if g:
            ws = [g.group(4)] + ([g.group(5)] if g.group(5) else [])
            for i, w in enumerate(ws):
                words[(g.group(2), int(g.group(3), 8) + i)] = int(w, 8)
    def word_at(sym):
        loc = symbols[sym]
        bank, addr = (loc.split(',') if ',' in loc else (None, loc))
        return words[(bank, int(addr, 8))]
    block = native_text[native_text.index('/* Fixed constants'):native_text.index('#define ALARM')]
    consts = re.findall(r'#define (\w+) 0?([0-7]+)u', block)
    for name, value in consts:
        assert word_at(LABELS.get(name, name)) == int(value, 8), (name, value)
    for name in EXTERNAL:
        value = int(re.search(r'#define %s 0?([0-7]+)u' % name, native_text).group(1), 8)
        assert int(symbols[name], 8) == value, name
    a = next(i for i, l in enumerate(lst) if '$T4RUPT_PROGRAM.agc' in l)
    b = next(i for i, l in enumerate(lst) if '(End of include-file T4RUPT_PROGRAM.agc' in l)
    executed = {int(x) for x in (R / 'build/lm-t4rupt-imu-executed.txt').read_text().split()}
    out, missing, cursor = [], [], 0
    for l in lst[a:b]:
        g = LINE.match(l)
        if not g or g.group(2) != '06':
            continue
        addr = int(g.group(3), 8)
        if not FIRST <= addr <= LAST:
            continue
        listed = l.split('\t')[0].split()[3 + (1 if g.group(5) else 0):]
        listed = [x for x in listed if not re.fullmatch(r'B[-+]?\d+', x)]
        def same(k):
            lab = not text[k][:1].isspace()
            got = [x for x in text[k].split('#')[0].split() if not re.fullmatch(r'B[-+]?\d+', x)]
            return got[lab:] == listed[lab:]
        k = next((k for k in range(cursor, len(text)) if text[k].strip() and not text[k].lstrip().startswith('#') and same(k)), None)
        assert k is not None, (listed, l)
        cursor = k + 1
        source = text[k].split('#')[0].split()
        ws = [g.group(4)] + ([g.group(5)] if g.group(5) else [])
        kind = 'constant' if any(d in source for d in DATA) else 'instruction'
        ran = 6 * 1024 + addr - 0o2000 in executed
        if kind == 'instruction' and not ran:
            missing.append(k + 1)
        out.append({'line': k + 1, 'address': f'06,{addr:04o}', 'source': ' '.join(source), 'words': ws, 'kind': kind,
                    'native': NATIVE[0], 'executed_by_oracle': ran})
    assert not missing, f'{SRC}: instructions never executed by the oracle: lines {missing}'
    doc = {'source_path': SRC, 'scope': 'IMUMON through 90SECS (06,2172-3005): the IMU monitor, its bit-change '
                                        'handlers, CAGESUB, LAMPTEST and their constants',
           'source_sha256': hashlib.sha256((R.parent / SRC).read_bytes()).hexdigest(),
           'rope_sha256': hashlib.sha256((R / 'build/fixed-data/Luminary099/MAIN.agc.bin').read_bytes()).hexdigest(),
           'native': {p: hashlib.sha256((R / p).read_bytes()).hexdigest() for p in NATIVE},
           'source_words': sum(len(e['words']) for e in out), 'source_lines_with_words': len(out),
           'instructions_executed_by_oracle': sum(e['executed_by_oracle'] for e in out if e['kind'] == 'instruction'),
           'native_constants_checked': len(consts), 'external_addresses_checked': len(EXTERNAL),
           'line_map': out}
    (R / 'build/lm-t4rupt-imu-source-map.json').write_text(json.dumps(doc, indent=1) + '\n')
    print(f"PASS: {len(out)} assembled lines of the {SRC} IMU monitor ({doc['source_words']} words); every instruction "
          f"executed by the original program in the lm-t4rupt-imu oracle run; {len(consts)} native constants and "
          f"{len(EXTERNAL)} external addresses equal the assembled program")


if __name__ == '__main__':
    main()
