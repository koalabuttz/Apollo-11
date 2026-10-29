#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build actual interbank branch overlays beside byte-pinned passive data."""
import argparse, pathlib, json, subprocess, hashlib, os
from overlay_build import crc16
from overlay_runtime_library import prepare
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--cc', default='mos-c64-clang')
p.add_argument('--c1541', default='c1541')
a = p.parse_args()
library = prepare(a.cc, B/'cm-interbank-extended-sdk')
common = [a.cc, '-Iinclude', '-Itests', '-Iplatform/c64', '-Ibuild', '-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
manifest = json.loads((B/'cm-fixed-compat-modules.json').read_text())[:6]
for m in manifest:
    assert hashlib.sha256((B/m['prg']).read_bytes()).hexdigest() == m['sha256']
for number in [207, 242, 243]:
    if number == 207:
        sources = ['platform/c64/cm_interbank_extended_control.c', 'src/cm_interbank_extended.c', 'src/agc_word.c']
    elif number == 242:
        sources = ['platform/c64/cm_interbank_extended_input.c', 'tests/cm_interbank_extended_cases.c', 'tests/cm_fixed_compat_cases.c', 'src/agc_word.c']
    else:
        sources = ['platform/c64/cm_interbank_extended_check.c']
    output = B/f'cm-interbank-extended-{number}.prg'
    subprocess.run(common+['-nostdlib', '-T', 'platform/c64/overlay_payload.ld', f'-Wl,--defsym=__overlay_module={number},-Map,build/cm-interbank-extended-{number}.map']+sources+['platform/c64/overlay_entry.S', '-lc', library, '-lcrt', '-o', str(output)], cwd=R, check=True)
    raw = output.read_bytes()
    assert raw[:2] == b'\x00\x80' and len(raw)-2 <= 16384
    manifest.append(dict(id=number, filename=f'G{number}', byte_count=len(raw)-2, crc16=crc16(raw[2:]), sha256=hashlib.sha256(raw).hexdigest(), prg=output.name, native_control=number==207))
    print(number, len(raw)-2, 'loaded bytes')
(B/'cm-interbank-extended-modules.json').write_text(json.dumps(manifest, indent=2)+'\n')
(B/'cm_interbank_extended_modules.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module cm_interbank_extended_modules[9]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n')
subprocess.run(common+['-T', 'platform/c64/overlay_resident.ld', '-Wl,-Map,build/cm-interbank-extended.map', 'src/cm_interbank_extended.c', 'src/agc_word.c', 'src/overlay_runtime.c', 'platform/c64/overlay_platform.c', 'platform/c64/cm_interbank_extended_diagnostic.c', '-o', 'build/cm-interbank-extended.prg'], cwd=R, check=True)
disk = B/'cm-interbank-extended.d64'
disk.unlink(missing_ok=True)
cmd = [a.c1541, '-format', 'APOLLO CM,11', 'd64', str(disk), '-write', str(B/'cm-interbank-extended.prg'), 'resident']
for m in manifest:
    cmd += ['-write', str(B/m['prg']), m['filename'].lower()]
subprocess.run(cmd, cwd=R, check=True, env=dict(os.environ, XDG_CACHE_HOME=str(B/'cm-interbank-extended-cache'), XDG_CONFIG_HOME=str(B/'cm-interbank-extended-config')))
