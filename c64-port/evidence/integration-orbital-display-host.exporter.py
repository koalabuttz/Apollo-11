from pathlib import Path
import hashlib,subprocess,shutil,json
repo=Path('/workspace/Apollo-11');dst=Path('/tmp/apollo-current-host');assert not dst.exists();dst.mkdir();head=subprocess.check_output(['git','-C',str(repo),'rev-parse','HEAD'],text=True).strip()
inv=json.loads((repo/'c64-port/docs/port-coverage.json').read_text());required={'c64-port/'+v['path']for r in inv['files']for v in r.get('complete_native_coverage',{}).get('evidence',{}).values()}
for f in ['evidence/lm-idle-start-reference.snapshots.xz','evidence/lm-taskover-resume-reference.snapshots.xz','evidence/lm-waitlist-dispatch-reference.snapshots.xz','evidence/lm-rcs-monitor-complete.snapshots.xz','evidence/dsky-nv-admission-reference.snapshots.xz','evidence/p66-application-start-source.snapshots.xz','evidence/p66-application-job-source.snapshots.xz','evidence/p66-application-live-source.snapshots.xz','evidence/fixed-data-audit.json.gz','evidence/fixed-data-lm-original.rope.bin','evidence/fixed-data-cm-original.rope.bin','evidence/lm-torque-reference.snapshots.xz','evidence/lm-torque-native.snapshots.xz']:required.add('c64-port/'+f)
required.update(v for v in subprocess.check_output(['git','-C',str(repo),'ls-tree','-r','--name-only',head],text=True).splitlines() if '/evidence/' in v and (('.snapshots.xz' in v or '.snapshots.bin.xz' in v or '.states.xz' in v) or v.endswith('p66-basic-task-reference.snapshots.gz')))
rows=subprocess.check_output(['git','-C',str(repo),'ls-tree','-rlz',head]).split(b'\0');count=0
for row in rows:
 if not row:continue
 meta,path=row.decode().split('\t',1);mode,typ,sha,size=meta.split()
 if typ!='blob':continue
 if '/evidence/'in path and int(size)>250000 and path not in required:continue
 f=repo/path;data=f.read_bytes()if f.exists()else b''
 if hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()!=sha:data=subprocess.check_output(['git','-C',str(repo),'cat-file','blob',sha])
 q=dst/path;q.parent.mkdir(parents=True,exist_ok=True);q.write_bytes(data)
 if mode=='100755':q.chmod(0o755)
 count+=1
(dst/'.git').write_text('gitdir: /workspace/Apollo-11/.git\n');(dst/'c64-port/deps').symlink_to(repo/'c64-port/deps',target_is_directory=True)
print('Isolated exact committed snapshot',head,'files',count,flush=True);(dst/'audit-state.json').write_text(json.dumps({'commit':head,'copied_files':count,'uncommitted_core_excluded':True,'shared_read_only_oracle_dependencies':'/workspace/Apollo-11/c64-port/deps'}))
with(dst/'host.log').open('w')as out:r=subprocess.run(['make','-j4','all','test'],cwd=dst/'c64-port',stdout=out,stderr=subprocess.STDOUT)
print('HOST EXIT',r.returncode,flush=True)
if r.returncode:print('\n'.join((dst/'host.log').read_text().splitlines()[-55:]))
raise SystemExit(r.returncode)
