#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
for f in ['INTERPRETER.agc','INTER-BANK_COMMUNICATION.agc','PINBALL_GAME_BUTTONS_AND_LIGHTS.agc','DISPLAY_INTERFACE_ROUTINES.agc','EXECUTIVE.agc','SERVICE_ROUTINES.agc']:
 a=rows(root.parent/'Luminary099'/f);b=rows(root/'deps/fixture'/('PINBALL_GAME__BUTTONS_AND_LIGHTS.agc' if f=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc' else f))
 if f=='INTER-BANK_COMMUNICATION.agc':
  a=a[next(i for i,x in enumerate(a)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(a)if x.startswith('POSTJUMP '))];b=b[next(i for i,x in enumerate(b)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(b)if x.startswith('POSTJUMP '))]
 if f=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc':a=[x.replace('DSPOCTWO','DSPOCTWD')for x in a]
 if f=='DISPLAY_INTERFACE_ROUTINES.agc':a=[x for x in a if x!='PRIOSAVE EQUALS R1SAVE']
 assert a==b,f
base=['gcc','-std=c99','-O2','-Iinclude','-Itests','-DP66_NATIVE_DISPATCH_ONLY','-ffunction-sections','-Wl,--gc-sections']
subprocess.run(base+['src/p66_nv_render.c','src/dsky_scale.c','src/guidance_product.c','src/p66_nv_words.c','src/p66_nv_rows.c','src/p66_nv_entry.c','src/dsky_two_digit.c','src/dsky_pinball.c','src/dsky_decimal.c','src/agc_multiword.c','src/p66_goplay.c','src/p66_makeplay.c','src/p66_makeplay_sleep.c','src/lm_basic_priority.c','src/lm_basic_wake.c','src/lm_basic_sleep.c','src/lm_basic_finish.c','src/lm_pinball_busy.c','src/p66_display_job.c','src/p66_native.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/agc_word.c','tests/throttle_bridge_cases.c','tests/makeplay_cases.c','tests/display_job_cases.c','tools/display_job_snapshot.c','-o','build/display-job-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'src/agc_word.c','tests/throttle_bridge_cases.c','tests/makeplay_cases.c','tests/display_job_cases.c','tools/display_job_reference.c','tests/scenarios.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/display-job-reference'],cwd=root,check=True)
output=[]
for name,cmd in [('native',['build/display-job-native']),('reference',['build/display-job-reference','deps/fixture/MAIN.agc.bin'])]:
 p=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/display-job-{name}.snapshots').write_text(p.stdout);output.append(p.stdout.splitlines())
assert len(output[0])==len(output[1])==11776
bad=[(a,b)for a,b in zip(*output)if a!=b]
for a,b in bad[:12]:
 av=list(map(int,a.split()));bv=list(map(int,b.split()));print('DIFF',av[0],[(oct(i-2),oct(x),oct(y))for i,(x,y)in enumerate(zip(av,bv))if x!=y])
if bad:sys.exit(f'{len(bad)} differing states')
print('PASS:11776 complete2048-word original actual native MAKEPLAY scheduling and complete V06N60 source checkpoints, plus SUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((root/'build/display-job-reference.snapshots').read_bytes()).hexdigest())
