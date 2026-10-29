#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Full original lead-in/source controls and sanitized canonical-image proof."""
import pathlib,subprocess,os,filecmp,lzma,hashlib,json,shutil
R=pathlib.Path(__file__).resolve().parents[1]
CORE=['src/standard_lead.c','src/standard_lead_interp.c','src/standard_lead_bank.c','src/standard_lead_wake.c','src/standard_lead_cm.c','src/cm_interbank_extended.c','src/lm_basic_wake.c','src/executive.c','src/cm_basic_wake.c','src/cm_native_image.c','src/cm_dispatch.c','src/cm_executive.c','src/agc_word.c']
CASES=['tests/standard_lead_cases.c','tests/standard_lead_bank_cases.c','tests/standard_lead_interp_cases.c','tests/standard_lead_wake_cases.c','tests/standard_lead_cm_cases.c','tests/standard_lead_resume_cases.c']
FAMILIES=[('',3584,1),('_bank',2048,3),('_interp',1024,4),('_wake',2048,1),('_resume',1024,1)]
def run(args,**kw):subprocess.run(args,cwd=R,check=True,**kw)
def sha(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(1<<20),b''):h.update(b)
 return h.hexdigest()
def main():
 run(['python3','tools/standard_lead_source.py'])
 common=['-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']
 objects=[f'deps/virtualagc/yaAGC/{n}.o' for n in ('agc_engine','agc_engine_init','rfopen')]
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0');records=[]
 for mission,short in enumerate(('lm','cm')):
  for suffix,cases,steps in FAMILIES+([('_cm',1536,1)] if mission else []):
   label='extra' if suffix=='_cm' else suffix.strip('_') or 'stage';prefix=f'build/standard-lead-{short}-{label}';macro=f'-DSTANDARD_LEAD_MISSION=STANDARD_LEAD_{short.upper()}'
   for mode,flags in [('native',['-O2']),('sanitize',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
    output=prefix+'-'+mode;run(['cc',*flags,*common,macro,f'tools/standard_lead{suffix}_snapshot.c',*CASES,*CORE,'-o',output])
    with (R/(output+'.snapshots')).open('wb') as out,(R/(output+'.io')).open('wb') as io:run([output],stdout=out,stderr=io,env=env)
   output=prefix+'-reference';run(['cc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC',macro,f'tools/standard_lead{suffix}_reference.c',*CASES,'tests/scenarios.c','src/agc_word.c',*objects,'-o',output])
   with (R/(output+'.snapshots')).open('wb') as out,(R/(output+'.io')).open('wb') as io:run([output,f'build/standard-lead-source/{"Luminary099" if mission==0 else "Comanche055"}/MAIN.agc.bin'],stdout=out,stderr=io)
   reference=R/(output+'.snapshots')
   assert sum(1 for _ in reference.open('rb'))==cases*steps
   for mode in ('native','sanitize'):assert filecmp.cmp(reference,R/(prefix+'-'+mode+'.snapshots'),shallow=False),(short,label,mode)
   if label=='extra':
    for mode in ('native','sanitize'):assert filecmp.cmp(R/(output+'.io'),R/(prefix+'-'+mode+'.io'),shallow=False)
   compressed=R/f'evidence/standard-lead-{short}-{label}.snapshots.xz'
   with reference.open('rb') as src,lzma.open(compressed,'wb',preset=6) as dst:shutil.copyfileobj(src,dst)
   records.append({'mission':short,'family':label,'input_cases':cases,'complete_image_checkpoints':cases*steps,'words_each':2048,'source_native_sanitized_sha256':sha(reference),'lossless_snapshot':str(compressed.relative_to(R)),'compressed_sha256':sha(compressed)})
   print('PASS',short,label,cases*steps,'complete source/native/sanitized images',flush=True)
 (R/'build/standard-lead-host-results.json').write_text(json.dumps(records,indent=2)+'\n')
 run(['cc','-std=c99','-O2','-Iinclude','-Itests','tools/standard_lead_input_dump.c','tests/standard_lead_target_input.c',*CASES,'src/agc_word.c','-o','build/standard-lead-input-dump'])
 for mission in range(2):
  for kind,count in [(0,3584),(1,2048),(2,1024),(3,2048),(5,1024)]+([(4,1536)] if mission else []):
   with (R/f'build/standard-lead-input-{mission}-{kind}.snapshots').open('wb') as out:run(['build/standard-lead-input-dump',str(mission),str(kind),str(count)],stdout=out)
 run(['python3','tools/freeze_standard_lead.py'])
if __name__=='__main__':main()
