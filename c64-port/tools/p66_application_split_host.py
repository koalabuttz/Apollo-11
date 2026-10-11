#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check additive prepare/body/finish composition against retained raw source."""
import argparse,hashlib,itertools,lzma,os,pathlib,subprocess
from p66_application_port_host import inventories
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
 inv=inventories();_,job,jinputs,jdefs=inv[1];_,live,linputs,ldefs=inv[2]
 extra=['src/p66_length_return_control.c','src/p66_application_recurrence.c','src/lm_clock.c','src/lm_idle_start.c','src/lm_taskover_resume.c','src/lm_waitlist_dispatch.c','src/lm_waitlist_irq_dispatch.c','src/p66_rodtask_image.c']
 cases=[('job',job+jinputs,jdefs,'tools/p66_application_job_snapshot.c','p66-application-job-source.snapshots.xz',13312,'bab1ff892632cd9c90d139229a9af041913c697301567a7ee1ac7dc6690bb22e'),('live',live+linputs+['src/p66_length_return_control.c'],ldefs,'tools/p66_length_return_application_snapshot.c','p66-application-live-source.snapshots.xz',1226,'9ea8e8bc75b90a91ac5185b2546cf93ca47dc786b691633527df6959819e22dc'),('recurrence',live+extra+linputs+['tests/p66_recurrence_live_cases.c'],ldefs,'tools/p66_recurrence_live_snapshot.c','p66-recurrence-live-source.snapshots.xz',2460,'d1aa5f4de9e02bf126731818227c4bc7592e62bddbbd66a425cc56346683f186')]
 flags=['cc','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']
 if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
 guard_binary=ROOT/('build/test-p66-application-split'+('-sanitize'if a.sanitize else''))
 subprocess.run(flags+list(dict.fromkeys(job+jinputs+['src/p66_application_split.c']))+jdefs+['tests/test_p66_application_split.c','-o',str(guard_binary)],cwd=ROOT,check=True)
 subprocess.run([str(guard_binary)],cwd=ROOT,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 pending_binary=ROOT/('build/test-p66-application-split-pending'+('-sanitize'if a.sanitize else''))
 pending_inputs=['tests/p66_vertical_start_cases.c','tests/p66_launch_cases.c','tests/p66_direct_launch_cases.c']
 subprocess.run(flags+list(dict.fromkeys(job+jinputs+pending_inputs+['src/p66_application_split.c']))+jdefs+['tests/test_p66_application_split_pending.c','-o',str(pending_binary)],cwd=ROOT,check=True)
 subprocess.run([str(pending_binary)],cwd=ROOT,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 for name,files,defs,snapshot,archive,count,digest in cases:
  translated=ROOT/('build/p66-application-split-'+name+'-snapshot.c')
  text=(ROOT/snapshot).read_text().replace('p66_application_init(','p66_application_split_init(')
  translated.write_text('#include "p66_application_split.h"\n'+text)
  binary=ROOT/('build/p66-application-split-'+name+('-sanitize'if a.sanitize else''))
  subprocess.run(flags+list(dict.fromkeys(files+['src/p66_application_split.c']))+defs+[str(translated),'-o',str(binary)],cwd=ROOT,check=True)
  h=hashlib.sha256()
  with lzma.open(ROOT/'evidence'/archive,'rb')as expected:
   process=subprocess.Popen([str(binary)],cwd=ROOT,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
   try:
    for n,(source,native)in enumerate(itertools.zip_longest(expected,process.stdout),1):
     if source!=native:raise AssertionError((name,n,source[:60]if source else None,native[:60]if native else None))
     h.update(source)
    assert process.wait()==0
   finally:
    if process.poll()is None:process.kill();process.wait()
    process.stdout.close()
  assert n==count and h.hexdigest()==digest
  print('PASS:',name,n,'complete raw original states, SHA256',h.hexdigest(),flush=True)
if __name__=='__main__':main()
