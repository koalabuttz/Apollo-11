#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Extract original higher-priority case1137 without changing its parent proof."""
import pathlib,subprocess,hashlib,gzip,json
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/p66_bias_cases.c','tests/p66_vertical_start_cases.c','tests/p66_launch_cases.c','tests/p66_direct_launch_cases.c','tools/p66_direct_launch_inputs.c','-o','build/p66-direct-pending-inputs'],cwd=R,check=True)
def stream(kind):
 p=R/('build/p66-direct-launch-'+kind+'.snapshots')
 return p.read_bytes()if p.exists()else gzip.decompress((R/('evidence/p66-direct-launch-'+kind+'.snapshots.gz')).read_bytes())
a=stream('reference');assert a==stream('native')
assert hashlib.sha256(a).hexdigest()=='56bd1037bd7ac328b5433be6e258fc9a76260b70ff8e4def4d5184eea2b9f274'
n=1137;source=a.decode().splitlines()[n];row=list(map(int,source.split()));assert row[:2]==[n,0]
before=list(map(int,subprocess.check_output(['build/p66-direct-pending-inputs'],cwd=R,text=True).splitlines()[n].split()))[2:];after=row[2:]
assert len(before)==len(after)==2050
text=['/* SPDX-License-Identifier: MIT',' * Complete unchanged original VRTSTART1137 source checkpoint.',' * Parent full source SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef P66_DIRECT_PENDING_VECTORS_H','#define P66_DIRECT_PENDING_VECTORS_H','static const uint16_t p66_direct_pending_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,(v,b)in enumerate(zip(after,before))if v!=b];text+=['};','#endif',''];(R/'tests/p66_direct_pending_vectors.h').write_text('\n'.join(text))
(R/'build/p66-direct-pending-reference.snapshots.gz').write_bytes(gzip.compress((source+'\n').encode(),mtime=0))
(R/'build/p66-direct-pending-source-case.json').write_text(json.dumps({'parent_original_stream_sha256':hashlib.sha256(a).hexdigest(),'original_case':n,'input_words_and_io':before,'original_words_and_io':after,'pending_priority':before[0o167+before[0o67]],'active_priority':before[0o167]},indent=2)+'\n')
print('Frozen original1137 complete2050-field checkpoint, pendingpriority036110 >active035531')
