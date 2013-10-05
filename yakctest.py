#!/usr/bin/env python

import unittest
import subprocess
import tempfile
import os
import yakc

class KyotoCabinetTest(unittest.TestCase):
    """
    Test cases for yet another kyoto cabinet
    """

    def setUp(self):
        self.tempkc = tempfile.mkstemp('.kch')
        #print self.tempkc
        subprocess.check_call(['kctreemgr', 'create', self.tempkc[1]])
        subprocess.check_call(['kctreemgr', 'setbulk', self.tempkc[1],
                               'a', '123',
                               'b', '456',
                               'this', 'is',
                               'which', 'pair'])
    
    def test_get(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual('123', d['a'])
        self.assertEqual('456', d['b'])
        self.assertEqual('is', d['this'])
        self.assertEqual('pair', d['which'])

        exception_raised = False
        try:
            dummy = d['test']
        except KeyError as e:
            exception_raised = True
        self.assertEqual(True, exception_raised)
            

    def test_set(self):
        d = yakc.KyotoDB(self.tempkc[1])
        d['x'] = '123'
        d['y'] = '456'
        self.assertEqual('123', d['x'])
        self.assertEqual('456', d['y'])
        d.close()

        p = subprocess.check_output(['kctreemgr', 'getbulk', self.tempkc[1], 'x', 'y'])
        self.assertEqual('x\t123\ny\t456\n', p)


    def test_keys(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual(['a', 'b', 'this', 'which'], d.keys())

    def test_items(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual([('a', '123'), ('b', '456'),
                          ('this', 'is'), ('which', 'pair')], d.items())

    def test_values(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual(['123', '456',
                          'is', 'pair'], d.values())

    def test_iterkeys(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual(['a', 'b', 'this', 'which'], list(d.iterkeys()))

    def test_iteritems(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual([('a', '123'), ('b', '456'),
                          ('this', 'is'), ('which', 'pair')], list(d.iteritems()))

    def test_itervalues(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual(['123', '456',
                          'is', 'pair'], list(d.itervalues()))

    def test_clear(self):
        d = yakc.KyotoDB(self.tempkc[1])
        d.clear()
        self.assertEqual([], d.keys())

    def test_contains(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual(True, 'a' in d)
        self.assertEqual(True, 'this' in d)
        self.assertEqual(False, 'pen' in d)
        self.assertEqual(False, 'OK' in d)

    def test_haskey(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual(True, d.has_key('a'))
        self.assertEqual(True, d.has_key('this'))
        self.assertEqual(False, d.has_key('pen'))
        self.assertEqual(False, d.has_key('OK'))

    def test_path(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual(self.tempkc[1], d.path())

    def test_pop(self):
        d = yakc.KyotoDB(self.tempkc[1])
        self.assertEqual('123', d.pop('a'))
        self.assertEqual(['b', 'this', 'which'], d.keys())
        self.assertEqual('abc', d.pop('a', 'abc'))

        exception_raised = False
        try:
            dummy = d.pop('test')
        except KeyError as e:
            exception_raised = True
        self.assertEqual(True, exception_raised)

    def test_update(self):
        d = yakc.KyotoDB(self.tempkc[1])
        d.update({'xx': 'yy', 'yy': 'zz'})
        self.assertEqual('yy', d['xx'])
        self.assertEqual('zz', d['yy'])

        d.update([('123', '456'), ('789', '000')])
        self.assertEqual('456', d['123'])
        self.assertEqual('000', d['789'])

        d.update(sight='view', ski='snow')
        self.assertEqual('view', d['sight'])
        self.assertEqual('snow', d['ski'])


    def tearDown(self):
        os.remove(self.tempkc[1])

    
if __name__ == '__main__':
    unittest.main()
