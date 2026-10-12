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
sys.path.insert(0,str(R/'tools'))
from cm_interp_generate import Rope,comanche
LST=R/'build/ephemeris-source/Luminary099/assembly.lst'
ROM=R/'deps/lm-rtb-fixture/MAIN.agc.bin'
# --mission cm runs the Comanche055 build (src/cm_interp_*.c) against the
# Comanche055 rope: the same cases, each Luminary099 address A() replaced by
# the same instruction's Comanche055 address (tools/cm_interp_generate.py).
MISSION='lm';ROPES=None
POODOO1,CM_POODOO=0o5726,0o5721   # SQRTABRT's alarm entry (TC POODOO1 / TC POODOO)
def A(a,bank=None):
 if MISSION=='lm':return a
 if a==POODOO1:return CM_POODOO
 c=comanche(ROPES[0],ROPES[1],None if a>=0o4000 else '%02o'%(bank or 0),a)
 if c is None:raise SystemExit('no Comanche055 address for %o'%a)
 return c
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
FILL=None   # a C-reproducible fill for --vectors cases (xs_fill)
def xs_fill(seed):
 # xorshift16 (7,9,8), as tests/interp_body_run.c regenerates it on the C64.
 x=seed&0xffff or 1;out=[]
 for _ in range(2048):
  x^=(x<<7)&0xffff;x^=x>>9;x^=(x<<8)&0xffff;out.append(x&0o77777)
 return out
