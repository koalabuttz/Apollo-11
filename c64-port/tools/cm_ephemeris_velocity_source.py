#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fresh original assembly audit of LUNPOS Horner/return source words."""
from pathlib import Path
import hashlib,json,subprocess
from fixed_data_generate import listing_records
R=Path(__file__).resolve().parents[1];work=R/'build/ephemeris-source/Comanche055';path='Comanche055/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc';original=(R.parent/path).read_text()
with(work/'cm-moon-assembly.lst').open('w')as out:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=out,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/'evidence/fixed-data-cm-original.rope.bin').read_bytes();audit=json.loads((R/'evidence/ephemeris-source-map.json').read_text())['CM'];assert hashlib.sha256(original.encode()).hexdigest()==audit['source_sha256'];alias=next(f.name for f in work.glob('*.agc')if f.read_bytes()==original.encode());records=listing_records(work/'cm-moon-assembly.lst',{alias:path},{path:original},rope);start=next(i for i,line in enumerate(original.splitlines(),1)if line.startswith('VEM\t'));stop=next(i for i,line in enumerate(original.splitlines(),1)if line.startswith('NINEB4\t'));selected=[r for r in records if start<=r['line']<stop];assert selected
result=dict(original_source=path,source_sha256=audit['source_sha256'],rope_sha256=audit['rope_sha256'],source_line_word_map=selected,group_words=sum(len(r['words'])for r in selected),scope='Original VEM/VEMA eighth-degree lunar derivative Horner loop, VSL2 and RVQ through actual orbital integration caller; earlier LUNPOS time path uses independently qualified composition. Other entry bodies remain open.',arithmetic_dependencies=['evidence/cm-ephemeris-time-proof.json','evidence/sband-linear-proof.json','evidence/sband-shift-proof.json'])
(R/'build/cm-ephemeris-velocity-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS source entry and time composition words',result['group_words'])
