licenses(["notice"])  # Apache 2

load(
    "//bazel:pagespeed_test.bzl",
    "pagespeed_cc_test",
    "pagespeed_cc_test_library",
)

proto_library(
    name = "property_cache_proto",
    srcs = [
        "property_cache.proto",
    ],
    deps = [
#        "//pagespeed/kernel/http:http_proto",
    ],
)

cc_proto_library(
    name = "property_cache_proto_cc",
    deps = [
        ":property_cache_proto",
    ],
)


cc_library(
    name = "http",
    srcs = [
        "abstract_property_store_get_callback.cc",
        "cache_property_store.cc",
        "fallback_property_page.cc",
        "property_cache.cc",
        "property_store.cc",
        "request_context.cc",
        "two_level_property_store.cc",
    ],
    hdrs = [
        "abstract_property_store_get_callback.h",
        "cache_property_store.h",
        "fallback_property_page.h",
        "property_cache.h",
        "property_store.h",
        "request_context.h",
        "two_level_property_store.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//pagespeed/kernel/cache",
        "//pagespeed/opt/logging",
        ":property_cache_proto_cc"
    ],
)

pagespeed_cc_test_library(
    name = "http_test_base",
    srcs = [
        "mock_property_page.cc",        
    ],
    hdrs = [
        "mock_property_page.h",
    ],
    deps = [
        ":http",
        "//pagespeed/kernel/base:kernel_test_util",
        "//pagespeed/kernel/base:pagespeed_gtest",
    ],
)


pagespeed_cc_test(
    name = "http_test",
    srcs = glob(["*_test.cc"]),
    deps = [":http_test_base"],
)
