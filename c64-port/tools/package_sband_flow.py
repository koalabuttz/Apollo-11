#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact antenna time/CDU caller proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','target-source.snapshots.bin.xz','target-results.json','vice-results.json','vice.log','monitor.log','mailbox.bin','host-validation.log','target-validation.log']:
  copies[f'build/sband-flow-{suffix}']=f'evidence/sband-flow-{suffix}'
 for suffix in ['prg','map']:copies[f'build/sband-flow.{suffix}']=f'evidence/sband-flow.{suffix}'
 copies['build/sband-flow-native.snapshots.bin.xz']='evidence/sband-flow-source.snapshots.bin.xz'
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-flow-native-results.json').read_text());san=json.loads((R/'evidence/sband-flow-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;p=R/'evidence/sband-flow-source.snapshots.bin.xz'
 with lzma.open(p,'rb')as s:
  while b:=s.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-flow-source-map.json').read_text());review=R/'evidence/sband-flow-independent-review.txt'
 proof=dict(status='qualified antenna leading time/CDU composition; both complete files remain partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),sanitizer_exact=True),target=json.loads((R/'evidence/sband-flow-vice-results.json').read_text()),groups=['LM TIME_FRONT','LM AFTER_TIME','LM TIME_PIPELINE','CM TIME_FRONT','CM AFTER_TIME','CM TIME_PIPELINE','CM AFTER_CDU','CM CM_TIME_TRIG'],source_map='evidence/sband-flow-source-map.json',original_sources={x['source_path']:x['source_sha256']for x in source_map.values()},native_source_sha256=digest(R/'src/sband_flow.c'),native_header_sha256=digest(R/'include/sband_flow.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['External conic bodies not executed; full antenna coordinate/angle sequence remains pending.','NEWJOB+0, five valid VAC bases, ordinary RAM/raw16 A/L/Q and producer exclusion throughout each call.','Delegated stable native helper bodies are composed at actual source controls; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 (R/'evidence/sband-flow-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_flow*.h'),('src','sband_flow*.c'),('tests','*sband_flow*'),('tools','*sband_flow*'),('evidence','sband-flow-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'platform/c64/sband_flow_diagnostic.c',R/'docs/NATIVE-SBAND-TIME-FLOW.md',R/'evidence/sband-flow.prg',R/'evidence/sband-flow.map']
 paths=sorted(p for p in set(paths)if p.name!='sband-flow-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-flow-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-flow-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-flow-proof.json'))
if __name__=='__main__':main()
