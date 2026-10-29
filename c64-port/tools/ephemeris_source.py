#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit original ephemerides and the native TAD dependency against both ropes."""
from pathlib import Path
import hashlib,json,shutil,subprocess
from fixed_data_generate import assemble,listing_records,source_line,symbols,filename_alias
R=Path(__file__).resolve().parents[1]
KEYS=('LSPOS','LUNPOS','LUNVEL','SOLPOS','LSTIME','RES','RESA','REM','REMA','VEM','VEMA','NINEB4','ONEB4','POSITA','POSITB','POSITD','POSITE','POSITF','STMP','GTMP','TIMEP','TEPHEM','TIMEMO','VECOEM','RESO','VESO','OMEGAES','KONMAT','RATESP','VAL67','CSTODAY','VMOON','VSUN','TAD','SETOVF','OVERFLOW','SETOVF2','DANZIG')
result={}
for mission,short in [('Luminary099','LM'),('Comanche055','CM')]:
 root=R/'build/ephemeris-source';assemble(mission,root);work=root/mission;name='LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc';path=mission+'/'+name;original=R.parent/path;includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')];alias=filename_alias(name,includes);shutil.copyfile(original,work/alias);helper_audit=json.loads((R/'evidence/rtb-vector-source-map.json').read_text())[short.lower()]['interpreter_helpers'];fixture_i=R/('deps/'+short.lower()+'-rtb-fixture/INTERPRETER.agc');assert hashlib.sha256(fixture_i.read_bytes()).hexdigest()==helper_audit['assembled_sha256'];shutil.copyfile(fixture_i,work/'INTERPRETER.agc')
 with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
 rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/('evidence/fixed-data-'+short.lower()+'-original.rope.bin')).read_bytes()
 text=original.read_text();records=listing_records(work/'assembly.lst',{alias:path},{path:text},rope);assert {r['line']for r in records}=={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}
 interp=mission+'/INTERPRETER.agc';orig_i=(R.parent/interp).read_text();all_helpers=listing_records(work/'assembly.lst',{'INTERPRETER.agc':interp},{interp:orig_i},rope)
 # Explicit source labels delimit the already reviewed basic TAD/OVERFLOW
 # groups; the following DSU/BVSU/DMPSUB/DOTSUB routines are not claimed here.
 selected=set();active=False
 for number,line in enumerate(orig_i.splitlines(),1):
  parsed=source_line(line)
  if parsed is None:continue
  label=parsed.get('label','')
  if label and label[0]not in '+-':active=label in {'TAD','SETOVF','OVERFLOW','SETOVF2'}
  if active:selected.add(number)
 helpers=[r for r in all_helpers if r['line']in selected]
 s=symbols((work/'assembly.lst').read_text());result[short]={'source_path':path,'source_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'assembled_source_sha256':hashlib.sha256((work/alias).read_bytes()).hexdigest(),'source_edits':0,'rope_sha256':hashlib.sha256(rope).hexdigest(),'source_words':sum(len(r['words'])for r in records),'source_line_word_map':records,'symbols':{k:s[k]for k in KEYS if k in s},'ephemerides_executable_port_status':'untranslated; source map only','native_arithmetic_dependency':{'source_path':interp,'source_sha256':hashlib.sha256((R.parent/interp).read_bytes()).hexdigest(),'assembled_source_sha256':helper_audit['assembled_sha256'],'indentation_repairs':helper_audit['indentation_repairs'],'audit_dependency':helper_audit['audit_dependency'],'basic_group':'TAD, SETOVF, OVERFLOW and SETOVF2','containing_file_status':'partial','source_line_word_map':helpers,'source_words':sum(len(r['words'])for r in helpers)}}
(R/'build/ephemeris-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS unchanged complete ephemerides source maps and TAD dependency bindings',[(k,v['source_words'],v['native_arithmetic_dependency']['source_words'])for k,v in result.items()])
