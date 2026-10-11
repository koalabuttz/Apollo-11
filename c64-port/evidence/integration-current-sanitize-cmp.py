#!/usr/bin/env python3
import sys,subprocess,hashlib,json,gzip,lzma,pathlib,os
args=sys.argv[1:]
if len(args)!=2 or any(x.startswith('-')for x in args):raise SystemExit(subprocess.call(['/usr/bin/cmp',*args]))
repo=pathlib.Path('/workspace/Apollo-11/c64-port');work=pathlib.Path.cwd();manifest=json.loads(subprocess.check_output(['git','-C',str(repo),'show','69801cb:c64-port/evidence/SHA256.json']));records=[]
def open_stream(name):
 p=work/name
 if p.exists():return p.open('rb')
 base=pathlib.Path(name).name;normal=lambda s:s.replace('_','-')
 candidates=[n for n in manifest if normal(n)==normal(base+'.gz')or normal(n)==normal(base+'.xz')]
 if not candidates and '-native.'in base:
  ref=base.replace('-native.','-reference.');candidates=[n for n in manifest if normal(n)==normal(ref+'.gz')or normal(n)==normal(ref+'.xz')]
 if not candidates:raise FileNotFoundError('No retained original corpus for '+name)
 if len({manifest[n]for n in candidates})!=1:raise RuntimeError('Ambiguous retained corpus '+name)
 n=candidates[0];f=repo/'evidence'/n;data=f.read_bytes();assert hashlib.sha256(data).hexdigest()==manifest[n],n
 expected=subprocess.check_output(['git','-C',str(repo),'rev-parse','69801cb:c64-port/evidence/'+n],text=True).strip();assert hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()==expected
 records.append({'missing_raw':name,'retained_artifact':n,'artifact_sha256':manifest[n]})
 return gzip.open(f,'rb')if n.endswith('.gz')else lzma.open(f,'rb')
try:
 with open_stream(args[0])as a,open_stream(args[1])as b:
  pos=0;digest=hashlib.sha256()
  while True:
   x=a.read(65536);y=b.read(65536)
   if x!=y:print('DIFF',args,'at chunk',pos,file=sys.stderr);raise SystemExit(1)
   if not x:break
   digest.update(x);pos+=len(x)
 print('PASS retained-stream comparison',args,pos,'bytes',digest.hexdigest(),flush=True)
 record={'paths':args,'bytes':pos,'decoded_sha256':digest.hexdigest(),'retained':records}
 with(pathlib.Path('/tmp/apollo-host-current')/'sanitize-stream-comparisons.jsonl').open('a')as out:out.write(json.dumps(record)+'\n')
 for arg in args:
  f=work/arg
  if 'sanitize' in f.name and f.suffix=='.snapshots'and f.is_file()and f.is_relative_to(pathlib.Path('/tmp/apollo-host-current/c64-port/build')):f.unlink()
except Exception as exc:
 print(type(exc).__name__+': '+str(exc),file=sys.stderr);raise SystemExit(2)
