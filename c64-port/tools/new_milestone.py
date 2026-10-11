#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Scaffold a port milestone in the house style.

  python3 tools/new_milestone.py <id> --title "Translate Luminary SERVICER average-g" \
      --lane A [--module lm_servicer_avetoms]

Creates, without touching any shared file:
  docs/milestones/<id>.md          index entry (title + one-paragraph summary)
  docs/<ID>.md                     long-form record: scope, source map, evidence
  src/<module>.c, include/<module>.h
  tests/test_<module>.c            host test (strict and sanitizer builds)
  platform/c64/<module>_diagnostic.c   C64 diagnostic writing the $c000 mailbox
  tools/mk/<id>.mk                 test-/sanitize-/c64-/vice-<id> targets
  evidence/<id>/SHA256.json        empty manifest (fill with tools/evidence.py add)
"""
import argparse
import datetime
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import evidence  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[1]
LANES = ('A', 'B', 'C', 'D1', 'D2', 'E', 'F', 'G', 'H')

FILES = {
    'docs/milestones/{id}.md': '''---
id: {id}
title: {title}
date: {date}
lane: {lane}
---
TODO one paragraph: what was translated, against how many original states, which strict/sanitizer and C64 checks passed. See `docs/{doc}.md`.
''',
    'docs/{doc}.md': '''# {title}

Lane {lane}. Milestone `{id}`.

## Scope

TODO: the AGC source files and line ranges this milestone covers, and what is left.

## Source map

TODO: AGC label -> native C function, one row each.

## Evidence

Hashes: `evidence/{id}/SHA256.json`. Bulk files are assets of the `evidence-{id}` release
(`python3 tools/evidence.py fetch {id}`).

- Independent assembly comparison: TODO
- Host and sanitizer results: `make test-{id} sanitize-{id}`
- C64 execution results: `make vice-{id}`
''',
    'include/{module}.h': '''/* SPDX-License-Identifier: MIT */
#ifndef {guard}
#define {guard}
#include "agc_word.h"

/* TODO: native translation of the AGC routine(s) for milestone {id}. */
int {module}_run(void);

#endif
''',
    'src/{module}.c': '''/* SPDX-License-Identifier: MIT */
#include "{module}.h"

int {module}_run(void) {{
    return 0;
}}
''',
    'tests/test_{module}.c': '''/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "{module}.h"

int main(void) {{
    unsigned failures = 0, checks = 0;
    ++checks;
    if ({module}_run() != 0) ++failures;
    printf("{id}: %u checks, %u failures\\n", checks, failures);
    return failures ? 1 : 0;
}}
''',
    'platform/c64/{module}_diagnostic.c': '''/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "{module}.h"

/* Mailbox contract (tools/vice_run.py): write $c001-$c004 first, then $a5 to $c000 last. */
int main(void) {{
    volatile uint8_t *mail = (volatile uint8_t *)0xc000;
    unsigned failures = 0, checks = 0;
    ++checks;
    if ({module}_run() != 0) ++failures;
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {{}}
}}
''',
    'tools/mk/{id}.mk': '''# SPDX-License-Identifier: MIT
# Milestone {id} (lane {lane}): {title}
{var}_SRC = src/agc_word.c src/{module}.c
{var}_HDR = include/agc_word.h include/{module}.h
.PHONY: test-{id} sanitize-{id} c64-{id} vice-{id}
all: build/test-{id}
test: test-{id}
sanitize: sanitize-{id}
build/test-{id}: $({var}_SRC) tests/test_{module}.c $({var}_HDR) | build
\t$(CC) $(CPPFLAGS) $(CFLAGS) $({var}_SRC) tests/test_{module}.c -o $@
test-{id}: build/test-{id}
\t./build/test-{id}
sanitize-{id}: | build
\t$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $({var}_SRC) tests/test_{module}.c -o build/test-{id}-sanitize
\tASAN_OPTIONS=detect_leaks=0 ./build/test-{id}-sanitize
c64-{id}: build/{id}.prg
build/{id}.prg: $({var}_SRC) platform/c64/{module}_diagnostic.c $({var}_HDR) | build
\t$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/{id}.map $({var}_SRC) platform/c64/{module}_diagnostic.c -o $@
vice-{id}: build/{id}.prg
\tpython3 tools/vice_run.py build/{id}.prg
''',
}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('id')
    parser.add_argument('--title', required=True)
    parser.add_argument('--lane', required=True, choices=LANES)
    parser.add_argument('--module', help='C module name (default: id with - replaced by _)')
    args = parser.parse_args()
    if not evidence.ID.match(args.id) or args.id.startswith('legacy-'):
        parser.error('id must be lowercase [a-z0-9-] and not start with legacy-')
    module = args.module or args.id.replace('-', '_')
    if not re.match(r'^[a-z][a-z0-9_]*$', module):
        parser.error('module must be a C identifier')
    values = dict(id=args.id, title=args.title, lane=args.lane, module=module,
                  doc=args.id.upper(), guard=module.upper() + '_H',
                  var=re.sub(r'[^A-Z0-9]', '_', args.id.upper()),
                  date=datetime.datetime.now(datetime.timezone.utc).strftime('%Y-%m-%dT%H:%MZ'))
    targets = {pattern.format(**values): template for pattern, template in FILES.items()}
    clashes = [path for path in targets if (ROOT / path).exists()]
    if clashes or (ROOT / 'evidence' / args.id).exists():
        sys.exit('refusing to overwrite: ' + ', '.join(clashes or ['evidence/' + args.id]))
    for path, template in targets.items():
        (ROOT / path).parent.mkdir(parents=True, exist_ok=True)
        (ROOT / path).write_text(template.format(**values))
        print('created', path)
    evidence.save(evidence.load(args.id))
    print(f'created evidence/{args.id}/SHA256.json')
    print(f'next: make test-{args.id} sanitize-{args.id} c64-{args.id} vice-{args.id}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
