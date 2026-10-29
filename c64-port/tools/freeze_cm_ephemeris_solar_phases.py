#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib,json,lzma,struct,subprocess
from frozen_words import encode_rows,c_header
R=Path(__file__).resolve().parents[1];ids=list(range(32))+[128+i*2051 for i in range(32)]
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/cm_ephemeris_solar_phase_reference.c','tests/cm_ephemeris_time_cases.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/cm-ephemeris-solar-phase-reference'],cwd=R,check=True)
raw=subprocess.check_output(['build/cm-ephemeris-solar-phase-reference','deps/cm-rtb-fixture/MAIN.agc.bin',*map(str,ids)],cwd=R);assert len(raw)%8206==0
initial=subprocess.check_output(['build/cm-ephemeris-solar-inputs','CM',*map(str,ids)],cwd=R);original={n:struct.unpack_from('<2050H',initial,idx*4100)for idx,n in enumerate(ids)}
rows=[[]for _ in range(5)];maxima=[[0,0]for _ in range(5)]
for fields in struct.iter_unpack('<4103H',raw):
 n=fields[0]+(fields[1]<<16);stage=fields[2];a=fields[7:2055];b=fields[2055:];inp=[(i,v)for i,(u,v)in enumerate(zip(original[n][:2048],a))if u!=v];out=[(i,v)for i,(u,v)in enumerate(zip(a,b))if u!=v];maxima[stage][0]=max(maxima[stage][0],len(inp));maxima[stage][1]=max(maxima[stage][1],len(out));rows[stage].append([n,stage,fields[3],fields[4],fields[5],len(inp),len(out),inp,out])
header='/* SPDX-License-Identifier: MIT; exact independently executed original checkpoint states. */\n#ifndef CM_EPHEMERIS_SOLAR_PHASE_VECTORS_H\n#define CM_EPHEMERIS_SOLAR_PHASE_VECTORS_H\n';reports=[]
for stage,all_data in enumerate(rows):
 for cohort in range(4 if stage==3 else 2):
  width=16 if stage==3 else 32;data=all_data[cohort*width:(cohort+1)*width];packed=[];ni,no=maxima[stage]
  for n,st,irq,super,status,ic,oc,inp,out in data:packed.append([n,st,irq,super,status,ic,oc,*sum(([i,v]for i,v in inp),[]),*([0]*((ni-ic)*2)),*sum(([i,v]for i,v in out),[]),*([0]*((no-oc)*2))])
  enc=encode_rows(packed);header+=f'#if CM_EPHEMERIS_SOLAR_STAGE == {stage} && CM_EPHEMERIS_SOLAR_COHORT == {cohort}\n#define CM_EPHEMERIS_SOLAR_MAX_INPUT {ni}u\n#define CM_EPHEMERIS_SOLAR_MAX_OUTPUT {no}u\n'+c_header(enc,'cm_ephemeris_solar_phase')+'#endif\n';reports.append(dict(stage=stage,cohort=cohort,states=len(data),max_input=ni,max_output=no,frozen_bytes=enc.storage_bytes))
header+='#endif\n';(R/'tests/cm_ephemeris_solar_phase_vectors.h').write_text(header)
with lzma.open(R/'build/cm-ephemeris-solar-phases-source.snapshots.bin.xz','wb',preset=6)as f:f.write(raw)
(R/'build/cm-ephemeris-solar-phases-target-results.json').write_text(json.dumps(dict(source_bytes=len(raw),source_sha256=hashlib.sha256(raw).hexdigest(),case_ids=ids,stages=reports),indent=2)+'\n');print('PASS source-only phase checkpoint freeze',reports,flush=True)
