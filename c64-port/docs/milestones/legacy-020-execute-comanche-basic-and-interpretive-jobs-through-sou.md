---
id: legacy-020-execute-comanche-basic-and-interpretive-jobs-through-sou
title: Execute Comanche basic and interpretive jobs through source-exact lifecycle transitions
seq: 20
---

The separate seven-core CM runtime now dispatches native C callbacks, exchanges basic and interpreted contexts, sleeps and wakes jobs, and changes priorities through original Comanche semantics. All 876 complete source-derived checkpoints and twenty real native sequences agree; host and sanitizer checks and four stock C64 diagnostics pass. IRQ coordination remains caller-owned pending the dedicated adapter. See `docs/COMANCHE_DISPATCH.md`.

