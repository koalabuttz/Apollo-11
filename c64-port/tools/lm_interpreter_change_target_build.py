#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze original full-state inputs/outputs and load32 actual fixture modules."""
import argparse,gzip,hashlib,json,os,subprocess
from pathlib import Path
from overlay_build import crc16
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args()
build=ROOT/'build';stem='lm-interpreter-change'
subprocess.run(['gcc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','src/agc_word.c','tests/lm_interpreter_change_cases.c','tools/lm_interpreter_change_inputs.c','-o','build/'+stem+'-inputs'],cwd=ROOT,check=True)
raw=subprocess.check_output(['build/'+stem+'-inputs'],cwd=ROOT);inputs=[list(map(int,line.split())) for line in raw.splitlines()];wanted={row[0] for row in inputs};outputs={}
with (build/(stem+'-reference.snapshots')).open() as f:
    for line in f:
        n=int(line.split(' ',1)[0])
        if n in wanted: outputs[n]=list(map(int,line.split()))[2:]
assert len(outputs)==64 and all(len(row)==2052 for row in outputs.values())
flags=[a.cc,'-Iinclude','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror']
manifest=[]
for group in range(1,33):
    rows=inputs[(group-1)*2:group*2];payload=build/f'{stem}-fixture-{group}.c';prg=build/f'{stem}-fixture-{group}.prg'
    payload.write_text('#include "lm_interpreter_change_fixture.h"\nstatic const uint16_t input[2][2052]={\n'+''.join('{'+','.join(map(str,row[1:]))+'},\n' for row in rows)+'};\nuint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t pointer){lm_interpreter_change_fixture *f=(lm_interpreter_change_fixture *)(uintptr_t)pointer;uint16_t i;if(!f||f->row>1)return 1;for(i=0;i<2048;++i)image->words[i]=input[f->row][i];f->io.superbank=input[f->row][2048];f->io.interrupt_enabled=input[f->row][2049];f->channel11=input[f->row][2050];f->channel14=input[f->row][2051];return 0;}\n')
    subprocess.run(flags+['-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={group},-Map,build/{stem}-fixture-{group}.map',str(payload),'platform/c64/overlay_entry.S','-lcrt','-o',str(prg)],cwd=ROOT,check=True)
    data=prg.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
    manifest.append(dict(id=group,filename=f'X{group}',byte_count=len(data)-2,crc16=crc16(data[2:]),prg=prg.name,sha256=hashlib.sha256(data).hexdigest()))
def fnv(words):
    h=2166136261
    for w in words:
        for b in (w&255,w>>8):h=((h^b)*16777619)&0xffffffff
    return h
header='#include "overlay_runtime.h"\nstatic const apollo_overlay_module lm_change_modules[32]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\nstatic const uint32_t lm_change_hash[64]={'+','.join(str(fnv(outputs[row[0]]))+'UL' for row in inputs)+'};\nstatic const uint16_t lm_change_next[64][3]={\n'
for row in inputs:
    w=row[1:];off=w[0o67];loc=w[0o164+off];interp=bool(loc&0o40000);header+='{%d,%d,%d},\n'%(((loc^0o77777)+1 if interp else loc),w[0o165+off],interp)
(build/'lm_interpreter_change_target.h').write_text(header+'};\n')
subprocess.run(flags+['-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/'+stem+'-target.map','src/overlay_runtime.c','platform/c64/overlay_platform.c','src/agc_word.c','src/lm_interpreter_change.c','platform/c64/lm_interpreter_change_diagnostic.c','-o','build/'+stem+'-target.prg'],cwd=ROOT,check=True)
disk=build/(stem+'-target.d81');disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','CHANG2 SOURCE,11','d81',str(disk),'-write',str(build/(stem+'-target.prg')),'resident']
for m in manifest:cmd+=['-write',str(build/m['prg']),m['filename'].lower()]
subprocess.run(cmd,cwd=ROOT,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')))
frozen=''.join(' '.join(map(str,row))+'\n'+' '.join(map(str,outputs[row[0]]))+'\n' for row in inputs).encode()
(build/(stem+'-target-full.snapshots.gz')).write_bytes(gzip.compress(frozen,mtime=0));(build/(stem+'-target-modules.json')).write_text(json.dumps(manifest,indent=2)+'\n')
print('Built64 complete original input/output cases,32 fixture modules; source state never normalized; SHA256',hashlib.sha256(frozen).hexdigest())
