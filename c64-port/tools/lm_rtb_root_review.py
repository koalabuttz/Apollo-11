#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independently audit all original LM RTB words and frozen native owners."""
from pathlib import Path
import hashlib,json,struct,subprocess
r=Path(__file__).resolve().parents[1];e=r/'evidence';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();x=json.loads((e/'lm-rtb-whole-source-map.json').read_text());source=r.parent/x['source_file'];baseline=subprocess.check_output(['git','show','911e5c0283c629c50cb97666f34065e8c07d71a5:'+x['source_file']],cwd=r);assert source.read_bytes()==baseline and sha(source)==x['original_sha256'];rope=e/'fixed-data-lm-original.rope.bin';assert sha(rope)==x['rope_sha256'];words=struct.unpack('>36864H',rope.read_bytes());seen=set();owners={};original=source.read_text().splitlines()
for w in x['source_word_map']:
 p=w['physical_fixed_word'];assert p not in seen;seen.add(p);bank=p//1024;wire=((bank^2)if bank<4 else bank)*1024+p%1024;assert words[wire]>>1==int(w['raw_octal'],8);assert int(w['bank'],8)==bank;assert int(w['address'],8)==02000+p%1024;assert original[w['original_line']-1].split('#',1)[0].strip();owner=w['local_native_owner'];owners[owner['packet']]=owners.get(owner['packet'],0)+1;assert owner['packet']in x['proof_dependencies']
assert len(seen)==130 and not x['uncovered_local_words']
for key in ['proof_dependencies','native_sources']:
 for name,h in x[key].items():assert sha(r/name)==h,name
result=dict(original_source_sha256=sha(source),rope_sha256=sha(rope),unique_original_local_words=len(seen),word_ownership=owners,source_unchanged_from_baseline=True,dependency_hashes_verified=True,whole_local_source_translation=True,whole_application_translation=False,reviewed_boundaries=x['external_handoffs'],qualification=x['qualification'])
print(json.dumps(result,indent=2))
