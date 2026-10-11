#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact original scalar entry evidence, retaining bounded source chunks."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','target-source.snapshots.bin.xz','target-results.json','vice-results.json','vice.log','monitor.log','mailbox.bin','host-validation.log','sanitize-validation.log','target-validation.log','frozen-validation.log']:
  shutil.copyfile(R/f'build/sband-scalar-{suffix}',R/f'evidence/sband-scalar-{suffix}')
 for suffix in ['prg','map']:shutil.copyfile(R/f'build/sband-scalar.{suffix}',R/f'evidence/sband-scalar.{suffix}')
 strict=json.loads((R/'evidence/sband-scalar-native-results.json').read_text());san=json.loads((R/'evidence/sband-scalar-sanitize-results.json').read_text());assert strict==san and strict['states']==786432
 chunks=[];h=hashlib.sha256();total=0
 for g in range(12):
  p=R/f'build/sband-scalar-source-{g}.snapshots.bin.xz';e=R/f'evidence/sband-scalar-source-{g}.snapshots.bin.xz'
  if p.exists():
   if e.exists():assert digest(p)==digest(e);p.unlink()
   else:p.rename(e)
  ch=hashlib.sha256();size=0
  with lzma.open(e,'rb')as f:
   while b:=f.read(1048576):h.update(b);ch.update(b);size+=len(b)
  assert size==65536*4106 and e.stat().st_size<64*1024*1024;total+=size;chunks.append(dict(path=str(e.relative_to(R)),compressed_sha256=digest(e),decoded_sha256=ch.hexdigest(),decoded_bytes=size,first_case=g*65536,states=65536))
 assert total==strict['states']*strict['record_bytes']and h.hexdigest()==strict['decoded_sha256']
 review=R/'evidence/sband-scalar-independent-review.txt';source=json.loads((R/'evidence/sband-scalar-source-map.json').read_text())
 proof=dict(status='qualified actual original DMP1/DAD/DSU scalar entries; containing INTERPRETER and antenna files remain partial',instruction_decoder=False,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=total,canonical_original_archives=chunks,sanitizer_exact=True),target=json.loads((R/'evidence/sband-scalar-vice-results.json').read_text()),native_source_sha256=digest(R/'src/sband_scalar.c'),native_header_sha256=digest(R/'include/sband_scalar.h'),source_map='evidence/sband-scalar-source-map.json',original_sources={x['source_path']:x['original_sha256']for x in source.values()},independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['MODE0/coherent ordinary image/Q/raw16A/L, producer exclusion.', 'Live ERA pair excludes CPU/editing registers; fixed operand requires two immutable nonaliasing ordinary words, no bank crossing.', 'Varied contexts and complete raw A/L/Q marginals, not exhaustive Cartesian or every source instruction edge.', 'Stops pending original mission DANZIG without scheduling or source decoding.', 'Diagnostic images/fixture allocation does not establish whole application memory.'])
 (R/'evidence/sband-scalar-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for d,p in [('include','sband_scalar*.h'),('src','sband_scalar*.c'),('tests','*sband_scalar*'),('tools','*sband_scalar*'),('evidence','sband-scalar-*')]:paths+=list((R/d).glob(p))
 paths+=[R/'platform/c64/sband_scalar_diagnostic.c',R/'docs/NATIVE-SBAND-SCALAR.md',R/'evidence/sband-scalar.prg',R/'evidence/sband-scalar.map'];paths=sorted(x for x in set(paths)if x.name!='sband-scalar-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-scalar-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-scalar-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n');print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-scalar-proof.json'))
if __name__=='__main__':main()
