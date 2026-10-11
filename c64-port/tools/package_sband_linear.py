#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact antenna time/CDU caller proof without whole-file certification."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={}
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','target-source.snapshots.bin.xz','target-results.json','vice-results.json','vice.log','monitor.log','mailbox.bin','host-validation.log','target-validation.log','frozen-validation.log']:
  copies[f'build/sband-linear-{suffix}']=f'evidence/sband-linear-{suffix}'
 for suffix in ['prg','map']:copies[f'build/sband-linear.{suffix}']=f'evidence/sband-linear.{suffix}'
 copies['build/sband-linear-native.snapshots.bin.xz']='evidence/sband-linear-source.snapshots.bin.xz'
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 strict=json.loads((R/'evidence/sband-linear-native-results.json').read_text());san=json.loads((R/'evidence/sband-linear-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0;p=R/'evidence/sband-linear-source.snapshots.bin.xz'
 with lzma.open(p,'rb')as s:
  while b:=s.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-linear-source-map.json').read_text());review=R/'evidence/sband-linear-independent-review.txt'
 proof=dict(status='qualified canonical VXSC/VAD/VSU; both containing antenna and INTERPRETER files remain partial',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),sanitizer_exact=True),target=json.loads((R/'evidence/sband-linear-vice-results.json').read_text()),groups=['Both missions, vector/scalar multiply, add/subtract, live ERA/fixed input data; sixteen groups'],source_map='evidence/sband-linear-source-map.json',original_sources={x['source_path']:x['original_sha256']for x in source_map.values()},native_source_sha256=digest(R/'src/sband_vector_linear.c'),native_header_sha256=digest(R/'include/sband_vector_linear.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['Only explicit original VXSC/VAD/VSU bodies plus their numerical helpers; no containing-file completion.','MODE0/-1 multiply or MODE-1 add/subtract, ordinary RAM/Q and raw16 A/L, coherent banks/ZERO; producer exclusion throughout each call.','Actual original helper/callee entry states retained; no arbitrary instruction dispatch.','Context coverage is varied, not exhaustive Cartesian.','Standalone diagnostic allocation is not whole-application memory.'])
 (R/'evidence/sband-linear-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_linear*.h'),('src','sband_linear*.c'),('tests','*sband_linear*'),('tools','*sband_linear*'),('evidence','sband-linear-*')]:paths+=list((R/directory).glob(pattern))
 paths+=[R/'src/sband_vector_linear.c',R/'include/sband_vector_linear.h',R/'platform/c64/sband_linear_diagnostic.c',R/'docs/NATIVE-SBAND-LINEAR-VECTORS.md',R/'evidence/sband-linear.prg',R/'evidence/sband-linear.map']
 paths=sorted(p for p in set(paths)if p.name!='sband-linear-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-linear-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-linear-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n')
 print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-linear-proof.json'))
if __name__=='__main__':main()
