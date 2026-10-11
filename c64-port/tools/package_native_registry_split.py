#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze additive directory/resident adapter qualification without old rewrites."""
import hashlib,json,re,shutil,subprocess
from pathlib import Path
from native_registry_build import RESIDENT_SOURCES,RESIDENT_DEFINES
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
owned=['include/native_module_registry.h','src/native_module_registry.c','platform/c64/native_registry_payload.c','platform/c64/native_registry_directory_diagnostic.c','platform/c64/native_registry_recurrence_diagnostic.c','tests/native_registry_manifest.h','tests/test_native_module_registry.c','tools/native_registry_build.py','tools/native_registry_vice_test.py','tools/native_registry.mk','tools/native_registry_progress_audit.py','docs/NATIVE-MODULE-REGISTRY.md','include/p66_application_split.h','src/p66_application_split.c','platform/c64/p66_application_split_payload.h','platform/c64/p66_application_split_payload.c','platform/c64/p66_application_split_recurrence_diagnostic.c','tests/test_p66_application_split.c','tests/test_p66_application_split_pending.c','tools/p66_application_split_host.py','tools/p66_application_split_build.py','tools/p66_application_split_vice_test.py','tools/p66_application_split_directory_build.py','tools/p66_application_split_directory_vice_test.py','docs/P66-APPLICATION-SPLIT.md','tools/package_native_registry_split.py']
def keep(name):
 src=build/name;assert src.exists(),name;dst=evidence/name;shutil.copyfile(src,dst);owned.append(str(dst.relative_to(root)))
for prefix in ['native-registry-recurrence','p66-application-split-recurrence']:
 assert(build/(prefix+'-vice-mailbox.bin')).read_bytes()==bytes.fromhex('a50000360102'),prefix
 subprocess.run(['python3','tools/native_registry_progress_audit.py',str(build/(prefix+'-vice-monitor.log')),'--output',str(build/(prefix+'-progress.json'))],cwd=root,check=True)
 audit=json.loads((build/(prefix+'-progress.json')).read_text());assert audit['per_input_completed_source_checkpoints']=={'0':153,'1':157} and audit['source_failures']==0
 for suffix in ['.prg','.map','.d81','-c64-result.log','-vice.log','-vice-monitor.log','-vice.mon','-vice-mailbox.bin','-vice-shared.bin','-vice-stack.bin','-vice-hardware-stack.bin','-progress.json']:
  keep(prefix+suffix)
 for suffix in ['-c64-result.log','-vice.log','-vice-monitor.log','-vice.mon']:keep(prefix+'-prior-cap24b'+suffix)
for prefix,count in [('native-registry-directory',222),('p66-application-split-directory',226)]:
 assert(build/(prefix+'-vice-mailbox.bin')).read_bytes()==bytes([0xa5,0,0,count,0,2])
 for suffix in ['.prg','.map','-c64-result.log','-vice.log','-vice-monitor.log','-vice.mon','-vice-mailbox.bin','-vice-shared.bin','-vice-stack.bin','-vice-hardware-stack.bin']:keep(prefix+suffix)
for name in ['native-registry-148','p66-application-split-148','p66-application-split-155','p66-application-split-159']:
 for ext in ['.prg','.map']:keep(name+ext)
for name in ['native-registry-host.log','p66-application-split-host.log','p66-application-split-sanitize.log','native-registry-modules.json','native-registry-build.json','p66-application-split-modules.json','p66-application-split-build.json','native-registry-split-reproduce.json','native-registry-reproduce-build.log','p66-application-split-reproduce-build.log','native-registry-debug-monitor.log','native-registry-debug.log']:keep(name)
reproduced=json.loads((build/'native-registry-split-reproduce.json').read_text());assert len(reproduced)==9 and all(r['identical']for r in reproduced)
for name in ['native_registry_table.h','native_registry_bootstrap.h']:
 keep(name)
 src=build/'p66-application-split'/name;dst=evidence/('p66-application-split-'+name);shutil.copyfile(src,dst);owned.append(str(dst.relative_to(root)))
