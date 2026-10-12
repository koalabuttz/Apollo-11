#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P33/P73, AVFLAGA, AVFLAGP, P20FLGON and DISDVLVC transfer
routines (src/lm_p33_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of P32-P35_P72-P75.agc in
bank 35 2216-2407 must then be owned by exactly one registered routine
entered at the first address of its range. Interpretive words must appear as
literals, in source order, in that routine (with the shared bodies it runs
on into); basic words belong to the routine whose ENTRY names that address;
the erasable words and destinations the basic code names must equal their
native macros in the fresh assembly; and the fixed constants it reads
(V06N13, V05N09, V06N75, the P30ZERO and 60MIN pairs) must equal their
native literals in the fresh rope. Every routine must own at least one
source line. Writes build/lm-p33-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P32-P35_P72-P75.agc':'P32-P35,_P72-P75.agc'}
root=R/'build/lm-p33-source';assemble(mission,root);work=root/mission
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
# (bank, first, last, routine, kind) ranges: kind 1 interpretive, 0 basic.
# Words a basic call carries (ADRES, OCT, CADR) belong to its caller.
OWNERS=[('35',0o2216,0o2216,'p33',0),('35',0o2217,0o2217,'avflaga_ret',0),('35',0o2220,0o2220,'p73',0),('35',0o2221,0o2221,'p33a',0),
 ('35',0o2222,0o2223,'p20_ret',0),('35',0o2224,0o2224,'vnpooh_ret',0),('35',0o2225,0o2232,'p33a_interp',1),('35',0o2233,0o2234,'p33b',1),
 ('35',0o2235,0o2236,'advance_ret',1),('35',0o2237,0o2244,'cdhmvr_ret',1),('35',0o2245,0o2246,'intint_ret',1),('35',0o2247,0o2254,'active_ret',1),
 ('35',0o2255,0o2256,'intint_ret2',1),('35',0o2257,0o2263,'passive_ret',1),('35',0o2264,0o2265,'s331_ret',1),('35',0o2266,0o2267,'r_alarm',0),
 ('35',0o2270,0o2272,'alarm_ret',0),('35',0o2273,0o2273,'flash_terminate',0),('35',0o2274,0o2274,'flash_proceed',0),('35',0o2275,0o2275,'flash_recycle',0),
 ('35',0o2276,0o2276,'r_intpret',0),('35',0o2277,0o2301,'proceed_ret',1),('35',0o2302,0o2305,'p33c',1),('35',0o2306,0o2313,'p33d',1),
 ('35',0o2314,0o2324,'p33e',1),('35',0o2325,0o2334,'p33f',1),('35',0o2335,0o2336,'v06n75',0),('35',0o2337,0o2337,'vnpooh_ret2',0),
 ('35',0o2340,0o2342,'vnpooh_ret_i',1),('35',0o2343,0o2344,'s3233_ret',1),('35',0o2345,0o2346,'loop',1),('35',0o2347,0o2352,'avflaga',0),
 ('35',0o2353,0o2353,'avflaga_up',0),('35',0o2354,0o2357,'avflagp',0),('35',0o2360,0o2360,'avflagp_down',0),('35',0o2361,0o2364,'p20flgon',0),
 ('35',0o2365,0o2366,'p20flgon_up',0),('35',0o2367,0o2367,'p20flgon_up2',0),('35',0o2370,0o2373,'disdvlvc',1),('35',0o2374,0o2402,'s3233x_ret',1),
 ('35',0o2403,0o2404,'disdvlvc_exit',0),('35',0o2405,0o2405,'vnpooh_ret3',0),('35',0o2406,0o2407,'normex_goto',1)]
HELPERS={'p33c':['p33c_body'],'p33d':['p33d_body'],'p33e':['p33e_body'],'p33f':['p33f_body']}
DISPLAY={}
NAMED={'UPFLAG':'UPFLAG','DOWNFLAG':'DOWNFLAG','VNPOOH':'VNPOOH','ALARM':'ALARM','BANKCALL':'BANKCALL','GOTOPOOH':'GOTOPOOH','INTPRET':'INTPRET','SUBEXIT':'SUBEXIT','VERBNOUN':'VERBNOUN'}
CONSTANTS={'K_V06N13':('V06N13',0),'K_V05N09':('V05N09',0),'K_V06N75':('V06N75',0),'K_P30ZERO_HI':('P30ZERO',0),'K_P30ZERO_LO':('P30ZERO',1),'K_60MIN_HI':('60MIN',0),'K_60MIN_LO':('60MIN',1)}
native=(R/'src/lm_p33_routines.c').read_text()
def body(name):
 m=re.search(r'^(?:INTERP\('+re.escape(name)+r',(0[0-7]+)u\)|ROUTINE\('+re.escape(name)+r'\)\{|static (?:void|uint8_t|enum exec_status) '+re.escape(name)+r'\([^)]*\)\{)',native,re.M)
 if not m:raise SystemExit('FAIL no native routine '+name)
 # INTERP opens its brace inside the macro; the others at the match end.
 depth=1;i=m.end()
 while depth:
  depth+=native[i]=='{';depth-=native[i]=='}';i+=1
 return (int(m.group(1),8)if m.group(1)else None),native[m.start():i]
def literals(text):return{int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',text)}
def table(name):return re.search(r'static const agc_routine_entry '+name+r'\[\]=\{(.*?)\};',native,re.S).group(1)
registered=dict(((b,int(a,8)),f)for b,a,f in re.findall(r'LM_P33_B(35)\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
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
 if not m or int(m.group(1),8)!=off or (','in t and t.split(',')[0]not in('35','E7')):raise SystemExit(f'FAIL {name} ({t}) differs from native {macro}')
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('P33','P73','P33/P73A','P33/P73B','P33/P73C','P33/P73D','P33/P73E','P33/P73F','AVFLAGA','AVFLAGP','P20FLGON','DISDVLVC','SELECTMU','ADVANCE','CDHMVR','INTINT3P','ACTIVE','PASSIVE','S33/34.1','S32/33.1','S32/33.X','VN1645','NORMEX','TTPI','TTPI0','TCDH','TIG','NOMTPI','T1TOT2','T2TOT3','DELVLVC','DELVEET2','ITSWICH','FINALFLG','UPDATFLG','AVFLAG','TRACKFLG',*NAMED.values(),'V06N13','V05N09','V06N75','P30ZERO','60MIN')
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p33_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,2216-35,2407'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of P32-P35_P72-P75.agc in 35,2216-2407 (P33/P73, AVFLAGA, AVFLAGP, P20FLGON, DISDVLVC) is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the erasable words and destinations the basic code names equal their native macros, and V06N13, V05N09, V06N75 and the P30ZERO and 60MIN pairs equal their native literals.')
(R/'build/lm-p33-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P32-P35_P72-P75 words (35,2216-2407) on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
