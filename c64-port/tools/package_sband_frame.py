#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact antenna time/CDU caller proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','target-source.snapshots.bin.xz','target-results.json','vice-results.json','host-validation.log','target-validation.log','frozen-validation.log']:
  copies[f'build/sband-frame-{suffix}']=f'evidence/sband-frame-{suffix}'
 for group in range(6):
  for suffix in ['prg','map']:copies[f'build/sband-frame-{group}.{suffix}']=f'evidence/sband-frame-{group}.{suffix}'
  for suffix in ['vice.log','monitor.log','mailbox.bin','vice-results.json']:copies[f'build/sband-frame-{group}-{suffix}']=f'evidence/sband-frame-{group}-{suffix}'
 copies['build/sband-frame-native.snapshots.bin.xz']='evidence/sband-frame-source.snapshots.bin.xz'
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-frame-native-results.json').read_text());san=json.loads((R/'evidence/sband-frame-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;p=R/'evidence/sband-frame-source.snapshots.bin.xz'
 with lzma.open(p,'rb')as s:
  while b:=s.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-frame-source-map.json').read_text());review=R/'evidence/sband-frame-independent-review.txt'
 proof=dict(status='qualified explicit antenna UNIT/CDUTRIG/MXV/VSL1 continuations; both complete files remain partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),sanitizer_exact=True),target=json.loads((R/'evidence/sband-frame-vice-results.json').read_text()),groups=['LM UNIT','LM AFTER_UNIT/CDUTRIG','LM MATRIX','CM MATRIX','LM SHIFT','CM SHIFT'],source_map='evidence/sband-frame-source-map.json',original_sources={x['source_path']:x['source_sha256']for x in source_map.values()},native_source_sha256=digest(R/'src/sband_frame.c'),native_header_sha256=digest(R/'include/sband_frame.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['Moon-return vector math, remaining body/pitch/yaw calculations, and display lifecycle pending.','NEWJOB+0, five valid VAC bases, ordinary RAM/Q and raw16 A/L, producer exclusion throughout each call.','Actual original helper/callee entry states retained; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 dependencies=['evidence/rtb-unit-proof.json','evidence/lm-powered-interface-proof.json','evidence/lm-powered-trig-proof.json','evidence/matrix-proof.json','evidence/sband-orbit-proof.json','evidence/sband-shift-proof.json']
 proof['dependencies']=[dict(path=p,sha256=digest(R/p))for p in dependencies]
 (R/'evidence/sband-frame-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_frame*.h'),('src','sband_frame*.c'),('tests','*sband_frame*'),('tools','*sband_frame*'),('evidence','sband-frame-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'platform/c64/sband_frame_diagnostic.c',R/'docs/NATIVE-SBAND-FRAME.md',R/'evidence/sband-frame-0.prg',R/'evidence/sband-frame-0.map']
 paths=sorted(p for p in set(paths)if p.name!='sband-frame-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-frame-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-frame-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-frame-proof.json'))
if __name__=='__main__':main()
