#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build one shared resident trampoline; retain the original milestone disk."""
import argparse,pathlib,shutil,subprocess,os
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');a=p.parse_args()
sources=['src/agc_word.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/p66_guidance_diagnostic.c']
subprocess.run([a.cc,'-Iinclude','-Itests','-Iplatform/c64','-Ibuild','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror','-DEXEC_NATIVE_FINISH_FAMILY=1','-DP66_NATIVE_DISPATCH_ONLY','-DP66_GUIDANCE_DISPATCH_ONLY','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/p66-resident-outlined.map']+sources+['-o','build/p66-resident-outlined.prg'],cwd=root,check=True)
disk=root/'build/p66-resident-outlined.d81'
shutil.copyfile(root/'evidence/p66-guidance.d81',disk)
subprocess.run([a.c1541,str(disk),'-delete','resident','-write',str(root/'build/p66-resident-outlined.prg'),'resident'],cwd=root,check=True,env=dict(os.environ,XDG_CACHE_HOME=str(root/'build/vice-cache'),XDG_CONFIG_HOME=str(root/'build/vice-config')))
print('Built outlined resident; all retained math and source-data modules remain unchanged.')
