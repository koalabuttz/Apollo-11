#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib,json,subprocess
from fixed_data_generate import listing_records,source_line
R=Path(__file__).resolve().parents[1];result={}
for mission,name in [('Luminary099','LM'),('Comanche055','CM')]:
 work=R/'build/ephemeris-source'/mission
 with(work/'day-assembly.lst').open('w')as out:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=out,stderr=subprocess.STDOUT,check=True)
 rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/f'evidence/fixed-data-{name.lower()}-original.rope.bin').read_bytes();path=mission+'/INTERPRETER.agc';original=(R.parent/path).read_text();audit=json.loads((R/'evidence/ephemeris-source-map.json').read_text())[name]['native_arithmetic_dependency'];assert hashlib.sha256((work/'INTERPRETER.agc').read_bytes()).hexdigest()==audit['assembled_source_sha256'];records=listing_records(work/'day-assembly.lst',{'INTERPRETER.agc':path},{path:original},rope);selected=set();active=False
 for n,line in enumerate(original.splitlines(),1):
  x=source_line(line)
  if x is None:continue
  if x.get('label')=='DDV/BDDV':active=True
  if x.get('label')=='V/SC':active=False
  if active:selected.add(n)
 group=[r for r in records if r['line']in selected];result[name]=dict(original_source=path,original_sha256=hashlib.sha256(original.encode()).hexdigest(),assembled_sha256=audit['assembled_source_sha256'],audited_assembly_dependency='evidence/ephemeris-source-map.json',rope_sha256=hashlib.sha256(rope).hexdigest(),source_line_word_map=group,scope='DDV basic body with literal positive divisor CSTODAY in BUF; mapped surrounding division bodies include unsupported other-divisor branches, no whole interpreter or ephemerides credit')
(R/'build/ephemeris-day-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS both byte-original division source maps')
