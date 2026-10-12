#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Collect the lm-time-free-fall results file from a run of
`make test-lm-time-free-fall sanitize-lm-time-free-fall` (log given with
--checks) and `make vice-lm-time-free-fall` (log given with --vice): host
and sanitizer PASS lines, each PRG's size, hash, sections and image end, the
VICE result per PRG and the branch-outcome coverage of the C64 rows
(build/lm-time-free-fall-c64-cover.json). Writes
evidence/lm-time-free-fall/lm-time-free-fall-results.json."""
import argparse,hashlib,json,pathlib,re
R=pathlib.Path(__file__).resolve().parents[1];E=R/'evidence/lm-time-free-fall'
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--checks',required=True);p.add_argument('--vice',required=True);a=p.parse_args()
steps=json.loads((E/'lm-time-free-fall-phase-steps.json').read_text())
checks=[l.strip()for l in pathlib.Path(a.checks).read_text().splitlines()if l.startswith('PASS')]
vice={m.group(1):l.strip()for l in pathlib.Path(a.vice).read_text().splitlines()for m in[re.search(r'(lm-time-free-fall-\S+)\.prg',l)]if m and l.startswith('PASS')}
prgs={}
for name in steps['prgs']:
 g,k=name.split('-');part=steps['groups'][g]['parts'][int(k)-1]
 prg=(R/f'build/lm-time-free-fall-{name}.prg').read_bytes();m=(R/f'build/lm-time-free-fall-{name}.map').read_text()
 sec={s:int(x,16)for x,s in re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+\.(text|rodata|data|bss)\s*$',m,re.M)}
 end=max(int(x,16)for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M))
 key=f'lm-time-free-fall-{name}'
 if key not in vice:raise SystemExit('FAIL no VICE PASS line for '+key)
 prgs[name]=dict(prg_bytes=len(prg),prg_sha256=hashlib.sha256(prg).hexdigest(),sections=sec,image_end=f'${end:04x}',images='$e000/$f000 under KERNAL ROM ($01=$35)',
  phases=steps['groups'][g]['phases'],steps=part['steps'],states=part['states'],vice=vice[key])
cover=json.loads((R/'build/lm-time-free-fall-c64-cover.json').read_text())
result=dict(toolchain='llvm-mos SDK (deps/llvm-mos), VICE x64sc PAL stock ROMs (tools/vice_run.py)',host_and_sanitizer=checks,
 c64=dict(results=[vice[f'lm-time-free-fall-{n}']for n in prgs],prgs=prgs,steps=sum(v['steps']for v in prgs.values()),branch_outcomes=cover,
  whole_routine_note='The whole routine is too big for one PRG beside two images, so the C64 check runs each phase step in the PRG of its group from a debris-only base state; the host runs the same steps and the whole chain. Whole-routine C64 runs wait for overlay ABI 2.'))
(E/'lm-time-free-fall-results.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS results for',len(prgs),'PRGs,',result['c64']['steps'],'C64 steps')
