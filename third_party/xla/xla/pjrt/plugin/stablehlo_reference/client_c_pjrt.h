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

#ifndef XLA_PJRT_PLUGIN_STABLEHLO_REFERENCE_CLIENT_C_PJRT_H_
#define XLA_PJRT_PLUGIN_STABLEHLO_REFERENCE_CLIENT_C_PJRT_H_

#include "xla/pjrt/c/pjrt_c_api.h"

extern "C" {

// Does not pass ownership of returned PJRT_Api* to caller.
const PJRT_Api* GetPjrtApi();
}

#endif  // XLA_PJRT_PLUGIN_STABLEHLO_REFERENCE_CLIENT_C_PJRT_H_
