#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-line original ALARM inventory with exact instruction-token alignment."""
import pathlib, re, hashlib, json
R = pathlib.Path(__file__).resolve().parents[1]
listing = (R/'build/fixed-data/Comanche055/assembly.lst').read_text().splitlines()
assert hashlib.sha256((R/'build/fixed-data/Comanche055/MAIN.agc.bin').read_bytes()).hexdigest() == '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
profiles = {'ALARM_AND_ABORT.agc':[
 (53,106,'alarm record and inline data read','cm-alarm-image'),
 (125,134,'priority alarm lead-in and -2SEC literal','cm-priolarm'),
 (143,154,'BAILOUT snapshot and BORTENT admission','cm-abort-snapshot'),
 (155,161,'WHIMPER-1 and WHIMPER through actual ENEMA','cm-abort-whimper'),
 (166,175,'POODOO snapshot and BORTENT admission','cm-abort-snapshot'),
 (177,195,'POODOO flag/restart recovery','cm-abort-recovery'),
 (196,204,'CCSHOLE snapshot and 1103 literal','cm-abort-snapshot'),
 (205,229,'CURTAINS/VARALARM and aliases','cm-alarm-image')
]}

data_ops = {'OCT','DEC','CADR','BBCON','ADRES','2CADR','-2CADR','GENADR','FCADR'}
layout_ops = {'BLOCK','BANK','SETLOC','EBANK=','COUNT','COUNT*','EQUALS'}
missions = []
for filename, groups in profiles.items():
    source = R.parent/'Comanche055'/filename
    original = source.read_bytes()
    start = next(i for i,x in enumerate(listing) if '$'+filename in x)
    end = next(i for i in range(start+1,len(listing)) if 'End of include-file '+filename in listing[i])
    support=R/'build/fixed-data/Comanche055'/filename
    original_statements=[(i+1,x.split('#',1)[0].split()) for i,x in enumerate(original.decode().splitlines()) if x.split('#',1)[0].split()]
    support_statements=[(i+1,x.split('#',1)[0].split()) for i,x in enumerate(support.read_text().splitlines()) if x.split('#',1)[0].split()]
    assert [x[1] for x in original_statements]==[x[1] for x in support_statements], 'support instruction/declaration tokens differ from byte-original'
    support_to_original={y[0]:x[0] for x,y in zip(original_statements,support_statements)}
    assembled = {}
    for line in listing[start+1:end]:
        m = re.match(r'\d+,(\d+):\s+((?:[0-7]{2},)?[0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?\s',line)
        if m:
            assembled[support_to_original[int(m[1])]] = {'support_source_line':int(m[1]),'address_octal':m[2], 'assembled_words_octal':[x for x in m.groups()[2:] if x]}
    rows = []
    for number, text in enumerate(original.decode().splitlines(),1):
        tokens = text.split('#',1)[0].split()
        row = dict(line=number, original_text=text)
        if number in assembled:
            row.update(assembled[number])
            row['classification'] = 'immutable data' if any(x in data_ops for x in tokens) else 'compiled instruction'
            group = next((x for x in groups if x[0]<=number<=x[1]),None)
            row['native_source_group'] = group[2] if group else 'unmapped'
            row['proof_prefix'] = group[3] if group else None
            row['qualification'] = 'bounded primitive source domains; no complete-file valid-caller certificate'
            if 'NO.CORES' in tokens:
                row['classification'] = 'immutable numeric word with source fault instruction alias'
                row['fault_alias'] = 'DEC6 is an executed TC6 word only after an invalid +0 core-priority CCS path; that fault domain remains open'
        elif tokens:
            row['classification'] = 'assembler directive' if any(x in layout_ops for x in tokens) else 'non-emitting source declaration'
        else:
            row['classification'] = 'comment/blank'
        rows.append(row)
    assert all(row.get('native_source_group') != 'unmapped' for row in rows), [(row['line'], row['original_text']) for row in rows if row.get('native_source_group') == 'unmapped']
    missions.append(dict(file=filename, original_sha256=hashlib.sha256(original).hexdigest(),
                         source_lines=len(rows), support_sha256=hashlib.sha256(support.read_bytes()).hexdigest(), support_code_tokens_exact=True, source_statement_count=len(original_statements), emitted_source_lines=len(assembled),
                         bounded_proof_files=[{'path':'evidence/'+prefix+'-proof.json','sha256':hashlib.sha256((R/'evidence'/(prefix+'-proof.json')).read_bytes()).hexdigest()} for prefix in sorted(set(g[3] for g in groups)) if (R/'evidence'/(prefix+'-proof.json')).exists()],
                         emitted_words=sum(len(x['assembled_words_octal']) for x in assembled.values()),
                         lines=rows))
