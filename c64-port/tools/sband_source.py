#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fresh original S-band source overlays and independent line/word maps."""
from pathlib import Path
import hashlib,json,shutil,subprocess
from fixed_data_generate import listing_records,source_line,symbols
R=Path(__file__).resolve().parents[1]
def main():
 audits=[]
 for mission,name in [('Luminary099','S-BAND_ANTENNA_FOR_LM.agc'),('Comanche055','S-BAND_ANTENNA_FOR_CM.agc')]:
  work=R/'build/sband-source'/mission;work.mkdir(parents=True,exist_ok=True)
  for p in (R/'build/service-routines-source'/mission).glob('*.agc'):shutil.copyfile(p,work/p.name)
  original=R.parent/mission/name;shutil.copyfile(original,work/name)
  with (work/'assembly.lst').open('w') as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
  rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/'build/fixed-data'/mission/'MAIN.agc.bin').read_bytes()
  path=mission+'/'+name;text=original.read_text();records=listing_records(work/'assembly.lst',{name:path},{path:text},rope)
  assert {r['line'] for r in records}=={i for i,line in enumerate(text.splitlines(),1) if source_line(line) is not None}
  sy=symbols((work/'assembly.lst').read_text());wanted=['SBANDANT','EISOI','CONV3','CONV4','CONV5','COVCNV','NOADJUST','SBANDEX','V06N51','PITCHANG','YAWANG','RLM','RCM','UR','URP','R','1OVSQRT2','VXV','VPROJ','ENDVXV','PREDOT','DOTSUB','DVXSC','VMODE','DANZIG','DMPSUB','VROUND','OVERFLWZ','OVERFLWY','OVERFLOW','SETOVF','SETOVF2','UNIT','ARCSIN','ARCCOS','SINE','COSINE']
  audits.append(dict(source_path=path,source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),source_edits=0,rope_sha256=hashlib.sha256(rope).hexdigest(),source_words=sum(len(r['words']) for r in records),source_line_word_map=records,symbols={k:sy[k] for k in wanted if k in sy},status='partial_pending_native_composition',instruction_decoder=False))
 (R/'build/sband-source/audit.json').write_text(json.dumps(audits,indent=2)+'\n')
 for a in audits:print(a['source_path'],a['source_words'],'original words',a['symbols'])
if __name__=='__main__':main()
