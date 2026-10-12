#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
CM_ENTRY_DIGITAL_AUTOPILOT (bank 15 2434-3754, bank 20 3612-3737). Every
emitted original word is read from the yaYUL listing of the unchanged source,
checked against the assembled rope bytes and assigned to its native label
(Lnnnn in bank 15, L20_nnnn in bank 20); every fixed constant the file reads
(CAF, CA, CS, AD, MASK, MP on fixed memory, not under INDEX) is read from the
rope and must appear as the same literal, tagged with the same operand, in
src/cm_entry_dap.c; the native TC Q dispatch must name exactly the returns of
the TCs into the file's TC Q subroutines (ANGOVCOR, RATEAVG, 3DDZ, BIASEDZ,
2D/SDZ), which hold every TC Q word; the native frontier returns must be
exactly the returns of the file's external TCs; and the 2CADR and INDEX
tables must equal the rope. Writes
evidence/cm-entry-dap/cm-entry-dap-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/CM_ENTRY_DIGITAL_AUTOPILOT.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_entry_dap.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(bank,a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(bank,a)
listing=(FIX/'assembly.lst').read_text().split('$CM_ENTRY_DIGITAL_AUTOPILOT.agc',1)[1].split('End of include-file CM_ENTRY_DIGITAL_AUTOPILOT',1)[0]
labels={15:sorted(int(x,8) for x in re.findall(r'^L([0-7]{4}):',NATIVE,re.M)),
        16:sorted(int(x,8) for x in re.findall(r'^L20_([0-7]{4}):',NATIVE,re.M))}
out=[];consts={}
for line in listing.splitlines():
 m=re.match(r'^(\d+),(\d+): ([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?: ([0-7]{5}))?',line)
 if not m:continue
 bank,addr=int(m[3],8),int(m[4],8)
 for k,raw in enumerate([m[5],m[6]] if m[6] else [m[5]]):
  assert word(bank,addr+k)==int(raw,8),(oct(bank),oct(addr+k),raw)
 lab,op,opnd=line[46:65].strip(),line[65:74].strip(),' '.join(line[74:105].split())
 inst=int(m[5],8);prev=out[-1] if out else None
 if op in('CAF','CS','AD','MASK','CA','MP') and opnd and not (prev and prev['op']=='INDEX'):
  a=inst&0o7777
  if a>=0o2000:consts[(bank,addr,opnd)]=fixed(bank,a)
 key=15 if bank==0o15 else 16
 native=max((l for l in labels[key] if l<=addr),default=None)
 name=None if native is None else (f'L{native:04o}' if key==15 else f'L20_{native:04o}')
 out.append({'source_line':int(m[2]),'bank':m[3],'address':m[4],'raw_octal':m[5]+(' '+m[6] if m[6] else ''),'label':lab,'op':op,'operand':opnd,'native_label':name})
words=sum(2 if ' ' in w['raw_octal'] else 1 for w in out)
assert words==(0o3755-0o2434)+(0o3740-0o3612),words
for (bank,addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* (?:[^*]*?[:;] )?(?:MP )?%s(?:;[^*]*)? \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(bank),oct(addr),sym,oct(v))
inside={(w['bank'],w['label']) for w in out if w['label']}
def target(w):
 t=w['operand'].split()
 if t[0][0] in '+-':return int(w['address'],8)+int(t[0])
 base=next(int(x['address'],8) for x in out if x['bank']==w['bank'] and x['label']==t[0])
 return base+(int(t[1]) if len(t)>1 else 0)
tcs=[w for w in out if w['op']=='TC' and w['operand']!='Q']
internal=[w for w in tcs if w['operand'][0] in '+-' or (w['bank'],w['operand'].split()[0]) in inside]
external=sorted({int(w['address'],8)+1 for w in tcs if w not in internal})
subs={int(x['address'],8) for x in out if x['bank']=='15' and x['label'] in('ANGOVCOR','RATEAVG','3DDZ','BIASEDZ','2D/SDZ')}
returns=sorted(int(w['address'],8)+1 for w in internal if target(w) in subs)
for w in out:
 if w['op']=='TC' and w['operand']=='Q':
  a=int(w['address'],8);assert w['bank']=='15' and (0o2522<=a<=0o2535 or 0o3001<=a<=0o3036),('TC Q outside the subroutines',w['address'])
disp=sorted(int(x,8) for x in re.findall(r'case 0([0-7]{4})u: goto L',NATIVE.split('TCQ:',1)[1].split('/* Bank 20. */',1)[0]))
assert disp==returns,('TC Q dispatch',[oct(x) for x in disp],[oct(x) for x in returns])
rets=sorted({int(x,8) for x in re.findall(r'LEAVE\(\w+, 0([0-7]{4})u\)',NATIVE)}-{0o3715})
assert rets==external,('frontier returns',[oct(x) for x in rets],[oct(x) for x in external])
assert 'LEAVE(02434u, 03715u)' in NATIVE and fixed(0o20,0o3714)==0o2434,'TS CM/GYMDT overflow: 2CADR word runs as TC 2434'
def arr(name):return [int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(r'\b'+name+r'\[[0-9]*\] = \{(.*?)\};',NATIVE,re.S)[1])]
def ropew(bank,lo,hi):return [fixed(bank,a) for a in range(lo,hi+1)]
assert arr('atdotcad')==ropew(0o15,0o2663,0o2664),'ATDOTCAD 2CADR'
assert arr('t5idler1')==ropew(0o20,0o3736,0o3737),'T5IDLER1 2CADR'
assert arr('t5idler2')==ropew(0o15,0o3744,0o3745),'T5IDLER2 2CADR'
assert arr('limits')==ropew(0,0o4777,0o5001),'LIMITS-1..LIMITS+1 INDEX table'
assert arr('halfpr')==ropew(0,0o5000,0o5002),'HALFPR-1..HALFPR+1 INDEX table'
assert arr('yjetcode')==ropew(0o15,0o3201,0o3203),'YJETCODE-1..+1 INDEX table'
assert arr('prjcode')==ropew(0o15,0o3204,0o3206),'P/RJCODE-1..+1 INDEX table'
x={'source_file':'Comanche055/CM_ENTRY_DIGITAL_AUTOPILOT.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_entry_dap.c','include/cm_entry_dap.h'],
 'entries':['bank 15: 2434 READGYMB, 2665 ATTRATES, 3207 SETJTAG, 3650 JETCALL (waitlist, T5 and restart tasks)',
  'bank 15 returns: FLUSHJET 2446, 2476; PHASCHNG 2500, 3056, 3215; WAITLIST 2507, 3064, 3642; SPCOS/SPSIN 2540, 2543, 2551, 2607, 2631, 2641, 2700, 2712, 2724; IBNKCALL NEEDLER 3717',
  'bank 20: 3612 CM/DAPON (POSTJUMP from P62), 3672 CM/DAPIC (BANKCALL from P62)',
  'bank 20 returns: DOWNFLAG 3616, 3620; FLUSHJET 3627; BANKCALL DELAYJOB 3636; WAITLIST 3716; 2PHSCHNG 3732'],
 'frontiers':{'TASKOVER 5324, WAITLIST 5245, PHASCHNG 5412, 2PHSCHNG 5372, FLUSHJET 6000':'Executive/waitlist and restart services',
  'SPSIN 5075, SPCOS 5074':'SINGLE_PRECISION_SUBROUTINES','IBNKCALL 4740':'NEEDLER (RCS-CSM_DIGITAL_AUTOPILOT)','RESUME 5333':'T5 task end',
  'DOWNFLAG 5560, BANKCALL 4662, POSTJUMP 4701':'bank-20 start-up','bank 20 2434':'TS CM/GYMDT overflow skips TC WAITLIST; the 2CADR word 02434 runs as TC 2434'},
 'fixed_constants_checked':{f'{b:02o},{a:04o} {s}':f'{v:05o}' for (b,a,s),v in sorted(consts.items())},
 'table_checked':'ATDOTCAD, T5IDLER1, T5IDLER2 2CADRs; INDEX tables LIMITS-1..+1 (4777-5001), HALFPR-1..+1 (5000-5002), YJETCODE-1..+1 (3201-3203), P/RJCODE-1..+1 (3204-3206); TC Q dispatch equals the returns of the TCs into ANGOVCOR, RATEAVG, 3DDZ, BIASEDZ and 2D/SDZ, which hold every TC Q; frontier returns equal the external TC returns',
 'reachability':{'TS overflow skips not taken in the native code':'unreachable: those TS words store loads, constants, masks, MP, MSU or DV results, CCS results or small counts',
  'CCS JETAG -0 (2775, 3064)':'refused at entry: JETAG is admitted -1..+1, the values the file stores; -0 at 2775 would fall into 3DDZ and TC Q to address 3',
  'CCS CALFA +0/-0 at 3046':'unreachable: TS A overflows only when |CALFA| exceeds cos 45',
  'CCS Q negative at 3431':'unreachable: Q is GETON1\'s return (3415) or ZQ\'s +0',
  'TCF -2 with A = +0 in CM/DAPIC':'unreachable: A = -5 + JETEM and each pass\'s A - 10 are never +0 in one\'s complement',
  'INDEX ranges':'ANGOVCOR and TRTAGXPI index with an overflow sign (+-1); the jet-code tables with dead-zone tags and ROR results (-1, +-0, +1); CA RREL with CMDAPMOD -1 (0) or ONE; CA ROLLTM and TS A with a masked bit; DXCH ENDBUF -1 and TS ENDBUF +1 with SW/NDX, admitted -13..+13'},
 'words':out}
p=R/'evidence/cm-entry-dap/cm-entry-dap-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original CM_ENTRY_DIGITAL_AUTOPILOT words mapped; listing equals rope; {len(consts)} fixed constants, 2CADR and INDEX tables equal native literals; TC Q dispatch and frontier returns equal the listing')
