# Copyright 2016 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
"""Tests for head.py."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import tensorflow as tf

from tensorflow.contrib.learn.python.learn.estimators import head as head_lib


class RegressionModelHeadTest(tf.test.TestCase):

  # TODO(zakaria): test multilabel regresssion.
  def testRegression(self):
    head = head_lib._regression_head()
    with tf.Graph().as_default(), tf.Session() as sess:
      prediction = tf.constant([[1.], [1.], [3.]])
      targets = tf.constant([[0.], [1.], [1.]])
      model_fn_ops = head.head_ops({}, targets,
                                   tf.contrib.learn.ModeKeys.TRAIN,
                                   None, logits=prediction)
      self.assertAlmostEqual(5. / 3, sess.run(model_fn_ops.loss))

  def testRegressionWithWeights(self):
    head = head_lib._regression_head(
        weight_column_name="label_weight")
    with tf.Graph().as_default(), tf.Session() as sess:
      features = {"label_weight": tf.constant([[2.], [5.], [0.]])}
      prediction = tf.constant([[1.], [1.], [3.]])
      targets = tf.constant([[0.], [1.], [1.]])
      model_fn_ops = head.head_ops(features, targets,
                                   tf.contrib.learn.ModeKeys.TRAIN,
                                   None, logits=prediction)
      self.assertAlmostEqual(2. / 3, sess.run(model_fn_ops.loss), places=3)


class MultiClassModelHeadTest(tf.test.TestCase):

  def testBinaryClassification(self):
    head = head_lib._multi_class_head(n_classes=2)
    with tf.Graph().as_default(), tf.Session() as sess:
      logits = tf.constant([[1.], [1.]])
      targets = tf.constant([[1.], [0.]])
      # logloss: z:label, x:logit
      # z * -log(sigmoid(x)) + (1 - z) * -log(1 - sigmoid(x))
      model_fn_ops = head.head_ops({}, targets,
                                   tf.contrib.learn.ModeKeys.TRAIN,
                                   None, logits=logits)
      self.assertAlmostEqual(.81326163, sess.run(model_fn_ops.loss))

  def testBinaryClassificationWithWeights(self):
    head = head_lib._multi_class_head(
        n_classes=2, weight_column_name="label_weight")
    with tf.Graph().as_default(), tf.Session() as sess:
      features = {"label_weight": tf.constant([[1.], [0.]])}
      logits = tf.constant([[1.], [1.]])
      targets = tf.constant([[1.], [0.]])
      # logloss: z:label, x:logit
      # z * -log(sigmoid(x)) + (1 - z) * -log(1 - sigmoid(x))
      model_fn_ops = head.head_ops(features, targets,
                                   tf.contrib.learn.ModeKeys.TRAIN,
                                   None, logits=logits)
      self.assertAlmostEqual(.31326166 / 2, sess.run(model_fn_ops.loss))

  def testMultiClass(self):
    head = head_lib._multi_class_head(n_classes=3)
    with tf.Graph().as_default(), tf.Session() as sess:
      logits = tf.constant([[1., 0., 0.]])
      targets = tf.constant([2])
      # logloss: z:label, x:logit
      # z * -log(sigmoid(x)) + (1 - z) * -log(1 - sigmoid(x))
      model_fn_ops = head.head_ops({}, targets,
                                   tf.contrib.learn.ModeKeys.TRAIN,
                                   None, logits=logits)
      self.assertAlmostEqual(1.5514446, sess.run(model_fn_ops.loss))

  def testMultiClassWithWeight(self):
    head = head_lib._multi_class_head(
        n_classes=3, weight_column_name="label_weight")
    with tf.Graph().as_default(), tf.Session() as sess:
      features = {"label_weight": tf.constant([0.1])}
      logits = tf.constant([[1., 0., 0.]])
      targets = tf.constant([2])
      # logloss: z:label, x:logit
      # z * -log(sigmoid(x)) + (1 - z) * -log(1 - sigmoid(x))
      model_fn_ops = head.head_ops(features, targets,
                                   tf.contrib.learn.ModeKeys.TRAIN,
                                   None, logits=logits)
      self.assertAlmostEqual(.15514446, sess.run(model_fn_ops.loss))

  def testMultiClassWithInvalidNClass(self):
    try:
      head_lib._multi_class_head(n_classes=1)
      self.fail("Softmax with no n_classes did not raise error.")
    except ValueError:
      # Expected
      pass


class BinarySvmModelHeadTest(tf.test.TestCase):

  def testBinarySVMDefaultWeights(self):
    head = head_lib._binary_svm_head()
    predictions = tf.constant([[-0.5], [1.2]])
    targets = tf.constant([0, 1])
    model_fn_ops = head.head_ops({}, targets,
                                 tf.contrib.learn.ModeKeys.TRAIN,
                                 None, logits=predictions)
    # Prediction for first example is in the right side of the hyperplane (i.e.,
    # < 0) but it is within the [-1,1] margin. There is a 0.5 loss incurred by
    # this example. The 2nd prediction is outside the margin so it incurs no
    # loss at all. The overall (normalized) loss is therefore 0.5/(1+1) = 0.25.
    with tf.Session() as sess:
      self.assertAlmostEqual(0.25, sess.run(model_fn_ops.loss))

  def testBinarySVMWithWeights(self):
    head = head_lib._binary_svm_head(
        weight_column_name="weights")
    predictions = tf.constant([[-0.7], [0.2]])
    targets = tf.constant([0, 1])
    features = {"weights": tf.constant([2.0, 10.0])}
    model_fn_ops = head.head_ops(features, targets,
                                 tf.contrib.learn.ModeKeys.TRAIN,
                                 None, logits=predictions)
    # Prediction for both examples are in the right side of the hyperplane but
    # within the margin. The (weighted) loss incurred is 2*0.3=0.6 and 10*0.8=8
    # respectively. The overall (normalized) loss is therefore 8.6/12.
    with tf.Session() as sess:
      self.assertAlmostEqual(8.6 / 2, sess.run(model_fn_ops.loss), places=3)


if __name__ == "__main__":
  tf.test.main()
