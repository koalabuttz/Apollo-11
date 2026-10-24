#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pre-check a coverage certificate for the gaps reviewers keep sending back.

  python3 tools/cert_precheck.py Luminary099/TJET_LAW.agc [...]
  python3 tools/cert_precheck.py --changed --base origin/port/c64-native-c
  python3 tools/cert_precheck.py --write-facts        # after tools/setup_cloud.sh
  python3 tools/cert_precheck.py --check-facts

Run it before drafting `pending_complete_certificate` in docs/port-coverage.json
(it checks a drafted or complete certificate, or, with neither, only the ABI
and refusal facts). One line per finding, FAIL or WARN, with the file and a fix
hint; NOTE lines list what the evidence does not compare. Exit 1 on any FAIL.
The checks come from the certificate reviews (docs/audits/, auditor issues):

  evidence    four distinct records that exist and match their sha256;
              independent_assembly_comparison is a yaAGC/oracle run record with
              a case count above zero, not the source map, a listing-to-rope
              word check or a review text (#27 #33 #44 #51 #57 #58 #77 #84 #85);
              the host and C64 records hold PASS lines with checks and no
              failures, the C64 one names its PRG sha256 (#76 #82 #118).
  abi         every label of the file that another file of the mission reaches
              (TC/TCF/TCR/BZF/BZMF, CADR/2CADR/GENADR/ADRES/FCADR words after
              BANKCALL, IBNKCALL, POSTJUMP, NOVAC, WAITLIST..., interpretive
              CALL/GOTO/RTB/branch operands, with +N offsets) is a registered
              agc_transfer routine key; every interpretive order, store or EXIT
              word that can follow a DANZIG is registered as a CHANG2 resume
              LOC (#54 #55 #56 #57 #77 #84 #88 #93 #109 #115 #118).
  refusals    OUT_OF_SCOPE/refusal sites in the native sources, so the reviewer
              sees what is never compared; refusing NEWJOB != 0 is flagged
              (#88 #115 #118); refused oracle cases are counted.
  c64         tools/c64_row_cover.py check when the tree has it and the cover
              record names its runner; otherwise the c64_branch_outcomes record:
              host outcomes, entries or branches the C64 rows miss (#76 #82 #88
              #93 #99 #108 #109 #115 #117 #118).
  manifest    every target the C64 record's vice_targets names PASSED with at
              least the certified checks in the newest complete nightly vice
              manifest (tools/vice_manifest.py), which is under MANIFEST_DAYS
              old; WARN instead when the manifest predates those vice_targets,
              when a certificate names none and claims no c64_exempt (#127
              migration), or when no manifest is available (#127). WARN
              when that manifest's c64_cover (tools/cover_check.py) shows
              the cited branch cover stale or missing host branches (#234).
  domain      no open item ("pending", "follow-up") in a certificate's text (#77
              #84), and every cited native file exists (#51).
  revision    reviewed_all_source_paths and the pinned source_revision, the
              rule tools/port_coverage.py applies to a complete certificate.
  spec keys   WARN (never FAIL), once per certificate, when the interp spec
              (tools/interp_specs.py, found by the evidence/<id>/ the
              certificate cites, or by its file) lacks a key of
              REFREEZE_SPEC_KEYS ('index_carry16': the generated INDEX keeps
              its end-around carry on the C64; 'image_w': flight builds address
              the fixed $7000 image; 'trusted_image': flight builds skip the
              ~121K-cycle erasable scan at every entry); set them at the next
              refreeze.
  arms        every spec the certificate rests on has a tools/cert_arms.py
              record measured on its current frozen cases and own sources,
              with every untaken arm triaged; after a refreeze run
              `tools/cert_arms.py measure SPEC` so the re-cite carries it.
  bovb        a BOVB to TCDANZIG/ENDT(X) in the file needs a native 'rtb'
              entry for that word in its interp spec, else the overflow
              branch stops there and the right-hand order never runs natively
              (#414); WARN on a certificate already accepted complete.

Calls between files certified together (one source map or comparison record,
IMU_PERFORMANCE_TEST_2/TESTS_4) are internal. On a certificate already flipped
to complete the two abi checks are WARN: TIME_OF_FREE_FALL and the LM ephemeris
were accepted before the reviewers asked for every entry and resume LOC, and
reopening them is the auditor's call, not a CI block.

Mechanical limits: a reachable path the case generator never produces (a ±0,
an edge Q, an even count) shows up here only as a refusal site or a missing
C64 outcome; gcov over the oracle producer is still the reviewer's job.
Release-stored evidence is read only when present; --fetch downloads it and
the newest vice manifest (else the one cached under build/vice-manifest/, or
--manifest PATH).

Label addresses, the file each code word belongs to, cross-file references and
interpretive resume words are assembler facts of the immutable original
source. They are compiled once from the yaYUL listings into
tools/cert_precheck_facts.json, so CI needs no listing. Routine keys are read
by compiling each src/*.c that defines agc_routine_entry tables into a probe
that prints them (host cc; nothing else is linked or run).
"""
import argparse
import concurrent.futures
import datetime
import hashlib
import json
import os
import pathlib
import re
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
REPOSITORY = ROOT.parent
INVENTORY = 'docs/port-coverage.json'
FACTS = ROOT / 'tools/cert_precheck_facts.json'
sys.path.insert(0, str(ROOT / 'tools'))
from port_coverage import BASE  # noqa: E402
MISSIONS = {'Luminary099': 'lm', 'Comanche055': 'cm'}
CATEGORIES = ('source_to_native_map', 'independent_assembly_comparison',
              'host_and_sanitizer_results', 'c64_execution_results')
CERTIFICATES = ('pending_complete_certificate', 'complete_native_coverage')

# ---------------------------------------------------------------- listing facts

LINE = re.compile(r'^\d{6},\d{6}: (.*)$')
ADDRESS = re.compile(r'^(?:(\d\d),)?([0-7]{4})$')
SYMBOL = re.compile(r'\b\d+,([FE]):\s+(\S+)\s+(?:(\d\d),)?([0-7]{4})\b')
BASIC = set('''TC TCF TCR CA CAF CAE CS AD ADS SU MSU MASK MP DV DAS DXCH LXCH QXCH INCR AUG
DIM DCA DCS INDEX NDX EXTEND RELINT INHINT XCH TS CCS -CCS BZF BZMF RAND READ WAND WOR ROR WRITE
RXOR EDRUPT RESUME NOOP COM DOUBLE DDOUBL ZL ZQ SQUARE OVSK RETURN DTCB DTCF TCAA XLQ XXALQ DCOM'''.split())
PSEUDO = set('''OCT 2OCT OCTAL DEC DEC* 2DEC 2DEC* CADR 2CADR -2CADR FCADR 2FCADR GENADR -GENADR
ADRES REMADR ECADR BBCON BBCON* 2BCADR MM NV VN = =MINUS EQUALS ERASE BANK BLOCK SETLOC EBANK=
SBANK= COUNT COUNT* BNKSUM MEMORY DNCHAN -DNCHAN DNPTR SUBRO'''.split())
# Operand words that name an entry (not data) when the word they name is code.
ENTRY_PSEUDO = {'CADR', '2CADR', '-2CADR', 'FCADR', '2FCADR', 'GENADR', '-GENADR', 'ADRES', 'REMADR'}
TRANSFER = {'TC', 'TCF', 'TCR', 'BZF', 'BZMF', 'DTCB', 'DTCF'}
# Services that take an entry from the operand after the call and transfer to
# it later, from their own (other-file) code.
SERVICES = {'BANKCALL', 'IBNKCALL', 'POSTJUMP', 'BANKJUMP', 'SWCALL', 'ISWCALL', 'USPRCADR', 'NOVAC',
            'FINDVAC', 'SPVAC', 'WAITLIST', 'LONGCALL', 'TWIDDLE', 'SUPDXCHZ'}


def is_interpretive(op):
    return bool(op) and op not in BASIC and op not in PSEUDO and not re.fullmatch(r'-?\d*\w*DNADR|-?\d+', op)


def physical(bank, offset):
    if bank is None:
        return 2048 + (offset - 0o4000) if 0o4000 <= offset < 0o10000 else None
    return bank * 1024 + (offset & 0o1777)


def where(phys):
    if 2048 <= phys < 4096:
        return f'{phys - 2048 + 0o4000:04o}'
    return f'{phys // 1024:02o},{phys % 1024 + 0o2000:04o}'


def listing_rows(text):
    """(file, line, phys, words, label, op, operand) for each assembled line."""
    current, rows = 'MAIN.agc', []
    for raw in text.splitlines():
        if raw.startswith('(End of include-file '):
            current = 'MAIN.agc'
            continue
        match = LINE.match(raw)
        if not match:
            continue
        if match[1].startswith('$'):
            current = match[1][1:].split()[0]
            continue
        address = ADDRESS.match(raw[14:23].strip())
        words = [int(w, 8) for w in raw[33:44].split() if re.fullmatch('[0-7]{5}', w)]
        phys = None
        if address and words:
            phys = physical(int(address[1], 8) if address[1] else None, int(address[2], 8))
        rows.append((current, int(raw[7:13]), phys, words, raw[45:65].strip(), raw[65:74].strip(),
                     raw[74:].split('#', 1)[0].strip()))
    return rows


def operand_target(text, symbols):
    """Physical word an operand names (NAME, NAME +N, NAME -N, NAME,1), or None."""
    parts = text.replace(',1', ' ').replace(',2', ' ').split()
    if not parts or parts[0] not in symbols:
        return None, None
    offset = 0
    for token in parts[1:]:
        if re.fullmatch(r'[+-]\d+', token):
            offset += int(token)
        elif token in '+-':
            continue
        elif re.fullmatch(r'\d+', token) and parts[1:2] in (['+'], ['-']):
            offset += int(token) * (-1 if parts[1] == '-' else 1)
        else:
            return None, None
    name = parts[0] + (f' {offset:+d}' if offset else '')
    return symbols[parts[0]] + offset, name


def mission_facts(listing):
    rows = listing_rows(listing)
    symbols = {}
    for kind, name, bank, offset in SYMBOL.findall(listing.split('Symbol Table', 1)[-1]):
        if kind == 'F':
            phys = physical(int(bank, 8) if bank else None, int(offset, 8))
            if phys is not None:
                symbols[name] = phys
    code = {}                                   # phys -> (file, line, op kind)
    for path, line, phys, words, label, op, operand in rows:
        if phys is None:
            continue
        kind = 'data' if op in PSEUDO or not op else ('interp' if is_interpretive(op) else 'basic')
        for k in range(len(words)):
            code.setdefault(phys + k, (path, line, kind if k == 0 else 'data'))
    files = {}

    def entry(path):
        return files.setdefault(path, {'refs': {}, 'resume': []})

    # Interpretive words that can follow a DANZIG: every order (pair), store or
    # EXIT word, except the store word that STADR consumes.
    previous = None
    for path, line, phys, words, label, op, operand in rows:
        if phys is None:
            continue
        if is_interpretive(op) and not (previous and 'STADR' in previous[5:7]):
            entry(path)['resume'].append(phys)
        previous = (path, line, phys, words, label, op, operand.split()[0] if operand else '')
    # References from one file to a code word of another file.
    for index, (path, line, phys, words, label, op, operand) in enumerate(rows):
        if phys is None or not operand or op in ('EQUALS', '=', 'SETLOC', 'BANK', 'EBANK=', 'SBANK='):
            continue
        if op in BASIC and op not in TRANSFER:
            continue
        if op in PSEUDO and op not in ENTRY_PSEUDO:
            continue
        if is_interpretive(op):
            # An order line's operand column is the second opcode (or, for a
            # store, its address); neither names an entry.
            continue
        target, name = operand_target(operand, symbols)
        if target is None or target not in code:
            continue
        owner, owner_line, kind = code[target]
        if kind == 'data':
            continue
        form = op or 'operand'
        service = None
        if op in ENTRY_PSEUDO:
            caller = next((r for r in reversed(rows[max(0, index - 6):index]) if r[2] is not None), None)
            if caller and caller[5] in TRANSFER and caller[6]:
                form = f'{caller[5]} {caller[6]} {op}'
                service = caller[6].split()[0]
        # A file's own entry is still reached through the trampoline when its
        # CADR goes to BANKCALL, NOVAC, WAITLIST... in another file (#115).
        if owner == path and service not in SERVICES:
            continue
        refs = entry(owner)['refs'].setdefault(str(target), {'name': name, 'forms': {}, 'callers': []})
        refs['forms'][form] = refs['forms'].get(form, 0) + 1
        if len(refs['callers']) < 2:
            refs['callers'].append(f'{path}:{line}')
    out = {}
    for path in sorted(files):
        data = files[path]
        refs = [[int(target), r['name'], ' '.join(f'{k}*{v}' for k, v in sorted(r['forms'].items())),
                 ' '.join(r['callers'][:2])] for target, r in sorted(data['refs'].items(), key=lambda t: int(t[0]))]
        if refs or data['resume']:
            out[path] = {'refs': refs, 'resume': ranges(sorted(set(data['resume'])))}
    return out


def ranges(values):
    """Sorted ints as 'a-b,c' runs."""
    runs = []
    for value in values:
        if runs and runs[-1][1] == value - 1:
            runs[-1][1] = value
        else:
            runs.append([value, value])
    return ','.join(f'{a}-{b}' if a != b else f'{a}' for a, b in runs)


def unranges(text):
    out = []
    for run in filter(None, text.split(',')):
        a, _, b = run.partition('-')
        out.extend(range(int(a), int(b or a) + 1))
    return out


def render_facts(facts):
    """JSON with one line per file header and one line per reference."""
    dump = json.dumps
    out = ['{"source_revision": %s, "missions": {' % dump(facts['source_revision'])]
    missions = list(facts['missions'].items())
    for m, (mission, files) in enumerate(missions):
        out.append(f' {dump(mission)}: {{')
        items = list(files.items())
        for f, (path, data) in enumerate(items):
            out.append(f'  {dump(path)}: {{"resume": {dump(data["resume"])}, "refs": [')
            out.extend('   ' + dump(ref) + (',' if r < len(data['refs']) - 1 else '')
                       for r, ref in enumerate(data['refs']))
            out.append('  ]}' + (',' if f < len(items) - 1 else ''))
        out.append(' }' + (',' if m < len(missions) - 1 else ''))
    out.append('}}')
    return '\n'.join(out) + '\n'


def code_rows(path):
    """Code fields of a source file, whitespace-normalized (the fixtures only
    repair indentation and add bracketed SBANK= workaround lines)."""
    rows, lines = [], path.read_text(errors='replace').splitlines()
    for i, line in enumerate(lines):
        code = line.split('#', 1)[0]
        if code.split()[:1] == ['SBANK='] and i and lines[i - 1].startswith('## [WORKAROUND]'):
            continue
        if code.strip():
            rows.append(' '.join(code.split()))
    return rows


def write_facts(check):
    facts = {'source_revision': BASE, 'missions': {}}
    for mission in MISSIONS:
        base = ROOT / 'build/fixed-data' / mission
        listing = base / 'assembly.lst'
        if not listing.is_file():
            print(f'FAIL: {listing.relative_to(ROOT)} missing; run tools/setup_cloud.sh', file=sys.stderr)
            return 1
        # The fixture names some files differently (P30,P37.agc); match by code.
        # Same code rows, else the original sharing most of them (the fixtures
        # of a few files carry small transcription repairs).
        original = {path.name: set(code_rows(path)) for path in (REPOSITORY / mission).glob('*.agc')}
        names = {}
        for path in base.glob('*.agc'):
            rows = set(code_rows(path))
            score, best = max((len(rows & other) / max(1, len(rows | other)), name)
                              for name, other in original.items())
            names[path.name] = best if score > 0.8 else None
        files = mission_facts(listing.read_text(errors='replace'))
        missing = sorted(name for name, repo in names.items() if repo is None and name in files)
        if missing:
            print(f'FAIL: {mission} fixture files match no original file: {missing}', file=sys.stderr)
            return 1
        for data in files.values():
            for ref in data['refs']:
                ref[3] = ' '.join(f'{names.get(c.split(":")[0]) or c.split(":")[0]}:{c.split(":")[1]}'
                                  for c in ref[3].split())
        facts['missions'][mission] = {names.get(path) or path: data for path, data in sorted(files.items())}
    text = render_facts(facts)
    if check:
        if not FACTS.is_file() or FACTS.read_text() != text:
            print(f'FAIL: {FACTS.relative_to(ROOT)} is stale; run tools/cert_precheck.py --write-facts',
                  file=sys.stderr)
            return 1
        print(f'PASS: {FACTS.relative_to(ROOT)} matches the listings')
        return 0
    FACTS.write_text(text)
    print(f'wrote {FACTS.relative_to(ROOT)} ({len(text)} bytes)')
    return 0


# ---------------------------------------------------------------- routine keys

ENTRY_ARRAY = re.compile(r'(?:static\s+)?const\s+agc_routine_entry\s+(\w+)\s*\[[^\]]*\]\s*=')
TABLE = re.compile(r'const\s+agc_routine_table\s+(\w+)\s*=\s*\{\s*(\w+)')
TABLES = re.compile(r'const\s+agc_routine_table\s+(\w+)\s*\[[^\]]*\]\s*=\s*\{(.*?)\};', re.S)


def table_mission(source, array, owners, others):
    """'lm', 'cm' or None (both) for a routine array, from its table's name."""
    for name in owners + [array, pathlib.Path(source).stem]:
        tokens = name.lower().split('_')
        for mission in ('lm', 'cm'):
            if mission in tokens:
                return mission
        if tokens[0] == 'p66':
            return 'lm'
    # An unprefixed table beside cm_ ones is the LM one (standard_lead_routines.c).
    return 'lm' if any(o.startswith('cm_') for o in others) else None


def probe(source, scratch, cc):
    """{array: (mission, [physical...])} of the agc_routine_entry tables a source defines."""
    flags = ['-Iinclude', '-Isrc', '-Itests', '-Iplatform/host']
    pre = subprocess.run([cc, '-E', '-P', *flags, source], cwd=ROOT, capture_output=True, text=True)
    if pre.returncode:
        return None, pre.stderr.strip().splitlines()[-1:] or ['preprocessor failed']
    arrays = list(dict.fromkeys(ENTRY_ARRAY.findall(pre.stdout)))
    owners = {}
    for table, array in TABLE.findall(pre.stdout):
        owners.setdefault(array, []).append(table)
    for table, body in TABLES.findall(pre.stdout):
        for array in re.findall(r'\{\s*(\w+)\s*,', body):
            owners.setdefault(array, []).append(table)
    if not arrays:
        return {}, None
    tables = [t for names in owners.values() for t in names]
    body = ''.join(f'for(unsigned i=0;i<sizeof {a}/sizeof *{a};i++)printf("{a} %u\\n",(unsigned){a}[i].physical);'
                   for a in arrays)
    folder = pathlib.Path(tempfile.mkdtemp(dir=scratch))
    (folder / 'probe.c').write_text(f'#include "{ROOT / source}"\n#include <stdio.h>\n'
                                    f'int main(void){{{body}return 0;}}\n')
    built = subprocess.run([cc, '-w', '-no-pie', *flags, str(folder / 'probe.c'), '-o', str(folder / 'probe'),
                            '-Wl,--unresolved-symbols=ignore-all'], cwd=ROOT, capture_output=True, text=True)
    if built.returncode:
        return None, built.stderr.strip().splitlines()[-1:] or ['probe failed']
    out = subprocess.run([str(folder / 'probe')], capture_output=True, text=True, timeout=60).stdout
    keys = {}
    for line in out.splitlines():
        array, value = line.split()
        keys.setdefault(array, []).append(int(value))
    return {a: (table_mission(source, a, owners.get(a, []), tables), v) for a, v in keys.items()}, None


def routine_keys(cc='cc'):
    """{'lm': {physical: [source...]}, 'cm': {...}}, plus probe errors."""
    sources = sorted(str(p.relative_to(ROOT)) for p in (ROOT / 'src').glob('*.c')
                     if re.search(r'agc_routine_(entry|table)\b', p.read_text(errors='replace')))
    # (A mission build of another source, as src/cm_interp_core.c includes
    # src/lm_interp_core.c, registers that source's tables under its names.)
    names = {pathlib.Path(s).name for s in sources}
    sources += sorted(str(p.relative_to(ROOT)) for p in (ROOT / 'src').glob('*.c')
                      if str(p.relative_to(ROOT)) not in sources and any(
                          n in names for n in re.findall(r'#\s*include\s*"([^"]+\.c)"', p.read_text(errors='replace'))))
    shared = hashlib.sha256()
    for path in sorted(ROOT.glob('include/*.h')) + sorted(ROOT.glob('src/*.inc')):
        shared.update(path.name.encode() + path.read_bytes())
    cache_dir = ROOT / 'build/cert-precheck'
    cache_dir.mkdir(parents=True, exist_ok=True)
    keys, errors = {'lm': {}, 'cm': {}}, []
    with tempfile.TemporaryDirectory(dir=cache_dir) as scratch:
        def one(source):
            digest = hashlib.sha256(shared.digest() + (ROOT / source).read_bytes()).hexdigest()[:24]
            cached = cache_dir / f'{pathlib.Path(source).stem}-{digest}.json'
            if cached.is_file():
                return source, json.loads(cached.read_text()), None
            found, error = probe(source, scratch, cc)
            if found is not None:
                cached.write_text(json.dumps(found))
            return source, found, error
        with concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
            for source, found, error in pool.map(one, sources):
                if found is None:
                    errors.append(f'{source}: {" ".join(error)}')
                    continue
                for array, (mission, values) in found.items():
                    for mission_key in ([mission] if mission else ['lm', 'cm']):
                        for value in values:
                            keys[mission_key].setdefault(value, []).append(source)
    return keys, errors


# ---------------------------------------------------------------- findings


class Report:
    def __init__(self):
        self.lines, self.fails, self.warns = [], 0, 0

    def add(self, level, path, message, hint=''):
        self.lines.append(f'{level}: {path}: {message}' + (f' -- fix: {hint}' if hint else ''))
        if level == 'FAIL':
            self.fails += 1
        elif level == 'WARN':
            self.warns += 1


def read_text(path):
    try:
        return (ROOT / path).read_text(errors='replace')
    except OSError:
        return None


def released(path):
    """True when evidence/<id>/<name> is a release-stored manifest entry."""
    parts = pathlib.PurePosixPath(path).parts
    if len(parts) != 3 or parts[0] != 'evidence':
        return False
    manifest = ROOT / 'evidence' / parts[1] / 'SHA256.json'
    try:
        record = json.loads(manifest.read_text())['files'].get(parts[2])
    except (OSError, ValueError, KeyError):
        return False
    return bool(record) and record.get('store') == 'release'


def fetch(path):
    """Download one release-stored evidence file (tools/evidence.py's store)."""
    sys.path.insert(0, str(ROOT / 'tools'))
    import evidence  # noqa: E402
    parts = pathlib.PurePosixPath(path).parts
    record = evidence.load(parts[1])['files'][parts[2]]
    evidence.download(evidence.asset_url(parts[1], parts[2], record), ROOT / path, record)


def walk(node):
    if isinstance(node, dict):
        for key, value in node.items():
            yield key, value
            yield from walk(value)
    elif isinstance(node, list):
        for value in node:
            yield None, value
            yield from walk(value)


CASES = re.compile(r'(^|_)(cases|states|rows_compared|cases_equal|equal)$')
ORACLE = re.compile(r'yaAGC|Virtual AGC|agc_engine|\boracle', re.I)
NOT_COMPARISON = re.compile(r'no native code read|listing .*against .*rope|word by word', re.I)
REVIEW_NAME = re.compile(r'review|audit|observations|source-map|whole-source|ownership', re.I)
PASS_LINE = re.compile(r'PASS[^\n]*?(?<![0-9a-f])[1-9][\d,]*(?![0-9a-f])|'
                       r'\b[1-9][\d,]* (?:checks|cases|states)\b[^\n]*\b0 (?:failures|differing)')
FAILED = re.compile(r'^\s*FAIL\b|\b[1-9][\d,]* (?:failures|differing|mismatches)\b', re.I | re.M)


def check_comparison(report, label, record, source_map):
    path = record.get('path', '')
    text = read_text(path)
    if text is None:
        return
    hint = ('cite the yaAGC run record (the differential\'s proof.json or <m>-source-stream.json with its '
            'snapshot archive), with the oracle rope and a case count; move reviews to evidence.independent_review')
    if path == source_map:
        return  # runs_distinct reports it
    try:
        data = json.loads(text)
    except ValueError:
        report.add('FAIL', label, f'independent_assembly_comparison {path} is not a run record (a text or review)', hint)
        return
    method = ' '.join(str(v) for k, v in walk(data) if k in ('method', 'note', 'qualification', 'kind'))
    if NOT_COMPARISON.search(method) and not ORACLE.search(method):
        report.add('FAIL', label, f'independent_assembly_comparison {path} is a listing-to-rope word check, '
                   'not an execution comparison (#77, #84)', hint)
        return
    counts = []
    for key, value in walk(data):
        if key and CASES.search(key):
            values = value.values() if isinstance(value, dict) else [value]
            counts += [v for v in values if isinstance(v, int) and not isinstance(v, bool)]
    for key, value in walk(data):
        if key == 'states' and isinstance(value, int):
            counts.append(value)
        if key in ('suites',) and isinstance(value, dict):
            counts += [s.get('cases', 0) for s in value.values() if isinstance(s, dict)]
    identity = ORACLE.search(text) or ('rope_sha256' in data if isinstance(data, dict) else False)
    if not identity:
        report.add('FAIL', label, f'independent_assembly_comparison {path} names no yaAGC/oracle run '
                   '(no oracle, agc_engine or rope_sha256)', hint)
    elif not any(c > 0 for c in counts):
        report.add('FAIL', label, f'independent_assembly_comparison {path} records no compared case count above zero '
                   '(#57, #85)', hint)
    differing = [v for k, v in walk(data) if k in ('differing', 'mismatches', 'unequal') and isinstance(v, int) and v]
    if differing or any(k == 'equal' and v is False for k, v in walk(data)):
        report.add('FAIL', label, f'independent_assembly_comparison {path} records differing cases', 'fix the native code')


def check_results(report, label, category, record):
    path = record.get('path', '')
    text = read_text(path)
    if text is None:
        return
    what = 'host' if category == 'host_and_sanitizer_results' else 'C64'
    if FAILED.search(text):
        report.add('FAIL', label, f'{category} {path} reports failures', 'rerun and recapture the log from a passing run')
    if not PASS_LINE.search(text):
        report.add('FAIL', label, f'{category} {path} has no PASS line with a check/case count above zero',
                   f'cite the {what} run log that `make {"test-/sanitize-" if what == "host" else "vice-"}<m>` writes')
    if what == 'C64':
        if not re.search(r'VICE|x64sc|C64', text):
            report.add('FAIL', label, f'{category} {path} is not a VICE/C64 run record', 'cite the vice-<m> log')
        elif not re.search(r'PRG sha256|prg_sha256|\.prg sha256|\.prg"\s*:\s*"[0-9a-f]{64}', text, re.I):
            report.add('WARN', label, f'{category} {path} records no PRG sha256 (#76, #82, #118)',
                       'have the vice log name the PRG hash so the reviewer can bind the binary')
    elif not re.search(r'sanitiz|ASan|UBSan|-fsanitize', text, re.I):
        report.add('WARN', label, f'{category} {path} shows no sanitizer run', 'include the sanitize-<m> output')


def check_evidence(report, label, certificate, fetch_missing):
    evidence = certificate.get('evidence')
    if not isinstance(evidence, dict):
        report.add('FAIL', label, 'certificate has no evidence dictionary', 'cite the four records')
        return
    for category in CATEGORIES:
        record = evidence.get(category)
        if not isinstance(record, dict) or not record.get('path'):
            report.add('FAIL', label, f'missing {category}', 'cite a record for each of the four categories')
            continue
        path = record['path']
        candidate = (ROOT / path).resolve()
        if ROOT not in candidate.parents:
            report.add('FAIL', label, f'{category} {path} is outside c64-port', 'cite evidence/<id>/<file>')
            continue
        if not candidate.is_file() and released(path) and fetch_missing:
            try:
                fetch(path)
            except (OSError, SystemExit, KeyError) as error:
                report.add('WARN', label, f'{category} {path} could not be fetched ({error})')
        if not candidate.is_file():
            if released(path):
                report.add('WARN', label, f'{category} {path} is release-stored and not fetched; not checked',
                           f'tools/evidence.py fetch {pathlib.PurePosixPath(path).parts[1]}')
            else:
                report.add('FAIL', label, f'{category} {path} does not exist', 'commit or publish it with tools/evidence.py')
            continue
        if hashlib.sha256(candidate.read_bytes()).hexdigest() != record.get('sha256'):
            report.add('FAIL', label, f'{category} {path} sha256 differs from the certificate', 'update the hash')
    source_map = (evidence.get('source_to_native_map') or {}).get('path')
    for category in CATEGORIES[1:]:
        # The rule port_coverage.runs_distinct enforces, kept here so the
        # pre-check reads a drafted certificate the same way.
        if source_map and (evidence.get(category) or {}).get('path') == source_map:
            what = 'an independent comparison' if category == CATEGORIES[1] else 'a test run'
            report.add('FAIL', label, f'{category} points at the source-to-native map, not {what}',
                       'cite the yaAGC run record and the run logs, not the map (#27, #44, #57, #58)')
    paths = {}
    for category in CATEGORIES[1:]:
        path = (evidence.get(category) or {}).get('path')
        if path and path in paths:
            report.add('FAIL', label, f'{category} and {paths[path]} cite the same record {path}',
                       'each of comparison, host run and C64 run is its own record (#57, #58)')
        paths.setdefault(path, category)
    if isinstance(evidence.get(CATEGORIES[1]), dict):
        check_comparison(report, label, evidence[CATEGORIES[1]], source_map)
    for category in CATEGORIES[2:]:
        record = evidence.get(category)
        if isinstance(record, dict) and record.get('path') != source_map:
            check_results(report, label, category, record)


OPEN_ITEM = re.compile(r'\b(pending|follow-up|follow up|TODO|TBD|not yet registered|registration pending)\b', re.I)


def check_text(report, label, certificate):
    for key in ('supported_domain', 'external_continuations', 'review_scope'):
        value = certificate.get(key)
        text = json.dumps(value) if not isinstance(value, str) else value
        if value and OPEN_ITEM.search(text):
            report.add('FAIL', label, f'{key} carries an open item ("{OPEN_ITEM.search(text)[0]}")',
                       'a complete certificate cannot carry open work (#77, #84); finish it or keep the file partial')


def check_revision(report, label, certificate):
    if certificate.get('reviewed_all_source_paths') is not True or certificate.get('source_revision') != BASE:
        report.add('FAIL', label, f'source_revision {certificate.get("source_revision")!r}, reviewed_all_source_paths '
                   f'{certificate.get("reviewed_all_source_paths")!r}: tools/port_coverage.py fails the flip',
                   f'review every source path at the pinned revision {BASE} and say so')


def native_files(entry, certificate):
    """The certificate's native sources: those its source map names, else the entry's."""
    found = []
    record = ((certificate or {}).get('evidence') or {}).get('source_to_native_map') or {}
    text = read_text(record.get('path', '')) if record.get('path', '').endswith('.json') else None
    if text:
        try:
            data = json.loads(text)
        except ValueError:
            data = None
        for key, value in walk(data):
            for item in ([key] if isinstance(key, str) else []) + ([value] if isinstance(value, str) else []):
                if re.fullmatch(r'(src|include|platform/[\w]+)/[\w./+-]+\.(c|h|inc)', item):
                    found.append(item)
    if not found:
        for item in entry.get('native_coverage', []):
            found += [f for f in item.get('native_files', []) if isinstance(f, str)]
    return list(dict.fromkeys(found))


REFUSAL = re.compile(r'\b\w*OUT_OF_SCOPE\b|\brefuse\w*\s*\(|EXEC_UNMAPPED')
NEWJOB = re.compile(r'NEWJOB|\[\s*0+67u?\s*\]')
# A range or multiple-of-12 test on NEWJOB refuses only values the Executive
# never stores (standard_lead_cm.c), not a waiting job.
NEWJOB_VALID = re.compile(r'(NEWJOB\w*|\[\s*0+67u?\s*\])\s*(>|%)')
# A test for NEWJOB +0 whose branch returns is the DANZIG that goes on to
# NEWOPS; NEWJOB set takes CHANG2 (tools/interp_compile.py next()). It
# refuses nothing unless the line also says so.
NEWJOB_ZERO = re.compile(r'!\s*(NEWJOB\w*|\w*\[\s*0+67u?\s*\])')


def check_refusals(report, label, entry, certificate, notes):
    files = native_files(entry, certificate)
    missing = [f for f in files if not (ROOT / f).is_file()]
    if missing and certificate:
        report.add('FAIL', label, f'cited native files do not exist: {", ".join(missing)}',
                   'remove them from the source map/native_coverage or add them (#51)')
    newjob = []
    for source in files:
        if not source.endswith('.c') or not (ROOT / source).is_file():
            continue
        lines = (ROOT / source).read_text(errors='replace').splitlines()
        sites = []
        for number, line in enumerate(lines, 1):
            code = line.split('//', 1)[0]
            if REFUSAL.search(code) and not code.lstrip().startswith(('#', '*', '/*')):
                sites.append(number)
            if NEWJOB.search(code) and not NEWJOB_VALID.search(code) and \
                    re.search(r'\breturn\b|OUT_OF_SCOPE|refuse', code) and \
                    not (NEWJOB_ZERO.search(code) and not re.search(r'OUT_OF_SCOPE|refuse', code)) and \
                    not code.lstrip().startswith(('*', '/*')):
                newjob.append(f'{source}:{number}')
        if sites and notes:
            report.add('NOTE', label, f'{len(sites)} refusal sites, never compared with yaAGC: {source}:'
                       + ','.join(map(str, sites[:40])) + (' ...' if len(sites) > 40 else ''))
    if newjob:
        report.add('WARN', label, f'refuses NEWJOB != +0 ({", ".join(newjob[:6])}); a job switch mid-sequence is '
                   'an ordinary flight state (#88, #115, #118)',
                   'take CHANG2 through the trampoline at registered resume LOCs and compare NEWJOB-set cases')
    record = ((certificate or {}).get('evidence') or {}).get('independent_assembly_comparison') or {}
    text = read_text(record.get('path', '')) if record.get('path', '').endswith('.json') else None
    try:
        data = json.loads(text) if text else None
    except ValueError:
        data = None
    refused = sum(v for k, v in walk(data) if isinstance(k, str) and re.search(r'refus', k, re.I)
                  and isinstance(v, int) and not isinstance(v, bool))
    if refused:
        report.add('WARN', label, f'the comparison refuses {refused} cases; each refused input must be '
                   'unreachable from the source (#82, #88, #99)', 'argue each refusal or compare it')


def check_abi(report, label, mission, name, facts, keys, unit, accepted):
    """unit: files certified together (one native module; their calls to each
    other are internal). accepted: a complete certificate the reviewers flipped
    before this bar was applied (TIME_OF_FREE_FALL, LM ephemeris); its gaps are
    WARN, for the auditor, not a CI block."""
    data = facts['missions'].get(mission, {}).get(name)
    if not data:
        return
    level = 'WARN' if accepted or not keys.get('complete', True) else 'FAIL'
    registered = keys[MISSIONS[mission]]
    missing = [ref for ref in data['refs'] if ref[0] not in registered
               and not all(c.split(':')[0] in unit for c in ref[3].split())]
    if missing:
        shown = '; '.join(f'{ref[1]} {where(ref[0])} ({ref[2].replace("*", " x")}; {ref[3].split()[0]})'
                          for ref in missing[:8])
        report.add(level, label, f'{len(missing)} of {len(data["refs"])} entries reached from other files are not '
                   f'registered agc_transfer routines: {shown}' + (' ...' if len(missing) > 8 else ''),
                   'register each at its physical address in an agc_routine_table and round-trip it on host and C64')
    resume = unranges(data['resume'])
    if resume:
        unregistered = [p for p in resume if p not in registered]
        if unregistered:
            report.add(level, label, f'{len(unregistered)} of {len(resume)} interpretive order/store/EXIT words '
                       'are not registered CHANG2 resume LOCs: ' + ' '.join(where(p) for p in unregistered[:10])
                       + (' ...' if len(unregistered) > 10 else ''),
                       'register every word that can follow a DANZIG as an interpretive key and compare '
                       'NEWJOB-set cases (#88, #109, #115, #118)')


def mk_sources(milestone):
    text = read_text(f'tools/mk/{milestone}.mk') or ''
    return sorted(set(re.findall(r'\bsrc/[\w./-]+\.c\b', text)))


def check_c64(report, label, certificate):
    evidence = (certificate or {}).get('evidence') or {}
    record = evidence.get('c64_branch_outcomes')
    tool = ROOT / 'tools/c64_row_cover.py'
    if not isinstance(record, dict):
        report.add('WARN', label, 'no c64_branch_outcomes record; the reviewers diff gcov branch outcomes of the C64 '
                   'rows against the host stream over every linked source (#76, #82, #88, #93, #118)',
                   ('pick the rows with tools/c64_row_cover.py pick and cite its check' if tool.is_file() else
                    'choose C64 rows by branch outcome and cite a cover record'))
        return
    text = read_text(record.get('path', ''))
    try:
        data = json.loads(text) if text else None
    except ValueError:
        data = None
    if not isinstance(data, dict):
        return
    command = data.get('row_cover_check')
    if tool.is_file() and isinstance(command, list):
        run = subprocess.run([sys.executable, str(tool), 'check', *map(str, command)], cwd=ROOT,
                             capture_output=True, text=True)
        if run.returncode:
            report.add('FAIL', label, 'tools/c64_row_cover.py check: ' + (run.stdout + run.stderr).strip()
                       .splitlines()[-1:][0] if (run.stdout + run.stderr).strip() else 'failed',
                       'add C64 rows for the missing arcs and re-run VICE')
        return
    missing = data.get('host_branches_missing_on_c64', data.get('host_only'))
    if missing:
        report.add('FAIL', label, f'{record["path"]} lists {len(missing)} host outcomes the C64 rows miss',
                   'add rows for them and re-run VICE')
    for slot, pair in enumerate(data.get('entry_slots_host_c64') or []):
        if isinstance(pair, list) and len(pair) == 2 and pair[0] and not pair[1]:
            report.add('FAIL', label, f'{record["path"]} entry slot {slot} runs on the host only',
                       'run every registered entry on the C64')
    for kind, pair in (data.get('outcomes_host_c64') or {}).items():
        if isinstance(pair, list) and len(pair) == 2 and pair[0] and not pair[1]:
            report.add('FAIL', label, f'{record["path"]} outcome {kind} runs on the host only', 'add C64 rows for it')
    measured = set(re.findall(r'\bsrc/[\w./-]+\.c\b', text))
    milestone = pathlib.PurePosixPath(record['path']).parts[1] if record['path'].count('/') == 2 else ''
    linked = [s for s in mk_sources(milestone) if s not in measured]
    if measured and linked:
        report.add('WARN', label, f'{record["path"]} measures only {", ".join(sorted(measured))}; the C64 build '
                   f'also links {", ".join(linked[:6])}' + (' ...' if len(linked) > 6 else '') +
                   ' (#108, #109, #115)', 'measure branch outcomes over every linked source')


# Spec keys of tools/interp_specs.py that change generated C a lane's frozen
# evidence pins. The steward keeps them off the specs that do not ask for them
# (regenerating every spec would stale the pinned sources), so a certificate
# whose spec lacks one gets a WARN, never a FAIL, until its next refreeze.
# 'image_w' (steward PR #260): W from include/agc_image_w.h, so flight builds
# address the fixed $7000 image.
# 'trusted_image': flight builds skip the entry checks' erasable-image scan
# (~121,000 6502 cycles a call; Lane G measured it as most of the CM entry
# DAP's cost at 79x real time).
REFREEZE_SPEC_KEYS = ('index_carry16', 'image_w', 'trusted_image')
# What a spec without each key gets wrong (the WARN's text).
REFREEZE_REASONS = {
    'index_carry16': 'the generated INDEX (ixw) drops its end-around carry on the C64 (16-bit unsigned) '
                     'for a negative index value',
    'image_w': 'flight builds (RK_FIXED_IMAGE) address the image through s->words instead of the fixed '
               '$7000 image',
    'trusted_image': 'flight builds (RK_FIXED_IMAGE) scan the whole erasable image at every routine entry, '
                     'about 121,000 6502 cycles a call',
}


def interp_specs():
    """tools/interp_specs.py's SPECS, or {} when it cannot be read."""
    try:
        import interp_specs  # noqa: E402
    except Exception:  # noqa: BLE001 -- a broken spec file is not this check's FAIL
        return {}
    return getattr(interp_specs, 'SPECS', {})


def specs_for(path, certificate, specs):
    """The interp specs a certificate rests on: the evidence ids it cites that
    are spec keys (evidence/<id>/ is the spec's milestone), and the specs whose
    'file' is this AGC file."""
    mission, name = path.split('/', 1)
    out = set()
    for record in ((certificate or {}).get('evidence') or {}).values():
        parts = pathlib.PurePosixPath(record.get('path', '') if isinstance(record, dict) else '').parts
        if len(parts) >= 2 and parts[0] == 'evidence' and parts[1] in specs:
            out.add(parts[1])
    for key, spec in specs.items():
        short = MISSIONS.get(mission)
        files = (spec.get('file') or {}).get(short) if short in (spec.get('missions') or ()) else None
        if name in ([files] if isinstance(files, str) else files or []):
            out.add(key)
    return sorted(out)


def check_spec_keys(report, label, path, certificate, specs):
    """One WARN per certificate whose interp spec lacks a REFREEZE_SPEC_KEYS key."""
    missing = {}
    for key in specs_for(path, certificate, specs):
        for flag in REFREEZE_SPEC_KEYS:
            if not specs[key].get(flag):
                missing.setdefault(flag, []).append(key)
    if missing:
        flags = sorted(missing)
        keys = sorted({k for v in missing.values() for k in v})
        report.add('WARN', label, f'interp spec {", ".join(keys)} lacks {", ".join(repr(f) for f in flags)}: '
                   + '; '.join(REFREEZE_REASONS[f] for f in flags) + ' (docs/INTERP-COMPILE.md)',
                   f'set {", ".join(repr(f) for f in flags)} in tools/interp_specs.py at the lane\'s next refreeze')


def check_arms(report, label, path, certificate, specs):
    """FAIL when a spec the certificate rests on has no current, fully
    triaged untaken-arm record (tools/cert_arms.py)."""
    try:
        import cert_arms  # noqa: E402
    except Exception:  # noqa: BLE001 -- a tree without the tool has nothing to check
        return
    allow = cert_arms.load()
    for key in specs_for(path, certificate, specs):
        proof = ROOT / 'evidence' / key / f'{key}-proof.json'
        if not proof.exists():
            continue
        loops_note(report, label, key, proof)
        why = cert_arms.problems(key, allow.get(key), specs[key])
        if why:
            report.add('FAIL', label, f'interp spec {key}: untaken-arm record {why}',
                       f'run `tools/cert_arms.py measure {key}` and triage its UNTRIAGED arms '
                       'in tools/cert_arms_allow.py')


def loops_note(report, label, key, proof):
    """NOTE the cases a spec's frozen set leaves out as loops (the proof's
    'loops', tools/interp_routine_differential.py loops_record): an arm only
    those cases reach shows in cert_arms as untaken."""
    try:
        loops = json.loads(proof.read_text()).get('loops') or {}
    except (OSError, ValueError):
        return
    for k, what in (('erasable_cadr_cycle', 'erasable-CADR cycle'), ('step_cap', 'step-cap loop')):
        by = loops.get(k) or {}
        if k == 'step_cap':
            by = by.get('routines') or {}
        if by:
            report.add('NOTE', label, f'interp spec {key}: {sum(by.values())} cases left out as {what}s (both sides '
                       'loop): ' + ', '.join(f'{r} x{n}' for r, n in by.items()))


DSKY_SWEEP_FILES = ('PINBALL_GAME_BUTTONS_AND_LIGHTS.agc',)
DSKY_SWEEP_MISSION = {'Luminary099': 'lm', 'Comanche055': 'cm'}
DSKY_SWEEP_KNOWN = ROOT / 'tools' / 'dsky_sweep_known.json'
DSKY_SWEEP_EVIDENCE = ROOT / 'evidence' / 'dsky-sweep' / 'dsky-sweep.json'
# The full report is release-stored (about 1 MB); CI has only the tree's
# summary (tools/dsky_sweep_vice.py summary), which keeps every failed case.
DSKY_SWEEP_SUMMARY = ROOT / 'evidence' / 'dsky-sweep' / 'dsky-sweep-summary.json'


def check_dsky_sweep(report, label, mission, name, accepted):
    """A Pinball certificate needs the C64 DSKY sweep (acceptance item 6,
    tools/dsky_sweep_vice.py, docs/DSKY-SWEEP.md) for its mission: the full
    sweep's result committed as evidence/dsky-sweep/dsky-sweep.json
    (--cases all), with no failure but the clock class, and no entry but the
    clock class in tools/dsky_sweep_known.json (the CI sample's ratchet).
    The class is the sweep tool's (each known entry's signature, which the
    run checks), not the entry's reason. FAIL on a draft, WARN on a
    certificate already accepted complete. The clock class (only the
    centiseconds and timers one count apart: the resident clock's, RK) is a
    WARN either way: it blocks item 6, not the Pinball file."""
    if name not in DSKY_SWEEP_FILES:
        return
    level = 'WARN' if accepted else 'FAIL'
    m = DSKY_SWEEP_MISSION.get(mission)
    if not DSKY_SWEEP_KNOWN.exists():
        report.add(level, label, 'tools/dsky_sweep_known.json is missing', 'restore it from the base branch')
        return
    known = {k: v for k, v in json.loads(DSKY_SWEEP_KNOWN.read_text()).items() if k.split(' ', 1)[0] == m}
    clock = sorted(k for k, v in known.items() if v.get('signature', '').startswith('clock:'))
    left = sorted(k for k in known if k not in clock)
    if left:
        report.add(level, label, f'{len(left)} DSKY sweep sample cases of {m} do not match the host on the C64 '
                   f"(tools/dsky_sweep_known.json: {', '.join(left[:6])}{' ...' if len(left) > 6 else ''})",
                   'fix them, run `make dsky-sweep-vice` and take them off the list')
    if DSKY_SWEEP_EVIDENCE.exists():
        sweep = json.loads(DSKY_SWEEP_EVIDENCE.read_text())
        if DSKY_SWEEP_SUMMARY.exists() and json.loads(DSKY_SWEEP_SUMMARY.read_text()).get('full_sha256') != \
                hashlib.sha256(DSKY_SWEEP_EVIDENCE.read_bytes()).hexdigest():
            report.add(level, label, 'evidence/dsky-sweep/dsky-sweep-summary.json is not the summary of '
                       'dsky-sweep.json (its full_sha256 differs)', 'commit the summary the sweep wrote with it')
    elif DSKY_SWEEP_SUMMARY.exists():
        sweep = json.loads(DSKY_SWEEP_SUMMARY.read_text())
        sweep['results'] = sweep.get('failed', [])
    else:
        dsky_sweep_clock(report, label, m, clock)
        report.add(level, label, f'no full DSKY sweep result for {m} (evidence/dsky-sweep/dsky-sweep.json)',
                   'run `tools/dsky_sweep_vice.py --cases all --evidence evidence/dsky-sweep` and commit it')
        return
    if sweep.get('mode') != 'all' or m not in sweep.get('missions', []):
        dsky_sweep_clock(report, label, m, clock)
        report.add(level, label, f"evidence/dsky-sweep/dsky-sweep.json is a {sweep.get('mode')} run of "
                   f"{', '.join(sweep.get('missions', []))}, not the full sweep of {m}",
                   'rerun it with --cases all on both missions')
        return
    store = sweep.get('stores', {}).get(m, {})
    rope = ROOT / 'evidence' / f'fixed-data-{m}-original.rope.bin'
    reu = ROOT / 'build' / 'flight-store' / f'{m}.reu'
    stale = [what for what, path, key in (('rope', rope, 'rope_sha256'), ('store', reu, 'reu_sha256'))
             if path.exists() and store.get(key) != hashlib.sha256(path.read_bytes()).hexdigest()]
    if stale:
        report.add(level, label, f"evidence/dsky-sweep/dsky-sweep.json's {m} {' and '.join(stale)} hash is not "
                   'the current one (the full sweep ran on another build)', 'rerun the full sweep and commit it')
    failed = [r for r in sweep.get('results', []) if r['m'] == m and not r['ok']]
    bad = [r for r in failed if r.get('class') != 'clock']
    if bad:
        report.add(level, label, f'{len(bad)} of the full DSKY sweep\'s {m} cases do not match the host on the C64 '
                   f"({', '.join(r['keys'] for r in bad[:6])}{' ...' if len(bad) > 6 else ''})",
                   'fix them and rerun the full sweep')
    dsky_sweep_clock(report, label, m, clock + [f"{m} {r['keys']}" for r in failed if r.get('class') == 'clock'])


def dsky_sweep_clock(report, label, m, clock):
    """WARN on the clock class: acceptance item 6 waits on it, Pinball does not."""
    if clock:
        clock = sorted(set(clock))
        report.add('WARN', label, f'{len(clock)} DSKY sweep cases of {m} differ by the C64 clock class only '
                   f"({', '.join(clock[:6])}{' ...' if len(clock) > 6 else ''}): acceptance item 6 waits on them",
                   'the clock class in flight-scenarios/replay-mismatches.md; no Pinball change needed')


# BOVB to ENDT(X)/TCDANZIG (TC DANZIG) with an order on its right: when
# overflow is set the compiled pair transfers to that basic word, and unless
# the spec runs it natively ('rtb' TCDANZIG -> <m>_integration_tcdanzig) the
# routine stops there and the right-hand order never runs natively (#414).
DANZIG_WORDS = ('TCDANZIG', 'ENDT(X)')


def bovb_danzig_words(source):
    """The DANZIG words BOVB pairs of an AGC source branch to, by line."""
    out = []
    pending = None
    for number, raw in enumerate(source.splitlines(), 1):
        line = raw.split('#', 1)[0].rstrip()
        if not line.strip():
            continue
        tokens = line.split()
        ops = tokens if line[0].isspace() else tokens[1:]
        if not ops:
            continue
        if pending is not None:
            if ops[0] in DANZIG_WORDS:
                out.append((pending, ops[0]))
            pending = None
        if ops[0] == 'BOVB' or (len(ops) > 1 and ops[1] == 'BOVB'):
            pending = number
    return out


def check_bovb_danzig(report, label, path, certificate, specs, accepted):
    """FAIL (WARN on a certificate already accepted complete) when a BOVB to
    TCDANZIG/ENDT(X) has no native 'rtb' entry in the file's interp spec."""
    mission, name = path.split('/', 1)
    short = MISSIONS.get(mission)
    keys = specs_for(path, certificate, specs)
    if not keys or not short:
        return
    try:
        source = (REPOSITORY / mission / name).read_text(errors='replace')
    except OSError:
        return
    sites = bovb_danzig_words(source)
    if not sites:
        return
    mapped = {word for key in keys for word, call in (specs[key].get('rtb') or {}).items()
              if word in DANZIG_WORDS and isinstance(call, dict) and call.get(short)}
    # TCDANZIG and ENDT(X) label the same word; the compiler names it by
    # either, so one mapping covers both.
    unmapped = [] if mapped else sites
    if unmapped:
        where = ', '.join(f'{name}:{line} BOVB {word}' for line, word in unmapped[:4])
        report.add('WARN' if accepted else 'FAIL', label,
                   f'{where}: the overflow branch stops at {unmapped[0][1]}, so the pair\'s right-hand order '
                   f'never runs natively (#414)',
                   f"map 'rtb' {unmapped[0][1]!r}: {{'{short}': '{short}_integration_tcdanzig(s)'}} in "
                   f"{', '.join(keys)} (with {short}_integration_moves.h and src/{short}_integration_moves.c), "
                   'then regenerate and refreeze')


MANIFEST_DAYS = 3


def check_manifest(report, label, path, certificate, manifest, now=None):
    """The certificate's vice_targets against the nightly vice manifest (#127)."""
    sys.path.insert(0, str(ROOT / 'tools'))
    import vice_manifest  # noqa: E402
    record = (certificate.get('evidence') or {}).get('c64_execution_results') or {}
    named = record.get('vice_targets') if isinstance(record, dict) else None
    hint = ('name the vice-<m> targets that prove the C64 run: "vice_targets": {"vice-<m>": <checks>} in '
            'c64_execution_results, or "c64_exempt": "<reason>" when no C64 run applies')
    if named is None:
        if not certificate.get('c64_exempt'):
            report.add('WARN', label, 'names no vice_targets or c64_exempt; WARN, not FAIL, while certificates '
                       'migrate (#127)', hint)
        return
    targets = vice_manifest.normalized(named)
    if targets is None:
        report.add('FAIL', label, 'vice_targets is not a list or {target: checks} of vice-* targets', hint)
        return
    if manifest is None:
        return  # one WARN for the run, from precheck()
    run = f'vice manifest run {manifest.get("run_id")} ({str(manifest.get("commit"))[:12]}, {manifest.get("date")})'
    now = now or datetime.datetime.now(datetime.timezone.utc)
    try:
        age = now - datetime.datetime.strptime(manifest['date'], '%Y-%m-%dT%H:%M:%SZ').replace(
            tzinfo=datetime.timezone.utc)
    except (KeyError, TypeError, ValueError):
        age = datetime.timedelta(days=MANIFEST_DAYS + 1)
    if age > datetime.timedelta(days=MANIFEST_DAYS):
        report.add('FAIL', label, f'{run} is over {MANIFEST_DAYS} days old, so it does not prove the tip',
                   'run port-ci with full from the Actions tab, or fix the nightly')
    check_cover(report, label, certificate, manifest, run)
    covered = targets in (manifest.get('certificates') or {}).get(path, {}).values()
    level, note = ('FAIL', '') if covered else (
        'WARN', '; the manifest predates these vice_targets, so the next nightly decides')
    for target, checks in targets.items():
        row = (manifest.get('targets') or {}).get(target)
        if row is None:
            report.add(level, label, f'{target} is not in {run}{note}', 'name a vice-* target the nightly runs')
        elif row.get('status') != 'pass':
            report.add(level, label, f'{target} FAILED in {run} ({row.get("failures")} failures){note}',
                       'fix the target, or the certificate no longer holds')
        elif (row.get('checks') or 0) < checks:
            report.add(level, label, f'{target} ran {row.get("checks")} checks in {run}, fewer than the '
                       f'{checks} certified{note}', 'restore the C64 rows or certify what runs')


def check_cover(report, label, certificate, manifest, run):
    """The cited C64 branch cover as the nightly re-derived it (cover_check.py, #234).
    WARN while records are refrozen with their cases and rows."""
    record = (certificate.get('evidence') or {}).get('c64_branch_outcomes') or {}
    path = record.get('path') if isinstance(record, dict) else None
    if not path or not path.startswith('evidence/'):
        return
    milestone = pathlib.PurePosixPath(path).parts[1]
    row = (manifest.get('c64_cover') or {}).get(milestone)
    if row is None or row.get('reason') == 'no re-derivation for this record yet':
        return
    if row.get('record') != path:
        report.add('WARN', label, f'{run} re-derived {row.get("record")}, not the cited {path}; '
                   'the next nightly decides')
    elif not row.get('verified'):
        report.add('WARN', label, f'C64 branch cover {path} is not re-derivable in {run}: {row.get("reason")}',
                   'refreeze the record with its cases_sha256 and C64 rows')
    elif row.get('missing') or row.get('missing_count'):
        report.add('WARN', label, f'C64 branch cover {path} misses {row.get("missing_count")} host branches '
                   f'in {run}', 'add C64 rows that take them, or show they are host-only')


# ---------------------------------------------------------------- driver


def inventory_entries(text):
    return {entry['path']: entry for entry in json.loads(text).get('files', [])}


def changed_files(base):
    """AGC files whose drafted or complete certificate a change adds or edits."""
    fork = subprocess.run(['git', 'merge-base', base, 'HEAD'], cwd=ROOT,
                          capture_output=True, text=True).stdout.strip() or base
    before = subprocess.run(['git', 'show', f'{fork}:c64-port/{INVENTORY}'], cwd=ROOT,
                            capture_output=True, text=True).stdout
    old = inventory_entries(before) if before else {}
    new = inventory_entries((ROOT / INVENTORY).read_text())
    out = []
    for path, entry in new.items():
        for key in CERTIFICATES:
            if entry.get(key) and (old.get(path) or {}).get(key) != entry.get(key):
                out.append(path)
                break
    return out


def shares_records(a, b):
    """Two certificates citing the same source map or comparison record."""
    if not a or not b:
        return False
    cited = lambda c: {(c.get('evidence') or {}).get(k, {}).get('path') for k in CATEGORIES[:2]} - {None}
    return bool(cited(a) & cited(b))


def precheck(paths, report, notes=True, fetch_missing=False, keys=None, manifest=False):
    """manifest: a vice manifest dict, None for none, or False to fetch (with
    fetch_missing) or read the cached one."""
    entries = inventory_entries((ROOT / INVENTORY).read_text())
    if manifest is False:
        sys.path.insert(0, str(ROOT / 'tools'))
        import vice_manifest  # noqa: E402
        manifest, why = vice_manifest.latest(remote=fetch_missing)
        if manifest is None:
            report.add('WARN', 'vice-manifest', f'{why}; vice_targets not checked')
    facts = json.loads(FACTS.read_text())
    specs = interp_specs()
    if keys is None:
        try:
            keys, errors = routine_keys()
        except OSError as error:            # no host C compiler
            keys, errors = {'lm': {}, 'cm': {}}, [f'cc: {error}']
        for error in errors:
            report.add('WARN', 'src', f'routine tables not read, so missing registrations are WARN: {error}')
        keys['complete'] = not errors
    for path in paths:
        entry = entries.get(path)
        if entry is None or entry.get('kind') != 'agc':
            report.add('FAIL', path, 'not an AGC file of docs/port-coverage.json', 'use Luminary099/<FILE>.agc')
            continue
        mission, name = path.split('/', 1)
        drafted = [(key, entry[key]) for key in CERTIFICATES if entry.get(key)]
        if not drafted:
            drafted = [(None, None)]
        for key, certificate in drafted:
            label = path + (f' ({key})' if key else '')
            documentation = (certificate or {}).get('qualification_kind') == 'documentation_preservation' or \
                'review_scope' in (certificate or {})
            if certificate:
                check_revision(report, label, certificate)
                check_manifest(report, label, path, certificate, manifest)
                check_review(report, label, key, certificate)
            if certificate and not documentation:
                check_evidence(report, label, certificate, fetch_missing)
                check_text(report, label, certificate)
            if documentation:
                continue
            unit = {other.split('/', 1)[1] for other, e in entries.items() if other.startswith(mission + '/')
                    and shares_records(certificate, e.get(key) if key else None)}
            check_abi(report, label, mission, name, facts, keys, unit - {name},
                      key == 'complete_native_coverage' and entry.get('status') == 'complete')
            check_refusals(report, label, entry, certificate, notes)
            if certificate:
                check_c64(report, label, certificate)
                check_spec_keys(report, label, path, certificate, specs)
                check_arms(report, label, path, certificate, specs)
                check_dsky_sweep(report, label, mission, name,
                                 key == 'complete_native_coverage' and entry.get('status') == 'complete')
                check_bovb_danzig(report, label, path, certificate, specs,
                                  key == 'complete_native_coverage' and entry.get('status') == 'complete')
    return report


def check_review(report, label, key, certificate):
    """A complete certificate's independent review is a reviewer's record.

    Reviewers write theirs under docs/audits/ (#249 for FINDCDUW). #248
    merged FINDCDUW complete citing the lane's own pending-review notes;
    a lane cannot review itself, so that FAILs."""
    if key != 'complete_native_coverage':
        return
    review = (certificate.get('evidence') or {}).get('independent_review')
    if not review:
        return
    where = review.get('path', '') if isinstance(review, dict) else str(review)
    if not where.startswith('docs/audits/'):
        report.add('FAIL', label, f'independent_review {where or "(no path)"} is not a reviewer record',
                   'keep the certificate pending; the reviewer accepts it with a docs/audits/ record')


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('files', nargs='*', help='AGC files, e.g. Luminary099/TJET_LAW.agc')
    parser.add_argument('--changed', action='store_true', help='every certificate changed since --base')
    parser.add_argument('--base', help='git ref for --changed')
    parser.add_argument('--fetch', action='store_true',
                        help='download release-stored evidence that is cited and the newest vice manifest')
    parser.add_argument('--manifest', help='a vice-manifest.json to check vice_targets against')
    parser.add_argument('--quiet', action='store_true', help='omit NOTE lines')
    parser.add_argument('--write-facts', action='store_true', help='regenerate the facts from the listings')
    parser.add_argument('--check-facts', action='store_true', help='fail when the facts are stale')
    args = parser.parse_args()
    if args.write_facts or args.check_facts:
        return write_facts(args.check_facts)
    paths = [p[len('../'):] if p.startswith('../') else p for p in args.files]
    if args.changed:
        if not args.base:
            parser.error('--changed needs --base')
        paths += changed_files(args.base)
    if not paths:
        print('PASS: no certificate to check')
        return 0
    manifest = False
    if args.manifest:
        sys.path.insert(0, str(ROOT / 'tools'))
        import vice_manifest  # noqa: E402
        manifest = vice_manifest.latest(path=args.manifest)[0]
    report = precheck(list(dict.fromkeys(paths)), Report(), notes=not args.quiet, fetch_missing=args.fetch,
                      manifest=manifest)
    for line in report.lines:
        print(line)
    verdict = 'FAIL' if report.fails else 'PASS'
    print(f'{verdict}: {len(paths)} file(s), {report.fails} FAIL, {report.warns} WARN')
    return 1 if report.fails else 0


if __name__ == '__main__':
    sys.exit(main())
