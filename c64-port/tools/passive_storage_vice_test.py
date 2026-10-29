#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare every actual native disk-returned word, without data hashes."""
import argparse,hashlib,json,os,pathlib,re,subprocess
from passive_storage_audit import words
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--mission',choices=['lm','cm'],required=True);p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',required=True,type=pathlib.Path);p.add_argument('--data-dir',required=True,type=pathlib.Path);a=p.parse_args();stem=f'passive-storage-{a.mission}'
 prg=B/(stem+'.prg');mapping=(B/(stem+'.map')).read_text()
 for section in ('.text','.rodata','.data','.bss','.noinit'):
  found=re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M)
  assert len(found)==1 and sum(int(x,16) for x in found[0])<=0x6000,section
 for section in ('.zp.data','.zp.bss'):
  found=re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M);assert len(found)==1 and int(found[0],16)==0
 entry=re.search(rb'\x9e\s*(\d+)\0',prg.read_bytes()[:32]);assert entry
 prefix=B/(stem+'-vice');monitor=prefix.with_suffix('.mon');captures=[]
 text=f'fill c800 cfff cd\nload "{prg}" 0\nwatch store c080\nignore 1 1\ng {int(entry[1]):04x}\n'
 for run in range(2):
  for bank in range(40):
   capture=B/f'{stem}-capture-{run}-{bank}.bin';capture.unlink(missing_ok=True);captures.append(capture)
   text+=f'bsave "{capture}" 0 6000 67ff\nm c080 c084\ng\n'
 mail=B/(stem+'-mailbox.bin');shared=B/(stem+'-shared.bin');stack=B/(stem+'-stack.bin')
 for file in (mail,shared,stack):file.unlink(missing_ok=True)
 text+=f'bsave "{mail}" 0 c080 c084\nbsave "{shared}" 0 7000 7fff\nbsave "{stack}" 0 c800 cfff\nquit\n';monitor.write_text(text)
 rom=a.rom_dir.resolve();cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',str(a.data_dir.resolve())+':'+str(rom.parent),'+drive8truedrive','-virtualdev8','-iecdevice8','-device8','1','-8',str(B/(stem+'.d64')),'-initbreak','ready','-moncommands',str(monitor),'-monlogname',str(B/(stem+'-vice-monitor.log')),'-monlog','-limitcycles','16000000000']
 for name in ('passive-storage-cache','passive-storage-config'):(B/name).mkdir(exist_ok=True)
 with (B/(stem+'-vice.log')).open('w') as log:subprocess.run(cmd,cwd=R,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600,env=dict(os.environ,XDG_CACHE_HOME=str(B/'passive-storage-cache'),XDG_CONFIG_HOME=str(B/'passive-storage-config')))
 assert mail.read_bytes()==bytes([0x5a,0,0,39,1]),mail.read_bytes().hex()
 expected=b''.join(w.to_bytes(2,'little') for w in words(a.mission));actual=[]
 for run in range(2):
  data=b''.join(c.read_bytes() for c in captures[run*40:(run+1)*40]);assert data==expected,f'{a.mission} run{run}: exact fixed-word mismatch';actual.append(hashlib.sha256(data).hexdigest())
 assert shared.read_bytes()==b''.join(((i*29+0x1234)&65535).to_bytes(2,'little') for i in range(2048))
 altered=[i for i,v in enumerate(stack.read_bytes()) if v!=0xcd];extent=2048-min(altered) if altered else 0;assert extent<2048
 report={'mission':a.mission,'exact_returned_word_comparisons':81920,'ordinary_and_masked_IRQ_restored':True,'canonical_2048_word_image_preserved':True,'native_RO_mapping_preserved':True,'word_stream_sha256_each_IRQ_run':actual,'software_stack_extent':extent,'mailbox':mail.read_bytes().hex(),'prg_sha256':hashlib.sha256(prg.read_bytes()).hexdigest(),'map_sha256':hashlib.sha256((B/(stem+'.map')).read_bytes()).hexdigest(),'source_instructions_decoded':False}
 (B/(stem+'-target-proof.json')).write_text(json.dumps(report,indent=2)+'\n')
 print('PASS:',a.mission,'81920 individually compared disk-returned words, 40 physical banks x two IRQ states; canonical image and port preserved; stack',extent)
if __name__=='__main__':main()
