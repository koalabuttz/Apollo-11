#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 S34/35 transfer routines (src/lm_s3435_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled interpretive word of S34/35.1
(bank 35 3274-3305), NOVRWRT, S34/35.4 and LOMAT (3424-3461) must then appear
as a literal, in source order, in the registered routine that owns its
address, and every
routine must own at least one source line. Writes
build/lm-s3435-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols
mission='Luminary099'
FILES={'P34-35_P74-75.agc':'P34-P35,_P74-P75.agc'}
root=R/'build/lm-s3435-source';assemble(mission,root);work=root/mission
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
# (first, last) bank-35 addresses and the routine that compiles them.
OWNERS=[(0o3274,0o3305,'s3435_1'),(0o3424,0o3441,'novrwrt'),(0o3442,0o3446,'s3435_4'),(0o3447,0o3461,'lomat')]
native=(R/'src/lm_s3435_routines.c').read_text()
def body(name):
 m=re.search(r'^ROUTINE\('+re.escape(name)+r',(0[0-7]+)u\)',native,re.M)
 if not m:raise SystemExit('FAIL no native routine '+name)
 end=native.index('\n}\n',m.end())
 return int(m.group(1),8),native[m.start():end+2]
def literals(text):return{int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',text)}
table=re.search(r'#if !defined LM_S3435_C64_GROUP\nstatic const agc_routine_entry entries\[\]=\{(.*?)\};',native,re.S).group(1)
registered=dict((int(a,8),f)for a,f in re.findall(r'LM_S3435_B35\((0[0-7]+)\),(\w+)\}',table))
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if bank!='35'or not any(lo<=a<=hi for lo,hi,_ in OWNERS):continue
 owner=[(lo,o)for lo,hi,o in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f'FAIL address {addr} has no single native owner')
 lo,name=owner[0];entry,text=body(name)
 if entry!=lo or registered.get(lo)!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at 35,{lo:o}')
 for w in r['words']:
  if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
# The words must also appear in source order: each routine's assembled words
# are a subsequence of its literals as written.
for lo,hi,name in OWNERS:
 seq=[int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',body(name)[1])];i=0
 for r in rows:
  if r['native']!=name:continue
  for w in r['words']:
   w=int(w,8)
   while i<len(seq)and seq[i]!=w:i+=1
   if i==len(seq):raise SystemExit(f"FAIL {name}: word {w:05o} (line {r['line']}) is out of source order in the native routine")
   i+=1
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('S34/35.1','S34/35.4','NOVRWRT','LOMAT','RPASS3','RACT3','VACT3','ULOS','UNRM','DELVEET3','DVLOS','NORMEX')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_s3435_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,3274-35,3305','35,3424-35,3461'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every interpretive word of S34/35.1, NOVRWRT, S34/35.4 and LOMAT is a literal bookkeeping argument of the registered transfer routine entered at the first address of its range.')
(R/'build/lm-s3435-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled S34/35 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
