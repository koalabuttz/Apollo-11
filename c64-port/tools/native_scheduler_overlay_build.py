#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build literal basic services using the unchanged fixed-slot C ABI."""
import argparse,pathlib,json,hashlib,subprocess
from overlay_build import crc16
from overlay_runtime_library import prepare
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');a=p.parse_args()
build=root/'build';library=prepare(a.cc,build)
flags=[a.cc,'-Iinclude','-Iplatform/c64','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror','-nostdlib','-T','platform/c64/overlay_payload.ld']
sources=['src/agc_word.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','platform/c64/native_scheduler_payload.c','platform/c64/overlay_entry.S']
manifest=[]
for selector in range(150,154):
 output=build/f'overlay-native-scheduler-{selector}.prg'
 subprocess.run(flags+[f'-DNATIVE_SCHEDULER_STAGE={1 if selector==150 else 2}',f'-DEXEC_NATIVE_FINISH_FAMILY={1 if selector==150 else selector-150}',f'-Wl,--defsym=__overlay_module={selector},-Map,build/overlay-native-scheduler-{selector}.map']+sources+['-lc',library,'-lcrt','-o',str(output)],cwd=root,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=selector,filename=f'S{selector}',byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=True))
 print(selector,len(data)-2,'loaded bytes')
(build/'overlay-native-scheduler-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
