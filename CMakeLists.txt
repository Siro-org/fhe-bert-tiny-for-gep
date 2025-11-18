cmake_minimum_required(VERSION 3.10)
project(FHE_BERT_Tiny)

# Указываем компилятор (Clang)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Собираем исходники
set(CONTROLLER_SOURCES
    src/FHEController.cpp
    src/FHEController.h
    src/Utils.h
)

set(CMAKE_CXX_STANDARD 17)
option( BUILD_STATIC "Set to ON to include static versions of the library" OFF)

find_package(OpenFHE)

set( CMAKE_CXX_FLAGS ${OpenFHE_CXX_FLAGS} )

# include_directories( ${OPENMP_INCLUDES} )
include_directories( ${OpenFHE_INCLUDE} )
include_directories( ${OpenFHE_INCLUDE}/third-party/include )
include_directories( ${OpenFHE_INCLUDE}/core )
include_directories( ${OpenFHE_INCLUDE}/pke )
include_directories( ${OpenFHE_INCLUDE}/binfhe )
### add directories for other OpenFHE modules as needed for your project

include_directories(include)

link_directories( ${OpenFHE_LIBDIR} )
link_directories( ${OPENMP_LIBRARIES} )
if(BUILD_STATIC)
    set( CMAKE_EXE_LINKER_FLAGS "${OpenFHE_EXE_LINKER_FLAGS} -static")
    link_libraries( ${OpenFHE_STATIC_LIBRARIES} )
else()
    set( CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS} )
    link_libraries( ${OpenFHE_SHARED_LIBRARIES} )
endif()

# Pipeline 1 - Encrypt BERT-Tiny Weights
add_executable(encrypt_weights
    src/encrypt_weights.cpp
    ${CONTROLLER_SOURCES}
)

# Pipeline 2 - Client Inference with Encrypted Weights (One file)
# add_executable(client_inference
#     src/client_inference.cpp
#     ${CONTROLLER_SOURCES}
# )


# Pipeline 3 - Client Inference with Encrypted Weights (Batch)
add_executable(client_inference_batch
    src/client_inference_batch.cpp
    ${CONTROLLER_SOURCES}
)

# Pipeline 4 - Benchmark Result Eval
add_executable(benchmark_eval
    src/benchmark_eval.cpp
    ${CONTROLLER_SOURCES}
)


# Unit tests / operations
#add_executable(test_operations
#    src/test_operations.cpp
#    ${CONTROLLER_SOURCES}
#)

# ============================================================================
# Build messages
# ============================================================================
message(STATUS "")
message(STATUS "Build Configuration:")
message(STATUS "  OpenFHE found: ${OpenFHE_FOUND}")
message(STATUS "  OpenFHE version: ${OpenFHE_VERSION}")
message(STATUS "  OpenFHE include: ${OpenFHE_INCLUDE}")
message(STATUS "  OpenFHE lib: ${OpenFHE_LIBDIR}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "")
message(STATUS "Targets to build:")
message(STATUS "  - test_operations (unit tests)")
message(STATUS "  - encrypt_weights (pipeline 1: key generation)")
message(STATUS "  - client_inference (pipeline 2: encrypted computation)")
message(STATUS "")
