#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 S34/35.2 head and tail and S34/35.5 transfer routines
(src/lm_s3435c_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of S34/35.2's head
(bank 35 3306-3326) and LOMAT tail (3367-3376) and of S34/35.5 (3522-3605)
must then be owned by exactly one registered routine entered at the first
address of its range. Interpretive words must appear as literals, in source
order, in that routine; basic words belong to the routine whose ENTRY names
that address; the CADR GOFLASH after TC BANKCALL (35,3540) must be the
generated INTER-BANK site operand; and the fixed constants the code reads
(V06N81, V06N59, EBANK7, FIVE, TWOPI) must equal their native literals in
the fresh rope. Every routine must own at least one source line. Writes
build/lm-s3435c-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P34-35_P74-75.agc':'P34-P35,_P74-P75.agc'}
root=R/'build/lm-s3435c-source';assemble(mission,root);work=root/mission
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
# (first, last, routine, interpretive) bank-35 ranges.
OWNERS=[(0o3306,0o3326,'s3435_2',1),(0o3367,0o3370,'s3435_2_initvel',1),(0o3371,0o3376,'s3435_2_lomat',1),
 (0o3522,0o3530,'s3435_5',1),(0o3531,0o3535,'flagon',1),(0o3536,0o3540,'r_flagon_flash',0),(0o3541,0o3541,'flash_terminate',0),
 (0o3542,0o3542,'flash_proceed',0),(0o3543,0o3543,'flash_recycle',0),(0o3544,0o3564,'r_ntargchk',0),(0o3565,0o3565,'upflag_intpret',0),
 (0o3566,0o3571,'ntarg_check',1),(0o3572,0o3575,'nochg',1),(0o3576,0o3577,'flagoff',1),(0o3600,0o3600,'flagoff_exit',1),
 (0o3601,0o3602,'flagoff_vnpooh',0),(0o3603,0o3603,'vnpooh_intpret',0),(0o3604,0o3605,'s3435_5_return',1)]
# Helpers a basic routine runs inline (the code its TCs fall into).
HELPERS={'r_flagon_flash':['flagon_flash'],'flash_proceed':['ntargchk'],'flash_recycle':['flagon_flash'],'r_ntargchk':['ntargchk']}
SITE=(0o3540,30560)
CONSTANTS={'K_V06N81':('V06N81',0),'K_V06N59':('V06N59',0),'K_EBANK7':('EBANK7',0),'K_FIVE':('FIVE',0),'K_TWOPI_HI':('TWOPI',0),'K_TWOPI_LO':('TWOPI',1)}
native=(R/'src/lm_s3435c_routines.c').read_text()
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
registered=dict((int(a,8),f)for a,f in re.findall(r'LM_S3435C_B35\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
sites=open(R/'src/lm_interbank_sites.inc').read()
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if bank!='35'or not any(lo<=a<=hi for lo,hi,_,_ in OWNERS):continue
 owner=[(lo,o,i)for lo,hi,o,i in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f'FAIL address {addr} has no single native owner')
 lo,name,interp=owner[0];entry,text=body(name)
 if registered.get(lo)!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at 35,{lo:o}')
 if interp:
  if entry!=lo:raise SystemExit(f'FAIL {name} is not keyed at 35,{lo:o}')
  for w in r['words']:
   if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
 elif a==SITE[0]:
  if not re.search(r'\{%du,0?%o\}'%(SITE[1],r['words'][0]),sites):raise SystemExit(f"FAIL CADR {r['words'][0]:05o} at 35,{a:o} is not the INTER-BANK site operand")
 elif not re.search(r'ENTRY\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} does not enter at 35,{lo:o}')
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
# Interpretive words must also appear in source order: each routine's
# assembled words are a subsequence of its literals as written.
for lo,hi,name,interp in OWNERS:
 if not interp:continue
 seq=[int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',body(name)[1])];i=0
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
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('S34/35.2','S3435.23','S3435.25','S34/35.5','FLAGON','NTARGCHK','NOCHG','FLAGOFF','INTINT','LOMAT','INITVEL','S34/35.3','S34/35.4','VNPOOH','BANKCALL','GOFLASH','GOTOPOOH','UPFLAG','INTPRET','SUBEXIT','VPASS3','RPASS3','INTIME','TPASS4','NN','ZEROVECS','DELVEET3','DELVLVC','GDT/2','DELVSIN','FINALFLG','UPDATFLG','NTARGFLG','XDELVFLG',*{n for n,_ in CONSTANTS.values()})
s=symbols((work/'assembly.lst').read_text())
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_s3435c_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,3306-35,3326','35,3367-35,3376','35,3522-35,3605'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of S34/35.2\'s head and LOMAT tail and of S34/35.5 is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the CADR GOFLASH at 35,3540 is the INTER-BANK site operand, and V06N81, V06N59, EBANK7, FIVE and TWOPI equal their native literals. S34/35.2\'s Lambert body (35,3327-3366) is not covered.')
(R/'build/lm-s3435c-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled S34/35.2 and S34/35.5 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
