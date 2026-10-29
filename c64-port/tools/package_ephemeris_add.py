from pathlib import Path
import hashlib,json,shutil,subprocess
p=Path(__file__).resolve().parents[1];sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();files=set()
for directory in ['include','src','tests','tools','platform/c64']:
 for f in (p/directory).glob('*ephemeris*'):
  if f.is_file():files.add(str(f.relative_to(p)))
files.add('docs/NATIVE-EPHEMERIS-ADD.md')
names=['ephemeris-add-peer-review.json','ephemeris-source-map.json','ephemeris-source-validation.log','ephemeris-add-target-source.snapshots.bin.xz','ephemeris-add-target-results.json','ephemeris-add-original-host.log','ephemeris-add-sanitize-validation.log','ephemeris-add-c64-validation.log','ephemeris-add.prg','ephemeris-add.map','ephemeris-add-mailbox.bin','ephemeris-add-vice.mon','ephemeris-add-monitor.log','ephemeris-add-vice.log','ephemeris-add-vice-results.json','ephemeris-add-hardware-stack.bin','ephemeris-add-software-stack.bin']
for n in names:
 f=p/'evidence'/n
 if n!='ephemeris-add-peer-review.json':shutil.copyfile(p/'build'/n,f)
 files.add(str(f.relative_to(p)))
for n in ['ephemeris-add-source-streams.json','ephemeris-add-LM-reference.snapshots.bin.xz','ephemeris-add-CM-reference.snapshots.bin.xz']:files.add('evidence/'+n)
src=p/'build/ephemeris-add-first-target-guard-failure';dest=p/'evidence/ephemeris-add-first-target-guard-failure';
if not dest.exists():shutil.copytree(src,dest)
else:
 for f in src.iterdir():assert sha(f)==sha(dest/f.name)
for f in dest.iterdir():files.add(str(f.relative_to(p)))
x={'scope':'Native original TAD basic arithmetic prerequisite only; no ephemerides program or whole interpreter certificate','source_states':131072,'state_words':2053,'strict_native':'PASS','full_ASAN_UBSAN':'PASS','source_streams':json.loads((p/'evidence/ephemeris-add-source-streams.json').read_text()),'c64':json.loads((p/'evidence/ephemeris-add-vice-results.json').read_text()),'source_dependency_words_per_mission':29,'historical_source_baseline':'911e5c0283c629c50cb97666f34065e8c07d71a5','whole_containing_files_certified':False,'native_instruction_decoder':False,'failed_target_retained':'evidence/ephemeris-add-first-target-guard-failure/reason.json','files':{n:sha(p/n)for n in sorted(files)}}
f=p/'evidence/ephemeris-add-proof.json';f.write_text(json.dumps(x,indent=2)+'\n');files.add(str(f.relative_to(p)));(p/'build/ephemeris-add-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print('PACKAGED',len(files),'immutable prerequisite paths',flush=True)
