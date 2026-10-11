#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain full canonical waitlist oracle and actual isolated C64 disk proof."""
import hashlib,json,lzma,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
owned=['include/lm_basic_change.h','src/lm_basic_change.c','tests/lm_basic_change_cases.h','tests/lm_basic_change_cases.c','tests/test_lm_basic_change.c','docs/LM-BASIC-CHANGE.md','tools/lm_basic_change.mk','tools/package_lm_basic_change.py']
owned += ['tools/lm_basic_change_'+s for s in ['reference.c','snapshot.c','inputs.c','differential.py','target_build.py','vice_test.py']]
owned += ['platform/c64/lm_basic_change_'+s for s in ['fixture.h','diagnostic.c']]
def keep(path,name=None):
 dest=evidence/(name or path.name);shutil.copyfile(path,dest);owned.append(str(dest.relative_to(root)))
archive=build/'lm-basic-change-reference.snapshots.xz'
with lzma.open(archive,'rb')as f:decoded=hashlib.file_digest(f,'sha256').hexdigest()
assert decoded=='ffb4bf5b82d4e07a1a377a3973bbad9d3ab5a2d6a8d49a22a1f64e828ac83329'
keep(archive)
for suffix in ['check.log','sanitize-check.log','guards.log','guards-sanitize.log','build.log','c64-result.log','vice.log','vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-hardware-stack.bin','modules.json']:keep(build/('lm-basic-change-'+suffix))
for ext in ['prg','map','d81']:keep(build/('lm-basic-change.'+ext))
for n in range(32):
 for ext in ['prg','map']:keep(build/f'lm-basic-change-{n}.{ext}')
keep(build/'lm_basic_change_modules.h')
assert (build/'lm-basic-change-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000400002')
assert sha(root/'../Luminary099/EXECUTIVE.agc')==sha(root/'deps/fixture/EXECUTIVE.agc')
sources=[p for p in owned if p.endswith('.c')and not p.endswith('_reference.c')]
deps={'src/agc_word.c','include/native_dispatch.h','tools/reference_runner.c','deps/fixture/MAIN.agc.bin','deps/fixture/MAIN.agc.symtab','deps/fixture/EXECUTIVE.agc','../Luminary099/EXECUTIVE.agc','deps/virtualagc/yaAGC/agc_engine.c','deps/virtualagc/yaAGC/agc_engine.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_platform.c','platform/c64/overlay_entry.S','src/overlay_runtime.c','tools/overlay_runtime_library.py','tools/overlay_build.py'}
text=subprocess.check_output(['gcc','-MM','-DP66_NATIVE_DISPATCH_ONLY','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+sources,cwd=root,text=True)
for line in text.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
proof={'scope':'Literal LM basic CHANG1/CHANJOB through selected basic DTCB or interpreter INTRSM NEWOPS; job bodies and complete containing-file claim excluded','states':16384,'words':2053,'decoded_sha256':decoded,'strict':'PASS','full_asan_ubsan':'PASS','invalid_domains_unchanged':25,'null_interfaces':3,'target_states':64,'target_mailbox':'a50000400002','software_stack_bytes':66,'hardware_stack_bytes':32,'extra_zp':0,'fixture_ids':'160..191 isolated diagnostic only','files':{p:sha(root/p)for p in owned},'dependencies':{p:sha(root/p)for p in sorted(deps)}}
p=evidence/'lm-basic-change-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(p.relative_to(root)));(build/'lm-basic-change-commit-files.txt').write_text('\n'.join(owned)+'\n');print('Frozen',len(owned),'files;16384 full original states and64 C64 states')
