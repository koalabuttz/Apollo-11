#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Stream canonical recurrence bridges against immutable original source states."""
import argparse
import ast
import hashlib
import itertools
import json
import lzma
import os
from pathlib import Path
import subprocess
ROOT = Path(__file__).resolve().parents[1]
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cc', default='cc')
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    proof = json.loads((ROOT / 'evidence/lm-recurrence-bridges-proof.json').read_text())
    flags = [args.cc, '-std=c99', '-O1' if args.sanitize else '-O2', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections']
    if args.sanitize:
        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
    (ROOT / 'build').mkdir(exist_ok=True)
    for key, expected_info in proof['host'].items():
        kind = key.replace('-', '_')
        tree = ast.parse((ROOT / f'tools/{kind}_differential.py').read_text())
        values = {node.targets[0].id: ast.literal_eval(node.value) for node in tree.body if isinstance(node, ast.Assign) and len(node.targets) == 1 and isinstance(node.targets[0], ast.Name) and node.targets[0].id in ('sources', 'factories')}
        binary = ROOT / f'build/{key}-port-host'
        subprocess.run(flags + values['sources'] + values['factories'] + [f'tools/{kind}_snapshot.c', '-o', str(binary)], cwd=ROOT, check=True)
        name = f'evidence/{key}-reference.snapshots.xz'
        archive = ROOT / name
        assert hashlib.sha256(archive.read_bytes()).hexdigest() == proof['files'][name]
        digest = hashlib.sha256()
        count = 0
        with lzma.open(archive, 'rb') as expected:
            process = subprocess.Popen([str(binary)], cwd=ROOT, stdout=subprocess.PIPE, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
            try:
                for count, (source, native) in enumerate(itertools.zip_longest(expected, process.stdout), 1):
                    if source != native:
                        raise AssertionError(f'{key} checkpoint{count}: original/native mismatch')
                    digest.update(source)
                assert process.wait() == 0
            finally:
                if process.poll() is None:
                    process.kill()
                    process.wait()
                process.stdout.close()
        assert count == expected_info['states']
        assert digest.hexdigest() == expected_info['sha256']
        print(f'PASS {key}: {count} complete original states, SHA256 {digest.hexdigest()}', flush=True)
if __name__ == '__main__':
    main()
