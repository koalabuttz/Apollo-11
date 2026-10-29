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
sources=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_throttle_service.c','src/agc_hardware_divide.c','src/p66_initialization.c','src/executive.c','src/p66_runtime_bridge.c','src/p66_runtime_fetch_bridge.c','src/p66_runtime_regodspr.c','src/p66_descent_control.c','platform/c64/p66_descent_control_payload.c','platform/c64/overlay_entry.S']
manifest=[]
for selector in range(90,95):
 output=build/f'overlay-p66-control-{selector}.prg'
 subprocess.run(flags+[f'-DP66_DESCENT_ONLY_CONTROL={selector}',f'-Wl,--defsym=__overlay_module={selector},-Map,build/overlay-p66-control-{selector}.map']+sources+['-lc',library,'-lcrt','-o',str(output)],cwd=root,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=selector,filename=f'C{selector}',byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=True))
 print(selector,len(data)-2,'loaded bytes')
(build/'overlay-p66-control-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
