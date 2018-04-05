/* Copyright 2017 Graphcore Ltd
 */

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

#ifndef TENSORFLOW_COMPILER_PLUGIN_POPLAR_DRIVER_COMPILER_RESOURCES_H_
#define TENSORFLOW_COMPILER_PLUGIN_POPLAR_DRIVER_COMPILER_RESOURCES_H_

#include "tensorflow/compiler/plugin/poplar/driver/allocation_finder.h"
#include "tensorflow/compiler/plugin/poplar/driver/inplace_finder.h"
#include "tensorflow/compiler/plugin/poplar/driver/visitor_subcomputation.h"

#include <popconv/Convolution.hpp>
#include <poplin/MatMul.hpp>
#include <poprand/RandomGen.hpp>

namespace xla {
namespace poplarplugin {

using ComputationMap = std::map<const HloComputation*, SubComputationVisitor>;

struct CompilerResources {
  ComputationMap computation_map;

  TensorAllocationMap tensor_allocation_map;

  InplaceInstructions inplace_instructions;

  popconv::PlanningCache convolution_cache;

  poplin::PlanningCache dot_cache;

  poprand::Random random;

  CompilerResources(uint64 seed, poprand::RandomGenMode mode) :
          random(mode, seed) {}
};

}  // namespace poplarplugin
}  // namespace xla

#endif
