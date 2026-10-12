#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for the
Luminary099 P32/P72, ALMXITA and ALMXIT transfer routines
(src/lm_p32_routines.c).

Reassembles unchanged Luminary099 with the pinned yaYUL (the fixture
INTERPRETER, as tools/lm_vn1645_source.py does) and requires the rope to equal
the certified fixed-data rope. Every assembled word of P32-P35_P72-P75.agc in
bank 35 2027-2215 must then be owned by exactly one registered routine
entered at the first address of its range. Interpretive words must appear as
literals, in source order, in that routine (with the shared bodies it runs
on into); basic words belong to the routine whose ENTRY names that address;
the erasable words and destinations the code names must equal their native
macros in the fresh assembly; and the fixed constants it reads (V06N11,
V05N09, V06N55, V06N37, V06N75, the P30ZERO, 60MIN, TWOPI and DPPOSMAX
pairs and the seven ALARM/TB words) must equal their native literals in
the fresh rope. Every routine must own at least one source line. Writes
build/lm-p32-source-map.json."""
import hashlib,json,pathlib,re,shutil,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from fixed_data_generate import assemble,listing_records,rope_word,source_line,symbols
mission='Luminary099'
FILES={'P32-P35_P72-P75.agc':'P32-P35,_P72-P75.agc'}
root=R/'build/lm-p32-source';assemble(mission,root);work=root/mission
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
# (bank, first, last, routine, kind) ranges: kind 1 interpretive, 0 basic.
# Words a basic call carries (ADRES, OCT, CADR) belong to its caller.
OWNERS=[('35',0o2027,0o2027,'p32',0),('35',0o2030,0o2030,'avflaga_ret',0),('35',0o2031,0o2031,'p72',0),('35',0o2032,0o2035,'p32strt',0),
 ('35',0o2036,0o2037,'almxita',1),('35',0o2040,0o2043,'almxit',1),('35',0o2044,0o2045,'almxit_exit',0),('35',0o2046,0o2050,'varalarm_ret',0),
 ('35',0o2051,0o2051,'flash_terminate',0),('35',0o2052,0o2052,'flash_recycle',0),('35',0o2053,0o2053,'p32a',0),('35',0o2054,0o2057,'p20_ret',0),
 ('35',0o2060,0o2061,'vn0611',0),('35',0o2062,0o2062,'vnpooh_ret',0),('35',0o2063,0o2071,'tcsi_check',1),('35',0o2072,0o2105,'precset_ret',1),
 ('35',0o2106,0o2111,'timerad_ret',1),('35',0o2112,0o2112,'tcsi_exit',0),('35',0o2113,0o2113,'vn0655',1),('35',0o2114,0o2116,'vn0655_exit',0),
 ('35',0o2117,0o2117,'flash_terminate2',0),('35',0o2120,0o2120,'flash_proceed2',0),('35',0o2121,0o2121,'flash_recycle2',0),('35',0o2122,0o2123,'v06n37',0),
 ('35',0o2124,0o2124,'vnpooh_ret2',0),('35',0o2125,0o2130,'tcsi_tig',1),('35',0o2131,0o2132,'p32b',1),('35',0o2133,0o2145,'advance_ret',1),
 ('35',0o2146,0o2147,'intint_ret',1),('35',0o2150,0o2151,'passive_ret',1),('35',0o2152,0o2155,'p32c',1),('35',0o2156,0o2157,'p32d',1),
 ('35',0o2160,0o2165,'p32e',1),('35',0o2166,0o2172,'p32f',1),('35',0o2173,0o2174,'v06n75',0),('35',0o2175,0o2175,'vnpooh_ret3',0),
 ('35',0o2176,0o2200,'vnpooh_ret_i',1),('35',0o2201,0o2207,'s3233_ret',1),('35',0o2210,0o2213,'disdvlvc_ret',1),('35',0o2214,0o2215,'loop',1)]
HELPERS={'p32c':['p32d_body','p32e_body','p32f_body'],'p32d':['p32d_body','p32e_body','p32f_body'],'p32e':['p32e_body','p32f_body'],'p32f':['p32f_body']}
DISPLAY={}
NAMED={'AVFLAGA':'AVFLAGA','AVFLAGP':'AVFLAGP','P20FLGON':'P20FLGON','VNPOOH':'VNPOOH','VARALARM':'VARALARM','BANKCALL':'BANKCALL','GOTOPOOH':'GOTOPOOH','INTPRET':'INTPRET','MPAC':'MPAC','CENTANG':'CENTANG','TCSI':'TCSI'}
# Erasable words named at an offset (NN +1) or by their physical E7 index.
OFFSET={'NN1':('NN',1)}
PHYSICAL={'CSIALRM':'CSIALRM'}
CONSTANTS={'K_P30ZERO_HI':('P30ZERO',0),'K_P30ZERO_LO':('P30ZERO',1),'K_V06N11':('V06N11',0),'K_V05N09':('V05N09',0),'K_V06N55':('V06N55',0),
 'K_V06N37':('V06N37',0),'K_V06N75':('V06N75',0),'K_60MIN_HI':('60MIN',0),'K_60MIN_LO':('60MIN',1),'K_TWOPI_HI':('TWOPI',0),'K_TWOPI_LO':('TWOPI',1),
 'K_DPPOSMAX_HI':('DPPOSMAX',0),'K_DPPOSMAX_LO':('DPPOSMAX',1),**{f'K_ALARMTB{k+1}':('ALARM/TB',k)for k in range(7)}}
native=(R/'src/lm_p32_routines.c').read_text()
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
registered=dict(((b,int(a,8)),f)for b,a,f in re.findall(r'LM_P32_B(35)\((0[0-7]+)\),(\w+)\}',table('interpretive')+table('basic')))
rows=[];owned=set()
for r in records:
 if not r['words']:continue
 bank,addr=r['assembled_address'].split(',');a=int(addr,8)
 owner=[(lo,o,i)for b,lo,hi,o,i in OWNERS if b==bank and lo<=a<=hi]
 if not owner:continue
 if len(owner)!=1:raise SystemExit(f'FAIL address {bank},{addr} has no single native owner')
 lo,name,interp=owner[0];entry,text=body(name)
 text+=''.join(body(h)[1]for h in HELPERS.get(name,[]))
 if interp==2:
  m=re.search(r'#define '+DISPLAY[name]+r' (0[0-7]*)u',native)
  if DISPLAY[name]not in text or not m or r['words']!=[int(m.group(1),8)]:raise SystemExit(f'FAIL {bank},{addr} is not the constant {name} reads')
 else:
  if registered.get((bank,lo))!=name:raise SystemExit(f'FAIL {name} is not the registered routine entered at {bank},{lo:o}')
  if interp:
   if entry!=lo:raise SystemExit(f'FAIL {name} is not keyed at {bank},{lo:o}')
   for w in r['words']:
    if w not in literals(text):raise SystemExit(f"FAIL {r['path']} line {r['line']} ({r['op']} {r['arguments']}) word {w:05o} is not a literal of {name}")
  elif not re.search(r'ENTRY\(0?%ou\)'%lo,text):raise SystemExit(f'FAIL {name} does not enter at {bank},{lo:o}')
 owned.add(name);rows.append(dict(path=r['path'],line=r['line'],address=r['assembled_address'],source=(r['label']+' '+r['op']+' '+r['arguments']).strip(),words=[f'{w:05o}'for w in r['words']],native=name))
# Interpretive words must also appear in source order: each routine's
# assembled words are a subsequence of its literals as written.
for b,lo,hi,name,interp in OWNERS:
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
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native);t=s[name];off=int(t.split(',')[-1],8)
 if not m or int(m.group(1),8)!=off or (','in t and t.split(',')[0]not in('35','E7')):raise SystemExit(f'FAIL {name} ({t}) differs from native {macro}')
for macro,(name,k) in OFFSET.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native);t=s[name]
 if not m or not t.startswith('E7,')or int(m.group(1),8)!=int(t[3:],8)+k:raise SystemExit(f'FAIL {name} +{k} ({t}) differs from native {macro}')
for macro,name in PHYSICAL.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native);t=s[name]
 if not m or not t.startswith('E7,')or int(m.group(1),8)!=7*256+int(t[3:],8)-0o1400:raise SystemExit(f'FAIL {name} ({t}) differs from native physical {macro}')
for macro,(name,off) in CONSTANTS.items():
 m=re.search(r'#define '+macro+r' (0[0-7]*)u',native)
 if not m or int(m.group(1),8)!=rope_word(rope,physical(s[name])+off):raise SystemExit(f'FAIL fixed constant {name} ({s[name]}) differs from native {macro}')
missing=set(registered.values())-owned
if missing:raise SystemExit('FAIL native routines own no source: '+', '.join(sorted(missing)))
KEYS=('P32','P72','P32STRT','P32/P72A','P32/P72B','P32/P72C','P32/P72D','P32/P72E','P32/P72F','VN0611','VN0655','ALMXITA','ALMXIT','ALARM/TB','PRECSET','TIMERAD','SELECTMU','ADVANCE','INTINT','PASSIVE','CSI/A','S32/33.1','DISDVLVC','VN1645','GOFLASH','TETLEM','TDEC1','TDEC2','RACT3','VACT3','RVEC','VVEC','RVSW','RDESIRED','TIG','VPASS1','RPASS1','TTPI','TTPI0','T1TOT2','T2TOT3','DELVEET1','DELVEET2','RACT1','RACT2','FINALFLG','UPDATFLG','NN','CSIALRM',*NAMED.values(),'P30ZERO','V06N11','V05N09','V06N55','V06N37','V06N75','60MIN','TWOPI','DPPOSMAX')
result=dict(source_paths=list(texts),source_sha256={p:hashlib.sha256(originals[p].read_bytes()).hexdigest()for p in texts},rope_sha256=hashlib.sha256(rope).hexdigest(),native_path='src/lm_p32_routines.c',native_sha256=hashlib.sha256(native.encode()).hexdigest(),
 source_words=sum(len(r['words'])for r in rows),source_lines=len(rows),symbols={k:s[k]for k in KEYS if k in s},line_map=rows,
 covered_ranges=['35,2027-35,2215'],
 note='Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every word of P32-P35_P72-P75.agc in 35,2027-2215 (P32/P72, ALMXITA, ALMXIT) is owned by the registered routine entered at the first address of its range: interpretive words are literal bookkeeping arguments in source order, basic words belong to the routine whose ENTRY names the address, the erasable words and destinations the code names equal their native macros, and V06N11, V05N09, V06N55, V06N37, V06N75, the P30ZERO, 60MIN, TWOPI and DPPOSMAX pairs and the ALARM/TB table equal their native literals.')
(R/'build/lm-p32-source-map.json').write_text(json.dumps(result,indent=1)+'\n')
print('PASS',result['source_words'],'assembled P32-P35_P72-P75 words (35,2027-2215) on',len(rows),'source lines map to',len(owned),'native routines',flush=True)
