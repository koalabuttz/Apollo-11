#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import subprocess,lzma,hashlib,json,struct
p=Path(__file__).resolve().parents[1]
core=['src/guidance_product.c','src/sband_trig.c', 'src/guidance_trig.c', 'src/sband_vector.c', 'src/rtb_unit.c', 'src/rtb_vector.c', 'src/rtb_angles.c', 'src/guidance_unit_phased.c', 'src/guidance_length_phased.c', 'src/guidance_norm.c']+['src/cm_ephemeris_solar.c', 'src/cm_ephemeris_time.c', 'src/ephemeris_time_scale.c', 'src/ephemeris_shift.c', 'src/ephemeris_add.c', 'src/interpreted_sign.c', 'src/sband_scalar.c', 'src/sband_vector_linear.c', 'src/sband_shift.c', 'src/guidance_shift.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/cm_ephemeris_time_cases.c']
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/cm_ephemeris_solar_snapshot.c',*core,'-o','build/cm-ephemeris-solar-native'],cwd=p,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/cm_ephemeris_solar_reference.c','tests/cm_ephemeris_time_cases.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/cm-ephemeris-solar-reference'],cwd=p,check=True)
a=subprocess.Popen(['build/cm-ephemeris-solar-reference','deps/cm-rtb-fixture/MAIN.agc.bin','CM','65536'],cwd=p,stdout=subprocess.PIPE)
b=subprocess.Popen(['build/cm-ephemeris-solar-native','CM','65536'],cwd=p,stdout=subprocess.PIPE)
h=hashlib.sha256();size=0
with lzma.open(p/'build/cm-ephemeris-solar-reference.snapshots.bin.xz','wb',preset=3)as out:
 for n in range(65536):
  x=a.stdout.read(4106);y=b.stdout.read(4106)
  if x!=y or len(x)!=4106:
   xa=struct.unpack('<2053H',x);ya=struct.unpack('<2053H',y);print('FAIL',n,[(f'{i-5:04o}',f'{v:06o}',f'{ya[i]:06o}')for i,v in enumerate(xa)if v!=ya[i]][:25],flush=True);a.kill();b.kill();raise SystemExit(1)
  out.write(x);h.update(x);size+=len(x)
 assert not a.stdout.read(1)and not b.stdout.read(1)and a.wait()==0 and b.wait()==0
state={'CM':{'archive':'evidence/cm-ephemeris-solar-reference.snapshots.bin.xz','decoded_sha256':h.hexdigest(),'decoded_bytes':size,'states':65536}}
(p/'build/cm-ephemeris-solar-source-streams.json').write_text(json.dumps(state,indent=2)+'\n')
print('PASS65536 complete source native2053word states',h.hexdigest(),flush=True)
