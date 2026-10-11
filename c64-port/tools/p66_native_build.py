#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Production resident and actual arithmetic overlays; proof inputs stream separately."""
import argparse,pathlib,json,subprocess,hashlib,os
from overlay_build import crc16
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');p.add_argument('--cases',type=int,choices=(3,8),default=3);a=p.parse_args();build=root/'build'
subprocess.run(['python3','tools/p66_overlay_build.py','--cc',a.cc],cwd=root,check=True)
manifest=json.loads((build/'overlay-p66-modules.json').read_text())
common=[a.cc,'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror']
for number,name,sources,flags in [(250,'P66IN',['platform/c64/p66_native_fixture_input.c','tests/p66_native_cases.c','src/p66_native.c','src/executive.c','src/agc_word.c'],['-DP66_NATIVE_C64_INPUT','-DP66_NATIVE_DISPATCH_ONLY']), (251,'P66CK',['platform/c64/p66_native_fixture_check.c','tests/p66_native_cases.c'],['-DP66_NATIVE_REFERENCE_INPUT'])]:
 output=build/f'p66-native-fixture-{number}.prg'
 subprocess.run(common+flags+['-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={number},-Map,build/p66-native-fixture-{number}.map']+sources+['platform/c64/overlay_entry.S','-lc','-lcrt','-o',str(output)],cwd=root,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=number,filename=name,byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=False))
(build/'p66-native-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
header='#include "overlay_runtime.h"\nstatic const apollo_overlay_module p66_native_modules[7]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n'
(build/'p66_native_modules.h').write_text(header)
subprocess.run(common+['-DP66_NATIVE_DISPATCH_ONLY',f'-DP66_NATIVE_DIAGNOSTIC_CASES={a.cases}','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/p66-native.map','src/agc_word.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/p66_native_diagnostic.c','-o','build/p66-native.prg'],cwd=root,check=True)
disk=build/'p66-native.d64';disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','APOLLO P66,11','d64',str(disk),'-write',str(build/'p66-native.prg'),'resident']
for m in manifest:cmd+=['-write',str(build/m['prg']),m['filename'].lower()]
env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config'))
subprocess.run(cmd,cwd=root,check=True,env=env)
print('Built real P66 Executive resident, five numerical modules, two proof-data modules and D64')
