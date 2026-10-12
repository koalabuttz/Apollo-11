#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P31 transfer routines (src/lm_p31_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of
LAMBERT_AIMPOINT_GUIDANCE.agc (bank 35 2450-2544) must then be owned by
exactly one registered routine entered at the first address of its range.
Interpretive words must appear as literals, in source order, in that
routine; basic words belong to the routine whose ENTRY names that address;
the destinations the basic code names must equal their native macros in the
fresh assembly; and the fixed constants it reads (V06N33, V06N81, V06N42,
the P30ZERO pair) must equal their native literals in the fresh rope. Every
routine must own at least one source line. Writes
build/lm-p31-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc':'LAMBERT_AIMPOINT_GUIDANCE.agc'}
root=R/'build/lm-p31-source';assemble(mission,root);work=root/mission
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
OWNERS=[('35',0o2450,0o2450,'p31',0),('35',0o2451,0o2452,'p20_ret',0),('35',0o2453,0o2453,'vnpooh_ret',0),('35',0o2454,0o2460,'p31_start',1),
 ('35',0o2461,0o2502,'lemprec_ret',1),('35',0o2503,0o2511,'initvel_ret',1),('35',0o2512,0o2516,'getlvc_ret',1),('35',0o2517,0o2520,'periapo_ret',1),
 ('35',0o2521,0o2522,'shiftr1_ret',1),('35',0o2523,0o2526,'maxchk_ret',1),('35',0o2527,0o2530,'shiftr1_ret2',1),('35',0o2531,0o2532,'maxchk_ret2',1),
 ('35',0o2533,0o2534,'v06n81',0),('35',0o2535,0o2536,'v06n42',0),('35',0o2537,0o2537,'v06n42_ret',0),('35',0o2540,0o2542,'revn1645',1),
 ('35',0o2543,0o2544,'loop',1)]
HELPERS={}
DISPLAY={}
NAMED={'P20FLGON':'P20FLGON','VNPOOH':'VNPOOH','INTPRET':'INTPRET'}
CONSTANTS={'K_V06N33':('V06N33',0),'K_V06N81':('V06N81',0),'K_V06N42':('V06N42',0),'K_P30ZERO_HI':('P30ZERO',0),'K_P30ZERO_LO':('P30ZERO',1)}
native=(R/'src/lm_p31_routines.c').read_text()
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
registered=dict(((b,int(a,8)),f)for b,a,f in re.findall(r'LM_P31_B(35)\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
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
KEYS=('P31','REVN1645','LEMPREC','INITVEL','GET.LVC','PERIAPO1','SHIFTR1','MAXCHK','VN1645','TIG','RATT','VATT','RTX1','RTX2','RTIG','VTIG','RINIT','VINIT','DELLT4','TPASS4','DELVEET3','DELVSIN','VGDISP','VIPRIME','HPER','HAPO','UPDATFLG','XDELVFLG','FINALFLG',*NAMED.values(),'V06N33','V06N81','V06N42','P30ZERO')
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p31_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,2450-35,2544'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of LAMBERT_AIMPOINT_GUIDANCE.agc (P31 and REVN1645) is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the destinations the basic code names equal their native macros, and V06N33, V06N81, V06N42 and the P30ZERO pair equal their native literals.')
(R/'build/lm-p31-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P31 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
