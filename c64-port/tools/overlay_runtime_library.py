#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rebuild SDK integer helpers with no-CRT overlay dynamic-stack attributes."""
import hashlib,pathlib,re,shutil,subprocess
def prepare(cc,build):
 cc=pathlib.Path(cc).resolve();sdk=cc.parent.parent/'mos-platform/common/lib/libcrt.a';ar=cc.parent/'llvm-ar';out=pathlib.Path(build)/'overlay-dynamic-libcrt.a';work=pathlib.Path(build)/'overlay-sdk-runtime';work.mkdir(parents=True,exist_ok=True)
 shutil.copyfile(sdk,out)
 for name in ('divmod.cc.obj','divmod-large.cc.obj','mul.cc.obj','shift.cc.obj','rotate.cc.obj'):
  original=work/(name+'.original');original.write_bytes(subprocess.check_output([str(ar),'p',str(sdk),name]))
  llvm=work/(name+'.ll');subprocess.run([str(cc),'-S','-emit-llvm','-x','ir',str(original),'-o',str(llvm)],check=True,capture_output=True)
  source=llvm.read_text();source=re.sub(r'^(attributes #\d+ = \{)(.*)(\})$',r'\1\2 "target-features"="-static-stack" \3',source,flags=re.M);llvm.write_text(source)
  obj=work/name;subprocess.run([str(cc),'-c','-emit-llvm','-Oz','-fno-static-stack','-x','ir',str(llvm),'-o',str(obj)],check=True,capture_output=True)
  subprocess.run([str(ar),'r',str(out),str(obj)],check=True)
 (work/'provenance.txt').write_text('SDK libcrt.a SHA256 '+hashlib.sha256(sdk.read_bytes()).hexdigest()+'\nOnly function target-features changed to -static-stack; SDK helper instructions and source algorithms retained.\n')
 return str(out)
