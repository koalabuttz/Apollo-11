#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build independently captured LM/CM native passive-data disk proofs."""
import argparse,hashlib,json,os,pathlib,subprocess,shutil
from overlay_build import crc16
from overlay_runtime_library import prepare
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--mission',choices=['lm','cm'],required=True);p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args()
 sdk=B/f'passive-storage-{a.mission}-sdk';sdk.mkdir(exist_ok=True);library=prepare(a.cc,sdk)
 common=[a.cc,'-Iinclude','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror'];manifest=[]
 for group in range(6):
  number=200+group;stem=f'passive-storage-{a.mission}-{number}';output=B/(stem+'.prg')
  sources=[f'platform/c64/{a.mission}_fixed_compat_data.c',f'src/{a.mission}_fixed_compat.c']
  if a.mission=='cm':
   # Reuse the source-reader owner's exact production payload bytes.
   original=B/f'cm-fixed-compat-{number}.prg';shutil.copyfile(original,output);shutil.copyfile(B/f'cm-fixed-compat-{number}.map',B/(stem+'.map'))
   recorded=next(m for m in json.loads((B/'cm-fixed-compat-modules.json').read_text()) if m['id']==number)
   assert hashlib.sha256(output.read_bytes()).hexdigest()==recorded['sha256']
  else:
   subprocess.run(common+[f'-D{a.mission.upper()}_FIXED_COMPAT_GROUP={group}','-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={number},-Map,build/{stem}.map']+sources+['platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(output)],cwd=R,check=True)
  raw=output.read_bytes();data=raw[2:];assert raw[:2]==b'\0\x80' and len(data)<=16384 and data[:4]==b'NATV' and data[5]==number and int.from_bytes(data[14:16],'little')==len(data)
  manifest.append(dict(id=number,filename=f'F{number}',byte_count=len(data),crc16=crc16(data),sha256=hashlib.sha256(raw).hexdigest(),prg=output.name,first_word=group*7168,present_words=min(7168,36864-group*7168),passive_data_only=True))
  print(a.mission,number,len(data),'loaded bytes')
 (B/'passive_storage_modules.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module passive_storage_modules[6]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n')
 stem=f'passive-storage-{a.mission}';resident=B/(stem+'.prg')
 subprocess.run(common+['-T','platform/c64/overlay_resident.ld',f'-Wl,-Map,build/{stem}.map','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/passive_storage_diagnostic.c','-o',str(resident)],cwd=R,check=True)
 disk=B/(stem+'.d64');disk.unlink(missing_ok=True);command=[a.c1541,'-format',f'APOLLO {a.mission.upper()},11','d64',str(disk),'-write',str(resident),'resident']
 for m in manifest:command+=['-write',str(B/m['prg']),m['filename'].lower()]
 for name in ('passive-storage-cache','passive-storage-config'):(B/name).mkdir(exist_ok=True)
 subprocess.run(command,cwd=R,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(B/'passive-storage-cache'),XDG_CONFIG_HOME=str(B/'passive-storage-config')))
 metadata={'mission':a.mission,'original_data_bytes':73728,'loaded_data_payload_bytes':sum(m['byte_count'] for m in manifest),'overlay_overhead_bytes':sum(m['byte_count'] for m in manifest)-73728,'resident_prg_bytes':resident.stat().st_size,'modules':manifest,'separate_mission_disk':str(disk.relative_to(R)),'production_registry_modified':False,'original_instruction_execution':False,'disk_sha256':hashlib.sha256(disk.read_bytes()).hexdigest()}
 (B/(stem+'-allocation.json')).write_text(json.dumps(metadata,indent=2)+'\n')
if __name__=='__main__':main()
