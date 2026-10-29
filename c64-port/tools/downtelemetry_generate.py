#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile the fixed words DOWN-TELEMETRY_PROGRAM reads as data.

DODOWNTM reads the downlists of DOWNLINK_LISTS (DNTABLE, the control list
words, DNPTR sublists and snapshot sublists) and a few constants. Three of
its own words (MINTIME2, DNADRDCR, MINB1314) are also executed as
instructions on the +0 branch of the CCS before them, so the fixed words
those instructions read are data too. Every word comes from the unmodified
source listing and is checked against the rope; native code never reads the
rope. Also emitted: every word the program stores in DNTMGOTO (TC targets
executed from erasable memory), with its producer.

  python3 tools/downtelemetry_generate.py --mission Comanche055 [--check] [--json FILE]
  python3 tools/downtelemetry_generate.py --mission Luminary099 [--check] [--json FILE]
"""
import argparse
import hashlib
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from agc_sites_generate import listing_lines, symbol_table  # noqa: E402
from fixed_data_generate import fixed_address, rope_word  # noqa: E402

MISSIONS = {
    'Comanche055': ('CM', 'cm', 'DOWN-TELEMETRY_PROGRAM', 'src/cm_downtelemetry_data.inc'),
    'Luminary099': ('LM', 'lm', 'DOWN-TELEMETRY_PROGRAM', 'src/lm_downtelemetry_data.inc'),
}
DATA_OPS = ('CA', 'CAF', 'CS', 'AD', 'MASK')
EXECUTED = ('MINTIME2', 'DNADRDCR', 'MINB1314')
# DNTMGOTO producers: the labels whose GENADR/ADRES words are stored there,
# and the return address of DNDUMPI's TC SENDID (QXCH DNTMGOTO).
GOTO_CONSTANTS = ('LDNPHAS1', 'LDNPHAS2', 'LDNDUMPI', 'LDNDUMP', 'LDNDUMP1')


def operand_physical(word, bank):
    address = word & 0o7777
    if address < 0o2000:
        return None
    if address >= 0o4000:
        return address
    return bank * 1024 + address - 0o2000


def build(mission):
    prefix, name, program, _ = MISSIONS[mission]
    base = ROOT / 'build/fixed-data' / mission
    listing = (base / 'assembly.lst').read_text(errors='replace')
    rope = (base / 'MAIN.agc.bin').read_bytes()
    table = symbol_table(listing)
    rows = list(listing_lines(listing))
    listed = {}
    for path, line, address, words, label, op, opnd in rows:
        at = fixed_address(address) if words else None
        if at is not None:
            for k, w in enumerate(words):
                listed[at + k] = (w, f'{path}:{line}', f'{label} {op} {opnd}'.strip())
    # Bank checksums ('Bugger word NNNNN at BB,AAAA.' after the listing body).
    for m in __import__('re').finditer(r'^Bugger word ([0-7]{5}) at ([0-7]{2},[0-7]{4})\.', listing, __import__('re').M):
        bb, aa = m.group(2).split(',')
        phys = int(bb, 8) * 1024 + int(aa, 8) - 0o2000
        if int(bb, 8) in (2, 3):
            phys = 0o4000 + (int(bb, 8) - 2) * 1024 + int(aa, 8) - 0o2000
        listed[phys] = (int(m.group(1), 8), 'assembly.lst bugger word', f'bugger word at {m.group(2)}')
    data = {}
    operands = set()
    lists = [r for r in rows if r[0].startswith('DOWNLINK_LISTS') and r[3]]
    for path, line, address, words, label, op, opnd in lists:
        at = fixed_address(address)
        for k, w in enumerate(words):
            data[at + k] = 'list'
    code = [r for r in rows if r[0].startswith(program) and r[3]]
    bank = fixed_address(code[0][2]) // 1024
    for path, line, address, words, label, op, opnd in code:
        if op in DATA_OPS and len(words) == 1:
            phys = operand_physical(words[0], bank)
            if phys is not None:
                data.setdefault(phys, 'constant')
                operands.add(phys)
        if label in EXECUTED:
            # Only when the word is assembled (or a bank checksum); an
            # unassembled word has no source, and the native path refuses.
            phys = operand_physical(words[0], bank)
            if phys in listed:
                data.setdefault(phys, f'read by {label} executed as an instruction')
                operands.add(phys)
    out = []
    for phys in sorted(data):
        word, where, text = listed[phys]
        assert rope_word(rope, phys) == word, (phys, where)
        out.append({'physical': phys, 'word': word, 'kind': data[phys], 'source': where, 'text': text})
    constants = [(row['physical'], row['word']) for row in out if row['physical'] in operands]
    gotos = []
    for label in GOTO_CONSTANTS:
        phys = fixed_address(table[label])
        word = rope_word(rope, phys)
        assert listed[phys][0] == word
        gotos.append((label, word))
    sendid = [r for r in code if r[5] == 'TC' and r[6].split()[:1] == ['SENDID']]
    assert len(sendid) == 1
    ret = fixed_address(sendid[0][2]) + 1
    gotos.append(('DNDUMPI+3 (QXCH DNTMGOTO in SENDID)', 0o2000 + ret % 1024))
    return prefix, name, out, gotos, bank, hashlib.sha256(rope).hexdigest(), constants


def render(mission, prefix, name, data, gotos, bank, rope_sha, constants):
    lines = ['/* SPDX-License-Identifier: MIT; generated by tools/downtelemetry_generate.py. */',
             f'/* {mission}: fixed words DOWN-TELEMETRY_PROGRAM reads as data (downlists and',
             f' * constants), keyed by physical word. Oracle rope sha256 {rope_sha}. */',
             f'#define {prefix}_DNTM_BANK 0{bank:o}u',
             f'#define {prefix}_DNTM_DATA_COUNT {len(data)}u',
             f'static const agc_site_operand {name}_dntm_data[{len(data)}]={{']
    for row in data:
        b, offset = divmod(row['physical'], 1024)
        lines.append(f" {{{row['physical']}u,0{row['word']:05o}}}, /* {b:02o},{offset + 0o2000:04o} "
                     f"{row['text'][:40]} */")
    lines.append('};')
    lines.append(f'/* Words stored in DNTMGOTO and executed there as TC: ' +
                 ', '.join(label for label, _ in gotos) + '. */')
    lines.append(f'#define {prefix}_DNTM_GOTO_COUNT {len(gotos)}u')
    lines.append(f'#define {prefix}_DNTM_GOTO_WORDS ' + ','.join(f'0{w:05o}u' for _, w in gotos))
    # The same words, for the ones the program's own instructions name
    # (operands of CA/CAF/CS/AD/MASK and of the words executed as
    # instructions): k() folds a constant address to its word at compile time
    # instead of searching the table on every read. Any other address takes
    # `miss`, the table search.
    lines.append(f'/* Fixed words the program\'s instructions name, by physical word: {prefix}_DNTM_CONSTANT(p,miss)')
    lines.append(f' * is the word at p, or miss for any other p (folds when p is constant). */')
    lines.append(f'#define {prefix}_DNTM_CONSTANT(p,miss) (' +
                 ''.join(f'(p)=={phys}u?0{word:05o}u:' for phys, word in constants) + '(miss))')
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--mission', required=True, choices=sorted(MISSIONS))
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--json', help='also write the data words as JSON (evidence)')
    args = parser.parse_args()
    prefix, name, data, gotos, bank, rope_sha, constants = build(args.mission)
    text = render(args.mission, prefix, name, data, gotos, bank, rope_sha, constants)
    output = MISSIONS[args.mission][3]
    if args.json:
        pathlib.Path(args.json).write_text(json.dumps(
            {'mission': args.mission, 'rope_sha256': rope_sha, 'data': data,
             'dntmgoto': [{'producer': l, 'word': w} for l, w in gotos]}, indent=1) + '\n')
    path = ROOT / output
    if args.check:
        if not path.exists() or path.read_text() != text:
            sys.exit(f'{output} is stale; run tools/downtelemetry_generate.py --mission {args.mission}')
        print(f'PASS: {output} matches {len(data)} {args.mission} data words')
        return
    path.write_text(text)
    print(f'wrote {output}: {len(data)} data words, {len(gotos)} DNTMGOTO words')


if __name__ == '__main__':
    main()
