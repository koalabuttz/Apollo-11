---
id: legacy-081-port-actual-comanche-setjtag-task-and-native-callback
title: Port actual Comanche SETJTAG task and native callback
seq: 81
---

The original SETJTAG body saves elapsed time, sets JETAG, changes phase and returns through TASKOVER. All 1024 complete source states and real native callback sequences pass host, sanitizer and C64 checks; the initial harness failure and corrected proof remain retained. See `docs/COMANCHE_ENTRY_TASKS.md`.

