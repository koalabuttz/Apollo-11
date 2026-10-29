#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Real scheduler/math/basic stages and raw 2052-word proofs on one D81."""
import argparse,pathlib,json,subprocess,hashlib,os
from overlay_build import crc16
from overlay_runtime_library import prepare
root=pathlib.Path(__file__).resolve().parents[1];build=root/'build'
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');p.add_argument('--rebuild-stages',action='store_true');a=p.parse_args()
if a.rebuild_stages:
 for script in('p66_overlay_build','p66_guidance_overlay_build','p66_descent_overlay_build','p66_descent_control_overlay_build','native_scheduler_overlay_build','p66_display_job_overlay_build'):subprocess.run(['python3',f'tools/{script}.py','--cc',a.cc],cwd=root,check=True)
subprocess.run(['python3','tools/p66_recurrence_overlay_build.py','--cc',a.cc],cwd=root,check=True)
subprocess.run(['python3','tools/freeze_p66_recurrence_live.py'],cwd=root,check=True)
manifest=[m for m in json.loads((build/'overlay-p66-modules.json').read_text())if m['id']in(1,2,4)]
for file in('overlay-p66-extended-modules.json','overlay-p66-descent-modules.json','overlay-p66-control-modules.json','overlay-native-scheduler-modules.json','overlay-p66-display-modules.json','overlay-p66-application-modules.json','overlay-p66-recurrence-modules.json'):manifest+=json.loads((build/file).read_text())
common=[a.cc,'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror'];library=prepare(a.cc,build)
for number,name,sources in((244,'PRLINF',['platform/c64/p66_recurrence_fixture_input.c']),(245,'PRLCKF',['platform/c64/p66_recurrence_fixture_check.c','tests/p66_guidance_live_cases.c','tests/p66_descent_live_cases.c','tests/p66_complete_live_cases.c'])):
 output=build/f'p66-recurrence-live-fixture-{number}.prg';subprocess.run(common+['-DPGL_FROZEN_INPUT','-DP66_NATIVE_DISPATCH_ONLY','-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={number},-Map,build/p66-recurrence-live-fixture-{number}.map']+sources+['platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(output)],cwd=root,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=number,filename=name,byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=False))
assert len({m['id']for m in manifest})==len(manifest)
(build/'p66-recurrence-live-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
header='#include "overlay_runtime.h"\n#define P66_RECURRENCE_MODULES %du\nstatic const apollo_overlay_module p66_recurrence_disk_modules[%d]={\n'%(len(manifest),len(manifest))+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16'])for m in manifest)+'};\n';(build/'p66_recurrence_modules.h').write_text(header)
sources=['src/agc_word.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c','src/p66_basic.c','src/p66_descent_invoke.c','src/p66_descent_runtime.c','src/p66_descent_prefix_control.c','src/guidance_control_observer.c','src/native_overlay_dispatch.c','src/p66_display_job.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','src/p66_application.c','src/p66_application_recurrence.c','src/lm_clock.c','src/p66_length_return_control.c','src/p66_direct_launch.c','src/dsky_display.c','platform/c64/p66_recurrence_runtime_diagnostic.c']
subprocess.run(common+['-DP66_NATIVE_DISPATCH_ONLY','-DP66_GUIDANCE_DISPATCH_ONLY','-DP66_BASIC_DISPATCH_ONLY','-DP66_DISPLAY_JOB_DISPATCH_ONLY','-DP66_APPLICATION_DISPATCH_ONLY','-DP66_RECURRENCE_DISPATCH_ONLY','-DP66_DIRECT_LAUNCH_DISPATCH_ONLY','-DEXEC_NATIVE_RESIDENT_SERVICES','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/p66-recurrence-live.map']+sources+['-o','build/p66-recurrence-live.prg'],cwd=root,check=True)
disk=build/'p66-recurrence-live.d81';disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','APOLLO DESCENT,11','d81',str(disk),'-write',str(build/'p66-recurrence-live.prg'),'resident']
for m in manifest:
 assert hashlib.sha256((build/m['prg']).read_bytes()).hexdigest()==m['sha256'];cmd+=['-write',str(build/m['prg']),m['filename'].lower()]
subprocess.run(cmd,cwd=root,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')))
print('Built actual resident/descent/control/scheduler stack and2 frozen proof modules:',len(manifest),'totaldiskmodules')
