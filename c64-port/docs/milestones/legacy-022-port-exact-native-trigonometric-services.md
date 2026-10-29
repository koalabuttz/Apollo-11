---
id: legacy-022-port-exact-native-trigonometric-services
title: Port exact native trigonometric services
seq: 22
---

All 65,536 complete source states match CDULOGIC, SIN, COS and CDUTRIGS; all-state sanitizers and 128 frozen C64 cases pass. The native fixed-point coefficients and overflow behavior retain the original instruction semantics. See `docs/TRIG.md`.

