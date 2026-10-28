#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""What an interpretive word costs the AGC, for the work clock's charge
(include/interp_word_mct.h, the C64 compact interpreter and the host
alike): a table per order code and store kind, in units of 4 MCT.

The table comes from --spans: every flight scenario runs scaled
(tools/flight_host_run.c FLIGHT_SPAN_OUT), and each interpreter routine
the core ran and yaAGC matched gives one row, its words' slots against
yaAGC's machine cycles over the span less those charged to other tables
inside it. That is what the core charges, so the fit's error is the
"interpreter charge" error each host run reports, and
flight_host_differential.py bounds it. A span ends where the core hands
back: an EXIT word's span is its dispatch only, and the basic code after
it is a routine of its own, charged by its table. The fit prints each
scenario's error before and after, the error over the rows of 1000+
spans, and a held-out check (fitted on half the scenarios, measured on
the other half, both ways). Without --write it changes nothing.

Without --spans it runs scenarios on yaAGC alone (FLIGHT_WORD_OUT, the
reference run) and fits whole words: the machine cycles each spent in
the interpreter's banks (00 and 03, outside interrupts). That counts
the basic code after an EXIT in those banks as the word's, which the
C64 charges again as its own routine (#765), so the table no longer
comes from it; docs/flight-interp-mct.json keeps its per-mission word
means.

  interp_word_mct.py --spans [--skip-build] [--write] [--jobs N]
  interp_word_mct.py [--skip-build] [--keep DIR]   (DIR keeps the word lists)
"""
import argparse, json, os, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'docs/flight-interp-mct.json'
HEADER = ROOT / 'include/interp_word_mct_table.h'
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402

# The flight scenarios that run the most interpretive code (entry guidance;
# the descent's SERVICER and guidance with the engine on).
SCENARIOS = ['cm-entry', 'lm-p63-descent-thrust']


def words(path):
    """(centisecond, LOC, BANKSET, MCT in the interpreter, MCT elsewhere,
    the word NEWOPS fetched) per word."""
    return [tuple(int(x, 8) if i in (1, 2, 5) else int(x) for i, x in enumerate(line.split()))
            for line in Path(path).read_text().splitlines()]


# The per-word charge (include/interp_word_mct.h, the C64 core and the host
# alike): a word's slots by the order codes NEWOPS decodes, keyed as the
# compact interpreter dispatches them (src/lm_interp_core.c order()).
SLOTS = 73
STORE = 65


def code_slot(c):
    """An order code's slot: address orders by their INDJUMP index, the
    miscellaneous ones by MISCJUMP's, the shifts as one, the unary ones by
    UNAJUMP's (EXIT, code 0, among them)."""
    return c >> 2 if c & 1 else 32 + (c >> 3) if c & 2 else 48 if c & 4 else 49 + (c >> 3)


def word_slots(code):
    """The slots a word is charged: a store code word (bit 15 clear) its
    store kind's (DOSTORE: bits 12-14); an order pair the base (None), the
    left order's and the right order's, if any. NEWOPS's CCS leaves the
    magnitude less one: its low seven bits are the left code; TS EDOP keeps
    bits 8-14, and DANZIG's CCS EDOP, when they are not zero, gives them
    less one as the right code."""
    if not code & 0o40000:
        return [STORE + ((code >> 11) & 7)]
    u = ((~code & 0o77777) - 1) & 0o37777
    left, edop = u & 0o177, (u >> 7) & 0o177
    return [None, code_slot(left)] + ([code_slot(edop - 1)] if edop else [])


def fit(rows):
    """Non-negative least squares of each word's MCT over its slots and the
    base (the last column): least squares, then the negative coefficients
    held at 0 and the rest refitted, until none is negative."""
    import numpy as np
    a = np.zeros((len(rows), SLOTS + 1))
    y = np.array([float(mct) for _, mct in rows])
    for r, (slots, _) in enumerate(rows):
        for k in slots:
            a[r, SLOTS if k is None else k] += 1
    free = [j for j in range(SLOTS + 1) if a[:, j].any()]
    while True:
        c = np.zeros(SLOTS + 1)
        c[free] = np.linalg.lstsq(a[:, free], y, rcond=None)[0]
        neg = [j for j in free if c[j] < 0]
        if not neg:
            break
        free = [j for j in free if j not in neg]
    e = y - a @ c
    return c, float(np.sqrt((e * e).mean())), float(np.sqrt(((y - y.mean()) ** 2).mean()))


def span_rows(path, rows, exact=None):
    """Add a FLIGHT_SPAN_OUT file's spans to rows ({slot counts: [MCT sum,
    spans]}), keyed by the slots the span's words are charged, and to exact
    ({(slot counts, MCT): spans}, for the absolute error) if given; returns
    the file's span count."""
    n = 0
    for line in Path(path).read_text().splitlines():
        f = line.split()
        counts = [0] * (SLOTS + 1)
        for c in f[2:]:
            for k in word_slots(int(c, 8)):
                counts[SLOTS if k is None else k] += 1
        key, mct = tuple(counts), int(f[0]) - int(f[1])
        r = rows.setdefault(key, [0, 0])
        r[0] += mct
        r[1] += 1
        if exact is not None:
            exact[key, mct] = exact.get((key, mct), 0) + 1
        n += 1
    return n


def fit_spans(rows):
    """fit() over aggregated spans: each distinct slot vector once, at its
    mean MCT, weighted by its span count (least squares over every span)."""
    import numpy as np
    keys = list(rows)
    a = np.array(keys, dtype=float)
    w = np.sqrt(np.array([rows[k][1] for k in keys], dtype=float))
    y = np.array([rows[k][0] / rows[k][1] for k in keys])
    free = [j for j in range(SLOTS + 1) if a[:, j].any()]
    while True:
        c = np.zeros(SLOTS + 1)
        c[free] = np.linalg.lstsq(a[:, free] * w[:, None], y * w, rcond=None)[0]
        neg = [j for j in free if c[j] < 0]
        if not neg:
            break
        free = [j for j in free if j not in neg]
    return c


def charge_error(exact, units):
    """(net, absolute) error of the charge at the given units over exact
    spans, in percent of yaAGC's MCT: the absolute error sums |charge -
    yaAGC| span by span, so over- and under-charges do not cancel."""
    net = ab = yaagc = 0
    for (k, mct), n in exact.items():
        d = 4 * sum(units[j] * k[j] for j in range(SLOTS + 1)) - mct
        net += n * d
        ab += n * abs(d)
        yaagc += n * mct
    return (100.0 * net / yaagc, 100.0 * ab / yaagc) if yaagc else (0.0, 0.0)


def span_units(total):
    """The table's units fitted to total (span_rows rows): least squares,
    each slot rounded to 4 MCT, then the base set (the other slots held) so
    the charge over every span sums to yaAGC's. Spans with no word (a
    resumed word: DANZIG after an RTB or a job switch, charged nothing)
    count in that sum, so the base carries their time."""
    units = [int(round(v / 4)) for v in fit_spans(total)]
    num = den = 0.0
    for k, (mct, n) in total.items():
        num += mct - n * 4 * sum(units[j] * k[j] for j in range(SLOTS))
        den += n * k[SLOTS]
    if den:
        units[SLOTS] = max(1, min(255, int(round(num / den / 4))))
    return units


def merge(into, rows):
    for k, (mct, n) in rows.items():
        t = into.setdefault(k, [0, 0])
        t[0] += mct
        t[1] += n


def spans_main(a):
    old = json.loads(OUT.read_text())['word']
    old_units = old['slots'] + [old['base']]
    per = {}
    with tempfile.TemporaryDirectory() as tmp:
        def run(row):
            name, m, cs, keys, env = row
            out = Path(tmp) / f'{name}.spans'
            subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []),
                           cwd=ROOT, check=True, stdout=subprocess.DEVNULL,
                           env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SPAN_OUT': str(out)})
            return name, out
        with ThreadPoolExecutor(a.jobs) as ex:
            for name, out in ex.map(run, fhd.FLIGHT):
                rows, exact = {}, {}
                if span_rows(out, rows, exact):
                    per[name] = (rows, exact)
                out.unlink()
    total = {}
    for rows, _ in per.values():
        merge(total, rows)
    units = span_units(total)
    big = [n for n, (rows, _) in per.items() if sum(c for _, c in rows.values()) >= 1000]
    print(f"{sum(n for _, n in total.values())} spans, {len(total)} distinct, {len(per)} scenarios "
          f"({len(big)} of 1000+ spans)")
    print('scenario: spans; charge error net, routine by routine: before -> after')
    after = {}
    for name, (rows, exact) in per.items():
        b, after[name] = charge_error(exact, old_units), charge_error(exact, units)
        spans = sum(n for _, n in rows.values())
        print(f"  {name}: {spans}; {b[0]:+.1f}%, {b[1]:.1f}% -> {after[name][0]:+.1f}%, {after[name][1]:.1f}%")

    def spread(errs):
        ab = [e[1] for e in errs]
        net = [e[0] for e in errs]
        return (f"routine by routine mean {sum(ab) / len(ab):.1f}% (max {max(ab):.1f}%), "
                f"net {min(net):+.1f}..{max(net):+.1f}%")
    print(f"rows of 1000+ spans, before: {spread([charge_error(per[n][1], old_units) for n in big])}")
    print(f"rows of 1000+ spans, after:  {spread([after[n] for n in big])}")
    # Held out: fit on every other scenario (by name), measure on the rest,
    # both ways, so the fit is not judged on its own data.
    names = sorted(per)
    for half in (names[0::2], names[1::2]):
        train = {}
        for n in half:
            merge(train, per[n][0])
        held = span_units(train)
        test = [n for n in big if n not in half]
        print(f"held out ({len(test)} rows of 1000+ spans, fitted on the other {len(half)} scenarios): "
              f"{spread([charge_error(per[n][1], held) for n in test])}; "
              f"today's table on them: {spread([charge_error(per[n][1], old_units) for n in test])}")
    print(f"base {4 * old_units[SLOTS]} -> {4 * units[SLOTS]} MCT")
    if a.write:
        data = json.loads(OUT.read_text())
        data['word'] = dict(base=units[SLOTS], slots=units[:SLOTS], unit=4, spans=sum(n for _, n in total.values()),
                            fit='spans')
        OUT.write_text(json.dumps(data, indent=1, sort_keys=True) + '\n')
        write_header(units)
        print(f'wrote {OUT.relative_to(ROOT)}, {HEADER.relative_to(ROOT)}')
    return 0


