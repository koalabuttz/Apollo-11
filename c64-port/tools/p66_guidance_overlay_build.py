#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build further real P66 arithmetic modules without changing prefix artifacts."""
import argparse,pathlib,subprocess,json,hashlib
from overlay_build import crc16
from overlay_runtime_library import prepare
root=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--groups',default='103,113,105,115,125,106,116,126,136,146,156,166,176,186,107,117,127,137,147,108,118,128,9,10');a=p.parse_args();build=root/'build'
sources=['src/agc_word.c','src/agc_multiword.c','src/p66_native_guidance.c']+[f'src/guidance_{name}.c' for name in ('elapsed','store','velocity_phased','vector','control_observer','radial','radial_phased','unit','unit_phased','norm','dot','shift','product','sum','acceleration','acceleration_phased','acceleration_store','gravity','dp_divide','projection','projection_phased','trig','rotation')]
manifest=[]
runtime=prepare(a.cc,build)
for number in map(int,a.groups.split(',')):
 output=build/f'overlay-p66-extended-{number}.prg'
 command=[a.cc,'-Iinclude','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror','-nostdlib','-DP66_GUIDANCE_ARITH_ONLY',f'-DP66_GUIDANCE_ONLY_GROUP={number}','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={number},-Map,build/overlay-p66-extended-{number}.map','platform/c64/p66_guidance_overlay_payload.c','platform/c64/overlay_entry.S']+sources+['-lc',runtime,'-lcrt','-o',str(output)]
 subprocess.run(command,cwd=root,check=True);data=output.read_bytes();assert data[:2]==b'\x00\x80' and len(data)-2<=16384
 manifest.append(dict(id=number,filename=f'G{number}',byte_count=len(data)-2,crc16=crc16(data[2:]),prg=output.name,sha256=hashlib.sha256(data).hexdigest()));print(number,len(data)-2)
(build/'overlay-p66-extended-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
