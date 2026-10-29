#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Passive original-word native data payloads and actual caller-control overlays."""
import argparse,pathlib,json,subprocess,hashlib,os
from overlay_build import crc16
from overlay_runtime_library import prepare
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args();(B/'cm-fixed-compat-sdk').mkdir(exist_ok=True);library=prepare(a.cc,B/'cm-fixed-compat-sdk')
common=[a.cc,'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror'];manifest=[]
for number in list(range(200,207))+[240,241]:
 if number<206:sources=['platform/c64/cm_fixed_compat_data.c','src/cm_fixed_compat.c'];extra=[f'-DCM_FIXED_COMPAT_GROUP={number-200}']
 elif number==206:sources=['platform/c64/cm_fixed_compat_control.c','src/cm_fixed_callers.c','src/agc_word.c'];extra=[]
 elif number==240:sources=['platform/c64/cm_fixed_compat_input.c','tests/cm_fixed_compat_cases.c','src/agc_word.c'];extra=[]
 else:sources=['platform/c64/cm_fixed_compat_check.c'];extra=[]
 output=B/f'cm-fixed-compat-{number}.prg';subprocess.run(common+extra+['-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={number},-Map,build/cm-fixed-compat-{number}.map']+sources+['platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(output)],cwd=R,check=True);raw=output.read_bytes();assert raw[:2]==b'\x00\x80' and len(raw)-2<=16384;manifest.append(dict(id=number,filename=f'F{number}',byte_count=len(raw)-2,crc16=crc16(raw[2:]),sha256=hashlib.sha256(raw).hexdigest(),prg=output.name,passive_fixed_word_data=number<206,actual_native_caller_control=number==206));print(number,len(raw)-2,'loaded bytes')
(B/'cm-fixed-compat-modules.json').write_text(json.dumps(manifest,indent=2)+'\n');(B/'cm_fixed_compat_modules.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module cm_fixed_compat_modules[9]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n')
subprocess.run(common+['-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/cm-fixed-compat.map','src/cm_fixed_callers.c','src/agc_word.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/cm_fixed_compat_diagnostic.c','-o','build/cm-fixed-compat.prg'],cwd=R,check=True)
env=dict(os.environ,XDG_CACHE_HOME=str(B/'cm-fixed-compat-cache'),XDG_CONFIG_HOME=str(B/'cm-fixed-compat-config'))
def disk(name,resident,files):
 path=B/name;path.unlink(missing_ok=True);cmd=[a.c1541,'-format','APOLLO CM,11','d64',str(path),'-write',str(resident),'resident']
 for m in files:cmd+=['-write',str(B/m['prg']),m['filename'].lower()]
 subprocess.run(cmd,cwd=R,check=True,env=env)
disk('cm-fixed-compat.d64',B/'cm-fixed-compat.prg',manifest)
# Allocate the exact production data/control bytes on actual existing mission media.
# Its older resident registry remains bounded; storage allocation alone executes no caller.
mission=json.loads((B/'cm-clock-mission-overlay-modules.json').read_text());production=manifest[:7];disk('cm-fixed-compat-production.d64',B/'cm-clock-mission-overlay.prg',mission+production)
bytes_total=(B/'cm-clock-mission-overlay.prg').stat().st_size+sum(m['byte_count']+2 for m in mission+production);blocks=sum((m['byte_count']+2+253)//254 for m in mission+production)+((B/'cm-clock-mission-overlay.prg').stat().st_size+253)//254
allocation={'original_data_bytes':73728,'data_payload_loaded_bytes':sum(m['byte_count'] for m in production[:6]),'data_overlay_overhead_bytes':sum(m['byte_count'] for m in production[:6])-73728,'production_control_payload_bytes':production[6]['byte_count'],'production_catalog':mission+production,'production_raw_file_bytes':bytes_total,'estimated_directory_file_blocks':blocks,'disk_allocated':'build/cm-fixed-compat-production.d64','existing_resident_scope':'actual CLOCKJOB mission resident registry unchanged; new files allocated for future reader/control registration; allocation adds no executable algorithm coverage'};(B/'cm-fixed-compat-production-allocation.json').write_text(json.dumps(allocation,indent=2)+'\n');print(json.dumps(allocation,indent=2))
