---
id: legacy-045-render-actual-native-dsky-relay-output-on-the-c64
title: Render actual native DSKY relay output on the C64
seq: 45
---

The source mode66 display job and channel10 relay drain now drive real C64 screen/color RAM. Host, sanitizer and target checks verify numeric/sign latches, lights, blanking and the128cs flash cycle; live application keyboard/device integration remains pending. See `docs/DSKY_DISPLAY.md`.

