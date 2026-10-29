#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Verify the complete local CM ephemerides word union and frozen owners."""
from pathlib import Path
import hashlib
import json
import struct
import subprocess
from fixed_data_generate import listing_records

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    path = 'Comanche055/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc'
    original = ROOT.parent / path
    baseline = subprocess.check_output(
        ['git', 'show', '911e5c0283c629c50cb97666f34065e8c07d71a5:' + path],
        cwd=ROOT)
    assert original.read_bytes() == baseline
    work = ROOT / 'build/ephemeris-source/Comanche055'
    alias = next(f.name for f in work.glob('*.agc')
                 if f.read_bytes() == baseline)
    listing = work / 'cm-complete-root-review.lst'
    with listing.open('w') as out:
        subprocess.run([str(ROOT / 'deps/virtualagc/yaYUL/yaYUL'), 'MAIN.agc'],
                       cwd=work, stdout=out, stderr=subprocess.STDOUT, check=True)
    rope = (work / 'MAIN.agc.bin').read_bytes()
    assert rope == (ROOT / 'evidence/fixed-data-cm-original.rope.bin').read_bytes()
    records = listing_records(listing, {alias: path}, {path: original.read_text()}, rope)
    raw = struct.unpack('>36864H', rope)
    emitted = {}
    for row in records:
        for offset, word in enumerate(row['words']):
            at = row['physical_fixed_word'] + offset
            assert at not in emitted
            physical = at ^ 2048 if at // 1024 < 4 else at
            assert raw[physical] >> 1 == word
            emitted[at] = dict(line=row['line'], raw_word=word, op=row['op'])
    assert len(emitted) == 71
    owners = {}
    dependencies = {}
    packets = ['time', 'moon-callers', 'velocity', 'solar', 'positions']
    for key in packets:
        prefix = 'cm-ephemeris-' + key
        mapname = 'evidence/' + prefix + '-source-map.json'
        proofname = 'evidence/' + prefix + '-proof.json'
        mapping = json.loads((ROOT / mapname).read_text())
        proof = json.loads((ROOT / proofname).read_text())
        assert mapping['source_sha256'] == digest(original)
        assert mapping['rope_sha256'] == hashlib.sha256(rope).hexdigest()
        assert proof['source_states'] == 65536
        assert proof['strict_native'] == proof['full_ASAN_UBSAN'] == 'PASS'
        for name, expected in proof['files'].items():
            assert digest(ROOT / name) == expected, name
        for name, expected in proof['dependencies'].items():
            assert digest(ROOT / name) == expected, name
        subprocess.run(['git', 'ls-files', '--error-unmatch', '--', mapname, proofname],
                       cwd=ROOT, stdout=subprocess.DEVNULL, check=True)
        dependencies[mapname] = digest(ROOT / mapname)
        dependencies[proofname] = digest(ROOT / proofname)
        for row in mapping['source_line_word_map']:
            for offset, word in enumerate(row['words']):
                at = row['physical_fixed_word'] + offset
                assert emitted[at]['raw_word'] == word
                owners.setdefault(at, []).append(prefix)
    constants = {}
    for at, item in emitted.items():
        if item['op'] == '2DEC':
            constants[at] = item['raw_word']
    assert len(constants) == 4
    for at in constants:
        owners.setdefault(at, []).append('cm-ephemeris-velocity-original-fixed-operand')
    assert set(owners) == set(emitted)
    reviews = ['evidence/cm-ephemeris-whole-source-independent-review.json',
               'evidence/cm-ephemeris-moon-callers-independent-review.json']
    for name in reviews:
        review = json.loads((ROOT / name).read_text())
        for dependency, expected in review['hashes'].items():
            assert digest(ROOT / dependency) == expected, dependency
        dependencies[name] = digest(ROOT / name)
    caller_contracts = {
        'LSPOS': ['022557', '030231'],
        'LUNPOS': ['065673', '062742', '023327'],
        'LUNVEL': ['023377'],
        'SOLPOS': ['023377 declared public entry; no original CALL sites'],
    }
    result = dict(
        source=path, source_sha256=digest(original),
        source_baseline_unchanged=True, rope_sha256=hashlib.sha256(rope).hexdigest(),
        original_emitted_words=71, compiled_control_words=67, fixed_operand_words=4,
        uncovered_words=[], source_line_word_map=records,
        word_owners={str(at): value for at, value in sorted(owners.items())},
        dependencies=dependencies, actual_caller_returns=caller_contracts,
        supported_domain='Frozen canonical image, five VACs, coherent entry banks, '
                         'compiled caller returns, quiesced computational producers; '
                         'live ordinary time/coefficient operands. Restricted public '
                         'SOLPOS return remains declared rather than arbitrary.',
        external_continuations=['Caller entry/resume services and application scheduling',
                                'Restart policy and physical timing',
                                'Prelaunch coefficients and original 15-day validity'],
        complete_application_claim=False)
    (ROOT / 'build/cm-ephemeris-complete-root-review.json').write_text(
        json.dumps(result, indent=2) + '\n')
    print('PASS complete CM ephemerides: 67 compiled control words +4 original operands; all actual local caller returns')


if __name__ == '__main__':
    main()
