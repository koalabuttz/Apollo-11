---
id: legacy-079-port-original-keyrupt-register-saves-and-time-sampling
title: Port original KEYRUPT register saves and time sampling
seq: 79
---

The real KEYRUPT1 path preserves raw overflow, interrupted Q and banks, double-word time sampling and channel15 key input before actual KEYCOM admission. All 4096 complete original states, sanitizer streams and 64 C64 cases agree; full interrupt entry and resume composition remain separately qualified. See `docs/DSKY-KEYBOARD-SERVICES.md`.

