#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain the exact vector dependency packet without touching shared inventory."""
import hashlib,json,lzma,pathlib,shutil
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 copies={
 'build/sband-source/audit.json':'evidence/sband-original-source-audit.json',
 'build/sband-vector-source-map.json':'evidence/sband-vector-source-map.json',
 'build/sband-vector-target-source.snapshots.bin.xz':'evidence/sband-vector-target-source.snapshots.bin.xz',
 'build/sband-vector-target-results.json':'evidence/sband-vector-target-results.json',
 'build/sband-vector-vice-results.json':'evidence/sband-vector-vice-results.json',
 'build/sband-vector.prg':'evidence/sband-vector.prg',
 'build/sband-vector.map':'evidence/sband-vector.map',
 'build/sband-vector-vice.log':'evidence/sband-vector-vice.log',
 'build/sband-vector-monitor.log':'evidence/sband-vector-monitor.log',
 'build/sband-vector-mailbox.bin':'evidence/sband-vector-mailbox.bin'}
 for mission in ('LM','CM'):
  for suffix in ('native','sanitize'):
   copies[f'build/sband-vector-{mission}-{suffix}-results.json']=f'evidence/sband-vector-{mission}-{suffix}-results.json'
  copies[f'build/sband-vector-{mission}-native.snapshots.bin.xz']=f'evidence/sband-vector-{mission}-source.snapshots.bin.xz'
  copies[f'build/sband-vector-{mission}-reference.trace']=f'evidence/sband-vector-{mission}-reference.trace'
 for a,b in copies.items():shutil.copyfile(R/a,R/b)
 proof=dict(status='qualified_vector_dependency; antenna and interpreter files remain partial',native_api='include/sband_vector.h',native_source='src/sband_vector.c',instruction_decoder=False,authoritative_erasable_images=1,source_map='evidence/sband-vector-source-map.json',sband_file_audit='evidence/sband-original-source-audit.json',independent_review='evidence/sband-vector-independent-review.txt',source_states=262144,complete_words_each=2048,external_fields=['SUPER','IRQ'],target=json.loads((R/'evidence/sband-vector-vice-results.json').read_text()),groups_each_mission=['erasable VXV32768','erasable VPROJ32768','fixed VXV32768','fixed VPROJ32768'],domain='Ordinary15-bit RAM/Q, raw16 A/L, coherent banks, vectorMODE -1, ordinary six-word data argument and producer exclusion. Immutable fixed array cannot alias canonical RAM.',limitations=['Neither whole S-band nor whole INTERPRETER file completion. No external orbital/display callee execution.','Input state tuples and branch combinations are sampled, not exhaustively Cartesian. Source OVERFLOW no-final-carry TC Q is compiled/reviewed but not executed in this cohort.','Fixed groups first256 retain original HIUNIT axes; subsequent six ordinary input data words are injected into the passive argument region of the host oracle; original instruction/scalar constants remain unchanged.','Standalone C64 diagnostic allocation is not whole-application resident memory.'],corpora={})
 for mission in ('LM','CM'):
  strict=json.loads((R/f'evidence/sband-vector-{mission}-native-results.json').read_text());san=json.loads((R/f'evidence/sband-vector-{mission}-sanitize-results.json').read_text());assert strict==san
  p=R/f'evidence/sband-vector-{mission}-source.snapshots.bin.xz';h=hashlib.sha256();size=0
  with lzma.open(p,'rb') as stream:
   while piece:=stream.read(1024*1024):h.update(piece);size+=len(piece)
  assert h.hexdigest()==strict['decoded_sha256'] and size==strict['states']*4106
  proof['corpora'][mission]=dict(strict,sanitizer_exact=True,canonical_original_archive=str(p.relative_to(R)),compressed_sha256=digest(p),decoded_bytes=size)
 (R/'evidence/sband-vector-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 summary='Actual completed validation results; this summary is not a raw process transcript.\n'+json.dumps(dict(source=proof['corpora'],target=proof['target']),indent=2)+'\nFrozen strict host and ASAN/UBSAN complete-image comparison plus unsupported-image preservation tests passed.\n'
 (R/'evidence/sband-vector-validation.txt').write_text(summary)
 paths=[p for p in R.glob('include/sband_vector*.h')]+[p for p in R.glob('src/sband_vector*.c')]+[p for p in R.glob('tests/*sband_vector*')]+[p for p in R.glob('tools/*sband_vector*')]+[R/'tools/sband_source.py',R/'platform/c64/sband_vector_diagnostic.c',R/'docs/NATIVE-SBAND-VECTORS.md']+[p for p in R.glob('evidence/sband-vector-*')]+[R/'evidence/sband-original-source-audit.json']
 paths=sorted(p for p in set(paths) if p.name!="sband-vector-SHA256.json");manifest={str(p.relative_to(R)):dict(sha256=digest(p),bytes=p.stat().st_size)for p in paths}
 ledger=R/'evidence/sband-vector-SHA256.json';ledger.write_text(json.dumps(manifest,indent=2)+'\n')
 (R/'build/sband-vector-commit-files.txt').write_text('\n'.join([*manifest,str(ledger.relative_to(R))])+'\n');print('READY',len(manifest)+1,'files; ledger',digest(ledger),'proof',digest(R/'evidence/sband-vector-proof.json'),flush=True)
if __name__=='__main__':main()
