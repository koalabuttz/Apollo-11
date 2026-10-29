#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build fixed horizontal entries in the existing 16KB arithmetic slot."""
import argparse,pathlib,json,hashlib,subprocess
from overlay_build import crc16
from overlay_runtime_library import prepare
root=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');a=p.parse_args();build=root/'build';library=prepare(a.cc,build)
flags=[a.cc,'-Iinclude','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror','-nostdlib','-T','platform/c64/overlay_payload.ld']
sources=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_vector.c','src/guidance_sum.c','src/guidance_shift.c','src/guidance_dp_divide.c','src/guidance_norm.c','src/guidance_unit.c','src/guidance_horizontal.c','src/guidance_feedback_pairs.c','src/guidance_unit_phased.c','src/guidance_length_phased.c','src/guidance_horizontal_phased.c','src/p66_native_descent.c','platform/c64/p66_descent_overlay_payload.c','platform/c64/overlay_entry.S'];manifest=[]
for selector in tuple(range(190,214))+tuple(range(220,228))+tuple(range(230,237)):
 if selector in(191,223,225):continue
 output=build/f'overlay-p66-descent-{selector}.prg'
 subprocess.run(flags+[f'-DP66_DESCENT_ONLY_GROUP={selector}',f'-Wl,--defsym=__overlay_module={selector},-Map,build/overlay-p66-descent-{selector}.map']+sources+['-lc',library,'-lcrt','-o',str(output)],cwd=root,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=selector,filename=f'G{selector}',byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=True))
 print(selector,len(data)-2,'loaded bytes')
(build/'overlay-p66-descent-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
