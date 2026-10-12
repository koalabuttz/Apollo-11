#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independent assembly comparison for a Comanche055 file in the CM flight
fixture (Lane G): every word yaYUL emits for the unchanged source file, read
from the assembly listing, against the same address in the original
Comanche055 rope bytes. It reads nothing from the native port, so it stands
apart from the source-to-native map. Writes
evidence/<id>/<id>-assembly-comparison.json.

usage: cm_flight_assembly_comparison.py ID AGC_FILE"""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];FIX=R/'deps/cm-flight-fixture'
ORIGINAL_ROPE='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
def compare(mid,agc):
 if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
 rope=(FIX/'MAIN.agc.bin').read_bytes();assert hashlib.sha256(rope).hexdigest()==ORIGINAL_ROPE,'fixture rope is not the original Comanche055 rope'
 W=struct.unpack('>%dH'%(len(rope)//2),rope)
 def word(bank,addr):
  off={2:0,3:1}.get(bank,bank);return W[off*1024+addr-(0o2000 if bank not in(2,3) else (0o4000 if bank==2 else 0o6000))]>>1
 src=R.parent/'Comanche055'/agc
 listing=(FIX/'assembly.lst').read_text().split('$'+agc,1)[1]
 rows=[]
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: \$',line):break
  m=re.match(r'^(\d+),(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?: ([0-7]{5}))?\s',line)
  if not m:continue
  bank,addr=int(m[3],8),int(m[4],8)
  for k,raw in enumerate([m[5],m[6]] if m[6] else [m[5]]):
   rows.append({'source_line':int(m[2]),'bank':f'{bank:02o}','address':f'{addr+k:04o}','listing':raw,'rope':f'{word(bank,addr+k):05o}'})
 bad=[r for r in rows if r['listing']!=r['rope']]
 assert rows and not bad,bad[:5]
 x={'source_file':'Comanche055/'+agc,'original_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'rope_sha256':ORIGINAL_ROPE,
  'method':'yaYUL listing of the unchanged file (deps/cm-flight-fixture/assembly.lst) against the original Comanche055 rope, word by word; no native code read',
  'emitted_words':len(rows),'mismatches':0,'words':rows}
 p=R/f'evidence/{mid}/{mid}-assembly-comparison.json';p.write_text(json.dumps(x,indent=1)+'\n')
 return len(rows)
if __name__=='__main__':
 if len(sys.argv)!=3:raise SystemExit(__doc__)
 n=compare(sys.argv[1],sys.argv[2])
 print(f'PASS {n} emitted {sys.argv[2]} words equal the original Comanche055 rope')
