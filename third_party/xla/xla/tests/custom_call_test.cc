/* Copyright 2017 The OpenXLA Authors.

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

#include <cstddef>
#include <memory>
#include <utility>

#include "absl/base/dynamic_annotations.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "xla/client/lib/constants.h"
#include "xla/client/xla_builder.h"
#include "xla/hlo/ir/hlo_computation.h"
#include "xla/hlo/ir/hlo_instruction.h"
#include "xla/hlo/ir/hlo_module.h"
#include "xla/hlo/ir/hlo_opcode.h"
#include "xla/layout_util.h"
#include "xla/literal.h"
#include "xla/literal_util.h"
#include "xla/service/custom_call_status.h"
#include "xla/service/custom_call_target_registry.h"
#include "xla/shape_util.h"
#include "xla/tests/client_library_test_base.h"
#include "xla/tests/hlo_test_base.h"
#include "xla/tests/literal_test_util.h"
#include "xla/tests/test_macros.h"
#include "xla/tests/test_utils.h"
#include "xla/xla_data.pb.h"
#include "tsl/platform/statusor.h"
#include "tsl/platform/test.h"

namespace {
void R0F32Add2(float* out, float** in) {
  ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(in, sizeof(float*));
  *out = **in + 2.0f;
}

void R2F32ReduceSum(float* out, float** in) {
  ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(in, sizeof(float) * 4);
  float* array = in[0];
  *out = array[0] + array[1] + array[2] + array[3];
}

void Add1ToValues(float* out, float** in) {
  ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(in, sizeof(float) * 4);
  float* array = in[0];
  out[0] = array[0] + 1;
  out[1] = array[1] + 1;
  out[2] = array[2] + 1;
  out[3] = array[3] + 1;
}

void F32TupleSwap(float** out, float** in) {
  ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(in[0], sizeof(float));
  ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(in[1], sizeof(float));
  *out[0] = *in[1];
  *out[1] = *in[0];
}

void R0F32Add2Succeed(float* out, float** in, XlaCustomCallStatus*) {
  ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(in, sizeof(float*));
  *out = **in + 2.0f;
  // Default state of 'status' is success.
}

void CustomCallFail(float*, float** in, XlaCustomCallStatus* status) {
  auto msg = absl::StrFormat("Failed: %.1f", in[0][0]);
  XlaCustomCallStatusSetFailure(status, msg.data(), msg.length());
}

void CustomCallFailWithBackendConfigStr(float*, float**, const char* opaque,
                                        size_t opaque_len,
                                        XlaCustomCallStatus* status) {
  ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(opaque, opaque_len);
  auto msg = absl::StrFormat("Fail with raw backend config str: %s.",
                             absl::string_view(opaque, opaque_len));
  XlaCustomCallStatusSetFailure(status, msg.data(), msg.length());
}

}  // namespace

XLA_CPU_REGISTER_CUSTOM_CALL_TARGET(R0F32Add2);
XLA_CPU_REGISTER_CUSTOM_CALL_TARGET(R2F32ReduceSum);
XLA_CPU_REGISTER_CUSTOM_CALL_TARGET(Add1ToValues);
XLA_CPU_REGISTER_CUSTOM_CALL_TARGET(F32TupleSwap);
XLA_CPU_REGISTER_CUSTOM_CALL_TARGET(R0F32Add2Succeed);
XLA_CPU_REGISTER_CUSTOM_CALL_TARGET(CustomCallFail);
XLA_CPU_REGISTER_CUSTOM_CALL_TARGET(CustomCallFailWithBackendConfigStr);

namespace xla {
namespace {

using ::testing::HasSubstr;

class CustomCallTest : public HloTestBase {
 public:
  CustomCallTest()
      : HloTestBase(),
        module_(CreateNewVerifiedModule()),
        builder_(TestName()) {}

 protected:
  // Call this function when builder_ is complete (i.e. when all instructions
  // have been added). Note that module_ is empty after calling this function.
  auto BuildAndExecute(absl::Span<Literal* const> arguments) {
    module_->AddEntryComputation(builder_.Build());
    return Execute(std::move(module_), arguments);
  }

  Shape r0f32_ = ShapeUtil::MakeShape(F32, {});
  Shape r2f32_ = ShapeUtil::MakeShape(F32, {2, 2});

  std::unique_ptr<HloModule> module_;
  HloComputation::Builder builder_;
};

XLA_TEST_F(CustomCallTest, CustomCallR0F32Add2) {
  auto constant = builder_.AddInstruction(
      HloInstruction::CreateConstant(LiteralUtil::CreateR0<float>(42.0f)));
  builder_.AddInstruction(
      HloInstruction::CreateCustomCall(r0f32_, {constant}, "R0F32Add2"));

  TF_ASSERT_OK_AND_ASSIGN(auto result, BuildAndExecute({}));
  LiteralTestUtil::ExpectR0Near<float>(44.0f, result, error_spec_);
}

XLA_TEST_F(CustomCallTest, CustomCallR2F32Reduce) {
  Array2D<float> array(2, 2);
  array(0, 0) = 1.0f;
  array(0, 1) = 2.0f;
  array(1, 0) = 3.0f;
  array(1, 1) = 4.0f;

  auto constant = builder_.AddInstruction(
      HloInstruction::CreateConstant(LiteralUtil::CreateR2FromArray2D(array)));
  builder_.AddInstruction(
      HloInstruction::CreateCustomCall(r0f32_, {constant}, "R2F32ReduceSum"));

  TF_ASSERT_OK_AND_ASSIGN(auto result, BuildAndExecute({}));
  LiteralTestUtil::ExpectR0Near<float>(10.0f, result, error_spec_);
}

XLA_TEST_F(CustomCallTest, UsedInOtherComputations) {
  auto input = builder_.AddInstruction(
      HloInstruction::CreateConstant(LiteralUtil::CreateR2FromArray2D(
          Array2D<float>{{1.0f, 2.0f}, {3.0f, 4.0f}})));
  auto incremented = builder_.AddInstruction(HloInstruction::CreateCustomCall(
      ShapeUtil::MakeShape(F32, {1, 2, 2}), {input}, "Add1ToValues"));
  auto incremented_again =
      builder_.AddInstruction(HloInstruction::CreateCustomCall(
          ShapeUtil::MakeShape(F32, {1, 2, 2}), {incremented}, "Add1ToValues"));

  // Concatenate the values along first dim.
  builder_.AddInstruction(
      HloInstruction::CreateConcatenate(ShapeUtil::MakeShape(F32, {2, 2, 2}),
                                        {incremented, incremented_again}, 0));

  TF_ASSERT_OK_AND_ASSIGN(auto result, BuildAndExecute({}));
  LiteralTestUtil::ExpectR3EqualArray3D<float>(
      Array3D<float>{{{2, 3}, {4, 5}}, {{3, 4}, {5, 6}}}, result);
}

XLA_TEST_F(CustomCallTest, InputAndOutputLayoutDiffer) {
  if (IsMlirLoweringEnabled()) {
    // The MLIR pipeline does /not/ transpose the output here, and there's no
    // obvious reason why it should.
    GTEST_SKIP() << "Appears to test an XLA current implementation detail";
  }

  auto input =
      builder_.AddInstruction(HloInstruction::CreateParameter(0, r2f32_, "p"));
  builder_.AddInstruction(
      HloInstruction::CreateCustomCall(r2f32_, {input}, "Add1ToValues"));

  module_->AddEntryComputation(builder_.Build());
  ForceParameterLayout(module_.get(), 0, LayoutUtil::MakeLayout({1, 0}));
  ForceResultLayout(module_.get(), LayoutUtil::MakeLayout({0, 1}));

  Literal argument = LiteralUtil::CreateR2<float>({{1.f, 2.f}, {3.f, 4.f}});

  // Note, the expected result is transposed! This is because the input and
  // output layouts of the custom call differ and the called function just
  // blindly adds one to each element.
  TF_ASSERT_OK_AND_ASSIGN(auto result,
                          Execute(std::move(module_), {&argument}));
  LiteralTestUtil::ExpectR2Equal<float>({{2.f, 4.f}, {3.f, 5.f}}, result);
}

XLA_TEST_F(CustomCallTest, LayoutConstrained) {
  // The argument and result of the computation are set to different layouts,
  // but the custom call is layout constrained to a fixed operand and result
  // layout, so the correct result should be produced.
  auto input =
      builder_.AddInstruction(HloInstruction::CreateParameter(0, r2f32_, "p"));

  const Shape& r2f32_dim0_major =
      ShapeUtil::MakeShapeWithDenseLayout(F32, {2, 2}, {1, 0});
  auto custom_call = builder_.AddInstruction(HloInstruction::CreateCustomCall(
      r2f32_dim0_major, {input}, "Add1ToValues", {r2f32_dim0_major}));
  builder_.AddInstruction(
      custom_call->CloneWithNewOperands(r2f32_dim0_major, {custom_call}));

  module_->AddEntryComputation(builder_.Build());
  ForceParameterLayout(module_.get(), 0, LayoutUtil::MakeLayout({1, 0}));
  ForceResultLayout(module_.get(), LayoutUtil::MakeLayout({0, 1}));

  Literal argument = LiteralUtil::CreateR2<float>({{1.f, 2.f}, {3.f, 4.f}});

  TF_ASSERT_OK_AND_ASSIGN(auto result,
                          Execute(std::move(module_), {&argument}));
  LiteralTestUtil::ExpectR2Equal<float>({{3.f, 4.f}, {5.f, 6.f}}, result);
}

XLA_TEST_F(CustomCallTest, TupleOutput) {
  const char* kModuleStr = R"(
    HloModule m
    test {
      p0 = f32[] parameter(0)
      p1 = f32[] parameter(1)
      ROOT %custom-call = (f32[], f32[]) custom-call(f32[] %p0, f32[] %p1), custom_call_target="F32TupleSwap", operand_layout_constraints={f32[], f32[]}
    }
  )";
  TF_ASSERT_OK_AND_ASSIGN(auto module,
                          ParseAndReturnVerifiedModule(kModuleStr));

  Literal arg0 = LiteralUtil::CreateR0<float>(7.f);
  Literal arg1 = LiteralUtil::CreateR0<float>(42.f);

  Literal expected = LiteralUtil::MakeTuple({&arg1, &arg0});
  TF_ASSERT_OK_AND_ASSIGN(auto result,
                          Execute(std::move(module), {&arg0, &arg1}));
  EXPECT_EQ(result, expected);
}

XLA_TEST_F(CustomCallTest, ReportsSuccess) {
  auto constant = builder_.AddInstruction(
      HloInstruction::CreateConstant(LiteralUtil::CreateR0<float>(42.0f)));
  builder_.AddInstruction(HloInstruction::CreateCustomCall(
      r0f32_, {constant}, "R0F32Add2Succeed",
      /*opaque=*/"", CustomCallApiVersion::API_VERSION_STATUS_RETURNING));

  TF_ASSERT_OK_AND_ASSIGN(auto result, BuildAndExecute({}));
  LiteralTestUtil::ExpectR0Near<float>(44.0f, result, error_spec_);
}

