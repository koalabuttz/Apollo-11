#!/usr/bin/env python3
"""Exact online replay of source-produced full-state streams."""
import hashlib,json,lzma,pathlib,subprocess,sys
from service_routines_compare import exact,RECORD
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 name=sys.argv[1]
 process=subprocess.Popen([str(R/'build'/name)],stdout=subprocess.PIPE)
 digest=hashlib.sha256();count=0
 try:
  for corpus in ('service-routines-native',):
   with lzma.open(R/'build'/(corpus+'.snapshots.bin.xz'),'rb') as source:
    while block:=exact(source,RECORD):
     actual=exact(process.stdout,RECORD)
     if block!=actual:raise RuntimeError('full-state mismatch at '+str(count))
     digest.update(block);count+=1
     if count%65536==0:print('PASS',name,count,'complete source states',flush=True)
  if process.stdout.read(1) or process.wait() or count!=106496:raise RuntimeError('truncated/extra/failing run')
  result=dict(binary=name,states=count,record_bytes=RECORD,decoded_sha256=digest.hexdigest())
  (R/'build'/(name+'-frozen-results.json')).write_text(json.dumps(result,indent=2)+'\n')
  print(json.dumps(result),flush=True)
 finally:
  if process.poll() is None:process.terminate()
  process.wait()
if __name__=='__main__':main()
