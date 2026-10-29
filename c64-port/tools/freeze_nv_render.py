#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
common=['src/agc_word.c','src/agc_multiword.c','src/p66_nv_entry.c','src/p66_nv_words.c','src/p66_nv_rows.c','src/dsky_two_digit.c','src/dsky_pinball.c','tests/throttle_bridge_cases.c','tests/nv_entry_cases.c','tests/nv_words_cases.c','tests/nv_rows_cases.c','tests/nv_render_cases.c']
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests']+common+['tools/nv_render_full_inputs.c','-o','build/nv-render-full-inputs'],cwd=root,check=True)
initial=[list(map(int,r.split()))[3:]for r in subprocess.check_output(['build/nv-render-full-inputs'],cwd=root,text=True).splitlines()]
data=(root/'build/nv-render-full-reference.snapshots').read_bytes();assert data==(root/'build/nv-render-full-native.snapshots').read_bytes();rows=[list(map(int,r.split()))for r in data.decode().splitlines()];assert len(rows)==12288
s='/* SPDX-License-Identifier: MIT\n * Literal original full2050-word phase entry/exit deltas; SHA256 '+hashlib.sha256(data).hexdigest()+' */\n#ifndef NV_RENDER_VECTORS_H\n#define NV_RENDER_VECTORS_H\n'
for stage in range(4):
 pre=[];after=[];po=[0];ao=[0];numbers=[];phases=[]
 for j in range(16):
  n=j*63;phase=(j%3)*4+stage;numbers.append(n);phases.append(phase);before=initial[n]if phase==0 else rows[n*12+phase-1][3:];exit=rows[n*12+phase][3:];assert rows[n*12+phase][2]==0
  pre.extend((f,v)for f,(b,v)in enumerate(zip(initial[n],before))if b!=v);po.append(len(pre));after.extend((f,v)for f,(b,v)in enumerate(zip(before,exit))if b!=v);ao.append(len(after))
 s+=f'#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED=={stage}\n'
 for label,values in [('numbers',numbers),('phases',phases),('pre_offsets',po),('after_offsets',ao)]:s+=f'static const uint16_t nv_render{stage}_{label}[]={{'+','.join(map(str,values))+'};\n'
 for label,values in [('pre_patch',pre),('after_patch',after)]:s+=f'static const uint16_t nv_render{stage}_{label}[][2]={{\n'+''.join(' {'+str(f)+','+str(v)+'},\n'for f,v in values)+'};\n'
 s+='#endif\n';print('Stage',stage,'16 entry/exit images; packedbytes',4*(len(pre)+len(after))+4*16+4*17)
s+='#endif\n';(root/'tests/nv_render_vectors.h').write_text(s)
