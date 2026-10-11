#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build actual canonical admission overlays and resource failure boundaries."""
import argparse, pathlib, json, subprocess, hashlib, os
from overlay_build import crc16
from overlay_runtime_library import prepare
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--cc', default='mos-c64-clang')
p.add_argument('--c1541', default='c1541')
a = p.parse_args()
library = prepare(a.cc, B/'cm-idle-wake-sdk')
common = [a.cc, '-Iinclude', '-Itests', '-Iplatform/c64', '-Ibuild', '-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
manifest = []
groups={247:(0,['src/cm_basic_sleep.c','src/cm_dispatch.c','src/cm_executive.c','src/cm_native_image.c','src/agc_word.c']),246:(1,['src/cm_idle_interrupt.c','src/agc_word.c']),248:(2,['src/cm_task_transfer.c','src/cm_waitlist.c','src/cm_native_image.c','src/agc_word.c']),249:(3,['src/service_routines.c','src/agc_word.c']),250:(4,['src/cm_wake_complete.c','src/agc_word.c']),251:(5,['src/cm_idle_dispatch.c','src/cm_dispatch.c','src/cm_executive.c','src/cm_native_image.c','src/agc_word.c'])}
for number in [247,246,248,249,250,251,254,255]:
    group=-1
    if number in groups:
        group,core=groups[number]
        sources=['platform/c64/cm_idle_wake_control.c']+core
    elif number==254:
        sources=['platform/c64/cm_idle_wake_input.c','tests/cm_idle_wake_cases.c','tests/cm_idle_interrupt_cases.c','src/cm_waitlist.c','src/cm_native_image.c','src/agc_word.c']
    else:
        sources=['platform/c64/cm_idle_wake_check.c']
    output = B/f'cm-idle-wake-{number}.prg'
    subprocess.run(common+[f'-DCM_IDLE_WAKE_GROUP={group}','-nostdlib', '-T', 'platform/c64/overlay_payload.ld', f'-Wl,--defsym=__overlay_module={number},-Map,build/cm-idle-wake-{number}.map']+sources+['platform/c64/overlay_entry.S', '-lc', library, '-lcrt', '-o', str(output)], cwd=R, check=True)
    raw = output.read_bytes()
    assert raw[:2] == b'\x00\x80' and len(raw)-2 <= 16384
    manifest.append(dict(id=number, filename=f'G{number}', byte_count=len(raw)-2, crc16=crc16(raw[2:]), sha256=hashlib.sha256(raw).hexdigest(), prg=output.name, native_control=number in groups))
    print(number, len(raw)-2, 'loaded bytes')
(B/'cm-idle-wake-modules.json').write_text(json.dumps(manifest, indent=2)+'\n')
(B/'cm_idle_wake_modules.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module cm_idle_wake_modules[8]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n')
subprocess.run(common+['-T', 'platform/c64/overlay_resident.ld', '-Wl,-Map,build/cm-idle-wake.map', 'src/overlay_runtime.c', 'platform/c64/overlay_platform.c', 'platform/c64/cm_idle_wake_diagnostic.c', '-o', 'build/cm-idle-wake.prg'], cwd=R, check=True)
disk = B/'cm-idle-wake.d64'
disk.unlink(missing_ok=True)
cmd = [a.c1541, '-format', 'APOLLO CM,11', 'd64', str(disk), '-write', str(B/'cm-idle-wake.prg'), 'resident']
for m in manifest:
    cmd += ['-write', str(B/m['prg']), m['filename'].lower()]
subprocess.run(cmd, cwd=R, check=True, env=dict(os.environ, XDG_CACHE_HOME=str(B/'cm-idle-wake-cache'), XDG_CONFIG_HOME=str(B/'cm-idle-wake-config')))
