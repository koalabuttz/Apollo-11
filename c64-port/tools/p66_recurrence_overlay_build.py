#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,hashlib,json,subprocess
from pathlib import Path
from overlay_build import crc16
from overlay_runtime_library import prepare
root=Path(__file__).resolve().parents[1];build=root/'build';p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');a=p.parse_args();library=prepare(a.cc,build)
sources=['src/p66_length_return_control.c','src/p66_application_recurrence.c','src/lm_clock.c','src/agc_word.c','src/executive.c','src/waitlist.c','src/p66_native.c','src/p66_startup.c','src/p66_rodtask_image.c','src/lm_waitlist_dispatch.c','src/lm_waitlist_irq_dispatch.c','src/lm_taskover_resume.c','src/lm_idle_start.c','src/dsky_display.c'];manifest=[]
for group in (57,58,59,84,85,86,87):
 out=build/f'overlay-p66-recurrence-{group}.prg';flags=[a.cc,'-Iinclude','-Iplatform/c64','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror','-nostdlib','-DP66_NATIVE_DISPATCH_ONLY',f'-DP66_RECURRENCE_SERVICE={group}','-T','platform/c64/overlay_payload.ld',f'-Wl,--defsym=__overlay_module={group},-Map,build/overlay-p66-recurrence-{group}.map'];subprocess.run(flags+sources+['platform/c64/p66_recurrence_payload.c','platform/c64/overlay_entry.S','-lc',library,'-lcrt','-o',str(out)],cwd=root,check=True);data=out.read_bytes();assert len(data)-2<=16384;manifest.append(dict(id=group,filename=f'A{group}',byte_count=len(data)-2,crc16=crc16(data[2:]),sha256=hashlib.sha256(data).hexdigest(),prg=out.name));print(group,len(data)-2,'loadedbytes')
(build/'overlay-p66-recurrence-modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
