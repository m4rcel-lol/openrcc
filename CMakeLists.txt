cmake_minimum_required(VERSION 3.25)
project(OpenRCC VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(GNUInstallDirs)

include(cmake/deps.cmake)
include(cmake/distro.cmake)
openrcc_print_distro_summary()

enable_testing()

find_package(Threads REQUIRED)

set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/rcc_control.proto")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

set(PROTO_SRC "${GENERATED_DIR}/rcc_control.pb.cc")
set(PROTO_HDR "${GENERATED_DIR}/rcc_control.pb.h")
set(GRPC_SRC "${GENERATED_DIR}/rcc_control.grpc.pb.cc")
set(GRPC_HDR "${GENERATED_DIR}/rcc_control.grpc.pb.h")

add_custom_command(
    OUTPUT "${PROTO_SRC}" "${PROTO_HDR}" "${GRPC_SRC}" "${GRPC_HDR}"
    COMMAND $<TARGET_FILE:protobuf::protoc>
    ARGS
        --grpc_out "${GENERATED_DIR}"
        --cpp_out "${GENERATED_DIR}"
        -I "${CMAKE_CURRENT_SOURCE_DIR}/proto"
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "Generating protobuf and gRPC sources"
)

add_library(openrcc_proto STATIC
    "${PROTO_SRC}"
    "${GRPC_SRC}"
)

target_include_directories(openrcc_proto PUBLIC "${GENERATED_DIR}")
target_link_libraries(openrcc_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_library(openrcc_warnings INTERFACE)
option(OPENRCC_ENABLE_WERROR "Treat warnings as errors for OpenRCC targets" ON)
if(MSVC)
    target_compile_options(openrcc_warnings INTERFACE
        /W4
    )
    if(OPENRCC_ENABLE_WERROR)
        target_compile_options(openrcc_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(openrcc_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
    )
    if(OPENRCC_ENABLE_WERROR)
        target_compile_options(openrcc_warnings INTERFACE -Werror)
    endif()
endif()

add_library(openrcc_common STATIC
    src/common/config.cpp
    src/common/logging.cpp
)
target_include_directories(openrcc_common PUBLIC src)
target_link_libraries(openrcc_common PUBLIC spdlog::spdlog openrcc_warnings)

add_library(openrcc_job_manager STATIC
    src/job_manager/job_fsm.cpp
)
target_include_directories(openrcc_job_manager PUBLIC src)
target_link_libraries(openrcc_job_manager PUBLIC openrcc_common Threads::Threads openrcc_warnings)

add_library(openrcc_luau_runtime STATIC
    src/luau_runtime/luau_sandbox.cpp
)
target_include_directories(openrcc_luau_runtime PUBLIC src)
target_link_libraries(openrcc_luau_runtime PUBLIC openrcc_common openrcc_warnings)
if(TARGET Luau.VM)
    target_link_libraries(openrcc_luau_runtime PUBLIC Luau.VM)
endif()

add_library(openrcc_tutorial STATIC
    src/tutorial/tutorial_mode.cpp
)
target_include_directories(openrcc_tutorial PUBLIC src)
target_link_libraries(openrcc_tutorial PUBLIC openrcc_proto spdlog::spdlog openrcc_warnings)

add_library(openrcc_service_impl STATIC
    src/service/rcc_service_impl.cpp
)
target_include_directories(openrcc_service_impl PUBLIC src "${GENERATED_DIR}")
target_link_libraries(openrcc_service_impl PUBLIC
    openrcc_common
    openrcc_job_manager
    openrcc_luau_runtime
    openrcc_proto
    spdlog::spdlog
    openrcc_warnings
)

add_executable(openrcc
    src/service/main.cpp
)
target_include_directories(openrcc PRIVATE src "${GENERATED_DIR}")
target_link_libraries(openrcc PRIVATE
    openrcc_service_impl
    openrcc_tutorial
    gRPC::grpc++
    spdlog::spdlog
    Threads::Threads
    openrcc_warnings
)
if(TARGET systemd)
    target_link_libraries(openrcc PRIVATE systemd)
endif()

add_executable(openrcc-soap
    src/soap/main.cpp
)
target_include_directories(openrcc-soap PRIVATE src "${GENERATED_DIR}")
target_link_libraries(openrcc-soap PRIVATE
    openrcc_proto
    spdlog::spdlog
    gRPC::grpc++
    openrcc_warnings
)

if(WIN32)
    set_target_properties(openrcc PROPERTIES OUTPUT_NAME "RCCService")
    set_target_properties(openrcc-soap PROPERTIES OUTPUT_NAME "RCCSoap")
endif()

install(TARGETS openrcc openrcc-soap
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    install(FILES systemd/openrcc.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system
    )
endif()

add_executable(openrcc-tests
    test/job_fsm_test.cpp
    test/luau_sandbox_test.cpp
    test/soap_bridge_test.cpp
)
target_include_directories(openrcc-tests PRIVATE src "${GENERATED_DIR}")
target_link_libraries(openrcc-tests PRIVATE
    openrcc_job_manager
    openrcc_luau_runtime
    openrcc_common
    openrcc_proto
    GTest::gtest_main
    spdlog::spdlog
    gRPC::grpc++
    openrcc_warnings
)
include(GoogleTest)
gtest_discover_tests(openrcc-tests)

option(OPENRCC_BUILD_FUZZERS "Build fuzzing targets" OFF)
if(OPENRCC_BUILD_FUZZERS)
    add_executable(openrcc-soap-fuzz test/soap_parser_fuzz.cpp)
    target_include_directories(openrcc-soap-fuzz PRIVATE src)
    target_link_libraries(openrcc-soap-fuzz PRIVATE openrcc_warnings)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(openrcc-soap-fuzz PRIVATE -fsanitize=fuzzer,address)
        target_link_options(openrcc-soap-fuzz PRIVATE -fsanitize=fuzzer,address)
    else()
        message(FATAL_ERROR "Fuzzers require Clang (libFuzzer). Configure with -DCMAKE_CXX_COMPILER=clang++")
    endif()
endif()
