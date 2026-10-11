#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless full-state fixtures selected solely from the original RCS monitor."""
import pathlib,struct,hashlib,subprocess,gzip,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
local=[0,1,2,3,4,5,6,7,15,16,17,31,32,33,255,256,257,511,512,513,1023,1024,2047,2048,4095,4096,8191,16383,32767,32768,65534,65535]
# Independently chosen bit operations across varying low-bit states and Executive pools.
for bit in range(8):
 for v in range(8):
  previous=(v*31+bit*17)&255
  current=previous^(1<<bit)
  local.append((previous<<8)|current)
indices=[base+n for base in (0,65536)for n in local]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/lm_rcs_monitor_inputs.c','tests/lm_rcs_monitor_cases.c','src/agc_word.c','-o','build/lm-rcs-monitor-inputs'],cwd=r,check=True)
data=subprocess.check_output(['build/lm-rcs-monitor-inputs'],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
initial={n:struct.unpack_from('<2051H',data,k*4102)for k,n in enumerate(indices)}
source={};h=hashlib.sha256();count=0;outcomes={};bits={};pools={};ebs={};supers={};directions={}
with gzip.open(r/'build/lm-rcs-monitor-reference.snapshots.gz','rb')as f:
 while raw:=f.read(4106):
  assert len(raw)==4106;h.update(raw);w=struct.unpack('<2053H',raw);n=w[0]+(w[1]<<16);irq,sup,ch=w[2:5];after=w[5:];count+=1
  outcome='bailout1202'if after[5]==0o5716 else'resume'
  outcomes[outcome]=outcomes.get(outcome,0)+1
  previous=(n>>8)&255;current=n&255;changed=previous^current
  bit=changed.bit_length()-1 if changed else-1
  bits[str(bit)]=bits.get(str(bit),0)+1
  direction='unchanged'if not changed else('opening'if previous&(1<<bit)else'closing')
  directions[direction]=directions.get(direction,0)+1
  pool=(n>>12)%9;pools[str(pool)]=pools.get(str(pool),0)+1
  ebs[str(after[3])]=ebs.get(str(after[3]),0)+1;supers[str(sup)]=supers.get(str(sup),0)+1
  if n in initial:source[n]=(after,irq,sup,ch)
rows=[]
for n in indices:
 after,irq,sup,ch=source[n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+3]]
 rows.append([n&65535,n>>16,irq,sup,ch,len(changes)]+[v for pair in changes for v in pair])
width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
s='/* SPDX-License-Identifier: MIT\n * Complete original corpus SHA256:'+h.hexdigest()+' */\n#ifndef LM_RCS_MONITOR_VECTORS_H\n#define LM_RCS_MONITOR_VECTORS_H\n'+c_header(packed,'lm_rcs_monitor')+'#endif\n'
(r/'tests/lm_rcs_monitor_vectors.h').write_text(s)
x={'complete_states':count,'decoded_sha256':h.hexdigest(),'handoff_counts':outcomes,'highest_changed_bit_counts':bits,'direction_counts':directions,'pool_context_counts':pools,'EB_counts':ebs,'SUPER_counts':supers,'frozen_states':len(indices),'frozen_storage_bytes':packed.storage_bytes}
(r/'build/lm-rcs-monitor-corpus-stats.json').write_text(json.dumps(x,indent=2)+'\n');print(x)
