#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
TVCROLLDAP. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CA, CS, AD, MASK,
MP on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_tvc_roll.c, and the native 2CADR, LIMITS and data-run
words must equal the rope. Writes
evidence/cm-tvc-roll/cm-tvc-roll-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/TVCROLLDAP.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_tvc_roll.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o16,a)
listing=(FIX/'assembly.lst').read_text().split('$TVCROLLDAP.agc',1)[1]
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
assert words==0o3720-0o3272,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
def arr(name):return [int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(name+r'\[[0-9]*\] = \{(.*?)\};',NATIVE,re.S)[1])]
assert arr('norol1t6')==[fixed(0o3704),fixed(0o3705)],'NOROL1T6 2CADR'
assert re.search(r'ai_adf\(m, 0%ou\);\s+goto L3340'%fixed(0o3342),NATIVE),'DELOGAC skip runs the 3342 word as AD'
assert re.findall(r'\? 0?([0-7]+)u : 0?([0-7]+)u\);',NATIVE)==[(f'{fixed(0o4777):o}',f'{fixed(0o5001):o}')]*2,'INDEX A; CS LIMITS -1/+1'
x={'source_file':'Comanche055/TVCROLLDAP.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_tvc_roll.c','include/cm_tvc_roll.h'],
 'entries':['3272 ROLLDAP (waitlist task, TVCEXECUTIVE 2CADR ROLLDAP)','3676 NOROLL1 (T6RUPT via T6LOC = NOROL1T6)'],
 'frontiers':{'TASKOVER 5324':'WAIT1/2, EXTENTST, SAMEJETS, RDAPEND','NOQRSM 5335':'NOROLL1 after KILLJETS'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'3704-3705 NOROL1T6 equal native norol1t6[]; LIMITS-1/+1 (4777, 5001) for INDEX A; CS LIMITS; 3342 for the DELOGAC skip',
 'reachability':{'TS overflow skips other than TEMREG at 3321, DELOGA at 3335, TEMREG at 3506 and 3520':'unreachable: they store MP high words, loads, constants, CCS results or ±1; the four are taken',
  'TS TEMREG at 3503 skip':'unreachable in the samples (DV quotient, ±POSMAX or ∓MAXLIM); translated (runs SU OGARATE as AD)',
  'CCS +0/-0 slots after BZF at 3467 and 3560':'unreachable: BZF has already taken zero',
  'INDEX I':'I is PRIO30 or BIT15 (set at 3343), so each 0-opcode word runs as CA or CS'},
 'words':out}
p=R/'evidence/cm-tvc-roll/cm-tvc-roll-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original TVCROLLDAP words mapped; listing equals rope; {len(consts)} fixed constants, the 2CADR and LIMITS words equal native literals')
