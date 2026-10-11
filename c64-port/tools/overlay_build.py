#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build two independent native overlay proof bodies, resident, and D64."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def crc16(data):
    crc = 65535
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ (0x1021 if crc & 32768 else 0)) & 65535
    return crc


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--cc', default='mos-c64-clang')
    p.add_argument('--c1541', default='c1541')
    args = p.parse_args()
    build = ROOT / 'build'
    build.mkdir(exist_ok=True)
    common = [args.cc, '-Iinclude', '-Iplatform/c64', '-Ibuild', '-std=c99', '-Os',
              '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
    manifest = []
    for number, name in enumerate(('a', 'b'), 1):
        filename = 'OVL' + name.upper()
        output = build / ('overlay-' + name + '.prg')
        subprocess.run(common + ['-nostdlib', '-T', 'platform/c64/overlay_payload.ld',
            f'-Wl,--defsym=__overlay_module={number},--defsym=__overlay_code_origin=0xa000,-Map,build/overlay-{name}.map',
            f'tests/overlay_payload_{name}.c', 'platform/c64/overlay_entry.S', '-lcrt',
            '-o', str(output)], cwd=ROOT, check=True)
        file = output.read_bytes()
        data = file[2:]
        assert file[:2] == b'\x00\x80' and data[:4] == b'NATV'
        assert data[5] == number and int.from_bytes(data[14:16], 'little') == len(data)
        assert len(data) <= 16384
        manifest.append(dict(id=number, filename=filename, byte_count=len(data),
                             crc16=crc16(data), sha256=hashlib.sha256(file).hexdigest(),
                             prg=output.name, native_sources=[f'tests/overlay_payload_{name}.c'],
                             mission_translation=False))
    (build / 'overlay-modules.json').write_text(json.dumps(manifest, indent=2) + '\n')
    header = '#include "overlay_runtime.h"\nstatic const apollo_overlay_module overlay_modules[2]={\n'
    header += ''.join('    {%d,"%s",%d,%d},\n' %
        (m['id'], m['filename'], m['byte_count'], m['crc16']) for m in manifest)
    header += '};\n'
    (build / 'overlay_modules.h').write_text(header)
    subprocess.run(common + ['-T', 'platform/c64/overlay_resident.ld',
        '-Wl,-Map,build/overlay-resident.map', 'src/overlay_runtime.c',
        'platform/c64/overlay_platform.c', 'platform/c64/overlay_diagnostic.c',
        'src/agc_word.c', 'src/executive.c', 'src/native_dispatch.c',
        'src/irq_delivery.c', '-o', 'build/overlay-resident.prg'], cwd=ROOT, check=True)
    disk = build / 'overlay.d64'
    disk.unlink(missing_ok=True)
    command = [args.c1541, '-format', 'APOLLO NATIVE,11', 'd64', str(disk)]
    # c1541 converts host lowercase ASCII to the unshifted PETSCII uppercase
    # used by the target's ordinary ASCII C filename literals.
    command += ['-write', str(build / 'overlay-resident.prg'), 'resident']
    for module in manifest:
        command += ['-write', str(build / module['prg']), module['filename'].lower()]
    environment = dict(os.environ, XDG_CACHE_HOME=str(build / 'vice-cache'),
                       XDG_CONFIG_HOME=str(build / 'vice-config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(environment[key]).mkdir(exist_ok=True)
    subprocess.run(command, cwd=ROOT, check=True, env=environment)
    print('Built native resident + two separately linked C ABI overlays + D64:', disk)


if __name__ == '__main__':
    main()
