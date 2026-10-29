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
 suffix=sys.argv[1] if len(sys.argv)>1 else 'native';bounds=sys.argv[2:] or ['0','786432'];start,end=map(int,bounds)
 source=subprocess.Popen([str(R/'build/sband-scalar-reference'),str(R/'build/sband-source/Luminary099/MAIN.agc.bin'),str(R/'build/sband-source/Comanche055/MAIN.agc.bin'),*bounds],stdout=subprocess.PIPE)
 native=subprocess.Popen([str(R/'build'/('sband-scalar-'+suffix)),*bounds],stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 h=hashlib.sha256();count=0
 try:
  with open(os.devnull,'wb')as unused:
   f=None;chunks=[]
   while True:
    a=exact(source.stdout,4106);b=exact(native.stdout,4106)
    if not a and not b:break
    if a!=b or len(a)!=4106:
     if len(a)==len(b)==4106:
      x,y=struct.unpack('<2053H',a),struct.unpack('<2053H',b);print('DIFF',count,x[0]|x[1]<<16,[(oct(i-5),oct(u),oct(v))for i,(u,v)in enumerate(zip(x,y))if u!=v],flush=True)
     else:print('TRUNCATION',count,len(a),len(b),flush=True)
     return 1
    h.update(a)
    if suffix=='native':
     if count%65536==0:
      if f:f.close()
      path=R/'build'/f'sband-scalar-source-{len(chunks)}.snapshots.bin.xz';chunks.append(str(path.relative_to(R)));f=lzma.open(path,'wb',preset=6)
     f.write(a)
    count+=1
   if f:f.close()
  expected=end-start;assert source.wait()==native.wait()==0 and count==expected
  result=dict(states=count,start=start,end=end,record_bytes=4106,decoded_sha256=h.hexdigest());(R/'build'/('sband-scalar-'+suffix+'-results.json')).write_text(json.dumps(result,indent=2)+'\n');print('PASS',result,flush=True)
 finally:
  for p in (source,native):
   if p.poll()is None:p.terminate()
   p.wait()
 return 0
if __name__=='__main__':raise SystemExit(main())
