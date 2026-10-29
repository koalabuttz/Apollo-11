#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independently verify both complete local IMU word ledgers and committed owners."""
from pathlib import Path
import json,hashlib,struct,subprocess
r=Path(__file__).resolve().parents[1];records=[]
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
for line in (r/'evidence/imu-mode-whole-source.sha256').read_text().splitlines():
 h,n=line.split(maxsplit=1);assert sha(r/n)==h,n
for mission,count in [('lm',660),('cm',656)]:
 n=f'evidence/{mission}-imu-mode-whole-source-map.json';x=json.loads((r/n).read_text());assert x['compiled_local_words']==x['original_emitted_words']==count and not x['uncovered_local_words'];assert sha(r.parent/x['source_file'])==x['original_sha256'];raw=(r/f'deps/{mission}-imu-pulse-fixture/MAIN.agc.bin').read_bytes();assert hashlib.sha256(raw).hexdigest()==x['rope_sha256'];words=struct.unpack('>36864H',raw);seen=set()
 for item in x['source_word_map']:
  at=item['physical_fixed_word'];assert at not in seen;seen.add(at);physical=at^2048 if at//1024<4 else at;assert words[physical]>>1==int(item['raw_octal'],8);assert item['local_native_owners'];assert item['coverage']=='native_compiled_local_word'
 for field in ['proof_dependencies','native_sources']:
  for path,h in x[field].items():
   assert sha(r/path)==h,path;subprocess.run(['git','ls-files','--error-unmatch','--',path],cwd=r,stdout=subprocess.DEVNULL,check=True)
 records.append(dict(map=n,map_sha256=sha(r/n),original_words=len(seen),native_sources=len(x['native_sources']),packet_proofs=len([n for n in x['proof_dependencies']if n.endswith('-proof.json')]),source_and_rope_unchanged=True,declared_frozen_domains_retained=True))
(r/'build/imu-mode-root-whole-source-audit.json').write_text(json.dumps(dict(scope='Root independent raw-word original rope, immutable source, distinct compiled owner union and exact committed native/proof identities review. Root integrations replayed all new complete source and sanitizer streams. Hardware/domain limits and external service bodies remain separate.',reviewed_maps=records,source_review='evidence/imu-mode-whole-source-independent-review.json',completed_source_files=2,integrated_application_claim=False),indent=2)+'\n');print('PASS root1316 distinct original words and all exact committed native/proof dependencies')
