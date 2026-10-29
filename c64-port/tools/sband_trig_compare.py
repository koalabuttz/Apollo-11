#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,lzma,os,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
def exact(stream,n):
 data=bytearray()
 while len(data)<n:
  part=stream.read(n-len(data))
  if not part:break
  data.extend(part)
 return bytes(data)
def main():
 suffix=sys.argv[1] if len(sys.argv)>1 else 'native';limit=int(sys.argv[2]) if len(sys.argv)>2 else 131072
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  source=subprocess.Popen([str(R/'build/sband-trig-reference'),str(R/'build/sband-source'/name/'MAIN.agc.bin'),mission,str(limit)],stdout=subprocess.PIPE)
  native=subprocess.Popen([str(R/'build'/('sband-trig-'+suffix)),mission,str(limit)],stdout=subprocess.PIPE,env=env)
  h=hashlib.sha256();count=0
  try:
   with lzma.open(R/'build'/('sband-trig-'+mission+'-'+suffix+'.snapshots.bin.xz'),'wb',preset=6) as f:
    while True:
     a=exact(source.stdout,4106);b=exact(native.stdout,4106)
     if not a and not b:break
     if a!=b or len(a)!=4106:
      if len(a)==len(b)==4106:
       x,y=struct.unpack('<2053H',a),struct.unpack('<2053H',b);print('DIFF',mission,count,[(oct(i-5),oct(u),oct(v))for i,(u,v)in enumerate(zip(x,y))if u!=v],flush=True)
      else:print('TRUNCATION',count,len(a),len(b),flush=True)
      return 1
     h.update(a);f.write(a);count+=1
   if source.wait() or native.wait() or count!=limit:raise RuntimeError('missing states')
   result=dict(mission=mission,states=count,record_bytes=4106,decoded_sha256=h.hexdigest());(R/'build'/('sband-trig-'+mission+'-'+suffix+'-results.json')).write_text(json.dumps(result,indent=2)+'\n');print('PASS',result,flush=True)
  finally:
   for p in (source,native):
    if p.poll() is None:p.terminate()
    p.wait()
 return 0
if __name__=='__main__':raise SystemExit(main())
