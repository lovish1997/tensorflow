# Description:
#    LIBXSMM: Library for small matrix-matrix multiplications targeting Intel Architecture (x86).

licenses(["notice"])  # BSD 3-clause
exports_files(["LICENSE"])

# Arguments to ./scripts/libxsmm_interface.py, see that file for detailed description.
#  precision: SP & DP
#  ilp64: no
#  prefetch: 1 (auto)
libxsmm_interface_arguments = "0 0 1"

# Arguments to ./scripts/libxsmm_config.py, see that file for detailed description.
#  ilp64: no
#  offload: no
#  alignment [b]
#  prefetch: 1 (auto)
#  threshold: fallback to BLAS if n*m*k above this
#  synchronize: yes
#  jit: yes
#  flags
#  alpha = 1
#  beta = 1
libxsmm_config_arguments = "0 0 64 1 0 1 1 0 1 1"

genrule(
    name = "libxsmm_headers",
    srcs = [
        "src/template/libxsmm.h",
        "src/template/libxsmm_config.h",
    ],
    outs = [
        "include/libxsmm.h",
        "include/libxsmm_config.h",
    ],
    cmd = "$(location :libxsmm_interface) $(location src/template/libxsmm.h) " + libxsmm_interface_arguments + " > $(location include/libxsmm.h);" +
          "$(location :libxsmm_config) $(location src/template/libxsmm_config.h) " + libxsmm_config_arguments + " > $(location include/libxsmm_config.h)",
    tools = [
        ":libxsmm_config",
        ":libxsmm_interface",
    ],
)

cc_library(
    name = "xsmm_avx",
    srcs = [
        "src/libxsmm_main.c",
        "src/libxsmm_dump.c",
        "src/libxsmm_malloc.c",
        "src/libxsmm_gemm.c",
        "src/libxsmm_timer.c",
        "src/libxsmm_trace.c",
        "src/libxsmm_trans.c",
        "src/libxsmm_sync.c",
        "src/libxsmm_perf.c",
        "src/libxsmm_dnn.c",
        "src/libxsmm_dnn_convolution_forward.c",
        "src/libxsmm_cpuid_x86.c",
    ] + glob([
        "src/generator_*.c",
    ]),
    hdrs = [
        "include/libxsmm_dnn.h",
        "include/libxsmm_frontend.h",
        "include/libxsmm_generator.h",
        "include/libxsmm_macros.h",
        "include/libxsmm_malloc.h",
        "include/libxsmm_sync.h",
        "include/libxsmm_timer.h",
        "include/libxsmm_typedefs.h",
        "include/libxsmm_dispatch.h",
        "src/libxsmm_gemm_diff.c",
        "src/libxsmm_cpuid_x86.c",
        "src/libxsmm_hash.c",
        # Generated:
        "include/libxsmm.h",
        "include/libxsmm_config.h",
    ] + glob([
        "src/*.h",
        "src/template/*.c",
    ]),
    copts = [
        "-mavx",  # JIT does not work without avx anyway, and this silences some CRC32 warnings.
        "-Wno-vla",  # Libxsmm convolutions heavily use VLA.
    ],
    defines = [
        "LIBXSMM_BUILD",
        "LIBXSMM_CPUID_X86_NOINLINE",
        "__BLAS=0",
    ],
    includes = ["include"],
    linkopts = ["-ldl"],
    visibility = ["//visibility:public"],
    deps = [
        ":libxsmm_headers",
    ],
)

py_library(
    name = "libxsmm_scripts",
    srcs = glob(["scripts/*.py"]),
    data = ["version.txt"],
)

py_binary(
    name = "libxsmm_interface",
    srcs = ["scripts/libxsmm_interface.py"],
    deps = [":libxsmm_scripts"],
)

py_binary(
    name = "libxsmm_config",
    srcs = ["scripts/libxsmm_config.py"],
    deps = [":libxsmm_scripts"],
)
