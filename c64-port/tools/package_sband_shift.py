#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact canonical vector-left shift proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','target-source.snapshots.bin.xz','target-results.json','vice-results.json','vice.log','monitor.log','mailbox.bin','host-validation.log','target-validation.log','frozen-validation.log']:
  copies[f'build/sband-shift-{suffix}']=f'evidence/sband-shift-{suffix}'
 for suffix in ['prg','map']:copies[f'build/sband-shift.{suffix}']=f'evidence/sband-shift.{suffix}'

 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-shift-native-results.json').read_text());san=json.loads((R/'evidence/sband-shift-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;chunks=[]
 with lzma.open(R/'build/sband-shift-native.snapshots.bin.xz','rb')as source:
  for part in range(2):
   p=R/f'evidence/sband-shift-source-{part}.snapshots.bin.xz';ch=hashlib.sha256();remaining=262144*4106
   with lzma.open(p,'wb',preset=6)as target:
    while remaining:
     data=source.read(min(1024*1024,remaining));assert data
     target.write(data);h.update(data);ch.update(data);size+=len(data);remaining-=len(data)
   assert p.stat().st_size<64*1024*1024
   chunks.append(dict(path=str(p.relative_to(R)),compressed_sha256=digest(p),decoded_sha256=ch.hexdigest(),decoded_bytes=262144*4106,states=262144))
  assert not source.read(1)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-shift-source-map.json').read_text());review=R/'evidence/sband-shift-independent-review.txt'
 proof=dict(status='qualified canonical short vector-left continuation; both complete antenna and INTERPRETER files remain partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archives=chunks,sanitizer_exact=True),target=json.loads((R/'evidence/sband-shift-vice-results.json').read_text()),groups=['LM/CM counts1..8, 32768 rows per group'],source_map='evidence/sband-shift-source-map.json',original_sources={x['source_path']:x['original_sha256']for x in source_map.values()},native_source_sha256=digest(R/'src/sband_shift.c'),native_header_sha256=digest(R/'include/sband_shift.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['Only SHORTV left branch counts1..8, VSSL and overflow helpers; right shift/generic dispatch excluded.','Original bank0, vector MODE-1, CYR source count, ordinary RAM/Q and raw16 A/L; producer exclusion throughout each call.','Actual original helper/callee entry states retained; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 (R/'evidence/sband-shift-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_shift*.h'),('src','sband_shift*.c'),('tests','*sband_shift*'),('tools','*sband_shift*'),('evidence','sband-shift-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'platform/c64/sband_shift_diagnostic.c',R/'docs/NATIVE-SBAND-VECTOR-SHIFT.md',R/'evidence/sband-shift.prg',R/'evidence/sband-shift.map']
 paths=sorted(p for p in set(paths)if p.name!='sband-shift-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-shift-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-shift-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-shift-proof.json'))
if __name__=='__main__':main()
