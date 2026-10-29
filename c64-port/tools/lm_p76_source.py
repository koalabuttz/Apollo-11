#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P76 transfer routines (src/lm_p76_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of P76 (bank 13
2207-2350) must then be owned by exactly one registered routine entered at
the first address of its range. Interpretive words must appear as literals,
in source order, in that routine; basic words belong to the routine whose
ENTRY names that address; the erasable word and destinations the basic code
names must equal their native macros in the fresh assembly; and the fixed
constants it reads (V06N84, V06N84 +1, ZERO) must equal their native
literals in the fresh rope. Every routine must own at least one source
line. Writes build/lm-p76-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P76.agc':'P76.agc'}
root=R/'build/lm-p76-source';assemble(mission,root);work=root/mission
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
# (first, last, routine, interpretive) bank-13 ranges. Words a basic call
# carries (ADRES, CADR, OCT) belong to its caller; the V06N84 pair (2336-
# 2337) follows ENDP76 in its range, and the constants check ties it to the
# flashes' literals.
OWNERS=[(0o2207,0o2210,'p76',0),(0o2211,0o2211,'trackflg_ret',0),(0o2212,0o2215,'p76_save',1),(0o2216,0o2220,'r_flash84',0),
 (0o2221,0o2221,'f84_terminate',0),(0o2222,0o2222,'f84_proceed',0),(0o2223,0o2223,'f84_recycle',0),(0o2224,0o2226,'r_flash33',0),
 (0o2227,0o2227,'f33_terminate',0),(0o2230,0o2230,'f33_proceed',0),(0o2231,0o2231,'f33_recycle',0),(0o2232,0o2232,'r_intpret',0),
 (0o2233,0o2236,'p76_tig',1),(0o2237,0o2260,'compmat',1),(0o2261,0o2262,'call_sub1',1),(0o2263,0o2301,'sub1_ret',1),
 (0o2302,0o2303,'integrvs_ret',1),(0o2304,0o2314,'intstall_ret',1),(0o2315,0o2315,'minirect_ret',1),(0o2316,0o2317,'phaschng',0),
 (0o2320,0o2321,'phaschng_ret',0),(0o2322,0o2322,'reintflg_ret',0),(0o2323,0o2324,'call_atopoth',1),(0o2325,0o2327,'atopoth_ret',1),
 (0o2330,0o2331,'intwake',0),(0o2332,0o2332,'out',1),(0o2333,0o2337,'r_endp76',0),(0o2340,0o2350,'p76sub1',1)]
HELPERS={'r_flash84':['flash84'],'f84_recycle':['flash84'],'f84_proceed':['flash33'],'r_flash33':['flash33'],'f33_recycle':['flash33'],
 'f84_terminate':['endp76'],'f33_terminate':['endp76'],'r_endp76':['endp76'],'p76_save':['exit_order'],'minirect_ret':['exit_order'],'out':['exit_order']}
NAMED={'MARKCTR':'MARKCTR','UPFLAG':'UPFLAG','PHASCHNG':'PHASCHNG','BANKCALL':'BANKCALL','GOTOPOOH':'GOTOPOOH','INTPRET':'INTPRET'}
CONSTANTS={'K_V06N84':('V06N84',0),'K_V06N33':('V06N84',1),'K_ZERO':('ZERO',0)}
native=(R/'src/lm_p76_routines.c').read_text()
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
registered=dict((int(a,8),f)for a,f in re.findall(r'LM_P76_B13\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if bank!='13'or not any(lo<=a<=hi for lo,hi,_,_ in OWNERS):continue
 owner=[(lo,o,i)for lo,hi,o,i in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f'FAIL address {addr} has no single native owner')
 lo,name,interp=owner[0];entry,text=body(name)
 text+=''.join(body(h)[1]for h in HELPERS.get(name,[]))
 if registered.get(lo)!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at 13,{lo:o}')
 if interp:
  if entry!=lo:raise SystemExit(f'FAIL {name} is not keyed at 13,{lo:o}')
  for w in r['words']:
   if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
 elif not re.search(r'ENTRY\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} does not enter at 13,{lo:o}')
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
 if not m or int(m.group(1),8)!=off or (','in t and t.split(',')[0]not in('13','E7')):raise SystemExit(f'FAIL {name} ({t}) differs from native {macro}')
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('P76','COMPMAT','INTOTHIS','OUT','ENDP76','P76SUB1','V06N84','OTHPREC','INTSTALL','INTEGRVS','MINIRECT','ATOPOTH','INTWAKE1','GOFLASH','DELVOV','DELVLVC','TIG','TET','TETTHIS','RRECT','TRACKFLG','REINTFLG','MOONFLAG','CMOONFLG','INTYPFLG',*NAMED.values(),'ZERO')
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p76_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['13,2207-13,2350'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of P76 is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the erasable word and destinations the basic code names equal their native macros, and V06N84, V06N84 +1 and ZERO equal their native literals.')
(R/'build/lm-p76-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P76 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
