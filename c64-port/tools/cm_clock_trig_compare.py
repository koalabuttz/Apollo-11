#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
r=pathlib.Path(__file__).resolve().parents[1];raw=(r/'build/cm-clock-trig-reference.snapshots').read_bytes();expected=b''.join(raw[n+4102:n+8204]for n in range(0,len(raw),8204));actual=(r/'build/cm-clock-trig-native.snapshots').read_bytes();assert expected==actual;print('512 complete2048word+IRQ actualCLOCKJOB QUICTRIG source entry/exit pairs exact; SHA256:'+hashlib.sha256(actual).hexdigest())
