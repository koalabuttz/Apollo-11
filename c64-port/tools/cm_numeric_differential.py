#!/usr/bin/env python3
"""Qualify shared numerical kernels against actual original Comanche addresses."""
import pathlib,hashlib,subprocess,difflib,json,re,sys
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-numeric-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_numeric_prepare_reference.py'],cwd=R,check=True)
p=E/'agc_engine.c';original=p.read_text();assert hashlib.sha256(p.read_bytes()).hexdigest()=='9c59728cdd968503caaaafe50a18ab3ed841f6176da3299c137f3a6a158fcbef'
needle='if (AbsA > AbsK || (AbsA == AbsK && AbsL != AGC_P0) || ValueOverflowed(Div16) != AGC_P0)';assert original.count(needle)==1
modified=original.replace(needle,needle.replace('if (','if (1 || ',1))
(R/'build/cm-numeric-hardware-dv.c').write_text(modified)
(R/'build/cm-numeric-hardware-dv.patch').write_text(''.join(difflib.unified_diff(original.splitlines(True),modified.splitlines(True),fromfile='pinned/agc_engine.c',tofile='cm-hardware-dv/agc_engine.c')))
subprocess.run(['cc','-O2','-I'+str(E),'-c','build/cm-numeric-hardware-dv.c','-o','build/cm-numeric-hardware-dv.o'],cwd=R,check=True)
if '--prepare-only' in sys.argv:
 sys.exit(0)
base=['cc','-std=c99','-O2','-Iinclude','-Itests'];defs=['-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
cases=['tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c']
common=['src/agc_word.c','src/agc_multiword.c'];specs=[
 ('norm',132096,20,[],[],['tests/norm_snapshot.c','src/guidance_norm.c']),
 ('dp-divide',163840,17,[],[],['tests/dp_divide_snapshot.c','src/guidance_dp_divide.c']),
 ('unit',50736,104,defs,cases,['tests/unit_snapshot.c','src/guidance_unit.c','src/guidance_norm.c','src/guidance_dp_divide.c'])]
proof={'fixture_rope_sha256':hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest(),'engine_source_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'hardware_dv_variant_sha256':hashlib.sha256(modified.encode()).hexdigest(),'kernels':{}}
for name,count,fields,flags,inputs,native in specs:
 tool=name.replace('-','_')
 subprocess.run(base+flags+native+common+inputs+['-o',f'build/cm-{name}-native'],cwd=R,check=True)
 subprocess.run(base+flags+(['-DUNIT_INPUT_ONLY'] if name=='unit' else [])+['-I'+str(E),f'tools/cm_{tool}_reference.c']+inputs+['tests/cm_executive_cases.c','build/cm-numeric-hardware-dv.o',str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o',f'build/cm-{name}-reference'],cwd=R,check=True)
 for role,cmd in [('native',[f'./build/cm-{name}-native']),('reference',[f'./build/cm-{name}-reference',str(F/'MAIN.agc.bin')])]:
  run=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
  (R/f'build/cm-{name}-{role}.snapshots').write_text(run.stdout);(R/f'build/cm-{name}-{role}.trace').write_text(run.stderr)
 a=(R/f'build/cm-{name}-native.snapshots').read_bytes();b=(R/f'build/cm-{name}-reference.snapshots').read_bytes();assert a==b,name
 rows=[line.split() for line in b.decode().splitlines()];assert len(rows)==count and all(len(row)==fields+2 for row in rows)
 h=hashlib.sha256(b).hexdigest();header=R/f'tests/{tool}_vectors.h';assert h in header.read_text(),f'{name}: existing frozen source vector provenance differs'
 proof['kernels'][name]={'cases':count,'modeled_words':fields,'snapshot_sha256':h,'frozen_vector_header':str(header.relative_to(R)),'frozen_header_sha256':hashlib.sha256(header.read_bytes()).hexdigest()}
 print(f'PASS: CM {name}: {count} cases/all {fields} modeled words; exact shared native kernel and existing lossless vectors; SHA256 {h}')
(R/'build/cm-numeric-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
