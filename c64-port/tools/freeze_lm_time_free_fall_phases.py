#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze native TIME_OF_FREE_FALL phase steps for the C64 phase diagnostics.

Each native phase frontier image (tools/lm_time_free_fall_phases.c) of the sampled
states must equal, in order, an original NEWOPS image that the pinned yaAGC
oracle reaches (tools/lm_time_free_fall_reference.c trace mode). Each frozen step is
then: n, phase, IRQ/SUPER and the entry image as a delta from the case's
orbit-free debris image (lm_time_free_fall_debris), followed by the next phase, IRQ/SUPER and the exit image as a delta from
the step's entry image. One header per diagnostic group, split into parts (one PRG each) that fit
under the C64 mailbox page.

The rows are chosen so the C64 runs every branch outcome the host stream
runs: tools/lm_time_free_fall_cover.c records, with clang edge guards, the
edges of src/lm_time_free_fall.c each native step takes over all sampled
chains, and a greedy set cover picks steps until every host edge is taken
(plus PER_PHASE evenly spaced steps of each phase from the fixed states).
tools/lm_time_free_fall_c64_cover.py checks the result with gcov."""
import hashlib,json,os,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from frozen_words import encode_rows,c_header
from lm_time_free_fall_core import CORE
IDS=[0,2,3,4,19,34,35,83,179,523,768,771,775,779,40007,65535]
PER_PHASE=2
# Phase numbers (include/lm_time_free_fall.h) in each C64 diagnostic group.
GROUPS={1:[0,1,2,3,4,5],2:[6,7,8,9,10,11,12,13,14],3:[15,16,17,18,19,20,21],4:[22,23,24]}
# Frozen-vector bytes one PRG of each group can hold below $c000 (its code
# and the other rodata take the rest); a group with more is split into parts.
BUDGET={1:5800,2:11500,3:18500,4:17800}
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_time_free_fall_phases.c',*CORE,'-o','build/lm-time-free-fall-phases'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_time_free_fall_inputs.c','tests/lm_time_free_fall_cases.c','tests/lm_time_free_fall_debris.c','-o','build/lm-time-free-fall-inputs'],cwd=R,check=True)
# Host clang (not llvm-mos's): HOST_CLANG overrides.
HOST_CLANG=os.environ.get('HOST_CLANG','/usr/bin/clang' if pathlib.Path('/usr/bin/clang').exists() else 'clang')
def cover_steps():
 """Greedy cover of the host edges: [(n, frontier, phase)]."""
 (R/'build/cover').mkdir(parents=True,exist_ok=True)
 subprocess.run([HOST_CLANG,'-std=c99','-O0','-Iinclude','-Itests','-Itools','-fsanitize-coverage=edge,trace-pc-guard','-c','src/lm_time_free_fall.c','-o','build/cover/lm-time-free-fall.o'],cwd=R,check=True)
 subprocess.run([HOST_CLANG,'-std=c99','-O2','-Iinclude','-Itests','-Itools','build/cover/lm-time-free-fall.o','tools/lm_time_free_fall_cover.c','tools/edge_cover.c',*[c for c in CORE if c!='src/lm_time_free_fall.c'],'-o','build/cover/lm-time-free-fall-cover'],cwd=R,check=True)
 seen=[]
 for l in subprocess.check_output(['build/cover/lm-time-free-fall-cover'],cwd=R,text=True).splitlines():
  tag,edges=l.split('|');seen.append((tuple(map(int,tag.split())),set(map(int,edges.split()))))
 need=set().union(*(e for _,e in seen));pick=[]
 while need:
  tag,edges=max(seen,key=lambda r:(len(r[1]&need),-r[0][0],-r[0][1]))
  pick.append(tag);need-=edges
 return sorted(pick)
COVER=cover_steps()
BASE=set(IDS)
IDS=sorted(BASE|{n for n,_,_ in COVER})
if not (R/'build/lm-time-free-fall-reference').exists():subprocess.run([sys.executable,'tools/lm_time_free_fall_differential.py'],cwd=R,check=True)
def rows(blob,width):return[struct.unpack_from(f'<{width}H',blob,i)for i in range(0,len(blob),width*2)]
steps={};source=hashlib.sha256();checked=0
for n in IDS:
 native=rows(subprocess.check_output(['build/lm-time-free-fall-phases',str(n)],cwd=R),2053)
 oracle=rows(subprocess.check_output(['build/lm-time-free-fall-reference','deps/lm-rtb-fixture/MAIN.agc.bin','1',str(n),'trace'],cwd=R),2053)
 entry=rows(subprocess.check_output(['build/lm-time-free-fall-inputs','--debris',str(n)],cwd=R),2050)[0]
 j=0
 for r in native:
  key=(r[2],r[3],r[5:])
  while j<len(oracle)and(oracle[j][2],oracle[j][3],oracle[j][5:])!=key:j+=1
  if j==len(oracle):raise SystemExit(f'FAIL state {n}: native frontier before phase {r[4]} is not an original NEWOPS image in order')
  source.update(bytes(struct.pack('<2053H',*oracle[j])));checked+=1;j+=1
 for i,(a,b) in enumerate(zip(native,native[1:])):
  if a[4]<25:steps.setdefault(a[4],[]).append((n,i,entry,a,b))
summary={};cover=set(COVER);headers={}
for group,phases in GROUPS.items():
 chosen=[]
 for ph in phases:
  pool=[x for x in steps[ph] if x[0] in BASE]
  pick=[pool[int(i*len(pool)/PER_PHASE)]for i in range(min(PER_PHASE,len(pool)))]
  pick+=[x for x in steps[ph] if (x[0],x[1],ph) in cover and x not in pick]
  if not pick:raise SystemExit(f'FAIL phase {ph} has no frozen step')
  chosen+=pick
 missing={c for c in cover if c[2] in phases}-{(n,i,a[4]) for n,i,_,a,_ in chosen}
 if missing:raise SystemExit(f'FAIL cover steps not frozen: {sorted(missing)}')
 table=[]
 for n,i,entry,a,b in chosen:
  ed=[(j,v)for j,(u,v)in enumerate(zip(entry[:2048],a[5:]))if u!=v];xd=[(j,v)for j,(u,v)in enumerate(zip(a[5:],b[5:]))if u!=v]
  table.append((n,a,b,ed,xd))
 def pack(part):
  emax=max(len(x[3]) for x in part);xmax=max(len(x[4]) for x in part);out=[]
  for n,a,b,ed,xd in part:
   row=[n,a[4],a[2],a[3],len(ed),*sum(([j,v]for j,v in ed),[])];row+=[0]*(5+2*emax-len(row))
   row+=[b[4],b[2],b[3],len(xd),*sum(([j,v]for j,v in xd),[])];row+=[0]*(5+2*emax+4+2*xmax-len(row));out.append(row)
  return encode_rows(out),emax,xmax
 # Greedy split in row order: each part's vectors stay within the group budget.
 parts=[];cur=[]
 for row in table:
  if cur and pack(cur+[row])[0].storage_bytes>BUDGET[group]:parts.append(cur);cur=[]
  cur.append(row)
 parts.append(cur)
 up=f'LM_TIME_FREE_FALL_G{group}'
 text=(f'/* SPDX-License-Identifier: MIT; native phase steps whose frontiers equal original NEWOPS images (tools/freeze_lm_time_free_fall_phases.py). */\n'
  f'#ifndef {up}_VECTORS_H\n#define {up}_VECTORS_H\n#define {up}_PARTS {len(parts)}u\n')
 runs=[];summary[group]=dict(phases=phases,parts=[])
 for k,part in enumerate(parts,1):
  packed,emax,xmax=pack(part);prefix=f'lm_time_free_fall_g{group}p{k}';pu=prefix.upper()
  text+=(f'#if !defined(LM_TIME_FREE_FALL_PART)||LM_TIME_FREE_FALL_PART==0||LM_TIME_FREE_FALL_PART=={k}\n'
   f'#ifdef {up}_FROZEN_IMPLEMENTATION\n#define {pu}_FROZEN_IMPLEMENTATION\n#endif\n'
   f'#define {pu}_ENTRY_CHANGES {emax}u\n#define {pu}_EXIT_CHANGES {xmax}u\n'+c_header(packed,prefix)+
   f'#define {pu}_RUN(st,ex,completed,s) if(!(s))(s)=lm_time_free_fall_step_rows({prefix}_reference_begin,{pu}_FROZEN_ROWS,{pu}_ENTRY_CHANGES,{pu}_EXIT_CHANGES,st,ex,completed)\n'
   f'#else\n#define {pu}_RUN(st,ex,completed,s) ((void)0)\n#endif\n')
  runs.append(f'{pu}_RUN(st,ex,completed,s);')
  summary[group]['parts'].append(dict(part=k,steps=len(part),states=sorted({x[0] for x in part}),entry_changes=emax,exit_changes=xmax,frozen_bytes=packed.storage_bytes))
 # {up}_CHECK runs this group's linked parts; s is the running status.
 text+=f'#define {up}_CHECK(st,ex,completed,s) do{{'+''.join(runs)+'}while(0)\n'
 (R/f'tests/lm_time_free_fall_g{group}_vectors.h').write_text(text+'#endif\n')
prgs=[f'{g}-{p["part"]}' for g in summary for p in summary[g]['parts']]
result=dict(states=IDS,cover_steps=[list(c) for c in COVER],frontiers_matched=checked,matched_oracle_rows_sha256=source.hexdigest(),prgs=prgs,groups=summary)
(R/'evidence/lm-time-free-fall/lm-time-free-fall-phase-steps.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS',checked,'native phase frontiers equal original NEWOPS images; PRGs',' '.join(prgs))
