#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze additive admission source, compatibility and actual target proofs."""
import hashlib,json,lzma,shutil,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
owned=['include/lm_spectest_handoff.h','src/lm_spectest_handoff.c',
 'tests/lm_spectest_handoff_cases.h','tests/lm_spectest_handoff_cases.c','tests/test_lm_spectest_handoff.c',
 'docs/LM-SPECTEST-HANDOFF.md','tools/package_lm_spectest_handoff.py',
 'tools/lm_spectest_handoff_port_host.py','tools/lm_spectest_handoff_port_integration.mk']
owned+=['tools/lm_spectest_handoff_'+s for s in ('reference.c','snapshot.c','inputs.c','differential.py','target_build.py','vice_test.py')]
owned+=['platform/c64/lm_spectest_handoff_'+s for s in ('fixture.h','diagnostic.c')]
def keep(name):
 shutil.copyfile(ROOT/'build'/name,ROOT/'evidence'/name);owned.append('evidence/'+name)
def main():
 for mode in ('strict','sanitize'):
  assert 'PASS:retained SPECTEST handoff' in (ROOT/('build/lm-spectest-handoff-port-'+mode+'.log')).read_text()
 assert (ROOT/'build/lm-spectest-handoff-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000400002')
 archive=ROOT/'build/lm-spectest-handoff-reference.snapshots.xz'
 with lzma.open(archive,'rb')as f:decoded=hashlib.file_digest(f,'sha256').hexdigest()
 assert decoded=='71eac6f842fdd2e7769c8b61c8deae6909d3f6e1349365cdbcc285efeb57aae0'
 for suffix in ('reference.snapshots.xz','sanitize.log','guards-strict.log','guards-sanitize.log',
                'port-strict.log','port-sanitize.log','target-build.log','vice-result.log','vice.log',
                'vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin',
                'vice-hardware-stack.bin'):
  keep('lm-spectest-handoff-'+suffix)
 for ext in ('prg','map','d81'):keep('lm-spectest-handoff.'+ext)
 for n in range(32):
  for ext in ('c','prg','map'):keep('lm-spectest-handoff-%d.%s'%(n,ext))
 keep('lm_spectest_handoff_modules.h')
 rows=json.loads((ROOT/'build/lm-spectest-handoff-modules.json').read_text())
 for row in rows:
  row['retained_prg']='evidence/'+row['prg'];assert sha(ROOT/row['retained_prg'])==row['sha256']
 manifest=ROOT/'evidence/lm-spectest-handoff-modules.json';manifest.write_text(json.dumps(rows,indent=2)+'\n');owned.append(str(manifest.relative_to(ROOT)))
 deps={'src/agc_word.c','src/executive.c','src/p66_native.c','src/lm_admission_raw.c','src/lm_wake_image.c',
       'tests/lm_admission_image_cases.c','tests/lm_admission_raw_cases.c','tests/throttle_bridge_cases.c','tests/lm_wake_cases.c','tests/lm_wake_image_cases.c','tools/reference_runner.c','tests/scenarios.c',
       'deps/fixture/EXECUTIVE.agc','../Luminary099/EXECUTIVE.agc','deps/fixture/MAIN.agc.bin',
       'deps/virtualagc/yaAGC/agc_engine.c','deps/virtualagc/yaAGC/agc_engine.o',
       'deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o',
       'platform/c64/overlay_platform.c','platform/c64/overlay_platform.h','src/overlay_runtime.c',
       'platform/c64/overlay_payload.ld','platform/c64/overlay_resident.ld','platform/c64/overlay_entry.S',
       'tools/native_resident_port_host.py','tools/overlay_build.py','tools/overlay_runtime_library.py'}
 src=[s for s in owned if s.endswith('.c')and not s.endswith('_reference.c')and not s.startswith('evidence/')]
 source=subprocess.check_output(['cc','-MM','-DP66_NATIVE_DISPATCH_ONLY','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+src+['src/lm_admission_raw.c','src/lm_wake_image.c','tests/lm_admission_image_cases.c','tests/lm_admission_raw_cases.c','tests/throttle_bridge_cases.c','tests/lm_wake_cases.c','tests/lm_wake_image_cases.c'],cwd=ROOT,text=True)
 for line in source.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
 resources={}
 for name in ('stack','hardware-stack'):
  data=(ROOT/('build/lm-spectest-handoff-vice-'+name+'.bin')).read_bytes();changed=[i for i,b in enumerate(data)if b!=0xcd];resources[name]=len(data)-min(changed)
 proof={'scope':'Actual exceptional admission/wake SPECTEST caller prefixes and direct+0/negative flag branches, stopping before original CCSHOLE',
 'source_entries':['05072','05105','05116','05112','05137','02651'],
 'terminal':'CCSHOLE05705; actualQ02654/+0 or02655/negative; recovery external',
 'source_flags':'preserved original exceptional flags; no normalization, lifecycle or IRQ poll',
 'original_states':16384,'full_source_words':2048,'explicit_io_fields':['SUPER','IRQ'],
 'decoded_sha256':decoded,'strict_full_asan_ubsan':'PASS','historical_excluded_domain_guards_unchanged':13654,
 'all_object_invalid_domains':21,'null_interfaces':2,
 'c64':{'stock_pal_rom_d81':True,'frozen_original_states':64,'fixture_images':32,'mailbox':'a50000400002',
        'resources':resources,'extra_zero_page':0,'source_normalization':False,'incoming_i_and_port_restored':True},
 'files':{p:sha(ROOT/p)for p in owned},'dependencies':{p:sha(ROOT/p)for p in sorted(deps)}}
 output=ROOT/'evidence/lm-spectest-handoff-proof.json';output.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(output.relative_to(ROOT)))
 allow=ROOT/'build/lm-spectest-handoff-commit-files.txt';allow.write_text('\n'.join(owned)+'\n')
 print('Frozen',len(owned),'files;allowlist SHA',sha(allow));print('Proof SHA',sha(output))
if __name__=='__main__':main()
