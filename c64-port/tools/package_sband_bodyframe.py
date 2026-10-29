#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact antenna zero-angle/SMNB caller proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','target-source.snapshots.bin.xz','target-results.json','vice-results.json','vice.log','monitor.log','mailbox.bin','host-validation.log','target-validation.log','frozen-validation.log']:
  copies[f'build/sband-bodyframe-{suffix}']=f'evidence/sband-bodyframe-{suffix}'
 for suffix in ['prg','map']:copies[f'build/sband-bodyframe.{suffix}']=f'evidence/sband-bodyframe.{suffix}'
 copies['build/sband-bodyframe-native.snapshots.bin.xz']='evidence/sband-bodyframe-source.snapshots.bin.xz'
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-bodyframe-native-results.json').read_text());san=json.loads((R/'evidence/sband-bodyframe-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;p=R/'evidence/sband-bodyframe-source.snapshots.bin.xz'
 with lzma.open(p,'rb')as s:
  while b:=s.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-bodyframe-source-map.json').read_text());review=R/'evidence/sband-bodyframe-independent-review.txt'
 proof=dict(status='qualified actual antenna zero-angle setup and complete SMNB caller; both containing antenna files remain partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),sanitizer_exact=True),target=json.loads((R/'evidence/sband-bodyframe-vice-results.json').read_text()),groups=['LM original PUSH/zero angles/pop and completeSMNB','CM original PDDL/zero yaw/RCM and completeSMNB'],source_map='evidence/sband-bodyframe-source-map.json',original_sources={x['source_path']:x['source_sha256']for x in source_map.values()},native_source_sha256=digest(R/'src/sband_bodyframe.c'),native_header_sha256=digest(R/'include/sband_bodyframe.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['Following original store/projection/angle/display bodies not executed here.','NEWJOB+0, five valid VAC bases, ordinary RAM/Q and raw16 A/L, producer exclusion throughout each call.','Actual original helper/callee entry states retained; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 proof['dependencies']=[dict(path=p,sha256=digest(R/p))for p in ['evidence/lm-powered-file-proof.json','evidence/cm-powered-file-proof.json','evidence/sband-frame-proof.json']]
 (R/'evidence/sband-bodyframe-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_bodyframe*.h'),('src','sband_bodyframe*.c'),('tests','*sband_bodyframe*'),('tools','*sband_bodyframe*'),('evidence','sband-bodyframe-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'platform/c64/sband_bodyframe_diagnostic.c',R/'docs/NATIVE-SBAND-BODY-FRAME.md',R/'evidence/sband-bodyframe.prg',R/'evidence/sband-bodyframe.map']
 paths=sorted(p for p in set(paths)if p.name!='sband-bodyframe-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-bodyframe-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-bodyframe-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-bodyframe-proof.json'))
if __name__=='__main__':main()
