#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Instruction differential for src/agc_image_ops.c: every helper against the
pinned yaAGC hardware-DV engine executing the same original instruction in
unswitched erasable, over deterministic sampled images (edge values, all
operand classes the helpers accept). Compares the whole 2048-word image."""
import argparse,random,struct,subprocess,sys,pathlib
R=pathlib.Path(__file__).resolve().parents[1]
OPS='CA CS AD SU MASK TS XCH LXCH QXCH ADS INCR AUG DIM DAS DCA DCS DXCH MP DV CCS BZF BZMF'.split()
# (extended, base word, address bits, address offset (K+1 forms), allowed register operands)
ENC={'CA':(0,0o30000,12,0,(1,2)),'CS':(0,0o40000,12,0,(0,1,2)),'AD':(0,0o60000,12,0,(0,1,2)),
 'SU':(1,0o60000,10,0,(0,1,2)),'MASK':(0,0o70000,12,0,(1,2)),'TS':(0,0o54000,10,0,(0,1,2)),
 'XCH':(0,0o56000,10,0,(1,2)),'LXCH':(0,0o22000,10,0,(0,2)),'QXCH':(1,0o22000,10,0,(0,1)),
 'ADS':(0,0o26000,10,0,(0,1,2)),'INCR':(0,0o24000,10,0,(1,2)),'AUG':(1,0o24000,10,0,(1,2)),
 'DIM':(1,0o26000,10,0,(1,2)),'DAS':(0,0o20000,10,1,(0,)),'DCA':(1,0o30000,12,1,(0,)),
 'DCS':(1,0o40000,12,1,(0,)),'DXCH':(0,0o52000,10,1,(0,)),'MP':(1,0o70000,12,0,(0,1,2)),
 'DV':(1,0o10000,10,0,(2,)),'CCS':(0,0o10000,10,0,(0,1,2)),'BZF':(1,0o10000,12,0,()),'BZMF':(1,0o60000,12,0,())}
EDGE=[0,0o77777,1,0o77776,0o37777,0o40000,0o37776,0o40001,0o20000,0o57777,2,0o77775]
EDGE16=[0,0o177777,1,0o177776,0o37777,0o140000,0o40000,0o137777,0o77777,0o100000,0o57777,0o120000]
def w15(r):return r.choice(EDGE) if r.random()<.3 else r.getrandbits(15)
def w16(r):
 if r.random()<.3:return r.choice(EDGE16)
 v=r.getrandbits(15);return v|((v<<1)&0o100000) if r.random()<.7 else r.getrandbits(16)
def case(r,op):
 ext,base,bits,off,regs=ENC[op]
 img=[w15(r) for _ in range(2048)]
 # Every original instruction ends with L overflow-corrected and sign-extended.
 l=w16(r);img[0]=w16(r);img[1]=(l&0o37777)|((l>>1)&0o40000);img[1]|=(img[1]<<1)&0o100000;img[2]=r.getrandbits(15)
 eb=r.randrange(8);fb=r.randrange(32)<<10
 img[3]=eb<<8;img[4]=fb;img[6]=fb|eb;img[5]=0o700;img[7]=0
 img[0o20]=w15(r);img[0o21]=w15(r);img[0o22]=w15(r);img[0o23]=r.getrandbits(7)
 if op in('BZF','BZMF'):k=r.randrange(0o2000,0o10000)
 else:
  c=r.random()
  if c<.25 and regs:k=r.choice(regs)
  elif c<.4:k=r.choice((0o20,0o21,0o22,0o23,0o17,0o24) if not off else (0o17,0o20,0o21,0o22,0o23))
  elif c<.55:
   k=r.randrange(0o1400,0o1777 if off else 0o2000)
   # The EB-0 window aliases A-Z and the code words; those aliases are not
   # helper operands (the caller names central registers directly).
   while eb==0 and (k<0o1410 or 0o1676<=k<=0o1703) or eb==1 and 0o1676<=k-0o400+0o400<=0o1703 and False:k=r.randrange(0o1410,0o1777 if off else 0o2000)
   if eb==1 and 0o1676<=k<=0o1703:k=0o1500
  else:
   k=r.randrange(0o61,0o1377)
   while 0o676<=k<=0o703:k=r.randrange(0o61,0o1377)
 code=([6] if ext else [])+[(base+((k+off)&((1<<bits)-1)))&0o77777]
 return [OPS.index(op),k,len(code)]+code+[0]*(3-len(code))+img
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cases',type=int,default=4000);p.add_argument('--seed',type=int,default=1969);a=p.parse_args()
 (R/'build').mkdir(exist_ok=True)
 if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/agc_image_ops_reference.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/agc-image-ops-reference'],cwd=R,check=True)
 subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','tools/agc_image_ops_native.c','src/agc_image_ops.c','src/agc_hardware_divide.c','src/agc_multiword.c','src/agc_word.c','-o','build/agc-image-ops-native'],cwd=R,check=True)
 r=random.Random(a.seed);cases=[case(r,op) for op in OPS for _ in range(a.cases)]
 blob=b''.join(struct.pack('<2054H',*c) for c in cases)
 ref=subprocess.run(['build/agc-image-ops-reference','deps/lm-rtb-fixture/MAIN.agc.bin'],cwd=R,input=blob,capture_output=True,check=True).stdout
 nat=subprocess.run(['build/agc-image-ops-native'],cwd=R,input=blob,capture_output=True,check=True).stdout
 bad={}
 for i,c in enumerate(cases):
  x=struct.unpack_from('<2048H',ref,i*4096);y=struct.unpack_from('<2048H',nat,i*4096)
  if x!=y:
   op=OPS[c[0]];bad[op]=bad.get(op,0)+1
   if bad[op]<=3:print('FAIL',op,'K=%o'%c[1],'A=%06o L=%06o Q=%06o'%(c[6],c[7],c[8]),[('%04o'%j,'%06o'%x[j],'%06o'%y[j]) for j in range(2048) if x[j]!=y[j]][:8])
 if bad:print('FAIL counts',bad);raise SystemExit(1)
 print('PASS',len(cases),'single-instruction images equal yaAGC across',len(OPS),'helpers')
if __name__=='__main__':main()
