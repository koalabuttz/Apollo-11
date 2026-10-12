#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
TVCRESTARTS. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CS, AD, MASK
on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_tvc_restarts.c, and the native
TVCCADR/TVCEXADR table must equal rope words 3257-3271. Writes
evidence/cm-tvc-restarts/cm-tvc-restarts-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/TVCRESTARTS.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_tvc_restarts.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o16,a)
listing=(FIX/'assembly.lst').read_text().split('$TVCRESTARTS.agc',1)[1]
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
 if op in('CAF','CS','AD','MASK') and opnd and not (prev and prev['op']=='INDEX'):
  a=inst&0o7777 if op=='CAF' else inst&0o7777
  if a>=0o2000:consts[(addr,opnd)]=fixed(a)
 native=max((l for l in labels if l<=addr),default=None)
 out.append({'source_line':int(m[2]),'bank':m[3],'address':m[4],'raw_octal':m[5]+(' '+m[6] if m[6] else ''),'label':lab,'op':op,'operand':opnd,'native_label':f'L{native:04o}' if native is not None else None})
words=sum(2 if ' ' in w['raw_octal'] else 1 for w in out)
assert words==0o3272-0o3140,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
table=[int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(r'table\[\] = \{(.*?)\};',NATIVE,re.S)[1])]
assert table==[fixed(a) for a in range(0o3257,0o3272)],'TVCCADR/TVCEXADR table'
x={'source_file':'Comanche055/TVCRESTARTS.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_tvc_restarts.c','include/cm_tvc_restarts.h'],
 'entries':['3140 REDOTVC (T5 via T5LOC)','3174 ENABL2 (T5)','3205 CMDSOUT (T5)','3232 CHKSTRK (IBNKCALL SWICHOVR +5 return)','3243 FINCOPY +3 (ISWCALL copycycle return)','3253 EXRSTRT (waitlist task)'],
 'frontiers':{'WAITLIST 5245':'Q 3150 (2CADR EXRSTRT)','RESUME 5333':'ENABL1','NOQRSM 5335':'ENABL2','POSTJUMP 4701':'Q 3227 (MRCLEAN) or 3237 (TVCINIT4)','IBNKCALL 4740':'Q 3231 (SWICHOVR +5)','ISWCALL 4744':'Q 3243, A the copycycle CADR','TVCEXECUTIVE 3003-3132':'EXRSTRT restart points'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'3257-3271 (TVCCADR, TVCEXADR) equal native table[]',
 'reachability':{'TS overflow skips':'unreachable: every TS stores a CAF constant or -0 plus a 15-bit word',
  '3154 CCS A -0':'unreachable (A is TVCPHASE masked with 040001, never -0)','3224 CCS A negative':'unreachable (A is the diminished absolute of TVCPHASE)',
  'FINCOPY index past 3271 and EXRSTRT TVCEXPHS outside 1..6':'refused at entry (OUT_OF_SCOPE)'},
 'words':out}
p=R/'evidence/cm-tvc-restarts/cm-tvc-restarts-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original TVCRESTARTS words mapped; listing equals rope; {len(consts)} fixed constants and the 11-word table equal native literals')
