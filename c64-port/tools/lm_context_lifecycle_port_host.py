#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retained source qualification without fixture-engine or target rebuilds."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from lm_context_lifecycle_differential import CORE
from native_resident_port_host import read_manifest

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--sanitize', action='store_true')
    args = p.parse_args()
    def tokens(path):
        return [' '.join(line.split('#', 1)[0].split()) for line in path.read_text().splitlines()
                if line.split('#', 1)[0].strip()]
    assert tokens(ROOT.parent / 'Luminary099/EXECUTIVE.agc') == \
        tokens(ROOT / 'deps/fixture/EXECUTIVE.agc')
    read_manifest('lm-context-lifecycle-target-modules.json', 32)
    (ROOT / 'build').mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='lm-context-lifecycle-port-',
                                     dir=ROOT / 'build') as temporary:
        workspace = Path(temporary)
        for directory in ('src', 'include', 'tests', 'evidence'):
            (workspace / directory).symlink_to(ROOT / directory, target_is_directory=True)
        (workspace / 'build').mkdir()
        tools = workspace / 'tools'
        tools.mkdir()
        runner = 'lm_context_lifecycle_differential.py'
        shutil.copyfile(ROOT / 'tools' / runner, tools / runner)
        (tools / 'lm_context_lifecycle_snapshot.c').symlink_to(
            ROOT / 'tools/lm_context_lifecycle_snapshot.c')
        command = [sys.executable, str(tools / runner), '--retained']
        flags = ['cc', '-std=c99', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests',
                 '-O1' if args.sanitize else '-O2']
        if args.sanitize:
            command.append('--sanitize')
            flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
        subprocess.run(command, cwd=workspace, check=True)
        binary = workspace / 'build/guards'
        subprocess.run(flags + CORE + ['src/executive.c', 'src/lm_basic_priority.c',
            'src/lm_basic_sleep.c', 'tests/test_lm_context_lifecycle.c', '-o', str(binary)],
            cwd=workspace, check=True)
        subprocess.run([str(binary)], cwd=workspace,
                       env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'), check=True)
    print('PASS:retained mixed lifecycle %s;32768 full original states, '
          '16384 historical comparisons,32 checked PRGs' %
          ('sanitize' if args.sanitize else 'strict'), flush=True)


if __name__ == '__main__':
    main()
