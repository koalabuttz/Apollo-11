#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,json
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','tests/dsky_monitor_consumer_cases.c','tools/dsky_monitor_consumer_inputs.c','-o','build/dsky-monitor-consumer-inputs'],cwd=R,check=True)
base={int(r.split(' ',1)[0]):list(map(int,r.split()))[1:]for r in subprocess.check_output(['build/dsky-monitor-consumer-inputs'],cwd=R,text=True).splitlines()}
previous=dict(base);chosen={};counts={}
with lzma.open(R/'build/dsky-monitor-consumer-reference.snapshots.xz','rt')as f:
 for line in f:
  a=list(map(int,line.split()));n,step,stage=a[:3];expected=a[3:];incoming=previous[n];previous[n]=expected[:-1];key=(stage,n>=128)
  if counts.get(key,0)>=8:continue
  counts[key]=counts.get(key,0)+1;chosen.setdefault(stage,[]).append((n,step,incoming,expected))
manifest={}
for stage,rows in sorted(chosen.items()):
 ip=[];op=[];ioff=[0];ooff=[0];numbers=[];steps=[]
 for n,step,incoming,expected in rows:
  numbers.append(n);steps.append(step);ip.extend((i,v)for i,(v,b)in enumerate(zip(incoming,base[n]))if v!=b);ioff.append(len(ip));op.extend((i,v)for i,(v,b)in enumerate(zip(expected,incoming+[99]))if v!=b);ooff.append(len(op))
 stem='dsky_monitor_consumer_'+str(stage);lines=['/* SPDX-License-Identifier: MIT */','#define DSKY_MONC_COUNT_'+str(stage)+' '+str(len(rows))+'u']
 for name,values in [('numbers',numbers),('steps',steps),('ioff',ioff),('ooff',ooff)]:lines.append('static const uint16_t '+stem+'_'+name+'[]={'+','.join(map(str,values))+'};')
 for name,values in [('in',ip),('out',op)]:lines.append('static const uint16_t '+stem+'_'+name+'[][2]={');lines.extend(' {'+str(i)+','+str(v)+'},'for i,v in (values or [(0,0)]));lines.append('};')
 (R/'tests'/('dsky_monitor_consumer_vectors_'+str(stage)+'.h')).write_text('\n'.join(lines)+'\n');manifest[str(stage)]=dict(count=len(rows),input_patches=len(ip),output_patches=len(op),source_numbers=numbers,source_steps=steps)
(R/'build/dsky-monitor-consumer-frozen.json').write_text(json.dumps(manifest,indent=2)+'\n');print('frozen',sum(len(x)for x in chosen.values()),'complete incoming/output source states across',len(chosen),'stages')
