#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independently check every original ALARM line, word and frozen proof."""
from pathlib import Path
import hashlib,json,struct,subprocess,collections
r=Path(__file__).resolve().parents[1];e=r/'evidence';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
x=json.loads((e/'cm-alarm-wholefile-review.json').read_text());f=x['files'][0];source=r.parent/'Comanche055'/f['file'];base=subprocess.check_output(['git','show','911e5c0283c629c50cb97666f34065e8c07d71a5:Comanche055/'+f['file']],cwd=r)
assert source.read_bytes()==base and sha(source)==f['original_sha256'];lines=source.read_text().splitlines();assert len(lines)==230 and len(f['lines'])==230
rope=e/'fixed-data-cm-original.rope.bin';assert sha(rope)==x['original_layout_rope_sha256'];words=struct.unpack('>36864H',rope.read_bytes());seen=set();roles=collections.Counter()
for v in f['lines']:
 assert lines[v['line']-1]==v['original_text'];roles[v['classification']]+=1
 if 'assembled_words_octal'not in v:continue
 adr=v['address_octal']
 if ','in adr:bk,a=map(lambda z:int(z,8),adr.split(','));off=a-0o2000
 else:
  a=int(adr,8);assert 0o4000<=a<=0o7777;bk=a//1024;off=a%1024
 for i,value in enumerate(v['assembled_words_octal']):
  physical=bk*1024+off+i;assert physical not in seen;seen.add(physical);wire=((bk^2)if bk<4 else bk)*1024+off+i;assert words[wire]>>1==int(value,8)
assert len(seen)==108 and roles['compiled instruction']==94 and roles['immutable data']==14 and x['complete_file_qualification_recommended']and not x['open_domains']
checked=0
for proof in f['bounded_proof_files']:
 for path,hashkey in [('source_proof','source_proof_sha256'),('target_report','target_report_sha256')]:assert sha(r/proof[path])==proof[hashkey];checked+=1
 original=json.loads((r/proof['source_proof']).read_text());target=json.loads((r/proof['target_report']).read_text());assert original['snapshot_sha256']==target['source_stream_sha256']==proof['original_stream_sha256'];assert original['original_layout_rope_sha256']==x['original_layout_rope_sha256'];assert target['mailbox_hex']==proof['actual_target_mailbox']
 ledger=e/(proof['prefix']+'-evidence.sha256')
 if ledger.exists():
  for record in ledger.read_text().splitlines():
   h,path=record.split(maxsplit=1);assert sha(r/path)==h,path;checked+=1
assert sha(e/'cm-alarm-independent-source-review.txt')=='ec314b7846fad490f7de7b1583229fbc0212fabc48704b38677d958acc634d61'
print(json.dumps(dict(source_unchanged_from_baseline=True,source_sha256=sha(source),original_lines_checked=230,unique_emitted_words_checked=108,instruction_words=94,data_words=14,classification_counts=dict(roles),frozen_dependencies_checked=checked,independent_peer_review_sha256=sha(e/'cm-alarm-independent-source-review.txt'),complete_local_source=True,complete_application=False,compiled_caller_contract=x['compiled_caller_contract'],external_program_boundaries=x['external_program_acceptance_not_claimed']),indent=2))
