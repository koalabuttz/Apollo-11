#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Package exact complete-state proof, deduplicating equal stream content."""
from pathlib import Path
import hashlib,json,gzip,shutil,re
r=Path(__file__).resolve().parents[1];b=r/'build';e=r/'evidence';files=[]
def sha(p):
 h=hashlib.sha256()
 with p.open('rb')as f:
  for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
 return h.hexdigest()
def add(p):files.append(str(p.relative_to(r)))
def copy(src,name):p=e/name;shutil.copyfile(src,p);add(p);return p
def pack(src,name):
 p=e/name
 with src.open('rb')as f,p.open('wb')as out:
  with gzip.GzipFile(filename='',mode='wb',fileobj=out,mtime=0)as z:shutil.copyfileobj(f,z,1024*1024)
 add(p);return p
main_files={name:b/f'cm-trig-{name}.snapshots'for name in ['reference','native','sanitize']};identity={k:sha(v)for k,v in main_files.items()};assert len(set(identity.values()))==1
main=pack(main_files['reference'],'cm-trig-complete.snapshots.gz');counts={'SPSIN':65536,'SPCOS':65536,'QUICTRIG':33024};assert main_files['reference'].stat().st_size==sum(counts.values())*4102
clock_files={k:b/f'cm-clock-trig-{k}.snapshots'for k in ['reference','native','sanitize']};clock_id={k:sha(v)for k,v in clock_files.items()};assert clock_id['native']==clock_id['sanitize'];clock_ref=pack(clock_files['reference'],'cm-clock-trig-reference.snapshots.gz');clock_native=pack(clock_files['native'],'cm-clock-trig-native-sanitize.snapshots.gz')
proof={'modeled_words':2048,'external_IRQ_word':1,'main_cases':counts,'main_complete_states':sum(counts.values()),'logical_main_streams':{k:{'sha256':v,'object':str(main.relative_to(r))}for k,v in identity.items()},'clockjob_cases':512,'logical_clockjob_streams':{k:{'sha256':v,'object':str((clock_ref if k=='reference'else clock_native).relative_to(r))}for k,v in clock_id.items()},'whole_strict_and_sanitizer_streams_match':True,'source_audit':json.loads((b/'cm-trig-source-audit.json').read_text()),'target_images':[],'coverage':{'Comanche055/SINGLE_PRECISION_SUBROUTINES.agc':{'numeric_public_entries':['SPSIN','SPCOS'],'internal_labels':['SPT','POLLEY','ARG90'],'final_TC_Q_return':'Caller control continuation remains to qualify; not yet a complete whole-file control certificate.'},'Comanche055/POWERED_FLIGHT_SUBROUTINES.agc':{'entry':'QUICTRIG','exit':'Before TC A at23,3605; caller owns BANKCALL/return control bridge.'}}}
for src in ['cm-trig-reference.trace','cm-trig-sanitize-validation.log','cm-clock-trig-reference.trace']:copy(b/src,src)
for n in range(4):
 name=f'cm-trig{n}'
 for suffix in ['.prg','.map','-build.log','-validation.log','-vice-mailbox.bin','-hardware-stack.bin','-software-stack.bin','-vice.log','-vice-monitor.log','-vice.mon']:copy(b/(name+suffix),name+suffix)
 mapping=(b/f'{name}.map').read_text();ends=[]
 for section in ['.text','.rodata','.data','.bss','.noinit']:
  m=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M);assert m;ends.append(int(m[1],16)+int(m[2],16))
 for section in ['.zp.data','.zp.bss']:
  m=re.search(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M);assert m and int(m[1],16)==0
 mailbox=(b/f'{name}-vice-mailbox.bin').read_bytes();assert mailbox==bytes([0xa5,0,0,32,n]);assert max(ends)<0xc000
 log=(b/f'{name}-validation.log').read_text();proof['target_images'].append({'selector':n,'states':32,'prg_bytes':(b/f'{name}.prg').stat().st_size,'loaded_end':max(ends),'hardware_stack':int(re.search('hardware-stack.bin: measured stack extent (\\d+)',log)[1]),'software_stack':int(re.search('software-stack.bin: measured stack extent (\\d+)',log)[1]),'zero_page':0,'mailbox':mailbox.hex()})
copy(b/'cm-trig-source-audit.json','cm-trig-source-audit.json');pack(r/'deps/cm-trig-fixture/assembly.lst','cm-trig-original-source-assembly.lst.gz')
for name in ['POWERED_FLIGHT_SUBROUTINES.agc','SINGLE_PRECISION_SUBROUTINES.agc','P40-P47.agc']:
 copy(r/'deps/cm-trig-fixture'/(name+'.original'),'cm-trig-'+name+'.original');copy(r/'deps/cm-trig-fixture'/(name+'.indentation.patch'),'cm-trig-'+name+'.indentation.patch')
p=e/'cm-trig-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');add(p)
owned=['include/cm_quictrig.h','src/cm_quictrig.c','tests/cm_trig_cases.h','tests/cm_trig_cases.c','tests/cm_trig_vectors.h','tests/test_cm_trig.c','docs/COMANCHE-QUICTRIG.md']
owned += ['tools/'+s for s in ['cm_trig_prepare_reference.py','cm_trig_reference.c','cm_trig_snapshot.c','cm_trig_inputs.c','cm_trig_diagnostic.c','cm_trig_vice_test.py','cm_trig_report.py','cm_trig.mk','freeze_cm_trig.py','cm_clock_trig_reference.c','cm_clock_trig_snapshot.c','cm_clock_trig_compare.py']]
files=owned+files
for f in files:assert(r/f).is_file(),f
manifest=e/'cm-trig-evidence.sha256';manifest.write_text(''.join(sha(r/f)+'  '+f+'\n'for f in sorted(files)));files.append(str(manifest.relative_to(r)));(b/'cm-trig-commit-files.txt').write_text('\n'.join(sorted(files))+'\n')
print('READY:',sum(counts.values()),'complete image states and512actualCLOCKJOB outputs;SHA',identity['reference']);print('Main compressed bytes',main.stat().st_size,'128completeC64states;files',len(files));print('Maxload',hex(max(i['loaded_end']for i in proof['target_images'])),'HW',max(i['hardware_stack']for i in proof['target_images']),'SW',max(i['software_stack']for i in proof['target_images']))
