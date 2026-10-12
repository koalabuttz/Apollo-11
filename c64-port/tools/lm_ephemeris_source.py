#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the native
Luminary099 LSPOS/LUNVEL (src/lm_ephemeris.c).

Reassembles the unchanged Luminary099 ephemerides with the pinned yaYUL
(tools/ephemeris_source.py, which also checks the rope against the certified
fixed-data rope) and requires the fresh LM line/word map to equal the
retained one. Every assembled word of the file must then appear as a literal
in the native function that owns its address, and every native phase must
own at least one source line."""
import hashlib,json,pathlib,re,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run([sys.executable,'tools/ephemeris_source.py'],cwd=R,check=True,stdout=subprocess.DEVNULL)
fresh=json.loads((R/'build/ephemeris-source-map.json').read_text())['LM'];kept=json.loads((R/'evidence/ephemeris-source-map.json').read_text())['LM']
if fresh['source_line_word_map']!=kept['source_line_word_map']or fresh['rope_sha256']!=kept['rope_sha256']or fresh['source_sha256']!=kept['source_sha256']:raise SystemExit('FAIL fresh assembly differs from the retained source map')
# Bank-15 address ranges and the native function that compiles them.
OWNERS=[(0o3664,0o3677,'lme_entry'),(0o3700,0o3721,'lme_posita'),(0o3722,0o3742,'lme_positd'),(0o3770,0o3774,'lme_positd'),
 (0o3743,0o3751,'lme_positf'),(0o3752,0o3752,'vdef'),(0o3753,0o3755,'lme_moon_unit'),(0o3756,0o3762,'lme_sun_angles'),
 (0o3763,0o3763,'vdef'),(0o3764,0o3767,'lme_sun_unit'),(0o3775,0o3775,'lme_entry')]
# Helpers a function reaches for words it passes on (SIN/PDDL pairs, the
# shared MXV/UNIT/STORE body and the POSITA/POSITE DLOAD/STORE).
HELPERS={'lme_positf':['trig_pddl'],'lme_sun_angles':['trig_pddl'],'lme_moon_unit':['vector'],'lme_sun_unit':['vector'],'lme_posita':['dload_frontier'],'lme_positd':['dload_frontier']}
native=(R/'src/lm_ephemeris.c').read_text()
def body(name):
 m=re.search(r'^(?:static )?uint8_t '+name+r'\([^)]*\)\{',native,re.M)
 if not m:raise SystemExit('FAIL no native function '+name)
 depth=0
 for i in range(m.start(),len(native)):
  depth+=native[i]=='{';depth-=native[i]=='}'
  if native[i]=='}'and depth==0:return native[m.start():i+1]
def literals(name):
 text=body(name)+''.join(body(h)for h in HELPERS.get(name,[]))
 return{int(x,8)for x in re.findall(r'\b0[0-7]*\b',text)}
rows=[];owned=set()
for r in kept['source_line_word_map']:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');assert bank=='15',r;a=int(addr,8)
 owner=[o for lo,hi,o in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f"FAIL address {addr} has no single native owner")
 for w in r['words']:
  if w not in literals(owner[0]):raise SystemExit(f"FAIL line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {owner[0]}")
 owned.add(owner[0]);rows.append(dict(line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=owner[0]))
missing={o for _,_,o in OWNERS}-owned
if missing:raise SystemExit('FAIL native phases own no source: '+', '.join(sorted(missing)))
result=dict(source_path=kept['source_path'],source_sha256=kept['source_sha256'],rope_sha256=kept['rope_sha256'],native_path='src/lm_ephemeris.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols=kept['symbols'],line_map=rows,
 note='Fresh yaYUL assembly of the unchanged file equals the retained map; every word is a literal bookkeeping argument of the native function owning its address. Constants (KONMAT, RATESP, VAL67, CSTODAY, RCB-13) are read from the certified fixed-data tables, not copied.')
(R/'build/lm-ephemeris-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled words on',len(rows),'source lines map to native literals in',len(owned),'functions',flush=True)
