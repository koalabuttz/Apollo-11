#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile the inline operands of Comanche055 PHASE_TABLE_MAINTENANCE callers.

NEWMODEX and CHECKMM read one word after their caller's `TC` (the major mode).
PHASCHNG reads a packed phase word and then, depending on its bits, a new
priority or delta time and a new 2CADR; 2PHSCHNG reads one more word first.
The count follows the source's own decoding (PHSCHNG2, ONEORTWO, CHECKB,
CON1): the type bits 014000 select A (004000: phase bit 0100 asks for a new
priority) or B (014000 new priority, 010000 old); phase bit 0200 asks for a
2CADR. Each word comes from the unmodified source listing and is checked
against the rope; native code never reads the rope.

Also emitted: the fixed constants the file reads as data (CADR SETUPDSP at
MMDSPLAY+3, 2CADR PHSCHNG2 at ADRPCHN2, GENADR CON2 at CON2ADR).

  python3 tools/cm_phase_sites_generate.py           # write src/cm_phase_sites.inc
  python3 tools/cm_phase_sites_generate.py --check   # fail if stale
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

MISSION = 'Comanche055'
OUTPUT = 'src/cm_phase_sites.inc'
CALLEES = ('NEWMODEX', 'CHECKMM', 'PHASCHNG', '2PHSCHNG')
CONSTANTS = (('SETUPDSP_CADR', 'MMDSPLAY', 3, 1), ('ADRPCHN2', 'ADRPCHN2', 0, 2), ('CON2ADR', 'CON2ADR', 0, 1))


def phaschng_words(word):
    """Words PHASCHNG reads after the packed word, as the source decodes it."""
    kind, phase = word & 0o14000, (word & 0o17770) >> 3
    if not kind:
        return 0
    prio = (phase & 0o100) != 0 if kind == 0o4000 else kind == 0o14000
    return int(prio) + (2 if phase & 0o200 else 0)


def build(mission=MISSION, callees=CALLEES):
    base = ROOT / 'build/fixed-data' / mission
    listing = (base / 'assembly.lst').read_text(errors='replace')
    rope = (base / 'MAIN.agc.bin').read_bytes()
    table = symbol_table(listing)
    targets = {fixed_address(table[name]): name for name in callees}
    rows = list(listing_lines(listing))
    operands, sites = [], []
    for index, (path, line, address, words, label, op, opnd) in enumerate(rows):
        if op not in ('TC', 'TCR') or len(words) != 1 or words[0] not in targets or path.startswith('PHASE_TABLE'):
            continue
        callee = targets[words[0]]
        physical = fixed_address(address)
        assert physical is not None and physical >= 2048, (path, line)
        stream = []
        for r in rows[index + 1:]:
            if r[3]:
                stream.extend((fixed_address(r[2]) + k, w, f'{r[5]} {r[6]}'.strip()) for k, w in enumerate(r[3]))
            if len(stream) >= 6:
                break
        if callee == 'PHSCHNGA':
            count = 0   # the phase word is in A
        elif callee in ('NEWMODEX', 'CHECKMM'):
            count = 1
        elif callee == 'PHASCHNG':
            count = 1 + phaschng_words(stream[0][1])
        else:
            count = 2 + phaschng_words(stream[1][1])
        for k, (at, word, text) in enumerate(stream[:count]):
            assert at == physical + 1 + k, (path, line, k)
            assert rope_word(rope, at) == word, (path, line)
            operands.append({'physical': at, 'word': word, 'callee': callee, 'caller': f'{path}:{line}', 'operand': text})
        sites.append({'physical': physical + 1, 'callee': callee, 'words': count, 'caller': f'{path}:{line}'})
    # SETUPDSP's own TC NOVAC: the 2CADR DSPMMJOB that NOVAC reads at Q.
    for index, (path, line, address, words, label, op, opnd) in enumerate(rows):
        if path.startswith('PHASE_TABLE') and op == 'TC' and opnd.split()[:1] == ['NOVAC']:
            nxt = next(r for r in rows[index + 1:] if r[3])
            assert nxt[5] == '2CADR', nxt
            at = fixed_address(nxt[2])
            for k, word in enumerate(nxt[3]):
                assert rope_word(rope, at + k) == word
                operands.append({'physical': at + k, 'word': word, 'callee': 'NOVAC',
                                 'caller': f'{path}:{line}', 'operand': f'2CADR {nxt[6]}' + (' +1' if k else '')})
    operands.sort(key=lambda r: r['physical'])
    assert len({r['physical'] for r in operands}) == len(operands)
    constants = []
    for name, label, offset, count in CONSTANTS:
        at = fixed_address(table[label]) + offset
        constants.append((name, [rope_word(rope, at + k) for k in range(count)], at))
    # The constants are emitted words of PHASE_TABLE_MAINTENANCE itself.
    listed = {fixed_address(r[2]) + k: w for r in rows if r[0].startswith('PHASE_TABLE') and r[3]
              for k, w in enumerate(r[3])}
    for name, words, at in constants:
        assert [listed[at + k] for k in range(len(words))] == words, name
    return operands, sites, constants, hashlib.sha256(rope).hexdigest()


