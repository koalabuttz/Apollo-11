#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P30 and S30.1 transfer routines (src/lm_p30_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of P30_P37.agc (bank 35
2000-2026, bank 34 2000-2056) must then be owned by exactly one registered
routine entered at the first address of its range. Interpretive words must
appear as literals, in source order, in that routine; basic words belong to
the routine whose ENTRY names that address; the display constants (V06N33,
V06N42) belong to the routine whose CAF reads them; the destinations the
basic code names must equal their native macros in the fresh assembly; and
the fixed constants it reads (V06N33, V06N81, V06N42) must equal their
native literals in the fresh rope. Every routine must own at least one
source line. Writes build/lm-p30-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P30_P37.agc':'P30,P37.agc'}
root=R/'build/lm-p30-source';assemble(mission,root);work=root/mission
includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')]
aliases={};texts={};originals={}
for name,alias in FILES.items():
 path=mission+'/'+name;original=R.parent/path
 if alias not in includes:raise SystemExit('FAIL no include '+alias)
 shutil.copyfile(original,work/alias);aliases[alias]=path;texts[path]=original.read_text();originals[path]=original
shutil.copyfile(R/'deps/lm-rtb-fixture/INTERPRETER.agc',work/'INTERPRETER.agc')
with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes()
if rope!=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes():raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
records=listing_records(work/'assembly.lst',aliases,texts,rope)
for path,text in texts.items():
 if {r['line']for r in records if r['path']==path}!={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}:raise SystemExit('FAIL listing does not cover every source line of '+path)
# (bank, first, last, routine, kind) ranges: kind 1 interpretive, 0 basic,
# 2 a display constant owned by the routine whose CAF reads it. Words a
# basic call carries (ADRES) belong to its caller.
OWNERS=[('35',0o2000,0o2001,'p30',0),('35',0o2002,0o2003,'updatflg_ret',0),('35',0o2004,0o2005,'p30n33',0),('35',0o2006,0o2007,'v06n81',0),
 ('35',0o2010,0o2011,'downflag',0),('35',0o2012,0o2012,'downflag_ret',0),('35',0o2013,0o2014,'call_s301',1),('35',0o2015,0o2016,'s301_ret',1),
 ('35',0o2017,0o2020,'param30',0),('35',0o2021,0o2021,'param30_ret',0),('35',0o2022,0o2024,'setgo',1),
 ('35',0o2025,0o2025,'p30n33',2),('35',0o2026,0o2026,'param30',2),
 ('34',0o2000,0o2004,'s301',1),('34',0o2005,0o2042,'lemprec_ret',1),('34',0o2043,0o2044,'periapo_ret',1),('34',0o2045,0o2046,'shiftr1_ret',1),
 ('34',0o2047,0o2052,'maxchk_ret',1),('34',0o2053,0o2054,'shiftr1_ret2',1),('34',0o2055,0o2056,'maxchk_ret2',1)]
HELPERS={}
DISPLAY={'p30n33':'K_V06N33','param30':'K_V06N42'}
NAMED={'UPFLAG':'UPFLAG','DOWNFLAG':'DOWNFLAG','VNPOOH':'VNPOOH','INTPRET':'INTPRET'}
CONSTANTS={'K_V06N33':('V06N33',0),'K_V06N81':('V06N81',0),'K_V06N42':('V06N42',0)}
native=(R/'src/lm_p30_routines.c').read_text()
def body(name):
 m=re.search(r'^(?:INTERP\('+re.escape(name)+r',FB3[45],(0[0-7]+)u\)|ROUTINE\('+re.escape(name)+r'\)\{|static (?:void|enum exec_status) '+re.escape(name)+r'\([^)]*\)\{)',native,re.M)
 if not m:raise SystemExit('FAIL no native routine '+name)
 # INTERP opens its brace inside the macro; the others at the match end.
 depth=1;i=m.end()
 while depth:
  depth+=native[i]=='{';depth-=native[i]=='}';i+=1
 return (int(m.group(1),8)if m.group(1)else None),native[m.start():i]
def literals(text):return{int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',text)}
def table(name):return re.search(r'static const agc_routine_entry '+name+r'\[\]=\{(.*?)\};',native,re.S).group(1)
registered=dict(((b,int(a,8)),f)for b,a,f in re.findall(r'LM_P30_B(3[45])\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 owner=[(lo,o,i)for b,lo,hi,o,i in OWNERS if b==bank and lo<=a<=hi]
 if not owner:continue
 if len(owner)!=1:raise SystemExit(f'FAIL address {bank},{addr} has no single native owner')
 lo,name,interp=owner[0];entry,text=body(name)
 text+=''.join(body(h)[1]for h in HELPERS.get(name,[]))
 if interp==2:
  m=re.search(r'#define '+DISPLAY[name]+r' (0[0-7]*)u',native)
  if DISPLAY[name]not in text or not m or r['words']!=[int(m.group(1),8)]:raise SystemExit(f'FAIL {bank},{addr} is not the constant {name} reads')
 else:
  if registered.get((bank,lo))!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at {bank},{lo:o}')
  if interp:
   if entry!=lo:raise SystemExit(f'FAIL {name} is not keyed at {bank},{lo:o}')
   for w in r['words']:
    if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
  elif not re.search(r'ENTRY\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} does not enter at {bank},{lo:o}')
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
# Interpretive words must also appear in source order: each routine's
# assembled words are a subsequence of its literals as written.
for b,lo,hi,name,interp in OWNERS:
 if interp!=1:continue
 seq=[int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',body(name)[1]+''.join(body(h)[1]for h in HELPERS.get(name,[])))];i=0
 for r in rows:
  if r['native']!=name:continue
  for w in r['words']:
   w=int(w,8)
   while i<len(seq)and seq[i]!=w:i+=1
   if i==len(seq):raise SystemExit(f"FAIL {name}: word {w:05o} (line {r['line']}) is out of source order in the native routine")
   i+=1
s=symbols((work/'assembly.lst').read_text())
def physical(text):
 if ','in text:b,o=text.split(',');return int(b,8)*1024+int(o,8)-0o2000
 a=int(text,8);return (a>>10)*1024+(a&0o1777)
for macro,name in NAMED.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native);t=s[name];off=int(t.split(',')[-1],8)
 if not m or int(m.group(1),8)!=off or (','in t and t.split(',')[0]not in('35',)):raise SystemExit(f'FAIL {name} ({t}) differs from native {macro}')
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('P30','P30N33','PARAM30','S30.1','LEMPREC','PERIAPO1','SHIFTR1','MAXCHK','REVN1645','QTEMP','TIG','RATT','VATT','RTX1','RTX2','RTIG','VTIG','DELVSIN','DELVSLV','DELVSAB','HPER','HAPO','UPDATFLG','TRACKFLG','XDELVFLG',*NAMED.values(),'V06N33','V06N81','V06N42')
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p30_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,2000-35,2026','34,2000-34,2056'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of P30_P37.agc (P30 and S30.1) is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, V06N33 and V06N42 belong to the routines whose CAF reads them, the destinations the basic code names equal their native macros, and V06N33, V06N81 and V06N42 equal their native literals.')
(R/'build/lm-p30-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P30_P37 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
