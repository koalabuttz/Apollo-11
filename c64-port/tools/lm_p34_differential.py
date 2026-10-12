#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run unchanged Luminary099 in the pinned yaAGC oracle from every
lm-p34-transfer case (tests/lm_p34_cases.c) to its first stop, check the
native trampoline run (this file's routines with the native INTER-BANK
routines) against every row, pin the stream identity, and freeze per-case
digests: tests/lm_p34_vectors.h (all cases) and tests/lm_p34_c64_vectors.h
(every 16th). Writes build/lm-p34-transfer-proof.json with the stops reached per
routine and archives the oracle stream as
build/lm-p34-transfer-reference.snapshots.bin.xz. --record rewrites the
pinned identity."""
import argparse,lzma,collections,hashlib,json,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R/'tools'))
from lm_p34_core import CORE,CASES
ROW=4106;STRIDE=16
BASIC,PER=20,256;BASIC_CASES=BASIC*PER;CLU=1024;VN=4096;MG=4096;STATES=BASIC_CASES+CLU+VN+MG
PIN=R/'evidence/lm-p34-transfer/lm-p34-transfer-source-stream.json'
ENTRIES=[0o3635,0o3636,0o3643,0o3646,0o3647,0o3650,0o3651,0o3654,0o3662,0o3664,0o3670,0o3674,0o3701,0o3702,0o3703,0o3704,0o3712,0o3720,0o3721,0o3722]
NAMES=['GET45 +1','COMPTGO return','DELAYJOB return','GOFLASH terminate (KILCLOCK)','GOFLASH proceed (N45PROC)','GOFLASH recycle (CLUPDATE)','KILCLOCK','N45PROC','PHASCHNG return (UPFLAG)','CLUPDATE','PHASCHNG return (INTPRET)','DISPLAYE','DISPLAYE terminate','DISPLAYE proceed (TC NORMEX)','DISPLAYE recycle','P3XORP7X','VNPOOH','VNPOOH terminate','VNPOOH proceed (TC RTRN)','VNPOOH recycle']
STOPS={(0o2432,0o72000):'COMPTGO',(0o3735,0):'DELAYJOB',(0o2476,0o20000):'GOFLASH',0o5353:'PHASCHNG',0o5504:'UPFLAG',0o6001:'GOTOPOOH',0o6037:'INTPRET',0o5726:'POODOO1 (1302)',(0o3723,0):'ARCSIN 1301'}
def stop_name(z,fb,loc):
 if z==0o6070:return f'NEWOPS {fb>>10:02o},{loc:04o}'
 return STOPS.get((z,fb))or STOPS.get(z)or 'caller return'
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
if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
subprocess.run(['gcc','-std=c99','-O2','-DLM_P34_NO_TRAMPOLINE','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/lm_p34_reference.c',*CASES,'tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-lm','-o','build/lm-p34-reference'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_p34_snapshot.c',*CORE,'-lm','-o','build/lm-p34-native'],cwd=R,check=True)
ref=subprocess.Popen(['build/lm-p34-reference','deps/lm-rtb-fixture/MAIN.agc.bin',str(STATES)],cwd=R,stdout=subprocess.PIPE)
nat=subprocess.Popen(['build/lm-p34-native',str(STATES)],cwd=R,stdout=subprocess.PIPE)
out_path=R/'build/lm-p34-transfer-reference.snapshots.bin.xz';tmp=out_path.with_suffix('.xz.part');archive=lzma.open(tmp,'wb',preset=6)
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
 w=X[5:];key=NAMES[n%BASIC]+f' ({ENTRIES[n%BASIC]:04o})' if n<BASIC_CASES else 'CLUPDATE CLEAR GOTO (NEWOPS 3671)' if n<BASIC_CASES+CLU else 'VN1645 (NEWOPS 3606)' if n<BASIC_CASES+CLU+VN else 'MIDGIM/SELECTMU ('+('GET+MGA','GET+MGA','GET+MGA','GET.LVC','GET.LVC','GET.LVC','SELECTMU','SELECTMU')[(n-BASIC_CASES-CLU-VN)%8]+')'
 name='refused' if X[4]==0 else stop_name(w[5],w[4]&0o76000,w[0o164])
 if key.startswith('CLUPDATE') and name.startswith('NEWOPS') and not name.startswith('NEWOPS 35,'):name='NEWOPS of QSAVED outside the file'
 stops[key][name]+=1
if ref.stdout.read(1)or nat.stdout.read(1)or ref.wait()or nat.wait():raise SystemExit('FAIL producer exit or trailing bytes')
archive.close();tmp.replace(out_path)
identity=dict(archive='build/lm-p34-transfer-reference.snapshots.bin.xz',decoded_sha256=h.hexdigest(),decoded_bytes=size,cases=STATES,tag_counts={str(k):v for k,v in sorted(tags.items())},oracle='pinned yaAGC hardware-DV variant, deps/lm-rtb-fixture/MAIN.agc.bin (unchanged Luminary099)')
if a.record:PIN.parent.mkdir(parents=True,exist_ok=True);PIN.write_text(json.dumps(identity,indent=2)+'\n')
pin=json.loads(PIN.read_text())
if (pin['decoded_sha256'],pin['decoded_bytes'])!=(identity['decoded_sha256'],size):raise SystemExit('FAIL oracle stream differs from the pinned identity')
head=['/* SPDX-License-Identifier: MIT; generated by tools/lm_p34_differential.py from the original',' * Luminary099 program (yaAGC oracle). Per-case digests of the tag, all 2048 erasable words',' * and the interrupt-enable state. */']
(R/'tests/lm_p34_vectors.h').write_text('\n'.join([*head,f'#define LM_P34_VECTORS {len(digests)}u',*table('lm_p34_expected',digests)])+'\n')
# C64 samples (each PRG links only the routines its cases reach): every
# 16th basic case (group 1); one in 64 of the CLUPDATE, VN1645 and SELECTMU
# cases whose run does not reach VN1645's CALL GET+MGA (group 2) and one in
# 128 of the GET.LVC cases (group 3). Offsets step so every generator kind
# (n%8) is sampled. Groups 2-3 carry their entries' orbit words frozen over
# the generator's integer debris (the C64 has no floating point). GET+MGA
# with its interpretive bodies leaves no C64 RAM below the mailbox for the
# trampoline, its cases and GET45, so it runs on the host only here (its
# phase bodies run on the C64 in lm-midgim).
MG0=BASIC_CASES+CLU+VN
def kind(n):return (n-MG0)%8 if n>=MG0 else None
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_p34_inputs.c',*CORE,'-lm','-o','build/lm-p34-inputs'],cwd=R,check=True)
def frozen(cases):
 out={}
 for i in range(0,len(cases),512):
  for line in subprocess.run(['build/lm-p34-inputs',*map(str,cases[i:i+512])],cwd=R,check=True,capture_output=True,text=True).stdout.splitlines():
   f=line.split();out[int(f[0])]=(int(f[1]),[(int(f[j],8),int(f[j+1],8))for j in range(2,len(f),2)])
 return out
pick=lambda n,m:n%m==(n//m)%8
two=[n for n in range(BASIC_CASES,STATES) if (kind(n) is None or kind(n) in (6,7)) and pick(n,64)]
info=frozen(two+[n for n in range(MG0,STATES) if kind(n) in (3,4,5) and pick(n,128)])
groups={2:[n for n in two if not info[n][0]],3:[n for n in range(MG0,STATES) if kind(n) in (3,4,5) and pick(n,128)]}
basic=digests[:BASIC_CASES:STRIDE]
vec=['/* SPDX-License-Identifier: MIT; generated by tools/lm_p34_differential.py: the',' * lm-p34-transfer case digests the C64 diagnostics check: every 16th basic case (group 1)',' * and the sampled interpretive-entry cases of tests/lm_p34_c64_inputs.h (groups 2-3). */',f'#define LM_P34_C64_STRIDE {STRIDE}u',f'#define LM_P34_C64_BASIC {len(basic)}u','#if LM_P34_C64_GROUP==1',*table('lm_p34_c64_basic',basic)]
inp=['/* SPDX-License-Identifier: MIT; generated by tools/lm_p34_differential.py: the sampled',' * interpretive-entry lm-p34-transfer cases of C64 groups 2-3, each with the words its',' * lm-vn1645 or lm-midgim generator sets over that generator\'s debris. */','typedef struct {uint16_t address,word;} lm_p34_word;']
for g,sampled in groups.items():
 first=[0];delta=[]
 for n in sampled:delta+=info[n][1];first.append(len(delta))
 vec+=[f'#elif LM_P34_C64_GROUP=={g}',*table('lm_p34_c64_interp',[digests[n] for n in sampled])]
 inp+=[f'#{"if" if g==2 else "elif"} LM_P34_C64_GROUP=={g}',f'#define LM_P34_C64_INPUTS {len(sampled)}u',
  f'static const uint32_t lm_p34_c64_input_case[{len(sampled)}]={{'+','.join(f'{n}ul'for n in sampled)+'};',
  f'static const uint16_t lm_p34_c64_input_first[{len(first)}]={{'+','.join(map(str,first))+'};',
  f'static const lm_p34_word lm_p34_c64_delta[{len(delta)}]={{']
 for i in range(0,len(delta),8):inp.append(''.join(f'{{0{a:o},0{w:o}}},'for a,w in delta[i:i+8]))
 inp.append('};')
(R/'tests/lm_p34_c64_vectors.h').write_text('\n'.join([*vec,'#endif'])+'\n')
(R/'tests/lm_p34_c64_inputs.h').write_text('\n'.join([*inp,'#endif'])+'\n')
c64_groups={str(g):len(v) for g,v in groups.items()}
proof=dict(cases=STATES,c64_groups=dict(basic=len(basic),**c64_groups),stream_sha256=h.hexdigest(),stops={k:dict(sorted(v.items()))for k,v in stops.items()},boundary='each case runs from its entry through the trampoline (this file\'s routines and the native INTER-BANK routines) to the first point outside them; the oracle stops at the same external entries, alarm frontiers, NEWOPS outside the file\'s interpretive code and the case\'s own basic returns',compared='2048 erasable words (A, L 16-bit), the interrupt-enable state and SUPERBNK')
(R/'build/lm-p34-transfer-proof.json').write_text(json.dumps(proof,indent=1)+'\n')
print(f'PASS {STATES} original P34-35_P74-75 transfer cases equal native trampoline runs',h.hexdigest(),flush=True)
