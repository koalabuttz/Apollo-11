#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Recheck historical source corpora without retaining duplicate raw streams."""
import hashlib,lzma,os,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
values=dict(line.split(' = ',1) for line in (R/'tools/display_job.mk').read_text().splitlines() if ' = ' in line)
flags=values['DJOB_FLAGS'].split()+['-DEXEC_NATIVE_RESIDENT_SERVICES']
core=values['DJOB_CORE'].split()+['src/native_overlay_dispatch.c']
for name,extra in [('display-job',[]),('display-job-lifecycle',['-DDISPLAY_JOB_LIFECYCLE','tests/display_job_lifecycle_cases.c']),('display-job-final',['-DDISPLAY_JOB_FINAL','tests/display_job_final_cases.c'])]:
 for sanitize in [False,True]:
  exe='build/'+name+'-scheduler-exchange-'+('sanitize' if sanitize else 'strict')
  opts=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'] if sanitize else ['-O2']
  subprocess.run(['cc']+opts+flags+extra+['tools/display_job_scheduler_snapshot.c']+core+['-o',exe],cwd=R,check=True)
  proc=subprocess.Popen([exe],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
  h=hashlib.sha256();count=0
  with lzma.open(R/('evidence/'+name+'-reference.snapshots.xz'),'rb') as source:
   for expected in source:
    actual=proc.stdout.readline();assert actual==expected,(name,sanitize,count)
    h.update(actual);count+=1
  assert not proc.stdout.read(1) and proc.wait()==0
  print('PASS',name,'SAN' if sanitize else 'strict',count,'complete original states; SHA256',h.hexdigest(),flush=True)
