#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import struct,hashlib,json,re,gzip,shutil
r=Path(__file__).resolve().parents[1];e=r/'evidence';b=r/'build'
files=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def add(p):files.append(str(p.relative_to(r)))
def copy(source,name):p=e/name;shutil.copyfile(source,p);add(p);return p
for stem in ['throttle-feedback-pairs','feedback-length','horizontal-phased']:
 for kind in ['reference','native','sanitize']:
  src=b/f'{stem}-{kind}.snapshots';dest=e/f'{stem}-{kind}.snapshots.gz'
  with dest.open('wb')as f:
   with gzip.GzipFile(filename='',mode='wb',fileobj=f,mtime=0)as z:z.write(src.read_bytes())
  add(dest)
 for kind in ['reference.trace','validation.log']:
  src=b/f'{stem}-{kind}'
  if not src.exists() and stem=='throttle-feedback-pairs':src=b/f'feedback-pairs-{kind}'
  copy(src,f'{stem}-{kind}')
proof={}
for stem,word_count,case_count,images in [('throttle-feedback-pairs',137,16384,20),('feedback-length',137,16384,4),('horizontal-phased',132,8192,13)]:
 raw=(b/f'{stem}-reference.snapshots').read_bytes();assert raw==(b/f'{stem}-native.snapshots').read_bytes()==(b/f'{stem}-sanitize.snapshots').read_bytes();rowbytes=(word_count+3)*2
 proof[stem]={'inputs':case_count,'checkpoints':len(raw)//rowbytes,'modeled_words':word_count,'corpus_sha256':hashlib.sha256(raw).hexdigest(),'whole_sanitizer_corpus_exact':True,'target_images':[]}
 prefix={'throttle-feedback-pairs':'feedback-pair','feedback-length':'feedback-length','horizontal-phased':'horizontal-phase'}[stem]
 for i in range(images):
  name=f'{prefix}{i}'
  for suffix in ['.prg','.map','-build.log','-validation.log','-vice-mailbox.bin','-hardware-stack.bin','-software-stack.bin','-vice.log','-vice-monitor.log','-vice.mon']:copy(b/(name+suffix),name+suffix)
  mapping=(b/f'{name}.map').read_text();ends=[]
  for section in ['.text','.rodata','.data','.bss','.noinit']:
   m=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M);assert m;ends.append(int(m[1],16)+int(m[2],16))
  assert max(ends)<=0xc000
  mailbox=(b/f'{name}-vice-mailbox.bin').read_bytes();assert mailbox==bytes([0xa5,0,0,32,i]);log=(b/f'{name}-validation.log').read_text();hw=int(re.search('hardware-stack.bin: measured stack extent (\\d+)',log)[1]);sw=int(re.search('software-stack.bin: measured stack extent (\\d+)',log)[1]);proof[stem]['target_images'].append({'selector':i,'rows':32,'prg_bytes':(b/f'{name}.prg').stat().st_size,'loaded_end':max(ends),'hardware_stack':hw,'software_stack':sw,'mailbox':mailbox.hex()})
proof['source']={'original_luminary_interpreter_sha256':sha(r.parent/'Luminary099/INTERPRETER.agc'),'original_luminary_landing_sha256':sha(r.parent/'Luminary099/LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'),'fixture_rope_sha256':sha(r/'deps/fixture/MAIN.agc.bin'),'pinned_engine_sha256':sha(r/'deps/virtualagc/yaAGC/agc_engine.c'),'oracle_DV':'Existing pinned SimulateDV selected for every DV; one host-only branch change, native arithmetic contains no instruction engine.'}
assert proof['source']['original_luminary_interpreter_sha256']==sha(r/'deps/fixture/INTERPRETER.agc')
assert proof['source']['original_luminary_landing_sha256']==sha(r/'deps/fixture/LUNAR_LANDING_GUIDANCE_EQUATIONS.agc')
p=e/'p66-fine-phases-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');add(p)
# Ownership list, deliberately excludes existing generic UNIT and published stages.
owned=['include/guidance_feedback_pairs.h','include/guidance_length_phased.h','include/guidance_horizontal_phased.h','src/guidance_feedback_pairs.c','src/guidance_length_phased.c','src/guidance_horizontal_phased.c','tests/throttle_feedback_pairs_cases.h','tests/throttle_feedback_pairs_cases.c','tests/throttle_feedback_pairs_vectors.h','tests/test_throttle_feedback_pairs.c','tests/feedback_length_vectors.h','tests/test_feedback_length.c','tests/horizontal_phased_cases.h','tests/horizontal_phased_cases.c','tests/horizontal_phased_vectors.h','tests/test_horizontal_phased.c','docs/P66-FINE-PHASES.md']
for stem in ['throttle_feedback_pairs','feedback_length','horizontal_phased']:
 owned += [f'tools/{stem}_{suf}'for suf in ['reference.c','snapshot.c','diagnostic.c','vice_test.py']]
 owned += [f'tools/freeze_{stem}.py']
owned+=['tools/throttle_feedback_pairs.mk','tools/horizontal_phased.mk','tools/p66_fine_phases_report.py']
files=owned+files
for file in files:assert(r/file).is_file(),file
manifest=e/'p66-fine-phases-evidence.sha256';manifest.write_text(''.join(sha(r/f)+'  '+f+'\n'for f in sorted(files)))
files.append(str(manifest.relative_to(r)));(b/'p66-fine-phases-commit-files.txt').write_text('\n'.join(sorted(files))+'\n')
print(json.dumps({k:(v['checkpoints'],v['corpus_sha256'])for k,v in proof.items()if k!='source'},indent=2));print('Files',len(files))
