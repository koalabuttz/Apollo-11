#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 MIDGIM and SELECTMU (src/lm_vn1645.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/ephemeris_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled interpretive word of
P34-35_P74-75's VN1645 head (bank 35 3606-3634) and its DP-.01 constant
(35,3741-3742) must then appear as a literal in the native function that owns its address
(or a helper it calls); the basic instructions (35,3624-3627 and
P3XORP7X 35,3704-3711) are owned by the function that compiles them, and
the fixed constant they read (HIGH9) must equal its native literal. Every
native phase must own at least one source line. Writes build/lm-vn1645-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols
mission='Luminary099'
# Original file and its include name in the fixture's MAIN.agc.
FILES={'P34-35_P74-75.agc':'P34-P35,_P74-P75.agc'}
root=R/'build/vn1645-source';assemble(mission,root);work=root/mission
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
# Address ranges (bank, first, last) and the native function that compiles them.
OWNERS=[('35',0o3606,0o3627,'lvn_vn1645'),('35',0o3630,0o3633,'lvn_mga'),('35',0o3634,0o3634,'lvn_get45'),('35',0o3704,0o3711,'lvn_vn1645'),('35',0o3741,0o3742,'lvn_vn1645')]
# Basic instructions: owned, not literal-checked (their fixed constants are).
BASIC=[('35',0o3624,0o3627),('35',0o3704,0o3711)]
CONSTANTS={'HIGH9':0o77700}
COVERED=lambda b,a:any(bb==b and lo<=a<=hi for bb,lo,hi,_ in OWNERS)
# Helpers a function reaches for words it passes on.
HELPERS={'lvn_vn1645':['dload_dpm01','dpm01_words']}
native=(R/'src/lm_vn1645.c').read_text()
def body(name):
 m=re.search(r'^(?:static )?(?:uint8_t|void) '+re.escape(name)+r'\([^)]*\)\{',native,re.M)
 if not m:raise SystemExit('FAIL no native function '+name)
 depth=0
 for i in range(m.start(),len(native)):
  depth+=native[i]=='{';depth-=native[i]=='}'
  if native[i]=='}'and depth==0:return native[m.start():i+1]
def literals(name):
 text=body(name)+''.join(body(h)for h in HELPERS.get(name,[]))
 return{int(x,8)for x in re.findall(r'\b0[0-7]*\b',text)}
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if not COVERED(bank,a):continue
 owner=[o for bb,lo,hi,o in OWNERS if bb==bank and lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f"FAIL address {addr} has no single native owner")
 basic=any(bb==bank and lo<=a<=hi for bb,lo,hi in BASIC)
 for w in r['words']:
  if not basic and w not in literals(owner[0]):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {owner[0]}")
 owned.add(owner[0]);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=owner[0]))
for name,value in CONSTANTS.items():
 if not re.search(r'\b0?%o\b[^\n]*/\* %s \*/'%(value,re.escape(name)),native):raise SystemExit('FAIL fixed constant %s %05o is not a tagged native literal'%(name,value))
missing={o for *_,o in OWNERS}-owned
if missing:raise SystemExit('FAIL native phases own no source: '+', '.join(sorted(missing)))
KEYS=('VN1645','GET45','P3XORP7X','DP-.01','HIGH9','SUBEXIT','+MGA','DELVSIN','FINALFLG','REFSMFLG','MODREG','GET+MGA','INTPRET','COMPTGO')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_vn1645.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,3606-35,3634','35,3704-35,3711','35,3741-35,3742'],
 note='Fresh yaYUL assembly of the unchanged files reproduces the certified rope; every interpretive word of the VN1645 head (VN1645 through GET45 EXIT) and DP-.01 is a literal bookkeeping argument or constant of the native function owning its address; the basic TC words and P3XORP7X are compiled agc_image_ops instructions in lvn_vn1645, and HIGH9 equals its native literal. GET+MGA is the lm-midgim step; COMPTGO and the display code after GET45 are not covered.')
(R/'build/lm-vn1645-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled VN1645 words on',len(rows),'source lines map to native literals in',len(owned),'functions',flush=True)
