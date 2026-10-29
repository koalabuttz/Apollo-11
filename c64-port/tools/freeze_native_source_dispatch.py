#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Select32 complete original priority/sleep fixture rows for target residency."""
import pathlib,re,hashlib
root=pathlib.Path(__file__).resolve().parents[1]
for family in('priority','sleep'):
 source=root/f'tests/lm_{family}_vectors.h';text=source.read_text();name=f'lm_{family}'
 def array(field):
  match=re.search(name+'_'+field+r'\[\][^=]*=\{(.*?)\};',text,re.S)
  return list(map(int,re.findall(r'\d+',match[1])))
 numbers,offsets,patches,results=map(array,('numbers','offsets','patch','results'))
 patches=list(zip(patches[::2],patches[1::2]));selected=list(range(16))+list(range(48,64));outpatch=[];outoffset=[0]
 for i in selected:outpatch+=patches[offsets[i]:offsets[i+1]];outoffset.append(len(outpatch))
 guard=f'NATIVE_SOURCE_DISPATCH_{family.upper()}_VECTORS_H'
 lines=['/* SPDX-License-Identifier: MIT',f' * Original full fixture header SHA256 {hashlib.sha256(source.read_bytes()).hexdigest()} */',f'#ifndef {guard}',f'#define {guard}',f'#include "lm_{family}_cases.h"']
 for field,values in(('numbers',[numbers[i]for i in selected]),('offsets',outoffset),('results',[results[i]for i in selected])):lines.append('static const uint16_t '+name+'_'+field+'[]={'+','.join(map(str,values))+'};')
 lines+=['static const uint16_t '+name+'_patch[][2]={']+['{%d,%d},'%r for r in outpatch]+['};','#endif','']
 (root/f'tests/native_source_dispatch_{family}_vectors.h').write_text('\n'.join(lines))
 print(family,len(selected),'full-state rows',len(outpatch),'source delta pairs')
