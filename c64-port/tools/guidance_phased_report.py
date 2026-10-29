#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit independently verified native phase evidence without rerunning kernels."""
import hashlib,json,pathlib,re,collections
root=pathlib.Path(__file__).resolve().parents[1];build=root/'build'
expected={'unit':(48324,'244e21a959d27d8746b8106b8e727a1cdadc85e180a4a369ecba9eea0e5e0327'),'radial':(24658,'261dc5f2ddf7f9b01f5ee87fb373e98dafb864deea4abef67e6b16801ec0653a'),'projection':(24576,'776adf4ab370d6193c7b0d2cea3bdb50c802cb78d70599fb13f76c3ed1fccba7')}
report={'scope':'Native UNIT/radial/projection numerical overlay phases. C64 entry fixtures are exact original-source phase states; this does not prove a complete flight application or independently schedule inside original pairs.','source':'Unchanged Luminary099 assembly, pinned original hardware-DV reference engine','phases':{}}
for kind,(count,digest)in expected.items():
 data=(build/f'{kind}-phased-reference.snapshots').read_bytes()
 assert len(data.splitlines())==count and hashlib.sha256(data).hexdigest()==digest
 assert data==(build/f'{kind}-phased-native.snapshots').read_bytes()==(build/f'{kind}-phased-sanitize.snapshots').read_bytes()
 states=[list(map(int,r.split()))for r in data.decode().splitlines()]
 if kind!='radial':assert (build/f'{kind}-phased-reference.inputs').read_bytes()==(build/f'{kind}-phased-native.inputs').read_bytes()
 controls=collections.Counter()
 for line in (build/f'{kind}-phased-reference.trace').read_text().splitlines():
  if ' FB'not in line:continue
  nums,raw=line.split(' FB',1);key='stage/status '+nums+' FB'+raw
  # Remove individual input/history identifiers; keep stage/status and raw controls.
  ids=nums.split();stage=ids[2]if kind=='radial'else ids[1];status=ids[3] if kind=='radial' else ('0' if kind=='projection' else ids[2])
  controls[(stage,status,'FB'+raw)]+=1
 entry={'checkpoints':count,'complete_state_words':104 if kind=='unit'else 134,'sha256':digest,'sanitizers':'Entire matching corpus, ASAN and UBSAN','source_control_groups':[{'stage':int(k[0]),'status':int(k[1]),'raw_control':k[2],'count':v}for k,v in sorted(controls.items())],'c64':[]}
 for phase in range(9 if kind=='radial'else 3):
  tag=f'{kind}_phase{phase}';mail=(build/f'{tag}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,phase])
  mapping=(build/f'{kind}-phase{phase}.map').read_text();end=0
  for name in ['.text','.rodata','.data','.bss','.noinit']:
   values=re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(name)+r'$',mapping,re.M);assert len(values)==1
   end=max(end,int(values[0][0],16)+int(values[0][1],16))
  assert end<=0xc000
  stack={}
  for name,base,top in [('hardware',0x100,0x1f2),('software',0xc100,0xd000)]:
   raw=(build/f'{tag}-{name}-stack.bin').read_bytes();changed=[i for i,v in enumerate(raw)if v!=0xcd];extent=top-base-min(changed)if changed else 0;assert extent<len(raw);stack[name]=extent
  entry['c64'].append({'phase':phase,'complete_entry_exit_pairs':32,'mailbox':mail.hex(),'image_end':f'{end:04x}','stack_extent_bytes':stack})
 report['phases'][kind]=entry
(build/'guidance-phased-proof.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS: 97,558 full source/sanitizer checkpoints and 480 complete C64 phase entry/exit pairs; report build/guidance-phased-proof.json')
