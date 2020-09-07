/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/lite/kernels/internal/reference/neg.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/neg.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace ops {
namespace micro {
namespace neg {

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

struct OpData {
  // sum of input and output zero points, accumulated on int16
  // highest possible value: 127 + 127 = 254
  // lowest possible value: -128 + - 128 = -256
  // thus, accumulate on int16
  int16_t zero_points_sum;
};

TfLiteStatus CalculateOpDataInt8(TfLiteContext* context,
                                 const TfLiteTensor* input,
                                 const TfLiteTensor* output, OpData* data) {
  constexpr auto kI8Min =
      static_cast<int16_t>(std::numeric_limits<int8_t>::min());
  constexpr auto kI8Max =
      static_cast<int16_t>(std::numeric_limits<int8_t>::max());

  // using EQ attempts to cast to int via the %d format specifier and gives
  // incorrect value and since some hardware platforms do not support float
  // formatting by default, I did a direct equality check instead
  TF_LITE_ENSURE(context, input->params.scale == output->params.scale);

  // within: [-128, 127]
  TF_LITE_ENSURE(context, input->params.zero_point <= kI8Max);
  TF_LITE_ENSURE(context, input->params.zero_point >= kI8Min);

  // within: [-128, 127]
  TF_LITE_ENSURE(context, output->params.zero_point <= kI8Max);
  TF_LITE_ENSURE(context, output->params.zero_point >= kI8Min);

  data->zero_points_sum = static_cast<int16_t>(input->params.zero_point) +
                          static_cast<int16_t>(output->params.zero_point);

  return kTfLiteOk;
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  const TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  TF_LITE_ENSURE_EQ(context, output->type, input->type);
  TF_LITE_ENSURE_EQ(context, output->dims->size, input->dims->size);
  for (int i = 0; i < output->dims->size; ++i) {
    TF_LITE_ENSURE_EQ(context, output->dims->data[i], input->dims->data[i]);
  }
  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);
  switch (input->type) {
    case kTfLiteInt8: {
      OpData op_data;
      TF_LITE_ENSURE_STATUS(
          CalculateOpDataInt8(context, input, output, &op_data));
      reference_integer_ops::Negate(
          GetTensorShape(input), GetTensorData<int8_t>(input),
          GetTensorShape(output), GetTensorData<int8_t>(output),
          op_data.zero_points_sum);
    } break;

    case kTfLiteFloat32:
      reference_ops::Negate(tflite::micro::GetTensorShape(input),
                            tflite::micro::GetTensorData<float>(input),
                            tflite::micro::GetTensorShape(output),
                            tflite::micro::GetTensorData<float>(output));
      break;

    default:
      TF_LITE_KERNEL_LOG(context,
                         "Type %s (%d) is not supported by 'Negate' kernel.",
                         TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace neg

<<<<<<< HEAD
TfLiteRegistration* Register_NEG() {
  static TfLiteRegistration r = {};
  r.prepare = neg::Prepare;
  r.invoke = neg::Eval;
  return &r;
=======
TfLiteRegistration Register_NEG() {
  return {/*init=*/nullptr,
          /*free=*/nullptr,
          /*prepare=*/nullptr,
          /*invoke=*/neg::Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
>>>>>>> bb49eafc080caf205d5ba4478d9c93552ce46d57
}

}  // namespace micro
}  // namespace ops
}  // namespace tflite
