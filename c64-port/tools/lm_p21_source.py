#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P21 (GROUND_TRACKING_DETERMINATION_PROGRAM) transfer routines
(src/lm_p21_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_p76_source.py does) and requires the rope
to equal the certified fixed-data rope. Every assembled word of the file
(bank 24 3505-3667) must then be owned by exactly one registered routine
entered at the first address of its range. Interpretive words must appear as
literals, in source order, in that routine (with the shared bodies it runs
on into); basic words belong to the routine whose ENTRY names that address;
the constants after the code belong to the routines whose literals read
them; the erasable word and destinations the code names must equal their
native macros in the fresh assembly; and the fixed constants it reads (ONE,
BIT2, V6N34, V06N43, the K.01 and 600SEC pairs) must equal their native
literals in the fresh rope. Every routine must own at least one source
line. Writes build/lm-p21-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'GROUND_TRACKING_DETERMINATION_PROGRAM.agc':'GROUND_TRACKING_DETERMINATION_PROGRAM_-_P21.agc'}
root=R/'build/lm-p21-source';assemble(mission,root);work=root/mission
includes=[line[1:].split()[0]for line in(work/'MAIN.agc').read_text().splitlines()if line.startswith('$')]
aliases={};texts={};originals={}
for name,alias in FILES.items():
 path=mission+'/'+name;original=R.parent/path
 if alias not in includes:raise SystemExit('FAIL no include '+alias)
 shutil.copyfile(original,work/alias);aliases[alias]=path;texts[path]=original.read_text();originals[path]=original
shutil.copyfile(R/'deps/lm-rtb-fixture/INTERPRETER.agc',work/'INTERPRETER.agc')
with(work/'assembly.lst').open('w')as log:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(work/'MAIN.agc.bin').read_bytes()
if rope!=(R/'evidence/fixed-data-lm-original.rope.bin').read_bytes():raise SystemExit('FAIL fresh rope differs from the certified fixed-data rope')
records=listing_records(work/'assembly.lst',aliases,texts,rope)
for path,text in texts.items():
 if {r['line']for r in records if r['path']==path}!={i for i,line in enumerate(text.splitlines(),1)if source_line(line)is not None}:raise SystemExit('FAIL listing does not cover every source line of '+path)
# (first, last, routine, kind) bank-24 ranges: kind 1 interpretive, 0
# basic, 2 a constant (named) whose words must equal the native literals
# DATA lists. Words a basic call carries (CADR) belong to its caller.
OWNERS=[(0o3505,0o3506,'prog21',0),(0o3507,0o3511,'r_perf4',0),(0o3512,0o3512,'perf4_terminate',0),(0o3513,0o3513,'perf4_proceed',0),
 (0o3514,0o3514,'perf4_enter',0),(0o3515,0o3517,'p21prog1',0),(0o3520,0o3520,'f34_terminate',0),(0o3521,0o3521,'f34_proceed',0),
 (0o3522,0o3522,'f34_recycle',0),(0o3523,0o3523,'r_intpret',0),(0o3524,0o3527,'p21_tdec1',1),(0o3530,0o3540,'p21_flags',1),
 (0o3541,0o3545,'p21_integrv',1),(0o3546,0o3547,'integrv_ret',1),(0o3550,0o3570,'p21cont',1),(0o3571,0o3607,'p21vsave',1),
 (0o3610,0o3624,'arcsin_ret',1),(0o3625,0o3641,'p21dsp',1),(0o3642,0o3645,'latlong_ret',1),(0o3646,0o3650,'v06n43',0),
 (0o3651,0o3651,'f43_terminate',0),(0o3652,0o3652,'f43_proceed',0),(0o3653,0o3653,'f43_recycle',0),(0o3654,0o3661,'p21_next',1),
 (0o3662,0o3663,'600SEC',2),(0o3664,0o3664,'V06N43',2),(0o3665,0o3665,'V6N34',2),(0o3666,0o3667,'K.01',2)]
HELPERS={'r_perf4':['perf4'],'prog21':['perf4'],'perf4_enter':['perf4'],'perf4_proceed':['flash34'],'p21prog1':['flash34'],'f34_recycle':['flash34'],
 'p21_flags':['p21_integrv_body'],'p21_integrv':['p21_integrv_body'],'arcsin_ret':['p21dsp_body'],'p21dsp':['p21dsp_body']}
# Constant words: the native literal names that hold them.
DATA={'600SEC':['K_600SEC_HI','K_600SEC_LO'],'V06N43':['K_V06N43'],'V6N34':['K_V6N34'],'K.01':['K_K01_HI','K_K01_LO']}
NAMED={'OPTION2':'OPTION2','BANKCALL':'BANKCALL','GOTOPOOH':'GOTOPOOH','INTPRET':'INTPRET','ARCSIN':'ARCSIN'}
CONSTANTS={'K_ONE':('ONE',0),'K_BIT2':('BIT2',0),'K_V6N34':('V6N34',0),'K_V06N43':('V06N43',0),'K_K01_HI':('K.01',0),'K_K01_LO':('K.01',1),
 'K_600SEC_HI':('600SEC',0),'K_600SEC_LO':('600SEC',1)}
