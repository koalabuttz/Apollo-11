#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact antenna plane projection/UNIT/BOV caller proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','sanitize-validation.log','target-source.snapshots.bin.xz','target-results.json','vice-results.json','host-validation.log','target-validation.log','frozen-validation.log']:
  copies[f'build/sband-lm-pitch-{suffix}']=f'evidence/sband-lm-pitch-{suffix}'
 for group in range(4):
  for suffix in ['prg','map','vice-results.json','vice.log','monitor.log','mailbox.bin']:copies[f'build/sband-lm-pitch-{group}.'+suffix if suffix in ['prg','map'] else f'build/sband-lm-pitch-{group}-'+suffix]=f'evidence/sband-lm-pitch-{group}.'+suffix if suffix in ['prg','map'] else f'evidence/sband-lm-pitch-{group}-'+suffix
 copies['build/sband-lm-pitch-initial-memory-trial.log']='evidence/sband-lm-pitch-initial-memory-trial.log'
 copies['build/sband-lm-pitch-initial-memory-trial.map']='evidence/sband-lm-pitch-initial-memory-trial.map'
 source=R/'build/sband-lm-pitch-native.snapshots.bin.xz';retained=R/'evidence/sband-lm-pitch-source.snapshots.bin.xz'
 if source.exists():
  if retained.exists():assert digest(source)==digest(retained);source.unlink()
  else:source.rename(retained)
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-lm-pitch-native-results.json').read_text());san=json.loads((R/'evidence/sband-lm-pitch-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;p=R/'evidence/sband-lm-pitch-source.snapshots.bin.xz'
 with lzma.open(p,'rb')as s:
  while b:=s.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-lm-pitch-source-map.json').read_text());review=R/'evidence/sband-lm-pitch-independent-review.txt'
 proof=dict(status='qualified complete original LM antenna pitch from normal-plane entry; containing antenna file remains partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),sanitizer_exact=True),target=json.loads((R/'evidence/sband-lm-pitch-vice-results.json').read_text()),groups=['LM original complete pitch computation and direction adjustment'],source_map='evidence/sband-lm-pitch-source-map.json',original_sources={source_map['source_path']:source_map['source_sha256']},native_source_sha256=digest(R/'src/sband_lm_pitch.c'),native_header_sha256=digest(R/'include/sband_lm_pitch.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['Following original EXIT/display bodies not executed here.','NEWJOB+0, five valid VAC bases, ordinary RAM/Q and raw16 A/L, producer exclusion throughout each call.','Actual original helper/callee entry states retained; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 proof['dependencies']=[dict(path=p,sha256=digest(R/p))for p in ['evidence/sband-dot-proof.json','evidence/sband-scalar-proof.json','evidence/inverse-trig-lm-proof.json','evidence/sband-length-proof.json','evidence/sband-vector-proof.json','evidence/sband-shift-proof.json','evidence/interpreted-sign-proof.json','evidence/inverse-trig-sweep-proof.json']]
 (R/'evidence/sband-lm-pitch-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_lm_pitch*.h'),('src','sband_lm_pitch*.c'),('tests','*sband_lm_pitch*'),('tools','*sband_lm_pitch*'),('evidence','sband-lm-pitch-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'platform/c64/sband_lm_pitch_diagnostic.c',R/'docs/NATIVE-SBAND-LM-PITCH.md',*[R/f'evidence/sband-lm-pitch-{g}.{s}'for g in range(4)for s in ['prg','map']]]
 paths=sorted(p for p in set(paths)if p.name!='sband-lm-pitch-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-lm-pitch-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-lm-pitch-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-lm-pitch-proof.json'))
if __name__=='__main__':main()
