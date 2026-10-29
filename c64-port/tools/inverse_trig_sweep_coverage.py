#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Inspect actual factory images independently of nominal case-index claims."""
import pathlib,subprocess,struct,json
r=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/inverse_trig_inputs.c','tests/inverse_trig_sweep_cases.c','src/agc_word.c','-o','build/inverse-trig-sweep-inputs'],cwd=r,check=True)
rows=[]
for mission in range(2):
 p=subprocess.Popen(['build/inverse-trig-sweep-inputs',str(mission)],cwd=r,stdin=subprocess.PIPE,stdout=subprocess.PIPE)
 #128KiB request requires concurrent write/read; communicate stores only256MiB.
 data,_=p.communicate(b''.join(struct.pack('<H',n)for n in range(65536)))
 assert p.returncode==0 and len(data)==65536*4098
 seen=[set(),set()]
 for n in range(65536):seen[n&1].add(struct.unpack_from('<H',data,n*4098+2*(1+0o154))[0])
 assert all(s==set(range(32768))for s in seen)
 rows.append(dict(mission='lm'if mission==0 else'cm',factory_inputs=65536,arccos_distinct_high_words=len(seen[0]),arcsin_distinct_high_words=len(seen[1]),missing_high_words=[]))
 del data
(r/'build/inverse-trig-sweep-input-coverage.json').write_text(json.dumps(rows,indent=2)+'\n');print(rows)
