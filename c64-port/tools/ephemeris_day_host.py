#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare compiled CSTODAY divide arithmetic with retained whole original source states."""
import argparse,hashlib,json,lzma,os,pathlib,shlex,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--sanitize',action='store_true');p.add_argument('--generate',action='store_true');a=p.parse_args()
core=['src/ephemeris_day.c','src/guidance_dp_divide.c','src/agc_word.c','src/agc_multiword.c','tests/ephemeris_day_cases.c']
flags=shlex.split(a.cc)+['-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
binary='build/ephemeris-day-'+('sanitize'if a.sanitize else'native')
subprocess.run(flags+['tools/ephemeris_day_snapshot.c',*core,'-o',binary],cwd=R,check=True)
if a.generate:
 assert not a.sanitize
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/ephemeris_day_reference.c','tests/ephemeris_day_cases.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/ephemeris-day-reference'],cwd=R,check=True)
 records={}
 for mission in ['LM','CM']:
  archive=R/('build/ephemeris-day-'+mission+'-reference.snapshots.bin.xz');h=hashlib.sha256();count=0
  process=subprocess.Popen(['build/ephemeris-day-reference','deps/'+mission.lower()+'-rtb-fixture/MAIN.agc.bin',mission,'65536'],cwd=R,stdout=subprocess.PIPE)
  with lzma.open(archive,'wb',preset=3) as out:
   while block:=process.stdout.read(1048576):out.write(block);h.update(block);count+=len(block)
  assert process.wait()==0 and count==65536*4106
  records[mission]={'archive':'evidence/'+archive.name,'decoded_sha256':h.hexdigest(),'decoded_bytes':count,'states':65536}
 (R/'build/ephemeris-day-source-streams.json').write_text(json.dumps(records,indent=2)+'\n')
records=json.loads((R/('build/ephemeris-day-source-streams.json'if a.generate else'evidence/ephemeris-day-source-streams.json')).read_text())
for mission,state in records.items():
 archive=('build/'+pathlib.Path(state['archive']).name)if a.generate else state['archive']
 subprocess.run(['python3','tools/qualified_stream_host.py','--binary',binary,'--archive',archive,'--sha256',state['decoded_sha256'],'--bytes',str(state['decoded_bytes']),'--arg',mission,'--arg','65536'],cwd=R,check=True)
print('PASS131072 complete original CSTODAY divide states'+(' full ASAN/UBSAN'if a.sanitize else' strict native'),flush=True)