def base_image(r):
 img=list(FILL) if FILL else [w15(r) for _ in range(2048)]
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
 t=sx(0o70000|A(0o6273))+sx(cyr)
 if t&0o200000:t=(t+1)&0o177777
 at=((t&0o37777)|((t>>1)&0o40000))&0o7777
 return word_at(0,at)&0o7777,at
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
def vsc_shape(r,img,hdr):
 # V/SC: mode picks which side is the vector. Shape the scalar (MPAC or
 # the operand) so dividends usually fit: small, zero major part (the
 # shift-left-14 path), zero, or a vector component near the divisor.
 img[0o163]=r.choice([0o77776,0o77776,0,1])
 vec=img[0o163]==0o77776
 if hdr[15]:ops=hdr[9:15]
 else:
  a=img[0o116];e=(img[3]>>8)*0o400+a-0o1400 if a>=0o1400 else a
  if e in range(0o115-5,0o117) or e in range(0o154-5,0o164) or e in range(0o130-5,0o142):img[0o116]=a=0o200;e=a
  ops=None
 def put(i,v):
  if ops is None:img[e+i]=v       # (fixed operands stay as the rope has them)
 def get(i):return hdr[9+i] if ops is not None else img[e+i]
 sc=(lambda i,v:put(i,v)) if vec else (lambda i,v:img.__setitem__(0o154+i,v))
 vc=(lambda i:0o154+i) if vec else None
 c=r.random()
 if c<.15:sc(0,0);sc(1,r.choice([0,0o77777,r.getrandbits(15)]))
 elif c<.2:sc(0,r.choice([0,0o77777]));sc(1,r.choice([0,0o77777]))
 elif c<.3:sc(0,0o37777);sc(1,0o37777)
 # Shrink vector components often so the quotient fits.
 for i in range(6):
  if r.random()<.5:
   v=r.getrandbits(r.randrange(1,14))
   v=v if r.random()<.6 else 0o77777-v
   if vec:img[0o154+i]=v
   else:put(i,v)
 if r.random()<.1:
  for i in range(6):
   v=r.choice([0,0o77777])
   if vec:img[0o154+i]=v
   else:put(i,v)
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
   img[5]=A(0o6103);img[0]=0o37776-(op<<1)
  else:img[5]=A(0o6106)
  if r.random()<.5:img[0o116]=erasable_addr(r,img)
  return hdr,img
 if body==3:
  b,a,w,l=r.choice(g.stores);setbanks(img,b);img[0o164]=a;img[0o165]=img[6];img[5]=A(0o6070);hdr[7]=w
  img[0o116]=0;tame_index(r,img,w)
  hdr[1]=STORE_NEXT[(w>>11)&7]
  return hdr,img
 if body==4:
  b,a,w,l=r.choice(g.stadr);setbanks(img,b);img[0o165]=img[6]
  # STADR is the left or right order of the pair at LOC; LOC+1 holds the complemented store code.
  img[0o164]=a;nxt=fixed(b,a+1-0o2000);hdr[7]=nxt;img[5]=A(0o6354);setbanks(img,b)
  code=((~nxt)&0o77777)-1;tame_index(r,img,code)
  hdr[1]=STORE_NEXT[(code>>11)&7]
  return hdr,img
 if body in(5,6):
  img[5]=A(0o6460) if body==5 else A(0o6463);hdr[1]=A(0o6106);return hdr,img
 if body in(7,8,9,10,11):
  img[5]={7:A(0o6052),8:A(0o6505),9:A(0o6557),10:A(0o6523),11:A(0o3247,0)}[body];hdr[1]=A(0o6061)
  if body==11:setbanks(img,0);return hdr,img
  if r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3772);img[0o116]=a
   hdr[9:15]=[fixed(b,a-0o2000+i) for i in range(6)];hdr[15]=1
  else:
   a=r.randrange(0o61,0o1360) if r.random()<.7 else r.randrange(0o1410,0o1770)
   img[0o116]=a
  return hdr,img
 hdr[1]=A(0o6061)
 if body in ARITH:
  # Operand at C(ADDRWD): live erasable (either window), or fixed data in a
  # bank <030 with the words passed to the native side.
  img[5]=ARITH[body];n=6 if body in(25,59) else 2
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
  c=(r.randrange(16)<<3)|4;img[0o20]=edit_cyr(edit_cyr(c));img[0]=edit_cyr(c)-1;img[5]=A(0o6247)
  return hdr,img
 if body==18:
  if r.random()<.6:v=r.randrange(0,0o60)|r.choice([0,0o200,0o400,0o600])|r.choice([0,0o1000])
  else:v=r.getrandbits(10)
  img[0o116]=v;img[5]=A(0o7621);return hdr,img
 if body==19:
  img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1410,0o1770)
  if r.random()<.15:
   for i in range(3):img[0o154+i]=r.choice([0,0o77777,1,0o77776])
  img[5]=A(0o7616);return hdr,img
 if body in(26,27):
  img[5]={26:A(0o7577),27:A(0o7603)}[body]
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
  setbanks(img,0);img[5]={28:A(0o3207,0),29:A(0o3176,0),30:A(0o3023,0)}[body];hdr[2]=A(0o5726)
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
 if body in(31,32,71):
  img[5]={31:A(0o7401),32:A(0o7460),71:A(0o7624)}[body]
  if r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3770);img[0o116]=a
   hdr[9:15]=[fixed(b,a-0o2000+i) for i in range(6)];hdr[15]=1
  else:img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1410,0o1770)
  if body==71:vsc_shape(r,img,hdr)
  return hdr,img
 if body==33:
  setbanks(img,1);img[5]=A(0o2457,1);hdr[2]=A(0o6646);hdr[3]=A(0o2450,1)
  img[0o121]=r.choice([0,1,0o77776,0o77777,r.getrandbits(15)])
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  return hdr,img
 if body==34:
  setbanks(img,1);img[5]=A(0o2450,1);b=r.randrange(0o30);off=r.choice([x for x in range(0o2000) if not A(0o2450,1)-0o2000<=x<=A(0o2453,1)-0o2000])  # (RTB's own words)
  img[0o117]=b*0o2000+off;hdr[1]=0o2000+off;return hdr,img
 if body==35:
  setbanks(img,r.randrange(0o30));img[5]=A(0o4631);img[0o133]=A(0o2453,1);img[0o134]=0o2000;return hdr,img
 if body==36:
  setbanks(img,0o27);img[5]=0o7222;img[2]=0o3736;hdr[1]=0o3753
  for i in range(2):
   v=r.getrandbits(14);img[0o154+i]=v if r.random()<.5 else 0o77777-v
  return hdr,img
 if body==37:
  setbanks(img,1);img[5]=A(0o2474,1);hdr[1]=A(0o6646);hdr[2]=A(0o6061)
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777),0o77777])  # +0 would run 5B10 as code
  for i in range(3):img[0o154+i]=r.choice([0,0o77777,r.getrandbits(15)]) if r.random()<.5 else r.getrandbits(15)
  return hdr,img
 if body in(38,39,40,41):
  img[5]={38:A(0o6751),39:A(0o6747),40:A(0o7036),41:A(0o7071)}[body];n=3 if body==41 else 6
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
  setbanks(img,0);img[5]=A(0o2116,0);img[0o163]=r.choice([0,0o77776,1])
  if r.random()<.2:img[0o154]=img[0o155]=r.choice([0o37777,0o40000])
  return hdr,img
 if body==43:
  b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3776);img[0o164]=a;img[0o165]=img[6]
  hdr[7]=fixed(b,a+1-0o2000);img[5]=A(0o6620)
  img[0o116]=r.randrange(0o61,0o1370) if r.random()<.7 else r.randrange(0o1400,0o2000)
  return hdr,img
 if body in(44,45,46,47):
  setbanks(img,1);img[5]={44:A(0o2400,1),45:A(0o2411,1),46:A(0o2354,1),47:A(0o2364,1)}[body]
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  if r.random()<.4:img[0o117]=r.randrange(0,0o55)
  else:
   while True:
    v=r.randrange(0o56,0o4000)
    if v>=0o61 and not 0o1370<=v<0o1400 and (v>>8)!=0 or (v<0o400 and v>=0o61):break
   img[0o117]=v
  return hdr,img
 if body in(68,69):
  # MXV/VXM: an 18-word matrix in erasable (inside one EB window) or in a
  # fixed bank <030, read from the rope by the native side.
  img[5]={68:A(0o7334),69:A(0o7337)}[body]
  if r.random()<.35:
   b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o4000-19);img[0o116]=a
   hdr[9]=b;hdr[10]=a;hdr[15]=2
  else:img[0o116]=r.randrange(0o61,0o1370-18) if r.random()<.7 else r.randrange(0o1410,0o1770-18)
  if r.random()<.2:
   for i in range(6):img[0o154+i]=r.choice([0o37777,0o40000,0o37776])
  return hdr,img
 if body==70:
  # LXC: as LXA.
  setbanks(img,1);img[5]=A(0o2360,1)
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  if r.random()<.4:img[0o117]=r.randrange(0,0o55)
  else:
   while True:
    v=r.randrange(0o56,0o4000)
    if v>=0o61 and not 0o1370<=v<0o1400 and (v>>8)!=0 or (v<0o400 and v>=0o61):break
   img[0o117]=v
  return hdr,img
 if body==61:
  setbanks(img,1);img[5]=A(0o2351,1)
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  img[0o117]=r.randrange(0,0o100000)
  return hdr,img
 if body==48:
  setbanks(img,1);img[5]=A(0o2452,1);hdr[2]=A(0o6646)
  img[0o154]=r.choice([0,0o77777,r.getrandbits(15)])
  return hdr,img
 if body==49:
  setbanks(img,1);img[5]=A(0o2507,1);hdr[1]=A(0o6646)
  img[0o20]=r.randrange(1,0o40000);img[0o117]=r.randrange(0o4000,0o100000)&0o77777
  img[0o164]=r.randrange(0o2000,0o4000)
  return hdr,img
 if body==50:
  setbanks(img,r.randrange(0o30));img[5]=A(0o6646);hdr[1]=A(0o6070)
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
  img[5]=A(0o6743);hdr[1]=a+1
  return hdr,img
 if body==52:
  b=r.randrange(2,0o30);setbanks(img,b);img[5]=A(0o6037);hdr[1]=A(0o6070);img[2]=r.randrange(0o2000,0o4000)
  return hdr,img
 if body==53:
  # TPMODE: Luminary099 10,3633; Comanche055 22,3565.
  if MISSION=='lm':setbanks(img,0o10);img[5]=0o3633
  else:setbanks(img,0o22);img[5]=0o3565
  return hdr,img
 if body==54:
  b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3776);img[0o164]=a;img[0o165]=img[6]
  hdr[7]=fixed(b,a+1-0o2000);img[5]=A(0o6252);img[0o20]=0o15;hdr[8]=A(0o2507,1);hdr[1]=A(0o2507,1)
  return hdr,img
 if body==55:
  setbanks(img,1);img[5]=A(0o2467,1);hdr[2]=A(0o6646)
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  for i in range(3):img[0o154+i]=r.choice([0,0o77777,r.getrandbits(15)]) if r.random()<.6 else r.getrandbits(15)
  return hdr,img
 if body==56:
  # ITA (STQ): CALL/ITA with CYR negative (edited 15-bit ITA code), POLISH
  # the erasable address, QPRET any CADR.
  setbanks(img,1);img[5]=A(0o2507,1)
  img[0o20]=r.randrange(0o40000,0o77777)
  img[(img[0o120]+0o52)&0o3777]=r.getrandbits(15)
  if r.random()<.4:img[0o117]=r.randrange(0,0o55)
  else:
   while True:
    v=r.randrange(0o61,0o4000)
    if not 0o1370<=v<0o1400 and v!=0o117:break
   img[0o117]=v
  return hdr,img
 if body==58:
  # TLOAD from erasable (either window or the work area after DIRADRES).
  img[5]=A(0o6470);hdr[1]=A(0o6061)
  img[0o116]=r.randrange(0o61,0o1360) if r.random()<.7 else r.randrange(0o1410,0o1765)
  return hdr,img
 if body==60:
  # TLOAD from fixed data in a bank <030, the three words passed natively.
  img[5]=A(0o6470);hdr[1]=A(0o6061)
  b=r.randrange(0o30);setbanks(img,b);a=r.randrange(0o2000,0o3775);img[0o116]=a
  hdr[9:12]=[fixed(b,a-0o2000+i) for i in range(3)];hdr[15]=1
  return hdr,img
 if body==57:
  # TIX: POLISH the branch CADR, X and S of the selected index either sign.
  setbanks(img,1);img[5]=A(0o2415,1);hdr[2]=A(0o6646)
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  img[0o117]=(r.randrange(2,0o40)<<10)|r.randrange(0o2000)
  vac=img[0o120]
  for x in(0o46,0o47,0o50,0o51):
   v=r.randrange(0,0o60);img[vac+x]=v if r.random()<.6 else 0o77777-v
  return hdr,img
 if body==61:
  # AXC: POLISH the operand word, CYR's sign selects X1 or X2.
  setbanks(img,1);img[5]=A(0o2351,1)
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  img[0o117]=r.choice([r.randrange(0,0o40),0o77777-r.randrange(0,0o40),r.getrandbits(15)])
  return hdr,img
 if body==62:
  # XCHX: POLISH an erasable address (work area or either window), as SXA.
  setbanks(img,1);img[5]=A(0o2372,1)
  img[0o20]=r.choice([r.randrange(1,0o40000),r.randrange(0o40000,0o77777)])
  if r.random()<.4:img[0o117]=r.randrange(0,0o55)
  else:
   while True:
    v=r.randrange(0o56,0o4000)
    if v>=0o61 and not 0o1370<=v<0o1400 and (v>>8)!=0 or (v<0o400 and v>=0o61):break
   img[0o117]=v
  return hdr,img
 if body in(63,64):
  # ARCCOS/ARCSIN of 2 MPAC: inside (-.5,.5), at and just past the .5 edge
  # (ACOS=0 and the 1301 alarm), zero forms, and near-edge arguments that
  # need ACOSSHR. Stops: DANZIG, the alarm service, SQRTSUB's abort.
  setbanks(img,0);img[5]=A(0o3612 if body==63 else 0o3610,0)
  hdr[2]=0o5732 if MISSION=='lm' else 0o5650;hdr[3]=A(POODOO1)
  c=r.random();sg=r.random()<.5
  def neg(v):return 0o77777-v if sg else v
  if c<.45:img[0o154]=neg(r.randrange(0,0o20000));img[0o155]=neg(r.getrandbits(14))
  elif c<.6:img[0o154]=neg(0o20000-r.randrange(1,4));img[0o155]=neg(r.getrandbits(14))
  elif c<.7:img[0o154]=neg(0o20000);img[0o155]=r.choice([0,0o77777,r.getrandbits(14),0o77777-r.getrandbits(14)])
  elif c<.78:img[0o154]=neg(r.choice([0o20001,0o20002,r.randrange(0o20001,0o40000)]));img[0o155]=r.getrandbits(15)
  elif c<.88:img[0o154]=r.choice([0,0o77777]);img[0o155]=r.choice([0,0o77777,r.getrandbits(15)])
  elif c<.94:img[0o154]=neg(r.randrange(0,4));img[0o155]=neg(r.getrandbits(14))
  else:img[0o154]=r.getrandbits(15);img[0o155]=r.getrandbits(15)
  img[0o163]=r.choice([0,1,0o77776])
  return hdr,img
 if body==67:
  # ACOSABRT after the alarm service returns past OCT 1301 (Luminary
  # 00,3727, Comanche 00,3724), with ESCAPE/ESCAPE2 as ARCSIN/ARCCOS set them.
  setbanks(img,0);img[5]=0o3727 if MISSION=='lm' else 0o3724
  w=lambda lm,cm:lm if MISSION=='lm' else cm
  las,ldz,tcs=w(0o13707,0o13706),w(0o16061,0o16030),w(0o13702,0o13701)
  c=r.randrange(3)
  if c==0:img[0o136]=las
  elif c==1:img[0o136]=ldz
  else:img[0o136]=tcs;img[0o137]=r.choice([las,ldz])
  return hdr,img
 if body in(65,66):
  # SIN/COS of MPAC (fraction of a revolution): any DP word pair, signed
  # zeros, quarter and half turns and their neighbours (the reductions).
  setbanks(img,0);img[5]=A(0o3530 if body==65 else 0o3517,0)
  c=r.random()
  if c<.6:img[0o154]=r.getrandbits(15);img[0o155]=r.getrandbits(15)
  elif c<.8:
   v=r.choice([0,0o10000,0o20000,0o30000,0o37777])+r.randrange(-2,3)
   v=max(0,min(0o37777,v));sg=r.random()<.5;img[0o154]=0o77777-v if sg else v
   img[0o155]=r.choice([0,0o77777,r.getrandbits(15)])
  else:img[0o154]=r.choice([0,0o77777]);img[0o155]=r.choice([0,0o77777,r.getrandbits(15)])
  img[0o163]=r.choice([0,1,0o77776])
  return hdr,img
 if body in BANK0:
  setbanks(img,0);img[5]=BANK0[body]
  if body==22 and r.random()<.15:
   for i in range(3):img[0o154+i]=r.choice([0,0o77777])
  return hdr,img
 raise SystemExit('unknown body')
