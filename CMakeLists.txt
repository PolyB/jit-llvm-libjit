cmake_minimum_required(VERSION 3.4.3)
project(test_jit CXX)

include(ExternalProject)

# prohibit in-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
                        "./bootstrap to run cmake in the build directory.\n"
                        "*** Do not forget to rm -rf CMakeCache.txt CMakeFiles")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -g")

set(SRC
  src/main.cc
  src/lj/Types.cc
  src/lj/Jitter.cc
  src/lj/JitFun.cc

  src/ll/Jitter.cc
  src/ll/JitFun.cc
  )


add_executable(test_jit ${SRC})

target_include_directories(test_jit PRIVATE 
  include/
  )

# --------
#  LIBJIT
# --------
set(LIBJIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libjit)

ExternalProject_Add(libjit_ext
  INSTALL_DIR ${LIBJIT_DIR}
  SOURCE_DIR ${LIBJIT_DIR}
  CONFIGURE_COMMAND ${LIBJIT_DIR}/configure --prefix=<INSTALL_DIR>
  BUILD_IN_SOURCE 1 # libjit is building source files, so we have to do that :/
  BUILD_COMMAND ${MAKE}
  BUILD_BYPRODUCTS ${LIBJIT_DIR}/jit/.libs/libjit.a
  INSTALL_COMMAND ""
  )

ExternalProject_Add_Step(libjit_ext
  bootstrap
  COMMAND ./bootstrap
  DEPENDEES download
  DEPENDERS configure
  WORKING_DIRECTORY ${LIBJIT_DIR}
  )

add_library(libjit STATIC IMPORTED GLOBAL)
set_property(TARGET libjit APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${LIBJIT_DIR}/include")
set_target_properties(libjit PROPERTIES IMPORTED_LOCATION ${LIBJIT_DIR}/jit/.libs/libjit.a)
add_dependencies(libjit libjit_ext)

target_link_libraries(test_jit libjit)

# ---------
#  PTHREAD
# ---------
find_package(Threads)
target_link_libraries(test_jit ${CMAKE_THREAD_LIBS_INIT})

# ------
#  LLVM
# ------
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs Core ExecutionEngine Interpreter Support nativecodegen mcjit)

target_link_libraries(test_jit ${llvm_libs})
