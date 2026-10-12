#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
TVCSTROKETEST. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CS, AD, MASK
on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_tvc_stroke.c, and the native
FCARD table must equal rope words 3524-3532. Writes
evidence/cm-tvc-stroke/cm-tvc-stroke-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/TVCSTROKETEST.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_tvc_stroke.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o20,a)
listing=(FIX/'assembly.lst').read_text().split('$TVCSTROKETEST.agc',1)[1]
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
assert words==0o3612-0o3501,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
table=[int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(r'fcard\[\] = \{(.*?)\};',NATIVE,re.S)[1])]
assert table==[fixed(a) for a in range(0o3524,0o3533)],'FCARD table'
x={'source_file':'Comanche055/TVCSTROKETEST.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_tvc_stroke.c','include/cm_tvc_stroke.h'],
 'entries':['3501 STRKTSTI (V68 job, EXTENDED VERBS 2CADR)','3533 HACK (TC HACK from the pitch and yaw TVC DAPs)','3541 (WAITLIST return past the 2CADR)','3542 HACKWLST (waitlist task)'],
 'frontiers':{'ENDOFJOB 5217':'STRKCHK, interrupts inhibited','WAITLIST 5245':'Q 3537 (2CADR HACKWLST), A 20MS','BUNKER 1657':'TC BUNKER with Q 3556, 3575, 3577 or 3612; BUNKER holds the DAP return (QXCH) or TC TASKOVER'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'3524-3532 (FCARD..FCARD6) equal native fcard[]',
 'reachability':{'TS overflow skips other than 3606':'unreachable: they store constants, CCS results, a negated constant or a loaded word; 3606 (CADDY after DOUBLE) is taken',
  'TSTINIT 3512 as an entry':'only reached by TCR from STRKTSTI','CARD outside -4..-1 at HACK/HACKWLST':'refused at entry (OUT_OF_SCOPE): only TSTINIT and the waveform write CARD'},
 'words':out}
p=R/'evidence/cm-tvc-stroke/cm-tvc-stroke-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original TVCSTROKETEST words mapped; listing equals rope; {len(consts)} fixed constants and the 7-word FCARD table equal native literals')
