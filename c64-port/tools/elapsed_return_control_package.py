#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,re,shutil,lzma
from pathlib import Path
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
owned=['include/p66_elapsed_return_control.h','src/p66_elapsed_return_control.c','tests/elapsed_return_control_cases.h','tests/elapsed_return_control_cases.c','tests/test_elapsed_return_control.c','tools/elapsed_return_control_reference.c','tools/elapsed_return_control_snapshot.c','tools/elapsed_return_control_differential.py','tools/elapsed_return_control_target_build.py','tools/elapsed_return_control_vice_test.py','tools/elapsed_return_control_package.py','tools/elapsed_return_control.mk','platform/c64/elapsed_return_control_fixture.h','platform/c64/elapsed_return_control_diagnostic.c','docs/P66-ELAPSED-RETURN-CONTROL.md','evidence/elapsed-return-control-reference.snapshots.xz']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for suffix in ['.prg','.map','-host.log','-original.log','-target-build.log','-c64-result.log','-vice.log','-vice-monitor.log','-vice.mon','-vice-mailbox.bin','-vice-shared.bin','-vice-stack.bin','-vice-hardware-stack.bin']:
 name='elapsed-return-control'+suffix;src=build/name;assert src.exists(),name
 dst=evidence/name;shutil.copyfile(src,dst);owned.append('evidence/'+name)
assert(build/'elapsed-return-control-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000400002')
assert 'PASS:8192' in(build/'elapsed-return-control-host.log').read_text()
h=hashlib.sha256();count=0
with lzma.open(evidence/'elapsed-return-control-reference.snapshots.xz','rb')as f:
 for line in f:h.update(line);count+=1;assert len(line.split())==2051
assert count==8192 and h.hexdigest()=='65eb3ca920095de18784edfa16ab46727a1dbe1ddf3e33bcb2d1640847de90a6'
dep=['include/agc_word.h','include/agc_multiword.h','include/overlay_runtime.h','include/guidance_elapsed.h','src/agc_word.c','src/agc_multiword.c','src/guidance_elapsed.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tools/reference_runner.c','platform/c64/overlay_resident.ld','deps/fixture/MAIN.agc.bin']
proof={'scope':'Bounded P66 DSU initial-DAS source return-Q control; no scheduler lifecycle or containing-file closure','source_cases':count,'source_state_words':2050,'source_decoded_sha256':h.hexdigest(),'strict_and_full_sanitizers':'PASS','invalid_domains':12,'null_image':1,'c64_frozen_records':64,'c64_scope':'Frozen original Q records; full2048-word mutation guard, not repeated numerical output qualification','c64_mailbox':'a50000400002','software_stack_extent':3,'hardware_stack_extent':12,'additional_zero_page':0,'dependency_sha256':{p:sha(root/p)for p in dep},'historical_source_sha256':{'Luminary099/INTERPRETER.agc':sha(root.parent/'Luminary099/INTERPRETER.agc')},'files':{p:sha(root/p)for p in owned}}
p='evidence/elapsed-return-control-proof.json';(root/p).write_text(json.dumps(proof,indent=2)+'\n');owned.append(p)
allow=build/'elapsed-return-control-commit-files.txt';allow.write_text('\n'.join(owned)+'\n')
print(len(owned),'files; allowlist SHA',sha(allow));print('Proof SHA',sha(root/p))
