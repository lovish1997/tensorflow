// Copyright 2024 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "tensorflow/lite/experimental/litert/vendors/qualcomm/qnn_tensor.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "third_party/qairt/latest/include/QNN/QnnTypes.h"

namespace litert {
namespace qnn {

QnnTensor::QnnTensor(const QnnTensor& other) : QnnTensor(other.Tensor()) {
  auto status = DeepCopy();
  // This should never fail because the input QnnTensor was already deep-copied.
  if (!status.ok()) {
    ABSL_LOG(ERROR) << "Failed to build QnnTensor: " << status;
    ABSL_CHECK_OK(status);
  }
}

QnnTensor::QnnTensor(QnnTensor&& other) {
  tensor_ = other.tensor_;
  // Swap managed memory.
  std::swap(name_, other.name_);
  std::swap(dimensions_, other.dimensions_);
  std::swap(is_dynamic_dimensions_, other.is_dynamic_dimensions_);
}

absl::StatusOr<QnnTensor> QnnTensor::Create(const Qnn_Tensor_t& tensor) {
  QnnTensor qnn_tensor(tensor);
  if (auto status = qnn_tensor.DeepCopy(); !status.ok()) {
    return status;
  }
  return qnn_tensor;
}

absl::Status QnnTensor::DeepCopy() {
  if (tensor_.version == QNN_TENSOR_VERSION_1) {
    dimensions_.reserve(tensor_.v1.rank);
    std::copy(tensor_.v1.dimensions, tensor_.v1.dimensions + tensor_.v1.rank,
              std::back_inserter(dimensions_));
    tensor_.v1.dimensions = dimensions_.data();

    // FIXME: Implement deep copy for quantizeParams.
    if (tensor_.v1.quantizeParams.quantizationEncoding ==
            QNN_QUANTIZATION_ENCODING_BLOCKWISE_EXPANSION ||
        tensor_.v1.quantizeParams.quantizationEncoding ==
            QNN_QUANTIZATION_ENCODING_VECTOR) {
      return absl::InternalError("Unsupported QNN quantization");
    }

  } else if (tensor_.version == QNN_TENSOR_VERSION_2) {
    dimensions_.reserve(tensor_.v2.rank);
    std::copy(tensor_.v2.dimensions, tensor_.v2.dimensions + tensor_.v2.rank,
              std::back_inserter(dimensions_));
    tensor_.v2.dimensions = dimensions_.data();

    if (tensor_.v2.isDynamicDimensions) {
      is_dynamic_dimensions_.reserve(tensor_.v2.rank);
      std::copy(tensor_.v2.isDynamicDimensions,
                tensor_.v2.isDynamicDimensions + tensor_.v2.rank,
                std::back_inserter(is_dynamic_dimensions_));
      tensor_.v2.isDynamicDimensions = is_dynamic_dimensions_.data();
    }

    // FIXME: Implement deep copy for quantizeParams.
    if (tensor_.v2.quantizeParams.quantizationEncoding ==
            QNN_QUANTIZATION_ENCODING_BLOCKWISE_EXPANSION ||
        tensor_.v2.quantizeParams.quantizationEncoding ==
            QNN_QUANTIZATION_ENCODING_VECTOR) {
      return absl::InternalError("Unsupported QNN quantization");
    }

  } else {
    return absl::InternalError("Unsupported QNN tensor version");
  }

  return {};
}

}  // namespace qnn
}  // namespace litert
