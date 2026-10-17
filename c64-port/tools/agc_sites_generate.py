#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile inline call-site operands into native constant tables.

AGC subroutines such as ALARM, BAILOUT, POODOO, BANKCALL and POSTJUMP read a
word that the caller placed right after its `TC`, through Q. A native caller
passes that word as a constant argument; this tool emits those constants for
every genuine `TC <callee>` site in the original program, keyed by the
physical fixed address of the operand word. Native code then never reads the
original rope to recover them.

Input is the host assembly listing of the unmodified source (built by
tools/setup_cloud.sh under build/fixed-data/<mission>/). Each operand is taken
from the source operand text (OCT/DEC/CADR...), checked against the listed
assembled word and against the binary rope. yaYUL is a build-time oracle only.

  python3 tools/agc_sites_generate.py cm-alarm            # write src/cm_alarm_sites.inc
  python3 tools/agc_sites_generate.py cm-alarm --check    # fail if stale
"""
import argparse
import difflib
import functools
import hashlib
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from fixed_data_generate import fixed_address, rope_word  # noqa: E402

# name -> (mission, callees that read an inline operand at Q, output, C symbol)
TABLES = {
    'cm-alarm': ('Comanche055', ('ALARM', 'ALARM2', 'BAILOUT', 'POODOO'),
                 'src/cm_alarm_sites.inc', 'cm_alarm_sites'),
    'cm-interbank': ('Comanche055', ('BANKCALL', 'IBNKCALL', 'POSTJUMP', 'USPRCADR'),
                     'src/cm_interbank_sites.inc', 'cm_interbank_sites'),
    'lm-interbank': ('Luminary099', ('BANKCALL', 'IBNKCALL', 'POSTJUMP', 'USPRCADR'),
                     'src/lm_interbank_sites.inc', 'lm_interbank_sites'),
    # Executive/Waitlist services: a 2CADR after NOVAC, FINDVAC, WAITLIST and
    # LONGCALL, an ADRES after TWIDDLE, the delay (DEC) after FIXDELAY.
    'lm-exec': ('Luminary099', ('NOVAC', 'FINDVAC', 'WAITLIST', 'LONGCALL', 'TWIDDLE', 'FIXDELAY'),
                'src/lm_exec_sites.inc', 'lm_exec_sites'),
    'cm-exec': ('Comanche055', ('NOVAC', 'FINDVAC', 'WAITLIST', 'LONGCALL', 'TWIDDLE', 'FIXDELAY'),
                'src/cm_exec_sites.inc', 'cm_exec_sites'),
    # SERVICE_ROUTINES: the flag descriptor (ADRES or a named-flag constant)
    # after UPFLAG and DOWNFLAG, the source and destination ADRES after GENTRAN
    # (and after GENTRAN +1, which AOSTASK calls with A already set).
    'lm-service': ('Luminary099', ('UPFLAG', 'DOWNFLAG', 'GENTRAN', 'GENTRAN +1'), 'src/lm_service_sites.inc',
                   'lm_service_sites'),
    'cm-service': ('Comanche055', ('UPFLAG', 'DOWNFLAG', 'GENTRAN', 'GENTRAN +1'), 'src/cm_service_sites.inc',
                   'cm_service_sites'),
}
# Tables the C64 flight resident holds (platform/c64/resident_flight.c): a
# flight build (AGC_SITES_ROPE) keeps only their addresses and reads each
# word from the REU rope image, 2 bytes a site instead of 4.
ADDRESS_TABLES = {'lm-exec', 'cm-exec'}
# Callees whose operand is two words (2CADR, or an OCT pair).
TWO_WORD = {'NOVAC', 'FINDVAC', 'WAITLIST', 'LONGCALL', 'GENTRAN', 'GENTRAN +1'}
# `TC <service>` words that are data, not calls: PINBALL keeps them as
# constants to store into erasable (TCNOVAC, TCWAIT, TCFINDVC).
CONSTANT_WORDS = {'TCNOVAC', 'TCWAIT', 'TCFINDVC'}
LINE = re.compile(r'^\d{6},(\d{6}): (.*)$')


def listing_lines(listing):
    """Yield (file, line, address, words, label, operation, operands)."""
    current = 'MAIN.agc'
    for text in listing.splitlines():
        if text.startswith('(End of include-file '):
            current = 'MAIN.agc'
            continue
        match = LINE.match(text)
        if not match:
            continue
        if match[2].startswith('$'):
            current = match[2][1:].split()[0]
            continue
        words = [int(w, 8) for w in text[33:44].split() if re.fullmatch('[0-7]{5}', w)]
        yield (current, int(match[1]), text[14:23].strip(), words, text[45:65].strip(),
               text[65:74].strip(), text[74:].split('#', 1)[0].strip())


@functools.lru_cache(maxsize=None)
def source_files(mission):
    """Assembled copy's file names -> repository names (a few are spelled differently)."""
    names = sorted(p.name for p in (ROOT.parent / mission).glob('*.agc'))
    key = {re.sub('[^A-Z0-9]', '', n[:-4].upper()): n for n in names}

    def match(n):
        k = re.sub('[^A-Z0-9]', '', n[:-4].upper())
        hits = [v for c, v in key.items() if c == k] or [v for c, v in key.items() if k.startswith(c)]
        return (hits or difflib.get_close_matches(n, names, 1, 0.8) or [None])[0]
    return {p.name: match(p.name) for p in (ROOT / 'build/fixed-data' / mission).glob('*.agc')}


