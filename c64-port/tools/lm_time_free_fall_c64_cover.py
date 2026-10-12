#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Branch outcomes of src/lm_time_free_fall.c (gcc gcov, -O0) taken by the
whole host stream (all 65536 sampled call chains, every phase step) and by
the frozen C64 phase rows (the same rows the PRGs run, replayed on the host).
Every outcome the host stream takes must also be taken by the C64 rows.
Measured over the phase code: the lines before lm_time_free_fall_step (the
host phase dispatcher; each PRG dispatches its group through the phase
target's own switch) and the whole-routine wrapper and admission after it,
which run on the host only (whole-routine C64 runs wait for overlay ABI 2).
Writes build/lm-time-free-fall-c64-cover.json."""
import json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from lm_time_free_fall_core import CORE
REST=[c for c in CORE if c!='src/lm_time_free_fall.c']
SRC=(R/'src/lm_time_free_fall.c').read_text().splitlines()
END=next(i for i,l in enumerate(SRC,1)if l.startswith('uint8_t lm_time_free_fall_step('))
def taken(name,sources,run):
 d=R/'build/cover'/name
 if d.exists():shutil.rmtree(d)
 d.mkdir(parents=True)
 subprocess.run(['gcc','-std=c99','-O0','--coverage','-Iinclude','-Itests','-Itools','-c',str(R/'src/lm_time_free_fall.c'),'-o',str(d/'lm_time_free_fall.o')],cwd=R,check=True)
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Itools','--coverage',str(d/'lm_time_free_fall.o'),*sources,'-o',str(d/'run')],cwd=R,check=True)
 subprocess.run([str(d/'run'),*run],cwd=R,check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 subprocess.run(['gcov','-b','-c','-o',str(d),str(R/'src/lm_time_free_fall.c')],cwd=d,check=True,stdout=subprocess.DEVNULL)
 out={};total=0;line=None;never=[]
 for l in (d/'lm_time_free_fall.c.gcov').read_text().splitlines():
  m=re.match(r'\s*(?:[-#=0-9*]+):\s*(\d+):',l)
  if m:line=int(m.group(1));continue
  m=re.match(r'branch\s+(\d+)\s+(taken (\d+)|never executed)',l)
  if m:
   if line>=END:continue
   total+=1
   if m.group(3)and int(m.group(3))>0:out.setdefault(line,set()).add(int(m.group(1)))
   else:never.append(f'{line}:{m.group(1)} {SRC[line-1].strip()[:120]}')
 return out,total,never
host,total,never=taken('ltf-host',['tools/lm_time_free_fall_cover.c','tools/edge_cover.c',*REST],[])
c64,_,_=taken('ltf-c64',['tools/lm_time_free_fall_c64_replay.c','tests/lm_time_free_fall_phase_target.c','tests/lm_time_free_fall_debris.c','tests/lm_time_free_fall_cases.c','tests/frozen_words.c',*[c for c in REST if not c.startswith('tests/')]],[])
count=lambda t:sum(len(v)for v in t.values())
missing=sorted((l,b)for l,v in host.items()for b in v if b not in c64.get(l,()))
result=dict(source='src/lm_time_free_fall.c',lines=f'1-{END-1} (phase code)',measure='gcc gcov -O0 branch outcomes',outcomes=total,host_stream_taken=count(host),c64_rows_taken=count(c64),
 host_only=[f'{l}:{b}'for l,b in missing],never_taken_on_host=never,
 note='Outcomes never taken on the host are the LTF_FAILED guards on compiled literal operands, and the UNIT/ABVAL/SQRT abort exits no sampled chain reaches.')
(R/'build/lm-time-free-fall-c64-cover.json').write_text(json.dumps(result,indent=1)+'\n')
if missing:raise SystemExit(f"FAIL C64 rows miss {len(missing)} branch outcomes the host stream takes: {result['host_only']}")
print(f"PASS C64 phase rows take every branch outcome the host stream takes: {count(c64)} of {total} outcomes ({count(host)} on the host)")
