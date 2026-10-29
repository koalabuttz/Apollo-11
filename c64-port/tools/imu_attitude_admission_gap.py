#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Add this packet's local words without rewriting frozen prior gap evidence."""
import pathlib,json,hashlib
r=pathlib.Path(__file__).resolve().parents[1]
prior=json.loads((r/'evidence/imu-mode-transfer-gap-map.json').read_text())
new=json.loads((r/'build/imu-attitude-admission-source-map.json').read_text())
for mission,gap in prior.items():
 added={w['physical_fixed_word']:w for w in new[mission]['words']}
 count=0
 for word in gap['words']:
  at=word['physical_fixed_word']
  if at not in added:continue
  assert word['raw_octal']==added[at]['raw_octal']
  assert not word['native_owners'],('overlapping source ownership',mission,at)
  word['native_owners']=[{'packet':'evidence/imu-attitude-admission-proof.json','group':added[at]['native_group']}]
  count+=1
 assert count==len(added)
 gap['previous_gap_dependency']={'path':'evidence/imu-mode-transfer-gap-map.json','sha256':hashlib.sha256((r/'evidence/imu-mode-transfer-gap-map.json').read_bytes()).hexdigest()}
 gap['native_covered_words']=sum(bool(w['native_owners'])for w in gap['words'])
 gap['remaining_words']=gap['original_emitted_words']-gap['native_covered_words']
 groups={}
 for word in gap['words']:
  if not word['native_owners']:groups.setdefault(word['current_local_group'],[]).append(word['physical_fixed_word'])
 gap['remaining_local_groups']={g:{'words':len(v),'physical_words':v}for g,v in groups.items()}
 print(mission,gap['native_covered_words'],'qualified;',gap['remaining_words'],'remaining')
(r/'build/imu-attitude-admission-gap-map.json').write_text(json.dumps(prior,indent=2)+'\n')
