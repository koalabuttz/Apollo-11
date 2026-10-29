#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run retained complete source comparisons in a private host build tree."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from native_resident_port_host import read_manifest, require
from p66_application_port_host import inventories

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    original = read_manifest('p66-application-split-modules.json', 114)
    current = read_manifest('p66-pending-flight-modules.json', 119)
    require(set(current) == set(original) | {88, 89, 97, 98, 99}, 'stage IDs')
    unchanged = set(original) - {148, 245}
    for identifier in unchanged:
        for field in ('id', 'filename', 'byte_count', 'crc16', 'sha256'):
            require(current[identifier][field] == original[identifier][field],
                    (identifier, field, 'historical image changed'))
    print('PASS:119 retained descriptors;112 historical stage identities '
          'unchanged; complete SHA/header/size/BSS/CRC checks', flush=True)
    (ROOT / 'build').mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='p66-pending-port-',
                                     dir=ROOT / 'build') as temporary:
        workspace = Path(temporary)
        for directory in ('src', 'include', 'tests', 'evidence'):
            (workspace / directory).symlink_to(ROOT / directory,
                                              target_is_directory=True)
        (workspace / 'build').mkdir()
        tools = workspace / 'tools'
        tools.mkdir()
        runner = 'p66_pending_flight_differential.py'
        shutil.copyfile(ROOT / 'tools' / runner, tools / runner)
        for name in ('p66_application_port_host.py',
                     'p66_pending_flight_snapshot.c',
                     'p66_pending_adoption_snapshot.c'):
            (tools / name).symlink_to(ROOT / 'tools' / name)
        for options in ([], ['--adoption'], ['--without-preemption']):
            command = [sys.executable, str(tools / runner), '--retained'] + options
            if args.sanitize:
                command.append('--sanitize')
            subprocess.run(command, cwd=workspace, check=True)
        _, sources, inputs, defines = inventories()[2]
        extra = ['src/p66_length_return_control.c', 'src/p66_application_recurrence.c',
                 'src/lm_clock.c', 'src/lm_idle_start.c', 'src/lm_taskover_resume.c',
                 'src/lm_waitlist_dispatch.c', 'src/lm_waitlist_irq_dispatch.c',
                 'src/p66_rodtask_image.c', 'src/p66_application_split.c',
                 'src/p66_application_pending.c', 'src/p66_elapsed_return_control.c',
                 'src/lm_danzig_change.c', 'src/lm_interpreter_change.c']
        flags = ['cc', '-std=c99', '-O1' if args.sanitize else '-O2',
                 '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests',
                 '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections']
        if args.sanitize:
            flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
        binary = workspace / 'build' / 'guards'
        subprocess.run(flags + defines + list(dict.fromkeys(sources + extra + inputs +
            ['tests/p66_recurrence_live_cases.c'])) + ['tests/test_p66_pending_guards.c',
            '-o', str(binary)], cwd=workspace, check=True)
        subprocess.run([str(binary)], cwd=workspace,
                       env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'), check=True)
    print('PASS:pending P66 port %s;2476 pending-job,64 first-adoption and '
          '2460 historical complete source states' %
          ('sanitize' if args.sanitize else 'strict'), flush=True)


if __name__ == '__main__':
    main()