XLA_TEST_F(CustomCallTest, ReportsFailure) {
  auto constant = builder_.AddInstruction(
      HloInstruction::CreateConstant(LiteralUtil::CreateR0<float>(42.0f)));
  builder_.AddInstruction(HloInstruction::CreateCustomCall(
      ShapeUtil::MakeShape(F32, {}), {constant}, "CustomCallFail",
      /*opaque=*/"", CustomCallApiVersion::API_VERSION_STATUS_RETURNING));

  auto status = BuildAndExecute({}).status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.message(), ::testing::HasSubstr("Failed: 42.0"));
}

XLA_TEST_F(CustomCallTest, ReportsFirstFailure) {
  auto constant_1 = builder_.AddInstruction(
      HloInstruction::CreateConstant(LiteralUtil::CreateR0<float>(1.0f)));
  auto constant_2 = builder_.AddInstruction(
      HloInstruction::CreateConstant(LiteralUtil::CreateR0<float>(2.0f)));
  auto res_1 = builder_.AddInstruction(HloInstruction::CreateCustomCall(
      ShapeUtil::MakeShape(F32, {}), {constant_1}, "CustomCallFail",
      /*opaque=*/"", CustomCallApiVersion::API_VERSION_STATUS_RETURNING));
  auto res_2 = builder_.AddInstruction(HloInstruction::CreateCustomCall(
      ShapeUtil::MakeShape(F32, {}), {constant_2}, "CustomCallFail",
      /*opaque=*/"", CustomCallApiVersion::API_VERSION_STATUS_RETURNING));
  builder_.AddInstruction(HloInstruction::CreateBinary(
      ShapeUtil::MakeShape(F32, {}), HloOpcode::kAdd, res_1, res_2));

  auto status = BuildAndExecute({}).status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.message(), ::testing::HasSubstr("Failed: 1.0"));
}

