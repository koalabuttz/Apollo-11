#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 source=R/'build/sband-exit-native.snapshots.bin.xz';retained=R/'evidence/sband-exit-source.snapshots.bin.xz'
 if source.exists():
  if retained.exists():assert digest(source)==digest(retained);source.unlink()
  else:source.rename(retained)
 for suffix in ['source-map.json','native-results.json','sanitize-results.json','sanitize-validation.log','target-source.snapshots.bin.xz','target-results.json','vice-results.json','host-validation.log','target-validation.log','frozen-validation.log','vice.log','monitor.log','mailbox.bin']:
  shutil.copyfile(R/f'build/sband-exit-{suffix}',R/f'evidence/sband-exit-{suffix}')
 for suffix in ['prg','map']:shutil.copyfile(R/f'build/sband-exit.{suffix}',R/f'evidence/sband-exit.{suffix}')
 strict=json.loads((R/'evidence/sband-exit-native-results.json').read_text());san=json.loads((R/'evidence/sband-exit-sanitize-results.json').read_text());assert strict==san
 h=hashlib.sha256();size=0
 with lzma.open(retained,'rb')as f:
  while b:=f.read(1024*1024):h.update(b);size+=len(b)
 assert h.hexdigest()==strict['decoded_sha256']and size==strict['states']*strict['record_bytes']
 source_map=json.loads((R/'evidence/sband-exit-source-map.json').read_text());review=R/'evidence/sband-exit-independent-review.txt';assert review.exists()
 proof=dict(status='qualified both original antenna EXIT to first basic display instruction',instruction_decoder=False,authoritative_erasable_images=1,source_states=strict['states'],words_each=2048,external_fields=['SUPER','IRQ'],corpus=dict(strict,decoded_bytes=size,canonical_original_archive=str(retained.relative_to(R)),compressed_sha256=digest(retained),sanitizer_exact=True),target=json.loads((R/'evidence/sband-exit-vice-results.json').read_text()),source_map='evidence/sband-exit-source-map.json',original_sources={v['source_path']:v['source_sha256']for v in source_map.values()},native_source_sha256=digest(R/'src/sband_exit.c'),native_header_sha256=digest(R/'include/sband_exit.h'),independent_review=dict(path=str(review.relative_to(R)),sha256=digest(review)),limitations=['No display/Executive/callee body executes.','Actual original compiled antenna context, ordinaryRAM/Q/rawA/L and producer exclusion.','Varied contexts are not exhaustive Cartesian.','Diagnostic memory does not establish full application allocation.'])
 (R/'evidence/sband-exit-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 paths=[]
 for directory,pattern in [('include','sband_exit*.h'),('src','sband_exit*.c'),('tests','*sband_exit*'),('tools','*sband_exit*'),('evidence','sband-exit-*')]:paths+=list((R/directory).glob(pattern))
 paths +=[R/'platform/c64/sband_exit_diagnostic.c',R/'docs/NATIVE-SBAND-EXIT.md',R/'evidence/sband-exit.prg',R/'evidence/sband-exit.map']
 paths=sorted(p for p in set(paths)if p.name!='sband-exit-SHA256.json');manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths};ledger=R/'evidence/sband-exit-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n');(R/'build/sband-exit-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n');print('READY',len(manifest)+1,'ledger',digest(ledger),'proof',digest(R/'evidence/sband-exit-proof.json'))
if __name__=='__main__':main()