def addresses():
 global STORE_NEXT,ARITH,BANK0
 STORE_NEXT=[A(0o6061),A(0o6061),A(0o6061),A(0o6460),A(0o6144),A(0o6463),A(0o6466),A(0o6252)]
 ARITH={12:A(0o6775),13:A(0o7005),14:A(0o7062),15:A(0o7572),16:A(0o7574),21:A(0o7655),25:A(0o7331),59:A(0o7425)}
 BANK0={20:A(0o7670),22:A(0o3226,0),23:A(0o3174,0),24:A(0o3245,0)}
NAMES=['DIRADRES','DIRADRES PUSHUP','ADDRESS PUSHUP','STORE codes','STADR','DODLOAD','DOVLOAD','DLOAD','VLOAD','PDVL','PDDL','PUSH',
 'DAD','DSU','BDSU','DMP','DMPR','short shifts','GSHIFT','TSLC','COMP','SIGN','ABS','DSQ','VSQ','DOT',
 'DDV','BDDV','SQRT','ABVAL/ABS','UNIT','VXSC','VXV','BOV(B)','RTB','SWRETURN','POLY','BPL/BMN',
 'VAD','VSU','BVSU','TAD','ROUND','SSP','XAD','XSU','LXA','SXA','BHIZ','CALL','GOTO indirect','EXIT','INTPRET','TPMODE','15BITADR','BZE/GOTO','ITA/STQ','TIX','TLOAD','VPROJ','TLOAD fixed','AXC',
 'XCHX','ARCCOS','ARCSIN','SIN','COS','ACOSABRT return','MXV','VXM','LXC','V/SC']
