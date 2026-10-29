---
id: legacy-083-port-actual-throttle-after-poll-continuation-fetch
title: Port actual throttle after-poll continuation fetch
seq: 83
---

The original NEWOPS and LOC03507/03510 fetch routes resume THROTTLE+3 with exact CYR and control debris. All 8192 complete source states and sanitizer snapshots match; 64 native C64 cases pass. See `docs/P66-RUNTIME-FETCH.md`.

