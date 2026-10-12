#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze native TIMETHET phase steps for the C64 phase diagnostics.

Each native phase frontier image (tools/lm_conic_timethet_phases.c) of the sampled
states must equal, in order, an original NEWOPS image that the pinned yaAGC
oracle reaches (tools/lm_conic_timethet_reference.c trace mode). Each frozen step is
then: n, phase, IRQ/SUPER and the entry image as a delta from the case's
orbit-free debris image (lm_conic_timethet_debris), followed by the next phase, IRQ/SUPER and the exit image as a delta from
the step's entry image. One header per diagnostic group."""
import hashlib,json,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from frozen_words import encode_rows,c_header
from lm_conic_timethet_core import CORE
IDS=[0,99,112,264,2360,3312,13002,65535]
# Phase numbers (include/lm_conic_timethet.h) in each C64 diagnostic group.
GROUPS={1:[0,1,2,3,4,6],2:[7,8,9,10,11,26],3:[12,13,14,15,16,17,18],4:[19,20,21,22,23,24,25]}
PER_PHASE=4
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_conic_timethet_phases.c',*CORE,'-lm','-o','build/lm-conic-timethet-phases'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_conic_timethet_inputs.c','tests/lm_conic_timethet_cases.c','tests/lm_conic_timethet_debris.c','-lm','-o','build/lm-conic-timethet-inputs'],cwd=R,check=True)
if not (R/'build/lm-conic-timethet-reference').exists():subprocess.run([sys.executable,'tools/lm_conic_timethet_differential.py'],cwd=R,check=True)
def rows(blob,width):return[struct.unpack_from(f'<{width}H',blob,i)for i in range(0,len(blob),width*2)]
steps={};source=hashlib.sha256();checked=0
for n in IDS:
 native=rows(subprocess.check_output(['build/lm-conic-timethet-phases',str(n)],cwd=R),2053)
 oracle=rows(subprocess.check_output(['build/lm-conic-timethet-reference','deps/lm-rtb-fixture/MAIN.agc.bin','1',str(n),'trace'],cwd=R),2053)
 entry=rows(subprocess.check_output(['build/lm-conic-timethet-inputs','--debris',str(n)],cwd=R),2050)[0]
 j=0
 for r in native:
  key=(r[2],r[3],r[5:])
  while j<len(oracle)and(oracle[j][2],oracle[j][3],oracle[j][5:])!=key:j+=1
  if j==len(oracle):raise SystemExit(f'FAIL state {n}: native frontier before phase {r[4]} is not an original NEWOPS image in order')
  source.update(bytes(struct.pack('<2053H',*oracle[j])));checked+=1;j+=1
 for a,b in zip(native,native[1:]):
  steps.setdefault(a[4],[]).append((n,entry,a,b))
summary={}
for group,phases in GROUPS.items():
 chosen=[]
 for ph in phases:
  pool=steps[ph];pick=[pool[int(i*len(pool)/PER_PHASE)]for i in range(min(PER_PHASE,len(pool)))]
  chosen+=pick
 table=[];emax=xmax=0
 for n,entry,a,b in chosen:
  ed=[(i,v)for i,(u,v)in enumerate(zip(entry[:2048],a[5:]))if u!=v];xd=[(i,v)for i,(u,v)in enumerate(zip(a[5:],b[5:]))if u!=v]
  emax=max(emax,len(ed));xmax=max(xmax,len(xd));table.append((n,a,b,ed,xd))
 out=[]
 for n,a,b,ed,xd in table:
  row=[n,a[4],a[2],a[3],len(ed),*sum(([i,v]for i,v in ed),[])];row+=[0]*(5+2*emax-len(row))
  row+=[b[4],b[2],b[3],len(xd),*sum(([i,v]for i,v in xd),[])];row+=[0]*(5+2*emax+4+2*xmax-len(row));out.append(row)
 packed=encode_rows(out);prefix=f'lm_conic_timethet_g{group}';up=prefix.upper()
 (R/f'tests/lm_conic_timethet_g{group}_vectors.h').write_text(f'/* SPDX-License-Identifier: MIT; native phase steps whose frontiers equal original NEWOPS images (tools/freeze_lm_conic_timethet_phases.py). */\n#ifndef {up}_VECTORS_H\n#define {up}_VECTORS_H\n#define {up}_ENTRY_CHANGES {emax}u\n#define {up}_EXIT_CHANGES {xmax}u\n'+c_header(packed,prefix)+'#endif\n')
 summary[group]=dict(phases=phases,steps=len(out),entry_changes=emax,exit_changes=xmax,frozen_bytes=packed.storage_bytes)
result=dict(states=IDS,frontiers_matched=checked,matched_oracle_rows_sha256=source.hexdigest(),groups=summary)
(R/'evidence/lm-conic-timethet/lm-conic-timethet-phase-steps.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',checked,'native phase frontiers equal original NEWOPS images;',summary)
