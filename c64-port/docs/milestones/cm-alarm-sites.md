---
id: cm-alarm-sites
title: Compile Comanche alarm call-site operands into native constants
date: 2026-10-11T23:23Z
lane: A
---
The alarm code after every genuine Comanche055 `TC ALARM`, `TC ALARM2`, `TC BAILOUT` and `TC POODOO` (62 sites) is now a compiled constant generated from the original source listing, so the native ALARM_AND_ABORT entries no longer read a copy of the rope at runtime. All 11,904 site/context cases match the existing native entries fed the original rope words (859,919 strict and sanitizer checks), and 372 sampled cases match the host digests in VICE. See `docs/CM-ALARM-SITES.md`.
