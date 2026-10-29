#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1]
for mission,tree in [('LM','Luminary099'),('CM','Comanche055')]:
 p=subprocess.Popen([str(R/'build/sband-vector-reference'),str(R/'build/sband-source'/tree/'MAIN.agc.bin'),mission],stdout=subprocess.PIPE)
 h=hashlib.sha256();size=0
 while chunk:=p.stdout.read(1024*1024):h.update(chunk);size+=len(chunk)
 assert p.wait()==0
 expected=json.loads((R/f'build/sband-vector-{mission}-native-results.json').read_text());assert h.hexdigest()==expected['decoded_sha256'] and size==expected['states']*4106
 (R/f'build/sband-vector-{mission}-trace-results.json').write_text(json.dumps(dict(states=expected['states'],decoded_sha256=h.hexdigest(),decoded_bytes=size),indent=2)+'\n')
 print('PASS',mission,'final traced source aggregate',h.hexdigest(),flush=True)
