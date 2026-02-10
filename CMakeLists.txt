# ========================================================
# CMake Configuration for VTK-Rust Image Processing Example
# ========================================================

cmake_minimum_required(VERSION 3.10.0)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Project Configuration
# =============================================================================
project(vtk-example VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

# =============================================================================
# Rust Library Setup
# =============================================================================
function(setup_rust_library targetname cratename rustlibname)
    # Fix Windows MSVC runtime library compatibility
    if(WIN32 AND MSVC)
        target_compile_options(${targetname} PRIVATE /MD)
        target_compile_definitions(${targetname} PRIVATE
            $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>
        )
    endif()

    # Set Rust build directories
    set(RUST_TARGET_DIR ${CMAKE_SOURCE_DIR}/target)
    set(RUST_BUILD_TYPE release) # always use release build for compatibility
    set(RUST_LIB_DIR ${RUST_TARGET_DIR}/${RUST_BUILD_TYPE})

    # Ensure Rust library is built
    if(NOT EXISTS ${RUST_LIB_DIR})
        message(WARNING "Rust library not found. Building Rust library now...")
        execute_process(
            COMMAND cargo build --release
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()

    # Set Rust generated headers directory
    target_include_directories(${targetname} PRIVATE
        ${RUST_TARGET_DIR}/cxxbridge
        ${RUST_TARGET_DIR}/cxxbridge/rust
    )

    # Gather Rust generated source files
    file(GLOB rust_src
        "${RUST_TARGET_DIR}/cxxbridge/${cratename}/src/*.rs.cpp"
        "${RUST_TARGET_DIR}/cxxbridge/${cratename}/src/*.rs.cc"
    )

    if(rust_src)
        target_sources(${targetname} PRIVATE ${rust_src})
    endif()

    # Link Rust library
    set(rust_lib "${RUST_LIB_DIR}/lib${rustlibname}${CMAKE_SHARED_LIBRARY_SUFFIX}")

    if(WIN32)
        set(rust_lib "${rust_lib}.lib")
    endif()

    if(NOT EXISTS ${rust_lib})
        message(FATAL_ERROR "Rust library not found at ${rust_lib}. Please build the Rust library.")
    endif()

    target_link_libraries(${targetname} PRIVATE "${rust_lib}")

    # link cxx library
    file(GLOB cxx_lib_files
        "${RUST_LIB_DIR}/build/cxx-*/out/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    foreach(cxx_lib IN LISTS cxx_lib_files)
        target_link_libraries(${targetname} PRIVATE ${cxx_lib})
    endforeach()

    # Copy DLL on Windows
    if(WIN32)
        add_custom_command(TARGET ${targetname} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${rust_lib_files}"
            $<TARGET_FILE_DIR:${targetname}>
            COMMENT "Copying Rust DLL for $<CONFIG>."
        )
    endif()
endfunction()

# =============================================================================
# VTK Configuration
# =============================================================================
function(setup_vtk targetname)
    if(UNIX)
        # Linux specific VTK setup
        include_directories(/usr/include/vtk)
        find_library(VTK_COMMON_CORE vtkCommonCore PATHS /usr/lib)
        find_library(VTK_COMMON_DATA_MODEL vtkCommonDataModel PATHS /usr/lib)
        find_library(VTK_COMMON_EXECUTION_MODEL vtkCommonExecutionModel PATHS /usr/lib)
        find_library(VTK_SYS vtksys PATHS /usr/lib)
        find_library(VTK_IO_IMAGE vtkIOImage PATHS /usr/lib)

        set(VTK_LIBRARIES ${VTK_COMMON_CORE} ${VTK_COMMON_DATA_MODEL}
            ${VTK_COMMON_EXECUTION_MODEL} ${VTK_SYS} ${VTK_IO_IMAGE})

        target_link_libraries(${targetname} PRIVATE ${VTK_LIBRARIES})
    else()
        message(FATAL_ERROR "VTK setup is currently only implemented for Linux in this example.")
    endif()
endfunction()

# =============================================================================
# Common build Configuration
# =============================================================================
include_directories(cpp_headers)

# =============================================================================
# Target Configuration
# =============================================================================
set(targetname demo)
add_executable(${targetname} cpp_src/main.cpp)
setup_vtk(${targetname})
setup_rust_library(${targetname} "mycrate" "mycrate")

# =============================================================================
# Global Variables
# =============================================================================

# Current working directory
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})

# Make PROJECT_ROOT visible in C++ code
add_compile_definitions(PROJECT_ROOT="${PROJECT_ROOT}")
