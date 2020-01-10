"""Tests for directory_watcher."""

import os
import shutil

from tensorflow.python.framework import test_util
from tensorflow.python.platform import googletest
from tensorflow.python.summary.impl import directory_watcher


class _ByteLoader(object):
  """A loader that loads individual bytes from a file."""

  def __init__(self, path):
    self._f = open(path)

  def Load(self):
    while True:
      byte = self._f.read(1)
      if byte:
        yield byte
      else:
        return


class DirectoryWatcherTest(test_util.TensorFlowTestCase):

  def setUp(self):
    # Put everything in a directory so it's easier to delete.
    self._directory = os.path.join(self.get_temp_dir(), 'monitor_dir')
    os.mkdir(self._directory)
    self._watcher = directory_watcher.DirectoryWatcher(
        self._directory, _ByteLoader)

  def tearDown(self):
    shutil.rmtree(self._directory)

  def _WriteToFile(self, filename, data):
    path = os.path.join(self._directory, filename)
    with open(path, 'a') as f:
      f.write(data)

  def assertWatcherYields(self, values):
    self.assertEqual(list(self._watcher.Load()), values)

  def testRaisesWithBadArguments(self):
    with self.assertRaises(ValueError):
      directory_watcher.DirectoryWatcher(None, lambda x: [])
    with self.assertRaises(ValueError):
      directory_watcher.DirectoryWatcher('asdf', None)

  def testEmptyDirectory(self):
    self.assertWatcherYields([])

  def testSingleWrite(self):
    self._WriteToFile('a', 'abc')
    self.assertWatcherYields(['a', 'b', 'c'])

  def testMultipleWrites(self):
    self._WriteToFile('a', 'abc')
    self.assertWatcherYields(['a', 'b', 'c'])
    self._WriteToFile('a', 'xyz')
    self.assertWatcherYields(['x', 'y', 'z'])

  def testMultipleLoads(self):
    self._WriteToFile('a', 'a')
    self._watcher.Load()
    self._watcher.Load()
    self.assertWatcherYields(['a'])

  def testMultipleFilesAtOnce(self):
    self._WriteToFile('b', 'b')
    self._WriteToFile('a', 'a')
    self.assertWatcherYields(['a', 'b'])

  def testFinishesLoadingFileWhenSwitchingToNewFile(self):
    self._WriteToFile('a', 'a')
    # Empty the iterator.
    self.assertEquals(['a'], list(self._watcher.Load()))
    self._WriteToFile('a', 'b')
    self._WriteToFile('b', 'c')
    # The watcher should finish its current file before starting a new one.
    self.assertWatcherYields(['b', 'c'])

  def testIntermediateEmptyFiles(self):
    self._WriteToFile('a', 'a')
    self._WriteToFile('b', '')
    self._WriteToFile('c', 'c')
    self.assertWatcherYields(['a', 'c'])

  def testFileFilter(self):
    self._watcher = directory_watcher.DirectoryWatcher(
        self._directory, _ByteLoader,
        path_filter=lambda path: 'do_not_watch_me' not in path)

    self._WriteToFile('a', 'a')
    self._WriteToFile('do_not_watch_me', 'b')
    self._WriteToFile('c', 'c')
    self.assertWatcherYields(['a', 'c'])

if __name__ == '__main__':
  googletest.main()
