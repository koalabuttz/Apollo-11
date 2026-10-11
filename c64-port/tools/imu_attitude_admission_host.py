#!/usr/bin/env python3
"""Stream original/native canonical IMU transfer states; no raw retained duplicates."""
import argparse, hashlib, json, lzma, os, subprocess
from pathlib import Path

def check_stream(a,b,out=None):
    h=hashlib.sha256();count=0
    while True:
        x=a.read(65536);y=b.read(len(x) if x else 1)
        if x!=y:raise RuntimeError(f"state bytes differ at block starting {count}")
        if not x:break
        h.update(x);count+=len(x)
        if out:out.write(x)
    return count,h.hexdigest()

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--generate',action='store_true');ap.add_argument('--sanitize',action='store_true');ap.add_argument('--count',type=int,default=65536);a=ap.parse_args()
    rows=[];env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
    for mission in range(2):
        for phase in range(5):
            name=f'imu-attitude-admission-{mission}-{phase}';archive=Path('build')/(name+'.snapshots.xz')
            if not a.generate and not archive.exists():archive=Path('evidence')/f'imu-attitude-admission-{"cm" if mission else "lm"}{phase}-complete.states.xz'
            native='build/imu-attitude-admission-native-san' if a.sanitize else 'build/imu-attitude-admission-native'
            b=subprocess.Popen([native,str(mission),str(phase),str(a.count)],stdout=subprocess.PIPE,env=env)
            if a.generate:
                source=subprocess.Popen(['build/imu-attitude-admission-reference',f'deps/{"cm" if mission else "lm"}-imu-pulse-fixture/MAIN.agc.bin',str(mission),str(phase),str(a.count)],stdout=subprocess.PIPE)
                with lzma.open(archive,'wb',preset=3) as out:size,sha=check_stream(source.stdout,b.stdout,out)
                if source.wait():raise RuntimeError('source failure')
            else:
                with lzma.open(archive,'rb') as inp:size,sha=check_stream(inp,b.stdout)
            if b.wait():raise RuntimeError('native failure')
            row=dict(mission=mission,stage=phase,states=a.count,bytes=size,sha256=sha,archive=str(archive));rows.append(row);print(json.dumps(row),flush=True)
    Path('build/imu-attitude-admission-'+('san' if a.sanitize else 'host')+'-proof.json').write_text(json.dumps(rows,indent=2)+'\n')
if __name__=='__main__':main()
