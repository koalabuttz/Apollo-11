#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Author explicit native source branches offline; never emit source opcodes."""
from pathlib import Path
import re,hashlib,json
R=Path(__file__).resolve().parents[1];parts=[];audit={}
for short,mission in [('lm','Luminary099'),('cm','Comanche055')]:
 source=(R.parent/mission/'EXTENDED_VERBS.agc').read_bytes();listing=(R/f'build/dsky-extended-dispatch-{short}-fixture/assembly.lst').read_text();branches=[]
 for line in listing.splitlines():
  m=re.match(r'^\d+,\d+:\s+43,([0-7]{4})\s+([0-7]{5})\s+(.*)$',line)
  if not m or not 0o2002<=int(m[1],8)<0o2002+60:continue
  operation=re.search(r'\b(TCF|TC)\s+(\S+)',m[3]);assert operation
  mnemonic,label=operation.groups();target=int(m[2],8)&0o7777
  branches.append({'verb':40+len(branches),'source_pc':m[1],'mnemonic':mnemonic,'source_label':label,'target_pc':format(target,'04o')})
 assert len(branches)==60
 assert [x['verb']for x in branches if x['mnemonic']=='TCF']==([49]if short=='lm'else[49,96])
 parts.append('static uint16_t '+short+'_extended_target(uint16_t index){switch(index){')
 parts.extend(' case '+str(x['verb']-40)+'u:return 0'+x['target_pc']+';'for x in branches)
 parts.append(' default:return 0;}}')
 audit[short]={'source_sha256':hashlib.sha256(source).hexdigest(),'representation':'explicit compiled native source branches, no source opcode storage or runtime decoder','branches':branches}
(R/'src/dsky_extended_dispatch_targets.inc').write_text('/* SPDX-License-Identifier: MIT; explicit native continuations from original source labels. */\n'+'\n'.join(parts)+'\n')
(R/'build/dsky-extended-dispatch-target-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
