#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
AUTOMATIC_MANEUVERS. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CA, CS, AD, MASK,
MP on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_auto_man.c; the internal TC
and the frontier TCs must set Q to their returns, and the DCA/DCS, 2CADR and
INDEX tables must equal the rope. Writes
evidence/cm-auto-man/cm-auto-man-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/AUTOMATIC_MANEUVERS.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_auto_man.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o21,a)
listing=(FIX/'assembly.lst').read_text().split('$AUTOMATIC_MANEUVERS.agc',1)[1]
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
assert words==0o3765-0o3250,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
inside={w['label'] for w in out if w['label']}
calls=[(int(w['address'],8)+1,w['operand']) for w in out if w['op'] in('TC','TCR') and w['operand']!='Q']
internal=sorted(a for a,t in calls if t.split()[0] in inside)
external=sorted(a for a,t in calls if t.split()[0] not in inside)
tcs=sorted(int(x,8) for x in re.findall(r'ai_tc\(m, 0([0-7]{4})u\)',NATIVE))
assert tcs==internal,('internal TC returns',[oct(x) for x in tcs],[oct(x) for x in internal])
rets=sorted(int(x,8) for x in re.findall(r'leave\(m, \w+, 0([0-7]{4})u\)',NATIVE))
assert rets==external,('frontier returns',[oct(x) for x in rets],[oct(x) for x in external])
def arr(name):return [int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(name+r'\[[0-9]*\] = \{(.*?)\};',NATIVE,re.S)[1])]
def ropew(lo,hi):return [fixed(a) for a in range(lo,hi+1)]
assert arr('wlh')==ropew(0o3432,0o3433),'WLH'
assert arr('wlmh')==ropew(0o3434,0o3435),'WLMH'
assert arr(' wl')==ropew(0o3436,0o3437),'WL'
assert arr('jetaddr')==ropew(0o3763,0o3764),'JETADDR 2CADR'
assert arr('mintau')==ropew(0o3316,0o3321),'MINTAU INDEX table'
assert arr('octs')==ropew(0o3032,0o3035),'OCT01400 INDEX table'
assert arr('bits')==ropew(0o5002,0o5005),'BIT13 INDEX table'
x={'source_file':'Comanche055/AUTOMATIC_MANEUVERS.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_auto_man.c','include/cm_auto_man.h'],
 'entries':['3250 AHFNOROT, 3441 JETS, 3757 T6PROG (TCF from RCS-CSM_DIGITAL_AUTOPILOT)','3304 STICKCHK return','3732, 3734 SMALLMP returns'],
 'frontiers':{'RESUME 5333':'T6PROG','REINIT 2534, T6PROGM 3067, STICKCHK 3130':'RCS-CSM_DIGITAL_AUTOPILOT','SMALLMP 2042':'MYSUBS calls in JTIME'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'WLH, WLMH, WL double words and the JETADDR 2CADR; INDEX tables MINTAU (3316-3321), OCT01760-OCT00300 (3032-3035), BIT14-BIT11 (5002-5005); internal TC J6. and frontier TCs set Q to their returns',
 'reachability':{'TS T5TEMP at 3443':'overflows when ADB is within 3 of POSMAX; the skip runs JLOOP with SPNDX = +1 (sampled)',
  'CCS SPNDX negative slots at 3750 (ZEROCMDS)':'unreachable: SPNDX is 2, 1, 0 or the +1 of that skip; translated',
  'other TS overflow skips':'unreachable: they store loads, masks, MP or MSU results or loop counters',
  'CCS A after MASK at 3257':'only + and +0 occur'},
 'words':out}
p=R/'evidence/cm-auto-man/cm-auto-man-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original AUTOMATIC_MANEUVERS words mapped; listing equals rope; {len(consts)} fixed constants, double-word, 2CADR and INDEX tables equal native literals; TC returns equal the listing')
