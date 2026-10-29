#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Reproduce the frozen ordinary BORT return collision without changing it."""
import pathlib, subprocess, gzip, struct, hashlib, json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';prefix='cm-alarm-return-collision'
source='''/* SPDX-License-Identifier: MIT; generated from retained audit tool. */
#include <stdlib.h>
#include "cm_alarm_complete_cases.h"
#include "cm_alarm_complete_dump.h"
int main(int argc,char **argv){uint32_t n;unsigned i;const uint32_t ids[2]={23683,23687};for(i=0;i<2u;++i){apollo_overlay_state s;cm_native_io io;enum exec_status status;n=ids[i];cm_alarm_complete_input(&s,&io,n);status=argc>1?cm_alarm_complete(&s,&io,CM_RECORD_BORTENT,0):cm_alarm_image(&s,&io,CM_RECORD_BORTENT,0);cm_alarm_complete_dump(n,status,&s,&io);}return 0;}
'''
(B/(prefix+'.c')).write_text(source)
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-Itools',str(B/(prefix+'.c')),'src/cm_alarm_complete.c','src/cm_alarm_image.c','src/cm_alarm_record.c','src/cm_native_image.c','src/agc_word.c','tests/cm_alarm_complete_cases.c','tests/cm_alarm_image_cases.c','tests/cm_alarm_record_cases.c','tests/cm_admission_complete_cases.c','-o',str(B/prefix)],cwd=R,check=True)
old=subprocess.check_output([str(B/prefix)],cwd=R);new=subprocess.check_output([str(B/prefix),'corrected'],cwd=R);expected=b''
with gzip.open(B/'cm-alarm-complete-reference.snapshots.bin.gz','rb') as f:
 for n in range(32240):
  a=f.read(4130)
  if n in [23683,23687]:expected+=a
assert new==expected and len(expected)==8260
x=struct.unpack('<4130H',old);y=struct.unpack('<4130H',expected);diff=[i for i,(a,b) in enumerate(zip(x,y)) if a!=b]
assert diff==[2059],diff
assert x[2059]==0 and y[2059]==1
for role,data in [('frozen-helper',old),('corrected-helper',new),('original-source',expected)]: (B/(prefix+'-'+role+'.bin')).write_bytes(data)
report={'source_cases':[23683,23687],'sourceQ_octal':'05720','true_returnPC_octal':'05721','true_returnA_octal':'05720','true_returnQ_octal':'05715','source_virtualIRQ_result':1,'old_difference':'Only first case incomingIRQ0: frozen cm_alarm_image restores0 because PC equals POODOO entry; source has executed RELINT and normally returned there. IncomingIRQ1 case matches. No frozen helper or artifact was changed.','corrected_condition':'A true TS FAILREG+2 overflow to POODOO has rawA=+1 or -1; normal return has rawA=ordinary callerQ>=0100. New cm_alarm_complete retainsIRQ1 on ordinary collision.','actual_target_failure':False,'historical_initial_metadata':{'path':'evidence/cm-alarm-return-collision-initial-metadata.json','sha256':hashlib.sha256((R/'evidence/cm-alarm-return-collision-initial-metadata.json').read_bytes()).hexdigest()},'earlier_ALARM2_draft_failure':{'case':15050,'native_stream_sha256':'d2f0963c582abe37aaaac3d883e450fe6f1a0dd18f9371b7a8947a5e9e3bc166','original_stream_sha256':'78c839a668ba4d764ac3c60a0152103554c641d234d48c7132adbc29fe61963e','preserved_failing_row_correction':'same normal-return collision; new ALARM2 calls unchanged record kernel and distinguishes source TS overflow by A, preserving IRQ only there'},'source_reference_proof':{'path':'build/cm-alarm-complete-proof.json','sha256':hashlib.sha256((B/'cm-alarm-complete-proof.json').read_bytes()).hexdigest()},'files':{path:hashlib.sha256((R/path).read_bytes()).hexdigest() for path in ['src/cm_alarm_image.c','src/cm_alarm_record.c','src/cm_alarm_complete.c','evidence/cm-alarm2-image-producer-first-target-failure/src/cm_alarm2_image.c']},'snapshots':{role:hashlib.sha256(data).hexdigest() for role,data in [('frozen-helper',old),('corrected-helper',new),('original-source',expected)]}}
(B/(prefix+'.json')).write_text(json.dumps(report,indent=2)+'\n');print('PASS: unchanged frozen BORT collision reproduced; source-qualified additive helper exact; only originalIRQ0 case differed')
