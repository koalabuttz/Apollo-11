#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P34/P74 transfer routines (src/lm_p34p74_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of P34/P74 (bank 35
2545-2724) must then be owned by exactly one registered routine entered at
the first address of its range. Interpretive words must appear as literals,
in source order, in that routine; basic words belong to the routine whose
ENTRY names that address; the erasable words and destinations the basic
code names must equal their native macros in the fresh assembly; and the
fixed constants it reads (V06N37, V06N58, 130DEG, P30ZERO, V05N09) must
equal their native literals in the fresh rope. Every routine must own at
least one source line. Writes build/lm-p34p74-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P34-35_P74-75.agc':'P34-P35,_P74-P75.agc'}
root=R/'build/lm-p34p74-source';assemble(mission,root);work=root/mission
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
# (first, last, routine, interpretive) bank-35 ranges. The CADR GOFLASH
# (2621) belongs to BANKCALL's caller and ALARM's OCT 611 (2616) to its.
OWNERS=[(0o2545,0o2545,'p34',0),(0o2546,0o2546,'avflaga_ret',0),(0o2547,0o2547,'p74',0),(0o2550,0o2550,'r_p34a',0),
 (0o2551,0o2552,'p20_ret',0),(0o2553,0o2560,'vnpooh_ret',0),(0o2561,0o2561,'displaye_ret',0),
 (0o2562,0o2571,'p34_start',1),(0o2572,0o2573,'p34b',1),(0o2574,0o2603,'p34c',1),(0o2604,0o2610,'r_intloop',1),
 (0o2611,0o2612,'call_s331',1),(0o2613,0o2614,'s331_ret',1),(0o2615,0o2616,'r_alarm',0),(0o2617,0o2621,'alarm_ret',0),
 (0o2622,0o2622,'flash_terminate',0),(0o2623,0o2623,'flash_proceed',0),(0o2624,0o2624,'flash_recycle',0),
 (0o2625,0o2632,'swchclr',1),(0o2633,0o2633,'swchclr_exit',0),(0o2634,0o2634,'displaye_ret2',0),(0o2635,0o2635,'p34d',1),
 (0o2636,0o2637,'p34d_exit',0),(0o2640,0o2640,'r_p34e',0),(0o2641,0o2663,'p34e',1),(0o2664,0o2672,'timethet_ret',1),
 (0o2673,0o2705,'s3435_2_ret',1),(0o2706,0o2707,'periapo_ret',1),(0o2710,0o2713,'shiftr1_ret',1),
 (0o2714,0o2715,'shiftr1_exit',0),(0o2716,0o2716,'vnpooh_ret2',0),(0o2717,0o2720,'call_s5',1),(0o2721,0o2722,'call_vn1645',1),
 (0o2723,0o2724,'loop',1)]
# Helpers a routine runs inline (the code its TCs fall into).
HELPERS={'avflaga_ret':['p34a'],'r_p34a':['p34a'],'flash_proceed':['p34a'],'p34c':['intloop'],'r_intloop':['intloop'],
 'r_alarm':['alarm611'],'flash_recycle':['alarm611'],'displaye_ret2':['p34e_intpret'],'r_p34e':['p34e_intpret']}
# Addresses the basic code names: erasable (EB window) and bank-35 or
# fixed-fixed destinations.
NAMED={'CENTANG':'CENTANG','NN':'NN','AVFLAGA':'AVFLAGA','AVFLAGP':'AVFLAGP','P20FLGON':'P20FLGON','DISPLAYE':'DISPLAYE',
 'VNPOOH':'VNPOOH','ALARM':'ALARM','BANKCALL':'BANKCALL','GOTOPOOH':'GOTOPOOH','INTPRET':'INTPRET'}
CONSTANTS={'K_V06N37':('V06N37',0),'K_V06N58':('V06N58',0),'K_130DEG_HI':('130DEG',0),'K_130DEG_LO':('130DEG',1),'K_P30ZERO':('P30ZERO',0),'K_V05N09':('V05N09',0)}
native=(R/'src/lm_p34p74_routines.c').read_text()
def body(name):
 m=re.search(r'^(?:INTERP\('+re.escape(name)+r',(0[0-7]+)u\)|ROUTINE\('+re.escape(name)+r'\)\{|static (?:void|enum exec_status) '+re.escape(name)+r'\([^)]*\)\{)',native,re.M)
 if not m:raise SystemExit('FAIL no native routine '+name)
 # INTERP opens its brace inside the macro; the others at the match end.
 depth=1;i=m.end()
 while depth:
  depth+=native[i]=='{';depth-=native[i]=='}';i+=1
 return (int(m.group(1),8)if m.group(1)else None),native[m.start():i]
def literals(text):return{int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',text)}
def table(name):return re.search(r'static const agc_routine_entry '+name+r'\[\]=\{(.*?)\};',native,re.S).group(1)
registered=dict((int(a,8),f)for a,f in re.findall(r'LM_P34P74_B35\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if bank!='35'or not any(lo<=a<=hi for lo,hi,_,_ in OWNERS):continue
 owner=[(lo,o,i)for lo,hi,o,i in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f'FAIL address {addr} has no single native owner')
 lo,name,interp=owner[0];entry,text=body(name)
 text+=''.join(body(h)[1]for h in HELPERS.get(name,[]))
 if registered.get(lo)!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at 35,{lo:o}')
 if interp:
  if entry!=lo:raise SystemExit(f'FAIL {name} is not keyed at 35,{lo:o}')
  for w in r['words']:
   if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
 elif not re.search(r'ENTRY\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} does not enter at 35,{lo:o}')
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
# Interpretive words must also appear in source order: each routine's
# assembled words are a subsequence of its literals as written.
for lo,hi,name,interp in OWNERS:
 if not interp:continue
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
def physical(text):
 if ','in text:b,o=text.split(',');return int(b,8)*1024+int(o,8)-0o2000
 a=int(text,8);return (a>>10)*1024+(a&0o1777)
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('P34','P74','P34/P74A','P34/P74B','P34/P74C','P34/P74D','P34/P74E','SWCHSET','INTLOOP','SWCHCLR','SELECTMU','PRECSET','S33/34.1','TIMETHET','S34/35.2','PERIAPO1','SHIFTR1','S34/35.5','VN1645','GOFLASH','ETPIFLAG','ITSWICH','RVSW','TTPI','TIG','ELEV','NOMTPI','TDEC1',*NAMED.values(),*{n for n,_ in CONSTANTS.values()})
s=symbols((work/'assembly.lst').read_text())
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p34p74_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,2545-35,2724'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of P34/P74 is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the erasable words and destinations the basic code names equal their native macros, and V06N37, V06N58, 130DEG, P30ZERO and V05N09 equal their native literals.')
(R/'build/lm-p34p74-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P34/P74 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
