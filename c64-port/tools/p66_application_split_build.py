#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build prepare/finish adapters and a new actual recurrence disk composition."""
import argparse,hashlib,json,os,pathlib,subprocess
from overlay_build import crc16
from overlay_runtime_library import prepare
from native_registry_build import RESIDENT_SOURCES,RESIDENT_DEFINES
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args();build=ROOT/'build';generated=build/'p66-application-split';generated.mkdir(exist_ok=True)
 manifest=json.loads((ROOT/'evidence/p66-recurrence-live-modules.json').read_text())
 retained={m['id']:ROOT/f'evidence/p66-recurrence-live-module-{m["id"]}.prg' for m in manifest}
 for m in manifest:assert hashlib.sha256(retained[m['id']].read_bytes()).hexdigest()==m['sha256'],m['id']
 common=[a.cc,'-I'+str(generated),'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror'];library=prepare(a.cc,build)
 def module(id,name,sources,defs):
  out=build/f'p66-application-split-{id}.prg';subprocess.run(common+defs+['-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={id},-Map,build/p66-application-split-{id}.map']+sources+['platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(out)],cwd=ROOT,check=True)
  data=out.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
  return dict(id=id,filename=name,byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=out.name,mission_translation=False,native_adapter=True)
 for id,name in((155,'SPLPRE'),(159,'SPLEND')):
  assert all(m['id']!=id for m in manifest)
  manifest.append(module(id,name,['platform/c64/p66_application_split_payload.c','src/p66_application_split.c','src/p66_native.c','src/agc_word.c'],['-DP66_APPLICATION_SPLIT_KERNEL_ONLY',f'-DP66_APPLICATION_SPLIT_ONLY_SERVICE={id}','-DP66_NATIVE_DISPATCH_ONLY']))
 table='#include "overlay_runtime.h"\n#define NATIVE_REGISTRY_MODULES %du\nstatic const apollo_overlay_module native_registry_modules[%d]={\n'%(len(manifest),len(manifest))+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16'])for m in manifest)+'};\n'
 (generated/'native_registry_table.h').write_text(table)
 boot=module(148,'MINFO',['platform/c64/native_registry_payload.c','src/native_module_registry.c'],[])
 (generated/'native_registry_bootstrap.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module native_registry_bootstrap={148,"MINFO",%d,%d};\n'%(boot['byte_count'],boot['crc16']))
 sources=list(RESIDENT_SOURCES)
 sources=[s for s in sources if s!='src/p66_direct_launch.c'and not s.endswith('native_registry_recurrence_diagnostic.c')]+['src/p66_application_split.c','platform/c64/p66_application_split_recurrence_diagnostic.c']
 defs=list(RESIDENT_DEFINES)+['P66_APPLICATION_SPLIT_DISPATCH_ONLY']
 subprocess.run(common+['-D'+d for d in defs]+['-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/p66-application-split-recurrence.map']+sources+['-o','build/p66-application-split-recurrence.prg'],cwd=ROOT,check=True)
 disk=build/'p66-application-split-recurrence.d81';disk.unlink(missing_ok=True)
 command=[a.c1541,'-format','APOLLO SPLIT,11','d81',str(disk),'-write',str(build/'p66-application-split-recurrence.prg'),'resident']
 for m in manifest+[boot]:command+=['-write',str(retained.get(m['id'],build/m['prg'])),m['filename'].lower()]
 subprocess.run(command,cwd=ROOT,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')))
 (build/'p66-application-split-modules.json').write_text(json.dumps(manifest+[boot],indent=2)+'\n')
 (build/'p66-application-split-build.json').write_text(json.dumps({'original_stages':111,'new_adapter_ids':[155,159],'bootstrap':boot,'sources':sources,'defines':defs},indent=2)+'\n')
 print('Built actual split callback/recurrence composition; all111 frozen stage identities retained')
if __name__=='__main__':main()
