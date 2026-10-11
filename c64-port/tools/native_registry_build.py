#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Add a checked native descriptor overlay; preserve recurrence111 artifacts."""
import argparse,hashlib,json,os,pathlib,subprocess
from overlay_build import crc16
from overlay_runtime_library import prepare
ROOT=pathlib.Path(__file__).resolve().parents[1]
RESIDENT_SOURCES=['src/agc_word.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c','src/p66_basic.c','src/p66_descent_invoke.c','src/p66_descent_runtime.c','src/p66_descent_prefix_control.c','src/guidance_control_observer.c','src/native_overlay_dispatch.c','src/p66_display_job.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','src/p66_application.c','src/p66_application_recurrence.c','src/lm_clock.c','src/p66_length_return_control.c','src/p66_direct_launch.c','src/dsky_display.c','src/native_module_registry.c','platform/c64/native_registry_recurrence_diagnostic.c']
RESIDENT_DEFINES=['P66_NATIVE_DISPATCH_ONLY','P66_GUIDANCE_DISPATCH_ONLY','P66_BASIC_DISPATCH_ONLY','P66_DISPLAY_JOB_DISPATCH_ONLY','P66_APPLICATION_DISPATCH_ONLY','P66_RECURRENCE_DISPATCH_ONLY','P66_DIRECT_LAUNCH_DISPATCH_ONLY','EXEC_NATIVE_RESIDENT_SERVICES']
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');p.add_argument('--fixed-lease',action='store_true');a=p.parse_args();build=ROOT/'build'
 manifest=json.loads((ROOT/'evidence/p66-recurrence-live-modules.json').read_text())
 assert len(manifest)==111 and len({m['id']for m in manifest})==111
 assert all(m['id']!=148 and 0<len(m['filename'])<17 for m in manifest)
 retained={m['id']:ROOT/f'evidence/p66-recurrence-live-module-{m["id"]}.prg' for m in manifest}
 for m in manifest:assert hashlib.sha256(retained[m['id']].read_bytes()).hexdigest()==m['sha256'],m['id']
 table='#include "overlay_runtime.h"\n#define NATIVE_REGISTRY_MODULES 111u\nstatic const apollo_overlay_module native_registry_modules[111]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16'])for m in manifest)+'};\n'
 (build/'native_registry_table.h').write_text(table)
 common=[a.cc,'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror']
 if a.fixed_lease:common+=['-DNATIVE_REGISTRY_FIXED_LEASE']
 library=prepare(a.cc,build)
 subprocess.run(common+['-nostdlib','-T','platform/c64/overlay_payload.ld','-Wl,--defsym=__overlay_module=148,-Map,build/native-registry-148.map','platform/c64/native_registry_payload.c','src/native_module_registry.c','platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o','build/native-registry-148.prg'],cwd=ROOT,check=True)
 payload=(build/'native-registry-148.prg').read_bytes();assert payload[:2]==b'\x00\x80' and len(payload)-2<=16384
 boot=dict(id=148,filename='MINFO',byte_count=len(payload)-2,crc16=crc16(payload[2:]),sha256=hashlib.sha256(payload).hexdigest(),prg='native-registry-148.prg',mission_translation=False)
 digest=bytearray()
 for m in sorted(manifest,key=lambda m:m['id']):digest+=bytes([m['id']])+m['byte_count'].to_bytes(2,'little')+m['crc16'].to_bytes(2,'little')+m['filename'].encode()+b'\0'
 (build/'native_registry_bootstrap.h').write_text('#include "overlay_runtime.h"\n#define NATIVE_REGISTRY_EXPECTED_COUNT 111u\n#define NATIVE_REGISTRY_EXPECTED_CRC %du\nstatic const apollo_overlay_module native_registry_bootstrap={148,"MINFO",%d,%d};\n'%(crc16(digest),boot['byte_count'],boot['crc16']))
 sources=list(RESIDENT_SOURCES)
 defines=list(RESIDENT_DEFINES)
 subprocess.run(common+['-D'+d for d in defines]+['-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/native-registry-recurrence.map']+sources+['-o','build/native-registry-recurrence.prg'],cwd=ROOT,check=True)
 subprocess.run(common+['-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/native-registry-directory.map','src/overlay_runtime.c','src/native_module_registry.c','platform/c64/overlay_platform.c','platform/c64/native_registry_directory_diagnostic.c','-o','build/native-registry-directory.prg'],cwd=ROOT,check=True)
 disk=build/'native-registry-recurrence.d81';disk.unlink(missing_ok=True)
 command=[a.c1541,'-format','APOLLO REGISTRY,11','d81',str(disk),'-write',str(build/'native-registry-recurrence.prg'),'resident']
 for m in manifest+[boot]:command+=['-write',str(retained.get(m['id'],build/m['prg'])),m['filename'].lower()]
 subprocess.run(command,cwd=ROOT,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')))
 (build/'native-registry-modules.json').write_text(json.dumps(manifest+[boot],indent=2)+'\n')
 (build/'native-registry-build.json').write_text(json.dumps({'original_registry_count':111,'bootstrap':boot,'descriptor_crc16':crc16(digest),'lease_address':0xc600,'lease_bytes':64,'sources':sources,'defines':defines},indent=2)+'\n')
 print('Built registry148 and resident derivative with111 byte-identical frozen stages; no recurrence artifacts changed')
if __name__=='__main__':main()
