#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Real compiled CM task/service overlays and one canonical-image resident."""
import argparse,pathlib,json,subprocess,hashlib,os
from overlay_build import crc16
from overlay_runtime_library import prepare
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args()
(B/'cm-mission-sdk').mkdir(exist_ok=True);library=prepare(a.cc,B/'cm-mission-sdk')
common=[a.cc,'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror']
base=['src/cm_native_image.c','src/agc_word.c'];parts={
 1:['src/cm_task_transfer.c','src/cm_waitlist.c'],
 2:['src/cm_entry_tasks.c','src/cm_clocktask.c','src/cm_tig5.c','src/cm_svct3.c','src/cm_phase.c','src/agc_multiword.c'],
 3:['src/cm_waitlist_image.c','src/cm_delay_image.c','src/cm_waitlist.c'],
 4:['src/cm_admission_image.c','src/cm_executive.c'],
 5:['src/cm_task_interrupt.c','src/cm_danzig.c','src/cm_basic_finish.c','src/cm_dispatch.c','src/cm_executive.c'],
 250:['platform/c64/cm_mission_overlay_input.c','tests/cm_mission_loop_cases.c','tests/cm_service_loop_cases.c','src/cm_executive.c','src/cm_waitlist.c'],
 251:['platform/c64/cm_mission_overlay_check.c']}
manifest=[]
for number,sources in parts.items():
 output=B/f'cm-mission-overlay-{number}.prg';extra=[]
 if number<250:sources=['platform/c64/cm_mission_overlay_payload.c']+sources;extra=[f'-DCM_MISSION_MODULE={number}']
 subprocess.run(common+extra+['-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={number},-Map,build/cm-mission-overlay-{number}.map']+sources+(base if number!=251 else [])+['platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(output)],cwd=R,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=number,filename=f'C{number}',byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=number<250));print(number,len(data)-2,'loaded bytes')
(B/'cm-mission-overlay-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
(B/'cm_mission_overlay_modules.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module cm_mission_overlay_modules[7]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n')
subprocess.run(common+['-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/cm-mission-overlay.map','src/cm_mission_loop.c','src/cm_service_loop.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/cm_mission_overlay_diagnostic.c','-o','build/cm-mission-overlay.prg'],cwd=R,check=True)
disk=B/'cm-mission-overlay.d64';disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','APOLLO CM,11','d64',str(disk),'-write',str(B/'cm-mission-overlay.prg'),'resident']
for m in manifest:cmd+=['-write',str(B/m['prg']),m['filename'].lower()]
subprocess.run(cmd,cwd=R,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(B/'cm-mission-overlay-cache'),XDG_CONFIG_HOME=str(B/'cm-mission-overlay-config')))
print('Built sole-image CM resident and five actual native control/service/body modules plus proof overlays')
