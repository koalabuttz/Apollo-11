#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build actual alarm inline-data/control and original-state diagnostic overlays."""
import argparse, pathlib, json, subprocess, hashlib, os
from overlay_build import crc16
from overlay_runtime_library import prepare
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--cc', default='mos-c64-clang')
p.add_argument('--c1541', default='c1541')
a = p.parse_args()
library = prepare(a.cc, B/'cm-alarm-complete-sdk')
common = [a.cc, '-Iinclude', '-Itests', '-Iplatform/c64', '-Ibuild', '-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
manifest = []
for number in [233, 254, 255]:
    if number == 233:
        sources = ['platform/c64/cm_alarm_complete_control.c', 'src/cm_alarm_complete.c','src/cm_alarm_image.c', 'src/cm_alarm_record.c', 'src/agc_word.c']
    elif number == 254:
        sources = ['platform/c64/cm_alarm_complete_input.c', 'tests/cm_alarm_complete_cases.c','tests/cm_alarm_image_cases.c', 'tests/cm_alarm_record_cases.c', 'tests/cm_admission_complete_cases.c', 'src/cm_native_image.c', 'src/agc_word.c']
    else:
        sources = ['platform/c64/cm_alarm_complete_check.c']
    output = B/f'cm-alarm-complete-{number}.prg'
    subprocess.run(common+['-nostdlib', '-T', 'platform/c64/overlay_payload.ld', f'-Wl,--defsym=__overlay_module={number},-Map,build/cm-alarm-complete-{number}.map']+sources+['platform/c64/overlay_entry.S', '-lc', library, '-lcrt', '-o', str(output)], cwd=R, check=True)
    raw = output.read_bytes()
    assert raw[:2] == b'\x00\x80' and len(raw)-2 <= 16384
    manifest.append(dict(id=number, filename=f'G{number}', byte_count=len(raw)-2, crc16=crc16(raw[2:]), sha256=hashlib.sha256(raw).hexdigest(), prg=output.name, native_control=number==233))
    print(number, len(raw)-2, 'loaded bytes')
for m in json.loads((B/'cm-fixed-compat-modules.json').read_text())[:6]:
    raw=(B/m['prg']).read_bytes();assert hashlib.sha256(raw).hexdigest()==m['sha256'];manifest.append(m)
(B/'cm-alarm-complete-modules.json').write_text(json.dumps(manifest, indent=2)+'\n')
(B/'cm_alarm_complete_modules.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module cm_alarm_complete_modules[9]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n')
subprocess.run(common+['-T', 'platform/c64/overlay_resident.ld', '-Wl,-Map,build/cm-alarm-complete.map', 'src/overlay_runtime.c', 'platform/c64/overlay_platform.c', 'platform/c64/cm_alarm_complete_diagnostic.c', '-o', 'build/cm-alarm-complete.prg'], cwd=R, check=True)
disk = B/'cm-alarm-complete.d64'
disk.unlink(missing_ok=True)
cmd = [a.c1541, '-format', 'APOLLO CM,11', 'd64', str(disk), '-write', str(B/'cm-alarm-complete.prg'), 'resident']
for m in manifest:
    cmd += ['-write', str(B/m['prg']), m['filename'].lower()]
subprocess.run(cmd, cwd=R, check=True, env=dict(os.environ, XDG_CACHE_HOME=str(B/'cm-alarm-complete-cache'), XDG_CONFIG_HOME=str(B/'cm-alarm-complete-config')))
