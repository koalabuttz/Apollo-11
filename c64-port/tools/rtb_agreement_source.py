#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""New selected containing-word qualification; reads frozen first-group map."""
import pathlib,json
r=pathlib.Path(__file__).resolve().parents[1];first=json.loads((r/'evidence/rtb-angles-source-map.json').read_text());out={}
for mission,x in first.items():
 selected=[]
 for old in x['words']:
  w=dict(old);a=int(w['address'],8)
  if (mission=='lm'and a in[0o3612,0o3613])or(mission=='cm'and a in[0o3544,0o3545]):group='SGNAGREE'
  elif 0o3712<=a<=0o3723:group='SIGNMPAC'
  elif mission=='cm'and 0o3660<=a<=0o3661:group='XNB_ECADR_data'
  elif mission=='cm'and 0o3662<=a<=0o3675:group='TRANSP1'
  elif mission=='cm'and 0o3676<=a<=0o3711:group='TRANSP2'
  else:continue
  w['native_group']=group;w['status']='qualified_agreement';selected.append(w)
 out[mission]=dict(source_file=x['source_file'],original_sha256=x['original_sha256'],rope_sha256=x['rope_sha256'],qualified_containing_words=len(selected),words=selected,prior_group_immutable='evidence/rtb-angles-source-map.json',containing_status='partial',source_helper='Actual TPAGREE including BRANCH and returnviaL(Q2); SIGNMPAC sourceSLOAD2/NEWMODE; sourceDANZIGentry notexecuted',fixed_data={'19376':'02713','19377':'02554'}if mission=='cm'else{})
assert out['lm']['qualified_containing_words']==12 and out['cm']['qualified_containing_words']==38
(r/'build/rtb-agreement-source-map.json').write_text(json.dumps(out,indent=2)+'\n');print({m:x['qualified_containing_words']for m,x in out.items()})
