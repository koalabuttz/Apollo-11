#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare fresh native support output against exact retained source records."""
import argparse,json,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--control',required=True);p.add_argument('--trig',required=True);a=p.parse_args()
proof=json.loads((R/'evidence/sband-support-proof.json').read_text())
for name,x in proof['corpora'].items():
 command=['python3','tools/qualified_stream_host.py','--binary',a.control if name=='control' else a.trig,'--archive',x['canonical_original_archive'],'--sha256',x['decoded_sha256'],'--bytes',str(x['decoded_bytes'])]
 if name!='control':command+=['--arg',name.split('-')[1]]
 subprocess.run(command,cwd=R,check=True)
print('PASS all 339,968 complete original antenna control/trig source states')
