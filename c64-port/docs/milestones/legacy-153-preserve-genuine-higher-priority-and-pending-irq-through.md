---
id: legacy-153-preserve-genuine-higher-priority-and-pending-irq-through
title: Preserve genuine higher priority and pending IRQ through P66 direct launch
seq: 153
---

The direct P66 launch preserves the original higher-priority job and pending IRQ rather than inserting an extra Executive poll. A complete original-source case is unchanged across strict host, sanitizer and actual C64 execution. See `docs/P66-DIRECT-PENDING.md`.

