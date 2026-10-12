#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
JET_SELECTION_LOGIC. Every emitted original word is read from the yaYUL listing of
the unchanged source, checked against the assembled rope bytes and assigned
to its native label; every fixed constant the file reads (CAF, CA, CS, AD, MASK,
MP on fixed memory) is read from the rope and must appear as the same literal,
tagged with the same operand, in src/cm_jet_select.c; the file's only
internal calls must be the three INDEX'd TC REPLACER words whose returns the
native T6START loop computes, it must make no external TC, and the 2CADR and
INDEX tables must equal the rope. Writes
evidence/cm-jet-select/cm-jet-select-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/JET_SELECTION_LOGIC.agc';FIX=R/'deps/cm-flight-fixture';NATIVE=(R/'src/cm_jet_select.c').read_text()
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();W=struct.unpack('>%dH'%(len(rope)//2),rope)
def word(bank,addr):
 off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
def fixed(a):return word(2,a) if 0o4000<=a<0o6000 else word(3,a) if a>=0o6000 else word(0o17,a)
listing=(FIX/'assembly.lst').read_text().split('$JET_SELECTION_LOGIC.agc',1)[1]
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
assert words==0o3737-0o2552,words
for (addr,sym),v in consts.items():
 pat=r'0?%ou\);\s+/\* (?:[A-Z0-9/]+: )?%s \*/'%(v,re.escape(sym))
 assert re.search(pat,NATIVE),(oct(addr),sym,oct(v))
inside={w['label'] for w in out if w['label']}
calls=[(int(w['address'],8)+1,w['operand']) for w in out if w['op'] in('TC','TCR') and w['operand']!='Q']
internal=sorted(a for a,t in calls if t.split()[0] in inside)
external=sorted(a for a,t in calls if t.split()[0] not in inside)
assert internal==[0o3663+5*r for r in range(3)],('REPLACER returns',[oct(x) for x in internal])
assert '(agc_word)(03663u + 5u * r)' in NATIVE and 'for (r = 0; r < 3u; ++r)' in NATIVE,'T6START REPLACER returns'
assert external==[],('external TCs',external)
def arr(name):return [int(x,8) for x in re.findall(r'0([0-7]+)u',re.search(r'\b'+name+r'\[[0-9]*\] = \{(.*?)\};',NATIVE,re.S)[1])]
def ropew(lo,hi):return [fixed(a) for a in range(lo,hi+1)]
assert arr('rcs2cadr')==ropew(0o3575,0o3576),'RCS2CADR 2CADR'
assert arr('xlnndx')==ropew(0o2660,0o2663),'XLNNDX INDEX table'
assert arr('pytable')==ropew(0o2714,0o2732),'PYTABLE INDEX table'
assert arr('xln1ndx')==ropew(0o2771,0o2774),'XLN1NDX INDEX table'
assert arr('rtable')==ropew(0o3130,0o3146),'RTABLE INDEX table'
assert arr('yztable')==ropew(0o3151,0o3161),'YZTABLE INDEX table'
assert arr('njet')==ropew(0o3350,0o3356),'NJET-3..NJET+3 INDEX table'
assert arr('dftmax')==ropew(0o3304,0o3312),'DFTMAX-3..DFTMAX+3 INDEX table'
x={'source_file':'Comanche055/JET_SELECTION_LOGIC.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':words,'native_files':['src/cm_jet_select.c','include/cm_jet_select.h'],
 'entries':['2552 JETSLECT (T5RUPT via T5LOC; XTRANS -4..+4)','3617 T6START (T6RUPT via T6ADDR; BLAST+1, BLAST1+1, BLAST2+1 each 0, 4 or 11)'],
 'frontiers':{'RESUME 5333':'the only exit of both tasks (ENDJETS, TCF RESUME in T6START, T6RUPTOR and its three restarts)'},
 'fixed_constants_checked':{f'{a:04o} {s}':f'{v:05o}' for (a,s),v in sorted(consts.items())},
 'table_checked':'RCS2CADR (3575-3576); INDEX tables XLNNDX (2660-2663), PYTABLE (2714-2732), XLN1NDX (2771-2774), RTABLE (3130-3146), YZTABLE (3151-3161), NJET-3..NJET+3 (3350-3356), DFTMAX-3..DFTMAX+3 (3304-3312); internal TC returns 3663, 3670, 3675 equal the native REPLACER loop; no external TC',
 'reachability':{'TS overflow skips other than TS TIME5 at 2555':'unreachable: the others store masks, MP results, table words, CCS results, small jet counts or times; TS TIME5 (DELTATT3 + T5TIME) is taken',
  'INDEX ranges':'XNDX1, XNDX2, YNDX, ZNDX are 2-bit fields of channel 31 or CCS XTRANS results (0..3); PINDEX, YINDEX, RINDEX 0..2 with the 0/3/6/9/12 bases keep PYTABLE and RTABLE indices in 0..14 and YZTABLE in 0..8; NRJETS, NPJETS, NYJETS are table jet counts (-3..+3); BLASTn+1 are the T6SETUP offsets 0, 4, 11 (admitted at T6START)',
  'XTRANS -0':'refused at JETSLECT: the admitted range is -4..+4'},
 'words':out}
p=R/'evidence/cm-jet-select/cm-jet-select-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {words} original JET_SELECTION_LOGIC words mapped; listing equals rope; {len(consts)} fixed constants, 2CADR and INDEX tables equal native literals; REPLACER returns equal the native loop; no external TC')
