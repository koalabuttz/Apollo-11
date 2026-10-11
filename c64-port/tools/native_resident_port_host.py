#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check retained registry images and complete native source-state streams.

Needs a host C compiler and retained evidence only. Each invocation owns a
temporary build tree; it never rebuilds target modules or changes disks.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from overlay_build import crc16
from p66_application_port_host import inventories

ROOT = Path(__file__).resolve().parents[1]


def require(condition, detail):
    if not condition:
        raise AssertionError(detail)


def read_manifest(name, count):
    records = json.loads((ROOT / 'evidence' / name).read_text())
    require(len(records) == count, (name, 'count'))
    result = {}
    names = set()
    for record in records:
        identifier = record['id']
        filename = record['filename']
        require(isinstance(identifier, int) and 0 < identifier < 256,
                (name, identifier, 'id'))
        require(identifier not in result and filename not in names,
                (name, identifier, 'duplicate'))
        encoded = filename.encode('ascii')
        require(0 < len(encoded) <= 16 and b'\0' not in encoded,
                (name, identifier, 'filename'))
        retained = record.get('retained_prg', 'evidence/' + record['prg'])
        path = ROOT / retained
        require(path.is_file() and path.resolve().is_relative_to(ROOT),
                (name, identifier, 'retained path'))
        image = path.read_bytes()
        payload = image[2:]
        require(hashlib.sha256(image).hexdigest() == record['sha256'],
                (name, identifier, 'SHA256'))
        require(image[:2] == b'\x00\x80' and 16 <= len(payload) <= 16384,
                (name, identifier, 'load address/slot'))
        require(payload[:8] == b'NATV\x01' + bytes([identifier, 16, 0]),
                (name, identifier, 'header'))
        word = lambda offset: int.from_bytes(payload[offset:offset + 2], 'little')
        require(word(8) == 0x8010 and word(14) == len(payload),
                (name, identifier, 'entry/size'))
        require(0x8000 + len(payload) <= word(10) <= 0xc000 and
                word(10) + word(12) <= 0xc000,
                (name, identifier, 'BSS bounds'))
        require(record['byte_count'] == len(payload) and
                record['crc16'] == crc16(payload),
                (name, identifier, 'descriptor size/CRC'))
        if 'bss_size' in record:
            require(record['bss_size'] == word(12),
                    (name, identifier, 'declared BSS'))
        result[identifier] = record
        names.add(filename)
    return result


def verify_retained():
    original = read_manifest('p66-recurrence-live-modules.json', 111)
    registry = read_manifest('native-registry-modules.json', 112)
    split = read_manifest('p66-application-split-modules.json', 114)
    require(set(registry) == set(original) | {148}, 'registry IDs')
    require(set(split) == set(original) | {148, 155, 159}, 'split IDs')
    for identifier, record in original.items():
        for extension in (registry, split):
            for field in ('id', 'filename', 'byte_count', 'crc16', 'sha256'):
                require(extension[identifier][field] == record[field],
                        (identifier, field, 'changed historical stage'))
    require(registry[148]['sha256'] != split[148]['sha256'],
            'separate bootstrap directories')
    # The released reader extracts explicit native source inventories from the
    # original qualified builders, without importing their build entry points.
    source_count = 0
    for name, sources, inputs, defines in inventories():
        require(sources and inputs and all(d.startswith('-D') for d in defines),
                (name, 'source-reader inventory'))
        for source in sources + inputs:
            require(source.startswith(('src/', 'tests/')) and
                    source.endswith('.c') and (ROOT / source).is_file(),
                    (name, source, 'source-reader path'))
            source_count += 1
    print('PASS:111 unchanged stages; 337 retained descriptors with full '
          'header/SHA256/CRC/BSS checks; %d source-reader entries' % source_count,
          flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    verify_retained()
    (ROOT / 'build').mkdir(exist_ok=True)
    mode = 'sanitize' if args.sanitize else 'strict'
    with tempfile.TemporaryDirectory(prefix='native-resident-port-' + mode + '-',
                                     dir=ROOT / 'build') as temporary:
        workspace = Path(temporary)
        for directory in ('src', 'include', 'tests', 'evidence'):
            (workspace / directory).symlink_to(ROOT / directory,
                                              target_is_directory=True)
        (workspace / 'build').mkdir()
        tool_dir = workspace / 'tools'
        tool_dir.mkdir()
        # Copy only the immutable runner so its output ROOT is this private
        # tree; its source-reader module still reads the real qualified inputs.
        runner = 'p66_application_split_host.py'
        shutil.copyfile(ROOT / 'tools' / runner, tool_dir / runner)
        for name in ('p66_application_port_host.py',
                     'p66_application_job_snapshot.c',
                     'p66_length_return_application_snapshot.c',
                     'p66_recurrence_live_snapshot.c'):
            (tool_dir / name).symlink_to(ROOT / 'tools' / name)
        flags = ['cc', '-std=c99', '-Wall', '-Wextra', '-Werror',
                 '-Iinclude', '-Itests', '-O1' if args.sanitize else '-O2']
        if args.sanitize:
            flags += ['-g', '-no-pie', '-fsanitize=address,undefined',
                      '-fno-omit-frame-pointer']
        binary = workspace / 'build' / 'registry'
        subprocess.run(flags + ['src/overlay_runtime.c',
            'src/native_module_registry.c', 'tests/test_native_module_registry.c',
            '-o', str(binary)], cwd=workspace, check=True)
        environment = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
        subprocess.run([str(binary)], cwd=workspace, env=environment, check=True)
        command = [sys.executable, str(tool_dir / runner)]
        if args.sanitize:
            command.append('--sanitize')
        subprocess.run(command, cwd=workspace, env=environment, check=True)
    print('PASS:native resident port %s; 16998 complete original states, '
          'resolver/adoption and registry/error guards' % mode, flush=True)


if __name__ == '__main__':
    main()
