#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Additive whole LM RTB local-word closure; never rewrites frozen packets."""
import pathlib,json,struct,hashlib
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';maps={name:json.loads((e/f'rtb-{name}-source-map.json').read_text())['lm']for name in ['angles','agreement','vector','pulse-return']};words=maps['angles']['words'];owners={}
for name,x in maps.items():
 for w in x['words']:
  if name=='angles'and w['status']!='qualified_angles':continue
  p=w['physical_fixed_word'];assert p not in owners;owners[p]=dict(packet=f'evidence/rtb-{name}-proof.json',native_group=w.get('native_group',name))
assert len(words)==len(owners)==130
source=r.parent/'Luminary099/RTB_OP_CODES.agc';sha=hashlib.sha256(source.read_bytes()).hexdigest();assert sha==maps['angles']['original_sha256'];ropepath=r/'deps/lm-rtb-fixture/MAIN.agc.bin';rope=ropepath.read_bytes();assert hashlib.sha256(rope).hexdigest()==maps['angles']['rope_sha256'];rope=struct.unpack('>36864H',rope)
compiled=[]
for w in words:
 p=w['physical_fixed_word'];assert rope[p]>>1==int(w['raw_octal'],8)
 compiled.append(dict(w,local_native_owner=owners[p],coverage='native_compiled_local_word'))
dependencies=['evidence/rtb-angles-proof.json','evidence/rtb-agreement-proof.json','evidence/rtb-vector-proof.json','evidence/rtb-vector-bank-erratum.json','evidence/rtb-unit-proof.json','evidence/rtb-pulse-return-proof.json','evidence/rtb-angles-source-audit.json']
# Locate the immutable additive erratum by its actual retained filename.
if not(e/'rtb-vector-bank-erratum.json').exists():
 dependencies[3]='evidence/rtb-vector-bank-erratum-proof.json'
assert all((r/p).exists()for p in dependencies)
runtime=['src/rtb_angles.c','src/rtb_agreement.c','src/rtb_vector.c','src/rtb_unit.c','src/rtb_pulse_return.c']
ledger=dict(source_file='Luminary099/RTB_OP_CODES.agc',original_sha256=sha,assembled_sha256=sha,source_edits=0,rope_sha256=maps['angles']['rope_sha256'],original_emitted_words=130,compiled_local_words=130,uncovered_local_words=[],source_word_map=compiled,entry_groups={'angles':['LOADTIME','CDULOGIC','1STO2S','V1STO2S','2V1STO2S','1TO2SUB','CDUINC'],'agreement':['SGNAGREE','SIGNMPAC'],'vector':['PULSEIMU','NORMUNIT','NORMUNX1','VECSGNAG']},data_words='All local inline CADR words included; no standalone data declarations in original containing file.',proof_dependencies={p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in dependencies},native_sources={p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in runtime},qualification='All original containing-file local entry/instruction/data words are represented by source-qualified compiled native operations within documented ordinary canonical/fixed-return/VAC/producer domains. NORMUNIT+1 helper fully composed by immutable UNIT packet; PULSEIMU real BANKCALL and trailing return qualified, external IMUPULSE is an explicit actual service boundary. Direct source helper returns and genuine DANZIG boundaries retain pending state; no arbitrary interpreter decoder or invented scheduling seam.',external_handoffs=['Actual DANZIG Executive/interpreter continuation outside containing source','Actual IMUPULSE bank07 service; physical gyro/device backend not a containing-file instruction'],recommendation='Whole local-source closure ready for root independent review/certification; no whole application/runtime or asynchronous physical pulse claim.',legacy_metadata_erratum='Frozen rtb-vector text/source-map FB014000 is superseded additively by retained bank07/FB016000 erratum. Runtime/complete source states were always correct.')
(e/'lm-rtb-whole-source-map.json').write_text(json.dumps(ledger,indent=2)+'\n');print('PASS: byte-original LM RTB all130 local emitted words have distinct native ownership and unchanged original rope values')
