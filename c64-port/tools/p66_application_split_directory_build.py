#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify every extended metadata record without altering the active disk."""
import argparse,json,subprocess
from pathlib import Path
from overlay_build import crc16
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');a=p.parse_args()
modules=[m for m in json.loads((root/'build/p66-application-split-modules.json').read_text())if m['id']!=148]
assert len(modules)==113 and len({m['id']for m in modules})==113
raw=bytearray()
for m in sorted(modules,key=lambda m:m['id']):raw+=bytes([m['id']])+m['byte_count'].to_bytes(2,'little')+m['crc16'].to_bytes(2,'little')+m['filename'].encode()+b'\0'
subprocess.run([a.cc,'-Ibuild/p66-application-split','-Iinclude','-Iplatform/c64','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror','-DNATIVE_REGISTRY_EXPECTED_COUNT=113',f'-DNATIVE_REGISTRY_EXPECTED_CRC={crc16(raw)}','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/p66-application-split-directory.map','src/overlay_runtime.c','src/native_module_registry.c','platform/c64/overlay_platform.c','platform/c64/native_registry_directory_diagnostic.c','-o','build/p66-application-split-directory.prg'],cwd=root,check=True)
print('Built113-record extended directory diagnostic; active resident/D81/metadata bytes untouched')
