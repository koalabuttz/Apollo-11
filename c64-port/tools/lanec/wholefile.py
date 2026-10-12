#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-file source-to-native map for DISPLAY_INTERFACE_ROUTINES.agc
(Luminary099 and Comanche055), the evidence behind its full-file
certificate.

  python3 tools/lanec/wholefile.py OUTDIR

For each mission it reads every assembled word of the file from the pinned
listing, assigns it to the Lane C milestone whose native module translates
it (by label interval) or to the file's constant table, and requires:
  - every word has exactly one owner and the intervals cover the file;
  - every instruction word (anything but OCT/DEC/VN/CADR/2CADR/GENADR/ADRES
    data) was started by the unchanged original program in Virtual AGC under
    some case of the Lane C display suites (LANEC_TRACE of the oracle), whose
    complete results the native code matches (tools/lanec/diff.py);
  - every constant-table word the native code reads is a symbols.txt WORD row.
Each instruction row names the C function that carries it (the static or
exported function named after its label, else the module's dispatch) and,
when the word is a transfer-ABI entry (src/lanec_di_routines.inc), the
routine table and physical address it is registered at. EQUALS aliases of
the file's code labels get alias rows naming the label they stand for.
Writes OUTDIR/lanec-di-{lm,cm}-source-map.json.
"""
import hashlib, json, os, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/lanec'))
import symbols  # noqa: E402

FILE = 'DISPLAY_INTERFACE_ROUTINES.agc'
# the include name in the listing, where it differs from FILE
LISTING = {'PINBALL_GAME_BUTTONS_AND_LIGHTS.agc': 'PINBALL_GAME__BUTTONS_AND_LIGHTS.agc'}
SUITES = ('di-dsp', 'di-mark', 'di-play', 'di-sleep', 'di-idle', 'di-nvdsp')
MODULE = {'lanec-di-dsp': 'src/lanec_di_dsp.c', 'lanec-di-mark': 'src/lanec_di_mark.c',
          'lanec-di-play': 'src/lanec_di_play.c', 'lanec-di-sleep': 'src/lanec_di_sleep.c',
          'lanec-di-idle': 'src/lanec_di_idle.c', 'lanec-di-nvdsp': 'src/lanec_di_nvdsp.c'}
# (first label, last label, offset from it, owner), in address order per bank.
INTERVALS = (('BLANKET', 'CLEARMRK', 7, 'lanec-di-mark'),
             ('MARKEND', 'COPYPACS', 7, 'lanec-di-mark'),
             ('CHKPRIO', 'PRIODSP', 2, 'lanec-di-dsp'),
             ('MAKEPRIO', 'PRIOPLAY', 1, 'lanec-di-play'),
             ('EXDSPRET', 'SKIPADD', 3, 'lanec-di-dsp'),
             ('GOSLEEPS', 'INITDSP', 8, 'lanec-di-sleep'),
             ('PINBRNCH', 'V50PASTE', 1, 'lanec-di-nvdsp'),
             ('WITCHONE', 'RELINTQ', 1, 'lanec-di-play'),
             ('CLEANEND', 'TERMATE', 1, 'lanec-di-idle'),
             ('LINUSCHR', 'LINUSCHR', 12, 'lanec-di-play'),
             ('PROCEED', 'ERASER', 3, 'lanec-di-idle'))
TABLE = 'PERFMASK'  # the constant table runs from here to the end of the file
DATA = {'OCT', 'DEC', '2DEC', 'VN', 'CADR', '2CADR', 'GENADR', 'ADRES', 'ECADR'}
ORIGINAL = {'LM': 'Luminary099', 'CM': 'Comanche055'}
FIXTURE = {'LM': 'lm-powered-fixture', 'CM': 'cm-powered-fixture'}
ROPE = {'LM': '1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e',
        'CM': '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'}


def sha(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def norm(text, mission):
    code = ' '.join(text.split('#', 1)[0].split())
    if mission == 'CM':
        code = code.replace('CAF EBANKSAV', 'CA EBANKSAV')
    # The transcribed Luminary099 Pinball reads TC DSPOCTWO; the rope assembles DSPOCTWD.
    code = re.sub(r'\bDSPOCTWO\b', 'DSPOCTWD', code)
    # yaYUL lists a relative address +8 as +8D
    return re.sub(r'(?<=\s)([+-]\d+)D(?=\s|$)', r'\1', code)


def listing_words(mission):
    lines = (ROOT / 'deps' / FIXTURE[mission] / 'assembly.lst').read_text(errors='replace').splitlines()
    name = LISTING.get(FILE, FILE)
    start = next(i for i, l in enumerate(lines) if '$' + name in l)
    end = next(i for i, l in enumerate(lines) if f'End of include-file {name}' in l)
    rows = []
    for l in lines[start:end]:
        m = re.match(r'^\d+,\d+:\s+(?:([0-7]{2}),)?([0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?\s+(.*)$', l)
        if m:
            words = [m.group(3)] + ([m.group(4)] if m.group(4) else [])
            rows.append({'bank': m.group(1), 'addr': int(m.group(2), 8), 'words': words, 'text': m.group(5)})
    return rows


def original_lines(mission, rows):
    """Line number in the unchanged original file of each listing row."""
    src = (ROOT.parent / ORIGINAL[mission] / FILE).read_text(errors='replace').splitlines()
    i = 0
    for r in rows:
        want = norm(r['text'], mission).split()
        # yaYUL annotates some data (DEC 22 B-14): the source fields come first.
        while i < len(src) and not (norm(src[i], mission) and want[:len(norm(src[i], mission).split())]
                                    == norm(src[i], mission).split()):
            i += 1
        if i == len(src):
            sys.exit(f'FAIL: {mission} listing row not in the original file: {r["text"]}')
        r['line'] = i + 1
        i += 1


def trace(mission):
    done = set()
    env = dict(os.environ)
    for suite in SUITES:
        out = ROOT / f'build/lanec-{suite}-trace.txt'
        env['LANEC_TRACE'] = str(out)
        subprocess.run(['build/lanec-oracle', suite, 'deps/lm-powered-fixture/MAIN.agc.bin',
                        'deps/cm-powered-fixture/MAIN.agc.bin'], cwd=ROOT, env=env, check=True,
                       stdout=subprocess.DEVNULL)
        for line in out.read_text().split('\n'):
            if line.startswith(mission + ' '):
                bank, addr = line[3:].split(',')
                done.add((None if bank == '0' else f'{int(bank, 8):02o}', int(addr, 8)))
    return done


def readers(mission, name):
    """Other source files of the program whose code fields name NAME."""
    found = []
    for path in sorted((ROOT.parent / ORIGINAL[mission]).glob('*.agc')):
        for i, line in enumerate(path.read_text(errors='replace').splitlines()):
            fields = line.split('#', 1)[0].split()
            if name in fields[1:] and not (path.name == FILE and fields[0] == name):
                found.append(f'{path.name}:{i + 1}')
    return found


def read_addresses(mission, table, native):
    """Fixed addresses whose words the native modules use as compiled
    constants: symbols.txt WORD rows referenced by the native sources."""
    found = {}
    for row in (ROOT / 'tools/lanec/symbols.txt').read_text().splitlines():
        f = row.split('#', 1)[0].split()
        if len(f) < 3 or f[0] != mission or f[1] != 'WORD':
            continue
        for name in f[2:]:
            base, _, off = name.partition('+')
            if name in table:
                base, off = name, ''
            if base not in table or (off and not off.isdigit()):
                continue
            c = symbols.cname(name)
            if f'LANEC_{mission}_{c}_WORD' in native or f'LANEC_LM_{c}_WORD' in native or f'W({c})' in native \
                    or f'_{mission}_{c}_WORD' in native or f'##_{c}_WORD' in native:
                _, bank, addr = table[base]
                found.setdefault((bank, addr + int(off or 0)), name)
    return found


def abi_entries(mission, family='di', interpretive=False):
    """Physical address -> wrapper of every transfer-ABI routine registered for
    MISSION (basic, or interpretive keyed by LOC) in src/lanec_<family>_routines.inc."""
    text = (ROOT / f'src/lanec_{family}_routines.inc').read_text()
    name = f'{mission.lower()}_{"interp_" if interpretive else ""}entries['
    if name not in text:
        return {}
    block = text[text.index(name):]
    block = block[:block.index('};')]
    return {int(a): w for a, w in re.findall(r'\{ (\d+)u, (\w+) \}', block)}


def physical(bank, addr):
    return 2048 + addr - 0o4000 if bank is None else int(bank, 8) * 1024 + addr - 0o2000


def c_function(module, label):
    """The C function of MODULE named after LABEL, else the module's dispatch."""
    text = (ROOT / module).read_text()
    stem = pathlib.Path(module).stem             # lanec_di_dsp
    name = symbols.cname(label).lower()
    for fn in (f'{stem.rsplit("_", 1)[0]}_{name}', name):
        if re.search(rf'^(?:static )?[a-z_ ]+\b{fn}\(', text, re.M):
            return fn
    return f'{stem}_dispatch'


def aliases(mission, table, line_map):
    """EQUALS rows of the file whose target is one of its code labels."""
    src = (ROOT.parent / ORIGINAL[mission] / FILE).read_text(errors='replace').splitlines()
    by_address = {e['address']: e for e in line_map}
    rows = []
    for i, line in enumerate(src):
        f = line.split('#', 1)[0].split()
        if len(f) != 3 or f[1] != 'EQUALS' or f[2] not in table:
            continue
        _, bank, addr = table[f[2]]
        e = by_address.get(f'{bank or "FF"},{addr:04o}')
        if e and e['kind'] == 'instruction':
            rows.append(dict(line=i + 1, alias=f[0], equals=f[2], address=e['address'], owner=e['owner'],
                             native=e['native'], **({'abi_routine': e['abi_routine']} if 'abi_routine' in e else {})))
    return rows


def build(mission):
    table = symbols.symbols(FIXTURE[mission])
    rows = listing_words(mission)
    original_lines(mission, rows)
    executed = trace(mission)
    native = '\n'.join((ROOT / p).read_text() for p in MODULE.values())
    spans = []
    for first, last, off, owner in INTERVALS:
        _, b0, a0 = table[first]
        _, b1, a1 = table[last]
        if b0 != b1:
            sys.exit(f'FAIL: {first}..{last} crosses banks')
        spans.append((b0, a0, a1 + off, owner, first))
    _, tb, ta = table[TABLE]
    src = (ROOT.parent / ORIGINAL[mission] / FILE).read_text(errors='replace').splitlines()
    read = read_addresses(mission, table, native)
    abi = abi_entries(mission)
    owners, line_map, label = {}, [], None
    for r in rows:
        key = (r['bank'], r['addr'])
        fields = r['text'].split('#', 1)[0].split()
        labelled = not src[r['line'] - 1][:1].isspace()
        if labelled:
            label = fields[0]
        op = next((f for f in fields if f in DATA), None) if fields else None
        hits = [s for s in spans if s[0] == r['bank'] and s[1] <= r['addr'] <= s[2]]
        if hits:
            if len(hits) > 1:
                sys.exit(f'FAIL: {mission} {r["addr"]:o} has two owners')
            owner, routine = hits[0][3], hits[0][4]
            ran = key in executed
            if not op and not ran:
                sys.exit(f'FAIL: {mission} instruction {r["bank"]},{r["addr"]:04o} "{r["text"].strip()}" never executed')
            entry = {'kind': 'instruction' if not op else 'inline data', 'owner': owner,
                     'native': f'{MODULE[owner]}: {c_function(MODULE[owner], label)} ({routine} block)',
                     'executed_by_oracle': ran}
            at = physical(r['bank'], r['addr'])
            if not op and at in abi:
                entry['abi_routine'] = f'lanec_di_{mission.lower()}_routines[{at}] -> {abi[at]}'
        elif r['bank'] == tb and r['addr'] >= ta:
            if not op:
                sys.exit(f'FAIL: {mission} non-data word in the constant table: {r["text"]}')
            name = label if labelled else f'{label} (following word)'
            if key in read:
                how = f'compiled constant {read[key]} (symbols.txt WORD row)'
            else:
                users = readers(mission, label) if labelled else []
                how = ('preserved in the rope; read only by ' + ', '.join(users) if users else
                       'preserved in the rope; no source line of the program refers to it')
                if not labelled:
                    sys.exit(f'FAIL: {mission} unlabelled constant {r["addr"]:o} is not read as a WORD row')
            entry = {'kind': 'constant', 'owner': 'constant table', 'native': how, 'constant': name}
            owner = 'constant table'
        else:
            sys.exit(f'FAIL: {mission} word {r["bank"]},{r["addr"]:04o} has no owner: {r["text"]}')
        owners[owner] = owners.get(owner, 0) + len(r['words'])
        line_map.append(dict(line=r['line'], address=f'{r["bank"] or "FF"},{r["addr"]:04o}',
                             source=' '.join(fields), words=r['words'], **entry))
    total = sum(len(r['words']) for r in rows)
    instr = [e for e in line_map if e['kind'] == 'instruction']
    return {
        'source_path': f'{ORIGINAL[mission]}/{FILE}',
        'source_sha256': sha(ROOT.parent / ORIGINAL[mission] / FILE),
        'rope_sha256': ROPE[mission],
        'native': {p: sha(ROOT / p) for p in sorted(set(MODULE.values()) | {'src/lanec.c'})},
        'source_words': total,
        'source_lines_with_words': len(rows),
        'line_map': line_map,
        'aliases': aliases(mission, table, line_map),
        'abi_entries': len(abi),
        'covered_ranges': [f'{b or "FF"},{lo:04o}-{b or "FF"},{hi:04o} {owner}' for b, lo, hi, owner, _ in spans]
                          + [f'{tb},{ta:04o}-end constant table'],
        'whole_file': {'assembled_words': total, 'words_by_owner': dict(sorted(owners.items())),
                       'instruction_words': sum(len(e['words']) for e in instr),
                       'instruction_words_executed_by_oracle': sum(len(e['words']) for e in instr
                                                                   if e['executed_by_oracle'])},
        'note': ('Every assembled word of the file is owned by one Lane C display milestone or by the '
                 'constant table at its end. Every instruction word was started by the unchanged original '
                 'program in Virtual AGC under at least one case of the six display suites (oracle '
                 'LANEC_TRACE), and the native modules reproduce those cases\' complete results '
                 '(2048 erasable words, registers, channels, superbank, interrupt state). Inline data words '
                 '(ADRES/OCT/2CADR/GENADR after a call) are the arguments of the call before them, compiled '
                 'into the native call. Every labelled entry and every return into the file from a callee is a '
                 'transfer-ABI routine (abi_routine; tools/lanec/routines.py di checks this against the listing), '
                 'and EQUALS aliases of code labels resolve to those rows. Words before CHKPRIO/after COPYPACS and the GOMARK family use the '
                 'display-start paths of lanec-di-dsp through include/lanec_di_internal.h.'),
    }


if __name__ == '__main__':
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'build/di-cert')
    out.mkdir(parents=True, exist_ok=True)
    for mission in ('LM', 'CM'):
        data = build(mission)
        path = out / f'lanec-di-{mission.lower()}-source-map.json'
        path.write_text(json.dumps(data, indent=1) + '\n')
        w = data['whole_file']
        print(f'PASS {mission} {data["source_path"]}: {w["assembled_words"]} assembled words, '
              f'{w["instruction_words_executed_by_oracle"]}/{w["instruction_words"]} instruction words executed '
              f'by the oracle; owners {w["words_by_owner"]}')
