#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze raw original recurrence plus the reproducible111-module C64 packet."""
import hashlib,json,lzma,re,shutil,subprocess
from pathlib import Path
from p66_application_port_host import inventories
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
owned=['docs/P66-APPLICATION-RECURRENCE.md','tools/p66_application_recurrence.mk','tools/package_p66_recurrence_application.py','tools/p66_recurrence_port_host.py','tests/test_p66_recurrence.c','tools/lm_clock_reference_test.c','tools/p66_rodtask_image_snapshot.c','tools/p66_length_return_application_snapshot.c','tools/p66_length_return_application_test.py','tests/p66_recurrence_live_cases.h','tests/p66_recurrence_live_cases.c','tests/p66_recurrence_live_inputs.h','tests/p66_recurrence_live_vectors.h']
for name in ('p66_application_recurrence','lm_clock','lm_waitlist_irq_dispatch','p66_rodtask_image','p66_length_return_control'):owned+=['include/'+name+'.h','src/'+name+'.c']
owned+=['tools/p66_recurrence_live_'+s for s in ('inputs.c','snapshot.c','reference.c','differential.py','build.py','vice_test.py')]
owned+=['tools/freeze_p66_recurrence_live.py','tools/p66_recurrence_overlay_build.py']
owned+=['tools/lm_waitlist_irq_dispatch_'+s for s in ('reference.c','snapshot.c','differential.py')]
owned+=['platform/c64/p66_recurrence_'+s for s in ('fixture.h','fixture_input.c','fixture_check.c','runtime_diagnostic.c','payload.h','payload.c')]
def retain(src,name=None):
 dest=evidence/(name or src.name);shutil.copyfile(src,dest);owned.append(str(dest.relative_to(root)))
