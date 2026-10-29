#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Real prepare150/CHANG2154/adopt153 returns against original source state."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
from overlay_build import crc16
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args();build=ROOT/'build';stem='lm-interpreter-exchange'
flags=[a.cc,'-Iinclude','-Iplatform/c64','-Itests','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror']
subprocess.run(flags+['-nostdlib','-T','platform/c64/overlay_payload.ld','-Wl,--defsym=__overlay_module=154,-Map,build/'+stem+'-payload.map','src/agc_word.c','src/lm_interpreter_change.c','platform/c64/lm_interpreter_exchange_payload.c','platform/c64/overlay_entry.S','-lcrt','-o','build/'+stem+'-payload.prg'],cwd=ROOT,check=True)
data=(build/(stem+'-payload.prg')).read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
manifest=json.loads((build/'lm-interpreter-change-target-modules.json').read_text());scheduler=json.loads((build/'overlay-native-scheduler-modules.json').read_text());manifest.extend(m for m in scheduler if m['id'] in (150,153));manifest.append(dict(id=154,filename='X154',byte_count=len(data)-2,crc16=crc16(data[2:]),prg=stem+'-payload.prg',sha256=hashlib.sha256(data).hexdigest()))
for m in manifest:assert hashlib.sha256((build/m['prg']).read_bytes()).hexdigest()==m['sha256']
header=(build/'lm_interpreter_change_target.h').read_text();tail=header.split('static const uint32_t',1)[1];header='#include "overlay_runtime.h"\nstatic const apollo_overlay_module lm_exchange_modules[35]={\n'+''.join('{%d,"%s",%d,%d},\n'%(m['id'],m['filename'],m['byte_count'],m['crc16']) for m in manifest)+'};\nstatic const uint32_t'+tail;(build/'lm_interpreter_exchange_target.h').write_text(header)
subprocess.run(flags+['-DEXEC_NATIVE_RESIDENT_SERVICES','-DLM_EXCHANGE_DISPATCH_ONLY','-DP66_NATIVE_DISPATCH_ONLY','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/'+stem+'-target.map','src/overlay_runtime.c','platform/c64/overlay_platform.c','src/agc_word.c','src/executive.c','src/native_dispatch.c','src/native_overlay_dispatch.c','src/irq_delivery.c','src/p66_native.c','tests/lm_interpreter_exchange_native.c','platform/c64/lm_interpreter_exchange_diagnostic.c','-o','build/'+stem+'-target.prg'],cwd=ROOT,check=True)
disk=build/(stem+'-target.d81');disk.unlink(missing_ok=True);cmd=[a.c1541,'-format','CHANG2 STEP,11','d81',str(disk),'-write',str(build/(stem+'-target.prg')),'resident']
for m in manifest:cmd+=['-write',str(build/m['prg']),m['filename'].lower()]
subprocess.run(cmd,cwd=ROOT,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(build/'vice-cache'),XDG_CONFIG_HOME=str(build/'vice-config')));(build/(stem+'-target-modules.json')).write_text(json.dumps(manifest,indent=2)+'\n')
print('Built64 original2052-state cases withactualdisk150→154→153, pendingnativeIRQticket retained; payload154',len(data)-2,'bytes')
