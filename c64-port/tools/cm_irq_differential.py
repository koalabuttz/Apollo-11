#!/usr/bin/env python3
"""Verify admitted CM ISR payloads against actual original seven-core code."""
import hashlib,json,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]; E=R/'deps/virtualagc/yaAGC'; F=R/'deps/comanche-fixture'
subprocess.run(['cc','-std=c99','-O2','-DCM_IRQ_INPUT_ONLY','-I'+str(E),'-Iinclude','-Itests','tools/cm_irq_reference.c','tests/cm_irq_cases.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-irq-reference'],cwd=R,check=True)
for name,cmd in [('native',['./build/cm-irq-snapshot']),('reference',['./build/cm-irq-reference',str(F/'MAIN.agc.bin')])]:
 p=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-irq-{name}.snapshots').write_text(p.stdout); (R/f'build/cm-irq-{name}.trace').write_text(p.stderr)
a=(R/'build/cm-irq-native.snapshots').read_bytes(); b=(R/'build/cm-irq-reference.snapshots').read_bytes()
assert a==b,'CM IRQ admission complete state mismatch'
rows=[list(map(int,r.split())) for r in a.decode().splitlines()];assert len(rows)==512 and all(len(r)==314 for r in rows)
print('PASS: 512 valid CM IRQ payloads / all312owned words exact against original admission; native FIFO publication/ticket consumption checked; oracle16guards')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())

assert (F/'EXECUTIVE.original.agc').read_bytes()==(R.parent/'Comanche055/EXECUTIVE.agc').read_bytes()
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
subprocess.run(['cc','-std=c99','-O2','-DDISPATCH_INPUT_ONLY','-I'+str(E),'-Iinclude','-Itests','tools/cm_dispatch_reference.c','tests/cm_executive_cases.c','tests/cm_dispatch_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-irq-dispatch-reference'],cwd=R,check=True)
ref=subprocess.run(['./build/cm-irq-dispatch-reference',str(F/'MAIN.agc.bin')],cwd=R,text=True,capture_output=True,check=True)
(R/'build/cm-irq-dispatch-reference.trace').write_text(ref.stderr)
p=subprocess.run(["./build/cm-irq-sequence-snapshot"],cwd=R,text=True,capture_output=True,check=True)
(R/"build/cm-irq-sequence-native.snapshots").write_text(p.stdout)
rows=[x for x in ref.stdout.splitlines() if 42<=int(x.split()[0])<50]
expected="\n".join(rows)+"\n"
(R/"build/cm-irq-sequence-reference.snapshots").write_text(expected)
assert len(rows)==64 and expected==p.stdout
print("PASS: eight actual CM IRQ callback sequences /64complete original dispatch states")

audit={'adaptation':'Identical restoring LLVM-MOS/KERNAL gate with separate CM typed queue producer. No Executive state or decoder in ISR.','files':{}}
for old,new in [('irq_gate.S','cm_irq_gate.S'),('irq_platform.c','cm_irq_platform.c'),('irq_platform.h','cm_irq_platform.h')]:
 original=(R/'platform/c64'/old).read_bytes(); adapted=(R/'platform/c64'/new).read_bytes()
 restored=adapted.decode().replace('cm_c64_','c64_').replace('CM_C64_IRQ_PLATFORM_H','C64_IRQ_PLATFORM_H').replace('"cm_irq_platform.h"','"irq_platform.h"').replace('"cm_irq.h"','"irq_delivery.h"').replace('cm_irq_queue','exec_irq_queue').replace('cm_irq_request','exec_request').replace('cm_irq_post','exec_irq_post')
 assert restored.encode()==original,new
 audit['files'][new]={'source_sha256':hashlib.sha256(original).hexdigest(),'adapted_sha256':hashlib.sha256(adapted).hexdigest(),'inverse_symbols_and_queue_types_equal':True}
(R/'build/cm-irq-platform-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
print('PASS: platform gate/producer/exclusion preserve original implementation after CM symbol/type adaptation')