XLA_TEST_F(CustomCallTest, TransitiveCustomCallReportsFirstFailure) {
  const char* const kModuleStr = R"(
    HloModule m
    sub {
      p0 = f32[] parameter(0)
      ROOT custom-call = f32[] custom-call(f32[] %p0), custom_call_target="CustomCallFail", api_version=API_VERSION_STATUS_RETURNING
    }
    ENTRY test {
      c0 = f32[] constant(1.0)
      c1 = f32[] constant(2.0)
      call0 = f32[] call(f32[] %c0), to_apply=sub
      call1 = f32[] call(f32[] %c1), to_apply=sub
      ROOT sum = f32[] add(%call0, %call1)
    }
  )";
  TF_ASSERT_OK_AND_ASSIGN(auto module,
                          ParseAndReturnVerifiedModule(kModuleStr));

  auto status = Execute(std::move(module), {}).status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.message(), HasSubstr("Failed: 1.0"));
}

XLA_TEST_F(CustomCallTest, FillStatusMsgWithBackendConfigStr) {
  if (IsMlirLoweringEnabled()) {
    GTEST_SKIP() << "Invalid values unsupported by MLIR";
  }

  const char* const kModuleStr = R"(
    HloModule m
    ENTRY test {
      c0 = f32[] constant(1.0)
      ROOT dummy-result = f32[] custom-call(f32[] %c0),
                                custom_call_target="CustomCallFailWithBackendConfigStr",
                                backend_config="foo",
                                api_version=API_VERSION_STATUS_RETURNING_UNIFIED
    }
  )";
  TF_ASSERT_OK_AND_ASSIGN(auto module,
                          ParseAndReturnVerifiedModule(kModuleStr));

  auto status = Execute(std::move(module), {}).status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.message(),
              HasSubstr("Fail with raw backend config str: foo"));
}

class CustomCallClientAPITest : public ClientLibraryTestBase {};

// When using the client API, CustomCall targets can't begin with '$' -- these
// are reserved for internal use.
XLA_TEST_F(CustomCallClientAPITest, IllegalCustomCallTarget) {
  XlaBuilder builder(TestName());
  CustomCall(&builder, "$illegal", /*operands=*/{},
             ShapeUtil::MakeShape(F32, {1}));

  absl::StatusOr<std::unique_ptr<GlobalData>> result =
      Execute(&builder, /*arguments=*/{});
  EXPECT_FALSE(result.ok());
}

}  // namespace
}  // namespace xla
