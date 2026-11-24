#!/usr/bin/env python3
"""Shared scheduler decoding preserves wide words, masks and serial replay."""
import unittest

import test_simultaneous_scan_chains as cli


class WordModelDecoding(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = cli.SimultaneousChains.run_tool
    replay = cli.SimultaneousChains.replay

    def test_primary_words_keep_width_and_leading_zeros(self):
        for width in (1, 31, 32, 33, 63, 64, 65, 127, 128, 1024):
            with self.subTest(width=width):
                source = ('Module Chains {DataInPort p[%d:0]; '
                          'DataOutPort q[%d:0] {Source p;}}' % (width-1, width-1))
                values = ('0'*width, '0'*(width-1)+'1', '1'+'0'*(width-1))
                commands = 'iPDLLevel 1\niProcsForModule Chains\n'
                for bits in values:
                    commands += 'iWrite p 0b%s; iApply; iRead q 0b%s; iApply\n' % (bits, bits)
                output = self.replay(source, commands)
                for bits in values:
                    self.assertIn(('iWrite p 0b'+bits+'\n').encode(), output)
                    self.assertIn(('iRead q 0b'+bits+'\n').encode(), output)

    def test_unknown_held_state_is_not_filled_or_reversed(self):
        for width in (1, 33, 65, 129, 1024):
            with self.subTest(width=width):
                source = ('Module Chains {DataInPort p; '
                          'DataRegister value[%d:0] {WriteCallBack Chains unused <D> <R>;} '
                          'DataOutPort q[%d:0] {Source value;}}' % (width-1, width-1))
                commands = ('iPDLLevel 1\niProcsForModule Chains\n'
                            'iProc unused {data reg} {error unexpected_callback}\n')
                for mask in ('x'*width, ('x10'*width)[:width], ('01x'*width)[:width]):
                    commands += 'iState value 0b%s; iWrite p 1; iApply; iRead q; iApply\n' % mask
                output = self.replay(source, commands)
                self.assertEqual(output.count(b'iWrite p 0b1\n'), 3)

    def test_unequal_serial_words_and_repeated_updates(self):
        for width in (33, 65, 129):
            with self.subTest(width=width):
                source = cli.NETWORK.replace('a[3:0]', 'a[%d:0]' % (width-1))
                source = source.replace("ResetValue 4'b0", "ResetValue %d'b0" % width)
                commands = 'iPDLLevel 1\niProcsForModule Chains\niReset\n'
                for bits in ('0'*(width-1)+'1', '1'+'0'*(width-1), '0'*width):
                    commands += ('iWrite a 0b%s; iWrite b 2; iApply -together; '
                                 'iRead a 0b%s; iRead b 2; iApply -together\n' % (bits, bits))
                output = self.replay(source, commands)
                self.assertIn(('iWrite a 0b'+'1'+'0'*(width-1)+'\n').encode(), output)


if __name__ == '__main__':
    unittest.main()
