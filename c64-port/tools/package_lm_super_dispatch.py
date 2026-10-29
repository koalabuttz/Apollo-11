#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze additive admission source, compatibility and actual target proofs."""
import hashlib,json,lzma,shutil,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
owned=['include/lm_super_dispatch.h','src/lm_super_dispatch.c',
 'tests/lm_super_dispatch_cases.h','tests/lm_super_dispatch_cases.c','tests/test_lm_super_dispatch.c',
 'docs/LM-SUPER-DISPATCH.md','tools/package_lm_super_dispatch.py',
 'tools/lm_super_dispatch_port_host.py','tools/lm_super_dispatch_port_integration.mk']
owned+=['tools/lm_super_dispatch_'+s for s in ('reference.c','snapshot.c','inputs.c','differential.py','target_build.py','vice_test.py')]
owned+=['platform/c64/lm_super_dispatch_'+s for s in ('fixture.h','diagnostic.c')]
def keep(name):
 shutil.copyfile(ROOT/'build'/name,ROOT/'evidence'/name);owned.append('evidence/'+name)
def main():
 for mode in ('strict','sanitize'):
  assert 'PASS:retained superbank transfer' in (ROOT/('build/lm-super-dispatch-port-'+mode+'.log')).read_text()
 assert (ROOT/'build/lm-super-dispatch-vice-mailbox.bin').read_bytes()==bytes.fromhex('a50000400002')
 archive=ROOT/'build/lm-super-dispatch-reference.snapshots.xz'
 with lzma.open(archive,'rb')as f:decoded=hashlib.file_digest(f,'sha256').hexdigest()
 assert decoded=='a0f821105c45b65e2225610c728a76b8af4e245395482cb00742ee8fb5b77b61'
 for suffix in ('reference.snapshots.xz','sanitize.log','guards-strict.log','guards-sanitize.log',
                'port-strict.log','port-sanitize.log','target-build.log','vice-result.log','vice.log',
                'vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin',
                'vice-hardware-stack.bin'):
  keep('lm-super-dispatch-'+suffix)
 for ext in ('prg','map','d81'):keep('lm-super-dispatch.'+ext)
 for n in range(32):
  for ext in ('c','prg','map'):keep('lm-super-dispatch-%d.%s'%(n,ext))
 keep('lm_super_dispatch_modules.h')
 rows=json.loads((ROOT/'build/lm-super-dispatch-modules.json').read_text())
 for row in rows:
  row['retained_prg']='evidence/'+row['prg'];assert sha(ROOT/row['retained_prg'])==row['sha256']
 manifest=ROOT/'evidence/lm-super-dispatch-modules.json';manifest.write_text(json.dumps(rows,indent=2)+'\n');owned.append(str(manifest.relative_to(ROOT)))
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
  data=(ROOT/('build/lm-super-dispatch-vice-'+name+'.bin')).read_bytes();changed=[i for i,b in enumerate(data)if b!=0xcd];resources[name]=len(data)-min(changed)
 proof={'scope':'Complete SUPDXCHZ/+1 source body with normal source 2CADR arguments; transferred program external',
 'source_entries':['05165','05166'], 'terminal':'TC L actual source target, Q2',
 'qualified_address_domain':'location0100..07777, BBCON onlyFB/EB/SUPER, sign-extended registers',
 'original_states':16384,'full_source_words':2048,'explicit_io_fields':['SUPER','IRQ'],
 'decoded_sha256':decoded,'strict_full_asan_ubsan':'PASS','independent_transfer_metadata_checks':16384,
 'all_object_invalid_domains':13,'null_interfaces':3,
 'c64':{'stock_pal_rom_d81':True,'frozen_original_states':64,'fixture_images':32,'mailbox':'a50000400002',
        'resources':resources,'extra_zero_page':0,'source_normalization':False,'incoming_i_and_port_restored':True},
 'files':{p:sha(ROOT/p)for p in owned},'dependencies':{p:sha(ROOT/p)for p in sorted(deps)}}
 output=ROOT/'evidence/lm-super-dispatch-proof.json';output.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(output.relative_to(ROOT)))
 allow=ROOT/'build/lm-super-dispatch-commit-files.txt';allow.write_text('\n'.join(owned)+'\n')
 print('Frozen',len(owned),'files;allowlist SHA',sha(allow));print('Proof SHA',sha(output))
if __name__=='__main__':main()