resources={}
for prefix in ['native-registry-recurrence','p66-application-split-recurrence']:
 stack=(build/(prefix+'-vice-stack.bin')).read_bytes();changed=[i for i,v in enumerate(stack)if v!=0xcd]
 hardware=(build/(prefix+'-vice-hardware-stack.bin')).read_bytes();used=[i for i,v in enumerate(hardware)if v!=0xcd]
 resources[prefix]={'software_stack_extent':len(stack)-min(changed),'hardware_stack_extent':len(hardware)-min(used),'additional_zero_page':0,'last_checkpoint_cycles':json.loads((build/(prefix+'-progress.json')).read_text())['last_event']['cycles']}
original=json.loads((evidence/'p66-recurrence-live-modules.json').read_text());assert len(original)==111
for m in original:assert sha(evidence/f'p66-recurrence-live-module-{m["id"]}.prg')==m['sha256']
source_files=list(RESIDENT_SOURCES)+['src/p66_application_split.c']+[p for p in owned if p.startswith('tests/')and p.endswith('.c')]
source_files=[p for p in source_files if not p.startswith('platform/')]
deps=set(source_files)|{'platform/c64/overlay_platform.c','platform/c64/overlay_platform.h','platform/c64/overlay_entry.S','platform/c64/overlay_payload.ld','platform/c64/overlay_resident.ld','tools/overlay_runtime_library.py','tools/overlay_build.py','evidence/p66-recurrence-live-modules.json','evidence/p66-application-recurrence-proof.json'}
text=subprocess.check_output(['gcc','-MM','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+['-D'+d for d in RESIDENT_DEFINES]+['-DP66_APPLICATION_SPLIT_DISPATCH_ONLY']+source_files,cwd=root,text=True)
for line in text.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
proof={'scope':'Additive checked native metadata overlay and returned prepare/finish callback composition; released bounded P66/application coverage retained, no new whole-application certificate','metadata_bootstrap_id':148,'adapter_ids':[155,159],'descriptor_counts':[111,113],'registry_lease':{'start':0xc600,'bytes':64,'target_record_bytes':27},'source_image':{'start':0x7000,'bytes':4096},'overlay':{'start':0x8000,'bytes':16384},'stack':{'start':0xc800,'bytes':2048},'resident_ends':{'prior':0x6f59,'directory_only':0x6f55,'fixed_lease_measurement_only':0x6f5b,'split':0x6bf6},'selected_fixed_lease_specialization':False,'split_resident_spare_bytes':1034,'frozen_stages_unchanged':111,'strict_and_full_sanitizers':'PASS','split_original_states':16998,'split_original_decoded_sha256':{'jobs':'bab1ff892632cd9c90d139229a9af041913c697301567a7ee1ac7dc6690bb22e','startup':'9ea8e8bc75b90a91ac5185b2546cf93ca47dc786b691633527df6959819e22dc','recurrence':'d1aa5f4de9e02bf126731818227c4bc7592e62bddbbd66a425cc56346683f186'},'resolver_domain_comparisons':196656,'higher_priority_pending_case':1137,'pending_irq_after_body_admission':'retained without extra source poll','c64':{'directory_descriptors':222,'extended_directory_descriptors':226,'directory_mailboxes':['a50000de0002','a50000e20002'],'per_resident_source_checkpoints':310,'per_input_source_checkpoints':[153,157],'full_resident_mailbox':'a50000360102','cap24b_attempt':'did not complete; retained failure logs, unchanged binaries; checkpoint traces established forward progress','qualified_cap_cycles':48000000000,'real_io':'stock PAL ROMs/D81 virtual IEC; checked metadata then real arithmetic/scheduler/basic stages','frames':'each overlay frame returns before next metadata/stage load','measured_resources':resources},'byte_identical_fresh_reproductions':reproduced,'files':{p:sha(root/p)for p in owned},'dependencies':{p:sha(root/p)for p in sorted(deps)}}
p='evidence/native-registry-split-proof.json';(root/p).write_text(json.dumps(proof,indent=2)+'\n');owned.append(p)
allow=build/'native-registry-split-commit-files.txt';allow.write_text('\n'.join(owned)+'\n');print('Frozen',len(owned),'files; allowlist SHA',sha(allow));print('Proof SHA',sha(root/p))
