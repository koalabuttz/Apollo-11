#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit each original emitted POWERED word to a native source group."""
import pathlib,re,json,hashlib,subprocess
r=pathlib.Path(__file__).resolve().parents[1]
src=r.parent/'Luminary099/POWERED_FLIGHT_SUBROUTINES.agc'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='379505fe1429c8b0d6ef20d85bad9d8fb991083fd7202dc89c46e584f0ba92b1'
listing=(r/'deps/lm-powered-fixture/assembly.lst').read_text().split('$POWERED_FLIGHT_SUBROUTINES.agc',1)[1].split('\n',1)[1]
listing=listing.split('\n$')[0]
groups=[(67,75,'permanent_interface'),(77,116,'cdu_trig'),(125,149,'quick_trig'),(171,208,'permanent_interface'),(228,290,'axis_rotation'),(292,298,'immutable_constants'),(310,314,'fleshpot_interface'),(316,316,'immutable_constants'),(318,318,'fleshpot_interface'),(320,446,'fleshpot')]
words=[]
for line in listing.splitlines():
 if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
 match=re.match(r'^\d+,(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?',line)
 if not match:continue
 lineno=int(match[1]);groups_here=[label for lo,hi,label in groups if lo<=lineno<=hi]
 assert len(groups_here)==1,(lineno,line)
 for k,value in enumerate([match[4],match[5]]):
  if value is None:continue
  bank=int(match[2],8);address=int(match[3],8)+k
  words.append({'original_line':lineno,'bank':match[2],'address':f'{address:04o}','physical_fixed_word':bank*1024+address-1024,'raw_octal':value,'native_group':groups_here[0],'unreachable_under_entry_contract':lineno==288})
assert len(words)==267,len(words)
assert len([w for w in words if w['native_group']=='immutable_constants'])==6
payload={'original_file_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'rope_sha256':'1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e','emitted_words':len(words),'words':words,'unreachable_note':'TC RTNSAVER at source line288 takes positive-zero CCS skip; DIM preserves sign and reaches negative zero from either documented +3/-3 direction, so valid AX callers execute line290 instead.'}
(r/'build/lm-powered-file-source-map.json').write_text(json.dumps(payload,indent=2)+'\n')
constants=[w for w in words if w['native_group']=='immutable_constants']
assert [int(w['raw_octal'],8)for w in constants]==[0o736,4,2,0,4,0o2664]
program='#include <stdio.h>\n#include "lm_powered_constants.h"\nint main(void){unsigned f=0;uint16_t value=0;\n'+''.join(f'if(!lm_powered_constant_read({w["physical_fixed_word"]},&value)||value!={int(w["raw_octal"],8)})++f;\n'for w in constants)+'printf("6 original POWERED fixed constants: %s\\n",f?"FAIL":"PASS");return f?1:0;}\n'
(r/'build/lm-powered-constants-check.c').write_text(program)
subprocess.run(['gcc','-O2','-std=c99','-Wall','-Wextra','-Werror','-pedantic','-Iinclude','build/lm-powered-constants-check.c','src/lm_powered_constants.c','-o','build/lm-powered-constants-check'],cwd=r,check=True)
subprocess.run(['build/lm-powered-constants-check'],cwd=r,check=True)
print('All',len(words),'original emitted words mapped to native groups.')
