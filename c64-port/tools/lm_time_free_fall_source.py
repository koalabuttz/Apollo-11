#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 TIME_OF_FREE_FALL (src/lm_time_free_fall.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/ephemeris_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of TIME_OF_FREE_FALL.agc
must then appear as a literal in the native function that owns its address
(or a helper it calls), and every native phase must own at least one source
line. Writes build/lm-time-free-fall-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,source_line,symbols,filename_alias
mission='Luminary099';name='TIME_OF_FREE_FALL.agc';path=mission+'/'+name;original=R.parent/path
root=R/'build/tff-source';assemble(mission,root);work=root/mission
includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')];alias=filename_alias(name,includes)
shutil.copyfile(original,work/alias);shutil.copyfile(R/'deps/lm-rtb-fixture/INTERPRETER.agc',work/'INTERPRETER.agc')
with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes()
if rope!=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes():raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
text=original.read_text();records=listing_records(work/'assembly.lst',{alias:path},{path:text},rope)
if {r['line']for r in records}!={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}:raise SystemExit('FAIL listing does not cover every source line')
# Bank-27 address ranges and the native function that compiles them.
OWNERS=[(0o3360,0o3360,'ltf_entry'),(0o3361,0o3420,'ltf_conmu'),(0o3421,0o3426,'ltf_conmu_end'),(0o3427,0o3460,'ltf_rpra'),
 (0o3461,0o3462,'ltf_maxra'),(0o3463,0o3464,'ltf_rapo'),(0o3465,0o3467,'ltf_calctper'),(0o3470,0o3471,'ltf_calctff'),
 (0o3472,0o3514,'ltf_terms'),(0o3515,0o3525,'ltf_qterm'),(0o3526,0o3534,'ltf_xterm'),(0o3535,0o3540,'ltf_saveden'),
 (0o3541,0o3547,'ltf_fallback'),(0o3550,0o3560,'ltf_inbound'),(0o3561,0o3572,'ltf_xtest'),(0o3573,0o3610,'ltf_series'),
 (0o3611,0o3630,'ltf_poly'),(0o3631,0o3640,'ltf_scale'),(0o3641,0o3644,'ltf_endtff'),(0o3645,0o3647,'ltf_negtff'),
 (0o3650,0o3650,'ltf_maxtff1'),(0o3651,0o3652,'ltf_maxtff'),(0o3653,0o3654,'ltf_tffell'),(0o3655,0o3673,'ltf_tffel1'),
 (0o3674,0o3734,'ltf_ellipse'),(0o3735,0o3753,'rtb_tx'),(0o3754,0o3771,'constants')]
# Helpers a function reaches for words it passes on (RVQ, BOVB TCDANZIG,
# RTB T(X) and its table).
HELPERS={'ltf_conmu_end':['rvq'],'ltf_rapo':['rvq'],'ltf_fallback':['rvq'],'ltf_endtff':['rvq'],'ltf_rpra':['bovb_danzig'],'ltf_qterm':['bovb_danzig'],
 'ltf_poly':['rtb_tx'],'ltf_ellipse':['rtb_tx'],'rtb_tx':['constants']}
native=(R/'src/lm_time_free_fall.c').read_text()
def body(name):
 if name=='constants':
  # File-level constant tables (bank 27 constants, R300K and T(X)).
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
 bank,addr=r['assembled_address'].split(',');assert bank=='27',r;a=int(addr,8)
 owner=[o for lo,hi,o in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f"FAIL address {addr} has no single native owner")
 for w in r['words']:
  if w not in literals(owner[0]):raise SystemExit(f"FAIL line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {owner[0]}")
 owned.add(owner[0]);rows.append(dict(line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=owner[0]))
missing={o for _,_,o in OWNERS}-owned
if missing:raise SystemExit('FAIL native phases own no source: '+', '.join(sorted(missing)))
KEYS=('TFFCONIC','TFFCONMU','TFFRP/RA','CALCTPER','CALCTFF','T(X)','PI/16','NEARONE','R300K','TFFZEROS','TFF1/4','RONE','VONE','TFFSW','SR30.1')
s=symbols((work/'assembly.lst').read_text())
result=dict(source_path=path,source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_time_free_fall.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word is a literal bookkeeping argument or constant of the native function owning its address. TFF1/4 and TFFZEROS (bank 23) are read from the certified fixed-data table; the bank-27 constants and the T(X) table are native literals.')
(R/'build/lm-time-free-fall-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled words on',len(rows),'source lines map to native literals in',len(owned),'functions',flush=True)
