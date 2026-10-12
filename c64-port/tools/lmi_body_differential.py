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
   # (77777 would be a pair of EXIT codes: CCS sees +0 and takes the
   # address-given path through NEG4, so it is never an order pair here.)
   if positive==(0<n<0o40000) and (positive or (n&0o40000 and n!=0o77777)):
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
 hdr[1]=0o6061
 if body in ARITH:
  # Operand at C(ADDRWD): live erasable (either window), or fixed data in a
  # bank <030 with the words passed to the native side.
  img[5]=ARITH[body];n=6 if body==25 else 2
  # (SIGN's INDEX ADDRWD; CCS 0 is a TCF for a fixed address: erasable only.)
  if body!=21 and r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o4000-n-1);img[0o116]=a
   hdr[9:9+n]=[fixed(b,a-0o2000+i) for i in range(n)];hdr[15]=1
  else:
   img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1410,0o1770)
  return hdr,img
 if body==17:
  # OPJUMP2 for a short shift code (low three bits 100): OPJUMP's TS CYR
  # and CCS CYR have rotated it twice and left DABS-1 of the first in A.
  c=(r.randrange(16)<<3)|4;img[0o20]=edit_cyr(edit_cyr(c));img[0]=edit_cyr(c)-1;img[5]=0o6247
  return hdr,img
 if body==18:
  if r.random()<.6:v=r.randrange(0,0o60)|r.choice([0,0o200,0o400,0o600])|r.choice([0,0o1000])
  else:v=r.getrandbits(10)
  img[0o116]=v;img[5]=0o7621;return hdr,img
 if body==19:
  img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1410,0o1770)
  if r.random()<.15:
   for i in range(3):img[0o154+i]=r.choice([0,0o77777,1,0o77776])
  img[5]=0o7616;return hdr,img
 if body in(26,27):
  img[5]={26:0o7577,27:0o7603}[body]
  if r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3775);img[0o116]=a
   hdr[9:11]=[fixed(b,a-0o2000+i) for i in range(2)];hdr[15]=1;op=hdr[9:11]
  else:
   while True:
    a=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1410,0o1770)
    e=(img[3]>>8)*0o400+a-0o1400 if a>=0o1400 else a
    if e not in(0o115,0o116):break   # (the operand never overlaps ADDRWD)
   img[0o116]=a;a=e;op=None
  # Usually a dividend smaller than the divisor, as the original's callers keep it.
  if r.random()<.6:
   sh=r.randrange(1,15);hi=img[0o154] if body==26 else (op[0] if op else img[a])
   v=(hi&0o37777)>>sh
   if hi&0o40000:v=0o77777-v
   if body==26:img[0o154]=v
   elif not op:img[a]=v
  if r.random()<.1:img[0o154]=0;img[0o155]=r.choice([0,0o77777,r.getrandbits(15)])
  if not op and r.random()<.15:img[a]=img[0o154]   # equal majors (MAXDV)
  return hdr,img
 if body in(28,29,30):
  setbanks(img,0);img[5]={28:0o3207,29:0o3176,30:0o3023}[body];hdr[2]=0o5726
  if body==30:img[0o163]=0o77776
  for i in range(7):
   if r.random()<.5:
    v=r.getrandbits(r.randrange(1,15))
    img[0o154+i]=v if r.random()<.6 else 0o77777-v
  if body==28 and r.random()<.6:
   for i in range(3):img[0o154+i]&=0o37777
  if r.random()<.1:img[0o154]=0
  if r.random()<.05:img[0o155]=0
  c=r.random()
  if c<.1:
   for i in(0,3,5):img[0o154+i]=r.choice([0,0o77777]);img[0o155+i]=r.getrandbits(r.randrange(1,15))
  elif c<.15:
   for i in range(7):img[0o154+i]=r.choice([0,0o77777])
  elif c<.2:img[0o154]=img[0o155]=0o37777
  return hdr,img
 if body in(31,32):
  img[5]={31:0o7401,32:0o7460}[body]
  if r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3770);img[0o116]=a
   hdr[9:15]=[fixed(b,a-0o2000+i) for i in range(6)];hdr[15]=1
  else:img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1410,0o1770)
  return hdr,img
 if body==33:
  setbanks(img,1);img[5]=0o2457;hdr[2]=0o6646;hdr[3]=0o2450
  img[0o121]=r.choice([0,1,0o77776,0o77777,r.getrandbits(15)])
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  return hdr,img
 if body==34:
  setbanks(img,1);img[5]=0o2450;b=r.randrange(0o30);off=r.choice([x for x in range(0o2000) if not 0o450<=x<=0o453])  # (RTB's own words)
  img[0o117]=b*0o2000+off;hdr[1]=0o2000+off;return hdr,img
 if body==35:
  setbanks(img,r.randrange(0o30));img[5]=0o4631;img[0o133]=0o2453;img[0o134]=0o2000;return hdr,img
 if body==36:
  setbanks(img,0o27);img[5]=0o7222;img[2]=0o3736;hdr[1]=0o3753
  for i in range(2):
   v=r.getrandbits(14);img[0o154+i]=v if r.random()<.5 else 0o77777-v
  return hdr,img
 if body==37:
  setbanks(img,1);img[5]=0o2474;hdr[1]=0o6646;hdr[2]=0o6061
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777),0o77777])  # +0 would run 5B10 as code
  for i in range(3):img[0o154+i]=r.choice([0,0o77777,r.getrandbits(15)]) if r.random()<.5 else r.getrandbits(15)
  return hdr,img
 if body in(38,39,40,41):
  img[5]={38:0o6751,39:0o6747,40:0o7036,41:0o7071}[body];n=3 if body==41 else 6
  if r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o4000-n-1);img[0o116]=a
   hdr[9:9+n]=[fixed(b,a-0o2000+i) for i in range(n)];hdr[15]=1
  else:
   img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1410,0o1770)
  if body!=41:img[0o163]=0o77776
  if r.random()<.2:
   for i in range(6):img[0o154+i]=r.choice([0o37777,0o40000,0o37776])
  return hdr,img
 if body==42:
  setbanks(img,0);img[5]=0o2116;img[0o163]=r.choice([0,0o77776,1])
  if r.random()<.2:img[0o154]=img[0o155]=r.choice([0o37777,0o40000])
  return hdr,img
 if body==43:
  b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3776);img[0o164]=a;img[0o165]=img[6]
  hdr[7]=fixed(b,a+1-0o2000);img[5]=0o6620
  img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1400,0o2000)
  return hdr,img
 if body in(44,45,46,47):
  setbanks(img,1);img[5]={44:0o2400,45:0o2411,46:0o2354,47:0o2364}[body]
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  if r.random()<.4:img[0o117]=r.randrange(0,0o55)
  else:
   while True:
    v=r.randrange(0o56,0o4000)
    if v>=0o61 and not 0o1370<=v<0o1400 and (v>>8)!=0 or (v<0o400 and v>=0o61):break
   img[0o117]=v
  return hdr,img
 if body==48:
  setbanks(img,1);img[5]=0o2452;hdr[2]=0o6646
  img[0o154]=r.choice([0,0o77777,r.getrandbits(15)])
  return hdr,img
 if body==49:
  setbanks(img,1);img[5]=0o2507;hdr[1]=0o6646
  img[0o20]=r.randrange(1,0o40000);img[0o117]=r.randrange(0o4000,0o100000)&0o77777
  img[0o164]=r.randrange(0o2000,0o4000)
  return hdr,img
 if body==50:
  setbanks(img,r.randrange(0o30));img[5]=0o6646;hdr[1]=0o6070
  tgt=(r.randrange(2,0o40)<<10)|r.randrange(0o2000)
  c=r.random()
  if c<.3:
   p_=r.randrange(0,0o55);img[0o117]=p_;img[(img[0o120]+p_)&0o3777]=tgt
  elif c<.35:
   img[0o117]=0o55;img[0o55]=tgt
  elif c<.9:
   while True:
    p_=r.randrange(0o56,0o4000);eb=p_>>8;off=p_&0o377
    absa=eb*0o400+off if eb>=3 else (0o1400+off if False else None)
    if eb>=3:break
   img[0o117]=p_;img[absa-0o1400+0o1400 if False else (eb*0o400+off)]=tgt
  else:
   img[0o117]=tgt
  return hdr,img
 if body==51:
  b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3777);img[0o164]=a;img[0o165]=img[6]
  img[5]=0o6743;hdr[1]=a+1
  return hdr,img
 if body==52:
  b=r.randrange(2,0o30);setbanks(img,b);img[5]=0o6037;hdr[1]=0o6070;img[2]=r.randrange(0o2000,0o4000)
  return hdr,img
 if body==53:
  setbanks(img,0o10);img[5]=0o3633
  return hdr,img
 if body==54:
  b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3776);img[0o164]=a;img[0o165]=img[6]
  hdr[7]=fixed(b,a+1-0o2000);img[5]=0o6252;img[0o20]=0o15;hdr[8]=0o2507;hdr[1]=0o2507
  return hdr,img
 if body==55:
  setbanks(img,1);img[5]=0o2467;hdr[2]=0o6646
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  for i in range(3):img[0o154+i]=r.choice([0,0o77777,r.getrandbits(15)]) if r.random()<.6 else r.getrandbits(15)
  return hdr,img
 if body==56:
  # ITA (STQ): CALL/ITA with CYR negative (edited 15-bit ITA code), POLISH
  # the erasable address, QPRET any CADR.
  setbanks(img,1);img[5]=0o2507
  img[0o20]=r.randrange(0o40000,0o77777)
  img[(img[0o120]+0o52)&0o3777]=r.getrandbits(15)
  if r.random()<.4:img[0o117]=r.randrange(0,0o55)
  else:
   while True:
    v=r.randrange(0o61,0o4000)
    if not 0o1370<=v<0o1400 and v!=0o117:break
   img[0o117]=v
  return hdr,img
 if body==57:
  # TIX: POLISH the branch CADR, X and S of the selected index either sign.
  setbanks(img,1);img[5]=0o2415;hdr[2]=0o6646
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  img[0o117]=(r.randrange(2,0o40)<<10)|r.randrange(0o2000)
  vac=img[0o120]
  for x in(0o46,0o47,0o50,0o51):
   v=r.randrange(0,0o60);img[vac+x]=v if r.random()<.6 else 0o77777-v
  return hdr,img
 if body in BANK0:
  setbanks(img,0);img[5]=BANK0[body]
  if body==22 and r.random()<.15:
   for i in range(3):img[0o154+i]=r.choice([0,0o77777])
  return hdr,img
 raise SystemExit('unknown body')
ARITH={12:0o6775,13:0o7005,14:0o7062,15:0o7572,16:0o7574,21:0o7655,25:0o7331}
BANK0={20:0o7670,22:0o3226,23:0o3174,24:0o3245}
NAMES=['DIRADRES','DIRADRES PUSHUP','ADDRESS PUSHUP','STORE codes','STADR','DODLOAD','DOVLOAD','DLOAD','VLOAD','PDVL','PDDL','PUSH',
 'DAD','DSU','BDSU','DMP','DMPR','short shifts','GSHIFT','TSLC','COMP','SIGN','ABS','DSQ','VSQ','DOT',
 'DDV','BDDV','SQRT','ABVAL/ABS','UNIT','VXSC','VXV','BOV(B)','RTB','SWRETURN','POLY','BPL/BMN',
 'VAD','VSU','BVSU','TAD','ROUND','SSP','XAD','XSU','LXA','SXA','BHIZ','CALL','GOTO indirect','EXIT','INTPRET','TPMODE','15BITADR','BZE/GOTO','ITA/STQ','TIX']
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
