#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map check for basic (non-interpretive) Luminary099 code
translated as routines of the control-transfer ABI (Lane F).

check() reassembles Luminary099 with the unchanged files copied in, using the
pinned yaYUL, and requires the rope to equal the certified fixed-data rope.
Every assembled word in the named address ranges then has exactly one native
owner, and the owner (after the C preprocessor, with the helpers it calls)
holds:
  - for an erasable operand, its address (the high word K of a double-word
    order, whose instruction word carries K+1);
  - for a fixed-memory data operand, the rope word at that address, so the
    native constant equals the original's;
  - for a TC/TCF/BZF/BZMF target, the target address, unless the owner
    composes it inline (INLINE);
  - for an I/O channel, the native channel field (CHANNELS);
  - for a constant (OCT/OCTAL/DEC/ADRES/...), its value.
EXTEND carries no operand. Words owned by a compiled site table are looked up
there instead. Returns the JSON-ready line map."""
import hashlib, pathlib, re, subprocess, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import assemble, listing_records, source_line, filename_alias, rope_word  # noqa: E402

BRANCH = {'TC', 'TCF', 'BZF', 'BZMF'}
NO_OPERAND = {'EXTEND', 'INHINT', 'RELINT', 'RESUME'}
CONSTANT = {'OCT', 'OCTAL', 'DEC', 'ADRES', 'CADR', 'FCADR', 'GENADR', '2DEC', '2OCT', 'BBCON', 'REMADR', '2CADR'}
# Quarter-coded orders take a 10-bit erasable address.
QUARTER = {'TS', 'XCH', 'LXCH', 'QXCH', 'ADS', 'DXCH', 'DAS', 'INCR', 'AUG', 'DIM', 'DV', 'CCS'}
DOUBLE = {'DCA', 'DCS', 'DXCH', 'DAS'}
READS = {'CA', 'CAF', 'CAE', 'CS', 'AD', 'MASK', 'MP', 'SU', 'DCA', 'DCS', 'CCS'}
IO = {'READ', 'WRITE', 'RAND', 'WAND', 'ROR', 'WOR', 'RXOR'}
# Alias forms the listing shows as their own operation.
ALIAS = {'COM': ('CS', 0), 'DOUBLE': ('AD', 0), 'ZL': ('LXCH', 7), 'ZQ': ('QXCH', 7), 'DCOM': ('DCS', 1),
         'DDOUBL': ('DAS', 1), 'SQUARE': ('MP', 0), 'RETURN': ('TC', 2), 'DTCB': ('DXCH', 5)}


def bodies(native):
    text = subprocess.run(['cc', '-E', '-P', '-Iinclude', '-Itests', native], cwd=R, check=True,
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


# Files this repository names differently from the assembler's MAIN.agc.
RENAMED = {'Q_R-AXIS_RCS_AUTOPILOT.agc': 'Q,R-AXES_RCS_AUTOPILOT.agc'}


def reassemble(files, work_name):
    root = R / 'build' / work_name
    assemble('Luminary099', root)
    work = root / 'Luminary099'
    includes = [line[1:].split()[0] for line in (work / 'MAIN.agc').read_text().splitlines() if line.startswith('$')]
    aliases, repairs = {}, {}
    for name in files:
        alias = RENAMED.get(name) or filename_alias(name, includes)
        text = (R.parent / 'Luminary099' / name).read_text()
        # The same indentation-only repair fixed_data_generate applies: a
        # '+n' location field written in the label column after a tab.
        normalized = re.sub(r'^\t([+-]\d+D?)\t', r' \1\t\t', text, flags=re.M)
        normalized = re.sub(r'^([+-]\d+D?)\t', r' \1\t', normalized, flags=re.M)
        repairs[name] = [n for n, (a, b) in enumerate(zip(text.splitlines(), normalized.splitlines()), 1) if a != b]
        (work / alias).write_text(normalized)
        aliases[alias] = 'Luminary099/' + name
    with (work / 'assembly.lst').open('w') as log:
        subprocess.run([str(R / 'deps/virtualagc/yaYUL/yaYUL'), 'MAIN.agc'], cwd=work, stdout=log,
                       stderr=subprocess.STDOUT, check=True)
    rope = (work / 'MAIN.agc.bin').read_bytes()
    if rope != (R / 'evidence/fixed-data-lm-original.rope.bin').read_bytes():
        raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
    return work, aliases, rope, repairs


def check(native, files, bank, owners, helpers, inline=None, channels=None, sites=None, work_name='basic-source'):
    """files: {name: None (whole file) or [(lo, hi)] address ranges}; owners:
    [(lo, hi, function)] in bank, or [(bank, lo, hi, function)] for a file
    that spans banks (bank is then a tuple of the banks allowed); sites:
    {function: path of a .inc table}."""
    inline, channels, sites = inline or {}, channels or {}, sites or {}
    work, aliases, rope, repairs = reassemble(list(files), work_name)
    funcs = bodies(native)
    texts = {f'Luminary099/{n}': (R.parent / 'Luminary099' / n).read_text() for n in files}
    records = listing_records(work / 'assembly.lst', aliases, texts, rope)
    for name, ranges in files.items():
        if ranges is None:
            path = 'Luminary099/' + name
            want = {i for i, line in enumerate(texts[path].splitlines(), 1) if source_line(line) is not None}
            if {r['line'] for r in records if r['path'] == path} != want:
                raise SystemExit('FAIL listing does not cover every source line of ' + name)

    def literals(name, seen=None):
        seen = seen if seen is not None else set()
        if name in seen:
            return set()
        seen.add(name)
        if name not in funcs:
            raise SystemExit('FAIL no native function ' + name)
        vals = numbers(funcs[name])
        for h in helpers.get(name, []):
            vals |= literals(h, seen)
        return vals

    def body_text(name, seen=None):
        seen = seen if seen is not None else set()
        if name in seen:
            return ''
        seen.add(name)
        return funcs[name] + ''.join(body_text(h, seen) for h in helpers.get(name, []))

    banks = bank if isinstance(bank, tuple) else (bank,)
    owners = [o if len(o) == 4 else (banks[0], *o) for o in owners]

    def fixed(addr, wbank=banks[0]):
        return 2048 + addr - 0o4000 if addr >= 0o4000 else wbank * 1024 + addr - 0o2000

    rows, owned = [], set()
    for r in records:
        if not r['words']:
            continue
        b, addr = r['assembled_address'].split(',')
        a = int(addr, 8)
        name = r['path'].split('/', 1)[1]
        ranges = files[name]
        if ranges is not None and not any(lo <= a <= hi for lo, hi in ranges):
            continue
        wbank = int(b, 8)
        if wbank not in banks:
            raise SystemExit(f"FAIL word outside bank {'/'.join(f'{x:o}' for x in banks)}: {r['assembled_address']}")
        owner = [o for ob, lo, hi, o in owners if ob == wbank and lo <= a <= hi]
        if len(owner) != 1:
            raise SystemExit(f'FAIL address {addr} has no single native owner')
        owner, op, args = owner[0], r['op'], r['arguments']
        if op.startswith('+') or op.startswith('-'):  # a '+n' location field parses as the operation
            op, _, args = args.partition(' ')
            op, args = op.strip(), args.strip()
        for k, w in enumerate(r['words']):
            if owner in sites:
                if not re.search(r'\{%du,0*%o\}' % (fixed(a + k, wbank), w), (R / sites[owner]).read_text()):
                    raise SystemExit(f'FAIL word {w:05o} at {wbank:o},{a + k:04o} is not in {sites[owner]}')
                continue
            if op in NO_OPERAND:
                continue
            lits = literals(owner)
            o = op
            if o in ALIAS:
                o, need = ALIAS[o]
                what = 'alias operand'
            elif o in CONSTANT:
                need, what = w, 'constant'
            elif o in IO:
                ch = w & 0o777
                if channels.get(ch) is None or channels[ch] not in body_text(owner):
                    raise SystemExit(f"FAIL line {r['line']} ({op} {args}) channel {ch:03o} is not used by {owner}")
                continue
            elif o in BRANCH:
                target = w & 0o7777
                if target in inline.get(owner, ()):
                    continue
                need, what = target, 'branch target'
            else:
                operand = w & (0o1777 if o in QUARTER else 0o7777)
                if o in DOUBLE:
                    operand -= 1
                if o in READS and operand >= 0o2000:
                    need, what = rope_word(rope, fixed(operand, wbank)), f'constant at {operand:05o}'
                else:
                    need, what = operand, 'erasable address'
            if need not in lits:
                raise SystemExit(f"FAIL {name}:{r['line']} ({op} {args}) {what} {need:05o} is not a literal of {owner}")
        owned.add(owner)
        rows.append(dict(path=r['path'], line=r['line'], address=r['assembled_address'],
                         source=(r['label'] + ' ' + r['op'] + ' ' + r['arguments']).strip(),
                         words=[f'{w:05o}' for w in r['words']], native=owner))
    missing = {o for _, _, _, o in owners} - owned
    if missing:
        raise SystemExit('FAIL native owners own no source: ' + ', '.join(sorted(missing)))
    mapped = {(r['path'], r['line']) for r in rows}
    fixes = {f'Luminary099/{n}': [k for k in v if (f'Luminary099/{n}', k) in mapped] for n, v in repairs.items()}
    fixes = {n: v for n, v in fixes.items() if v}
    return dict(sources={p: hashlib.sha256((R.parent / p).read_bytes()).hexdigest() for p in texts},
                rope_sha256=hashlib.sha256(rope).hexdigest(), native_path=native,
                native_sha256=hashlib.sha256((R / native).read_bytes()).hexdigest(),
                source_words=sum(len(r['words']) for r in rows), source_lines=len(rows),
                **({'indentation_only_repairs': fixes} if fixes else {}), line_map=rows), owned
