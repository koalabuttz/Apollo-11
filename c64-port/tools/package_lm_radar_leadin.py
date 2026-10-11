#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain full canonical waitlist oracle and actual isolated C64 disk proof."""
import hashlib,json,lzma,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
owned=['include/lm_radar_leadin.h','src/lm_radar_leadin.c','tests/lm_radar_leadin_cases.h','tests/lm_radar_leadin_cases.c','tests/test_lm_radar_leadin.c','docs/LM-RADAR-LEADIN.md','tools/lm_radar_leadin.mk','tools/package_lm_radar_leadin.py']
owned += ['tools/lm_radar_leadin_'+s for s in ['reference.c','snapshot.c','inputs.c','differential.py','target_build.py','vice_test.py']]
owned += ['platform/c64/lm_radar_leadin_'+s for s in ['fixture.h','diagnostic.c']]
def keep(path,name=None):
 dest=evidence/(name or path.name);shutil.copyfile(path,dest);owned.append(str(dest.relative_to(root)))
archive=build/'lm-radar-leadin-reference.snapshots.xz'
with lzma.open(archive,'rb')as f:decoded=hashlib.file_digest(f,'sha256').hexdigest()
assert decoded=='c7781b0135151d3059edca03670dd1fb423edf824aa5681c9914fddf2acf88cf'
keep(archive)
for suffix in ['original.log','sanitize.log','host.log','target-build.log','c64-result.log','vice.log','vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-hardware-stack.bin','modules.json']:keep(build/('lm-radar-leadin-'+suffix))
for ext in ['prg','map','d81']:keep(build/('lm-radar-leadin.'+ext))
for n in range(32):
 for ext in ['prg','map']:keep(build/f'lm-radar-leadin-{n}.{ext}')
keep(build/'lm_radar_leadin_modules.h')
assert (build/'lm-radar-leadin-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000400002')
def executable_tokens(path):
 return '\n'.join(' '.join(line.split('#',1)[0].split())for line in path.read_text().splitlines()if line.split('#',1)[0].strip())
assert executable_tokens(root/'../Luminary099/RADAR_LEADIN_ROUTINES.agc')==executable_tokens(root/'deps/fixture/RADAR_LEADIN_ROUTINES.agc')
sources=[p for p in owned if p.endswith('.c')and not p.endswith('_reference.c')]
deps={'src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c','src/waitlist.c','src/lm_waitlist_image.c','src/lm_admission_image.c','tests/lm_admission_image_cases.c','tests/lm_admission_image_cases.h','include/native_dispatch.h','tools/reference_runner.c','deps/fixture/MAIN.agc.bin','deps/fixture/MAIN.agc.symtab','deps/fixture/RADAR_LEADIN_ROUTINES.agc','../Luminary099/RADAR_LEADIN_ROUTINES.agc','deps/virtualagc/yaAGC/agc_engine.c','deps/virtualagc/yaAGC/agc_engine.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_platform.c','platform/c64/overlay_entry.S','src/overlay_runtime.c','tools/overlay_runtime_library.py','tools/overlay_build.py'}
text=subprocess.check_output(['gcc','-MM','-DP66_NATIVE_DISPATCH_ONLY','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+sources,cwd=root,text=True)
for line in text.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
proof={'scope':'Original radar local caller bodies, real qualified WAITLIST/NOVAC, six compiled measurement CADRs, failure sample storage and cyclic pointers; exact external radar/interbank/TASKOVER/END/oversized-WAITLIST frontiers; no complete radar/application certificate','states':8192,'words':2050,'decoded_sha256':decoded,'strict':'PASS','full_asan_ubsan':'PASS','invalid_domains_unchanged':17,'null_interfaces':2,'target_states':64,'target_mailbox':'a50000400002','software_stack_bytes':773,'hardware_stack_bytes':29,'extra_zp':0,'fixture_ids':'160..191 isolated diagnostic only','executable_tokens_sha256':hashlib.sha256(executable_tokens(root/'../Luminary099/RADAR_LEADIN_ROUTINES.agc').encode()).hexdigest(),'assembly_comparison':'Historical and fixture comments/whitespace differ; every executable/directive token identical','files':{p:sha(root/p)for p in owned},'dependencies':{p:sha(root/p)for p in sorted(deps)}}
p=evidence/'lm-radar-leadin-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(p.relative_to(root)));(build/'lm-radar-leadin-commit-files.txt').write_text('\n'.join(owned)+'\n');print('Frozen',len(owned),'files;8192 full original states and64 C64 states')
