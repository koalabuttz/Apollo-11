#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare fresh native VXV/VPROJ to both complete retained original corpora."""
import argparse,pathlib,json,subprocess
r=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--binary',required=True);a=p.parse_args()
proof=json.loads((r/'evidence/sband-vector-proof.json').read_text())
for mission,x in proof['corpora'].items():
 subprocess.run(['python3','tools/qualified_stream_host.py','--binary',a.binary,'--archive',x['canonical_original_archive'],'--sha256',x['decoded_sha256'],'--bytes',str(x['decoded_bytes']),'--arg',mission],cwd=r,check=True)
print('PASS all262144 original VXV/VPROJ full source-state bytes')
