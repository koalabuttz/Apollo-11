#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lane G oracle fixture: the byte-original Comanche055 rope with every CM
flight, entry and TVC source file taken from Comanche055/ unchanged except for
yaYUL's offset-label indentation (recorded as a patch per file). The rope must
equal the pinned original CM rope, so every Lane G oracle runs the original
words. Writes deps/cm-flight-fixture and build/cm-flight-source-audit.json."""
import pathlib,shutil,subprocess,hashlib,json,re,difflib,sys
R=pathlib.Path(__file__).resolve().parents[1];base=R/'build/fixed-data/Comanche055';out=R/'deps/cm-flight-fixture'
ROPE='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
FILES=['TVCMASSPROP.agc','TVCEXECUTIVE.agc','TVCRESTARTS.agc','TVCSTROKETEST.agc','TVCINITIALIZE.agc','TVCROLLDAP.agc','TVCDAPS.agc','RCS-CSM_DIGITAL_AUTOPILOT.agc','JET_SELECTION_LOGIC.agc','CM_ENTRY_DIGITAL_AUTOPILOT.agc','CM_BODY_ATTITUDE.agc','P61-P67.agc','SERVICER207.agc','P40-P47.agc','AUTOMATIC_MANEUVERS.agc']
if not (base/'MAIN.agc.bin').exists():
 sys.path.insert(0,str(R/'tools'));from fixed_data_generate import assemble;assemble('Comanche055',R/'build/fixed-data')
out.mkdir(parents=True,exist_ok=True)
for p in base.glob('*.agc'):shutil.copyfile(p,out/p.name)
records={}
for name in FILES:
 p=R.parent/'Comanche055'/name;text=p.read_text();normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',text,flags=re.M);normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M);(out/name).write_text(normalized)
 (out/(name+'.indentation.patch')).write_text(''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile='Comanche055/'+name,tofile='fixture/'+name)))
 records[name]={'original_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'assembled_source_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'indentation_repairs':[i for i,(a,b) in enumerate(zip(text.splitlines(),normalized.splitlines()),1) if a!=b]}
with (out/'assembly.lst').open('w') as f:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
rope=hashlib.sha256((out/'MAIN.agc.bin').read_bytes()).hexdigest()
assert rope==ROPE,('rope differs from the original CM rope',rope)
x={'rope_sha256':rope,'original_sources':records,'declaration_data_domain':'Byte-original CM declarations and data (docs/FIXED_DATA.md)','historical_source_edits':0}
(R/'build').mkdir(exist_ok=True);(R/'build/cm-flight-source-audit.json').write_text(json.dumps(x,indent=2)+'\n');print('cm-flight fixture rope',rope)
