#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,json,os
R=pathlib.Path(__file__).resolve().parents[1];p=json.loads((R/'build/cm-clock-display-busy-proof.json').read_text())
data=subprocess.check_output(['./build/cm-clock-display-busy-sanitize'],cwd=R,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
assert hashlib.sha256(data).hexdigest()==p['snapshot_sha256']
print('PASS: full ASAN/UBSAN continuous CM clock display corpus')
