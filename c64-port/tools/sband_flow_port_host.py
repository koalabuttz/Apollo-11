#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,json,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--binary',required=True);a=p.parse_args();x=json.loads((R/'evidence/sband-flow-proof.json').read_text())['corpus']
subprocess.run(['python3','tools/qualified_stream_host.py','--binary',a.binary,'--archive',x['canonical_original_archive'],'--sha256',x['decoded_sha256'],'--bytes',str(x['decoded_bytes'])],cwd=R,check=True)
print('PASS all 32,768 original antenna time/CDU caller states')
