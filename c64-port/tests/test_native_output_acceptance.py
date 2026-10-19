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
                replay_first=[dict(at='TIME4', host='1', c64='2')] if differences else [], reu_bounds=dict(ok=True))


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
            (d / 'commit').write_text(HEAD + '\n')
            self.assertEqual(no.load_downlink(d, HEAD)[0]['commit'], HEAD)

    def test_waits_and_not_exercised(self):
        rows = [dict(JETS, waits_on='D1'), dict(section='d', row='dump', kind='downlink', status='not exercised', note='no V74'),
                dict(section='d', row='x', kind='downlink', status='not exercised')]
        with mock.patch.object(no, 'window_states', return_value=states(0, 3)):
            out, counts = no.assess(rows, {'lm-a': [replay('lm-a', 0)]}, None)
        self.assertFalse(out[0]['ok'])
        self.assertIn('waits on D1', out[0]['text'])
        self.assertEqual((counts['not_exercised'], counts['unexplained'], counts['ok']), (2, 1, False))

    def test_committed_maps(self):
        for p in ('Luminary099', 'Comanche055'):
            plan = json.loads((ROOT / f'docs/acceptance/{p}-native-output-map.json').read_text())
            for row in plan['rows']:
                self.assertIn(row['kind'], ('dsky', 'downlink', 'channels'), row['row'])
                if row.get('status') == 'not exercised':
                    self.assertTrue(row.get('note'), row['row'])
                if row['kind'] == 'channels':
                    self.assertTrue(set(row['channels']) <= set(no.CHANNELS), row['row'])
                    m = 'lm' if p == 'Luminary099' else 'cm'
                    self.assertTrue(all(w[0].startswith(f'{m}-') for w in row['windows']), row['row'])


if __name__ == '__main__':
    unittest.main()
