#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/native_output_acceptance.py on made-up downlink and replay results,
and the committed native output maps."""
import json
import pathlib
import sys
import tempfile
import unittest
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import native_output_acceptance as no  # noqa: E402

HEAD = 'abc123'


def replay(name, snap, differences=0, matched=(100,), commit=HEAD):
    return dict(scenario=name, snap_cs=snap, snap_at=snap, requested_cs=1000, status=0, replay_rows=10,
                replay_ended=True, replay_differences=differences, replay_matched_cs=list(matched),
                replay_differ_at_cs=200 if differences else None, commit=commit,
                replay_first=[dict(at='TIME4', host='1', c64='2')] if differences else [],
                reu_bounds=dict(ok=True, dma_outside=0, dma_trace_armed=True), replay_unpaired_count=0)


def states(*values):
    """Host states with channel 5 taking each value in turn, 100 cs apart."""
    return [(100 * i, {c: (v if c == 5 else 0) for c in no.CHANNELS}) for i, v in enumerate(values)]


JETS = dict(section='channels', row='jets', kind='channels', channels=[5], windows=[['lm-a', 0, 1000]])


class NativeOutputAcceptance(unittest.TestCase):
    def test_driven(self):
        self.assertEqual(no.driven(states(0, 3, 0), [5], 100), {5: [0, 3]})
        self.assertEqual(no.driven(states(0, 0, 3), [5], 100), {})

    def test_channel_row_needs_a_match_and_a_driven_channel(self):
        with mock.patch.object(no, 'window_states', return_value=states(0, 3)):
            ok, text = no.assess_channels(JETS, {'lm-a': [replay('lm-a', 0)]})
            self.assertTrue(ok, text)
            self.assertIn('ch5 00000/00003', text)
            ok, _ = no.assess_channels(JETS, {'lm-a': [replay('lm-a', 0, matched=())]})
            self.assertFalse(ok)
            ok, _ = no.assess_channels(JETS, {'lm-a': [replay('lm-a', 500)]})   # a window the row does not name
            self.assertFalse(ok)
        with mock.patch.object(no, 'window_states', return_value=states(0, 0)):
            ok, text = no.assess_channels(JETS, {'lm-a': [replay('lm-a', 0)]})
            self.assertFalse(ok)
            self.assertIn('unchanged', text)

    def test_channel_row_needs_the_whole_window_to_match(self):
        # #804: one matched checkpoint in a window that fails is not a pass.
        failed = dict(replay('lm-a', 0, differences=7), ok=False, replay_skipped=['DSPCNT'],
                      reu_bounds=dict(ok=False, outside=12))
        skipped = dict(replay('lm-a', 0), replay_skipped=['DSPCNT'])
        with mock.patch.object(no, 'window_states', return_value=states(0, 3)):
            for r in (failed, skipped, dict(replay('lm-a', 0), ok=False)):
                ok, text = no.assess_channels(JETS, {'lm-a': [r]})
                self.assertFalse(ok, text)

    def test_downlink(self):
        row = dict(kind='downlink', host=['lm-p00-restart'], c64=['lm-p00'])
        good = dict(host=[dict(scenario='lm-p00-restart', ok=True, native_words=400, span_differ=0)],
                    c64=[dict(window='lm-p00', ok=True, records=100, records_differ=0, list_starts=[])])
        self.assertTrue(no.assess_downlink(row, good)[0])
        self.assertFalse(no.assess_downlink(row, None)[0])
        bad = dict(good, c64=[dict(window='lm-p00', ok=False, records=100, records_differ=3)])
        self.assertFalse(no.assess_downlink(row, bad)[0])
        self.assertFalse(no.assess_downlink(dict(row, c64=[]), good)[0])

    def test_downlink_stale(self):
        with tempfile.TemporaryDirectory() as d:
            d = pathlib.Path(d)
            (d / 'summary.json').write_text(json.dumps(dict(host=[], c64=[])))
            self.assertEqual(no.load_downlink(d, HEAD), (None, 'unknown'))
            (d / 'commit').write_text(HEAD + '\n')   # a stamp beside the summary does not count (#804)
            self.assertEqual(no.load_downlink(d, HEAD), (None, 'unknown'))
            (d / 'summary.json').write_text(json.dumps(dict(commit=HEAD, host=[], c64=[])))
            self.assertEqual(no.load_downlink(d, HEAD)[0]['commit'], HEAD)
            # C64 windows count only on the store they ran on.
            c64 = [dict(window='lm-p00', mission='lm', ok=True)]
            (d / 'summary.json').write_text(json.dumps(dict(commit=HEAD, stores=dict(lm='ab' * 32), host=[], c64=c64)))
            self.assertEqual(no.load_downlink(d, HEAD, 'lm', 'ab' * 32)[0]['commit'], HEAD)
            self.assertIsNone(no.load_downlink(d, HEAD, 'lm', 'cd' * 32)[0])
            self.assertEqual(no.load_downlink(d, HEAD, 'cm', 'cd' * 32)[0]['commit'], HEAD)

    def test_dsky_sweep(self):
        store = 'ab' * 32
        good = dict(mode='all', missions=['lm', 'cm'], replay_exact=True, item6=True, failed=[],
                    stores=dict(lm=dict(reu_sha256=store), cm=dict(reu_sha256='cd' * 32)),
                    by_mission=dict(lm=dict(cases=1830, passed=1830, skipped_cases=0),
                                    cm=dict(cases=1830, passed=1830, skipped_cases=0)))
        row = dict(kind='dsky')
        with tempfile.TemporaryDirectory() as d:
            f = pathlib.Path(d) / 'dsky-sweep-summary.json'
            self.assertEqual(no.load_sweep(f, 'lm', store), (None, None))
            ok, text = no.assess_dsky(row, None, 'lm')
            self.assertFalse(ok)
            self.assertTrue(text.startswith('pending'), text)
            f.write_text(json.dumps(good))
            sweep, why = no.load_sweep(f, 'lm', store)
            self.assertIsNone(why)
            self.assertTrue(no.assess_dsky(row, sweep, 'lm')[0])
            # Another store does not count, and says why.
            sweep, why = no.load_sweep(f, 'lm', 'ef' * 32)
            self.assertIsNone(sweep)
            self.assertFalse(no.assess_dsky(row, sweep, 'lm', why)[0])
            f.write_text(json.dumps(dict(good, missions=['cm'])))
            self.assertIsNone(no.load_sweep(f, 'lm', store)[0])
        # Item 6: no failure, no skipped word, --cases all, --replay-exact.
        bad = [dict(good, replay_exact=False), dict(good, mode='ci'), dict(good, item6=False),
               dict(good, failed=[dict(m='lm', keys='V16N36E', ok=False, **{'class': 'clock'})]),
               dict(good, by_mission=dict(lm=dict(cases=1830, passed=1830, skipped_cases=2))),
               dict(good, by_mission=dict(lm=dict(cases=1830, passed=1829, skipped_cases=0)))]
        for s in bad:
            self.assertFalse(no.assess_dsky(row, s, 'lm')[0], s)
        out, counts = no.assess([dict(section='DSKY', row='sweep', kind='dsky')], {}, None, good, 'lm')
        self.assertTrue(out[0]['ok'], out[0]['text'])

    def test_waits_and_not_exercised(self):
        rows = [dict(JETS, waits_on='D1'), dict(section='d', row='dump', kind='downlink', status='not exercised', note='no V74'),
                dict(section='d', row='x', kind='downlink', status='not exercised')]
        with mock.patch.object(no, 'window_states', return_value=states(0, 3)):
            out, counts = no.assess(rows, {'lm-a': [replay('lm-a', 0)]}, None)
        self.assertFalse(out[0]['ok'])
        self.assertIn('waits on D1', out[0]['text'])
        self.assertEqual((counts['not_exercised'], counts['unexplained'], counts['ok']), (2, 1, False))

    def test_end_states_counts_whole_states(self):
        import struct
        import flight_scenario_vice as fsv
        k = fsv.STATE // 2
        data = b''.join(struct.pack(f'<{k}H', cs, *range(1, k)) for cs in (10, 20, 30))
        got = no.end_states(data)
        self.assertEqual([cs for cs, _ in got], [10, 20, 30])
        self.assertEqual(got[0][1][2048:2048 + len(no.CHANNELS)], tuple(range(2049, 2049 + len(no.CHANNELS))))

    def test_dump_row(self):
        goto, entries = no.DUMP['lm']
        row = dict(section='d', row='dump', kind='dump', windows=[['lm-a', 0, 300, 1, 50]])

        def words(*gotos):
            return [(100 * i, [g if a == goto else 0 for a in range(2048 + len(no.CHANNELS))])
                    for i, g in enumerate(gotos)]
        with mock.patch.object(no, 'window_states', return_value=words(0o3523, entries[1])):
            ok, text = no.assess_dump(row, {'lm-a': [dict(replay('lm-a', 0), downrupt=True)]})
            self.assertTrue(ok, text)
            self.assertIn('by 100 cs', text)
            ok, text = no.assess_dump(row, {'lm-a': [replay('lm-a', 0)]})
            self.assertFalse(ok)
            self.assertIn('without --downrupt', text)
            # The dump starts after the last checkpoint the C64 matched.
            ok, _ = no.assess_dump(row, {'lm-a': [dict(replay('lm-a', 0, matched=(50,)), downrupt=True)]})
            self.assertFalse(ok)
        with mock.patch.object(no, 'window_states', return_value=words(0o3523, 0o3523)):
            ok, text = no.assess_dump(row, {'lm-a': [dict(replay('lm-a', 0), downrupt=True)]})
            self.assertFalse(ok)
            self.assertIn('never points', text)

    def test_dump_windows_run_with_downrupt(self):
        rows = [dict(kind='dump', windows=[['lm-a', 0, 300, 1, 50]]), JETS]
        self.assertEqual(no.downrupt_windows(rows), {('lm-a', 0, 300, 1, 50)})
        self.assertEqual(no.window_command(('lm-a', 0, 300, 1, 50), True)[-1], '--downrupt')
        self.assertNotIn('--downrupt', no.window_command(('lm-a', 0, 1000)))

    def test_doc_names_the_store(self):
        # Acceptance contract rules 3 and 9: one header commit, the store's
        # full sha256 on one line.
        import re
        counts = dict(ok=True, rows=0, passed=0, not_exercised=0, waiting=0, unexplained=0)
        with tempfile.TemporaryDirectory() as d:
            out = pathlib.Path(d) / 'doc.md'
            no.write_doc('lm', [], counts, [], None, {}, HEAD, out, store_sha='ab' * 32)
            text = out.read_text()
        self.assertRegex(text, r'flight-store/lm\.reu\b[^\n]*\b' + 'ab' * 32)
        self.assertEqual(len(re.findall(r'commit `', text)), 1)

    def test_committed_maps(self):
        for p in ('Luminary099', 'Comanche055'):
            plan = json.loads((ROOT / f'docs/acceptance/{p}-native-output-map.json').read_text())
            for row in plan['rows']:
                self.assertIn(row['kind'], ('dsky', 'downlink', 'channels', 'dump'), row['row'])
                if row.get('status') == 'not exercised':
                    self.assertTrue(row.get('note'), row['row'])
                if row['kind'] == 'channels':
                    self.assertTrue(set(row['channels']) <= set(no.CHANNELS), row['row'])
                    m = 'lm' if p == 'Luminary099' else 'cm'
                    self.assertTrue(all(w[0].startswith(f'{m}-') for w in row['windows']), row['row'])
                if row['kind'] == 'dump':
                    # A --downrupt window writes the same result file as a
                    # program map window with the same start, so none may share one.
                    m = 'lm' if p == 'Luminary099' else 'cm'
                    used = json.dumps(json.loads((ROOT / f'docs/acceptance/{p}-program-map.json').read_text()))
                    for w in row['windows']:
                        self.assertTrue(w[0].startswith(f'{m}-'), row['row'])
                        self.assertNotIn(f'"{w[0]}", {w[1]}', used, row['row'])


if __name__ == '__main__':
    unittest.main()
