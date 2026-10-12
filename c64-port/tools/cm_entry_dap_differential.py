#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run unchanged Comanche055 CM_ENTRY_DIGITAL_AUTOPILOT from every entry in the pinned yaAGC for all 65536
cases (tests/cm_entry_dap_cases.c), check the native port against every row,
and keep the oracle stream as per-case deltas from the case input
(build/cm-entry-dap-reference.deltas.xz: n, status, channels 5/6,
changed-word count, then address/value pairs; little-endian uint16, n as two words). The
pinned identity is the SHA256 of the full 2053-word rows.
--record rewrites the pinned stream identity; --sanitize checks an
ASan/UBSan native build instead of the strict one."""
import argparse,hashlib,json,lzma,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
STATES=65536;ROW=2053*2
CORE=['src/cm_entry_dap.c','src/agc_image_ops.c','src/agc_word.c','src/agc_multiword.c','src/agc_hardware_divide.c','tests/cm_entry_dap_cases.c']
PIN=R/'evidence/cm-entry-dap/cm-entry-dap-source-stream.json'
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--record',action='store_true');p.add_argument('--sanitize',action='store_true');p.add_argument('--cc',default='cc');a=p.parse_args()
(R/'build').mkdir(exist_ok=True)
if not (R/'deps/cm-flight-fixture/MAIN.agc.bin').exists():subprocess.run([sys.executable,'tools/cm_flight_prepare_reference.py'],cwd=R,check=True)
if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/cm_entry_dap_reference.c','tests/cm_entry_dap_cases.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/cm-entry-dap-reference'],cwd=R,check=True)
native='build/cm-entry-dap-native'+('-sanitize' if a.sanitize else '')
flags=['-g','-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'] if a.sanitize else ['-O2']
subprocess.run([a.cc,'-std=c99',*flags,'-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/cm_entry_dap_snapshot.c',*CORE,'-o',native],cwd=R,check=True)
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_entry_dap_inputs.c','tests/cm_entry_dap_cases.c','src/agc_word.c','-o','build/cm-entry-dap-inputs'],cwd=R,check=True)
inputs=subprocess.run(['build/cm-entry-dap-inputs'],input=b''.join(struct.pack('<I',n) for n in range(STATES)),cwd=R,check=True,stdout=subprocess.PIPE).stdout
ref=subprocess.Popen(['build/cm-entry-dap-reference','deps/cm-flight-fixture/MAIN.agc.bin',str(STATES)],cwd=R,stdout=subprocess.PIPE)
nat=subprocess.Popen([native,str(STATES)],cwd=R,stdout=subprocess.PIPE,env={'ASAN_OPTIONS':'detect_leaks=0'})
h=hashlib.sha256();size=0;status={};entries={};out_path=R/'build/cm-entry-dap-reference.deltas.xz';tmp=out_path.with_suffix('.xz.part')
with lzma.open(tmp,'wb',preset=3) as out:
 for n in range(STATES):
  x=ref.stdout.read(ROW);y=nat.stdout.read(ROW)
  if len(x)!=ROW or x!=y:
   ref.kill();nat.kill();tmp.unlink()
   if len(x)==ROW==len(y):
    X=struct.unpack('<2053H',x);Y=struct.unpack('<2053H',y)
    print('FAIL state',n,[(f'{i-5:04o}',f'{v:06o}',f'{Y[i]:06o}') for i,v in enumerate(X) if v!=Y[i]][:25])
   else:print('FAIL state',n,'short row')
   raise SystemExit(1)
  w=struct.unpack('<2053H',x);status[w[2]]=status.get(w[2],0)+1;h.update(x);size+=len(x)
  init=struct.unpack_from('<2048H',inputs,n*4100+4);ch=[(i,v) for i,v in enumerate(w[5:]) if v!=init[i]]
  out.write(struct.pack(f'<{6+2*len(ch)}H',w[0],w[1],w[2],w[3],w[4],len(ch),*[v for pair in ch for v in pair]))
 if ref.stdout.read(1) or nat.stdout.read(1) or ref.wait() or nat.wait():raise SystemExit('FAIL producer exit or trailing bytes')
tmp.replace(out_path)
identity=dict(archive='build/cm-entry-dap-reference.deltas.xz',decoded_sha256=h.hexdigest(),decoded_bytes=size,states=STATES,status_counts={'returned':status.get(0,0)},oracle='pinned yaAGC hardware-DV variant, deps/cm-flight-fixture/MAIN.agc.bin (byte-original Comanche055 rope 6e3150ae...)')
if a.record:PIN.write_text(json.dumps(identity,indent=2)+'\n')
pin=json.loads(PIN.read_text())
if (pin['decoded_sha256'],pin['decoded_bytes'])!=(identity['decoded_sha256'],size):raise SystemExit('FAIL oracle stream differs from the pinned identity')
print(f'PASS {STATES} original CM_ENTRY_DIGITAL_AUTOPILOT oracle states equal native 2053-word rows{" (ASan/UBSan)" if a.sanitize else ""}',identity['status_counts'],h.hexdigest(),flush=True)
