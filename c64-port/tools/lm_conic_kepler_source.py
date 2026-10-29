#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 CONIC_SUBROUTINES Kepler subroutine (src/lm_conic_kepler.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/ephemeris_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of the file's KEPLERN,
DELTIME and CHECKCTR range (bank 12 2023-2535 and 2644-2652) must then appear as a literal in the native function that owns its address
(or a helper it calls), and every native phase must own at least one source
line. Writes build/lm-conic-kepler-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols,filename_alias
mission='Luminary099';name='CONIC_SUBROUTINES.agc';path=mission+'/'+name;original=R.parent/path
root=R/'build/kepler-source';assemble(mission,root);work=root/mission
includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')];alias=filename_alias(name,includes)
shutil.copyfile(original,work/alias);shutil.copyfile(R/'deps/lm-rtb-fixture/INTERPRETER.agc',work/'INTERPRETER.agc')
with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes()
if rope!=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes():raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
text=original.read_text();records=listing_records(work/'assembly.lst',{alias:path},{path:text},rope)
if {r['line']for r in records}!={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}:raise SystemExit('FAIL listing does not cover every source line')
# Bank-12 address ranges and the native function that compiles them.
OWNERS=[(0o2023,0o2072,'lck_entry'),(0o2073,0o2076,'lck_1rev'),(0o2077,0o2111,'lck_storemax'),(0o2112,0o2123,'lck_periodch'),
 (0o2124,0o2137,'lck_moddone'),(0o2140,0o2151,'lck_storbnds'),(0o2152,0o2154,'lck_storemin'),(0o2155,0o2165,'lck_dxcomp'),
 (0o2166,0o2176,'lck_keploop'),(0o2177,0o2237,'lck_kepret'),(0o2240,0o2245,'lck_ndxchnge'),(0o2246,0o2257,'lck_posdelx'),
 (0o2260,0o2266,'lck_newdelx'),(0o2267,0o2273,'lck_brnchctr'),(0o2274,0o2301,'lck_pdxchnge'),(0o2302,0o2310,'lck_badx'),
 (0o2311,0o2314,'lck_timeovfl'),(0o2315,0o2327,'lck_cmntovfl'),(0o2330,0o2332,'lck_negtovfl'),(0o2333,0o2420,'lck_kepconvg'),
 (0o2421,0o2452,'lck_deltime'),(0o2453,0o2502,'lck_deltime2'),(0o2503,0o2535,'lck_deltail'),(0o2644,0o2652,'lck_brnchctr')]
COVERED=lambda a:any(lo<=a<=hi for lo,hi,_ in OWNERS)
# Helpers a function reaches for words it passes on (the S(XI)/C(XI) POLY
# tables, RVQ, the 0.9 NEWDELX bound and RTB TPMODE).
HELPERS={'lck_deltime':['constants'],'lck_deltime2':['constants'],'lck_deltail':['rvq','rtb_tpmode'],'lck_posdelx':['dp910_newdelx'],'lck_pdxchnge':['dp910_newdelx'],'lck_ndxchnge':['dp910_newdelx']}
native=(R/'src/lm_conic_kepler.c').read_text()
def body(name):
 if name=='constants':
  # File-level constant tables (the S(XI)/C(XI) POLY tables).
  return ''.join(re.findall(r'^(?:static )?const agc_word [^;]*;',native,re.M|re.S))
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
 if bank!='12'or not COVERED(a):continue
 owner=[o for lo,hi,o in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f"FAIL address {addr} has no single native owner")
 for w in r['words']:
  if w not in literals(owner[0]):raise SystemExit(f"FAIL line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {owner[0]}")
 owned.add(owner[0]);rows.append(dict(line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=owner[0]))
missing={o for _,_,o in OWNERS}-owned
if missing:raise SystemExit('FAIL native phases own no source: '+', '.join(sorted(missing)))
KEYS=('KEPLERN','KEPPREP','DELTIME','CHECKCTR','KEPRTN','MUTABLE','KEPZERO','D1/64','D1/128','D1/256','2PISC','-50SC','BEE22','RRECT','VRECT','TAU.','XKEPNEW','XPREV','TC','EPSILONT','ITERCTR')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_path=path,source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_conic_kepler.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['12,2023-12,2535','12,2644-12,2652'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of KEPLERN, DELTIME (with its S(XI)/C(XI) tables) and CHECKCTR is a literal bookkeeping argument or constant of the native function owning its address. MUTABLE and KEPZERO are read from the certified fixed-data table; the D1/64, D1/128, D1/256, 2PISC, -50SC, 0.9 and BEE22 constants are native literals. The rest of the file (ITERATOR, TIMETHET, TIMERAD, APSIDES, LAMBERT and their constants) is not yet ported.')
(R/'build/lm-conic-kepler-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled KEPLERN words on',len(rows),'source lines map to native literals in',len(owned),'functions',flush=True)
