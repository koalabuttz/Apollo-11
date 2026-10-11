#!/usr/bin/env python3
"""CM context/lifecycle/interpretive boundaries against source-derived assembly."""
import hashlib,json,pathlib,re,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]; F=ROOT/'deps/comanche-fixture'; E=ROOT/'deps/virtualagc/yaAGC'
assert (F/'EXECUTIVE.original.agc').read_bytes()==(ROOT.parent/'Comanche055/EXECUTIVE.agc').read_bytes()
listing=(F/'assembly.lst').read_text()
for name,addr in [('CHANG1','5164'),('CHANG2','5170'),('INTPRET','6006'),('INTRSM','6017'),('DANZIG','6030'),('NEWOPS','6037'),('EXIT','6712'),('JOBSLEEP','5175'),('JOBWAKE','5201'),('PRIOCHNG','5210'),('ENDJOB1','01,3124'),('NUDIRECT','01,3242')]:
 assert re.search(r'(?<!\w)'+name+r'\s+'+addr+r'\b',listing),name
# These exact original CM Interpreter boundaries use pinned supporting source.
# Compare labels/opcodes/operands directly before running any oracle.
original=(ROOT.parent/'Comanche055/INTERPRETER.agc').read_text()
pinned=(ROOT/'deps/virtualagc/Comanche055/INTERPRETER.agc').read_text()
def instructions(source,start,end):
 text=source[source.index('\n'+start):source.index('\n'+end,source.index('\n'+start))]
 return [line.split('#')[0].split() for line in text.splitlines() if line.split('#')[0].strip()]
assert instructions(original,'INTPRET','DLOAD')==instructions(pinned,'INTPRET','DLOAD')
assert instructions(original,'DANZIG','NEWOPS')==instructions(pinned,'DANZIG','NEWOPS')
for source in [original,pinned]:
 rows=[line.split('#')[0].split() for line in source[source.index('\nEXIT'):].splitlines() if line.split('#')[0].strip()][:4]
 assert rows==[['EXIT','CA','BANKSET'],['TS','BBANK'],['INDEX','LOC'],['TC','1']]
(ROOT/'build/cm-dispatch-interpreter-source-audit.json').write_text(json.dumps({
 'original_sha256':hashlib.sha256(original.encode()).hexdigest(),
 'pinned_sha256':hashlib.sha256(pinned.encode()).hexdigest(),
 'checked_boundaries':['INTPRET/INTRSM','DANZIG/NOIBNKSW','EXIT four instructions'],
 'comparison':'All opcode/operand/label tokens agree; comments and pseudo-label indentation excluded.'},indent=2)+'\n')
print('PASS: original/pinned CM Interpreter boundary instruction tokens agree')
for stage,count,fields,defines,inputs in [
 ('dispatch',109,317,['DISPATCH_INPUT_ONLY'],['tests/cm_executive_cases.c','tests/cm_dispatch_cases.c']),
 ('lifecycle',115,317,['DISPATCH_INPUT_ONLY','LIFECYCLE_INPUT_ONLY'],['tests/cm_executive_cases.c','tests/cm_dispatch_cases.c','tests/cm_lifecycle_cases.c']),
 ('interpreter',512,319,['CM_INTERPRETER_ROW_ONLY'],['tests/cm_executive_cases.c','tests/cm_interpreter_cases.c','platform/host/cm_interpreter_snapshot.c']),
 ('mixed',140,320,['IN_INPUT_ONLY','IN_ROW_ONLY'],['tests/cm_executive_cases.c','tests/cm_mixed_cases.c','platform/host/cm_mixed_snapshot.c'])]:
 subprocess.run(['cc','-std=c99','-O2',*['-D'+x for x in defines],'-I'+str(E),'-Iinclude','-Itests',f'tools/cm_{stage}_reference.c',*inputs,str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o',f'build/cm-{stage}-reference'],cwd=ROOT,check=True)
 rows=[]
 for name,cmd in [('native',[f'./build/cm-{stage}-snapshot']),('reference',[f'./build/cm-{stage}-reference',str(F/'MAIN.agc.bin')])]:
  r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,check=True)
  (ROOT/f'build/cm-{stage}-{name}.snapshots').write_text(r.stdout)
  (ROOT/f'build/cm-{stage}-{name}.trace').write_text(r.stderr)
  rows.append([list(map(int,line.split())) for line in r.stdout.splitlines()])
 assert all(len(x)==count for x in rows),(stage,'count')
 for n,(a,b) in enumerate(zip(*rows)):
  assert len(a)==len(b)==fields,(stage,n,'fields')
  assert a==b,(stage,n,[(i,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y])
 print(f'PASS: CM {stage}: {count} complete-state checkpoints; physical transfers and 16 guard words checked')
 print('Snapshot SHA256:',hashlib.sha256((ROOT/f'build/cm-{stage}-reference.snapshots').read_bytes()).hexdigest())
print('PASS: 876 CM context checkpoints; 8 basic, 5 lifecycle, and 7 mixed native callback sequences')
