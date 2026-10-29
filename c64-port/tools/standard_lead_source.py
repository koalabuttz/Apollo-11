#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fresh byte-original lead-in files and original control helper assembly."""
import pathlib,hashlib,json,re,shutil,subprocess,difflib,gzip
from fixed_data_generate import listing_records,source_line,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 result=[]
 for mission in ('Luminary099','Comanche055'):
  work=R/'build/standard-lead-source'/mission;work.mkdir(parents=True,exist_ok=True)
  for p in (R/'build/fixed-data'/mission).glob('*.agc'):shutil.copyfile(p,work/p.name)
  support=[]
  for name in ('SYSTEM_TEST_STANDARD_LEAD_INS.agc','INTER-BANK_COMMUNICATION.agc','EXECUTIVE.agc','INTERPRETER.agc','PHASE_TABLE_MAINTENANCE.agc','POWERED_FLIGHT_SUBROUTINES.agc','RTB_OP_CODES.agc' if mission=='Luminary099' else 'RT8_OP_CODES.agc'):
   original=R.parent/mission/name;raw=original.read_bytes();text=raw.decode();normalized=text
   if mission=='Comanche055':
    normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',normalized,flags=re.M);normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M)
   (work/name).write_text(normalized);changes=[i for i,(a,b) in enumerate(zip(text.splitlines(),normalized.splitlines()),1) if a!=b]
   if name=='SYSTEM_TEST_STANDARD_LEAD_INS.agc':assert not changes
   patch=''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile=mission+'/'+name,tofile='fixture/'+name))
   (work/(name+'.indentation.patch')).write_text(patch)
   support.append({'source_path':mission+'/'+name,'source_sha256':hashlib.sha256(raw).hexdigest(),'fixture_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'indentation_only_repair_lines':changes,'numerical_edits':0})
  with (work/'assembly.lst').open('w') as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
  rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/'build/fixed-data'/mission/'MAIN.agc.bin').read_bytes()
  # These passive input literals are arbitrary untouched original fixed words
  # used only by the source/native inline-reader tests, never executed by C.
  fixed_header=(R/'tests/standard_lead_fixed_inputs.h').read_text()
  arrays=re.findall(r'\{([0-9,]+)\}',fixed_header)
  literals=list(map(int,arrays[0 if mission=='Luminary099' else 1].split(',')))
  assert literals==[rope_word(rope,i) for i in range(2048,2112)]
  name='SYSTEM_TEST_STANDARD_LEAD_INS.agc';path=mission+'/'+name;text=(R.parent/path).read_text();rows=listing_records(work/'assembly.lst',{name:path},{path:text},rope)
  assert {x['line'] for x in rows}=={i for i,line in enumerate(text.splitlines(),1) if source_line(line) is not None}
  result.append({'source_path':path,'source_sha256':hashlib.sha256(text.encode()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),'source_words':sum(len(x['words']) for x in rows),'source_line_word_map':rows,'original_control_support':support,'whole_file_translation_status':'pending native control composition and source proof','instruction_decoder':False})
 (R/'build/standard-lead-source/audit.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS: both byte-original lead-in files and fresh original interbank/Executive/interpreter/phase helper support, unchanged original-layout ropes')
 for record in result:print(record['source_path'],record['source_words'],'emitted words')
if __name__=='__main__':main()
