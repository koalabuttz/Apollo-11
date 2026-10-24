# Per-PR CI timings

A PR that adds CI targets, or changes how long one takes, puts their seconds
here in a file of its own (for example `x86-flight-rows.json`), shaped like
`../ci_target_seconds.json`:

```json
{"c64": {"vice-new-check": 900}}
```

`tools/ci_run.py` reads these after the main file, so a new target is
balanced into the shards by its real time instead of the median. Two PRs
never edit the same lines this way. Don't edit `ci_target_seconds.json` in a
PR. The next refresh (`tools/ci_target_seconds.py`) folds these files into it
and removes them.

A target split into parts (`<name>-1` .. `<name>-N`) needs no file: each
unmeasured part gets an even share of the whole target's seconds.
