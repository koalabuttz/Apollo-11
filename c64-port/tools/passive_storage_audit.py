#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independently compare native passive words with complete original ropes."""
import argparse,gzip,hashlib,json,pathlib,subprocess,os,re
R=pathlib.Path(__file__).resolve().parents[1]
IDENTITIES={'cm':('Comanche055','6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'),'lm':('Luminary099','1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e')}
def words(mission):
 raw=(R/f'evidence/fixed-data-{mission}-original.rope.bin').read_bytes()
 assert len(raw)==73728 and hashlib.sha256(raw).hexdigest()==IDENTITIES[mission][1]
 # Explicit file ordering differs from native physical-bank ordering only
 # for the four low banks. Read whole banks in canonical physical order.
 order=[2,3,0,1]+list(range(4,36))
 result=[]
 for file_bank in order:
  result.extend(int.from_bytes(raw[file_bank*2048+i:file_bank*2048+i+2],'big')//2 for i in range(0,2048,2))
 return result+[0]*4096
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cc',default='cc');a=p.parse_args()
 audit=json.loads(gzip.decompress((R/'evidence/fixed-data-audit.json.gz').read_bytes()));report={}
 for mission,(program,sha) in IDENTITIES.items():
  record=next(x for x in audit['missions'] if x['mission']==program)
  assert record['assembled_rope_sha256']==sha
  for source in record['source_files']:assert hashlib.sha256((R.parent/source['path']).read_bytes()).hexdigest()==source['sha256']
  expected=words(mission);source=R/f'src/{mission}_fixed_compat_words.inc'
  arrays=re.findall(r'static const agc_word \w+\[(\d+)\]=\{(.*?)\};',source.read_text(),re.S)
  actual=[]
  for size,body in arrays:
   chunk=[int(v) for v in re.findall(r'\d+',body)];assert len(chunk)==int(size);actual+=chunk
  assert actual==expected[:36864]
  domains=[]
  for sanitize in (False,True):
   for group in range(-1,6):
    exe=R/f'build/passive-storage-{mission}-{group}-{int(sanitize)}'
    flags=['-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude']
    if mission=='lm':flags+=['-DPASSIVE_STORAGE_LM=1']
    if group>=0:flags+=[f'-D{mission.upper()}_FIXED_COMPAT_GROUP={group}']
    if sanitize:flags+=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']
    subprocess.run([a.cc]+flags+['tests/passive_storage_host.c',f'src/{mission}_fixed_compat.c','-o',str(exe)],cwd=R,check=True)
    result=subprocess.run([str(exe)],cwd=R,check=True,capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')).stdout
    assert len(result)==65536*3
    for address in range(65536):
     valid=address<40960 and (group<0 or address>=36864 or address//7168==group)
     got=result[address*3:address*3+3]
     assert bool(got[0])==valid
     assert int.from_bytes(got[1:],'little')==(expected[address] if valid else 0xdead),(mission,group,address)
    domains.append({'group':group,'sanitized':sanitize,'all_address_results_sha256':hashlib.sha256(result).hexdigest()})
  report[mission]={'source_program':program,'original_rope_sha256':sha,'source_set_sha256':record['source_set_sha256'],'present_words':36864,'all_physical_words':40960,'original_words_exact':True,'source_files_byte_exact':True,'host_and_sanitized_all65536_addresses_each_link_group':domains,'generated_arrays_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'instructions_executed_from_rope':False}
 out=R/'evidence/passive-storage-host-audit.json';out.write_text(json.dumps(report,indent=2)+'\n');print('PASS: both original source sets, all73728 data bytes per mission; full and each group all65536 addresses, null/bounds/output preservation, strict host + ASAN/UBSAN')
if __name__=='__main__':main()
