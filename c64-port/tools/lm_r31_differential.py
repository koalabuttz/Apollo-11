#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run unchanged Luminary099 in the pinned yaAGC oracle from every lm-r31
case (tests/lm_r31_cases.c) to its first stop, check the native trampoline
run of the R31 routines against every row, pin the stream identity, and
freeze per-case digests: tests/lm_r31_vectors.h (all cases, and the C64 sample). Archives the oracle stream as
build/lm-r31-reference.snapshots.bin.xz and writes build/lm-r31-proof.json
with the stops per routine. --record rewrites the pinned identity."""
import argparse,collections,hashlib,json,lzma,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R/'tools'))
from lm_r31_core import CORE,CASES
ROW=4106;ROUTINES=42;STATES=ROUTINES*512
PIN=R/'evidence/lm-r31/lm-r31-source-stream.json'
NAMES=['after V83CALL INTPRET (3132)', 'after LOADTIME (3134)', 'after LEMPREC (3136)', 'DOCMBASE (3144)', 'after CSMPREC (3147)', 'after REV83 INTPRET (3166)', 'after LOADTIME (3170)', 'after INTSTALL (3172)', 'OTHCONIC (3211)', 'after INTSTALL (3217)', 'OTHINT (3222)', 'COMPDISP (3240)', 'RANGE (3246)', 'after CDU*NBSM (3263)', 'after ARCCOS (3304)', 'GETRVN (3331)', 'after PRIOCHNG (3347)', 'VXM VSL1 (3355)', 'GETRVN2 (3363)', 'after INTSTALL (3365)', 'R31SURF (3371)', 'after LOADTIME (3373)', 'after LEMPREC (3375)', 'ONEBASE (3404)', 'DISPN5X (3113)', 'B5OFF (3116)', 'B5OFF (3117)', 'recycle (3120)', 'V83CALL (3121)', 'REV83 (3155)', 'after EXIT (3316)', 'GETRVN (3326)', 'after PRIOCHNG (3330)', 'after EXIT (3344)', 'after PRIOCHNG (3346)', 'R31SURF (3370)', 'MUNG? (3377)', 'ONEBASE (3403)', 'R31CALL (40,3674)', 'DSPDELAY (40,3700)', 'after FIXDELAY (40,3702)', 'after NOVAC (40,3712)']
def digest(vals):
 a=b=0
 for v in vals:a=(a+v)&0xffff;b=(b+a)&0xffff
 return b<<16|a
def table(name,values):
 out=[f'static const uint32_t {name}[{len(values)}]={{']
 for i in range(0,len(values),6):out.append(''.join(f'0x{v:08x}ul,'for v in values[i:i+6]))
 return out+['};']
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--record',action='store_true');a=p.parse_args()
(R/'build').mkdir(exist_ok=True)
# CI runs a spec's test- and sanitize- targets at once; both rebuild these build/ outputs, so one waits.
import fcntl;LOCK=open(R/'build'/(pathlib.Path(__file__).stem+'.lock'),'w');fcntl.flock(LOCK,fcntl.LOCK_EX)
if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
subprocess.run(['gcc','-std=c99','-O2','-DLM_R31_NO_TRAMPOLINE','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/lm_r31_reference.c',*CASES,'tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-lm','-o','build/lm-r31-reference'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_r31_snapshot.c',*CORE,'-lm','-o','build/lm-r31-native'],cwd=R,check=True)
ref=subprocess.Popen(['build/lm-r31-reference','deps/lm-rtb-fixture/MAIN.agc.bin',str(STATES)],cwd=R,stdout=subprocess.PIPE)
nat=subprocess.Popen(['build/lm-r31-native',str(STATES)],cwd=R,stdout=subprocess.PIPE)
out_path=R/'build/lm-r31-reference.snapshots.bin.xz';tmp=out_path.with_suffix('.xz.part');archive=lzma.open(tmp,'wb',preset=6)
h=hashlib.sha256();size=0;tags=collections.Counter();digests=[];stops=collections.defaultdict(collections.Counter)
for n in range(STATES):
 x=ref.stdout.read(ROW);y=nat.stdout.read(ROW)
 if len(x)!=ROW or x!=y:
  ref.kill();nat.kill()
  if len(x)==ROW==len(y):
   X=struct.unpack('<2053H',x);Y=struct.unpack('<2053H',y)
   print('FAIL case',n,[(f'{i-5:04o}',f'{v:06o}',f'{Y[i]:06o}')for i,v in enumerate(X)if v!=Y[i]][:25])
  else:print('FAIL case',n,'short row')
  raise SystemExit(1)
 X=struct.unpack('<2053H',x);h.update(x);archive.write(x);size+=len(x);tags[X[4]]+=1
 digests.append(digest([X[4],*X[5:],X[2]]))
 w=X[5:];z=w[5];fb=w[4]&0o76000
 if z==0o6070:name=f'NEWOPS {fb>>10:02o},{w[0o164]:04o}'
 else:name={0o3573:'LOADTIME',0o3612:'ARCCOS',0o3402:'UNIT 1302 exit',0o5105:'FINDVAC',0o5221:'FIXDELAY',0o5072:'NOVAC',0o5261:'TASKOVER',0o4616:'BANKCALL',0o5563:'B5OFF',0o5146:'PRIOCHNG',0o5472:'ENDEXT',0o6037:'INTPRET',0o5726:'POODOO1 (1302)'}.get(z,f'Z {z:04o}')
 stops[NAMES[n%ROUTINES]][name]+=1
if ref.stdout.read(1)or nat.stdout.read(1)or ref.wait()or nat.wait():raise SystemExit('FAIL producer exit or trailing bytes')
archive.close();tmp.replace(out_path)
identity=dict(archive='build/lm-r31-reference.snapshots.bin.xz',decoded_sha256=h.hexdigest(),decoded_bytes=size,cases=STATES,tag_counts={str(k):v for k,v in sorted(tags.items())},oracle='pinned yaAGC hardware-DV variant, deps/lm-rtb-fixture/MAIN.agc.bin (unchanged Luminary099)')
if a.record:PIN.parent.mkdir(parents=True,exist_ok=True);PIN.write_text(json.dumps(identity,indent=2)+'\n')
pin=json.loads(PIN.read_text())
if (pin['decoded_sha256'],pin['decoded_bytes'])!=(identity['decoded_sha256'],size):raise SystemExit('FAIL oracle stream differs from the pinned identity')
head=['/* SPDX-License-Identifier: MIT; generated by tools/lm_r31_differential.py: digests of the',' * original Luminary099 run of every lm-r31 case (see tests/lm_r31_cases.h). */']
# C64 samples: each group's cases spread over the 512 variants, cycling
# through the group's entries (case k: variant k*512/count, entry k%len).
# One group holds every entry.
GROUPS={1:(11,),2:(12,),3:(13,),4:(16,18),5:(17,),6:tuple(r for r in range(ROUTINES)if r not in(11,12,13,16,17))};COUNT={1:128,2:128,3:128,4:128,5:128,6:518}
c64={g:[k*512//COUNT[g]*ROUTINES+r[k%len(r)]for k in range(COUNT[g])]for g,r in GROUPS.items()}
lines=[*head,f'#define LM_R31_VECTORS {len(digests)}u','#ifndef LM_R31_C64_GROUP',*table('lm_r31_expected',digests)]
for g,r in GROUPS.items():lines+=[f'#elif LM_R31_C64_GROUP=={g}',f'#define LM_R31_C64_CASES {len(c64[g])}u','static const uint8_t lm_r31_c64_routine[%d]={%s};'%(len(r),','.join(map(str,r))),*table('lm_r31_expected',[digests[n]for n in c64[g]])]
(R/'tests/lm_r31_vectors.h').write_text('\n'.join([*lines,'#endif'])+'\n')
proof=dict(cases=STATES,c64_groups={str(g):len(v)for g,v in c64.items()},stream_sha256=h.hexdigest(),stops={k:dict(sorted(v.items()))for k,v in stops.items()},boundary='each case runs from its entry (NEWOPS or Z) through the trampoline (this file\'s routines) to the first point outside them; the oracle stops at the same point (tests/lm_r31_cases.c lm_r31_stop: a NEWOPS at the split points 37,3240, 3246 and 3355 or outside 37,3132-3154, 3166-3315, 3331-3343, 3347-3367, 3371-3376 and 3404-3406, LOADTIME, ARCCOS, FINDVAC, FIXDELAY, NOVAC, TASKOVER, BANKCALL, B5OFF, PRIOCHNG, ENDEXT, INTPRET, POODOO1, UNIT 1302 exit)',compared='2048 erasable words (A, L 16-bit), the interrupt-enable state and SUPERBNK')
(R/'build/lm-r31-proof.json').write_text(json.dumps(proof,indent=1)+'\n')
print(f'PASS {STATES} original R31 entries equal native trampoline runs',h.hexdigest(),flush=True)
