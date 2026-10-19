#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/program_acceptance.py on made-up host and replay results."""
import json
import pathlib
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import program_acceptance as pa  # noqa: E402

HEAD = 'abc123'


def host_row(name, exit=0, misses=0, bridged=0, commit=HEAD):
    holes = [dict(point='43,3303', label='SMODECHK+2', file='X.agc', lane='A', core_start=True)]
    holes += [dict(point='50,2002', label='?', file='?', lane='?', core_start=False)] * bridged
    return dict(scenario=f'{name}-scaled-x1', schedule='1,1,2', exit=exit, commit=commit, centiseconds=100,
                routine_mismatches=[dict(point='01,2000')] * misses, unregistered=holes)


def replay(name, snap, differences=0, first='TIME4', status=0, commit=HEAD):
    return dict(scenario=name, snap_cs=snap, snap_at=snap, requested_cs=1000, status=status, at='12,2644',
                replay_rows=10, replay_ended=True, replay_differences=differences, replay_matched_cs=[100],
                replay_differ_at_cs=100 if differences else None, commit=commit,
                replay_first=[dict(at=first, host='1', c64='2')] if differences else [],
                reu_bounds=dict(ok=True), records=[1, 2, 3])


ROWS = [dict(row='P63', section='modes', entry='P63LM', status='mapped', scenarios=['lm-a']),
        dict(row='P64', section='modes', entry='STARTP64', status='mapped', scenarios=['lm-a', 'lm-b']),
        dict(row='V45', section='verbs', entry='ALM/END', status='not exercised', scenarios=[], note='spare'),
        dict(row='V40', section='verbs', entry='VBZERO', status='TBD', scenarios=[])]


class ProgramAcceptance(unittest.TestCase):
    def test_host_verdicts(self):
        self.assertEqual(pa.host_result(None), (False, 'not run'))
        self.assertTrue(pa.host_result(host_row('lm-a'))[0])          # core-start points do not count
        self.assertFalse(pa.host_result(host_row('lm-a', exit=1))[0])
        self.assertIn('1 routines differ', pa.host_result(host_row('lm-a', misses=1))[1])
        self.assertIn('2 points bridged', pa.host_result(host_row('lm-a', bridged=2))[1])

    def test_replay_verdicts(self):
        self.assertEqual(pa.replay_result(replay('lm-a', 0)), (True, 'matches host'))
        self.assertIn('differs (clock', pa.replay_result(replay('lm-a', 0, differences=3))[1])
        self.assertIn('differs (data: first at 0107', pa.replay_result(replay('lm-a', 0, 3, first='0107'))[1])
        self.assertEqual(pa.replay_result(replay('lm-a', 0, status=2)), (False, 'stops at 12,2644'))

    def test_counts_and_pass_rule(self):
        host = {'lm-a': host_row('lm-a'), 'lm-b': host_row('lm-b')}
        _, counts = pa.assess(ROWS[:3], host, {'lm-a': [replay('lm-a', 0)]})
        self.assertEqual((counts['mapped'], counts['host_clean'], counts['c64_match']), (2, 2, 2))
        self.assertTrue(counts['ok'])
        _, counts = pa.assess(ROWS, host, {'lm-a': [replay('lm-a', 0)]})
        self.assertFalse(counts['ok'])                                 # a TBD row
        _, counts = pa.assess(ROWS[:3], host, {'lm-a': [replay('lm-a', 0, differences=1)]})
        self.assertEqual(counts['c64_match'], 0)
        _, counts = pa.assess(ROWS[:3], {'lm-a': host_row('lm-a')}, {})
        self.assertEqual(counts['host_clean'], 1)                      # lm-b not run

    def test_stale_inputs_are_not_counted(self):
        with tempfile.TemporaryDirectory() as d:
            d = pathlib.Path(d)
            (d / 'summary.json').write_text(json.dumps([host_row('lm-a'), host_row('lm-b', commit='old (dirty)')]))
            host, stale = pa.load_host(d, HEAD)
            self.assertEqual((sorted(host), stale), (['lm-a'], {'lm-b': 'old (dirty)'}))
            (d / 'r1.json').write_text(json.dumps(replay('lm-a', 200)))
            (d / 'r2.json').write_text(json.dumps(replay('lm-a', 100)))
            (d / 'r3.json').write_text(json.dumps(replay('lm-b', 0, commit=None)))
            (d / 'other.json').write_text(json.dumps(dict(scenario='lm-a')))   # not a replay
            runs, stale = pa.load_replays(sorted(d.glob('r*.json')) + [d / 'other.json'], 'lm', HEAD)
            self.assertEqual([r['snap_cs'] for r in runs['lm-a']], [100, 200])
            self.assertEqual(stale, [('r3.json', 'unknown')])

    def test_doc_and_evidence(self):
        host = {'lm-a': host_row('lm-a'), 'lm-b': host_row('lm-b')}
        replays = {'lm-a': [replay('lm-a', 0)]}
        assessed, counts = pa.assess(ROWS, host, replays)
        with tempfile.TemporaryDirectory() as d:
            d = pathlib.Path(d)
            evidence = pa.write_evidence('lm', host, replays, d / 'ev', store=False)
            self.assertNotIn('records', json.loads((d / 'ev/replay-lm-a-0.json').read_text()))
            pa.write_doc('lm', assessed, counts, host, replays, {}, [], evidence, HEAD, d / 'doc.md')
            text = (d / 'doc.md').read_text()
        self.assertIn('Result: **FAIL**', text)
        self.assertIn('| 4 | 2 | 1 | 1 | 2 of 2 | 2 of 2 |', text)
        self.assertIn('lm-a 0: matches host', text)

    def test_real_map_names_real_scenarios(self):
        import flight_host_differential as fhd
        names = {n for n, *_ in fhd.FLIGHT}
        for program in ('Luminary099',):
            plan = json.loads((ROOT / f'docs/acceptance/{program}-program-map.json').read_text())
            rows, windows = plan['rows'], plan['replay_windows']
            self.assertLessEqual({s for row in rows for s in row['scenarios']}, {w[0] for w in windows})
            self.assertTrue(all(w[0] in names and 0 < w[2] <= 1000 for w in windows), windows)
            for row in rows:
                self.assertIn(row['status'], ('mapped', 'not exercised', 'TBD'), row['row'])
                self.assertEqual(bool(row['scenarios']), row['status'] == 'mapped', row['row'])
                self.assertLessEqual(set(row['scenarios']), names, row['row'])


if __name__ == '__main__':
    unittest.main()
