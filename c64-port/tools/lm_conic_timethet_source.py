#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 CONIC_SUBROUTINES time-theta subroutine (src/lm_conic_timethet.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/ephemeris_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of the file's NEWSTATE
through INFINITY range (bank 12 2653-3211), ABTCONIC (12,3742-3744) and the
bank-4 PARAM/GEOM block (4,3064-3163) must then appear as a literal in the
native function that owns its address
(or a helper it calls), and every native phase must own at least one source
line. Writes build/lm-conic-timethet-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols,filename_alias
mission='Luminary099';name='CONIC_SUBROUTINES.agc';path=mission+'/'+name;original=R.parent/path
root=R/'build/timethet-source';assemble(mission,root);work=root/mission
includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')];alias=filename_alias(name,includes)
shutil.copyfile(original,work/alias);shutil.copyfile(R/'deps/lm-rtb-fixture/INTERPRETER.agc',work/'INTERPRETER.agc')
with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes()
if rope!=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes():raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
text=original.read_text();records=listing_records(work/'assembly.lst',{alias:path},{path:text},rope)
if {r['line']for r in records}!={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}:raise SystemExit('FAIL listing does not cover every source line')
# Address ranges (bank, first, last) and the native function that compiles them.
OWNERS=[('12',0o2653,0o2731,'ltt_newstate'),('12',0o2732,0o2743,'ltt_entry'),('12',0o2744,0o2746,'ltt_afterparam'),
 ('12',0o2747,0o2755,'ltt_commnout'),('12',0o2756,0o2761,'ltt_afterdt'),('12',0o2762,0o2763,'ltt_final'),('12',0o2764,0o2766,'ltt_cogaovfl'),
 ('12',0o2767,0o3012,'ltt_getx'),('12',0o3013,0o3030,'ltt_wloop'),('12',0o3031,0o3065,'ltt_polycoef'),('12',0o3066,0o3117,'ltt_xcommon'),
 ('12',0o3120,0o3121,'ltt_resetx2'),('12',0o3122,0o3126,'ltt_360check'),('12',0o3127,0o3150,'ltt_invrseqn'),('12',0o3151,0o3171,'ltt_1wloop'),
 ('12',0o3172,0o3204,'ltt_true360x'),('12',0o3205,0o3211,'ltt_infinity'),('12',0o3742,0o3744,'ltt_abtconic'),
 ('04',0o3064,0o3074,'ltt_param'),('04',0o3075,0o3127,'ltt_param2'),('04',0o3130,0o3153,'ltt_geom'),('04',0o3154,0o3157,'ltt_unitnorm'),
 ('04',0o3160,0o3161,'ltt_colinear'),('04',0o3162,0o3163,'ltt_havenorm')]
COVERED=lambda b,a:any(bb==b and lo<=a<=hi for bb,lo,hi,_ in OWNERS)
# Helpers a function reaches for words it passes on (the POLY table).
HELPERS={'ltt_polycoef':['constants'],'ltt_xcommon':['rvq'],'ltt_havenorm':['rvq'],'ltt_1wloop':['rtb_tpmode']}
native=(R/'src/lm_conic_timethet.c').read_text()
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
KEYS=('TIMETHET','PARAM','GEOM','UNITNORM','COLINEAR','HAVENORM','GETX','WLOOP','POLYCOEF','XCOMMON','INVRSEQN','1/WLOOP','TRUE360X','INFINITY','COMMNOUT','NEWSTATE','LAMENTER','ABTCONIC','RTNTT','RTNPRM','COGA','P','SNTH','CSTH','UR1','U2','UN','R1A','MAGVEC2','GEOMSGN','360SW','COGAFLAG','INFINFLG','NORMSW','RVSW','MUTABLE')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_path=path,source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_conic_timethet.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['12,2653-12,3211','12,3742-12,3744','04,3064-04,3163'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of NEWSTATE/LAMENTER, TIMETHET, COMMNOUT, COGAOVFL, GETX with its POLY series table, XCOMMON through INFINITY, ABTCONIC and the bank-4 PARAM/GEOM block is a literal bookkeeping argument or constant of the native function owning its address. DELTIME is the Kepler milestone\'s (lm_conic_kepler.c). MUTABLE is read from the certified fixed-data table; D1/4 through D1/256, 2PISC and the VLOAD* code are native literals. HAVENORM is compiled but unreachable from TIMETHET (PARAM clears NORMSW). The rest of the file (ITERATOR, TIMERAD, APSIDES, LAMBERT) is not yet ported.')
(R/'build/lm-conic-timethet-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled TIMETHET words on',len(rows),'source lines map to native literals in',len(owned),'functions',flush=True)
