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
        self.suffix = '.kct'

    def setUp(self):
        self.tempkc = tempfile.mkstemp(self.suffix)
        self.d = yakc.KyotoDB(self.tempkc[1])
    
    def test_setget(self):
        self.d['x'] = 123
        self.d['y'] = ['456', '789']
        self.d[45] = 35
        self.assertEqual(123, self.d['x'])
        self.assertEqual(['456', '789'], self.d['y'])
        self.assertEqual(35, self.d[45])

        exception_raised = False
        try:
            dummy = self.d['test']
        except KeyError as e:
            exception_raised = True
        self.assertEqual(True, exception_raised)

    def test_keys(self):
        self.d[1] = 6
        self.d['k'] = [1,2,3]
        self.assertEqual([1, 'k'], self.d.keys())

    def test_items(self):
        self.d[1] = 6
        self.d['k'] = [1,2,3]
        self.assertEqual([(1, 6), ('k', [1,2,3])], self.d.items())

    def test_values(self):
        self.d[1] = 6
        self.d['k'] = [1,2,3]
        self.assertEqual([6, [1,2,3]], self.d.values())

    def test_iterkeys(self):
        self.d[1] = 6
        self.d['k'] = [1,2,3]
        self.assertEqual([1, 'k'], list(self.d.iterkeys()))

    def test_iteritems(self):
        self.d[1] = 6
        self.d['k'] = [1,2,3]
        self.assertEqual([(1, 6), ('k', [1,2,3])], list(self.d.iteritems()))

    def test_itervalues(self):
        self.d[1] = 6
        self.d['k'] = [1,2,3]
        self.assertEqual([6, [1,2,3]], list(self.d.itervalues()))

    def test_clear(self):
        self.d[1] = 1
        self.d['1'] = []
        self.d.clear()
        self.assertEqual([], self.d.keys())

    def test_contains(self):
        self.d['a'] = 34
        self.d[98] = []
        self.assertEqual(True, 'a' in self.d)
        self.assertEqual(True, 98 in self.d)
        self.assertEqual(False, 'pen' in self.d)
        self.assertEqual(False, 203 in self.d)

    def test_haskey(self):
        self.d['a'] = 34
        self.d[98] = []
        self.assertEqual(True, self.d.has_key('a'))
        self.assertEqual(True, self.d.has_key(98))
        self.assertEqual(False, self.d.has_key('pen'))
        self.assertEqual(False, self.d.has_key(2))

    def test_path(self):
        self.assertEqual(self.tempkc[1], self.d.path())

    def test_pop(self):
        self.d[1] = 1
        self.d['k'] = set([1,2,3])
        self.assertEqual(set([1,2,3]), self.d.pop('k'))
        self.assertEqual([1], self.d.keys())
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