native=(R/'src/lm_p21_routines.c').read_text()
def body(name):
 m=re.search(r'^(?:INTERP\('+re.escape(name)+r',(0[0-7]+)u\)|ROUTINE\('+re.escape(name)+r'\)\{|static (?:void|uint8_t|enum exec_status) '+re.escape(name)+r'\([^)]*\)\{)',native,re.M)
 if not m:raise SystemExit('FAIL no native routine '+name)
 # INTERP opens its brace inside the macro; the others at the match end.
 depth=1;i=m.end()
 while depth:
  depth+=native[i]=='{';depth-=native[i]=='}';i+=1
 return (int(m.group(1),8)if m.group(1)else None),native[m.start():i]
def literals(text):return{int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',text)}
def table(name):return re.search(r'static const agc_routine_entry '+name+r'\[\]=\{(.*?)\};',native,re.S).group(1)
registered=dict((int(a,8),f)for a,f in re.findall(r'LM_P21_B24\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
rows=[];owned=set()
def macro_value(name):
 m=re.search(r'#define '+re.escape(name)+r' ([0-7]+)u',native)
 if not m:raise SystemExit('FAIL no native literal '+name)
 return int(m.group(1),8)
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 if bank!='24'or not any(lo<=a<=hi for lo,hi,_,_ in OWNERS):continue
 owner=[(lo,o,i)for lo,hi,o,i in OWNERS if lo<=a<=hi]
 if len(owner)!=1:raise SystemExit(f'FAIL address {addr} has no single native owner')
 lo,name,kind=owner[0]
 if kind==2:
  macros=DATA[name][a-lo:a-lo+len(r['words'])]
  if [macro_value(m)for m in macros]!=r['words']:raise SystemExit(f'FAIL 24,{addr} ({name}) differs from native {macros}')
  rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=' '.join(macros)));continue
 entry,text=body(name)
 text+=''.join(body(h)[1]for h in HELPERS.get(name,[]))
 if registered.get(lo)!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at 24,{lo:o}')
 if kind:
  if entry!=lo:raise SystemExit(f'FAIL {name} is not keyed at 24,{lo:o}')
  for w in r['words']:
   if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
 elif not re.search(r'ENTRY\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} does not enter at 24,{lo:o}')
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
# Interpretive words must also appear in source order: each routine's
# assembled words are a subsequence of its literals as written.
for lo,hi,name,interp in OWNERS:
 if interp!=1:continue
 seq=[int(x,8)for x in re.findall(r'\b(0[0-7]*)u?\b',body(name)[1]+''.join(body(h)[1]for h in HELPERS.get(name,[])))];i=0
 for r in rows:
  if r['native']!=name:continue
  for w in r['words']:
   w=int(w,8)
   while i<len(seq)and seq[i]!=w:i+=1
   if i==len(seq):raise SystemExit(f"FAIL {name}: word {w:05o} (line {r['line']}) is out of source order in the native routine")
   i+=1
s=symbols((work/'assembly.lst').read_text())
def physical(text):
 if ','in text:b,o=text.split(',');return int(b,8)*1024+int(o,8)-0o2000
 a=int(text,8);return (a>>10)*1024+(a&0o1777)
for macro,name in NAMED.items():
 m=re.search(r'(?:#define '+macro+r' |[{,]'+macro+r'=)(0[0-7]*)u?\b',native);t=s[name];off=int(t.split(',')[-1],8)
 if not m or int(m.group(1),8)!=off or (','in t and t.split(',')[0]not in('24','00')):raise SystemExit(f'FAIL {name} ({t}) differs from native {macro}')
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' ([0-7]+)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('PROG21','P21PROG1','P21CONT','P21VSAVE','P21DSP','600SEC','SURFFLAG','V06N43','V6N34','K.01','GOPERF4','GOFLASH','INTSTALL','INTEGRV','INTEGRVS','LAT-LONG','DSPTEM1','TDEC1','P21FLAG','VINTFLAG','DIM0FLAG','INTYPFLG','MOONFLAG','LUNAFLAG','ERADFLAG','P21TIME','P21BASER','P21BASEV','P21ORIG','P21VEL','P21GAM','P21ALT','ALPHAV',*NAMED.values(),'ONE','BIT2')
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p21_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['24,3505-24,3667'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of GROUND_TRACKING_DETERMINATION_PROGRAM.agc (P21) is owned by the registered routine entered at the first address of its range or is a constant equal to its native literals: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the erasable word and destinations the code names equal their native macros, and ONE, BIT2, V6N34, V06N43 and the K.01 and 600SEC pairs equal their native literals.')
(R/'build/lm-p21-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P21 words on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
