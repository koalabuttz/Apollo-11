#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map and independent assembly comparison for Comanche055
TVCMASSPROP. Every emitted original word is read from the yaYUL listing of the
unchanged source, checked against the assembled rope bytes, and assigned to
the native construct that implements it; the constant words are compared with
the native operand table (cm_tvc_massprop_fixed). Writes
evidence/cm-tvc-massprop/cm-tvc-massprop-source-map.json."""
import hashlib,json,pathlib,re,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
SRC=R.parent/'Comanche055/TVCMASSPROP.agc';FIX=R/'deps/cm-flight-fixture'
if not (FIX/'assembly.lst').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
rope=(FIX/'MAIN.agc.bin').read_bytes();words=struct.unpack('>%dH'%(len(rope)//2),rope)
listing=(FIX/'assembly.lst').read_text().split('$TVCMASSPROP.agc',1)[1]
GROUPS=[(0o3366,0o3412,'MASSPROP ten-pass breakpoint load (L3366-L3412)'),(0o3413,0o3421,'DXTEST descent-stage adjustment (L3413)'),(0o3422,0o3434,'FIXCW slope selection (L3422-L3433)'),(0o3435,0o3456,'PEGGY outputs IXX/IAVG/IAVG-TLX, WEIGHT/G, ENDMASSP TC Q (L3435-L3456)'),(0o3457,0o3462,'BOKKEP2 (L3457)'),(0o3463,0o3520,'operand table bank05[] / cm_tvc_massprop_fixed')]
out=[]
for line in listing.splitlines():
 if re.match(r'^\d+,\d+: \$',line):break
 m=re.match(r'^(\d+),(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(\s+)(\S+)(?:\s+(\S+))?(?:\s+(\S+))?',line)
 if not m:continue
 bank,addr,raw=int(m[3],8),int(m[4],8),int(m[5],8)
 rw=words[bank*1024+addr-0o2000]>>1
 assert rw==raw,(m[4],oct(rw),m[5])
 group=next(g for lo,hi,g in GROUPS if lo<=addr<=hi)
 out.append({'source_line':int(m[2]),'bank':m[3],'address':m[4],'raw_octal':m[5],**({'label':m[7],'op':m[8] or '','operand':m[9] or ''} if len(m[6])<12 else {'label':'','op':m[7],'operand':m[8] or ''}),'native':group})
assert len(out)==91,len(out)
assert {w['address'] for w in out}=={f'{a:04o}' for a in range(0o3366,0o3521)}
# Native operand table: compile a tiny reader and compare every constant.
exe=R/'build/cm-tvc-massprop-fixed';(R/'build').mkdir(exist_ok=True)
(R/'build/cm_tvc_massprop_fixed_dump.c').write_text('#include <stdio.h>\n#include "cm_tvc_massprop.h"\nint main(void){unsigned a;agc_word w;for(a=03400u;a<03600u;++a)if(cm_tvc_massprop_fixed((uint16_t)a,&w))printf("%o %o\\n",a,w);return 0;}\n')
subprocess.run(['cc','-std=c99','-Iinclude','build/cm_tvc_massprop_fixed_dump.c','src/cm_tvc_massprop.c','src/agc_image_ops.c','src/agc_word.c','src/agc_multiword.c','src/agc_hardware_divide.c','-o',str(exe)],cwd=R,check=True)
table={int(a,8):int(v,8) for a,v in (l.split() for l in subprocess.check_output([str(exe)]).decode().split('\n') if l)}
for a,v in table.items():assert words[5*1024+a-0o2000]>>1==v,(oct(a),oct(v))
assert set(table)=={0o3435}|set(range(0o3462,0o3521))
x={'source_file':'Comanche055/TVCMASSPROP.agc','original_sha256':hashlib.sha256(SRC.read_bytes()).hexdigest(),'rope_sha256':hashlib.sha256(rope).hexdigest(),
 'emitted_words':len(out),'executable_words':61,'data_words':30,'entries':{'MASSPROP':'05,3366','FIXCW':'05,3422'},'return':'ENDMASSP TC Q (05,3456): runs the TC word held in Q, leaving Q = 3',
 'native_files':['src/cm_tvc_massprop.c','include/cm_tvc_massprop.h'],'native_operand_table_words':len(table),
 'reachability':{'TS overflow skips at 3367, 3423, 3424, 3434, 3457':'unreachable: A there is CAF 9/2/NEG2, a CCS result or +-1 from an earlier TS skip, never overflowed; translated anyway',
  'CCS -0 at 3411 and 3446':'unreachable: PHI333/PSI333 are set only from CCS results or +-1',
  'CCS negative/-0 at 3452':'unreachable: A is MASK BIT14 of DAPDATR1',
  'non-returning loops':'TS VARST0 / TS IXX / TS WEIGHT/G overflow detours that revisit a loop head with the same loop state; native reports NO_EXIT where the oracle never reaches the caller return'},
 'words':out}
p=R/'evidence/cm-tvc-massprop/cm-tvc-massprop-source-map.json';p.write_text(json.dumps(x,indent=1)+'\n')
print(f'PASS {len(out)} original TVCMASSPROP words mapped; listing equals rope; {len(table)} native operand words equal the rope')
