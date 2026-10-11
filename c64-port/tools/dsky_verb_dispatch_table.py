#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Extract only the original40 passive CADR words, not executable source code."""
from pathlib import Path
import re,hashlib,json
R=Path(__file__).resolve().parents[1]
rows=[];audit={}
for short,mission in [('lm','Luminary099'),('cm','Comanche055')]:
 source=(R.parent/mission/'PINBALL_GAME_BUTTONS_AND_LIGHTS.agc').read_bytes()
 listing=(R/f'build/dsky-verb-dispatch-{short}-fixture/assembly.lst').read_text()
 words=[]
 for line in listing.splitlines():
  m=re.match(r'^\d+,\d+:\s+41,([0-7]{4})\s+([0-7]{5})\s+(.*)$',line)
  if m and 0o2151<=int(m[1],8)<0o2151+40:
   assert re.search(r'\bCADR\b',m[3]);words.append(int(m[2],8))
 assert len(words)==40
 text=source.decode().split('VERBTAB\t')[1].split('# THE LIST2')[0]
 labels=re.findall(r'^\s*(?:[A-Z][A-Z0-9]*\s+)?CADR\s+(\S+)',text,re.M)[:40]
 assert len(labels)==40
 rows.append('static const uint16_t verb_'+short+'[40]={'+','.join('0%05o'%w for w in words)+'};')
 audit[short]={'source_sha256':hashlib.sha256(source).hexdigest(),'first_source_pc':'41,2151','count':40,'passive_cadr_words':words,'source_labels':labels,'native_runtime_instruction_decoder':False}
(R/'src/dsky_verb_dispatch_table.inc').write_text('/* SPDX-License-Identifier: MIT; unchanged original passive CADR tables. */\n'+'\n'.join(rows)+'\n')
(R/'build/dsky-verb-dispatch-table-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
