#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact antenna plane projection/UNIT/BOV caller proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','sanitize-validation.log','target-source.snapshots.bin.xz','target-results.json','vice-results.json','host-validation.log','target-validation.log','frozen-validation.log']:
  copies[f'build/sband-cm-angles-{suffix}']=f'evidence/sband-cm-angles-{suffix}'
 for group in range(4):
  for suffix in ['prg','map','vice-results.json','vice.log','monitor.log','mailbox.bin']:copies[f'build/sband-cm-angles-{group}.'+suffix if suffix in ['prg','map'] else f'build/sband-cm-angles-{group}-'+suffix]=f'evidence/sband-cm-angles-{group}.'+suffix if suffix in ['prg','map'] else f'evidence/sband-cm-angles-{group}-'+suffix
 source=R/'build/sband-cm-angles-native.snapshots.bin.xz';retained=R/'evidence/sband-cm-angles-source.snapshots.bin.xz'
 if source.exists():
  if retained.exists():assert digest(source)==digest(retained);source.unlink()
  else:source.rename(retained)
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-cm-angles-native-results.json').read_text());san=json.loads((R/'evidence/sband-cm-angles-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;p=R/'evidence/sband-cm-angles-source.snapshots.bin.xz'
 with lzma.open(p,'rb')as s:
  while b:=s.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-cm-angles-source-map.json').read_text());review=R/'evidence/sband-cm-angles-independent-review.txt'
 proof=dict(status='qualified complete CM antenna yaw/pitch including actual null entry; containing antenna file remains partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),sanitizer_exact=True),target=json.loads((R/'evidence/sband-cm-angles-vice-results.json').read_text()),groups=['CM original full yaw/pitch computation','CM original null-plane NOADJUST pitch computation'],source_map='evidence/sband-cm-angles-source-map.json',original_sources={source_map['source_path']:source_map['source_sha256']},native_source_sha256=digest(R/'src/sband_cm_angles.c'),native_header_sha256=digest(R/'include/sband_cm_angles.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['Following original EXIT/display bodies not executed here.','NEWJOB+0, five valid VAC bases, ordinary RAM/Q and raw16 A/L, producer exclusion throughout each call.','Actual original helper/callee entry states retained; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 proof['dependencies']=[dict(path=p,sha256=digest(R/p))for p in ['evidence/sband-dot-proof.json','evidence/sband-scalar-proof.json','evidence/inverse-trig-cm-proof.json','evidence/inverse-trig-sweep-proof.json']]
 (R/'evidence/sband-cm-angles-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_cm_angles*.h'),('src','sband_cm_angles*.c'),('tests','*sband_cm_angles*'),('tools','*sband_cm_angles*'),('evidence','sband-cm-angles-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'platform/c64/sband_cm_angles_diagnostic.c',R/'docs/NATIVE-SBAND-CM-ANGLES.md',*[R/f'evidence/sband-cm-angles-{g}.{s}'for g in range(4)for s in ['prg','map']]]
 paths=sorted(p for p in set(paths)if p.name!='sband-cm-angles-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-cm-angles-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-cm-angles-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-cm-angles-proof.json'))
if __name__=='__main__':main()
