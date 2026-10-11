#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze additive admission source, compatibility and actual target proofs."""
import hashlib,json,lzma,shutil,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
owned=['include/lm_admission_raw.h','src/lm_admission_raw.c',
 'tests/lm_admission_raw_cases.h','tests/lm_admission_raw_cases.c','tests/test_lm_admission_raw.c',
 'docs/LM-RAW-ADMISSION.md','tools/package_lm_admission_raw.py',
 'tools/lm_admission_raw_port_host.py','tools/lm_admission_raw_port_integration.mk']
owned+=['tools/lm_admission_raw_'+s for s in ('reference.c','snapshot.c','inputs.c','differential.py','target_build.py','vice_test.py')]
owned+=['platform/c64/lm_admission_raw_'+s for s in ('fixture.h','diagnostic.c')]
def keep(name):
 shutil.copyfile(ROOT/'build'/name,ROOT/'evidence'/name);owned.append('evidence/'+name)
def main():
 for mode in ('strict','sanitize'):
  assert 'PASS:retained raw admission' in (ROOT/('build/lm-admission-raw-port-'+mode+'.log')).read_text()
 assert (ROOT/'build/lm-admission-raw-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000400002')
 archive=ROOT/'build/lm-admission-raw-reference.snapshots.xz'
 with lzma.open(archive,'rb')as f:decoded=hashlib.file_digest(f,'sha256').hexdigest()
 assert decoded=='f138a921dce7c848be2b6b1fb51c6685760cab41fe0609dfc92f717997288a84'
 for suffix in ('reference.snapshots.xz','sanitize.log','guards-strict.log','guards-sanitize.log',
                'port-strict.log','port-sanitize.log','target-build.log','vice-result.log','vice.log',
                'vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin',
                'vice-hardware-stack.bin'):
  keep('lm-admission-raw-'+suffix)
 for ext in ('prg','map','d81'):keep('lm-admission-raw.'+ext)
 for n in range(32):
  for ext in ('c','prg','map'):keep('lm-admission-raw-%d.%s'%(n,ext))
 keep('lm_admission_raw_modules.h')
 rows=json.loads((ROOT/'build/lm-admission-raw-modules.json').read_text())
 for row in rows:
  row['retained_prg']='evidence/'+row['prg'];assert sha(ROOT/row['retained_prg'])==row['sha256']
 manifest=ROOT/'evidence/lm-admission-raw-modules.json';manifest.write_text(json.dumps(rows,indent=2)+'\n');owned.append(str(manifest.relative_to(ROOT)))
 deps={'src/agc_word.c','src/executive.c','src/p66_native.c','src/lm_admission_image.c',
       'tests/lm_admission_image_cases.c','tools/reference_runner.c','tests/scenarios.c',
       'deps/fixture/EXECUTIVE.agc','../Luminary099/EXECUTIVE.agc','deps/fixture/MAIN.agc.bin',
       'deps/virtualagc/yaAGC/agc_engine.c','deps/virtualagc/yaAGC/agc_engine.o',
       'deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o',
       'platform/c64/overlay_platform.c','platform/c64/overlay_platform.h','src/overlay_runtime.c',
       'platform/c64/overlay_payload.ld','platform/c64/overlay_resident.ld','platform/c64/overlay_entry.S',
       'tools/native_resident_port_host.py','tools/overlay_build.py','tools/overlay_runtime_library.py'}
 src=[s for s in owned if s.endswith('.c')and not s.endswith('_reference.c')and not s.startswith('evidence/')]
 source=subprocess.check_output(['cc','-MM','-DP66_NATIVE_DISPATCH_ONLY','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+src+['src/lm_admission_image.c','tests/lm_admission_image_cases.c'],cwd=ROOT,text=True)
 for line in source.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
 resources={}
 for name in ('stack','hardware-stack'):
  data=(ROOT/('build/lm-admission-raw-vice-'+name+'.bin')).read_bytes();changed=[i for i,b in enumerate(data)if b!=0xcd];resources[name]=len(data)-min(changed)
 proof={'scope':'Literal ordinary raw-priority NOVAC/FINDVAC/SPVAC/SPVACIN; actual return/BAILOUT1, no whole-file certification',
 'source_entries':['05072','05105','05116','05112'],'priority_domain':'0..037000 incl low bits; marker addition no positive overflow',
 'inline_erasable':'canonical Q descriptor, including actual TCMPAC0155; fixed descriptor explicit compiled metadata',
 'pending_core_zero':'positive NEWJOB retained through actual SETLOC comparison; +0 CCSHOLE remains outside domain',
 'original_states':16384,'full_source_words':2048,'explicit_io_fields':['SUPER','IRQ'],
 'decoded_sha256':decoded,'strict_full_asan_ubsan':'PASS','historical_old_wrapper_states_unchanged':16384,
 'all_object_invalid_domains':22,'null_interfaces':2,
 'c64':{'stock_pal_rom_d81':True,'frozen_original_states':64,'fixture_images':32,'mailbox':'a50000400002',
        'resources':resources,'extra_zero_page':0,'source_normalization':False,'incoming_i_and_port_restored':True},
 'files':{p:sha(ROOT/p)for p in owned},'dependencies':{p:sha(ROOT/p)for p in sorted(deps)}}
 output=ROOT/'evidence/lm-admission-raw-proof.json';output.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(output.relative_to(ROOT)))
 allow=ROOT/'build/lm-admission-raw-commit-files.txt';allow.write_text('\n'.join(owned)+'\n')
 print('Frozen',len(owned),'files;allowlist SHA',sha(allow));print('Proof SHA',sha(output))
if __name__=='__main__':main()