@functools.lru_cache(maxsize=None)
def source_lines(mission, path):
    """Map a listing line (of the assembled copy under build/fixed-data) to the
    line of the repository source, whose header comments and indentation
    differ. A line that differs takes the offset of the last matching one."""
    def words(p):
        return [' '.join(x.split()) for x in p.read_text(errors='replace').splitlines()]
    built = words(ROOT / 'build/fixed-data' / mission / path)
    source = words(ROOT.parent / mission / source_files(mission)[path])
    out, delta = {}, 0
    blocks = difflib.SequenceMatcher(None, built, source, autojunk=False).get_matching_blocks()
    starts = {a: b - a for a, b, n in blocks if n}
    for k in range(len(built)):
        delta = starts.get(k, delta)
        out[k + 1] = k + 1 + delta
    return out


def source_line(mission, path, line):
    return f'{source_files(mission)[path]}:{source_lines(mission, path)[line]}'


def symbol_table(listing):
    return {name: value for name, value in re.findall(r'\b\d+,F:\s+(\S+)\s+(\S+)', listing)}


def fixed_fixed(value):
    """Callee entry addresses of interest live in fixed-fixed memory."""
    address = fixed_address(value)
    assert address is not None and 0o4000 <= address < 0o10000, value
    return address


def sites(mission, callees):
    base = ROOT / 'build/fixed-data' / mission
    listing = (base / 'assembly.lst').read_text(errors='replace')
    rope = (base / 'MAIN.agc.bin').read_bytes()
    table = symbol_table(listing)
    # A callee 'NAME +k' is a call k words into NAME.
    targets = {fixed_fixed(table[name.split()[0]]) + int(name.split('+')[1] if '+' in name else 0): name
               for name in callees}
    rows = list(listing_lines(listing))
    result = []
    for index, (path, line, address, words, label, op, operands) in enumerate(rows):
        if op != 'TC' or len(words) != 1 or words[0] not in targets:
            continue
        if label in CONSTANT_WORDS:
            continue
        physical = fixed_address(address)
        assert physical is not None, (path, line, address)
        callee = targets[words[0]]
        # The operand is the next emitted word(s) in the same location counter.
        following = (r for r in rows[index + 1:] if r[3])
        nxt = next(following)
        n_path, n_line, n_address, n_words, _l, n_op, n_operands = nxt
        operand_physical = fixed_address(n_address)
        assert operand_physical == physical + 1, (path, line, n_address)
        if callee in TWO_WORD:
            if n_op == '2CADR':
                assert len(n_words) == 2, (n_path, n_line)
                pair = [(n_words[0], f'2CADR {n_operands}'), (n_words[1], f'2CADR {n_operands} +1')]
            elif callee.startswith('GENTRAN'):
                second = next(following)
                assert len(n_words) == 1 and len(second[3]) == 1, (n_path, n_line)
                assert fixed_address(second[2]) == operand_physical + 1, (n_path, n_line)
                pair = [(n_words[0], f'{n_op} {n_operands}'.strip()), (second[3][0], f'{second[5]} {second[6]}'.strip())]
            else:
                second = next(following)
                assert n_op == 'OCT' and second[5] == 'OCT' and len(n_words) == 1, (n_path, n_line)
                assert fixed_address(second[2]) == operand_physical + 1, (n_path, n_line)
                pair = [(n_words[0], f'OCT {n_operands}'), (second[3][0], f'OCT {second[6]}')]
                for word, text in pair:
                    assert int(text.split()[1], 8) == word, (n_path, n_line)
            operands_out = [(operand_physical + k, word, text) for k, (word, text) in enumerate(pair)]
        else:
            assert len(n_words) == 1, (n_path, n_line)
            word = n_words[0]
            if n_op in ('OCT', 'DEC'):
                text = n_operands.split()[0]
                value = int(text.rstrip('D'), 10 if (n_op == 'DEC' or text.endswith('D')) else 8)
                assert value == word, (n_path, n_line, n_operands, oct(word))
            operands_out = [(operand_physical, word, f'{n_op} {n_operands}'.strip())]
        for at, word, text in operands_out:
            assert rope_word(rope, at) == word, (n_path, n_line)
            result.append({'physical': at, 'word': word, 'callee': callee,
                           'callee_address': words[0],
                           'caller': source_line(mission, path, line), 'operand': text})
    result.sort(key=lambda r: r['physical'])
    assert len({r['physical'] for r in result}) == len(result)
    return result, hashlib.sha256(rope).hexdigest()


