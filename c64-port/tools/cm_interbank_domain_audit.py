#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Reconcile bounded original interbank proofs without rewriting historical audits."""
import hashlib,json,pathlib
R=pathlib.Path(__file__).resolve().parents[1]
def read(name):return json.loads((R/'evidence'/name).read_text())
prior=read('cm-interbank-coverage-audit.json');extended=read('cm-interbank-extended-report.json');callers=read('cm-fixed-compat-report.json');data=read('passive-storage-cm-target-proof.json')
source=R.parent/'Comanche055/INTER-BANK_COMMUNICATION.agc'
assert hashlib.sha256(source.read_bytes()).hexdigest()==prior['original_source_sha256']
assert extended['actual_executed_source_states']==901120 and extended['rejected_invalid_typed_inputs']==16384
assert callers['host_actual_caller_cases']==81920 and callers['host_complete_checkpoints']==163840
proofs=['cm-interbank-coverage-audit.json','cm-interbank-extended-report.json','cm-fixed-compat-report.json','passive-storage-cm-target-proof.json']
report=dict(original_source_sha256=prior['original_source_sha256'],instruction_count=prior['instruction_count'],lines=prior['lines'],native_control_entries=prior['native_control_entries'],complete_file_qualified=False,
 historical_audit=dict(path='evidence/cm-interbank-coverage-audit.json',qualification='historical bounded span/overflow findings retained unchanged'),
 qualified_extensions=dict(actual_original_interbank_states=extended['actual_executed_source_states'],source_stream_sha256=extended['source_stream_sha256'],raw_registers='all16-bit A/L corners and ordinary Q/saved descriptors; negative-overflow BANKJUMP is invalid typed descriptor and returns unsupported unchanged',
 actual_original_caller_cases=callers['host_actual_caller_cases'],actual_original_caller_checkpoints=callers['host_complete_checkpoints'],caller_stream_sha256=callers['source_snapshot_sha256'],passive_target_individually_compared_words=81920),
 actual_SUPDACAL_callers=[dict(x,passive_original_word_address_domain_closed=True,qualified_control='actual DSPFMEM or COMADRS entry through source-selected SUPDACAL/read/return boundary',algorithm_scope='This certifies these reader/control boundaries; no whole DSPFMEM display chain, ROPECHK algorithm or preserved-word executable credit') for x in prior['SUPDACAL_actual_mission_callers']],
 passive_data_contract=dict(original_rope_sha256='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061',physical_addresses=40960,original_assembled_words=36864,hardware_zero_holes=4096,access='original words strictly passive data, no decoder or dispatch',data_payload_loaded_bytes=74802,source_rope_bytes=73728),
 remaining_open_domains=['ordinary-Q and saved-return ABI excludes raw/high return or index words; complete mission caller closure is not yet certified','valid-state cross products beyond retained predicates and source corpus','arbitrary interrupt timing outside caller-owned producer exclusion','whole source caller algorithms after the retained DSPFMEM/COMADRS returns'],
 provenance=[dict(path='evidence/'+n,sha256=hashlib.sha256((R/'evidence'/n).read_bytes()).hexdigest()) for n in proofs],
 scope='updated original whole-line control inventory and explicit passive-data caller closure; complete-file qualification stays false')
(R/'evidence/cm-interbank-domain-audit.json').write_text(json.dumps(report,indent=2)+'\n')
print('SUPDACAL passive word address closure qualified; whole interbank file remains bounded')
