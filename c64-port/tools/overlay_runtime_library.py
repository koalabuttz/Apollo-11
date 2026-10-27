#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rebuild SDK integer helpers with no-CRT overlay dynamic-stack attributes."""
import hashlib,os,pathlib,re,shutil,subprocess,tempfile
def replace(path,data):
 """Write data to path atomically, and not at all when path already holds it,
 so parallel builders never see a half-written file (#974)."""
 if path.is_file() and path.read_bytes()==data:return
 with tempfile.NamedTemporaryFile(dir=path.parent,prefix=path.name+'.',delete=False) as f:f.write(data)
 os.replace(f.name,path)


def prepare(cc,build):
 cc=pathlib.Path(cc).resolve();sdk=cc.parent.parent/'mos-platform/common/lib/libcrt.a';ar=cc.parent/'llvm-ar';out=pathlib.Path(build)/'overlay-dynamic-libcrt.a';work=pathlib.Path(build)/'overlay-sdk-runtime';work.mkdir(parents=True,exist_ok=True)
 # Up to 118 builders call this under make -j: each works in its own scratch
 # folder and installs the finished archive with one atomic rename.
 with tempfile.TemporaryDirectory(dir=work) as scratch:
  scratch=pathlib.Path(scratch);archive=scratch/out.name
  shutil.copyfile(sdk,archive)
  for name in ('divmod.cc.obj','divmod-large.cc.obj','mul.cc.obj','shift.cc.obj','rotate.cc.obj'):
   original=scratch/(name+'.original');original.write_bytes(subprocess.check_output([str(ar),'p',str(sdk),name]))
   llvm=scratch/(name+'.ll');subprocess.run([str(cc),'-S','-emit-llvm','-x','ir',str(original),'-o',str(llvm)],check=True,capture_output=True)
   source=llvm.read_text();source=re.sub(r'^(attributes #\d+ = \{)(.*)(\})$',r'\1\2 "target-features"="-static-stack" \3',source,flags=re.M);llvm.write_text(source)
   obj=scratch/name;subprocess.run([str(cc),'-c','-emit-llvm','-Oz','-fno-static-stack','-x','ir',str(llvm),'-o',str(obj)],check=True,capture_output=True)
   subprocess.run([str(ar),'r',str(archive),str(obj)],check=True)
  replace(out,archive.read_bytes())
 replace(work/'provenance.txt',('SDK libcrt.a SHA256 '+hashlib.sha256(sdk.read_bytes()).hexdigest()+'\nOnly function target-features changed to -static-stack; SDK helper instructions and source algorithms retained.\n').encode())
 return str(out)