def main():
 global rom,MISSION,ROPES,LST,ROM
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cases',type=int,default=3000);p.add_argument('--seed',type=int,default=1969)
 p.add_argument('--mission',choices=['lm','cm'],default='lm')
 p.add_argument('--bodies');p.add_argument('--dump')
 p.add_argument('--vectors',help='also write C64 vectors: cases rebuilt from an xorshift fill plus their edits, with yaAGC digests')
 p.add_argument('--vector-cases',type=int,default=2);p.add_argument('--proof',help='write a JSON summary');a=p.parse_args()
 MISSION=a.mission
 if MISSION=='cm':
  # (POLY, body 36, is driven by T(X)'s Luminary table; Comanche cases use other bodies.)
  LST=R/'build/fixed-data/Comanche055/assembly.lst';ROM=R/'deps/cm-rtb-fixture/MAIN.agc.bin'
  ROPES=(Rope('Luminary099'),Rope('Comanche055'))
 if a.bodies is None:a.bodies=','.join(str(i) for i in range(len(NAMES)) if NAMES[i]!='reserved' and not (MISSION=='cm' and i==36))
 addresses()
 (R/'build').mkdir(exist_ok=True)
 if not LST.exists():subprocess.run([sys.executable,'tools/ephemeris_source.py'],cwd=R,check=True,stdout=subprocess.DEVNULL)
 if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
 rom=ROM.read_bytes()
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/lmi_body_reference.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/lmi-body-reference'],cwd=R,check=True)
 native='build/lmi-body-native' if MISSION=='lm' else 'build/cmi-body-native'
 bodies_src=['src/lm_interp_orders.c','src/lm_interp_steps.c'] if MISSION=='lm' else ['-include','src/cm_interp_names.h','src/cm_interp_orders.c','src/cm_interp_steps.c']
 subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Isrc','-Itests','tools/lmi_body_native.c','tests/interp_body_run.c',*bodies_src,'src/agc_image_ops.c','src/agc_hardware_divide.c','src/agc_multiword.c','src/agc_word.c','-o',native],cwd=R,check=True)
 g=Gen(random.Random(a.seed),listing());bodies=[int(x) for x in a.bodies.split(',')]
 # One body at a time keeps memory to one body's cases.
 bad={};total=0
 for b in bodies:
  cases=[case(g,b) for _ in range(a.cases)];total+=len(cases)
  blob=b''.join(struct.pack('<2064H',*(h+i)) for h,i in cases)
  if a.dump:pathlib.Path(a.dump+'.%d'%b).write_bytes(blob)
  ref=subprocess.run(['build/lmi-body-reference',str(ROM)],cwd=R,input=blob,capture_output=True)
  if ref.returncode:print(ref.stderr.decode()[-2000:]);raise SystemExit('FAIL reference exit %d'%ref.returncode)
  nat=subprocess.run([native,str(ROM)],cwd=R,input=blob,capture_output=True,check=True).stdout
  for i,(h,img) in enumerate(cases):
   x=struct.unpack_from('<2049H',ref.stdout,i*4098);y=struct.unpack_from('<2049H',nat,i*4098)
   if x!=y:
    n=NAMES[h[0]];bad[n]=bad.get(n,0)+1
    if a.dump:pathlib.Path(a.dump+'.fail%d'%bad[n]).write_bytes(blob[i*4128:(i+1)*4128])
    if bad[n]<=3:print('FAIL',n,'case',i,'args %o %o'%(h[7],h[8]),'stop',x[0],y[0],[('%04o'%(j-1),'%06o'%x[j],'%06o'%y[j]) for j in range(1,2049) if x[j]!=y[j]][:10])
 if bad:print('FAIL counts',bad);raise SystemExit(1)
 print('PASS',total,'body images equal yaAGC:',', '.join(NAMES[b] for b in bodies))
 if a.proof:
  import json
  pathlib.Path(a.proof).write_text(json.dumps({'mission':MISSION,'seed':a.seed,'cases_per_body':a.cases,'cases':total,
   'bodies':[NAMES[b] for b in bodies],'result':'all body images and stop addresses equal yaAGC'},indent=1)+'\n')
 if a.vectors:vectors(a,g,bodies,native)
