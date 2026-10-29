#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fresh original assembly audit of four entry/LSTIME source words."""
from pathlib import Path
import hashlib,json,subprocess
from fixed_data_generate import listing_records
R=Path(__file__).resolve().parents[1];work=R/'build/ephemeris-source/Comanche055';path='Comanche055/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc';original=(R.parent/path).read_text()
with(work/'cm-time-assembly.lst').open('w')as out:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=out,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/'evidence/fixed-data-cm-original.rope.bin').read_bytes();audit=json.loads((R/'evidence/ephemeris-source-map.json').read_text())['CM'];assert hashlib.sha256(original.encode()).hexdigest()==audit['source_sha256'];alias=next(f.name for f in work.glob('*.agc')if f.read_bytes()==original.encode());records=listing_records(work/'cm-time-assembly.lst',{alias:path},{path:original},rope);stop=next(i for i,line in enumerate(original.splitlines(),1)if line.startswith('RES\t'));selected=[r for r in records if r['line']<stop];assert selected
result=dict(original_source=path,source_sha256=audit['source_sha256'],rope_sha256=audit['rope_sha256'],source_line_word_map=selected,group_words=sum(len(r['words'])for r in selected),scope='Four original entry wrappers and LSTIME through before RES/REM/VEM bodies. Compiled C source control and existing canonical arithmetic, no decoder; containing ephemerides remains partial.',arithmetic_dependencies=['evidence/ephemeris-add-proof.json','evidence/ephemeris-time-scale-proof.json','evidence/ephemeris-shift-proof.json','src/interpreted_sign.c'])
(R/'build/cm-ephemeris-time-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS source entry and time composition words',result['group_words'])
