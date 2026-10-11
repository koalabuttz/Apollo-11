#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile native application paths; stream against retained original archives.

No original AGC engine, generated build snapshots, or target toolchain is needed.
The snapshots contain every original output word; no normalization is applied.
"""
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


def module_sources(names):
    result = []
    for name in names:
        for line in (ROOT / f'tools/{name}.mk').read_text().splitlines():
            if '_CORE = ' in line:
                result.extend(p for p in line.split(' = ', 1)[1].split()
                              if p.startswith('src/'))
    return result


def inventories():
    bodies = module_sources(('p66_launch', 'p66_mode_display',
                             'dsky_charin_nv', 'dsky_charin_number'))
    start = ['src/p66_application.c', 'src/p66_launch_phased.c'] + bodies
    start_inputs = ['tests/p66_bias_cases.c', 'tests/p66_vertical_start_cases.c',
                    'tests/p66_launch_cases.c', 'tests/p66_direct_launch_cases.c']
    job = ['src/p66_application.c', 'src/p66_launch_phased.c',
           'src/p66_direct_launch.c', 'src/p66_native_guidance.c',
           'src/p66_guidance_invoke.c', 'src/p66_descent_runtime.c',
           'src/p66_descent_prefix_control.c', 'src/p66_descent_invoke.c',
           'src/guidance_control_observer.c', 'src/p66_basic.c',
           'src/dsky_display.c'] + module_sources(('display_job',)) + bodies
    job_inputs = ['tests/p66_bias_cases.c', 'tests/p66_mode_display_cases.c',
                  'tests/dsky_nv_key_cases.c', 'tests/dsky_charin_nv_cases.c',
                  'tests/dsky_charin_number_cases.c',
                  'tests/p66_application_job_cases.c']
    # Read only the explicit build inventory assignments in the released path.
    scope = {'root': ROOT}
    names = {'inputs', 'defines', 'numerical', 'native', 'extra'}
    for node in ast.parse((ROOT / 'tools/p66_complete_live_differential.py')
                          .read_text()).body:
        if isinstance(node, (ast.Assign, ast.AugAssign)):
            targets = node.targets if isinstance(node, ast.Assign) else [node.target]
            if all(isinstance(t, ast.Name) and t.id in names for t in targets):
                exec(compile(ast.Module(body=[node], type_ignores=[]),
                             'compiled-source-inventory', 'exec'), scope)
    live = scope['native'] + scope['numerical'] + [
        'src/p66_application.c', 'src/p66_launch_phased.c',
        'src/p66_direct_launch.c', 'src/dsky_display.c'] + bodies
    guard = ['-DP66_NATIVE_DISPATCH_ONLY', '-DP66_GUIDANCE_DISPATCH_ONLY',
             '-DP66_BASIC_DISPATCH_ONLY', '-DP66_DIRECT_LAUNCH_DISPATCH_ONLY']
    return [('start', start, start_inputs, []),
            ('job', job, job_inputs, guard),
            ('live', live, scope['inputs'] + ['tests/p66_application_live_cases.c'],
             scope['defines'] + ['-DP66_BASIC_DISPATCH_ONLY',
                                 '-DP66_DIRECT_LAUNCH_DISPATCH_ONLY'])]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cc', default='cc')
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    (ROOT / 'build').mkdir(exist_ok=True)
    proof = json.loads((ROOT / 'evidence/p66-application-startup-proof.json').read_text())
    flags = [args.cc, '-std=c99', '-O1' if args.sanitize else '-O2', '-Wall',
             '-Wextra', '-Werror', '-Iinclude', '-Itests', '-ffunction-sections',
             '-fdata-sections', '-Wl,--gc-sections']
    if args.sanitize:
        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
    for name, sources, factories, defines in inventories():
        binary = ROOT / f'build/p66-application-port-{name}'
        subprocess.run(flags + list(dict.fromkeys(sources + factories)) + defines
                       + [f'tools/p66_application_{name}_snapshot.c', '-o', str(binary)],
                       cwd=ROOT, check=True)
        archive = ROOT / f'evidence/p66-application-{name}-source.snapshots.xz'
        assert hashlib.sha256(archive.read_bytes()).hexdigest() == proof['files'][str(archive.relative_to(ROOT))]
        digest = hashlib.sha256()
        count = 0
        with lzma.open(archive, 'rb') as expected, (ROOT / f'build/p66-application-port-{name}.trace').open('wb') as trace:
            process = subprocess.Popen([str(binary)], cwd=ROOT, stdout=subprocess.PIPE,
                                       stderr=trace, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
            try:
                for count, (source, native) in enumerate(itertools.zip_longest(expected, process.stdout), 1):
                    if source != native:
                        raise AssertionError(f'{name} checkpoint {count}: raw original/native mismatch')
                    digest.update(source)
                assert process.wait() == 0, f'{name}: native executable failed'
            finally:
                if process.poll() is None:
                    process.kill()
                    process.wait()
                process.stdout.close()
        assert count == proof['host'][name]['states']
        assert digest.hexdigest() == proof['host'][name]['sha256']
        print(f'PASS: {name}: {count} complete original states; SHA256 {digest.hexdigest()}', flush=True)


if __name__ == '__main__':
    main()