def write_header(units):
    """include/interp_word_mct_table.h from the fitted units (the slots, then
    the base)."""
    # By code, as the C64 indexes it (include/interp_word_mct.h): entry 0 is
    # no order (EDOP 0, no right order); 1 + c an order code c's (the left
    # order's at its code, the right one's at EDOP, its code plus one);
    # 129 + k a store kind's; 137 an order pair's base. A store word is
    # charged at least 1 unit: every word run is charged.
    base = units[SLOTS]
    assert 1 <= base <= 255, base
    codes = [0] + [min(255, units[code_slot(c)]) for c in range(128)] + \
            [max(1, min(255, units[STORE + k])) for k in range(8)] + [base]
    HEADER.write_text(
        '/* SPDX-License-Identifier: MIT */\n/* Generated by tools/interp_word_mct.py from yaAGC\'s interpretive words\n'
        ' * (docs/flight-interp-mct.json): the work clock\'s charge per word, in units\n'
        ' * of 4 MCT (include/interp_word_mct.h). */\n'
        '#define INTERP_WORD_CODES {' + ','.join(map(str, codes)) + '}\n')


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-build', action='store_true', help='reuse build/flight-store/flight-host-run-<m>')
    p.add_argument('--keep', help='directory to keep each scenario\'s word list in')
    p.add_argument('--header', action='store_true', help='only rewrite the header from docs/flight-interp-mct.json')
    p.add_argument('--spans', action='store_true', help='fit to whole spans over every flight scenario (above)')
    p.add_argument('--write', action='store_true', help='with --spans: write the fitted table')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 1, help='with --spans: scenarios run at once')
    a = p.parse_args()
    if a.header:
        w = json.loads(OUT.read_text())['word']
        write_header(w['slots'] + [w['base']])
        return 0
    if not a.skip_build:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
        engine = fhd.frozen_engine(a.host_cc)
        for m in ('lm', 'cm'):
            fhd.build(a.host_cc, m, engine)
    if a.spans:
        return spans_main(a)
    data, rows = {}, []
    with tempfile.TemporaryDirectory() as tmp:
        keep = Path(a.keep) if a.keep else Path(tmp)
        keep.mkdir(parents=True, exist_ok=True)
        for name, m, cs, keys, env in fhd.FLIGHT:
            if name not in SCENARIOS:
                continue
            out = keep / f'{name}.words'
            # The reference keeps 32,767 centiseconds.
            subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(min(cs, 32767)), keys],
                           cwd=ROOT, check=True, stdout=subprocess.DEVNULL,
                           env={**os.environ, **env, 'FLIGHT_WORD_OUT': str(out)})
            w = words(out)
            d = data.setdefault(m, dict(scenarios=[], words=0, mct=0))
            d['scenarios'].append(name)
            d['words'] += len(w)
            d['mct'] += sum(x[3] for x in w)
            rows += [(word_slots(x[5]), x[3]) for x in w]
    for m, d in data.items():
        d['mean'] = round(d['mct'] / d['words']) if d['words'] else 0
        print(f"{m}: {d['words']} words in {', '.join(d['scenarios'])}, {d['mean']} MCT a word")
    # One table for both missions (the same INTERPRETER.agc), in units of
    # 4 MCT: a byte each, a word's charge at most 255 units.
    c, rmse, flat = fit(rows)
    units = [int(round(v / 4)) for v in c]
    print(f"per word (not written; the table comes from --spans): base {4 * units[SLOTS]} MCT, "
          f"RMSE {rmse:.0f} MCT ({flat:.0f} for one mean) over {len(rows)} words")
    data['word'] = json.loads(OUT.read_text())['word']
    OUT.write_text(json.dumps(data, indent=1, sort_keys=True) + '\n')
    print(f'wrote {OUT.relative_to(ROOT)} (word means)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
