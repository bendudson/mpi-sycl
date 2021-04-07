cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(mpi-sycl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# An option to select a SYCL runtime, or none
set(SYCL_RUNTIME "NONE" CACHE STRING "SYCL runtime implementation")
set_property(CACHE SYCL_RUNTIME PROPERTY STRINGS NONE HIPSYCL COMPUTECPP DPCPP)

if (NOT ${SYCL_RUNTIME} STREQUAL "NONE")

  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

  if (${SYCL_RUNTIME} STREQUAL "HIPSYCL")

    set(HIPSYCL_INSTALL_DIR "" CACHE STRING "HIPSYCL installation directory")

    if (NOT ${HIPSYCL_INSTALL_DIR})
      message(FATAL_ERROR "HIPSYCL_INSTALL_DIR is undefined")
    endif ()

    set(hipSYCL_DIR ${HIPSYCL_INSTALL_DIR}/lib/cmake)
    find_package(hipSYCL CONFIG REQUIRED)
    set(EXTRA_FLAGS -Wno-sign-compare -Wno-stringop-truncation)

  elseif (${SYCL_RUNTIME} STREQUAL "COMPUTECPP")

    set(ComputeCpp_DIR "" CACHE STRING "ComputeCpp installation directory")

    if (NOT ${ComputeCpp_DIR})
      message(FATAL_ERROR "ComputeCpp_DIR is undefined")
    endif ()

    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    set(COMPUTECPP_USER_FLAGS -O3 -fsycl-split-modules=20 -mllvm -inline-threshold=10000 -no-serial-memop)
    find_package(ComputeCpp REQUIRED)

  elseif (${SYCL_RUNTIME} STREQUAL "DPCPP")

    set(CMAKE_CXX_COMPILER "dpcpp")
    set(EXTRA_FLAGS -pedantic)
  endif ()
endif ()


option(MPI_AS_LIBRARY "Use MPI as a library rather than through mpi compiler wrapper" OFF)
if (MPI_AS_LIBRARY)

  set(MPI_C_LIB_DIR "" CACHE STRING "Typically <mpi_root_dir>/lib")
  set(MPI_C_INCLUDE_DIR "" CACHE STRING "Typically <mpi_root_dir>/include")
  set(MPI_C_LIB, "" CACHE STRING "For example: mpich for libmpich.so in MPI_C_LIB_DIR")

  message(STATUS "Using MPI as a library (${MPI_C_LIB})")
  message(STATUS "MPI include dir: ${MPI_C_INCLUDE_DIR}")
  message(STATUS "MPI library dir: ${MPI_C_LIB_DIR}")

  include_directories(${MPI_C_INCLUDE_DIR})
  link_directories(${MPI_C_LIB_DIR})
else ()
    find_package(MPI REQUIRED)
    set(MPI_C_LIB MPI::MPI_C)
endif ()

set(SOURCES
    src/mpi-sycl.cxx)

include_directories(src)
add_executable(mpi-sycl ${SOURCES})

target_compile_options(mpi-sycl
        PUBLIC
        -Wall
        -Wextra
        -Wcast-align
        -Wfatal-errors
        -Werror=return-type
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-ignored-attributes

        ${EXTRA_FLAGS}
        )

target_link_libraries(mpi-sycl PUBLIC ${MPI_C_LIB})
target_link_options(mpi-sycl PUBLIC LINKER:${CXX_EXTRA_LINKER_FLAGS})

if (${SYCL_RUNTIME} STREQUAL "HIPSYCL" OR ${SYCL_RUNTIME} STREQUAL "COMPUTECPP")
    add_sycl_to_target(
            TARGET mpi-sycl
            SOURCES ${SOURCES}) # must be the last
endif ()
