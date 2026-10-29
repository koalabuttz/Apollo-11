#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build real startup, keyboard and relay entries in the existing fixed slot."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from overlay_build import crc16
from overlay_runtime_library import prepare

ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--cc', default='mos-c64-clang')
p.add_argument('--groups', default='50,83,51,52,53,54,55,56')
a = p.parse_args()
build = ROOT / 'build'
library = prepare(a.cc, build)
sources = ['src/p66_direct_launch.c', 'src/p66_launch_phased.c']
for file, assignment in (('p66_launch', 'P66_LAUNCH_CORE'), ('p66_mode_display', 'P66_MODE_CORE'),
                         ('dsky_key_irq', 'DSKY_IRQ_CORE'), ('dsky_charin_nv', 'DSKY_CHARIN_CORE'),
                         ('dsky_charin_number', 'DSKY_CHARIN_NUM_CORE'), ('dsky_relay', 'DSKY_RELAY_CORE')):
    text = (ROOT / f'tools/{file}.mk').read_text()
    candidates = [line for line in text.splitlines() if 'CORE = ' in line]
    assert candidates, file
    for line in candidates:
        sources += [name for name in line.split(' = ', 1)[1].split() if name.startswith('src/')]
sources = list(dict.fromkeys(sources))
manifest = []
for group in map(int, a.groups.split(',')):
    assert 50 <= group <= 56 or group == 83
    output = build / f'overlay-p66-application-{group}.prg'
    flags = [a.cc, '-Iinclude', '-Iplatform/c64', '-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror', '-nostdlib',
             f'-DP66_APPLICATION_SERVICE={group}', '-T', 'platform/c64/overlay_payload.ld',
             f'-Wl,--defsym=__overlay_module={group},-Map,build/overlay-p66-application-{group}.map']
    subprocess.run(flags + sources + ['platform/c64/p66_application_service_payload.c', 'platform/c64/overlay_entry.S', '-lc', library, '-lcrt', '-o', str(output)], cwd=ROOT, check=True)
    data = output.read_bytes()
    assert data[:2] == b'\x00\x80' and len(data) - 2 <= 16384
    manifest.append(dict(id=group, filename=f'A{group}', byte_count=len(data) - 2, crc16=crc16(data[2:]), sha256=hashlib.sha256(data).hexdigest(), prg=output.name, mission_translation=True))
    print(group, len(data) - 2, 'loaded bytes')
(build / 'overlay-p66-application-modules.json').write_text(json.dumps(manifest, indent=2) + '\n')
