#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Actual Comanche ISR→idle→CHARIN→genuine ENDOFJOB source checkpoints."""
import pathlib,subprocess,hashlib,sys,shutil,re,json
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC';F=R/'build/cm-keyboard-runtime-fixture';S=R/'deps/cm-services-fixture'
if not (S/'MAIN.agc.bin').exists():subprocess.run(['python3','tools/cm_services_prepare_reference.py'],cwd=R,check=True)
F.mkdir(exist_ok=True)
for p in S.glob('*.agc'):shutil.copyfile(p,F/p.name)
audit={}
for name in ['KEYRUPT_UPRUPT.agc','INTERRUPT_LEAD_INS.agc','EXECUTIVE.agc','PINBALL_GAME_BUTTONS_AND_LIGHTS.agc','T4RUPT_PROGRAM.agc','INTERPRETER.agc']:
 raw=(R.parent/'Comanche055'/name).read_bytes();original=raw.decode();normalized=re.sub(r'^[ \t]*([+-]\d+D?)\t',r' \1\t\t',original,flags=re.M)
 (F/(name+'.original')).write_bytes(raw)
 target='KEYRUPT,_UPRUPT.agc' if name=='KEYRUPT_UPRUPT.agc' else 'PINBALL_GAME__BUTTONS_AND_LIGHTS.agc' if name=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc' else name
 (F/target).write_text(normalized)
 audit[name]={'original_sha256':hashlib.sha256(raw).hexdigest(),'indentation_repairs':sum(a!=b for a,b in zip(original.splitlines(),normalized.splitlines())),'normalized_fixture':target}
with (F/'assembly.lst').open('w')as out:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=out,stderr=subprocess.STDOUT,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==(S/'MAIN.agc.bin').read_bytes()
audit['rope_sha256']=hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest();(R/'build/cm-keyboard-runtime-source-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
inputs=['tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_charin_nv_cases.c','tests/dsky_charin_number_cases.c','tests/cm_charin_nv_cases.c','tests/cm_charin_number_cases.c','tests/cm_keyboard_runtime_cases.c']
core=['src/agc_word.c','src/agc_multiword.c','src/cm_executive.c','src/cm_dispatch.c','src/cm_native_image.c','src/cm_admission_image.c','src/cm_keyboard_irq.c','src/cm_idle_dispatch.c','src/cm_basic_finish.c','src/dsky_pinball.c','src/dsky_nv_key.c','src/dsky_charin_nv.c','src/dsky_charin_number.c','src/cm_charin_nv.c','src/cm_charin_number.c','src/cm_keyboard_runtime.c']
base=['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-ffunction-sections','-Iinclude','-Itests']
subprocess.run(base+core+inputs+['tools/cm_keyboard_runtime_snapshot.c','-Wl,--gc-sections','-o','build/cm-keyboard-runtime-native'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-ffunction-sections','-Iinclude','-Itests','-I'+str(E),'src/agc_word.c']+inputs+['tools/cm_keyboard_runtime_reference.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-Wl,--gc-sections','-o','build/cm-keyboard-runtime-reference'],cwd=R,check=True)
for kind,cmd in [('native',['./build/cm-keyboard-runtime-native']),('reference',['./build/cm-keyboard-runtime-reference',str(F/'MAIN.agc.bin')])]:
 with (R/f'build/cm-keyboard-runtime-{kind}.snapshots').open('w')as out:subprocess.run(cmd,cwd=R,stdout=out,check=True)
errors=[]
with (R/'build/cm-keyboard-runtime-native.snapshots').open()as a,(R/'build/cm-keyboard-runtime-reference.snapshots').open()as b:
 for n,(x,y)in enumerate(zip(a,b)):
  av=list(map(int,x.split()));bv=list(map(int,y.split()));assert len(av)==len(bv)==2059
  if av!=bv and len(errors)<20:errors.append((n,[(oct(i-3),oct(v),oct(w))for i,(v,w)in enumerate(zip(av,bv))if v!=w]))
 assert n==4095
if errors:sys.exit(str(errors))
print('PASS:1024 complete original Comanche keyboard lifecycles,4096 whole2056-field checkpoints')
print('Snapshot SHA256:',hashlib.sha256((R/'build/cm-keyboard-runtime-reference.snapshots').read_bytes()).hexdigest())
