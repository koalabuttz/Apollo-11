#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Reconcile mission descriptive metadata to retained original tuple outcomes."""
import pathlib,hashlib,json,collections
R=pathlib.Path(__file__).resolve().parents[1];E=R/'evidence';B=R/'build';ledger=E/'cm-mission-overlay-evidence.sha256'
original=ledger.read_text().splitlines();entries={line.split('  ',1)[1]:line.split('  ',1)[0]for line in original}
for name,digest in entries.items():assert hashlib.sha256((R/name).read_bytes()).hexdigest()==digest,name
rows=[list(map(int,line.split()))for line in (B/'cm-mission-loop-reference.snapshots').read_text().splitlines()];assert hashlib.sha256((B/'cm-mission-loop-reference.snapshots').read_bytes()).hexdigest()=='3d56d8023cc2264d042a4bf08e1246c54e807feba422398a0094d2e4d98e3503'
final=[r for r in rows if r[2]==102];counts=collections.Counter((r[-3],r[-2],r[-1])for r in final);assert len(final)==384
assert sum(v for k,v in counts.items()if k==(0,0o3332,0o50066))==0
metadata=['docs/COMANCHE_MISSION_LOOP.md','evidence/cm-mission-loop-proof.json','evidence/cm-mission-overlay-report.json'];prior={};files=[]
for name in metadata:
 p=R/name;backup=E/('cm-mission-scope-prior-'+p.name);raw=p.read_bytes();assert not backup.exists();backup.write_bytes(raw);prior[name]={'old_sha256':hashlib.sha256(raw).hexdigest(),'old_copy':backup.relative_to(R).as_posix()};files.append(backup.relative_to(R).as_posix())
p=R/metadata[0];s=p.read_text();assert 'rescheduling and CLOCKJOB admission' in s;s=s.replace('rescheduling and CLOCKJOB admission','rescheduling and TBASE update without job admission').replace('genuine CLOCKJOB03332/050066 or NBDONLY03542/014063','genuine NBDONLY03542/014063');s+='\nThe descriptive CLOCKJOB-exercise claim in the original metadata has been corrected from retained source tuple outcomes. See evidence/cm-mission-scope-correction.json for exact prior bytes/hashes; original source/native/C64 artifacts and runtime source files are unchanged. Historical exporter/differential descriptions predate this reconciliation; apply tools/cm_mission_scope_correct.py after historical export when reproducing corrected metadata.\n';p.write_text(s)
p=R/metadata[1];v=json.loads(p.read_text());assert 'CLOCKJOB03332/050066' in v['actual_foreground'];v['actual_foreground'].remove('CLOCKJOB03332/050066');v['actual_clockjob_selected_cases']=0;v['scope_correction']='NVWORD=0 CLOKTASK cohort reschedules and updates TBASE without admitting CLOCKJOB.';p.write_text(json.dumps(v,indent=2)+'\n');(B/'cm-mission-loop-proof.json').write_bytes(p.read_bytes())
p=R/metadata[2];v=json.loads(p.read_text());v['actual_foreground']=['unchanged interpreted NEWOPS','NBDONLY03542/014063'];v['actual_clockjob_selected_cases']=0;v['scope_correction']='Retained source tuples confirm no CLOCKJOB admission/execution in this replay; original fidelity artifacts are unchanged.';p.write_text(json.dumps(v,indent=2)+'\n')
for name in metadata:prior[name]['new_sha256']=hashlib.sha256((R/name).read_bytes()).hexdigest()
unchanged={name:digest for name,digest in entries.items()if name not in metadata}
for name,digest in unchanged.items():assert hashlib.sha256((R/name).read_bytes()).hexdigest()==digest,name
report={'artifact':'evidence/cm-mission-overlay-report.json','source_snapshot_sha256':'3d56d8023cc2264d042a4bf08e1246c54e807feba422398a0094d2e4d98e3503','correction':'NVWORD=0 CLOKTASK reschedules and updates TBASE; it does not admit CLOCKJOB. Prior descriptive metadata incorrectly listed CLOCKJOB as exercised. All original/native/C64 artifacts and runtime/source files retain their exact frozen identities.','metadata_changes':prior,'unchanged_frozen_entries':len(unchanged),'unchanged_identity_set_sha256':hashlib.sha256(''.join(d+'  '+n+'\n'for n,d in sorted(unchanged.items())).encode()).hexdigest(),'observed_actual_foreground':[{'interpretive':k[0],'location_octal':format(k[1],'05o'),'bank_octal':format(k[2],'06o'),'cases':v}for k,v in sorted(counts.items())],'covered_cases':384,'clockjob_selected_cases':0,'actual_clockjob_body_execution_cases':0}
p=E/'cm-mission-scope-correction.json';p.write_text(json.dumps(report,indent=2)+'\n');files+=[p.relative_to(R).as_posix(),'tools/cm_mission_scope_correct.py'];newnames=set(entries)|set(files)
ledger.write_text(''.join(hashlib.sha256((R/n).read_bytes()).hexdigest()+'  '+n+'\n'for n in sorted(newnames)))
commit=metadata+files+[ledger.relative_to(R).as_posix()];(B/'cm-mission-scope-correction-commit-files.txt').write_text('\n'.join(sorted(set(commit)))+'\n')
print('PASS: corrected only three descriptive metadata files; '+str(len(unchanged))+' prior frozen source/runtime/source-state/C64 identities unchanged')
