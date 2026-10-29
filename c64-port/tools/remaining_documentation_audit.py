#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Preserve the remaining comment-only mission source and prove zero emissions."""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
FILES = {
    'Comanche055': ('ASSEMBLY_AND_OPERATION_INFORMATION.agc', 'ENTRY_LEXICON.agc'),
    'Luminary099': ('ASSEMBLY_AND_OPERATION_INFORMATION.agc',
                    'INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS.agc'),
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    records = []
    ropes = {}
    for mission, names in FILES.items():
        baseline = ROOT / 'build/fixed-data' / mission
        work = ROOT / 'build/remaining-documentation' / mission
        work.mkdir(parents=True, exist_ok=True)
        for source in baseline.glob('*.agc'):
            shutil.copyfile(source, work / source.name)
        expected = (ROOT / ('evidence/fixed-data-' +
                    ('lm' if mission == 'Luminary099' else 'cm') +
                    '-original.rope.bin')).read_bytes()
        assert (baseline / 'MAIN.agc.bin').read_bytes() == expected
        for name in names:
            source = ROOT.parent / mission / name
            original = source.read_bytes()
            lines = original.splitlines()
            assert all(not row.strip() or row.lstrip().startswith(b'#') for row in lines)
            preserved = ROOT / 'evidence' / ('remaining-documentation-' + mission + '-' + name + '.original')
            preserved.write_bytes(original)
            assert preserved.read_bytes() == original
            shutil.copyfile(source, work / name)
            records.append({
                'source_path': mission + '/' + name,
                'source_sha256': digest(original),
                'preserved_original': str(preserved.relative_to(ROOT)),
                'lines': [{'line': i, 'mapping': 'byte-exact documentation; no native operation'}
                          for i in range(1, len(lines) + 1)],
                'algorithms_translated': 0, 'emitted_words': 0,
            })
        with (work / 'assembly.lst').open('w') as output:
            subprocess.run([str(ROOT / 'deps/virtualagc/yaYUL/yaYUL'), 'MAIN.agc'],
                           cwd=work, stdout=output, stderr=subprocess.STDOUT, check=True)
        rope = (work / 'MAIN.agc.bin').read_bytes()
        assert rope == expected
        ropes[mission] = digest(rope)
    result = {'schema': 1, 'files': records, 'fresh_rope_sha256': ropes,
              'whole_original_ropes_identical': True,
              'algorithms_translated': 0, 'emitted_words': 0,
              'application_completion_claim': False}
    (ROOT / 'evidence/remaining-documentation-audit.json').write_text(json.dumps(result, indent=2) + '\n')
    print('PASS: four original files are entirely comments/whitespace, preserved byte-for-byte')
    print('PASS: fresh Luminary and Comanche assemblies match original-layout ropes')
    print(json.dumps(ropes, sort_keys=True))


if __name__ == '__main__':
    main()
