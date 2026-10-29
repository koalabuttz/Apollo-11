#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 CONIC_SUBROUTINES time-radius and apsides subroutines
(src/lm_conic_timerad.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/ephemeris_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of the file's TIMERAD
through INFINAPO range (bank 12 3547-3741) must then appear as a literal in
the native function that owns its address
(or a helper it calls), and every native phase must own at least one source
line. Writes build/lm-conic-timerad-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols,filename_alias
mission='Luminary099';name='CONIC_SUBROUTINES.agc';path=mission+'/'+name;original=R.parent/path
root=R/'build/timerad-source';assemble(mission,root);work=root/mission
includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')];alias=filename_alias(name,includes)
shutil.copyfile(original,work/alias);shutil.copyfile(R/'deps/lm-rtb-fixture/INTERPRETER.agc',work/'INTERPRETER.agc')
with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes()
if rope!=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes():raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
text=original.read_text();records=listing_records(work/'assembly.lst',{alias:path},{path:text},rope)
if {r['line']for r in records}!={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}:raise SystemExit('FAIL listing does not cover every source line')
# Address ranges (bank, first, last) and the native function that compiles them.
OWNERS=[('12',0o3547,0o3560,'ltr_timerad'),('12',0o3561,0o3627,'ltr_afterparam'),('12',0o3630,0o3654,'ltr_termnvec'),
 ('12',0o3655,0o3657,'ltr_soln'),('12',0o3660,0o3663,'ltr_circular'),('12',0o3664,0o3673,'ltr_badr2'),
 ('12',0o3674,0o3705,'ltr_apsides'),('12',0o3706,0o3736,'ltr_getecc'),('12',0o3737,0o3741,'ltr_infinapo')]
COVERED=lambda b,a:any(bb==b and lo<=a<=hi for bb,lo,hi,_ in OWNERS)
# Helpers a function reaches for words it passes on (the shared entry head).
HELPERS={'ltr_timerad':['head'],'ltr_apsides':['head']}
native=(R/'src/lm_conic_timerad.c').read_text()
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
 if not COVERED(bank,a):continue
 owner=[o for bb,lo,hi,o in OWNERS if bb==bank and lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f"FAIL address {addr} has no single native owner")
 for w in r['words']:
  if w not in literals(owner[0]):raise SystemExit(f"FAIL line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {owner[0]}")
 owned.add(owner[0]);rows.append(dict(line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=owner[0]))
missing={o for *_,o in OWNERS}-owned
if missing:raise SystemExit('FAIL native phases own no source: '+', '.join(sorted(missing)))
KEYS=('TIMERAD','TERMNVEC','CIRCULAR','BADR2','APSIDES','GETECC','INFINAPO','RTNTR','RTNAPSE','SGNRDOT','RDESIRED','ECC','COSF','SOLNSW','APSESW','LODPHALF','KEPZERO','LDPOSMAX','DP1/4')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_path=path,source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_conic_timerad.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['12,3547-12,3741'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of TIMERAD, TERMNVEC, CIRCULAR, BADR2, APSIDES, GETECC and INFINAPO is a literal bookkeeping argument or constant of the native function owning its address. PARAM/GEOM, GETX, COMMNOUT, COGAOVFL, ABTCONIC and DELTIME are the lm-conic-timethet and lm-conic-kepler milestones\' (src/lm_conic_timethet.c, src/lm_conic_kepler.c). LODPHALF, KEPZERO and LDPOSMAX are read from the certified fixed-data table; the D1/n constants and DP1/4 are native literals. ITERATOR and LAMBERT are not yet ported.')
(R/'build/lm-conic-timerad-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled TIMERAD/APSIDES words on',len(rows),'source lines map to native literals in',len(owned),'functions',flush=True)