report = dict(mission='Comanche055', original_layout_rope_sha256='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061',
              complete_file_qualified=False, files=missions,
              open_domains=['normal-code and compiled caller contracts beyond the source corpus; no raw overflow VARALARM/BORTENT reserved-register execution claim',
                            'raw/high Q/index/return caller closure beyond each explicit compiled ABI',
                            'all actual application/task bodies selected by these services and a final resident registry',
                            'arbitrary interrupt timing outside certified compiled source safepoints',
                            'source state combinations outside each retained corpus/predicate'],
              scope='whole-line/instruction/data/directive inventory only; bounded helper presence does not imply full original file qualification')
report['whimper_minus_one_producers']=[{'file':'ALARM_AND_ABORT.agc','line':181,'address_octal':'5735','native_group':'cm-abort-recovery phase0','contract':'actual original masked V37FLBIT inFLAGWRD7 nonzero; source TC producesQ5736 then WHIMPER-1 executesINHINT; uninterrupted lifecycle proof includesbothflagbranches'}]
report['literal_return_arcs']=[{'literal':'-2SEC','address_octal':'10,3774','word_octal':'77467','real_return':'PRIOENT+1 viaMULTEXIT indexedTC1→3775','role':'original immutable -200 numeric data skipped by genuine return'}, {'literal':'OCT40400','address_octal':'7765','word_octal':'40400','real_return':'BORTENT viaMULTEXIT indexedTC1→WHIMPER-1 at7766','role':'original lamp mask data skipped by genuine return'}, {'literal':'OCT77770','address_octal':'5731','word_octal':'77770','real_return':'BORTENT viaMULTEXIT indexedTC1→5732','role':'original flag mask data skipped by genuine return'}, {'literal':'OCT217','address_octal':'5762','word_octal':'00217','real_return':'ALARM2 viaMULTEXIT indexedTC1→TCALMCADR5763','role':'original CURTAINS alarm code skipped by genuine return'}, {'literal':'OCT14','address_octal':'5771','word_octal':'00014','real_return':'PRIOENT viaMULTEXIT indexedTC1→TCALMCADR5772','role':'original bit3+4 constant skipped by genuine return'}]
callers=[]
entries={'ALARM','ALARM2','VARALARM','CURTAINS','BORTENT','PRIOENT','PRIOLARM','BAILOUT','ABORT','POODOO','CCSHOLE','WHIMPER','DOALARM'}
for source in sorted((R.parent/'Comanche055').glob('*.agc')):
    lines=source.read_text().splitlines()
    for number,text in enumerate(lines,1):
        tokens=text.split('#',1)[0].split()
        for i,token in enumerate(tokens[:-1]):
            if token in {'TC','TCF','CADR','2CADR'} and tokens[i+1] in entries:
                callers.append({'source_file':source.name,'original_line':number,'statement':' '.join(tokens),'target':tokens[i+1],'preceding_original_lines':lines[max(0,number-5):number-1],'qualification':'source producer inventory only; actual application/body acceptance and caller contract independently required'})
report['original_direct_callers']=callers
report['caller_inventory_source_set_sha256']=hashlib.sha256(b''.join(source.name.encode()+b'\0'+source.read_bytes() for source in sorted((R.parent/'Comanche055').glob('*.agc')))).hexdigest()
(R/'build/cm-alarm-image-source-audit.json').write_text(json.dumps(report,indent=2)+'\n')
print('Mapped all original ALARM_AND_ABORT emitting lines; complete-file qualification FALSE')
for file in missions:
    print(file['file'],file['emitted_words'],'words',file['source_lines'],'original lines')
