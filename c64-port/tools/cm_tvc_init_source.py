#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
TVCINITIALIZE. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CA, CS, AD, MASK,
MP on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_tvc_init.c, and the native coefficient, 2CADR and INDEX
tables must equal the rope. Writes
evidence/cm-tvc-init/cm-tvc-init-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/TVCINITIALIZE.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_tvc_init.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o17,a)
listing=(FIX/'assembly.lst').read_text().split('$TVCINITIALIZE.agc',1)[1]
labels=sorted(int(x,8) for x in re.findall(r'^L([0-7]{4}):',NATIVE,re.M))
out=[];consts={}
for line in listing.splitlines():
 if re.match(r'^\d+,\d+: \$',line):break
 m=re.match(r'^(\d+),(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?: ([0-7]{5}))?(\s+)(\S+)(?:\s+(\S+))?(?:\s+(\S+))?(?:\s+(\+\d+))?',line)
 if not m:continue
 bank,addr=int(m[3],8),int(m[4],8)
 for k,raw in enumerate([m[5],m[6]] if m[6] else [m[5]]):
  assert word(bank,addr+k)==int(raw,8),(oct(addr+k),raw)
 lab,op,opnd=(m[8],m[9] or '',m[10] or '') if len(m[7])<12 else ('',m[8],m[9] or '')
 off=m[11] if len(m[7])<12 else m[10]
 if op.startswith('+'):lab,op,opnd,off=op,opnd,(m[10] if len(m[7])>=12 else m[11]) or '',None
 if off and off.startswith('+'):opnd+=' '+off
 inst=int(m[5],8);prev=out[-1] if out else None
 if op in('CAF','CS','AD','MASK','CA','MP') and opnd and not (prev and prev['op']=='INDEX'):
  a=inst&0o7777 if op=='CAF' else inst&0o7777
  if a>=0o2000:consts[(addr,opnd)]=fixed(a)
 native=max((l for l in labels if l<=addr),default=None)
 out.append({'source_line':int(m[2]),'bank':m[3],'address':m[4],'raw_octal':m[5]+(' '+m[6] if m[6] else ''),'label':lab,'op':op,'operand':opnd,'native_label':f'L{native:04o}' if native is not None else None})
words=sum(2 if ' ' in w['raw_octal'] else 1 for w in out)
assert words==0o2552-0o2147,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
def arr(name):return [int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(name+r'\[[0-9]*\] = \{(.*?)\};',NATIVE,re.S)[1])]
assert arr('coef')==[fixed(a) for a in range(0o2500,0o2531)],'CSMN10..D32 table'
assert arr('initloc2')==[fixed(0o2550),fixed(0o2551)] and arr('dapinit5')==[fixed(0o2546),fixed(0o2547)],'2CADRs'
assert re.search(r'ai_caf\(m, 0%ou\);\s+ai_adf\(m, 0%ou\);'%(fixed(0o7777),fixed(0o7657)),NATIVE),'CSMN10 data run as CA 7777, AD 7657'
assert [int(x,8) for x in re.findall(r'\? 0?([0-7]+)u : 0?([0-7]+)u\);\s+/\* INDEX CNTR; CSMCFADR',NATIVE)[0]]==[fixed(0o2532),fixed(0o2531)]
assert [int(x,8) for x in re.findall(r'\? 0?([0-7]+)u : 0?([0-7]+)u\);\s+/\* INDEX CNTR; TCORR',NATIVE)[0]]==[fixed(0o2541),fixed(0o2540)]
assert [int(x,8) for x in re.findall(r'\? 0?([0-7]+)u : 0?([0-7]+)u\);\s+/\* INDEX A; BIT2',NATIVE)[0]]==[fixed(0o5015),fixed(0o5016)]
x={'source_file':'Comanche055/TVCINITIALIZE.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_tvc_init.c','include/cm_tvc_init.h'],
 'entries':['2147 TVCDAPON (T5 via DOTVCON, P40-P47)','2152 MRCLEAN (POSTJUMP from TVCRESTARTS PHSCHK2)','2171 TVCINIT1 (T5 via INITLOC2)','2176 (IBNKCALL MASSPROP return)','2324 TVCINIT4 (IBNKCALL NEEDLER return; POSTJUMP from TVCRESTARTS CHKSTRK)','2343 (WAITLIST return past 2CADR TVCEXEC)','2351 PRESWTCH (V46, EXTENDED VERBS)','2361 SWICHOVR +5 (IBNKCALL from TVCRESTARTS)','2462 S40.15 (IBNKCALL from TVCEXECUTIVE)'],
 'frontiers':{'RESUME 5333':'ENDMRC, ENDTVCIN','IBNKCALL 4740':'Q 2175 (MASSPROP) or 2323 (NEEDLER)','WAITLIST 5245':'Q 2341 (2CADR TVCEXEC), A .5SEC','TC RTRNLOC 1613':'Q 2422; RTRNLOC holds the return QXCH saved','caller return':'S40.15 entry: its TC Q runs the TC held in the caller\'s Q','TC 1703':'TS VARK overflow skips TC Q into CSMN10 data (CA 7777, AD 7657, TC 1703)'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'2500-2530 (CSMN10..D32) equal native coef[]; 2546-2551 2CADRs; INDEX tables CSMCFADR/HBCFADR, TCORR, BIT2/BIT3; fixed-fixed 7777 and 7657',
 'reachability':{'TS overflow skips other than MASSTMP, 1/CONACC and VARK':'unreachable: they store constants, loads, CCS or MP results; those three are taken',
  'CCS negative/-0 slots on the MRCLEAN, ATTINIT and MCLEANJR counters':'unreachable: the counters are set by the file from NZERO, NZEROJR and BIT1 and only count down to +0',
  'TVCINIT2, LOADCOEF, S40.15 +7, SWICHOVR':'reached only from inside the file (TCR/TC with the file\'s own Q)'},
 'words':out}
p=R/'evidence/cm-tvc-init/cm-tvc-init-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original TVCINITIALIZE words mapped; listing equals rope; {len(consts)} fixed constants, the coefficient and 2CADR tables equal native literals')
