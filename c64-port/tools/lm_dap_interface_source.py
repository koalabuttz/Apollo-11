#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly check for the native
Luminary099 DAP_INTERFACE_SUBROUTINES (src/lm_dap_interface.c).

Reassembles Luminary099 with the unchanged file copied in, using the pinned
yaYUL, and requires the rope
to equal the certified fixed-data rope. Every assembled word of
DAP_INTERFACE_SUBROUTINES.agc then has exactly one native owner, and the
owner (after the C preprocessor, together with the helpers it calls) holds:
  - for an erasable operand (DAPBOOLS, DB, RUPTREG1, Q, EBANK...), the address;
  - for a fixed-memory data operand (CAF, MASK, CS of a constant), the rope
    word at that address, so the native constant equals the original's;
  - for a TC/TCF/BZF target, the target address, unless the owner composes
    the target inline (INLINE below);
  - for an OCTAL/DEC constant, its value.
EXTEND words carry no operand. The 2CADR 1/ACCJOB is the inline NOVAC
operand: it must be in the compiled site table src/lm_exec_sites.inc with the
same word. Writes build/lm-dap-interface-source-map.json."""
import hashlib, json, pathlib, re, shutil, subprocess, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import assemble, listing_records, source_line, filename_alias, rope_word  # noqa: E402

MISSION, NAME = 'Luminary099', 'DAP_INTERFACE_SUBROUTINES.agc'
PATH = MISSION + '/' + NAME
ORIGINAL = R.parent / PATH
NATIVE = 'src/lm_dap_interface.c'
# Bank-20 address ranges and the native function that compiles them.
OWNERS = [(0o2123, 0o2123, 'restordb'), (0o2124, 0o2126, 'restordb_a'), (0o2127, 0o2127, 'setmaxdb'),
          (0o2130, 0o2132, 'set_db'), (0o2133, 0o2134, 'callaccs'), (0o2135, 0o2136, 'lm_exec_sites'),
          (0o2137, 0o2137, 'accreturn'), (0o2140, 0o2141, 'setmindb'), (0o2142, 0o2144, 'pflitedb'),
          (0o2145, 0o2147, 'pflite_return'), (0o2150, 0o2150, 'setmindb'), (0o2151, 0o2151, 'setmaxdb'),
          (0o2152, 0o2152, 'pflite_return'), (0o2153, 0o2164, 'zatteror'), (0o2165, 0o2167, 'save_eb'),
          (0o2170, 0o2203, 'clear_rates'), (0o2204, 0o2206, 'allcoast'), (0o2207, 0o2224, 'allcoast_clear'),
          (0o2225, 0o2225, 'allcoast_return')]
HELPERS = {'restordb': ['restordb_a'], 'restordb1': ['restordb_a'], 'restordb_a': ['set_db'], 'set_db': ['qxch', 'callaccs'],
           'setmaxdb': ['set_db'], 'setmindb': ['set_db'], 'accreturn': ['tc_indirect'], 'pflitedb': ['qxch'],
           'pflite_return': ['callaccs'], 'zatteror': ['save_eb', 'clear_rates', 'ea'], 'save_eb': ['xch_eb', 'set_eb'],
           'clear_rates': ['ea', 'xch_eb', 'set_eb', 'tc_indirect'], 'allcoast': ['qxch'],
           'allcoast_clear': ['ea', 'xch_eb', 'set_eb'], 'allcoast_return': ['tc_indirect'], 'xch_eb': ['set_eb']}
# Branch targets inside the file that the owner composes inline rather than
# entering as a routine: RESTORDB+1's BZF picks NARROWDB (SETMINDB) or falls
# into SETMAXDB; SETMINDB's TCF SETMAXDB +1 and PFLITEDB's TCF CALLACCS are
# set_db and callaccs; ZATTEROR's TCF STOPRATE +3 is clear_rates.
INLINE = {'restordb_a': {0o2140}, 'setmindb': {0o2130}, 'pflite_return': {0o2133}, 'zatteror': {0o2170}}
BRANCH = {'TC', 'TCF', 'BZF'}
NO_OPERAND = {'EXTEND'}
CONSTANT = {'OCTAL', 'DEC'}


def bodies():
    text = subprocess.run(['cc', '-E', '-P', '-Iinclude', '-Itests', NATIVE], cwd=R, check=True,
                          capture_output=True, text=True).stdout
    out = {}
    for m in re.finditer(r'^static [^;{]*?\b(\w+)\([^)]*\)\{', text, re.M):
        depth = 0
        for i in range(m.end() - 1, len(text)):
            depth += text[i] == '{'
            depth -= text[i] == '}'
            if depth == 0:
                out[m[1]] = text[m.start():i + 1]
                break
    return out


def numbers(text):
    vals = set()
    for x in re.findall(r'\b(0x[0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)[uUlL]*\b', text):
        vals.add(int(x, 16) if x.startswith('0x') else int(x, 8) if x.startswith('0') else int(x))
    return vals


def main():
    root = R / 'build/dap-source'
    assemble(MISSION, root)
    work = root / MISSION
    includes = [line[1:].split()[0] for line in (work / 'MAIN.agc').read_text().splitlines() if line.startswith('$')]
    alias = filename_alias(NAME, includes)
    shutil.copyfile(ORIGINAL, work / alias)
    with (work / 'assembly.lst').open('w') as log:
        subprocess.run([str(R / 'deps/virtualagc/yaYUL/yaYUL'), 'MAIN.agc'], cwd=work, stdout=log,
                       stderr=subprocess.STDOUT, check=True)
    rope = (work / 'MAIN.agc.bin').read_bytes()
    if rope != (R / 'evidence/fixed-data-lm-original.rope.bin').read_bytes():
        raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
    text = ORIGINAL.read_text()
    records = listing_records(work / 'assembly.lst', {alias: PATH}, {PATH: text}, rope)
    if {r['line'] for r in records} != {i for i, line in enumerate(text.splitlines(), 1) if source_line(line) is not None}:
        raise SystemExit('FAIL listing does not cover every source line')
    funcs = bodies()
    sites = (R / 'src/lm_exec_sites.inc').read_text()

    def literals(name, seen=None):
        seen = seen or set()
        if name in seen:
            return set()
        seen.add(name)
        if name not in funcs:
            raise SystemExit('FAIL no native function ' + name)
        vals = numbers(funcs[name])
        for h in HELPERS.get(name, []):
            vals |= literals(h, seen)
        return vals

    def fixed(addr):
        if addr >= 0o4000:
            return 2048 + addr - 0o4000
        return 0o20 * 1024 + addr - 0o2000  # bank 20, this file's FB

    rows, owned = [], set()
    for r in records:
        if not r['words']:
            continue
        bank, addr = r['assembled_address'].split(',')
        a = int(addr, 8)
        if bank != '20':
            raise SystemExit('FAIL word outside bank 20: ' + r['assembled_address'])
        owner = [o for lo, hi, o in OWNERS if lo <= a <= hi]
        if len(owner) != 1:
            raise SystemExit(f'FAIL address {addr} has no single native owner')
        owner, op, args = owner[0], r['op'], r['arguments']
        if op.startswith('+'):  # a '+n' location field parses as the operation
            op, _, args = args.partition(' ')
        for k, w in enumerate(r['words']):
            if owner == 'lm_exec_sites':
                if not re.search(r'\{%du,0*%o\}' % (fixed(a + k), w), sites):
                    raise SystemExit(f'FAIL 2CADR word {w:05o} at 20,{a + k:04o} is not in lm_exec_sites')
                continue
            if op in NO_OPERAND:
                continue
            lits = literals(owner)
            if op in CONSTANT:
                need, what = w, 'constant'
            elif op in BRANCH:
                target = w & 0o7777
                if target in INLINE.get(owner, ()):
                    continue
                need, what = target, 'branch target'
            else:
                operand = w & (0o1777 if op in ('QXCH', 'LXCH', 'XCH', 'TS', 'ADS') else 0o7777)
                if op in ('CAF', 'MASK', 'CS', 'CA', 'CAE') and operand >= 0o2000:
                    need, what = rope_word(rope, fixed(operand)), f'constant at {operand:05o}'
                else:
                    need, what = operand, 'erasable address'
            if need not in lits:
                raise SystemExit(f"FAIL line {r['line']} ({op} {args}) {what} {need:05o} is not a literal of {owner}")
        owned.add(owner)
        rows.append(dict(line=r['line'], address=r['assembled_address'],
                         source=(r['label'] + ' ' + r['op'] + ' ' + r['arguments']).strip(),
                         words=[f'{w:05o}' for w in r['words']], native=owner))
    missing = {o for _, _, o in OWNERS} - owned
    if missing:
        raise SystemExit('FAIL native owners own no source: ' + ', '.join(sorted(missing)))
    native = (R / NATIVE).read_bytes()
    result = dict(source_path=PATH, source_sha256=hashlib.sha256(ORIGINAL.read_bytes()).hexdigest(),
                  rope_sha256=hashlib.sha256(rope).hexdigest(), native_path=NATIVE,
                  native_sha256=hashlib.sha256(native).hexdigest(),
                  source_words=sum(len(r['words']) for r in rows), source_lines=len(rows), line_map=rows,
                  note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every assembled '
                       'word has one native owner holding its erasable address, the rope value of its fixed '
                       'operand, its branch target (or composes it inline) or its constant; the 1/ACCJOB 2CADR is '
                       'the compiled NOVAC site operand.')
    (R / 'build/lm-dap-interface-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', len(rows), 'source lines map to', len(owned),
          'native owners', flush=True)


if __name__ == '__main__':
    main()
