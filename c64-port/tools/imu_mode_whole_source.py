#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Additive whole local IMU source ledger, without changing frozen packets."""
from pathlib import Path
import hashlib,json,struct,collections
R=Path(__file__).resolve().parents[1]
hashfile=lambda p:hashlib.sha256((R/p).read_bytes()).hexdigest()
gaps=json.loads((R/'evidence/imu-standby-gap-map.json').read_text())
review='evidence/imu-mode-whole-source-independent-review.json'
sources=['src/'+n+'.c'for n in ['imu_mode_control','imu_pulse','imu_pulse_service','imu_coarse','imu_mode_transfer','imu_pulse_end','imu_pulse_wake','imu_pulse_task','imu_pulse_task_register','imu_attitude_admission','imu_standby']]
for mission,expected in [('lm',660),('cm',656)]:
 g=gaps[mission];original=R.parent/g['source_file'];assert hashlib.sha256(original.read_bytes()).hexdigest()==g['original_sha256']
 rope=(R/f'deps/{mission}-imu-pulse-fixture/MAIN.agc.bin').read_bytes();assert hashlib.sha256(rope).hexdigest()==g['rope_sha256'];rom=struct.unpack('>36864H',rope)
 assert g['native_covered_words']==g['original_emitted_words']==expected and not g['remaining_words']
 seen=set();words=[];owners=collections.Counter()
 for w in g['words']:
  p=w['physical_fixed_word'];assert p not in seen;seen.add(p);bank=p//1024;index=p^2048 if bank<4 else p;assert rom[index]>>1==int(w['raw_octal'],8)
  assert w['native_owners'];owner=w['native_owners'];owners.update(x['packet']for x in owner)
  words.append(dict(original_line=w['line'],label=w['label'],operator=w['op'],arguments=w['arguments'],address=w['address'],physical_fixed_word=p,raw_octal=w['raw_octal'],local_native_owners=owner,coverage='native_compiled_local_word'))
 assert len(words)==expected
 proof={p:hashfile(p)for p in sorted(owners)};proof['evidence/imu-pulse-source-audit.json']=hashfile('evidence/imu-pulse-source-audit.json');proof['evidence/imu-standby-gap-map.json']=hashfile('evidence/imu-standby-gap-map.json')
 result=dict(source_file=g['source_file'],original_sha256=g['original_sha256'],assembled_sha256=g['assembled_sha256'],semantic_edits=g['semantic_edits'],indentation_repairs=g['indentation_repairs'],rope_sha256=g['rope_sha256'],original_emitted_words=expected,compiled_local_words=expected,uncovered_local_words=[],source_word_map=words,owner_word_counts_including_shared_helpers=dict(owners),shared_helper_words=sum(len(w['local_native_owners'])>1 for w in words),proof_dependencies=proof,native_sources={p:hashfile(p)for p in sources},independent_review=dict(path=review,sha256=hashfile(review)),qualification='All local original entry, branch, arithmetic and data words have compiled native ownership within their frozen canonical/compiled caller/helper-return/producer/device contracts. Local source control effects and real external service entries are qualified; no arbitrary interpreter or fixed-instruction decoding is introduced. Internal C returns retain source ownership, with Executive checks only at genuine original boundaries.',external_handoffs=['Actual ALARM/POODOO and mode/flag/phase/display/bank services','Actual WAITLIST/task return/sleep/wake and interrupt scheduling services','Actual FINETIME scaler sampling and SHORTMP/TPAGREE helper bodies from other source files','Asynchronous device pulse/coarse transport and physical standby transition'],recommendation='Whole local-source closure ready for independent root certification. No full mission/runtime or arbitrary physical-device/scaler context acceptance claim.')
 (R/f'evidence/{mission}-imu-mode-whole-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print(mission,len(words),'original words checked;',len(owners),'native owners')
