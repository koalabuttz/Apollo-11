#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 INITVEL (src/lm_initvel.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/ephemeris_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of P34-35_P74-75's INITVEL
(bank 11 2000-2274) and of ORBITAL_INTEGRATION's SETITCTR (11,3765-3775)
must then appear as a literal in the native function that owns its address
(or a helper it calls), and every native phase must own at least one source
line. Writes build/lm-initvel-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols
mission='Luminary099'
# Original file and its include name in the fixture's MAIN.agc.
FILES={'P34-35_P74-75.agc':'P34-P35,_P74-P75.agc','ORBITAL_INTEGRATION.agc':'ORBITAL_INTEGRATION.agc'}
root=R/'build/initvel-source';assemble(mission,root);work=root/mission
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
OWNERS=[('11',0o2000,0o2001,'liv_initvel'),('11',0o2002,0o2025,'liv_havegues'),('11',0o2026,0o2061,'liv_initvel1'),
 ('11',0o2062,0o2104,'liv_initvel2'),('11',0o2105,0o2144,'liv_initvel3'),('11',0o2145,0o2154,'liv_afterlambert'),
 ('11',0o2155,0o2177,'liv_afterintstall'),('11',0o2200,0o2224,'liv_afterintegrvs'),('11',0o2225,0o2227,'liv_initvel6'),
 ('11',0o2230,0o2274,'liv_initvel7'),('11',0o3765,0o3775,'liv_initvel3')]
COVERED=lambda b,a:any(bb==b and lo<=a<=hi for bb,lo,hi,_ in OWNERS)
# Helpers a function reaches for words it passes on.
HELPERS={'liv_havegues':['abval_rtmag'],'liv_initvel1':['setpd0'],'liv_initvel7':['setpd0']}
native=(R/'src/lm_initvel.c').read_text()
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
 for w in r['words']:
  if w not in literals(owner[0]):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {owner[0]}")
 owned.add(owner[0]);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=owner[0]))
missing={o for *_,o in OWNERS}-owned
if missing:raise SystemExit('FAIL native phases own no source: '+', '.join(sorted(missing)))
KEYS=('INITVEL','HAVEGUES','INITVEL1','INITVEL2','INITVEL3','INITVEL4','INITVEL5','INITVEL6','INITVEL7','INITVELX','SETITCTR','LAMBERT','INTSTALL','INTEGRVS','NORMEX','RINIT','VINIT','RTARG','RTARG1','DELLT4','INTIME','VIPRIME','VTPRIME','DELVEET3','VTARGTAG','ITCTR','COZY4','RTX1','RTX2','MUEARTH','ZEROVEC','MUTABLE')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_initvel.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['11,2000-11,2274','11,3765-11,3775'],
 note='Fresh yaYUL assembly of the unchanged files reproduces the certified rope; every word of INITVEL (HAVEGUES through INITVELX) and of SETITCTR is a literal bookkeeping argument or constant of the native function owning its address. LAMBERT and its conic steps are the lm-conic-lambert milestone\'s (src/lm_conic_lambert.c); INTSTALL and INTEGRVS are not ported. The rest of both files is not covered here.')
(R/'build/lm-initvel-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled INITVEL words on',len(rows),'source lines map to native literals in',len(owned),'functions',flush=True)
