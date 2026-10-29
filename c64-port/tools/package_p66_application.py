#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze the exact independently qualified startup application packet."""
import ast,hashlib,json,lzma,re,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
owned=['include/p66_application.h','src/p66_application.c','tests/p66_application_job_cases.h','tests/p66_application_job_cases.c','tests/p66_application_live_cases.h','tests/p66_application_live_cases.c','tests/p66_application_live_inputs.h','tests/p66_application_live_vectors.h','platform/c64/p66_application_fixture_input.c','platform/c64/p66_application_fixture_check.c','platform/c64/p66_application_runtime_diagnostic.c','docs/P66-APPLICATION-STARTUP.md','tools/package_p66_application.py','tools/p66_application.mk','tools/p66_application_source_audit.py','tools/p66_application_start_snapshot.c','tools/p66_application_start_differential.py','tools/p66_application_job_snapshot.c','tools/p66_application_job_reference.c','tools/p66_application_job_differential.py','tools/p66_application_live_inputs.c','tools/p66_application_live_snapshot.c','tools/p66_application_live_reference.c','tools/p66_application_live_differential.py','tools/p66_application_live_build.py','tools/p66_application_live_vice_test.py','tools/freeze_p66_application_live.py']
def retain(source,destination):
 shutil.copyfile(source,evidence/destination);owned.append('evidence/'+destination)
streams={}
for label,count,words,expected in [('start',2048,2050,'56bd1037bd7ac328b5433be6e258fc9a76260b70ff8e4def4d5184eea2b9f274'),('job',13312,2052,'bab1ff892632cd9c90d139229a9af041913c697301567a7ee1ac7dc6690bb22e'),('live',1226,2052,'9ea8e8bc75b90a91ac5185b2546cf93ca47dc786b691633527df6959819e22dc')]:
 reference=build/('p66-direct-launch-reference.snapshots'if label=='start'else f'p66-application-{label}-reference.snapshots')
 strict=build/f'p66-application-{label}-native.snapshots'
 sanitized=build/f'p66-application-{label}-sanitize.snapshots'
 # The earliest router test used a separate native stem; accept that exact one.
 if label=='start':sanitized=build/'p66-application-start-sanitize.snapshots'
 assert sha(reference)==sha(strict)==sha(sanitized)==expected
 archive=evidence/f'p66-application-{label}-source.snapshots.xz'
 with reference.open('rb') as source,lzma.open(archive,'wb',preset=6) as out:
  for chunk in iter(lambda:source.read(1048576),b''):out.write(chunk)
 decoded=hashlib.sha256()
 with lzma.open(archive,'rb') as source:
  for chunk in iter(lambda:source.read(1048576),b''):decoded.update(chunk)
 assert decoded.hexdigest()==expected
 owned.append(str(archive.relative_to(root)));streams[label]=dict(states=count,words=words,sha256=expected,strict='PASS',full_asan_ubsan='PASS')
 for kind in ('check','sanitize-check'):retain(build/f'p66-application-{label}-{kind}.log',f'p66-application-{label}-{kind}.log')
manifest=json.loads((build/'p66-application-live-modules.json').read_text());assert len(manifest)==104 and len({m['id']for m in manifest})==104
for m in manifest:
 source=build/m['prg'];assert sha(source)==m['sha256'];destination=f'p66-application-live-module-{m["id"]}.prg';retain(source,destination);m['retained_prg']='evidence/'+destination
 mapfile=source.with_suffix('.map');assert mapfile.exists();retain(mapfile,f'p66-application-live-module-{m["id"]}.map')
manifestfile=evidence/'p66-application-live-modules.json';manifestfile.write_text(json.dumps(manifest,indent=2)+'\n');owned.append(str(manifestfile.relative_to(root)))
for extension in ('prg','map','d81'):retain(build/f'p66-application-live.{extension}',f'p66-application-live.{extension}')
for suffix in ('vice.log','vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-hardware-stack.bin'):retain(build/f'p66-application-live-{suffix}',f'p66-application-live-{suffix}')
retain(build/'p66-application-source-audit.json','p66-application-source-audit.json');retain(build/'p66-application-sizes.ll','p66-application-target-sizes.ll')
assert (evidence/'p66-application-live-vice-mailbox.bin').read_bytes()==bytes.fromhex('a500009c0002')
scope={'root':root};names={'inputs','defines','numerical','native','extra'}
for node in ast.parse((root/'tools/p66_complete_live_differential.py').read_text()).body:
 if isinstance(node,(ast.Assign,ast.AugAssign)):
  targets=node.targets if isinstance(node,ast.Assign)else[node.target]
  if all(isinstance(t,ast.Name) and t.id in names for t in targets):exec(compile(ast.Module(body=[node],type_ignores=[]),'source-inventory','exec'),scope)
sources=scope['native']+scope['numerical']+scope['inputs']+[p for p in owned if p.endswith('.c')and not p.startswith('platform/')]+['src/p66_launch_phased.c','src/p66_direct_launch.c','src/dsky_display.c']
for file in ('p66_launch','p66_mode_display','dsky_charin_nv','dsky_charin_number'):
 for line in (root/f'tools/{file}.mk').read_text().splitlines():
  if '_CORE = 'in line:sources.extend(name for name in line.split(' = ',1)[1].split()if name.startswith('src/'))
sources=list(dict.fromkeys(sources));sources=[s for s in sources if not s.endswith('_reference.c')]
output=subprocess.check_output(['gcc','-MM','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+scope['defines']+sources,cwd=root,text=True)
dependencies=set(sources)
for line in output.replace('\\\n',' ').splitlines():dependencies.update(line.split(':',1)[1].split())
dependencies.update(['include/native_dispatch.h','src/native_dispatch.c','src/native_overlay_dispatch.c','include/native_overlay_dispatch.h','tools/p66_complete_live_differential.py','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_platform.c','platform/c64/p66_application_service_payload.h','platform/c64/p66_application_service_payload.c','tools/p66_application_service_build.py'])
mapping=(build/'p66-application-live.map').read_text();ends=[]
for section in ('.text','.rodata','.data','.bss','.noinit'):
 pairs=re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M);assert len(pairs)==1;ends.append(sum(int(x,16)for x in pairs[0]))
assert max(ends)==0x6810
proof={'scope':'actual STARTP66 through source startup/P66, deferred mode and MAKEPLAY display jobs, canonical END and idle; keyboard bodies and recurrence separate','host':streams,'source_audit':'evidence/p66-application-source-audit.json','target_modules':104,'target_checkpoints':156,'target_words':2052,'mailbox':'a500009c0002','software_stack_bytes':777,'hardware_stack_bytes':70,'extra_zp':0,'resident_end':'0x6810','resident_spare':2032,'metadata_bytes':163,'metadata_used':'c400..c4a2','metadata_lease':'c400..c5ff','executive_view':'648B c100..c387','reserved_task_views':'76B c388..c3d3','remaining':'actual queued recurrence/outer keyboard IRQ and remaining keys; canonical higher-priority flight-job composition; other landing programs and both full applications','files':{p:sha(root/p)for p in owned},'dependencies':{p:sha(root/p)for p in sorted(dependencies)}}
path=evidence/'p66-application-startup-proof.json';path.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(path.relative_to(root)));(build/'p66-application-startup-commit-files.txt').write_text('\n'.join(owned)+'\n');print('Frozen exact startup application packet:',len(owned),'owned files;104 disk modules;156 C64/1226 fullsource path checkpoints')
