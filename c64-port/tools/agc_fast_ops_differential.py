#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Instruction differential for src/agc_fast_ops.c. The records of
tools/agc_image_ops_differential.py, with most operands moved to plain cells
(the cells the fast helpers take), run through tools/agc_fast_ops_native.c
(afo_* for plain cells under the record's EB, ai_* otherwise) and compared,
whole 2048-word image, with the pinned yaAGC engine and with agc_image_ops."""
import argparse,random,struct,subprocess,sys,pathlib
R=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R/'tools'))
import agc_image_ops_differential as D
CC=['-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude']
LIB=['src/agc_image_ops.c','src/agc_hardware_divide.c','src/agc_multiword.c','src/agc_word.c']
def plain_k(r,op,eb):
 """A plain-cell operand under EB (code words 0700-0702 and their EB-1
 alias stay out, as in the base generator)."""
 off=D.ENC[op][3]
 while True:
  c=r.random()
  if c<.1:k=r.randrange(0o10,0o20-off)
  elif c<.55:k=r.randrange(0o24,0o1400-off)
  else:k=r.randrange(0o1400,0o2000-off)
  ea=k if k<0o1400 else eb*0o400+k-0o1400
  if eb==0 and k>=0o1400 and ea<0o24+off:continue
  if 0o676<=ea<=0o703:continue
  return k
def case(r,op):
 c=D.case(r,op)
 if op in('BZF','BZMF') or r.random()<.25:return c
 ext,base,bits,off,regs=D.ENC[op];eb=(c[6+3]>>8)&7
 k=plain_k(r,op,eb);code=([6] if ext else [])+[(base+((k+off)&((1<<bits)-1)))&0o77777]
 return [c[0],k,len(code)]+code+[0]*(3-len(code))+c[6:]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cases',type=int,default=4000);p.add_argument('--seed',type=int,default=1201);a=p.parse_args()
 (R/'build').mkdir(exist_ok=True)
 if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/agc_image_ops_reference.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/agc-image-ops-reference'],cwd=R,check=True)
 subprocess.run(['cc',*CC,'tools/agc_image_ops_native.c',*LIB,'-o','build/agc-image-ops-native'],cwd=R,check=True)
 subprocess.run(['cc',*CC,'tools/agc_fast_ops_native.c','src/agc_fast_ops.c',*LIB,'-o','build/agc-fast-ops-native'],cwd=R,check=True)
 r=random.Random(a.seed);cases=[case(r,op) for op in D.OPS for _ in range(a.cases)]
 blob=b''.join(struct.pack('<2054H',*c) for c in cases)
 ref=subprocess.run(['build/agc-image-ops-reference','deps/lm-rtb-fixture/MAIN.agc.bin'],cwd=R,input=blob,capture_output=True,check=True).stdout
 gen=subprocess.run(['build/agc-image-ops-native'],cwd=R,input=blob,capture_output=True,check=True).stdout
 run=subprocess.run(['build/agc-fast-ops-native'],cwd=R,input=blob,capture_output=True,check=True)
 fast=run.stdout;sys.stderr.write(run.stderr.decode())
 bad={}
 for i,c in enumerate(cases):
  x=struct.unpack_from('<2048H',ref,i*4096)
  for name,out in (('fast',fast),('generic',gen)):
   y=struct.unpack_from('<2048H',out,i*4096)
   if x!=y:
    op=D.OPS[c[0]];key=name+' '+op;bad[key]=bad.get(key,0)+1
    if bad[key]<=3:print('FAIL',key,'K=%o'%c[1],'A=%06o L=%06o Q=%06o'%(c[6],c[7],c[8]),[('%04o'%j,'%06o'%x[j],'%06o'%y[j]) for j in range(2048) if x[j]!=y[j]][:8])
 if bad:print('FAIL counts',bad);raise SystemExit(1)
 print('PASS',len(cases),'single-instruction images: agc_fast_ops and agc_image_ops both equal yaAGC across',len(D.OPS),'instructions')
if __name__=='__main__':main()
