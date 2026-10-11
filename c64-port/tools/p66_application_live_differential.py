#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source STARTP66 through descent, its queued mode job, MAKEPLAY and idle."""
import argparse,hashlib,os,subprocess,ast
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
# Reuse the committed complete-path source build inventory. Only its explicit
# dependency declarations are evaluated; no command or argument parser runs.
inventory=ast.parse((ROOT/'tools/p66_complete_live_differential.py').read_text())
scope={'root':ROOT};names={'inputs','defines','numerical','native','extra'}
for node in inventory.body:
 if isinstance(node,(ast.Assign,ast.AugAssign)):
  targets=node.targets if isinstance(node,ast.Assign) else [node.target]
  if all(isinstance(t,ast.Name) and t.id in names for t in targets):exec(compile(ast.Module(body=[node],type_ignores=[]),'source-build-inventory','exec'),scope)
sources=scope['native']+scope['numerical']+['src/p66_application.c','src/p66_launch_phased.c','src/p66_direct_launch.c','src/dsky_display.c']
for file in ('p66_launch','p66_mode_display','dsky_charin_nv','dsky_charin_number'):
 for line in (ROOT/f'tools/{file}.mk').read_text().splitlines():
  if '_CORE = ' in line:sources.extend(name for name in line.split(' = ',1)[1].split() if name.startswith('src/'))
sources=list(dict.fromkeys(sources));inputs=scope['inputs']+['tests/p66_application_live_cases.c']
base=['gcc','-std=c99','-O1' if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-Iinclude','-Itests']+scope['defines']
if a.sanitize:base+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
name='p66-application-live-sanitize' if a.sanitize else 'p66-application-live-native'
subprocess.run(base+['-DP66_BASIC_DISPATCH_ONLY','-DP66_DIRECT_LAUNCH_DISPATCH_ONLY','tools/p66_application_live_snapshot.c']+sources+inputs+['-o','build/'+name],cwd=ROOT,check=True)
def capture(command,stem):
 with (ROOT/f'build/{stem}.snapshots').open('w') as out,(ROOT/f'build/{stem}.trace').open('w') as err:subprocess.run(command,cwd=ROOT,stdout=out,stderr=err,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if not a.sanitize:
 subprocess.run(['python3','tools/prepare_hardware_dv.py'],cwd=ROOT,check=True)
 engine=ROOT/'deps/virtualagc/yaAGC'
 subprocess.run(base+['-DP66_NATIVE_DISPATCH_ONLY','-I'+str(engine),'tools/p66_application_live_reference.c','tests/scenarios.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c']+inputs+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-application-live-reference'],cwd=ROOT,check=True)
 capture(['build/p66-application-live-reference','deps/fixture/MAIN.agc.bin'],'p66-application-live-reference')
capture(['build/'+name],name)
got=(ROOT/f'build/{name}.snapshots').read_text().splitlines();expected=(ROOT/'build/p66-application-live-reference.snapshots').read_text().splitlines()
if len(got)!=len(expected):raise SystemExit(f'Checkpoint count native{len(got)} original{len(expected)}')
for index,(left,right) in enumerate(zip(got,expected)):
 if left!=right:
  x=list(map(int,left.split()));y=list(map(int,right.split()));diff=[(oct(i-3),oct(v),oct(w))for i,(v,w)in enumerate(zip(x,y))if v!=w]
  raise SystemExit(f'row{index} case/stage/boundary {x[:3]} original{y[:3]} native/original {diff[:30]}')
print('PASS16 actual startup→P66→deferred DSPMMJB→MAKEPLAY→END→idle paths:',len(got),'full2052 raw source checkpoints')
print('Snapshot SHA256:',hashlib.sha256((ROOT/f'build/{name}.snapshots').read_bytes()).hexdigest())
