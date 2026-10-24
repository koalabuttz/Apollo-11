#!/usr/bin/env python3
"""Host-only GPL oracle variant: route DV through pinned hardware model.

The pinned fast mathematical DV path mishandles sign-extended L=-0 in
SpToDecent. Select its existing SimulateDV for every DV; change no assembly,
register timing, non-DV instruction or native code. Keep the pinned checkout
intact and emit a reviewable one-line diff and source identities.
"""
import difflib,hashlib,os,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
engine=ROOT/'deps/virtualagc/yaAGC'; source=engine/'agc_engine.c'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='9c59728cdd968503caaaafe50a18ab3ed841f6176da3299c137f3a6a158fcbef'
original=source.read_text()
needle='if (AbsA > AbsK || (AbsA == AbsK && AbsL != AGC_P0) || ValueOverflowed(Div16) != AGC_P0)'
assert original.count(needle)==1
modified=original.replace(needle,needle.replace('if (','if (1 || ',1))
# Differential scripts run this and then link the object, and may run it
# concurrently (make -j), so every output is written to a per-process
# temporary and renamed into place: a reader sees the old file or the new one,
# never a half-written one.
def put(name,text):
    final=ROOT/'build'/name; tmp=final.with_name(f'{final.name}.{os.getpid()}.tmp')
    tmp.write_text(text); os.replace(tmp,final)
put('agc_engine_hardware_dv.c',modified)
patch=''.join(difflib.unified_diff(original.splitlines(True),modified.splitlines(True),fromfile='pinned/agc_engine.c',tofile='hardware-dv/agc_engine.c'))
put('divide-hardware-dv.patch',patch)
obj=ROOT/'build/agc_engine_hardware_dv.o'; tmp=obj.with_name(f'{obj.name}.{os.getpid()}.tmp')
try:
    subprocess.run(['gcc','-O2','-I'+str(engine),'-c',str(ROOT/'build/agc_engine_hardware_dv.c'),'-o',str(tmp)],check=True)
    os.replace(tmp,obj)
finally:
    tmp.unlink(missing_ok=True)
print('Pinned engine SHA256:',hashlib.sha256(source.read_bytes()).hexdigest())
print('Hardware-DV variant SHA256:',hashlib.sha256(modified.encode()).hexdigest())
print('DV uses pinned SimulateDV in every case; one branch changed, checkout untouched')