def digest(words):
 # Two 16-bit running sums over the stop tag and the 2048-word image.
 s1=s2=0
 for w in words:s1=(s1+w)&0xffff;s2=(s2+s1)&0xffff
 return s2<<16|s1
def vectors(a,g,bodies,native):
 global FILL
 # Their own generator, so the vectors do not depend on --cases.
 g=Gen(random.Random(a.seed+1),g.rows)
 cases=[];seeds=[]
 for b in bodies:
  for j in range(a.vector_cases):
   seed=(b*a.vector_cases+j)*40503+1
   # (The C64 has no rope, so a fixed-memory MXV/VXM matrix is redrawn.)
   while True:
    FILL=xs_fill(seed);c=case(g,b)
    if not c[0][15]&2:break
   cases.append(c);seeds.append((seed,list(FILL)))
 FILL=None
 blob=b''.join(struct.pack('<2064H',*(h+i)) for h,i in cases)
 ref=subprocess.run(['build/lmi-body-reference',str(ROM)],cwd=R,input=blob,capture_output=True,check=True).stdout
 nat=subprocess.run([native,str(ROM)],cwd=R,input=blob,capture_output=True,check=True).stdout
 if ref!=nat:raise SystemExit('FAIL vector cases differ from yaAGC')
 rows=[];edits=[]
 for i,((h,img),(seed,fill)) in enumerate(zip(cases,seeds)):
  out=struct.unpack_from('<2049H',ref,i*4098)
  e=[(k,img[k]) for k in range(2048) if img[k]!=fill[k]]
  rows.append((h,seed,len(edits),len(e),digest(out)));edits+=e
 L=['/* Generated by tools/lmi_body_differential.py --mission %s --vectors: do not edit. */'%MISSION,
  '/* Each case: image = xorshift16 fill from `seed` plus its edits; header words; */',
  '/* digest of the stop tag and the image after the original program ran it (yaAGC). */',
  '/* Define INTERP_BODY_LO/HI to keep only the cases of bodies LO..HI-1. */',
  '#include <stdint.h>','#ifndef INTERP_BODY_LO','#define INTERP_BODY_LO 0u','#define INTERP_BODY_HI 128u','#endif']
 def guarded(name,decl,item):
  L.append(decl+'={')
  for k,r in enumerate(rows):
   L.append('#if %d>=INTERP_BODY_LO&&%d<INTERP_BODY_HI'%(r[0][0],r[0][0]));L.append(item(k,r));L.append('#endif')
  L.append('};')
 L.append('#define INTERP_VECTOR_CASES ((uint16_t)(sizeof interp_vector_seed/sizeof interp_vector_seed[0]))')
 guarded('header','static const uint16_t interp_vector_header[][16]',lambda k,r:' {%s},'%','.join('0%o'%x if x else '0' for x in r[0]))
 guarded('seed','static const uint16_t interp_vector_seed[]',lambda k,r:' %d,'%(r[1]&0xffff))
 guarded('count','static const uint16_t interp_vector_count[]',lambda k,r:' %d,'%r[3])
 guarded('digest','static const uint32_t interp_vector_digest[]',lambda k,r:' 0x%08xu,'%r[4])
 guarded('edit','static const uint16_t interp_vector_edit[][2]',lambda k,r:' '+''.join('{0%o,0%o},'%(x,v) for x,v in edits[r[2]:r[2]+r[3]]))
 pathlib.Path(a.vectors).write_text('\n'.join(L)+'\n')
 print('PASS',len(rows),'C64 vector cases equal yaAGC;',len(edits),'edits written to',a.vectors)
main()
