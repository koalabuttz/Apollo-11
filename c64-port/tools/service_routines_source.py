#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Assemble unchanged original service files and audit all emitted words."""
from pathlib import Path
import hashlib,json,shutil,subprocess,re
from fixed_data_generate import listing_records,source_line,symbols,rope_word,fixed_address
R=Path(__file__).resolve().parents[1]
KEYS=('UPENT2','DOWNENT2','UPFLAG','DOWNFLAG','DEBIT','COMFLAG','DELAYJOB','DELLOOP','OK2DELAY','TCGETCAD','WAKER','TCSLEEP','WAKECAD','GENTRAN','B5OFF','TRFAILOF','TRFAILON','REQ','DELAYLOC','DELAYNUM','DLY2','BAILOUT','BAILOUT1','JOBSLEEP','JOBWAKE','TASKOVER','ENDOFJOB','Q+1','Q+2')
def main():
 audits=[];sets=[];fixed_header='/* SPDX-License-Identifier: MIT; passive original input words only. */\n#ifndef SERVICE_ROUTINES_FIXED_INPUTS_H\n#define SERVICE_ROUTINES_FIXED_INPUTS_H\n#include <stdint.h>\n'
 for mission in ('Luminary099','Comanche055'):
  base=R/'build/standard-lead-source'/mission;work=R/'build/service-routines-source'/mission;work.mkdir(parents=True,exist_ok=True)
  for p in base.glob('*.agc'):shutil.copyfile(p,work/p.name)
  p=R.parent/mission/'SERVICE_ROUTINES.agc';shutil.copyfile(p,work/p.name)
  with (work/'assembly.lst').open('w') as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
  rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(R/'build/fixed-data'/mission/'MAIN.agc.bin').read_bytes()
  path=mission+'/'+p.name;text=p.read_text();records=listing_records(work/'assembly.lst',{p.name:path},{path:text},rope)
  assert {r['line'] for r in records}=={i for i,line in enumerate(text.splitlines(),1) if source_line(line) is not None}
  sy=symbols((work/'assembly.lst').read_text());selected={k:sy[k] for k in KEYS if k in sy}
  values={k:(int(v.split(',')[-1],8)) for k,v in selected.items()}
  values['delay_count']=rope_word(rope,fixed_address(sy['DELAYNUM']))
  for name in ('TCSLEEP','WAKECAD'):values[name+'_word']=rope_word(rope,fixed_address(sy[name]))
  sets.append(values)
  m=len(sets)-1;flag=[(i,rope_word(rope,i)) for i in range(0o4000,0o7775) if rope_word(rope,i)<210][:32];assert len(flag)==32
  fixed_header+='static const uint16_t service_flag_fixed_'+str(m)+'[32][2]={'+','.join('{'+str(i)+','+str(v)+'}' for i,v in flag)+'};\n'
  words=[rope_word(rope,0o4000+i) for i in range(512)]
  fixed_header+='static const uint16_t service_data_fixed_'+str(m)+'[512]={'+','.join(map(str,words))+'};\n'
  audits.append(dict(source_path=path,source_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),fixture_source_sha256=hashlib.sha256((work/p.name).read_bytes()).hexdigest(),source_edits=0,rope_sha256=hashlib.sha256(rope).hexdigest(),source_words=sum(len(r['words']) for r in records),source_line_word_map=records,symbols=selected))
 names=sorted(set().union(*sets))
 # Identifiers are normalized here only; immutable numeric bindings remain exact.
 ident=lambda n:n.lower().replace('+','plus')
 header='/* SPDX-License-Identifier: MIT; immutable original source bindings. */\n#ifndef SERVICE_ROUTINES_SOURCE_H\n#define SERVICE_ROUTINES_SOURCE_H\n#include <stdint.h>\ntypedef struct {\n'+''.join(' uint16_t '+ident(n)+';\n' for n in names)+'} service_routines_bindings;\nstatic const service_routines_bindings service_routines_source[2]={\n'
 header+=',\n'.join(' {'+','.join('0'+format(v.get(n,0),'o') for n in names)+'}' for v in sets)+'\n};\n#endif\n'
 (R/'include/service_routines_source.h').write_text(header)
 fixed_header+='#endif\n'
 assert (R/'tests/service_routines_fixed_inputs.h').read_text()==fixed_header,'immutable fixed input words differ from original source rope'
 for audit in audits:
  mission=audit['source_path'].split('/')[0];calls=[]
  for source in (R.parent/mission).glob('*.agc'):
   lines=source.read_text().splitlines()
   for i,line in enumerate(lines):
    if re.search(r'\b(?:TC|TCF)\s+GENTRAN\b',line.split('#')[0]):calls.append(dict(path=mission+'/'+source.name,line=i+1,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),source_context=[dict(line=j+1,text=lines[j]) for j in range(max(0,i-6),min(len(lines),i+4))]))
  audit['original_gentran_callers']=calls
 (R/'build/service-routines-source/audit.json').write_text(json.dumps(audits,indent=2)+'\n')
 print('PASS byte-original both service files, unchanged original ropes',[(a['source_path'],a['source_words']) for a in audits])
if __name__=='__main__':main()
