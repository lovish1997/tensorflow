/* Copyright 2024 The OpenXLA Authors.

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

#include "xla/stream_executor/cuda/ptx_compiler.h"

#include <sys/types.h>

#include <cstdint>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "xla/stream_executor/cuda/ptx_compiler_support.h"
#include "xla/stream_executor/device_description.h"
#include "xla/stream_executor/gpu/gpu_asm_opts.h"
#include "xla/stream_executor/semantic_version.h"
#include "tsl/platform/status_matchers.h"
#include "tsl/platform/test.h"

namespace {

// Generated by the following command:
//
// echo "__global__ void kernel(int* in) { for (int i=0; i < 16; i++) \
// { in[i] += i; } for (int i=0; i < 16; i++) { in[15-i] += i; }}" \
// | nvcc -o - -rdc true --ptx --x cu - -O0
//
// The `.maxnreg` directive was added manually afterwards.
constexpr const char kSpillingPtx[] = R"(
//
// Generated by NVIDIA NVVM Compiler
//
// Compiler Build ID: CL-32267302
// Cuda compilation tools, release 12.0, V12.0.140
// Based on NVVM 7.0.1
//

.version 8.0
.target sm_52
.address_size 64

        // .globl       _Z6kernelPi

.visible .entry _Z6kernelPi(
        .param .u64 _Z6kernelPi_param_0
)
        .maxnreg 16
{
        .reg .b32       %r<33>;
        .reg .b64       %rd<3>;


        ld.param.u64    %rd1, [_Z6kernelPi_param_0];
        cvta.to.global.u64      %rd2, %rd1;
        ld.global.u32   %r1, [%rd2+4];
        ld.global.u32   %r2, [%rd2+8];
        ld.global.u32   %r3, [%rd2+12];
        ld.global.u32   %r4, [%rd2+16];
        ld.global.u32   %r5, [%rd2+20];
        ld.global.u32   %r6, [%rd2+24];
        ld.global.u32   %r7, [%rd2+28];
        ld.global.u32   %r8, [%rd2+32];
        ld.global.u32   %r9, [%rd2+36];
        ld.global.u32   %r10, [%rd2+40];
        ld.global.u32   %r11, [%rd2+44];
        ld.global.u32   %r12, [%rd2+48];
        ld.global.u32   %r13, [%rd2+52];
        ld.global.u32   %r14, [%rd2+56];
        ld.global.u32   %r15, [%rd2+60];
        add.s32         %r16, %r15, 15;
        st.global.u32   [%rd2+60], %r16;
        add.s32         %r17, %r14, 15;
        st.global.u32   [%rd2+56], %r17;
        add.s32         %r18, %r13, 15;
        st.global.u32   [%rd2+52], %r18;
        add.s32         %r19, %r12, 15;
        st.global.u32   [%rd2+48], %r19;
        add.s32         %r20, %r11, 15;
        st.global.u32   [%rd2+44], %r20;
        add.s32         %r21, %r10, 15;
        st.global.u32   [%rd2+40], %r21;
        add.s32         %r22, %r9, 15;
        st.global.u32   [%rd2+36], %r22;
        add.s32         %r23, %r8, 15;
        st.global.u32   [%rd2+32], %r23;
        add.s32         %r24, %r7, 15;
        st.global.u32   [%rd2+28], %r24;
        add.s32         %r25, %r6, 15;
        st.global.u32   [%rd2+24], %r25;
        add.s32         %r26, %r5, 15;
        st.global.u32   [%rd2+20], %r26;
        add.s32         %r27, %r4, 15;
        st.global.u32   [%rd2+16], %r27;
        add.s32         %r28, %r3, 15;
        st.global.u32   [%rd2+12], %r28;
        add.s32         %r29, %r2, 15;
        st.global.u32   [%rd2+8], %r29;
        add.s32         %r30, %r1, 15;
        st.global.u32   [%rd2+4], %r30;
        ld.global.u32   %r31, [%rd2];
        add.s32         %r32, %r31, 15;
        st.global.u32   [%rd2], %r32;
        ret;
}
)";

// Generated by the following command:
//
// echo "__global__ void kernel(int* output) { *output = 42; }" |
//   nvcc -o - -rdc true --ptx --x cu -
//
constexpr const char kSimplePtx[] = R"(
.version 8.0
.target sm_52
.address_size 64

        // .globl       _Z6kernelPi

.visible .entry _Z6kernelPi (
        .param .u64 _Z6kernelPi_param_0
)
{
        .reg .b32       %r<16>;
        .reg .b64       %rd<3>;


        ld.param.u64    %rd1, [_Z6kernelPi_param_0];
        cvta.to.global.u64      %rd2, %rd1;
        mov.u32         %r1, 42;
        st.global.u32   [%rd2], %r15;
        ret;

})";

constexpr stream_executor::CudaComputeCapability kDefaultComputeCapability{5,
                                                                           2};

absl::StatusOr<std::vector<uint8_t>> CompileHelper(
    stream_executor::CudaComputeCapability cc, const char* const ptx_input,
    bool disable_gpuasm_optimizations = false, bool cancel_if_reg_spill = false,
    std::vector<std::string> extra_flags = {}) {
  stream_executor::GpuAsmOpts options(disable_gpuasm_optimizations,
                                      /*preferred_cuda_dir=*/"", extra_flags);

  return stream_executor::CompileGpuAsmUsingLibNvPtxCompiler(
      cc.major, cc.minor, ptx_input, options, cancel_if_reg_spill);
}

