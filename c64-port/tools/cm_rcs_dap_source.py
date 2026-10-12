#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
RCS-CSM_DIGITAL_AUTOPILOT. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CA, CS, AD, MASK,
MP on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_rcs_dap.c; the native TC Q
dispatch must name exactly the returns of the file's internal TC/TCR words,
the frontier TCs their returns, and the 2CADR and INDEX tables the rope. Writes
evidence/cm-rcs-dap/cm-rcs-dap-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/RCS-CSM_DIGITAL_AUTOPILOT.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_rcs_dap.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o21,a)
listing=(FIX/'assembly.lst').read_text().split('$RCS-CSM_DIGITAL_AUTOPILOT.agc',1)[1]
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
assert words==0o3250-0o2105,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* %s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
inside={w['label'] for w in out if w['label']}
calls=[(int(w['address'],8)+1,w['operand']) for w in out if w['op'] in('TC','TCR') and w['operand']!='Q']
internal=sorted(a for a,t in calls if t.split()[0] in inside)
external=sorted(a for a,t in calls if t.split()[0] not in inside)
disp=sorted(int(x,8) for x in re.findall(r'case 0([0-7]{4})u: goto L',NATIVE.split('TCQ:',1)[1]))
assert disp==internal,('TC Q dispatch',[oct(x) for x in disp],[oct(x) for x in internal])
rets=sorted(int(x,8) for x in re.findall(r'leave\(m, \w+, 0([0-7]{4})u\)',NATIVE))
assert rets==external,('frontier returns',[oct(x) for x in rets],[oct(x) for x in external])
def arr(name):return [int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(name+r'\[[0-9]*\] = \{(.*?)\};',NATIVE,re.S)[1])]
def ropew(lo,hi):return [fixed(a) for a in range(lo,hi+1)]
assert arr('rcsloc')==ropew(0o2120,0o2121),'RCSLOC 2CADR'
assert arr('t6addr')==ropew(0o2630,0o2631),'T6ADDR 2CADR'
assert arr('gains')==ropew(0o3074,0o3130),'GAIN1/GAIN2 INDEX table'
assert arr('mantable')==ropew(0o3037,0o3050),'MANTABLE INDEX table'
assert arr('freetau')==ropew(0o3063,0o3066),'FREETAU INDEX table'
assert arr('daclimit')==ropew(0o2541,0o2543),'DACLIMIT INDEX table'
x={'source_file':'Comanche055/RCS-CSM_DIGITAL_AUTOPILOT.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_rcs_dap.c','include/cm_rcs_dap.h'],
 'entries':['2105 REDORCS (restart table)','2122 RCSATT (T5RUPT via T5LOC = RCSLOC, RCSADDR, RCS2CADR)',
  '2236 SMALLMP return, 2401 DPADD return, 2360/2616 NOVAC returns, 2550/2552 IBNKCALL returns',
  '2534 REINIT, 3067 T6PROGM (TCF from AUTOMATIC_MANEUVERS)',
  '2420 NEEDLER, 2443 NEEDLE11, 2476 NEEDLES, 2632 ZEROJET, 3130 STICKCHK (IBNKCALL or TC; TC Q to the caller)'],
 'frontiers':{'RESUME 5333':'KRESUME1, KRESUME2, REINIT, RCSSWIT, FDAIDSP2, GETAKS','IBNKCALL 4740':'REDAP (S41.2, S40.14)','NOVAC 5134':'KMATRIX, IMUAOK (AMBGUPDT)',
  'SMALLMP 2042, DPADD 2054':'MYSUBS calls in DRHOLOOP and DELOOP','JETS 3441, T6PROG 3757, AHFNOROT 3250':'AUTOMATIC_MANEUVERS','caller Q':'TC Q of the five subroutine entries'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'RCSLOC and T6ADDR 2CADRs; INDEX tables GAIN1-3..GAIN2+11 (3074-3130), MANTABLE-1..=+14MS (3037-3050), FREETAU (3063-3066), DACLIMIT-1..+1 (2541-2543); TC Q dispatch equals the internal TC/TCR returns; frontier TCs equal their returns',
 'reachability':{'TS overflow skips other than TS T5TEMP at 2512, TS TIME5 at 3151 and TS T5TEMP at 3130':'unreachable: they store loads, constants, masks, MP or MSU results or small indices; the three are taken',
  'CCS A slots 2/3 at 2173, 2352, 2605, 2744':'unreachable: A is a mask result (+0 or positive)',
  'CCS SPNDX negative slots':'unreachable: SPNDX is the file\'s own 0..2, 0..11 or 0..36 loop counter (admitted 0..2 at the SMALLMP/DPADD returns)',
  'INDEX ranges':'ATTKALMN -1..11 and RATEINDX 0..6 admitted at the entries that reach GAIN1/GAIN2 and MANTABLE; STICKCHK leaves RMANNDX/PMANNDX/YMANNDX in 0..3'},
 'words':out}
p=R/'evidence/cm-rcs-dap/cm-rcs-dap-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original RCS-CSM_DIGITAL_AUTOPILOT words mapped; listing equals rope; {len(consts)} fixed constants, 2CADR and INDEX tables equal native literals; TC Q dispatch and frontier returns equal the listing')
