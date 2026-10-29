#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Preserve and assemble original CM restart table data without source edits."""
import pathlib,subprocess,shutil,hashlib,json,re
R=pathlib.Path(__file__).resolve().parents[1];S=R/'deps/cm-services-fixture';F=R/'deps/cm-restart-types-fixture';V=R/'deps/virtualagc'
PIN='1b5d6474841a7ab53237ead070f76f35f162f68d';assert subprocess.check_output(['git','-C',str(V),'rev-parse','HEAD'],text=True).strip()==PIN
if not (S/'MAIN.agc.bin').exists():subprocess.run(['python3','tools/cm_services_prepare_reference.py'],cwd=R,check=True)
F.mkdir(exist_ok=True)
for p in S.glob('*.agc'):shutil.copyfile(p,F/p.name)
original=(R.parent/'Comanche055/RESTART_TABLES.agc').read_bytes();(F/'RESTART_TABLES.agc.original').write_bytes(original);(F/'RESTART_TABLES.agc').write_bytes(original)
with (F/'assembly.lst').open('w') as o:subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=o,stderr=subprocess.STDOUT,check=True)
a=(F/'MAIN.agc.bin').read_bytes();assert a==(S/'MAIN.agc.bin').read_bytes()
metadata={'pin':PIN,'original_source_sha256':hashlib.sha256(original).hexdigest(),'source_repairs':0,'rope_sha256':hashlib.sha256(a).hexdigest()}
(R/'build/cm-restart-types-source-audit.json').write_text(json.dumps(metadata,indent=2)+'\n');print(json.dumps(metadata,indent=2))
# Only literal restart table data is exported. No executable source words.
listing=(F/'assembly.lst').read_text();data={}
for line in listing.splitlines():
 m=re.match(r'^\d+,\d+: 01,([0-7]{4})\s+([0-7]{5})(?: ([0-7]{5}))?\s+',line)
 if not m:continue
 address=int(m[1],8)
 if not 0o2000<=address<=0o2370:continue
 data[address]=int(m[2],8)
 if m[3]:data[address+1]=int(m[3],8)
assert sorted(data)==list(range(0o2000,0o2371))
h='/* SPDX-License-Identifier: MIT\n * Original CM RESTART_TABLES scalar priorities, times, addresses and index constants.\n * No executable source words. Original SHA256: '+metadata['original_source_sha256']+' */\n#ifndef CM_RESTART_TABLE_DATA_H\n#define CM_RESTART_TABLE_DATA_H\n#include "agc_word.h"\nstatic const agc_word cm_restart_table_data[249]={\n'+','.join('0'+format(data[i],'o') for i in sorted(data))+'};\n#endif\n'
assert len(data)==249
(R/'include/cm_restart_table_data.h').write_text(h)
