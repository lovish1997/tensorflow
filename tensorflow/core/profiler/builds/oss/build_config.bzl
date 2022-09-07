# Platform-specific build configurations.
"""
TF profiler build macros for use in OSS.
"""

load("//tensorflow:tensorflow.bzl", "cc_header_only_library", "clean_dep")

def tf_profiler_alias(target_dir, name):
    return target_dir + "oss:" + name

def tf_profiler_pybind_cc_library_wrapper(name, actual, **kwargs):
    """Wrapper for cc_library used by tf_python_pybind_extension.

    This wrapper ensures that cc libraries headers are made available to pybind
    code, without creating ODR violations in the dynamically linked case.  The
    symbols in these deps symbols should be linked to, and exported by, the core
    pywrap_tensorflow_internal.so
    """
    cc_header_only_library(name = name, deps = [actual], **kwargs)

def tf_profiler_xla_proto_header():
    return [
        clean_dep("//tensorflow/compiler/xla/service:hlo_proto_cc_headers_only"),
    ]
