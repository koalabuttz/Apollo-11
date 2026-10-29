#!/usr/bin/env python3
"""Compare complete source/native records online without raw stream copies."""
import lzma
import hashlib
import json
import os
import pathlib
import struct
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
RECORD = 4108

def exact(stream, count):
    data = bytearray()
    while len(data) < count:
        part = stream.read(count-len(data))
        if not part:
            break
        data.extend(part)
    return bytes(data)

def main():
    suffix = sys.argv[1] if len(sys.argv) > 1 else "native"
    start = int(sys.argv[2]) if len(sys.argv)>2 else 0
    end = int(sys.argv[3]) if len(sys.argv)>3 else 921088
    bounds = [str(start),str(end)]
    source = subprocess.Popen([str(ROOT/'build'/os.environ.get('LM_INTERBANK_REFERENCE','lm-interbank-reference')), str(ROOT/'build/standard-lead-source/Luminary099/MAIN.agc.bin'),*bounds], stdout=subprocess.PIPE)
    native = subprocess.Popen([str(ROOT/'build'/('lm-interbank-'+suffix)),*bounds], stdout=subprocess.PIPE)
    digest = hashlib.sha256()
    count = 0
    try:
        with lzma.open(ROOT/'build'/('lm-interbank-'+suffix+'.snapshots.bin.xz'), 'wb', preset=6) as packed:
            while True:
                a = exact(source.stdout, RECORD)
                b = exact(native.stdout, RECORD)
                if not a and not b:
                    break
                if len(a) != RECORD or a != b:
                    if len(a) == RECORD and len(b) == RECORD:
                        x, y = struct.unpack('<2054H', a), struct.unpack('<2054H', b)
                        print('DIFFERENCE',count+start,'kind',x[2],[(i-4,oct(u),oct(v)) for i,(u,v) in enumerate(zip(x,y)) if u!=v], flush=True)
                    else:
                        print('TRUNCATION',count,len(a),len(b),flush=True)
                    return 1
                digest.update(a)
                packed.write(a)
                count += 1
                if count % 65536 == 0:
                    print('PASS group', count//65536-1,count,'states',flush=True)
        if source.wait() or native.wait() or count != end-start:
            raise RuntimeError('process failure or missing records')
        result = dict(states=count,start=start,end=end,record_bytes=RECORD,decoded_sha256=digest.hexdigest())
        (ROOT/'build'/('lm-interbank-'+suffix+'-results.json')).write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps(result),flush=True)
        return 0
    finally:
        for process in (source,native):
            if process.poll() is None:
                process.terminate()
            process.wait()

if __name__ == '__main__':
    raise SystemExit(main())