def render(name, mission, symbol, rows, rope_sha):
    out = [f'/* SPDX-License-Identifier: MIT; generated by tools/agc_sites_generate.py {name}. */',
           f'/* {mission}: every genuine TC site whose callee reads an inline operand at Q. */',
           f'/* Keyed by physical fixed word (bank*1024+offset). Oracle rope sha256 {rope_sha}. */',
           f'#define {symbol.upper()}_COUNT {len(rows)}u']
    if name in ADDRESS_TABLES:
        out.append('#ifdef AGC_SITES_ROPE /* addresses only; each word is read from the rope (agc_transfer.h) */')
        out.append(f'static const uint16_t {symbol}_physical[{len(rows)}]={{')
        for i in range(0, len(rows), 10):
            out.append(' ' + ','.join(f"{r['physical']}u" for r in rows[i:i + 10]) + ',')
        out.append('};')
        out.append('#endif')
        out.append('#if !defined(AGC_SITES_ROPE) || defined(AGC_SITES_CALLEES) /* tests read the words too */')
    out.append(f'static const agc_site_operand {symbol}[{len(rows)}]={{')
    for row in rows:
        bank, offset = divmod(row['physical'], 1024)
        out.append(f" {{{row['physical']}u,0{row['word']:05o}}}, /* {bank:02o},{offset + 0o2000:04o} "
                   f"{row['callee']} {row['operand']} <- {row['caller']} */")
    out.append('};')
    if name in ADDRESS_TABLES:
        out.append('#endif')
    out.append('#ifdef AGC_SITES_CALLEES /* tests: the TC target of each site */')
    out.append(f'static const agc_word {symbol}_callee[{len(rows)}]={{')
    for i in range(0, len(rows), 10):
        out.append(' ' + ','.join(f"0{r['callee_address']:05o}" for r in rows[i:i + 10]) + ',')
    out.append('};')
    out.append('#endif')
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('table', choices=sorted(TABLES))
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--json', help='also write the site list as JSON (evidence)')
    args = parser.parse_args()
    mission, callees, output, symbol = TABLES[args.table]
    rows, rope_sha = sites(mission, callees)
    text = render(args.table, mission, symbol, rows, rope_sha)
    path = ROOT / output
    if args.json:
        pathlib.Path(args.json).write_text(json.dumps(
            {'mission': mission, 'callees': callees, 'rope_sha256': rope_sha, 'sites': rows},
            indent=1) + '\n')
    if args.check:
        if not path.exists() or path.read_text() != text:
            sys.exit(f'{output} is stale; run tools/agc_sites_generate.py {args.table}')
        print(f'PASS: {output} matches {len(rows)} {mission} sites')
        return
    path.write_text(text)
    print(f'wrote {output}: {len(rows)} sites')


if __name__ == '__main__':
    main()