class PtxCompilerTest : public ::testing::Test {
  void SetUp() override {
    // This can't be in the constructor because `GTEST_SKIP` can't be called
    // from constructors.
    if (!stream_executor::IsLibNvPtxCompilerSupported()) {
      // We skip these tests if this is a build without libnvptxcompiler
      // support.
      GTEST_SKIP();
    }
  }
};

TEST_F(PtxCompilerTest, IdentifiesUnsupportedArchitecture) {
  EXPECT_THAT(
      CompileHelper(stream_executor::CudaComputeCapability{100, 0}, kSimplePtx),
      tsl::testing::StatusIs(absl::StatusCode::kUnimplemented));
}

TEST_F(PtxCompilerTest, CanCompileSingleCompilationUnit) {
  EXPECT_THAT(CompileHelper(kDefaultComputeCapability, kSimplePtx),
              tsl::testing::IsOk());
}

TEST_F(PtxCompilerTest, CancelsOnRegSpill) {
  // We have to disable optimization here, otherwise PTXAS will optimize our
  // trivial register usages away and we don't spill as intended.
  EXPECT_THAT(CompileHelper(kDefaultComputeCapability, kSpillingPtx,
                            /*disable_gpuasm_optimizations=*/true,
                            /*cancel_if_reg_spill=*/true),
              tsl::testing::StatusIs(absl::StatusCode::kCancelled));

  // We also test the converse to ensure our test case isn't broken.
  EXPECT_THAT(CompileHelper(kDefaultComputeCapability, kSpillingPtx,
                            /*disable_gpuasm_optimizations=*/true,
                            /*cancel_if_reg_spill=*/false),
              tsl::testing::IsOk());
}

TEST_F(PtxCompilerTest, AcceptsExtraArguments) {
  // It's tricky to test whether `extra_arguments` works without depending on
  // too much nvptx internals. So we pass the `--generate-line-info` flags and
  // expect strictly larger outputs than without the flag.
  auto reference_cubin = CompileHelper(kDefaultComputeCapability, kSimplePtx,
                                       /*disable_gpuasm_optimizations=*/false,
                                       /*cancel_if_reg_spill=*/false, {});
  auto cubin_with_line_info =
      CompileHelper(kDefaultComputeCapability, kSimplePtx,
                    /*disable_gpuasm_optimizations=*/false,
                    /*cancel_if_reg_spill=*/false, {"--generate-line-info"});

  EXPECT_THAT(reference_cubin, tsl::testing::IsOk());
  EXPECT_THAT(cubin_with_line_info, tsl::testing::IsOk());
  EXPECT_GT(cubin_with_line_info->size(), reference_cubin->size());

  // We also test whether invalid flags lead to a compilation error.
  EXPECT_THAT(
      CompileHelper(kDefaultComputeCapability, kSimplePtx,
                    /*disable_gpuasm_optimizations=*/false,
                    /*cancel_if_reg_spill=*/false, {"--flag-does-not-exist"}),
      tsl::testing::StatusIs(absl::StatusCode::kInternal));
}

TEST_F(PtxCompilerTest, ReturnsReasonableVersion) {
  constexpr stream_executor::SemanticVersion kMinSupportedVersion = {12, 0, 0};

  EXPECT_THAT(stream_executor::GetLibNvPtxCompilerVersion(),
              tsl::testing::IsOkAndHolds(testing::Ge(kMinSupportedVersion)));
}

}  // namespace
