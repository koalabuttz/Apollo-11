#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,hashlib,os,subprocess
from pathlib import Path
from p66_application_port_host import inventories
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args();_,sources,inputs,defines=inventories()[2];extra=['src/p66_length_return_control.c','src/p66_application_recurrence.c','src/lm_clock.c','src/lm_idle_start.c','src/lm_taskover_resume.c','src/lm_waitlist_dispatch.c','src/lm_waitlist_irq_dispatch.c','src/p66_rodtask_image.c'];inputs+=['tests/p66_recurrence_live_cases.c'];flags=['gcc','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']+defines
if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
name='p66-recurrence-live-sanitize'if a.sanitize else'p66-recurrence-live-native';subprocess.run(flags+list(dict.fromkeys(sources+extra+inputs))+['tools/p66_recurrence_live_snapshot.c','-o','build/'+name],cwd=root,check=True)
def capture(command,stem):
 with (root/f'build/{stem}.snapshots').open('w')as out,(root/f'build/{stem}.trace').open('w')as err:subprocess.run(command,cwd=root,stdout=out,stderr=err,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if not a.sanitize:
 engine=root/'deps/virtualagc/yaAGC';subprocess.run(flags+['-DP66_NATIVE_DISPATCH_ONLY','-I'+str(engine),'tools/p66_recurrence_live_reference.c','tests/scenarios.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c']+inputs+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-recurrence-live-reference'],cwd=root,check=True);capture(['build/p66-recurrence-live-reference','deps/fixture/MAIN.agc.bin'],'p66-recurrence-live-reference')
capture(['build/'+name],name)
left=(root/'build/p66-recurrence-live-reference.snapshots').read_text().splitlines();right=(root/f'build/{name}.snapshots').read_text().splitlines();assert len(left)==len(right),(len(left),len(right));errors=[]
for x,y in zip(left,right):
 if x!=y:
  xx=list(map(int,x.split()));yy=list(map(int,y.split()));errors.append((xx[:3],[(oct(i-3),oct(u),oct(v))for i,(u,v)in enumerate(zip(xx,yy))if u!=v]))
if errors:raise SystemExit(str(errors[:8])+f'\n{len(errors)} differing states')
print('PASS:16 original START→P66→display→idle→100hardware ticks→T3/RODTASK/RESUME→idle admission→P66→display→idle paths;',len(left),'complete2053-word states; no normalization')
print('Snapshot SHA256:',hashlib.sha256((root/'build/p66-recurrence-live-reference.snapshots').read_bytes()).hexdigest())
