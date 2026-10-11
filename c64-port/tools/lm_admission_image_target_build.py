#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,hashlib,json,os,subprocess
from pathlib import Path
from overlay_build import crc16
from overlay_runtime_library import prepare
root=Path(__file__).resolve().parents[1];build=root/'build';p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args();library=prepare(a.cc,build)
subprocess.run(['gcc','-DP66_NATIVE_DISPATCH_ONLY','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','src/executive.c','src/p66_native.c','tests/lm_admission_image_cases.c','tools/lm_admission_image_inputs.c','-o','build/lm-admission-image-inputs'],cwd=root,check=True)
initial=subprocess.check_output(['build/lm-admission-image-inputs'],cwd=root);(build/'lm-admission-image-inputs.snapshots').write_bytes(initial);inputs=[list(map(int,l.split()))[1:]for l in initial.splitlines()]
import lzma
wanted={(n*257+29)%16384 for n in range(64)}
with lzma.open(build/'lm-admission-image-reference.snapshots.xz','rt')as archive:rows={int(l.split(' ',1)[0]):l for l in archive if int(l.split(' ',1)[0])in wanted}
expected=[list(map(int,rows[(n*257+29)%16384].split()))[1:]for n in range(64)]
def hash(words):
 h=2166136261
 for w in words:
  for byte in(w&255,w>>8):h=((h^byte)*16777619)&0xffffffff
 return h
common=[a.cc,'-Iinclude','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror'];manifest=[]
for pair in range(32):
 records=expected[2*pair:2*pair+2];source=build/f'lm-admission-image-{pair}.c';source.write_text('''/* SPDX-License-Identifier: MIT */
#include "lm_admission_image_fixture.h"
static const uint16_t initial[2][2053]={'''+','.join('{'+','.join(map(str,r))+'}'for r in inputs[2*pair:2*pair+2])+'''};
static const uint32_t expected[2]={'''+','.join('UINT32_C(%d)'%hash(r)for r in records)+'''};
static uint32_t word(uint32_t h,uint16_t w){h=(h^(uint8_t)w)*UINT32_C(16777619);return(h^(uint8_t)(w>>8))*UINT32_C(16777619);}
uint16_t apollo_overlay_main(apollo_overlay_state*m,uint16_t selector){lm_admission_fixture*f=LM_ADMISSION_FIXTURE;uint8_t n=(uint8_t)(selector&1u);uint16_t i;uint32_t h=UINT32_C(2166136261);if(selector<2){for(i=0;i<2048u;++i)m->words[i]=initial[n][i+3u];f->entry=initial[n][0];f->job.location=initial[n][1];f->job.bank=initial[n][2];f->io.superbank=initial[n][2051];f->io.interrupt_enabled=initial[n][2052];return 0;}h=word(h,f->status);for(i=0;i<2048u;++i)h=word(h,m->words[i]);h=word(h,f->io.superbank);h=word(h,f->io.interrupt_enabled);if(h!=expected[n])++f->failures;return 0;}
''')
 output=build/f'lm-admission-image-{pair}.prg';subprocess.run(common+['-nostdlib','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={160+pair},-Map,build/lm-admission-image-{pair}.map',str(source),'platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(output)],cwd=root,check=True);data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384;manifest.append(dict(id=160+pair,filename=f'R{pair}',byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name))
(build/'lm-admission-image-modules.json').write_text(json.dumps(manifest,indent=2)+'\n');(build/'lm_admission_image_modules.h').write_text('#include "overlay_runtime.h"\nstatic const apollo_overlay_module lm_admission_image_modules[32]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16'])for m in manifest)+'};\n')
subprocess.run(common+['-DP66_NATIVE_DISPATCH_ONLY','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/lm-admission-image.map','src/agc_word.c','src/executive.c','src/p66_native.c','src/lm_admission_image.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/lm_admission_image_diagnostic.c','-o','build/lm-admission-image.prg'],cwd=root,check=True)
disk=build/'lm-admission-image.d81';disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','LM ADMISSION,11','d81',str(disk),'-write',str(build/'lm-admission-image.prg'),'resident']
for m in manifest:cmd+=['-write',str(build/m['prg']),m['filename'].lower()]
subprocess.run(cmd,cwd=root,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')));print('Built64 complete NOVAC/FINDVAC/SPVAC/SPVACIN return/alarm states and32 disk fixture modules')
