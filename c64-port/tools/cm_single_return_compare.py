#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
r=pathlib.Path(__file__).resolve().parents[1];raw=(r/'build/cm-single-return-reference.snapshots').read_bytes();expected=b''.join(raw[n+4104:n+8208]for n in range(0,len(raw),8208));actual=(r/'build/cm-single-return-native.snapshots').read_bytes();assert len(raw)==6144*8208 and actual==expected;print('6144 complete2048word+IRQ source TCQreturn states exact, covering every02000..07777fixed caller address each SPSIN/SPCOS;SHA256:'+hashlib.sha256(actual).hexdigest())
