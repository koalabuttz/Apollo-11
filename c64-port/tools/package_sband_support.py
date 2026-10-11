#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain partial antenna support proof; never edit whole-file inventory."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={};corpora={};targets={}
 for module in ('control','trig'):
  for suffix in ('target-source.snapshots.bin.xz','target-results.json','vice-results.json','prg','map','vice.log','monitor.log','mailbox.bin'):
   copies[f'build/sband-{module}-{suffix}' if suffix not in ('prg','map') else f'build/sband-{module}.{suffix}']=f'evidence/sband-{module}-{suffix}' if suffix not in ('prg','map') else f'evidence/sband-{module}.{suffix}'
 copies['build/sband-control-source-audit.json']='evidence/sband-control-source-audit.json'
 copies['build/sband-trig-source-map.json']='evidence/sband-trig-source-map.json'
 for name in ('control','trig-LM','trig-CM'):
  for suffix in ('native','sanitize'):
   copies[f'build/sband-{name}-{suffix}-results.json']=f'evidence/sband-{name}-{suffix}-results.json'
  copies[f'build/sband-{name}-native.snapshots.bin.xz']=f'evidence/sband-{name}-source.snapshots.bin.xz'
 for mission in ('LM','CM'):copies[f'build/sband-trig-{mission}-reference.trace']=f'evidence/sband-trig-{mission}-reference.trace'
 for src,dest in copies.items():shutil.copyfile(R/src,R/dest)
 for name in ('control','trig-LM','trig-CM'):
  strict=json.loads((R/f'evidence/sband-{name}-native-results.json').read_text());san=json.loads((R/f'evidence/sband-{name}-sanitize-results.json').read_text());assert strict==san
  p=R/f'evidence/sband-{name}-source.snapshots.bin.xz';h=hashlib.sha256();size=0
  with lzma.open(p,'rb') as s:
   while piece:=s.read(1024*1024):h.update(piece);size+=len(piece)
  assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
  corpora[name]=dict(strict,sanitizer_exact=True,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),decoded_bytes=size)
 for name in ('control','trig'):targets[name]=json.loads((R/f'evidence/sband-{name}-vice-results.json').read_text())
 sourceids={}
 for mission in ('Luminary099','Comanche055'):
  p=R.parent/mission/f'S-BAND_ANTENNA_FOR_{"LM" if mission=="Luminary099" else "CM"}.agc';sourceids[str(p.relative_to(R.parent))]=digest(p)
 for name in ('sband_control','sband_trig'):
  for directory,suffix in (('src','c'),('include','h')):
   p=R/directory/f'{name}.{suffix}';sourceids[str(p.relative_to(R))]=digest(p)
 proof=dict(status='qualified native support; both complete antenna files remain partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=sum(c['states']for c in corpora.values()),words_each=2048,external_fields=['SUPER','IRQ'],corpora=corpora,target=targets,source_identities=sourceids,source_maps=['evidence/sband-control-source-audit.json','evidence/sband-trig-source-map.json'],independent_review='evidence/sband-support-independent-review.txt',dependency_vector_packet='evidence/sband-vector-proof.json',limitations=['External orbital/display/Executive callee bodies are genuine source handoffs, not completed implementations.','Canonical SINE/COSINE bridge is DP MODE0 only; producer exclusion throughout each call.','No whole INTERPRETER or whole S-band claim; interpreted antenna sequence awaits composition.','Input marginals and varied contexts do not establish exhaustive Cartesian coverage.','Standalone diagnostic memory allocation is not integrated application memory.'])
 (R/'evidence/sband-support-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for module in ('control','trig'):
  key=f'sband_{module}'
  for directory,pattern in [('include',key+'*.h'),('src',key+'*.c'),('tests','*'+key+'*'),('tools','*'+key+'*'),('evidence',f'sband-{module}-*')]:paths+=list((R/directory).glob(pattern))
  paths+=[R/f'platform/c64/{key}_diagnostic.c',R/f'evidence/sband-{module}.prg',R/f'evidence/sband-{module}.map']
 paths+=[R/'tools/package_sband_support.py',R/'docs/NATIVE-SBAND-CALLERS.md']+list((R/'evidence').glob('sband-support-*'))
 paths=sorted(p for p in set(paths) if p.name!='sband-support-SHA256.json')
 manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths};ledger=R/'evidence/sband-support-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n')
 (R/'build/sband-support-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n');print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-support-proof.json'),flush=True)
if __name__=='__main__':main()
