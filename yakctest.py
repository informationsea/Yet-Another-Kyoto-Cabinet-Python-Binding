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

    def __init__(self, *vargs, **kwds):
        unittest.TestCase.__init__(self, *vargs, **kwds)
        self.mgr = 'kctreemgr'
        self.suffix = '.kct'

    def setUp(self):
        self.tempkc = tempfile.mkstemp(self.suffix)
        #print self.tempkc
        subprocess.check_call([self.mgr, 'create', self.tempkc[1]])
        subprocess.check_call([self.mgr, 'setbulk', self.tempkc[1],
                               'a', '123',
                               'b', '456',
                               'this', 'is',
                               'which', 'pair'])
        self.d = yakc.KyotoDB(self.tempkc[1])
    
    def test_get(self):
        self.assertEqual('123', self.d['a'])
        self.assertEqual('456', self.d['b'])
        self.assertEqual('is', self.d['this'])
        self.assertEqual('pair', self.d['which'])

        exception_raised = False
        try:
            dummy = self.d['test']
        except KeyError as e:
            exception_raised = True
        self.assertEqual(True, exception_raised)
            

    def test_set(self):
        self.d['x'] = '123'
        self.d['y'] = '456'
        self.assertEqual('123', self.d['x'])
        self.assertEqual('456', self.d['y'])
        self.d.close()

        p = subprocess.check_output([self.mgr, 'getbulk', self.tempkc[1], 'x', 'y'])
        self.assertEqual('x\t123\ny\t456\n', p)


    def test_keys(self):
        self.assertEqual(['a', 'b', 'this', 'which'], self.d.keys())

    def test_items(self):
        self.assertEqual([('a', '123'), ('b', '456'),
                          ('this', 'is'), ('which', 'pair')], self.d.items())

    def test_values(self):
        self.assertEqual(['123', '456',
                          'is', 'pair'], self.d.values())

    def test_iterkeys(self):
        self.assertEqual(['a', 'b', 'this', 'which'], list(self.d.iterkeys()))

    def test_iteritems(self):
        self.assertEqual([('a', '123'), ('b', '456'),
                          ('this', 'is'), ('which', 'pair')],
                         list(self.d.iteritems()))

    def test_itervalues(self):
        self.assertEqual(['123', '456',
                          'is', 'pair'], list(self.d.itervalues()))

    def test_clear(self):
        self.d.clear()
        self.assertEqual([], self.d.keys())

    def test_contains(self):
        self.assertEqual(True, 'a' in self.d)
        self.assertEqual(True, 'this' in self.d)
        self.assertEqual(False, 'pen' in self.d)
        self.assertEqual(False, 'OK' in self.d)

    def test_haskey(self):
        self.assertEqual(True, self.d.has_key('a'))
        self.assertEqual(True, self.d.has_key('this'))
        self.assertEqual(False, self.d.has_key('pen'))
        self.assertEqual(False, self.d.has_key('OK'))

    def test_path(self):
        self.assertEqual(self.tempkc[1], self.d.path())

    def test_pop(self):
        self.assertEqual('123', self.d.pop('a'))
        self.assertEqual(['b', 'this', 'which'], self.d.keys())
        self.assertEqual('abc', self.d.pop('a', 'abc'))

        exception_raised = False
        try:
            dummy = self.d.pop('test')
        except KeyError as e:
            exception_raised = True
        self.assertEqual(True, exception_raised)

    def test_update(self):
        self.d.update({'xx': 'yy', 'yy': 'zz'})
        self.assertEqual('yy', self.d['xx'])
        self.assertEqual('zz', self.d['yy'])

        self.d.update([('123', '456'), ('789', '000')])
        self.assertEqual('456', self.d['123'])
        self.assertEqual('000', self.d['789'])

        self.d.update(sight='view', ski='snow')
        self.assertEqual('view', self.d['sight'])
        self.assertEqual('snow', self.d['ski'])


    def tearDown(self):
        self.d.close()
        os.remove(self.tempkc[1])

    
class KyotoCabinetHashDb(KyotoCabinetTest):
    """
    """
    
    def __init__(self, *vargs, **kwds):
        KyotoCabinetTest.__init__(self, *vargs, **kwds)
        self.mgr = 'kchashmgr'
        self.suffix = '.kch'
        

if __name__ == '__main__':
    unittest.main()
