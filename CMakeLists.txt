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

function(setup_rust_library targetname rustlibname)
    # Fix Windows MSVC runtime library compatibility
    if(WIN32 AND MSVC)
        target_compile_options(${targetname} PRIVATE /MD)
        target_compile_definitions(${targetname} PRIVATE
            $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>
        )
    endif()

    # Build Rust library if not present
    set(RUST_TARGET_DIR ${CMAKE_SOURCE_DIR}/target)
    set(rust_lib_dir ${RUST_TARGET_DIR}/release)

    if(NOT EXISTS ${rust_lib_dir})
        message(WARNING "Rust library not found. Building now...")
        execute_process(
            COMMAND cargo build --release
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()

    # Link Rust library
    set(rust_lib_path "${rust_lib_dir}/lib${rustlibname}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(WIN32)
        set(rust_lib_path "${rust_lib_path}.lib")
    endif()
    target_link_libraries(${targetname} PRIVATE "${rust_lib_path}")

    # Include CXX bridge headers
    file(GLOB cxxbridge_include_dirs
        "${RUST_TARGET_DIR}/cxxbridge/*/src"
        "${RUST_TARGET_DIR}/cxxbridge/*/src/*"
    )
    target_include_directories(${targetname} PRIVATE
        ${RUST_TARGET_DIR}/cxxbridge
        ${RUST_TARGET_DIR}/cxxbridge/rust
        ${cxxbridge_include_dirs}
    )

    # Add CXX bridge source files
    file(GLOB cxxbridge_src
        "${RUST_TARGET_DIR}/cxxbridge/*/src/*.rs.cpp"
        "${RUST_TARGET_DIR}/cxxbridge/*/src/*.rs.cc"
        "${RUST_TARGET_DIR}/cxxbridge/*/src/*/*.rs.cpp"
        "${RUST_TARGET_DIR}/cxxbridge/*/src/*/*.rs.cc"
    )
    if(cxxbridge_src)
        target_sources(${targetname} PRIVATE ${cxxbridge_src})
    endif()

    # Link CXX runtime library
    file(GLOB cxx_lib_files
        "${rust_lib_dir}/build/cxx-*/out/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
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
        )
    endif()
endfunction()

# =============================================================================
# VTK Configuration (Linux Example)
# =============================================================================

include_directories(/usr/include/vtk)

find_library(VTK_COMMON_CORE vtkCommonCore PATHS /usr/lib)
find_library(VTK_COMMON_DATA_MODEL vtkCommonDataModel PATHS /usr/lib)
find_library(VTK_COMMON_EXECUTION_MODEL vtkCommonExecutionModel PATHS /usr/lib)
find_library(VTK_SYS vtksys PATHS /usr/lib)
find_library(VTK_IO_IMAGE vtkIOImage PATHS /usr/lib)

set(VTK_LIBRARIES ${VTK_COMMON_CORE} ${VTK_COMMON_DATA_MODEL}
                  ${VTK_COMMON_EXECUTION_MODEL} ${VTK_SYS} ${VTK_IO_IMAGE})

# =============================================================================
# Target Configuration
# =============================================================================

set(targetname demo)
add_executable(${targetname} cpp_src/main.cpp)
setup_rust_library(${targetname} "mycrate")
target_link_libraries(${targetname} PRIVATE ${VTK_LIBRARIES})
