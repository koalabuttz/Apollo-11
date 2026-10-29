#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain full canonical admission oracle and actual isolated C64 disk proof."""
import hashlib,json,lzma,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
owned=['include/lm_admission_image.h','src/lm_admission_image.c','tests/lm_admission_image_cases.h','tests/lm_admission_image_cases.c','tests/test_lm_admission_image.c','docs/LM-CANONICAL-ADMISSION.md','tools/lm_admission_image.mk','tools/package_lm_admission_image.py']
owned += ['tools/lm_admission_image_'+s for s in ['reference.c','snapshot.c','inputs.c','differential.py','target_build.py','vice_test.py']]
owned += ['platform/c64/lm_admission_image_'+s for s in ['fixture.h','diagnostic.c']]
def keep(path,name=None):
 dest=evidence/(name or path.name);shutil.copyfile(path,dest);owned.append(str(dest.relative_to(root)))
archive=build/'lm-admission-image-reference.snapshots.xz'
with lzma.open(archive,'rb')as f:decoded=hashlib.file_digest(f,'sha256').hexdigest()
assert decoded=='578771084570f6c3bf9c849443105d0307385cfe7c3b02ee551ac27e4e7a0ffe'
keep(archive)
for suffix in ['check.log','sanitize-check.log','guards.log','guards-sanitize.log','build.log','c64-result.log','vice.log','vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-hardware-stack.bin','modules.json']:keep(build/('lm-admission-image-'+suffix))
for ext in ['prg','map','d81']:keep(build/('lm-admission-image.'+ext))
for n in range(32):
 for ext in ['prg','map']:keep(build/f'lm-admission-image-{n}.{ext}')
keep(build/'lm_admission_image_modules.h')
assert (build/'lm-admission-image-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000400002')
assert sha(root/'../Luminary099/EXECUTIVE.agc')==sha(root/'deps/fixture/EXECUTIVE.agc')
sources=[p for p in owned if p.endswith('.c')and not p.endswith('_reference.c')]
deps={'src/agc_word.c','src/executive.c','src/p66_native.c','include/native_dispatch.h','tools/reference_runner.c','deps/fixture/MAIN.agc.bin','deps/fixture/MAIN.agc.symtab','deps/fixture/EXECUTIVE.agc','../Luminary099/EXECUTIVE.agc','deps/virtualagc/yaAGC/agc_engine.c','deps/virtualagc/yaAGC/agc_engine.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_platform.c','platform/c64/overlay_entry.S','src/overlay_runtime.c','tools/overlay_runtime_library.py','tools/overlay_build.py'}
text=subprocess.check_output(['gcc','-MM','-DP66_NATIVE_DISPATCH_ONLY','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+sources,cwd=root,text=True)
for line in text.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
proof={'scope':'Literal LM NOVAC/FINDVAC/SPVAC/SPVACIN through actual caller-return or BAILOUT1 entry; bounded initialized Executive contract; no admitted body, recovery or complete containing-file claim','states':16384,'words':2050,'decoded_sha256':decoded,'strict':'PASS','full_asan_ubsan':'PASS','invalid_domains_unchanged':22,'null_interfaces':2,'target_states':64,'target_mailbox':'a50000400002','software_stack_bytes':752,'hardware_stack_bytes':30,'extra_zp':0,'fixture_ids':'160..191 isolated diagnostic only','files':{p:sha(root/p)for p in owned},'dependencies':{p:sha(root/p)for p in sorted(deps)}}
p=evidence/'lm-admission-image-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(p.relative_to(root)));(build/'lm-admission-image-commit-files.txt').write_text('\n'.join(owned)+'\n');print('Frozen',len(owned),'files;16384 full original states and64 C64 states')
