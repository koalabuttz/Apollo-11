#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Clean streaming original-image checks for recurrence and prior compatibility."""
import argparse,gzip,hashlib,itertools,lzma,os,subprocess
from pathlib import Path
from p66_application_port_host import inventories
ROOT=Path(__file__).resolve().parents[1]
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--sanitize',action='store_true');args=parser.parse_args()
 _,sources,inputs,defines=inventories()[2]
 extra=['src/p66_length_return_control.c','src/p66_application_recurrence.c','src/lm_clock.c','src/lm_idle_start.c','src/lm_taskover_resume.c','src/lm_waitlist_dispatch.c','src/lm_waitlist_irq_dispatch.c','src/p66_rodtask_image.c']
 cases=[('p66-recurrence-live',sources+extra+inputs+['tests/p66_recurrence_live_cases.c'],'tools/p66_recurrence_live_snapshot.c','evidence/p66-recurrence-live-source.snapshots.xz',2460,'d1aa5f4de9e02bf126731818227c4bc7592e62bddbbd66a425cc56346683f186'),('p66-recurrence-prior',sources+inputs+['src/p66_length_return_control.c'],'tools/p66_length_return_application_snapshot.c','evidence/p66-application-live-source.snapshots.xz',1226,'9ea8e8bc75b90a91ac5185b2546cf93ca47dc786b691633527df6959819e22dc'),('lm-waitlist-irq-recurrence',['src/agc_word.c','src/waitlist.c','src/lm_waitlist_dispatch.c','src/lm_waitlist_irq_dispatch.c','tests/lm_taskover_resume_cases.c','tests/lm_waitlist_dispatch_cases.c'],'tools/lm_waitlist_irq_dispatch_snapshot.c','evidence/lm-waitlist-irq-dispatch-reference.snapshots.xz',8192,'28399982b6a4a2d694a1ee160e934c4f92afcfe49514864ff820fea432d492d3'),('p66-rodtask-image',['src/agc_word.c','src/executive.c','src/p66_native.c','src/p66_startup.c','src/p66_rodtask_image.c','tests/p66_basic_task_cases.c'],'tools/p66_rodtask_image_snapshot.c','evidence/p66-basic-task-reference.snapshots.gz',2048,None)]
 flags=['gcc','-std=c99','-O1'if args.sanitize else'-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']+defines
 if args.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
 for name,files,snapshot,archive,count,digest in cases:
  binary=ROOT/('build/'+name+'-port'+('-sanitize'if args.sanitize else''));subprocess.run(flags+(['-DP66_NATIVE_DISPATCH_ONLY']if name=='p66-rodtask-image'else[])+list(dict.fromkeys(files))+[snapshot,'-o',str(binary)],cwd=ROOT,check=True)
  h=hashlib.sha256();opener=gzip.open if archive.endswith('.gz')else lzma.open
  with opener(ROOT/archive,'rb')as expected:
   process=subprocess.Popen([str(binary)],cwd=ROOT,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
   try:
    for n,(source,native)in enumerate(itertools.zip_longest(expected,process.stdout),1):
     assert source==native,(name,n,source[:60]if source else None,native[:60]if native else None);h.update(source)
    assert process.wait()==0
   finally:
    if process.poll()is None:process.kill();process.wait()
    process.stdout.close()
  assert n==count and (digest is None or h.hexdigest()==digest)
  print('PASS:',name,n,'complete original states, SHA256',h.hexdigest(),flush=True)
if __name__=='__main__':main()
