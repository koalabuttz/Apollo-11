#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 CONIC_SUBROUTINES Lambert subroutine (src/lm_conic_lambert.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/ephemeris_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of the file's ITERATOR
(bank 12 2536-2643), LAMENTER (12,2701-2731) and LAMBERT through TARGETV
(12,3212-3546) ranges must then appear as a literal in
the native function that owns its address
(or a helper it calls), and every native phase must own at least one source
line. Writes build/lm-conic-lambert-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols,filename_alias
mission='Luminary099';name='CONIC_SUBROUTINES.agc';path=mission+'/'+name;original=R.parent/path
root=R/'build/lambert-source';assemble(mission,root);work=root/mission
includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')];alias=filename_alias(name,includes)
shutil.copyfile(original,work/alias);shutil.copyfile(R/'deps/lm-rtb-fixture/INTERPRETER.agc',work/'INTERPRETER.agc')
with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes()
if rope!=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes():raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
text=original.read_text();records=listing_records(work/'assembly.lst',{alias:path},{path:text},rope)
if {r['line']for r in records}!={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}:raise SystemExit('FAIL listing does not cover every source line')
# Address ranges (bank, first, last) and the native function that compiles them.
OWNERS=[('12',0o2536,0o2561,'lla_iterator'),('12',0o2562,0o2570,'lla_sgncheck'),('12',0o2571,0o2577,'lla_mincheck'),
 ('12',0o2600,0o2605,'lla_modngdel'),('12',0o2606,0o2617,'lla_firstime'),('12',0o2620,0o2624,'lla_posdel'),
 ('12',0o2625,0o2631,'lla_maxcheck'),('12',0o2632,0o2633,'lla_delok'),('12',0o2634,0o2635,'lla_newdel'),
 ('12',0o2636,0o2643,'lla_modpsdel'),('12',0o2701,0o2731,'lla_targetv'),
 ('12',0o3212,0o3234,'lla_lambert'),('12',0o3235,0o3276,'lla_aftergeom'),('12',0o3277,0o3301,'lla_uplim'),
 ('12',0o3302,0o3317,'lla_maxcoga'),('12',0o3320,0o3327,'lla_mincoga'),('12',0o3330,0o3362,'lla_lambloop'),
 ('12',0o3363,0o3372,'lla_aftergetx'),('12',0o3373,0o3406,'lla_afterdt'),('12',0o3407,0o3416,'lla_afteriter'),
 ('12',0o3417,0o3421,'lla_negp'),('12',0o3422,0o3425,'lla_hienergy'),('12',0o3426,0o3436,'lla_commonlm'),
 ('12',0o3437,0o3441,'lla_bigtime'),('12',0o3442,0o3447,'lla_loenergy'),('12',0o3450,0o3462,'lla_suffchek'),
 ('12',0o3463,0o3466,'lla_360lamb'),('12',0o3467,0o3502,'lla_noguess'),('12',0o3503,0o3505,'lla_lolim'),
 ('12',0o3506,0o3540,'lla_initv'),('12',0o3541,0o3546,'lla_targetv')]
COVERED=lambda b,a:any(bb==b and lo<=a<=hi for bb,lo,hi,_ in OWNERS)
# Helpers a function reaches for words it passes on.
HELPERS={'lla_modngdel':['dp910_newdel'],'lla_modpsdel':['dp910_newdel'],'lla_hienergy':['bound','setpd0'],
 'lla_loenergy':['bound','setpd0'],'lla_360lamb':['setpd0'],'lla_targetv':['lamenter']}
native=(R/'src/lm_conic_lambert.c').read_text()
def body(name,native=native):
 if name=='constants':
  # File-level constant tables (the CONICS1 constants).
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
# The whole file: every other assembled word belongs to the Kepler,
# time-theta or time-radius milestone, whose source maps (each checked
# against the current native file) list it, or is a CONICS1 constant
# (bank 4 03027-03063) that the native files carry as literal tables, or
# HAVENORM's RVQ (04,3164), compiled in ltt_havenorm and reached only from
# LAMBERT (NORMSW set).
milestones={}
for m in('kepler','timethet','timerad'):
 doc=json.loads((R/f'evidence/lm-conic-{m}/lm-conic-{m}-source-map.json').read_text())
 if doc['source_sha256']!=hashlib.sha256(original.read_bytes()).hexdigest()or doc['native_sha256']!=hashlib.sha256((R/doc['native_path']).read_bytes()).hexdigest():
  raise SystemExit(f'FAIL lm-conic-{m} source map is stale')
 for row in doc['line_map']:milestones[row['address']]=(f'lm-conic-{m}',row['words'])
timethet=(R/'src/lm_conic_timethet.c').read_text();kepler=(R/'src/lm_conic_kepler.c').read_text()
tables={int(x,8)for text in(native,timethet,kepler)for x in re.findall(r'\b0[0-7]*\b',body('constants',text))}
havenorm={int(x,8)for x in re.findall(r'\b0[0-7]*\b',body('ltt_havenorm',timethet)+body('rvq',timethet))}
mine={r['address']for r in rows};whole={}
for r in records:
 if not r['words']:continue
 at=r['assembled_address'];bank,addr=at.split(',');a=int(addr,8);words=[f'{w:05o}'for w in r['words']]
 if at in mine:owner='lm-conic-lambert'
 elif at in milestones:
  owner,listed=milestones[at]
  if listed!=words:raise SystemExit(f'FAIL {at} words differ from the {owner} source map')
 elif bank=='04'and 0o3027<=a<=0o3063 and all(w in tables for w in r['words']):owner='CONICS1 constant tables'
 elif at=='04,3164'and all(w in havenorm for w in r['words']):owner='lm-conic-timethet (ltt_havenorm)'
 else:raise SystemExit(f"FAIL line {r['line']} ({at} {r['op']} {r['arguments']}) has no native owner in the whole file")
 whole[owner]=whole.get(owner,0)+len(r['words'])
KEYS=('LAMBERT','ITERATOR','LAMENTER','TARGETV','SUFFCHEK','360LAMB','INITV','RTNLAMB','R1VEC','R2VEC','TDESIRED','GEOMSGN','UN','VTARGTAG','VTARGET','COGA','SOLNSW','SLOPESW','GUESSW','NORMSW','ORDERSW','EPSILONL','TERRLAMB','BEE17','BEE19','COGUPLIM','COGLOLIM','ONEBIT','D1/1024')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_path=path,source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_conic_lambert.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['12,2536-12,2643','12,2701-12,2731','12,3212-12,3546'],
 whole_file=dict(assembled_words=sum(whole.values()),words_by_owner=dict(sorted(whole.items()))),
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of ITERATOR, LAMENTER and LAMBERT through TARGETV is a literal bookkeeping argument or constant of the native function owning its address. GEOM, GETX and DELTIME are the lm-conic-timethet and lm-conic-kepler milestones\' (src/lm_conic_timethet.c, src/lm_conic_kepler.c); CHECKCTR (12,2644-2652) is compiled inline at its RTB from the Kepler milestone\'s words; LAMENTER is compiled here as well as in the time-theta NEWSTATE step. MUTABLE and ONE are read from the certified fixed-data table; the CONICS1 constants are native literals.')
(R/'build/lm-conic-lambert-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled LAMBERT/ITERATOR words on',len(rows),'source lines map to native literals in',len(owned),'functions; whole file',result['whole_file'],flush=True)
