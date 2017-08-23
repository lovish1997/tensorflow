/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef THIRD_PARTY_TENSORFLOW_C_C_TEST_UTIL_H_
#define THIRD_PARTY_TENSORFLOW_C_C_TEST_UTIL_H_

#include "tensorflow/c/c_api.h"

#include <vector>

#include "tensorflow/core/framework/attr_value.pb.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/node_def.pb.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/types.pb.h"
#include "tensorflow/core/platform/test.h"

using ::tensorflow::string;

typedef std::unique_ptr<TF_Tensor, decltype(&TF_DeleteTensor)>
    unique_tensor_ptr;

// Create a tensor with values of type TF_INT8 provided by `values`.
TF_Tensor* Int8Tensor(const int64_t* dims, int num_dims, const char* values);

TF_Tensor* Int32Tensor(int32_t v, const tensorflow::TensorShape shape = {});

TF_Operation* Placeholder(TF_Graph* graph, TF_Status* s,
                          const char* name = "feed",
                          const tensorflow::PartialTensorShape shape = tensorflow::PartialTensorShape());

TF_Operation* Const(TF_Tensor* t, TF_Graph* graph, TF_Status* s,
                    const char* name = "const");

TF_Operation* ScalarConst(int32_t v, TF_Graph* graph, TF_Status* s,
                          const char* name = "scalar",
                          const tensorflow::TensorShape shape = {});

TF_Operation* Add(TF_Operation* l, TF_Operation* r, TF_Graph* graph,
                  TF_Status* s, const char* name = "add");

TF_Operation* Add(TF_Output l, TF_Output r, TF_Graph* graph, TF_Status* s,
                  const char* name = "add");

TF_Operation* Neg(TF_Operation* n, TF_Graph* graph, TF_Status* s);

TF_Operation* LessThan(TF_Output l, TF_Output r, TF_Graph* graph, TF_Status* s);

bool IsPlaceholder(const tensorflow::NodeDef& node_def);

bool IsScalarConst(const tensorflow::NodeDef& node_def, int v);

bool IsAddN(const tensorflow::NodeDef& node_def, int n);

bool IsNeg(const tensorflow::NodeDef& node_def, const string& input);

bool GetGraphDef(TF_Graph* graph, tensorflow::GraphDef* graph_def);

bool GetNodeDef(TF_Operation* oper, tensorflow::NodeDef* node_def);

bool GetAttrValue(TF_Operation* oper, const char* attr_name,
                  tensorflow::AttrValue* attr_value, TF_Status* s);

class CSession {
 public:
  CSession(TF_Graph* graph, TF_Status* s);
  explicit CSession(TF_Session* session);

  ~CSession();

  void SetInputs(std::vector<std::pair<TF_Operation*, TF_Tensor*>> inputs);
  void SetOutputs(std::initializer_list<TF_Operation*> outputs);
  void SetOutputs(const std::vector<TF_Output>& outputs);
  void SetTargets(std::initializer_list<TF_Operation*> targets);

  void Run(TF_Status* s);

  void CloseAndDelete(TF_Status* s);

  TF_Tensor* output_tensor(int i) { return output_values_[i]; }

 private:
  void DeleteInputValues();
  void ResetOutputValues();

  TF_Session* session_;
  std::vector<TF_Output> inputs_;
  std::vector<TF_Tensor*> input_values_;
  std::vector<TF_Output> outputs_;
  std::vector<TF_Tensor*> output_values_;
  std::vector<TF_Operation*> targets_;
};

#endif  // THIRD_PARTY_TENSORFLOW_C_C_TEST_UTIL_H_
