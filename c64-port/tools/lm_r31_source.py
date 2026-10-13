#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 R31 transfer routines
(src/lm_r31_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_p76_source.py does) and requires the rope
to equal the certified fixed-data rope. Every assembled word of the file
(bank 40 3674-3712 and bank 37 3113-3407) must then be owned by exactly one registered routine
entered at the first address of its range. Interpretive words must appear as
literals, in source order, in that routine (with the shared bodies it runs
on into); basic words belong to the routine whose ENTRY names that address;
the constants after the code belong to the routines whose literals read
them; the erasable word and destinations the code names must equal their
native macros in the fresh assembly; and the fixed constants it reads (ONE,
BIT2, V6N34, V06N43, the K.01 and 600SEC pairs) must equal their native
literals in the fresh rope. Every routine must own at least one source
line. Writes build/lm-r31-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'R31.agc':'R31.agc'}
root=R/'build/lm-r31-source';assemble(mission,root);work=root/mission
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
# 2 a constant (named) whose words must equal the native literals DATA
# lists. Words a basic call carries (CADR, 2CADR, DEC) belong to its caller.
OWNERS=[('40',0o3674,0o3677,'r31call',0),('40',0o3700,0o3701,'dspdelay',0),('40',0o3702,0o3711,'fixdelay_ret',0),('40',0o3712,0o3712,'r_taskover',0),
 ('37',0o3113,0o3115,'dispn5x',0),('37',0o3116,0o3116,'b5off_1',0),('37',0o3117,0o3117,'b5off_2',0),('37',0o3120,0o3120,'redisplay',0),
 ('37',0o3121,0o3131,'v83call',0),('37',0o3132,0o3133,'v83_rtb',1),('37',0o3134,0o3135,'v83_lemprec',1),('37',0o3136,0o3143,'lemprec_ret',1),
 ('37',0o3144,0o3146,'docmbase',1),('37',0o3147,0o3154,'csmprec_ret',1),('37',0o3155,0o3165,'rev83',0),('37',0o3166,0o3167,'rev83_rtb',1),
 ('37',0o3170,0o3171,'rev83_intstall',1),('37',0o3172,0o3210,'intstall_ret',1),('37',0o3211,0o3216,'othconic',1),('37',0o3217,0o3221,'intstall_ret2',1),
 ('37',0o3222,0o3237,'othint',1),('37',0o3240,0o3245,'compdisp',1),('37',0o3246,0o3262,'range_rate',1),('37',0o3263,0o3303,'cdunbsm_ret',1),('37',0o3304,0o3315,'acos_ret',1),
 ('37',0o3316,0o3325,'exit_ret',0),('37',0o3326,0o3327,'getrvn',0),('37',0o3330,0o3330,'prio_ret',0),('37',0o3331,0o3343,'getrvn_i',1),
 ('37',0o3344,0o3345,'getrvn_exit',0),('37',0o3346,0o3346,'prio_ret2',0),('37',0o3347,0o3354,'getrvn_i2',1),('37',0o3355,0o3362,'vxm_vsl1',1),('37',0o3363,0o3364,'getrvn2',1),
 ('37',0o3365,0o3367,'intstall_ret3',1),('37',0o3370,0o3370,'r31surf',0),('37',0o3371,0o3372,'r31surf_rtb',1),('37',0o3373,0o3374,'r31surf_lemprec',1),
 ('37',0o3375,0o3376,'lemprec_ret2',1),('37',0o3377,0o3402,'mung',0),('37',0o3403,0o3403,'onebase',0),('37',0o3404,0o3406,'onebase_rtb',1),
 ('37',0o3407,0o3407,'V16N54',2)]
HELPERS={}
# Constant words: the native literal names that hold them.
DATA={'V16N54':['K_V16N54']}
NAMED={'FLAGWRD6':'FLAGWRD6','FLAGWRD7':'FLAGWRD7','FLAGWRD8':'FLAGWRD8','EXTVBACT':'EXTVBACT','FINDVAC':'FINDVAC','FIXDELAY':'FIXDELAY',
 'NOVAC':'NOVAC','TASKOVER':'TASKOVER','BANKCALL':'BANKCALL','B5OFF':'B5OFF','PRIOCHNG':'PRIOCHNG','ENDEXT':'ENDEXT','INTPRET':'INTPRET','ARCCOS':'ARCCOS'}
