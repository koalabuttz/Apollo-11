#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
TVCDAPS. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CA, CS, AD, MASK,
MP on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_tvc_daps.c; the native TC Q
dispatch must name exactly the return addresses of the file's TCR words. Writes
evidence/cm-tvc-daps/cm-tvc-daps-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/TVCDAPS.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_tvc_daps.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o20,a)
listing=(FIX/'assembly.lst').read_text().split('$TVCDAPS.agc',1)[1]
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
assert words==0o3501-0o2530,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
tcr=sorted(int(w['address'],8)+1 for w in out if w['op']=='TCR')
disp=sorted(int(x,8) for x in re.findall(r'case 0([0-7]{4})u: goto L',NATIVE.split('TCQ:',1)[1]))
assert disp==tcr,('TC Q dispatch',[oct(x) for x in disp],[oct(x) for x in tcr])
hack=sorted(int(w['address'],8)+1 for w in out if w['op']=='TC' and w['operand']=='HACK')
assert sorted(int(x,8) for x in re.findall(r'leave\(m, HACK, 0([0-7]{4})u\)',NATIVE))==hack,'HACK returns'
x={'source_file':'Comanche055/TVCDAPS.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_tvc_daps.c','include/cm_tvc_daps.h'],
 'entries':['2530 PITCHDAP (T5RUPT via T5LOC = PITCHT5)','2732 YAWDAP (T5RUPT via T5LOC = YAWT5)','3107 DAPINIT (T5RUPT via DAPT5)',
  '2541, 2543, 2743, 2745 (HACK returns)','2675 PCOPY+1, 3051 YCOPY+1 (FINCOPY ISWCALL; TC Q to the caller)'],
 'frontiers':{'HACK 3533':'TC HACK at 2540/2542/2742/2744 (TVCSTROKETEST)','RESUME 5333':'PDAPEND, YDAPEND','NOQRSM 5335':'DAPINIT','caller Q':'TC Q of PCOPY/YCOPY entered at +1'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'native TC Q dispatch equals the return addresses of the file\'s TCR words; HACK returns equal the TC HACK addresses +1',
 'reachability':{'TS overflow skips other than TS L at 3141 and TS TIME5 at 3114':'unreachable: they store MSU or MP results, loads, constants or CCS results; the two are taken',
  'CCS A slots 2/3 at 3212 and 3407':'unreachable: A is DAPDATR1 masked with BIT14 or BIT13 (+0 or the bit)',
  'CCS +0/-0 slots at 3131 and 3150':'unreachable after BZF has taken a zero product; translated (they store +0)'},
 'words':out}
p=R/'evidence/cm-tvc-daps/cm-tvc-daps-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original TVCDAPS words mapped; listing equals rope; {len(consts)} fixed constants equal native literals; TC Q dispatch and HACK returns equal the listing')
