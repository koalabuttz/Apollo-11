#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate passive LM original-layout fixed words; never executable mappings."""
import pathlib,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];raw=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes();assert len(raw)==73728 and hashlib.sha256(raw).hexdigest()=='1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e'
audit=json.loads(gzip.decompress((R/'evidence/fixed-data-audit.json.gz').read_bytes()));cm=next(x for x in audit['missions'] if x['mission']=='Luminary099');assert cm['assembled_rope_sha256']==hashlib.sha256(raw).hexdigest()
for p in cm['source_files']:assert hashlib.sha256((R.parent/p['path']).read_bytes()).hexdigest()==p['sha256']
words=[]
for bank in range(36):
 index=bank^2 if bank<4 else bank;chunk=raw[index*2048:(index+1)*2048];words.extend(int.from_bytes(chunk[i:i+2],'big')>>1 for i in range(0,2048,2))
text='/* Passive original Luminary099 fixed-word compatibility data. Public-domain source words.\n * SHA256 original parity rope1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e.\n * These words may only be read as data: no decoder, execution or dispatch. */\n'
for group in range(6):
 begin=group*7168;chunk=words[begin:min(begin+7168,len(words))];text+=f'#if !defined(LM_FIXED_COMPAT_GROUP) || LM_FIXED_COMPAT_GROUP == {group}\nstatic const agc_word lm_fixed_compat_group_{group}[{len(chunk)}]={{\n'
 text+='\n'.join(','.join(str(x) for x in chunk[i:i+16])+',' for i in range(0,len(chunk),16))+'\n};\n#endif\n'
(R/'src/lm_fixed_compat_words.inc').write_text(text)
metadata={'original_layout_rope_sha256':hashlib.sha256(raw).hexdigest(),'original_layout_source_set_sha256':cm['source_set_sha256'],'original_packed_bytes':len(raw),'native_word_bytes':73728,'present_physical_banks':36,'unpopulated_zero_banks':4,'data_words':36864,'addressable_words':40960,'payload_groups':6,'words_per_full_payload_group':7168,'native_instruction_decoder':False,'executable_coverage_claim':False,'source_audit_artifact':'evidence/fixed-data-audit.json.gz','source_bank_file_permutation':'banks0..3 use file bank^2; banks4..35 unchanged; big-endian parity word>>1'}
(R/'build/lm-fixed-compat-source-audit.json').write_text(json.dumps(metadata,indent=2)+'\n');print(json.dumps(metadata,indent=2))
