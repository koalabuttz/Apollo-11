#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact antenna body-axis/UNIT caller proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','sanitize-validation.log','target-source.snapshots.bin.xz','target-results.json','vice-results.json','host-validation.log','target-validation.log','frozen-validation.log']:
  copies[f'build/sband-navframe-{suffix}']=f'evidence/sband-navframe-{suffix}'
 for group in range(2):
  for suffix in ['prg','map','vice-results.json','vice.log','monitor.log','mailbox.bin']:copies[f'build/sband-navframe-{group}.'+suffix if suffix in ['prg','map'] else f'build/sband-navframe-{group}-'+suffix]=f'evidence/sband-navframe-{group}.'+suffix if suffix in ['prg','map'] else f'evidence/sband-navframe-{group}-'+suffix
 copies['build/sband-navframe-initial-memory-trial.log']='evidence/sband-navframe-initial-memory-trial.log'
 copies['build/sband-navframe.map']='evidence/sband-navframe-initial-memory-trial.map'
 source=R/'build/sband-navframe-native.snapshots.bin.xz';retained=R/'evidence/sband-navframe-source.snapshots.bin.xz'
 if source.exists():
  if retained.exists():assert digest(source)==digest(retained);source.unlink()
  else:source.rename(retained)
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-navframe-native-results.json').read_text());san=json.loads((R/'evidence/sband-navframe-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;p=R/'evidence/sband-navframe-source.snapshots.bin.xz'
 with lzma.open(p,'rb')as s:
  while b:=s.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-navframe-source-map.json').read_text());review=R/'evidence/sband-navframe-independent-review.txt'
 proof=dict(status='qualified actual antenna vector store, LM body-axis mixing and complete UNIT caller; both containing antenna files remain partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),sanitizer_exact=True),target=json.loads((R/'evidence/sband-navframe-vice-results.json').read_text()),groups=['LM original vectorstore/full body-axis mixing and completeUNIT','CM original vectorstore and completeUNIT'],source_map='evidence/sband-navframe-source-map.json',original_sources={x['source_path']:x['source_sha256']for x in source_map.values()},native_source_sha256=digest(R/'src/sband_navframe.c'),native_header_sha256=digest(R/'include/sband_navframe.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['Following original PDVL/projection/angle/display bodies not executed here.','NEWJOB+0, five valid VAC bases, ordinary RAM/Q and raw16 A/L, producer exclusion throughout each call.','Actual original helper/callee entry states retained; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 proof['dependencies']=[dict(path=p,sha256=digest(R/p))for p in ['evidence/rtb-unit-proof.json','evidence/sband-scalar-proof.json','evidence/sband-bodyframe-proof.json']]
 (R/'evidence/sband-navframe-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_navframe*.h'),('src','sband_navframe*.c'),('tests','*sband_navframe*'),('tools','*sband_navframe*'),('evidence','sband-navframe-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'platform/c64/sband_navframe_diagnostic.c',R/'docs/NATIVE-SBAND-NAV-FRAME.md',*[R/f'evidence/sband-navframe-{g}.{s}'for g in range(2)for s in ['prg','map']]]
 paths=sorted(p for p in set(paths)if p.name!='sband-navframe-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-navframe-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-navframe-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-navframe-proof.json'))
if __name__=='__main__':main()
