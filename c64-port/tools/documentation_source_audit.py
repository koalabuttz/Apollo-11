#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Preserve and qualify documentation-only source, with no algorithm claim."""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
FILES = ('LUNAR_LANDMARK_SELECTION_FOR_CM.agc', 'CONTRACT_AND_APPROVALS.agc')


def main():
    work = ROOT / 'build/documentation-source'
    work.mkdir(exist_ok=True)
    baseline = ROOT / 'build/fixed-data/Comanche055'
    for source in baseline.glob('*.agc'):
        shutil.copyfile(source, work / source.name)
    maps = []
    for name in FILES:
        source = ROOT.parent / 'Comanche055' / name
        original = source.read_bytes()
        lines = original.decode().splitlines()
        # This independent lexical check accepts only full-line comments or
        # whitespace, so it cannot discard a trailing actual source operation.
        assert all(not line.strip() or line.lstrip().startswith('#') for line in lines)
        shutil.copyfile(source, work / name)
        preserved = ROOT / 'evidence' / ('documentation-source-' + name + '.original')
        preserved.write_bytes(original)
        assert preserved.read_bytes() == original
        maps.append({'source_path': 'Comanche055/' + name,
                     'source_sha256': hashlib.sha256(original).hexdigest(),
                     'preserved_original': str(preserved.relative_to(ROOT)),
                     'byte_count': len(original), 'line_count': len(lines),
                     'source_lines': [{'line': i, 'kind': 'comment' if text.strip() else 'blank',
                                       'mapping': 'byte-exact retained documentation; no native operation'}
                                      for i, text in enumerate(lines, 1)],
                     'emitted_code_words': 0, 'emitted_data_words': 0,
                     'declarations': 0, 'algorithms_translated': 0})
    with (work / 'assembly.lst').open('w') as out:
        subprocess.run([str(ROOT / 'deps/virtualagc/yaYUL/yaYUL'), 'MAIN.agc'],
                       cwd=work, stdout=out, stderr=subprocess.STDOUT, check=True)
    rope = (work / 'MAIN.agc.bin').read_bytes()
    assert rope == (baseline / 'MAIN.agc.bin').read_bytes()
    result = {'schema': 1, 'source_files': maps,
              'fresh_original_comments_rope_sha256': hashlib.sha256(rope).hexdigest(),
              'whole_rope_identical_to_existing_original_layout_baseline': True,
              'runtime_code_added': False, 'native_algorithm_translation_claim': False,
              'instruction_decoder': False, 'application_completion_claim': False}
    (ROOT / 'evidence/documentation-source-audit.json').write_text(json.dumps(result, indent=2) + '\n')
    print('PASS: both original CM files contain comments/blank lines only; no code, data or declarations')
    print('PASS: original bytes preserved; fresh assembly emits a byte-identical complete CM rope')
    print('CM rope SHA256:', hashlib.sha256(rope).hexdigest())


if __name__ == '__main__':
    main()
