#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='cm-quality';files=[];record=json.loads((r/'build/cm-quality-source-stream.json').read_text());archive=r/'build/cm-quality-original.states.xz';h=hashlib.sha256();size=0
with lzma.open(archive,'rb')as f:
 while raw:=f.read(1048576):h.update(raw);size+=len(raw)
assert size==record['decoded_bytes']and h.hexdigest()==record['decoded_sha256'];shutil.copyfile(archive,e/archive.name);files.append('evidence/'+archive.name)
targets={}
for phase in range(20):
 target=f'{stem}-{phase}'
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  name=target+('.'if suffix in ['prg','map']else'-')+suffix;shutil.copyfile(r/'build'/name,e/name);files.append('evidence/'+name)
 prg=(e/f'{target}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(e/f'{target}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(e/f'{target}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,phase])and len(stack)==2
 targets[str(phase)]=dict(complete_source_states=32,rejected_calls=9,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['host.log','sanitize.log','freeze.log','frozen-host.log','source-validation.log','source-map.json','source-stream.json','corpus-stats.json']:
 name=stem+'-'+suffix;shutil.copyfile(r/'build'/name,e/name);files.append('evidence/'+name)
review=e/'cm-quality-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_source_inputs=65536,complete_states=688128,model_words=2048,external_words=['IRQ','SUPER'],source_group_words=25,source_map='evidence/cm-quality-source-map.json',source_audit_dependency='evidence/rtb-angles-source-audit.json',logical_complete_streams={role:record for role in ['original_assembly','strict_native','whole_ASAN_UBSAN']},actual_C64=targets,independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),qualification='Twenty explicit original QUALITY1/2 canonical boundaries with actual fixed orbital caller023146, fiveVACs and producer exclusion; pendingEDOP retains ownership, only genuineDANZIG Executive checks. Constructor cohort only, no exhaustive Cartesian input or whole orbital/runtime claim.',decoded_stream_identity='Single deterministicXZ original object; strict and fullASANUBSAN directly compare every decoded source byte. Source instruction and original declaration rope unchanged.')
p=e/'cm-quality-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/NATIVE-CM-QUALITY.md','include/cm_quality.h','src/cm_quality.c','tests/cm_quality_cases.h','tests/cm_quality_cases.c','tests/test_cm_quality.c']+[f'tests/cm_quality_vectors_{n}.h'for n in range(20)]
files+=['tools/'+n for n in ['cm_quality.mk','cm_quality_host.py','cm_quality_reference.c','cm_quality_snapshot.c','cm_quality_inputs.c','cm_quality_source.py','cm_quality_diagnostic.c','cm_quality_vice_test.py','freeze_cm_quality.py','package_cm_quality.py']]
files=sorted(set(files));manifest='evidence/cm-quality-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/'build/cm-quality-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;688128 complete source states;640 C64 states')
