#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
TVCEXECUTIVE. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, MASK, MP on
fixed memory) is read from the rope and must appear as the same literal,
tagged with the same symbol, in src/cm_tvc_exec.c. Writes
evidence/cm-tvc-exec/cm-tvc-exec-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/TVCEXECUTIVE.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_tvc_exec.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def logical(a):return (2,a) if 0o4000<=a<0o6000 else (3,a) if a>=0o6000 else None
listing=(FIX/'assembly.lst').read_text().split('$TVCEXECUTIVE.agc',1)[1]
labels=sorted(int(x,8) for x in re.findall(r'^L([0-7]{4}):',NATIVE,re.M))
out=[];consts={}
for line in listing.splitlines():
 if re.match(r'^\d+,\d+: \$',line):break
 m=re.match(r'^(\d+),(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?: ([0-7]{5}))?(\s+)(\S+)(?:\s+(\S+))?(?:\s+(\S+))?',line)
 if not m:continue
 bank,addr=int(m[3],8),int(m[4],8)
 for k,raw in enumerate([m[5],m[6]] if m[6] else [m[5]]):
  assert word(bank,addr+k)==int(raw,8),(oct(addr+k),raw)
 lab,op,opnd=(m[8],m[9] or '',m[10] or '') if len(m[7])<12 else ('',m[8],m[9] or '')
 if op in('CAF','MASK','MP') and opnd and not opnd.startswith('TEMPDAP'):
  sym=opnd;inst=int(m[5],8);a=inst&0o7777
  if a>=0o4000:consts[sym]=word(*logical(a))
  elif 0o2000<=a<0o4000:consts[sym]=word(0o16,a)
 native=max((l for l in labels if l<=addr),default=None)
 entry={'source_line':int(m[2]),'bank':m[3],'address':m[4],'raw_octal':m[5]+(' '+m[6] if m[6] else ''),'label':lab,'op':op,'operand':opnd,'native_label':f'L{native:04o}' if native is not None else None}
 out.append(entry)
words=sum(2 if ' ' in w['raw_octal'] else 1 for w in out)
assert words==0o3140-0o2720,words
for sym,v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE) or (sym=='-BIT14' and re.search(r'0%ou\);\s+/\* -BIT14'%v,NATIVE)),(sym,oct(v))
x={'source_file':'Comanche055/TVCEXECUTIVE.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_tvc_exec.c','include/cm_tvc_exec.h','src/cm_tvc_routines.c'],
 'entries':['2720 TVCEXEC','2730 ROLLPREP','2746 NEEDLEUP','2750 VARGAINS','2773','2775','3003 EXECCOPY+1','3011 1SHOTCHK','3040 TEMPSET','3043 CORSETUP','3121 CORCOPY+1','3132 CNTRCOPY'],
 'frontiers':{'WAITLIST 5245':'Q 2726 (2CADR TVCEXEC) or 2744 (2CADR ROLLDAP)','IBNKCALL 4740':'Q 2747 (NEEDLER), 2772 (FIXCW), 2774 (S40.15)','TASKOVER 5324':'TVCEXFIN'},
 'fixed_constants_checked':{k:f'{v:05o}' for k,v in sorted(consts.items())},
 'reachability':{'TS overflow skips other than 2777 and 3051':'unreachable (A is a constant, CCS result, load, MSU or MP result); translated anyway',
  'CCS negative/-0 at 2753 and 3034':'unreachable (A is RAND of BIT13)','2763 TCF +0':'VCNTR negative: the source spins; native NO_EXIT',
  '3135 TS overflow into FCORFRAC data':'unreachable (A is +0 or +-1); native FAULT'},
 'words':out}
p=R/'evidence/cm-tvc-exec/cm-tvc-exec-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original TVCEXECUTIVE words mapped; listing equals rope; {len(consts)} fixed constants equal native literals')
