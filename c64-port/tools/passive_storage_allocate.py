#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Allocate LM passive files on an unchanged bounded mission base disk."""
import argparse,hashlib,json,os,pathlib,re,shutil,subprocess
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--c1541',default='c1541');a=p.parse_args()
 base=R/'evidence/p66-native.d64';committed=subprocess.check_output(['git','show','HEAD:c64-port/evidence/p66-native.d64'],cwd=R);assert base.read_bytes()==committed
 destination=B/'passive-storage-lm-production.d64';shutil.copyfile(base,destination)
 env=dict(os.environ,XDG_CACHE_HOME=str(B/'passive-storage-cache'),XDG_CONFIG_HOME=str(B/'passive-storage-config'))
 def command(*args):return subprocess.check_output([a.c1541]+list(args),cwd=R,env=env,text=True)
 before=command('-attach',str(base),'-list');names=re.findall(r'^\d+\s+"([^"]+)"\s+prg',before,re.M);assert names and not any(n.startswith('f20') for n in names)
 manifest=json.loads((B/'passive-storage-lm-allocation.json').read_text())['modules']
 args=['-attach',str(destination)]
 for m in manifest:args+=['-write',str(B/m['prg']),m['filename'].lower()]
 command(*args);after=command('-attach',str(destination),'-list');old=[]
 for n in names:
  one=B/('passive-storage-before-'+n+'.prg');two=B/('passive-storage-after-'+n+'.prg')
  for disk,out in ((base,one),(destination,two)):command('-attach',str(disk),'-read',n,str(out))
  assert one.read_bytes()==two.read_bytes();old.append({'filename':n,'prg_sha256':hashlib.sha256(one.read_bytes()).hexdigest()});one.unlink();two.unlink()
 for m in manifest:
  out=B/('passive-storage-readback-'+m['filename']+'.prg');command('-attach',str(destination),'-read',m['filename'].lower(),str(out));assert hashlib.sha256(out.read_bytes()).hexdigest()==m['sha256'];out.unlink()
 metadata={'base_disk':'evidence/p66-native.d64','base_disk_sha256':hashlib.sha256(committed).hexdigest(),'allocated_disk':'build/passive-storage-lm-production.d64','allocated_disk_sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'unchanged_existing_files':old,'base_directory':before,'allocated_directory':after,'free_blocks_before':int(re.search(r'(\d+) blocks free',before)[1]),'free_blocks_after':int(re.search(r'(\d+) blocks free',after)[1]),'passive_files':manifest,'old_registry_unchanged':True,'scope':'Existing bounded startup/P66 proof resident only; passive allocation registers no new entry. F200..205 names are a separate data namespace. Full-descent executable G200..205 IDs collide if merged into one runtime table; unified resolver needs distinct allocation.','new_executable_algorithm_coverage':False}
 (B/'passive-storage-lm-production-allocation.json').write_text(json.dumps(metadata,indent=2)+'\n');print('PASS: old resident/all old files byte-exact, six passive file readbacks byte-exact; blocks',metadata['free_blocks_before'],'->',metadata['free_blocks_after'])
if __name__=='__main__':main()
