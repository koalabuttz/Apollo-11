#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit each original emitted POWERED word to a native source group."""
import pathlib,re,json,hashlib,subprocess
r=pathlib.Path(__file__).resolve().parents[1]
src=r.parent/'Comanche055/POWERED_FLIGHT_SUBROUTINES.agc'
assert hashlib.sha256(src.read_bytes()).hexdigest()=='dbf2f3db7c28220733b86b30b994cfccf29f4cfce0c4e59084c13bdbeccc5766'
listing=(r/'deps/cm-powered-fixture/assembly.lst').read_text().split('$POWERED_FLIGHT_SUBROUTINES.agc',1)[1].split('\n',1)[1]
listing=listing.split('\n$')[0]
groups=[(69,77,'permanent_interface'),(79,118,'cdu_trig'),(128,152,'quick_trig'),(162,172,'legacy_nbsm'),(189,228,'permanent_interface'),(250,314,'axis_rotation'),(317,323,'immutable_constants'),(342,371,'smcdure')]
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
  words.append({'original_line':lineno,'bank':match[2],'address':f'{address:04o}','physical_fixed_word':bank*1024+address-1024,'raw_octal':value,'native_group':groups_here[0],'unreachable_under_entry_contract':lineno==312})
assert len(words)==187,len(words)
assert len([w for w in words if w['native_group']=='immutable_constants'])==5
payload={'original_file_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'rope_sha256':'6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061','emitted_words':len(words),'words':words,'unreachable_note':'TC RTNSAVER at source line312 takes positive-zero CCS skip; DIM preserves sign and reaches negative zero from either documented +3/-3 direction, so valid AX callers execute line314 instead.'}
(r/'build/cm-powered-file-source-map.json').write_text(json.dumps(payload,indent=2)+'\n')
constants=[w for w in words if w['native_group']=='immutable_constants']
assert [int(w['raw_octal'],8)for w in constants]==[0o736,4,2,0,4]
program='#include <stdio.h>\n#include "cm_powered_constants.h"\nint main(void){unsigned f=0;uint16_t value=0;\n'+''.join(f'if(!cm_powered_constant_read({w["physical_fixed_word"]},&value)||value!={int(w["raw_octal"],8)})++f;\n'for w in constants)+'printf("5 original POWERED fixed constants: %s\\n",f?"FAIL":"PASS");return f?1:0;}\n'
(r/'build/cm-powered-constants-check.c').write_text(program)
subprocess.run(['gcc','-O2','-std=c99','-Wall','-Wextra','-Werror','-pedantic','-Iinclude','build/cm-powered-constants-check.c','src/cm_powered_constants.c','-o','build/cm-powered-constants-check'],cwd=r,check=True)
subprocess.run(['build/cm-powered-constants-check'],cwd=r,check=True)
print('All',len(words),'original emitted words mapped to native groups.')
