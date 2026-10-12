#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P34-35_P74-75 transfer routines (src/lm_p34_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of GET45 +1 through VNPOOH
(bank 35 3635-3722) must then be owned by exactly one native routine (or a
helper it calls) that names the routine's entry address; CLUPDATE's
interpretive words (35,3671-3673) must be literals of lm_p34_clupdate, the two
CADR GOFLASH words after TCR BANKCALL (35,3700, 35,3717) must be the native
site operands, and the fixed constants the basic code reads (BIT6, 1SEC, HIGH9,
V06N55, V16N45) must equal their native literals in the fresh rope. Every
routine of the native basic table must own at least one source line. Writes
build/lm-p34-transfer-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P34-35_P74-75.agc':'P34-P35,_P74-P75.agc'}
root=R/'build/lm-p34-source';assemble(mission,root);work=root/mission
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
# (first, last) bank-35 addresses and the native routine that runs them.
OWNERS=[(0o3635,0o3635,'lm_p34_get45_comptgo'),(0o3636,0o3642,'get45_delay'),(0o3643,0o3645,'get45_flash'),
 (0o3646,0o3646,'get45_terminate'),(0o3647,0o3647,'get45_proceed'),(0o3650,0o3650,'get45_recycle'),
 (0o3651,0o3653,'r_kilclock'),(0o3654,0o3661,'r_n45proc'),(0o3662,0o3663,'n45proc_upflag'),
 (0o3664,0o3667,'r_clupdate'),(0o3670,0o3670,'clupdate_intpret'),(0o3671,0o3673,'lm_p34_clupdate'),
 (0o3674,0o3700,'r_displaye'),(0o3701,0o3701,'displaye_terminate'),(0o3702,0o3702,'displaye_proceed'),
 (0o3703,0o3703,'displaye_recycle'),(0o3704,0o3711,'r_p3xorp7x'),(0o3712,0o3717,'r_vnpooh'),
 (0o3720,0o3720,'vnpooh_terminate'),(0o3721,0o3721,'vnpooh_proceed'),(0o3722,0o3722,'vnpooh_recycle')]
# Helpers a routine runs inline (the TC targets it falls into).
HELPERS={'get45_terminate':['kilclock'],'get45_proceed':['n45proc','kilclock'],'get45_recycle':['clupdate'],
 'r_kilclock':['kilclock'],'r_n45proc':['n45proc','kilclock'],'r_clupdate':['clupdate'],
 'r_displaye':['displaye_flash'],'displaye_recycle':['displaye_flash'],'displaye_proceed':['stored_return'],
 'r_vnpooh':['vnpooh_flash'],'vnpooh_recycle':['vnpooh_flash'],'vnpooh_proceed':['stored_return']}
INTERPRETIVE=(0o3671,0o3673)
SITES=(0o3700,0o3717)
CONSTANTS={'K_BIT6':'BIT6','K_1SEC':'1SEC','K_HIGH9':'HIGH9','K_V06N55':'V06N55','K_V16N45':'V16N45'}
native=(R/'src/lm_p34_routines.c').read_text()
def body(name):
 m=re.search(r'^(?:ROUTINE|INTERP)\('+re.escape(name)+r'\)\{|^(?:static )?(?:uint8_t|void|enum exec_status) '+re.escape(name)+r'\([^)]*\)\{',native,re.M)
 if not m:raise SystemExit('FAIL no native function '+name)
 depth=0
 for i in range(m.end()-1,len(native)):
  depth+=native[i]=='{';depth-=native[i]=='}'
  if native[i]=='}'and depth==0:return native[m.start():i+1]
def literals(text):return{int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',text)}
table=re.search(r'basic\[LM_P34_BASIC_ROUTINES\]=\{(.*?)\};',native,re.S).group(1)
registered=dict((int(a,8),f)for a,f in re.findall(r'LM_P34_B35\((0[0-7]+)\),(\w+)\}',table))
sites=dict((int(a,8),int(w,8))for a,w in re.findall(r'LM_P34_B35\((0[0-7]+)\),(0[0-7]+)u\}',re.search(r'sites\[LM_P34_SITES\]=\{(.*?)\};',native,re.S).group(1)))
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if bank!='35'or not 0o3635<=a<=0o3722:continue
 owner=[(lo,o)for lo,hi,o in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f'FAIL address {addr} has no single native owner')
 lo,name=owner[0];text=body(name)+''.join(body(h)for h in HELPERS.get(name,[]))
 if INTERPRETIVE[0]<=a<=INTERPRETIVE[1]:
  for w in r['words']:
   if w not in literals(body(name)):raise SystemExit(f"FAIL line {r['line']} word {w:05o} is not a literal of {name}")
 elif a in SITES:
  if sites.get(a)!=r['words'][0]or lo not in registered or a not in literals(text):raise SystemExit(f"FAIL line {r['line']} CADR {r['words'][0]:05o} is not a native site operand after {name}")
 elif registered.get(lo)!=name or not re.search(r'ENTRY\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} is not the registered routine entered at 35,{lo:o}')
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
s=symbols((work/'assembly.lst').read_text())
def physical(text):
 if ','in text:b,o=text.split(',');return int(b,8)*1024+int(o,8)-0o2000
 return int(text,8)
for macro,name in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('GET45','KILCLOCK','N45PROC','CLUPDATE','DISPLAYE','P3XORP7X','VNPOOH','COMPTGO','BANKCALL','DELAYJOB','GOFLASH','PHASCHNG','UPFLAG','GOTOPOOH','INTPRET','QSAVED','NORMEX','RTRN','SUBEXIT','VERBNOUN','DISPDEX','MODREG','UPDATFLG','FINALFLG',*CONSTANTS.values())
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p34_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,3635-35,3722'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of GET45 +1 through VNPOOH is owned by the registered transfer routine entered at its first address (with the inline helpers its TCs fall into); CLUPDATE\'s CLEAR GOTO UPDATFLG QSAVED words are literals of lm_p34_clupdate, both CADR GOFLASH words after TCR BANKCALL are native site operands, and BIT6, 1SEC, HIGH9, V06N55 and V16N45 equal their native literals. COMPTGO (P32-P35_P72-P75) and the display constants after 35,3722 are not covered.')
(R/'build/lm-p34-transfer-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P34-35 transfer words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
