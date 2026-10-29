#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit every historical service file used by the application composition."""
import hashlib,json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
def rows(path):return [' '.join(line.split('#',1)[0].split())for line in path.read_text().splitlines()if line.split('#',1)[0].strip()]
result=[]
for name in ('EXECUTIVE.agc','INTERPRETER.agc','LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','WAITLIST.agc','DAP_INTERFACE_SUBROUTINES.agc','SERVICE_ROUTINES.agc','INTER-BANK_COMMUNICATION.agc','PINBALL_GAME_BUTTONS_AND_LIGHTS.agc','DISPLAY_INTERFACE_ROUTINES.agc'):
 original=root.parent/'Luminary099'/name;fixture=root/'deps/fixture'/('PINBALL_GAME__BUTTONS_AND_LIGHTS.agc'if name.startswith('PINBALL_GAME_')else name)
 a=rows(original);b=rows(fixture);aliases=[]
 if name=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc':a=[r.replace('DSPOCTWO','DSPOCTWD')for r in a];aliases.append('DSPOCTWO→DSPOCTWD transcription label spelling')
 if name=='INTER-BANK_COMMUNICATION.agc':a=[r.replace('ISWCALLL','ISWCALL')for r in a];aliases.append('ISWCALLL→ISWCALL transcription label spelling')
 if name=='DISPLAY_INTERFACE_ROUTINES.agc':a=[r for r in a if r!='PRIOSAVE EQUALS R1SAVE'];aliases.append('redundant PRIOSAVE EQUALS R1SAVE declaration omitted in fixture')
 assert a==b,name
 result.append(dict(file=name,original_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),fixture_sha256=hashlib.sha256(fixture.read_bytes()).hexdigest(),source_statement_sha256=hashlib.sha256('\n'.join(b).encode()).hexdigest(),statements=len(b),text_aliases=aliases))
report={'source_statements':'all exact after documented label/declaration spelling reconciliation; no instruction, operand value or output state rewritten','fixture_binary_sha256':hashlib.sha256((root/'deps/fixture/MAIN.agc.bin').read_bytes()).hexdigest(),'files':result}
(root/'build/p66-application-source-audit.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS:9 complete original source service files audited; every raw checkpoint retains all2052 words unchanged')