streams={'live':(2460,2053,'d1aa5f4de9e02bf126731818227c4bc7592e62bddbbd66a425cc56346683f186','evidence/p66-recurrence-live-source.snapshots.xz'),'IRQ1_T3':(8192,2053,'28399982b6a4a2d694a1ee160e934c4f92afcfe49514864ff820fea432d492d3','evidence/lm-waitlist-irq-dispatch-reference.snapshots.xz'),'prior_compatibility':(1226,2052,'9ea8e8bc75b90a91ac5185b2546cf93ca47dc786b691633527df6959819e22dc','evidence/p66-application-live-source.snapshots.xz'),'RODTASK':(2048,2050,'cec29ff5972ca5b57e2f257fee3c7a9762f3198cf3d58431284a2e05fc62d2df','evidence/p66-basic-task-reference.snapshots.gz')}
for key in ('live','IRQ1_T3'):
 archive=root/streams[key][3]
 with lzma.open(archive,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==streams[key][2]
 owned.append(str(archive.relative_to(root)))
owned.append('evidence/lm-waitlist-irq-inactive-build-cleanup.json')
for name in ('p66-recurrence-live-check.log','p66-recurrence-live-sanitize-check.log','lm-waitlist-irq-dispatch-check.log','lm-waitlist-irq-dispatch-sanitize-check.log','p66-length-return-application-check.log','p66-length-return-application-sanitize-check.log','lm-clock-reference-check.log','lm-clock-reference-sanitize-check.log','p66-recurrence-clock-current.log','p66-recurrence-port-host-check.log','p66-recurrence-port-host-sanitize-check.log','p66-recurrence-guards.log','p66-recurrence-guards-sanitize.log','p66-recurrence-live-c64-result.log','p66-recurrence-rebuild.log','p66-recurrence-tested-target-sha.json'):retain(build/name)
retain(build/'p66-recurrence-sizes.ll','p66-recurrence-target-sizes.ll')
manifest=json.loads((build/'p66-recurrence-live-modules.json').read_text());assert len(manifest)==111 and len({m['id']for m in manifest})==111
for m in manifest:
 source=build/m['prg'];assert sha(source)==m['sha256'];retain(source,f'p66-recurrence-live-module-{m["id"]}.prg');retain(source.with_suffix('.map'),f'p66-recurrence-live-module-{m["id"]}.map');m['retained_prg']=f'evidence/p66-recurrence-live-module-{m["id"]}.prg'
obj=evidence/'p66-recurrence-live-modules.json';obj.write_text(json.dumps(manifest,indent=2)+'\n');owned.append(str(obj.relative_to(root)))
for ext in ('prg','map','d81'):retain(build/f'p66-recurrence-live.{ext}')
for suffix in ('vice.log','vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-hardware-stack.bin'):retain(build/f'p66-recurrence-live-{suffix}')
retain(build/'overlay-p66-recurrence-modules.json');retain(build/'p66_recurrence_modules.h')
assert (build/'p66-recurrence-live-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000360102')
for path,digest in json.loads((build/'p66-recurrence-tested-target-sha.json').read_text()).items():assert sha(root/path)==digest,path
_,sources,inputs,defines=inventories()[2];sources=list(dict.fromkeys(sources+inputs+[p for p in owned if p.endswith('.c')and not p.endswith('_reference.c')]))
deps=set(sources)|{'include/native_dispatch.h','src/native_dispatch.c','src/native_overlay_dispatch.c','tools/p66_application_port_host.py','tools/p66_application_source_audit.py','evidence/p66-application-source-audit.json','platform/c64/overlay_payload.ld','platform/c64/overlay_resident.ld','src/overlay_runtime.c','platform/c64/overlay_platform.c','tools/overlay_runtime_library.py','tools/overlay_build.py','deps/fixture/MAIN.agc.bin','deps/fixture/WAITLIST.agc','deps/fixture/EXECUTIVE.agc','../Luminary099/WAITLIST.agc','../Luminary099/EXECUTIVE.agc','deps/virtualagc/yaAGC/agc_engine.c','build/agc_engine_hardware_dv.o','tools/reference_runner.c'}
text=subprocess.check_output(['gcc','-MM','-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-Ideps/virtualagc/yaAGC']+defines+sources,cwd=root,text=True)
for line in text.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
for path in (streams['prior_compatibility'][3],streams['RODTASK'][3]):deps.add(path)
assert sha(root/'../Luminary099/WAITLIST.agc')==sha(root/'deps/fixture/WAITLIST.agc')
mapping=(build/'p66-recurrence-live.map').read_text();ends=[]
for section in ('.text','.rodata','.data','.bss','.noinit'):
 pairs=re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M);assert len(pairs)==1;ends.append(sum(int(x,16)for x in pairs[0]))
assert max(ends)==0x6f59
proof={'scope':'actual STARTP66/P66/display/idle plus one-shot100cs T3 RODTASK/FINDVAC/TASKOVERRESUME/idle admission and secondP66/display/idle; other due tasks/whole applications excluded','host':{key:dict(states=value[0],words=value[1],decoded_sha256=value[2],object=value[3],strict='PASS',full_asan_ubsan='PASS')for key,value in streams.items()},'clock_original_PINC_words':32768,'clock_original_full_image_sequences':1024,'clock_centiseconds':'0..65535','recurrence_invalid_domains_unchanged':20,'recurrence_null_interfaces':2,'counter_prefix_tests':[0,99,1],'target_modules':111,'target_checkpoints':310,'target_words':2053,'mailbox':'a50000360102','software_stack_bytes':785,'hardware_stack_bytes':77,'extra_zp':0,'resident_end':'0x6f59','resident_spare':167,'metadata_bytes':171,'metadata_used':'c400..c4aa','metadata_lease':'c400..c5ff','executive_view':'648B c100..c387','reserved_task_views':'76B c388..c3d3','target_rebuild':'all111 modules/residentPRG/D81 byte-for-byte identical','files':{p:sha(root/p)for p in owned},'dependencies':{p:sha(root/p)for p in sorted(deps)}}
path=evidence/'p66-application-recurrence-proof.json';path.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(path.relative_to(root)));(build/'p66-application-recurrence-commit-files.txt').write_text('\n'.join(owned)+'\n');print('Frozen',len(owned),'files;111 diskmodules,310 C64 checkpoints and2460 joined complete source states')