def render(operands, sites, constants, rope_sha, mission=MISSION, callees=CALLEES, prefix='CM_PHASE',
           name='cm_phase', tool='tools/cm_phase_sites_generate.py'):
    out = [f'/* SPDX-License-Identifier: MIT; generated by {tool}. */',
           f'/* {mission}: inline operands after every TC ' + ', '.join(callees[:-1]) + f' and {callees[-1]},',
           f' * keyed by physical fixed word. Oracle rope sha256 {rope_sha}. */']
    for cname, words, at in constants:
        bank, offset = divmod(at, 1024)
        out.append(f'#define {prefix}_{cname} ' + ','.join(f'0{w:05o}u' for w in words) + f' /* {bank:02o},{offset + 0o2000:04o} */')
    out.append(f'#define {prefix}_SITE_WORDS {len(operands)}u')
    out.append(f'static const agc_site_operand {name}_sites[{len(operands)}]={{')
    for row in operands:
        bank, offset = divmod(row['physical'], 1024)
        out.append(f" {{{row['physical']}u,0{row['word']:05o}}}, /* {bank:02o},{offset + 0o2000:04o} "
                   f"{row['callee']} {row['operand']} <- {row['caller']} */")
    out.append('};')
    out.append(f'#ifdef {prefix}_SITE_LIST /* tests: the first operand word of each site */')
    out.append(f'#define {prefix}_SITE_COUNT {len(sites)}u')
    short = prefix.split('_')[0] + '_PS_'
    out.append('enum { ' + ','.join(f'{short}{c}={i}' for i, c in enumerate(callees)) + ' };')
    out.append(f'static const struct {{ uint16_t physical; uint8_t callee,words; }} {name}_site_list[{len(sites)}]={{')
    for i in range(0, len(sites), 6):
        out.append(' ' + ''.join(f"{{{s['physical']}u,{callees.index(s['callee'])},{s['words']}}},"
                                for s in sites[i:i + 6]))
    out.append('};')
    out.append('#endif')
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--json', help='also write the operands and sites as JSON (evidence)')
    args = parser.parse_args()
    operands, sites, constants, rope_sha = build()
    text = render(operands, sites, constants, rope_sha)
    if args.json:
        pathlib.Path(args.json).write_text(json.dumps(
            {'mission': MISSION, 'rope_sha256': rope_sha, 'operands': operands, 'sites': sites,
             'constants': {n: {'physical': a, 'words': w} for n, w, a in constants}}, indent=1) + '\n')
    path = ROOT / OUTPUT
    if args.check:
        if not path.exists() or path.read_text() != text:
            sys.exit(f'{OUTPUT} is stale; run tools/cm_phase_sites_generate.py')
        print(f'PASS: {OUTPUT} matches {len(sites)} {MISSION} sites, {len(operands)} operand words')
        return
    path.write_text(text)
    print(f'wrote {OUTPUT}: {len(sites)} sites, {len(operands)} operand words')


if __name__ == '__main__':
    main()
