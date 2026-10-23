#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/flight_scenario_vice.py --replay: host and C64 states paired by
centisecond, the C64 end a centisecond after the host's included."""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_scenario_vice as fsv  # noqa: E402
import program_acceptance as pa  # noqa: E402

N = fsv.STATE // 2 - 1          # erasable words and output channels in a state
WORD = next(i for i in range(0o100, 2048) if fsv.compared(i) and i not in fsv.TIMERS)


def state(cs, **set_):
    """A state at cs: TIME1 (025) and TIME3 (026) count with the window's
    centiseconds, everything else 0 except the words given (octal names)."""
    w = [0] * N
    w[0o25], w[0o26] = cs, 0o37000 + cs
    for k, v in set_.items():
        w[int(k[1:], 8)] = v
    return (cs, w)


class Pairing(unittest.TestCase):
    def test_same_cs_ends_pair(self):
        v = fsv.replay_verdict([state(50), state(100)], [state(50), state(100)], True, after=[state(101)])
        self.assertEqual((v['diff'], v['matched'], v['end']), ([], [50, 100], dict(shift_cs=0, paired=True)))

    def test_late_c64_end_pairs_with_the_hosts_state_there(self):
        # The host ends at 172; the C64's first sample point is 173. The
        # host's state at 173 (written past its end) is the same as the C64's.
        v = fsv.replay_verdict([state(100), state(172)], [state(100), state(173)], True,
                               after=[state(173), state(175)], after_rows=[])
        self.assertEqual(v['diff'], [])
        self.assertEqual(v['matched'], [100, 173])
        self.assertEqual(v['end'], dict(shift_cs=1, paired=True))
        self.assertEqual(v['unpaired'], [])

    def test_late_c64_end_with_a_data_difference_still_fails(self):
        v = fsv.replay_verdict([state(100), state(172)], [state(100), state(173, **{f'w{WORD:o}': 0o123})], True,
                               after=[state(173)], after_rows=[])
        self.assertEqual(v['end'], dict(shift_cs=1, paired=True))
        self.assertEqual(v['at_cs'], 173)
        self.assertEqual(v['diff'], [(f'{WORD:04o}', 0, 0o123)])

    def test_late_end_unpaired_when_an_input_comes_in_between(self):
        v = fsv.replay_verdict([state(172)], [state(173)], True, after=[state(173)], after_rows=[172])
        self.assertEqual(v['diff'], [])
        self.assertEqual(v['matched'], [])
        self.assertFalse(v['end']['paired'])
        self.assertTrue(v['end']['inputs_between'])
        self.assertEqual([(u['side'], u['cs']) for u in v['unpaired']], [('host', 172), ('C64', 173)])

    def test_late_end_unpaired_when_the_host_has_no_state_there(self):
        v = fsv.replay_verdict([state(172)], [state(174)], True, after=[state(173), state(176)], after_rows=[])
        self.assertEqual(v['end'], dict(shift_cs=2, paired=False, inputs_between=False, host_has_cs=False))

    def test_unpaired_end_is_never_a_match(self):
        r = dict(status=0, replay_ended=True, replay_differences=0, replay_matched_cs=[100], ok=False,
                 replay_end_paired=False, replay_end_shift_cs=2, reu_bounds=dict(ok=True))
        ok, text = pa.replay_result(r)
        self.assertFalse(ok)
        self.assertIn('end not compared', text)

    def test_shifted_match_is_shown(self):
        r = dict(status=0, replay_ended=True, replay_differences=0, replay_matched_cs=[100, 173],
                 replay_end_paired=True, replay_end_shift_cs=1, reu_bounds=dict(ok=True))
        self.assertEqual(pa.replay_result(r), (True, 'matches host (end compared 1 cs late)'))

    def test_unpaired_checkpoints_are_counted_in_the_result(self):
        r = dict(status=0, replay_ended=True, replay_differences=0, replay_matched_cs=[100, 300],
                 replay_end_paired=True, replay_end_shift_cs=0, replay_unpaired_count=2, reu_bounds=dict(ok=True))
        self.assertEqual(pa.replay_result(r), (True, 'matches host (2 checkpoints not compared)'))

    def test_channel_10_and_interrupt_enable_are_compared(self):
        h, c = state(100)[1], state(100)[1]
        c[2048 + fsv.END_CHANNELS.index('channel 10')] = 0o40017
        c[2048 + fsv.END_CHANNELS.index('interrupts enabled')] = 1
        self.assertEqual([x[0] for x in fsv.replay_compare(h, c, None, set())], ['channel 10', 'interrupts enabled'])

    def test_chain_window_ends_always_pair(self):
        # A window before a chain's last has no .after states: its end is
        # the next window's seam and pairs as before (timers then differ).
        v = fsv.replay_verdict([state(320)], [state(321)], True, after=None)
        self.assertTrue(v['end']['paired'])
        self.assertEqual(v['diff'][0], ('centiseconds', 320, 321))


if __name__ == '__main__':
    unittest.main()
