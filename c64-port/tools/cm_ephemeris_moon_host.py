#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare compiled LUNPOS polynomial and return arithmetic with retained whole original source states."""
import argparse,hashlib,json,lzma,os,pathlib,shlex,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--sanitize',action='store_true');p.add_argument('--generate',action='store_true');a=p.parse_args()
core=['src/cm_ephemeris_moon.c', 'src/cm_ephemeris_time.c', 'src/ephemeris_time_scale.c', 'src/ephemeris_shift.c', 'src/ephemeris_add.c', 'src/interpreted_sign.c', 'src/sband_vector_linear.c', 'src/sband_shift.c', 'src/guidance_shift.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/cm_ephemeris_time_cases.c']
flags=shlex.split(a.cc)+['-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
binary='build/cm-ephemeris-moon-'+('sanitize'if a.sanitize else'native')
subprocess.run(flags+['tools/cm_ephemeris_moon_snapshot.c',*core,'-o',binary],cwd=R,check=True)
assert not a.generate, 'Original source generation uses the independently compiled reference and streaming audit.'
records=json.loads((R/'evidence/cm-ephemeris-moon-source-streams.json').read_text())
for mission,state in records.items():
 archive=('build/'+pathlib.Path(state['archive']).name)if a.generate else state['archive']
 subprocess.run(['python3','tools/qualified_stream_host.py','--binary',binary,'--archive',archive,'--sha256',state['decoded_sha256'],'--bytes',str(state['decoded_bytes']),'--arg',mission,'--arg','65536'],cwd=R,check=True)
print('PASS65536 complete original LUNPOS polynomial and return states'+(' full ASAN/UBSAN'if a.sanitize else' strict native'),flush=True)
