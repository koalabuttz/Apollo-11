#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit scalar ephemerides time-scale dependencies in byte-original mission ropes."""
from pathlib import Path
import hashlib,json,subprocess
from fixed_data_generate import listing_records,source_line
R=Path(__file__).resolve().parents[1];result={}
for mission,name in [('Luminary099','LM'),('Comanche055','CM')]:
 work=R/'build/ephemeris-source'/mission
 with (work/'time-scale-assembly.lst').open('w')as out:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=out,stderr=subprocess.STDOUT,check=True)
 rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/f'evidence/fixed-data-{name.lower()}-original.rope.bin').read_bytes();path=mission+'/INTERPRETER.agc';original=(R.parent/path).read_text();audit=json.loads((R/'evidence/ephemeris-source-map.json').read_text())[name]['native_arithmetic_dependency'];assert hashlib.sha256((work/'INTERPRETER.agc').read_bytes()).hexdigest()==audit['assembled_source_sha256'];all_records=listing_records(work/'time-scale-assembly.lst',{'INTERPRETER.agc':path},{path:original},rope);selected=set();active=False
 for n,line in enumerate(original.splitlines(),1):
  x=source_line(line)
  if x is None:continue
  label=x.get('label','')
  if label and label[0]not in '+-':active=label in {'GENSHIFT','GENSHFT2','RIGHT','GENSCR','TSSL'}
  if active:selected.add(n)
 records=[r for r in all_records if r['line']in selected];result[name]=dict(original_source=path,original_sha256=hashlib.sha256(original.encode()).hexdigest(),assembled_sha256=audit['assembled_source_sha256'],audited_assembly_dependency='evidence/ephemeris-source-map.json',rope_sha256=hashlib.sha256(rope).hexdigest(),group_words=sum(len(r['words'])for r in records),source_line_word_map=records,scope='Positive scalar SR14 at original GENSHIFT with A LOW7 and true processed ADDRWD020616; no whole INTERPRETER or ephemerides credit')
(R/'build/ephemeris-time-scale-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS source shift dependencies',[(n,x['group_words'])for n,x in result.items()])
