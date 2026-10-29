#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Read-only check of immutable PULSEIMU frontier bank metadata erratum."""
import pathlib,lzma,struct,hashlib,json
r=pathlib.Path(__file__).resolve().parents[1];audit=json.loads((r/'evidence/rtb-vector-bank-erratum.json').read_text())
for mission,x in audit['corpora'].items():
 count=0;h=hashlib.sha256()
 with lzma.open(r/x['path'],'rb')as f:
  while raw:=f.read(4104):
   assert len(raw)==4104;h.update(raw);w=struct.unpack('<2052H',raw);m=w[4:]
   assert m[4]==0o16000 and m[5]==int(x['PC'],8)and m[6]==(m[4]|(m[3]>>8));count+=1
 assert count==x['records']and h.hexdigest()==x['decoded_sha256']
for p,h in audit['unchanged_byte_identities'].items():assert hashlib.sha256((r/p).read_bytes()).hexdigest()==h
print('PASS: all131072 immutable complete frontier states select bank07/FB016000; prior proof bytes unchanged')
