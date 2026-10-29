#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Offline exact original words and genuine caller inventory, never runtime decoding."""
import hashlib,json,pathlib
R=pathlib.Path(__file__).resolve().parents[1];E=R/'evidence'
rope=(E/'fixed-data-cm-original.rope.bin').read_bytes()
assert hashlib.sha256(rope).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
def fixed(address):return int.from_bytes(rope[(address-0o4000)*2:(address-0o4000)*2+2],'big')>>1
expected={0o5227:0o56001,0o5230:0o00006,0o5231:0o01007,0o5232:0o54006,0o5233:0o00001,0o4622:0o00006,0o4623:0o34626,0o4624:0o15227,0o4625:0o02653,0o4626:0o66107}
assert all(fixed(a)==word for a,word in expected.items())
callers=[]
for p in sorted((R.parent/'Comanche055').glob('*.agc')):
 for n,line in enumerate(p.read_text().splitlines(),1):
  code=line.split('#')[0]
  if 'SUPDXCHZ' in code and code.split() and code.split()[0] in ['TC','TCF']:
   callers.append({'file':p.name,'original_line':n,'source':line,'source_sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
assert len(callers)==5
report={'original_rope_sha256':hashlib.sha256(rope).hexdigest(),'original_executive_sha256':hashlib.sha256((R.parent/'Comanche055/EXECUTIVE.agc').read_bytes()).hexdigest(),'entries':['SUPDXCHZ05227','SUPDXCHZ+105230','actual GOTOV5604622'],'exact_original_words':[{'fixed_address_octal':f'{a:05o}','word_octal':f'{w:05o}'} for a,w in expected.items()],'actual_callers':callers,'source_data_tuple':{'VB56CADR':['02653','66107'],'source_target':'TRACKTRM'},'scope':'exact source-word/data/caller provenance; native executable proof separately retained. Other containing callers and TRACKTRM are not certified by this inventory. No runtime decoder.'}
target=E/'cm-super-dispatch-source-audit.json';data=(json.dumps(report,indent=2)+'\n').encode();assert not target.exists() or target.read_bytes()==data;target.write_bytes(data)
print('PASS:10 exact original words,5 genuine public callers; original source unmodified')
