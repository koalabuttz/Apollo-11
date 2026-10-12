#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Interpreter body differential: each general compiled Luminary099 order
body or address-formation path (src/lm_interp_orders.c, lm_interp_steps.c)
against the pinned yaAGC hardware-DV engine running the unchanged rope from
the same image to the same stop address. Images are sampled
deterministically: order/operand words come from real Luminary099
interpretive code in the yaYUL listing, fixed operands from the rope, and
erasable state covers modes, signs, overflow and edge values. The whole
2048-word image and the stop address must agree."""
import argparse,random,re,struct,subprocess,sys,pathlib
R=pathlib.Path(__file__).resolve().parents[1]
LST=R/'build/ephemeris-source/Luminary099/assembly.lst'
ROM=R/'deps/lm-rtb-fixture/MAIN.agc.bin'
ORDER=[2,3,0,1]+list(range(4,36))
rom=b''
def fixed(bank,off):
 i=ORDER.index(bank)*1024+off;return((rom[2*i]<<8)|rom[2*i+1])>>1
def word_at(fb,a):
 return fixed(a>>10,a&0o1777) if a>=0o4000 else fixed(fb,a-0o2000)
def listing():
 rows=[]
 for l in LST.read_text(errors='replace').split('\n'):
  m=re.match(r'^\d+,\d+: (\d\d),(\d{4})\s+(\d{5})\s+(\S*)\s+(\S+)\s*(\S*)',l)
  if not m:
   m2=re.match(r'^\d+,\d+: (\d\d),(\d{4})\s+(\d{5})\s+(\S+)\s*(\S*)',l)
   continue
  bank,addr,w=int(m.group(1),8),int(m.group(2),8),int(m.group(3),8)
  rows.append((bank,addr,w,l))
 return rows
EDGE=[0,0o77777,1,0o77776,0o37777,0o40000,0o37776,0o40001,0o20000,0o57777,2,0o77775]
def w15(r):return r.choice(EDGE) if r.random()<.25 else r.getrandbits(15)
def w16(r):
 v=w15(r);v|=(v<<1)&0o100000
 if r.random()<.1:v^=0o40000
 return v
def base_image(r):
 img=[w15(r) for _ in range(2048)]
 eb=r.randrange(8);img[3]=eb<<8;img[4]=0;img[6]=eb;img[5]=0;img[7]=0
 img[0]=w16(r);l=w16(r);img[1]=(l&0o37777)|((l>>1)&0o40000);img[1]|=(img[1]<<1)&0o100000
 img[2]=r.getrandbits(12)
 vac=r.choice([0o401+0o54*i for i in range(5)])
 img[0o120]=vac;img[0o115]=0;img[0o67]=0;img[0o23]=0
 img[0o163]=r.choice([0o77776,0,1])
 img[0o166]=vac+r.randrange(8,38)          # PUSHLOC inside the work area
 img[0o165]=img[6]
 # Index registers as interpretive code keeps them (small, either sign).
 for x in(0o46,0o47):v=r.randrange(0,0o40);img[vac+x]=v if r.random()<.5 else 0o77777-v
 return img
def setbanks(img,fb):
 img[4]=fb<<10;img[6]=(fb<<10)|(img[3]>>8)
def erasable_addr(r,img):
 c=r.random()
 if c<.4:return r.randrange(0,0o55)          # work-area relative (fixed up by DIRADRES)
 if c<.7:return r.randrange(0o61,0o1370)
 a=r.randrange(0o1400,0o1770)
 if img[3]==0 and a<0o1410:a+=0o10
 return a
def edit_cyr(v):return((v>>1)|((v&1)<<14))&0o77777
class Gen:
 def __init__(s,r,rows):
  s.r=r;s.rows=rows
  s.pairs=[x for x in rows if x[2]&0o40000 and x[0]<0o40]
  s.stores=[x for x in rows if re.search(r'\s(STORE|STODL|STOVL|STODL\*|STOVL\*|STORE,[12]|STCALL)\s',x[3]) and not x[2]&0o40000 and x[0]<0o30]
  s.stadr=[x for x in rows if re.search(r'\sSTADR\s',x[3]) and x[0]<0o30]
  s.addr_next=[x for x in rows if x[2]&0o40000 and x[0]<0o30]
 def loc_with_next(s,img,positive):
  r=s.r
  while True:
   b,a,w,l=r.choice(s.pairs)
   if a>=0o3777 or b>=0o30:continue
   n=fixed(b,a+1-0o2000)
   if positive==(0<n<0o40000) and (positive or n&0o40000):
    setbanks(img,b);img[0o164]=a;img[0o165]=img[6];return n
def sx(v):return(v&0o77777)|((v<<1)&0o100000)
def stop_body(cyr):
 # INDEX CYR applied to "7 INDJUMP -1" (076273) and the INDJUMP table TCF.
 t=sx(0o76273)+sx(cyr)
 if t&0o200000:t=(t+1)&0o177777
 at=((t&0o37777)|((t>>1)&0o40000))&0o7777
 return word_at(0,at)&0o7777,at
STORE_NEXT=[0o6061,0o6061,0o6061,0o6460,0o6144,0o6463,0o6466,0o6252]
def tame_index(r,img,code):
 # Keep stores inside one EB window: the original never stores a vector or
 # triple across the end of the window, so such a store is done in DP mode.
 kind=(code>>11)&7;eff=code&0o3777
 if kind in(1,2):
  x=img[0o120]+(0o46 if kind==1 else 0o47)
  while True:
   v=r.randrange(0,0o40);v=v if r.random()<.5 else 0o77777-v
   sv=v if v<0o40000 else -(0o77777-v);e=(code&0o3777)-sv
   if e>=0:img[x]=v;eff=e;break
 if eff>=0o55 and (eff&0o377)>0o372:img[0o163]=0
def case(g,body):
 r=g.r;img=base_image(r);hdr=[body,0,0,0,0,r.randrange(2),r.choice([0,0o20,0o100]),0,0]+[0]*6+[0]
 if body in(0,1,2):
  # CYR as OPJUMP leaves it for a direct-address order (40000 + code,
  # edited), or the DLOADCOD/VLOADCOD that STODL/STOVL load.
  # (OPJUMP's TS CYR and CCS CYR rotate a direct order code 4*op+1 to 20000+op.)
  op=r.choice([o for o in range(0o40) if o!=0o35]);cyr=0o20000|op
  img[0o20]=cyr
  nxt=g.loc_with_next(img,body==0)
  hdr[7]=nxt
  tgt,_=stop_body(cyr);hdr[8]=tgt;hdr[1]=tgt
  if body==2:
   img[5]=0o6103;img[0]=0o37776-(op<<1)
  else:img[5]=0o6106
  if r.random()<.5:img[0o116]=erasable_addr(r,img)
  return hdr,img
 if body==3:
  b,a,w,l=r.choice(g.stores);setbanks(img,b);img[0o164]=a;img[0o165]=img[6];img[5]=0o6070;hdr[7]=w
  img[0o116]=0;tame_index(r,img,w)
  hdr[1]=STORE_NEXT[(w>>11)&7]
  return hdr,img
 if body==4:
  b,a,w,l=r.choice(g.stadr);setbanks(img,b);img[0o165]=img[6]
  # STADR is the left or right order of the pair at LOC; LOC+1 holds the complemented store code.
  img[0o164]=a;nxt=fixed(b,a+1-0o2000);hdr[7]=nxt;img[5]=0o6354;setbanks(img,b)
  code=((~nxt)&0o77777)-1;tame_index(r,img,code)
  hdr[1]=STORE_NEXT[(code>>11)&7]
  return hdr,img
 if body in(5,6):
  img[5]=0o6460 if body==5 else 0o6463;hdr[1]=0o6106;return hdr,img
 if body in(7,8,9,10,11):
  img[5]={7:0o6052,8:0o6505,9:0o6557,10:0o6523,11:0o3247}[body];hdr[1]=0o6061
  if body==11:setbanks(img,0);return hdr,img
  if r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3772);img[0o116]=a
   hdr[9:15]=[fixed(b,a-0o2000+i) for i in range(6)];hdr[15]=1
  else:
   a=r.randrange(0o61,0o1360) if r.random()<.7 else r.randrange(0o1410,0o1770)
   img[0o116]=a
  return hdr,img
 raise SystemExit('unknown body')
NAMES=['DIRADRES','DIRADRES PUSHUP','ADDRESS PUSHUP','STORE codes','STADR','DODLOAD','DOVLOAD','DLOAD','VLOAD','PDVL','PDDL','PUSH']
def main():
 global rom
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cases',type=int,default=3000);p.add_argument('--seed',type=int,default=1969)
 p.add_argument('--bodies',default=','.join(str(i) for i in range(len(NAMES))));p.add_argument('--dump');a=p.parse_args()
 (R/'build').mkdir(exist_ok=True)
 if not LST.exists():subprocess.run([sys.executable,'tools/ephemeris_source.py'],cwd=R,check=True,stdout=subprocess.DEVNULL)
 if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
 rom=ROM.read_bytes()
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/lmi_body_reference.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/lmi-body-reference'],cwd=R,check=True)
 subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','tools/lmi_body_native.c','src/lm_interp_orders.c','src/lm_interp_steps.c','src/agc_image_ops.c','src/agc_hardware_divide.c','src/agc_multiword.c','src/agc_word.c','-o','build/lmi-body-native'],cwd=R,check=True)
 g=Gen(random.Random(a.seed),listing());bodies=[int(x) for x in a.bodies.split(',')]
 cases=[case(g,b) for b in bodies for _ in range(a.cases)]
 blob=b''.join(struct.pack('<2064H',*(h+i)) for h,i in cases)
 if a.dump:pathlib.Path(a.dump).write_bytes(blob)
 ref=subprocess.run(['build/lmi-body-reference',str(ROM)],cwd=R,input=blob,capture_output=True)
 if ref.returncode:print(ref.stderr.decode()[-2000:]);raise SystemExit('FAIL reference exit %d'%ref.returncode)
 nat=subprocess.run(['build/lmi-body-native'],cwd=R,input=blob,capture_output=True,check=True).stdout
 bad={}
 for i,(h,img) in enumerate(cases):
  x=struct.unpack_from('<2049H',ref.stdout,i*4098);y=struct.unpack_from('<2049H',nat,i*4098)
  if x!=y:
   n=NAMES[h[0]];bad[n]=bad.get(n,0)+1
   if a.dump:pathlib.Path(a.dump+'.fail%d'%bad[n]).write_bytes(blob[i*4128:(i+1)*4128])
   if bad[n]<=3:print('FAIL',n,'case',i,'args %o %o'%(h[7],h[8]),'stop',x[0],y[0],[('%04o'%(j-1),'%06o'%x[j],'%06o'%y[j]) for j in range(1,2049) if x[j]!=y[j]][:10])
 if bad:print('FAIL counts',bad);raise SystemExit(1)
 print('PASS',len(cases),'body images equal yaAGC:',', '.join(NAMES[b] for b in bodies))
main()