CONSTANTS={'K_PRIO3':('PRIO3',0),'K_PRIO5':('PRIO5',0),'K_PRIO22':('PRIO22',0),'K_BIT5':('BIT5',0),'K_BIT12':('BIT12',0),'K_AVEGFBIT':('AVEGFBIT',0),
 'K_SURFFBIT':('SURFFBIT',0),'K_MUNFLBIT':('MUNFLBIT',0),'K_V16N54':('V16N54',0),**{f'K_UNITZ_{i}':('UNITZ',i)for i in range(6)},
 'K_DPPOSMAX_HI':('DPPOSMAX',0),'K_DPPOSMAX_LO':('DPPOSMAX',1)}
native=(R/'src/lm_r31_routines.c').read_text()
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
registered=dict(((b,int(a,8)),f)for b,a,f in re.findall(r'LM_R31_B(37|40)\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
rows=[];owned=set()
def macro_value(name):
 m=re.search(r'#define '+re.escape(name)+r' ([0-7]+)u',native)
 if not m:raise SystemExit('FAIL no native literal '+name)
 return int(m.group(1),8)
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if not any(b==bank and lo<=a<=hi for b,lo,hi,_,_ in OWNERS):continue
 owner=[(lo,o,i)for b,lo,hi,o,i in OWNERS if b==bank and lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f'FAIL address {addr} has no single native owner')
 lo,name,kind=owner[0]
 if kind==2:
  macros=DATA[name][a-lo:a-lo+len(r['words'])]
  if [macro_value(m)for m in macros]!=r['words']:raise SystemExit(f'FAIL {bank},{addr} ({name}) differs from native {macros}')
  rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=' '.join(macros)));continue
 entry,text=body(name)
 text+=''.join(body(h)[1]for h in HELPERS.get(name,[]))
 if registered.get((bank,lo))!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at {bank},{lo:o}')
 if kind:
  if entry!=lo:raise SystemExit(f'FAIL {name} is not keyed at {bank},{lo:o}')
  for w in r['words']:
   if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
 elif not re.search(r'ENTRY(?:40)?\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} does not enter at {bank},{lo:o}')
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
# Interpretive words must also appear in source order: each routine's
# assembled words are a subsequence of its literals as written.
for _,lo,hi,name,interp in OWNERS:
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
 m=re.search(r'(?:#define '+macro+r' |[{,]'+macro+r'=)(0[0-7]*)u?\b',native);t=s[name];off=int(t.split(',')[-1],8)
 if not m or int(m.group(1),8)!=off or (','in t and t.split(',')[0]not in('37','40','00')):raise SystemExit(f'FAIL {name} ({t}) differs from native {macro}')
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' ([0-7]+)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('R31CALL','DSPDELAY','DISPN5X','V83CALL','DOCMBASE','REV83','OTHCONIC','OTHINT','COMPDISP','GETRVN','GETRVN2','R31SURF','MUNG?','ONEBASE','V16N54','LOADTIME','LEMPREC','CSMPREC','INTSTALL','INTEGRVS','NORMUNX1','CDU*NBSM','GOMARKF','REFSMMAT','MOONTHIS','MUNFLAG','AVEGFLAG','SURFFLAG','RANGE','RRATE','RTHETA','BASETHP','BASETHV','BASEOTP','BASEOTV','BASETIME',*NAMED.values(),'PRIO3','PRIO5','PRIO22','BIT5','BIT12','AVEGFBIT','SURFFBIT','MUNFLBIT','UNITZ','DPPOSMAX')
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_r31_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['40,3674-40,3712','37,3113-37,3407'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of R31.agc is owned by the registered routine entered at the first address of its range or is a constant equal to its native literal: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the erasable words and destinations the code names equal their native macros, and V16N54, UNITZ, DPPOSMAX and the PRIO and BIT masks equal their native literals.')
(R/'build/lm-r31-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled R31 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
