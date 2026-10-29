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
sources=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/dsky_scale.c','src/p66_nv_words.c','src/p66_nv_rows.c','src/p66_nv_entry.c','src/p66_nv_render.c','src/dsky_two_digit.c','src/dsky_pinball.c','src/dsky_decimal.c','src/p66_goplay.c','src/p66_makeplay.c','src/p66_makeplay_sleep.c','src/lm_basic_priority.c','src/lm_basic_wake.c','src/lm_basic_sleep.c','src/lm_basic_finish.c','src/lm_pinball_busy.c','src/executive.c','src/p66_display_job.c','platform/c64/p66_display_job_payload.c','platform/c64/overlay_entry.S']
manifest=[]
for selector in range(300,322):
 output=build/f'overlay-p66-display-{selector}.prg'
 subprocess.run(flags+['-DP66_DISPLAY_JOB_KERNEL_ONLY',f'-DP66_DISPLAY_JOB_ONLY_SELECTOR={selector}',f'-Wl,--defsym=__overlay_module={selector-240},-Map,build/overlay-p66-display-{selector}.map']+sources+['-lc',library,'-lcrt','-o',str(output)],cwd=root,check=True)
 data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=selector-240,selector=selector,filename=f'D{selector-240}',byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=output.name,mission_translation=True))
 print(selector,len(data)-2,'loaded bytes')
(build/'overlay-p66-display-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
