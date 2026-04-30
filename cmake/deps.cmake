include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for vendored dependencies" FORCE)
endif()

set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(gRPC_INSTALL OFF CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
set(utf8_range_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_TESTS OFF CACHE BOOL "" FORCE)

if(APPLE)
    set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "Use std::format for spdlog on Apple platforms" FORCE)
endif()

set(OPENRCC_GRPC_FETCHCONTENT_ARGS
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG v1.65.1
    GIT_SHALLOW TRUE
)

if(APPLE)
    list(APPEND OPENRCC_GRPC_FETCHCONTENT_ARGS
        PATCH_COMMAND
            ${CMAKE_COMMAND}
            -DOPENRCC_GRPC_SOURCE_DIR=<SOURCE_DIR>
            -P "${CMAKE_CURRENT_LIST_DIR}/patches/grpc-apple-silicon-compat.cmake"
    )
endif()

FetchContent_Declare(grpc ${OPENRCC_GRPC_FETCHCONTENT_ARGS})

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    luau
    GIT_REPOSITORY https://github.com/luau-lang/luau.git
    GIT_TAG 0.634
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(grpc spdlog luau googletest)

if(TARGET grpc++ AND NOT TARGET gRPC::grpc++)
    add_library(gRPC::grpc++ ALIAS grpc++)
endif()

if(TARGET grpc_cpp_plugin AND NOT TARGET gRPC::grpc_cpp_plugin)
    add_executable(gRPC::grpc_cpp_plugin ALIAS grpc_cpp_plugin)
endif()
