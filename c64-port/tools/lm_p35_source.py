#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P35/P75 transfer routines (src/lm_p35_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of P35/P75 (bank 35
2725-2763) must then be owned by exactly one registered routine entered at
the first address of its range. Interpretive words must appear as literals,
in source order, in that routine; basic words belong to the routine whose
ENTRY names that address; and the erasable words and destinations the basic
code names (ATIGINC, PTIGINC, KT, AVFLAGA, AVFLAGP, P20FLGON, INTPRET) must
equal their native macros in the fresh assembly. Every routine must own at
least one source line. Writes build/lm-p35-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P34-35_P74-75.agc':'P34-P35,_P74-P75.agc'}
root=R/'build/lm-p35-source';assemble(mission,root);work=root/mission
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
# (first, last, routine, interpretive) bank-35 ranges. P35/P75A
# (2734-2735) is P75's fall-through; P35's return reaches it by TC.
OWNERS=[(0o2725,0o2725,'p35',0),(0o2726,0o2730,'p35_avflaga',0),(0o2731,0o2731,'p75',0),(0o2732,0o2735,'p75_avflagp',0),
 (0o2736,0o2736,'p20_intpret',0),(0o2737,0o2740,'sel_mu',1),(0o2741,0o2742,'r_p35b',1),(0o2743,0o2751,'loadtime_ret',1),
 (0o2752,0o2753,'call_s1',1),(0o2754,0o2755,'call_s2',1),(0o2756,0o2757,'call_s5',1),(0o2760,0o2761,'call_vn1645',1),(0o2762,0o2763,'loop',1)]
# Helpers a routine runs inline (the code its TCs fall into).
HELPERS={'p35_avflaga':['p35a'],'p75_avflagp':['p35a'],'r_p35b':['p35b']}
# Addresses the basic code names: erasable (EB window, EBANK=KT) and bank-35
# or fixed-fixed destinations.
NAMED={'ATIGINC':'ATIGINC','PTIGINC':'PTIGINC','KT':'KT','AVFLAGA':'AVFLAGA','AVFLAGP':'AVFLAGP','P20FLGON':'P20FLGON','INTPRET':'INTPRET'}
native=(R/'src/lm_p35_routines.c').read_text()
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
registered=dict((int(a,8),f)for a,f in re.findall(r'LM_P35_B35\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
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
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('P35','P75','P35/P75A','P35/P75B','SELECTMU','LOADTIME','PRECSET','S34/35.1','S34/35.2','S34/35.5','VN1645','TSTRT','TIG','INTIME','TDEC1',*NAMED.values())
s=symbols((work/'assembly.lst').read_text())
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p35_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,2725-35,2763'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of P35/P75 is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, and the erasable words and destinations the basic code names equal their native macros.')
(R/'build/lm-p35-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P35/P75 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
