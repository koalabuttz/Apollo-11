#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Count actual input-factory highwords and MODE; avoid nominal sweep claims."""
import pathlib,subprocess,struct,json
r=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/interpreted_sign_coverage.c','tests/interpreted_sign_cases.c','src/agc_word.c','-o','build/interpreted-sign-coverage'],cwd=r,check=True)
data=subprocess.check_output(['build/interpreted-sign-coverage'],cwd=r);assert len(data)==131072*6
rows=[]
for kind in range(2):
 values=[set(),set(),set()]
 for n in range(kind*65536,(kind+1)*65536):
  for i,v in enumerate(struct.unpack_from('<3H',data,n*6)):values[i].add(v)
 assert values[1]==values[2]==set(range(32768))
 assert values[0]==set(range(32767))
 rows.append(dict(entry='COMP'if kind==0 else'SIGN',inputs=65536,distinct_MODE_words=len(values[0]),missing_valid_MODE_words=32767-len(values[0]),distinct_SIGN_high_words=len(values[1]),distinct_MPAC_high_words=len(values[2]),ordinary_high_word_values=32768))
(r/'build/interpreted-sign-input-coverage.json').write_text(json.dumps(rows,indent=2)+'\n');print(rows)
