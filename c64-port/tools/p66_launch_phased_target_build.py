#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Actual startup service overlays compared with whole original source seams."""
import argparse,gzip,hashlib,json,os,subprocess
from pathlib import Path
from overlay_build import crc16
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args();build=ROOT/'build';stem='p66-launch-phased'
factories=['tests/p66_bias_cases.c','tests/p66_vertical_start_cases.c','tests/p66_launch_cases.c']
subprocess.run(['gcc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','src/agc_word.c']+factories+['tools/p66_launch_phased_inputs.c','-o','build/'+stem+'-inputs'],cwd=ROOT,check=True)
inputs=[list(map(int,line.split())) for line in subprocess.check_output(['build/'+stem+'-inputs'],cwd=ROOT).splitlines()];wanted={row[0] for row in inputs};outputs={n:[] for n in wanted}
with (build/(stem+'-reference.snapshots')).open() as f:
    for line in f:
        n=int(line.split(' ',1)[0])
        if n in wanted:outputs[n].append(list(map(int,line.split())))
assert all(1<=len(rows)<=3 and all(len(row)==2053 for row in rows) for rows in outputs.values())
flags=[a.cc,'-Iinclude','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror'];manifest=[]
for group in range(1,17):
    rows=inputs[(group-1)*2:group*2];payload=build/f'{stem}-fixture-{group}.c';prg=build/f'{stem}-fixture-{group}.prg'
    payload.write_text('#include "lm_interpreter_change_fixture.h"\nstatic const uint16_t input[2][2052]={\n'+''.join('{'+','.join(map(str,row[1:]))+'},\n' for row in rows)+'};\nuint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t pointer){lm_interpreter_change_fixture *f=(lm_interpreter_change_fixture *)(uintptr_t)pointer;uint16_t i;if(!f||f->row>1)return 1;for(i=0;i<2048;++i)image->words[i]=input[f->row][i];f->io.superbank=input[f->row][2048];f->io.interrupt_enabled=input[f->row][2049];f->channel11=input[f->row][2050];f->channel14=input[f->row][2051];return 0;}\n')
    subprocess.run(flags+['-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={group},-Map,build/{stem}-fixture-{group}.map',str(payload),'platform/c64/overlay_entry.S','-lcrt','-o',str(prg)],cwd=ROOT,check=True)
    data=prg.read_bytes();assert len(data)-2<=16384;manifest.append(dict(id=group,filename=f'L{group}',byte_count=len(data)-2,crc16=crc16(data[2:]),prg=prg.name,sha256=hashlib.sha256(data).hexdigest()))
services=json.loads((build/'overlay-p66-application-modules.json').read_text());manifest.extend(m for m in services if m['id'] in (50,83,52));assert len(manifest)==19
def fnv(words):
    h=2166136261
    for w in words:
        for b in (w&255,w>>8):h=((h^b)*16777619)&0xffffffff
    return h
header='#include "overlay_runtime.h"\nstatic const apollo_overlay_module p66_launch_modules[19]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\n'
header+='static const uint8_t p66_launch_count[32]={'+','.join(str(len(outputs[row[0]])) for row in inputs)+'};\n'
header+='static const uint16_t p66_launch_status[32][3]={\n'+''.join('{'+','.join(str(out[2]) for out in outputs[row[0]])+'},\n' for row in inputs)+'};\n'
header+='static const uint32_t p66_launch_hash[32][3]={\n'+''.join('{'+','.join(str(fnv(out[3:]))+'UL' for out in outputs[row[0]])+'},\n' for row in inputs)+'};\n'
(build/'p66_launch_phased_target.h').write_text(header)
subprocess.run(flags+['-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/'+stem+'-target.map','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/p66_launch_phased_diagnostic.c','-o','build/'+stem+'-target.prg'],cwd=ROOT,check=True)
disk=build/(stem+'-target.d81');disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','P66 START SOURCE,11','d81',str(disk),'-write',str(build/(stem+'-target.prg')),'resident']
for m in manifest:cmd+=['-write',str(build/m['prg']),m['filename'].lower()]
subprocess.run(cmd,cwd=ROOT,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')))
frozen=''.join(' '.join(map(str,row))+'\n'+''.join(' '.join(map(str,out))+'\n' for out in outputs[row[0]]) for row in inputs).encode();(build/(stem+'-target-full.snapshots.gz')).write_bytes(gzip.compress(frozen,mtime=0));(build/(stem+'-target-modules.json')).write_text(json.dumps(manifest,indent=2)+'\n')
print('Built32 frozen cases,',sum(len(rows) for rows in outputs.values()),'full2050-word source checkpoints through actual startup disk calls; SHA256',hashlib.sha256(frozen).hexdigest())
