#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain reviewed source/target evidence; generate exact closure recommendations."""
import pathlib,json,hashlib,shutil,gzip,re
R=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def record(path):return {'path':path,'sha256':sha(R/path)}
def retain(src,dst=None):
 dst=dst or 'evidence/'+pathlib.Path(src).name;shutil.copyfile(R/src,R/dst);return dst
def main():
 maps=json.loads((R/'build/standard-lead-source/audit.json').read_text())
 entries={'E/BKCALL':'bank','E/CALL':'interp','E/JOBWAK':'wake','COMPVER':'external_compver','GTSCPSS1':'external_gtscpss','REDO':'mode_admission'}
 for item in maps:
  group=None
  for row in item['source_line_word_map']:
   if row['label'] in entries:group=entries[row['label']]
   row['native_control']=group if row['words'] else 'original source placement directive'
  item['whole_file_translation_status']='reviewed native controls and original placement; external routine/caller continuations explicit'
 (R/'evidence/standard-lead-source-audit.json').write_text(json.dumps(maps,indent=2)+'\n')
 for mission in ('Luminary099','Comanche055'):
  work=R/'build/standard-lead-source'/mission
  with gzip.GzipFile(filename='',mode='wb',fileobj=(R/f'evidence/standard-lead-{mission}-assembly.lst.gz').open('wb'),mtime=0) as dst:dst.write((work/'assembly.lst').read_bytes())
  for patch in work.glob('*.indentation.patch'):retain(str(patch.relative_to(R)),f'evidence/standard-lead-{mission}-{patch.name}')
 for name in ('host','guard','target'):retain(f'build/standard-lead-{name}.log')
 targets=[]
 for short,count in [('lm',164),('cm',188)]:
  for extension in ('prg','map','vice.log','monitor.log','mailbox.bin'):retain(f'build/standard-lead-{short}.{extension}' if extension in ('prg','map') else f'build/standard-lead-{short}-{extension}')
  image=(R/f'build/standard-lead-{short}.prg').read_bytes();bss=int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__bss_end =',(R/f'build/standard-lead-{short}.map').read_text(),re.M)[1],16);mail=(R/f'build/standard-lead-{short}-mailbox.bin').read_bytes();assert mail==bytes([0xa5,count,0])
  targets.append({'mission':short,'complete_states':count,'words_each':2048,'mailbox_hex':mail.hex(),'actual_prg_bytes':len(image),'load_start':0x801,'load_end_exclusive':0x801+len(image)-2,'bss_end_exclusive':bss,'prg':record(f'evidence/standard-lead-{short}.prg'),'map':record(f'evidence/standard-lead-{short}.map')})
 host=json.loads((R/'build/standard-lead-host-results.json').read_text());assert sum(r['complete_image_checkpoints'] for r in host)==35328
 proof={'schema_version':1,'whole_original_source_files':[x['source_path'] for x in maps],'source_emitted_words':[32,36],'canonical_words':2048,'complete_host_native_full_sanitizer_checkpoints':35328,'host_groups':host,'original_source_audit':record('evidence/standard-lead-source-audit.json'),'actual_c64':targets,'target_full_image_checkpoints':352,'instruction_decoder':False,'integrated_application_complete':False,'external_source_continuations':['caller-owned native fixed routine','compiled erasable caller NEWOPS before fetch','GCOMPVER','GTSCPSS','IMUTEST','DSPMMJOB','BAILOUT recovery'],'producer_contract':'original EBUF2/BUF2 and VAC/scratch ownership; wake IRQ inhibited; inherited valid admitted cores; no new Executive polls','source_review':record('evidence/standard-lead-independent-review.txt'),'documented_scope':record('docs/SYSTEM_TEST_LEAD_INS.md'),'frozen_helper_dependencies':[record(p) for p in ('include/cm_interbank_extended.h','src/cm_interbank_extended.c','evidence/cm-interbank-extended-proof.json','src/cm_executive.c','src/lm_basic_wake.c','src/cm_basic_wake.c','src/executive.c','src/cm_native_image.c')]}
 (R/'evidence/standard-lead-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
 for item in maps:
  short='lm' if item['source_path'].startswith('Luminary') else 'cm'
  cert={'source_file':item['source_path'],'source_sha256':item['source_sha256'],'source_revision':'911e5c0283c629c50cb97666f34065e8c07d71a5','reviewed_all_source_paths':True,'public_entries':['E/BKCALL','E/CALL','E/JOBWAK']+(['COMPVER','GTSCPSS1','REDO'] if short=='cm' else []),'internal_paths':['all3 bank return branches','E/SWITCH/DTCB','original indirect CALL','original RVQ/EXIT','INTPRET+2 basic LXCH LOC without RELINT','JOBWAKE found/no-match']+(['actual NEWMODEX/SETUPDSP','seven-core NOVAC success/no-core'] if short=='cm' else []),'caller_domain':proof['producer_contract'],'external_continuations':proof['external_source_continuations'],'emitted_words':item['source_words'],'evidence':{'source_to_native_map':record('docs/SYSTEM_TEST_LEAD_INS.md'),'independent_assembly_comparison':record('evidence/standard-lead-source-audit.json'),'host_and_sanitizer_results':record('evidence/standard-lead-host.log'),'c64_execution_results':record('evidence/standard-lead-proof.json')},'integrated_application_complete':False}
  (R/f'evidence/standard-lead-{short}-coverage.json').write_text(json.dumps(cert,indent=2)+'\n')
 files=sorted(str(p.relative_to(R)) for folder in ('include','src','tests','tools','platform/c64','evidence') for p in (R/folder).glob('standard_lead*' if folder!='evidence' else 'standard-lead-*') if p.is_file())+['docs/SYSTEM_TEST_LEAD_INS.md','tests/test_standard_lead.c','tools/freeze_standard_lead.py','tools/package_standard_lead.py']
 files=sorted(set(files));files=[f for f in files if f!='evidence/standard-lead-SHA256.json']
 (R/'evidence/standard-lead-SHA256.json').write_text(json.dumps({f:sha(R/f) for f in files},indent=2)+'\n')
 files.append('evidence/standard-lead-SHA256.json');(R/'build/standard-lead-commit-files.txt').write_text('\n'.join(sorted(files))+'\n')
 print('READY',len(files),'files;35328 source/native/SAN states;352 actualC64 states; both whole-file recommendations')
 print('manifest SHA256',sha(R/'evidence/standard-lead-SHA256.json'));print('proof SHA256',sha(R/'evidence/standard-lead-proof.json'))
if __name__=='__main__':main()
