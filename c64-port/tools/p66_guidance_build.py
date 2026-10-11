#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fit source-safe resident scheduler and actual arithmetic on a D81 disk."""
import argparse,pathlib,json,subprocess,hashlib,os
from overlay_build import crc16
from overlay_runtime_library import prepare
root=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');p.add_argument('--rebuild-arithmetic',action='store_true');a=p.parse_args();build=root/'build'
if a.rebuild_arithmetic:subprocess.run(['python3','tools/p66_guidance_overlay_build.py','--cc',a.cc],cwd=root,check=True)
manifest=[m for m in json.loads((build/'overlay-p66-modules.json').read_text())if m['id']in(1,2,4)]+json.loads((build/'overlay-p66-extended-modules.json').read_text())
common=[a.cc,'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror'];library=prepare(a.cc,build)
for number,name,sources in[(248,'P66INX',['platform/c64/p66_guidance_fixture_input.c','tests/p66_guidance_live_cases.c','src/p66_native.c','src/executive.c','src/agc_word.c']),(249,'P66CKX',['platform/c64/p66_guidance_fixture_check.c','tests/p66_guidance_live_cases.c'])]:
 output=build/f'p66-guidance-fixture-{number}.prg'
 subprocess.run(common+['-DPGL_FROZEN_INPUT','-DP66_NATIVE_DISPATCH_ONLY','-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={number},-Map,build/p66-guidance-fixture-{number}.map']+sources+['platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(output)],cwd=root,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=number,filename=name,byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=False))
(build/'p66-guidance-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
header='#include "overlay_runtime.h"\n#define P66_GUIDANCE_MODULES %du\nstatic const apollo_overlay_module p66_guidance_disk_modules[%d]={\n'%(len(manifest),len(manifest))+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16'])for m in manifest)+'};\n';(build/'p66_guidance_modules.h').write_text(header)
subprocess.run(common+['-DP66_NATIVE_DISPATCH_ONLY','-DP66_GUIDANCE_DISPATCH_ONLY','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/p66-guidance.map','src/agc_word.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/p66_guidance_diagnostic.c','-o','build/p66-guidance.prg'],cwd=root,check=True)
disk=build/'p66-guidance.d81';disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','APOLLO P66,11','d81',str(disk),'-write',str(build/'p66-guidance.prg'),'resident']
for m in manifest:
 data=(build/m['prg']).read_bytes();assert hashlib.sha256(data).hexdigest()==m['sha256'];cmd+=['-write',str(build/m['prg']),m['filename'].lower()]
subprocess.run(cmd,cwd=root,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')))
print('Built real source-safe P66 resident,27 arithmetic and2 proof-data overlays on D81')
