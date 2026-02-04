# VTK-Rust Image Processing Example

Demonstrates passing VTK image data (`vtkImageData`) from C++ to Rust for image processing using the [CXX](https://cxx.rs/) FFI library.


## Prerequisites

- Rust 1.70+ with Cargo
- CMake 3.10+
- VTK 9.x (with development headers)
- C++17 compatible compiler

## Building

### 1. Build Rust Library

```bash
cargo build --release
```

This generates the Rust library and CXX bridge headers in `target/`.

### 2. Build C++ Demo

```bash
cmake -B build/Release -DCMAKE_BUILD_TYPE=Release
cmake --build build/Release
```

## Running

```bash
./build/Release/demo  # Linux/macOS
./build/Release/demo.exe  # Windows
```

Expected output:
```
VTK loaded - Image size: 640x480
Rust FFI - Image size: 640x480
Converted back to VTK - Image size: 640x480
Rotated - Image size: 480x640
```

## API Overview

### Rust FFI Functions (exposed to C++)

| Function | Description |
|----------|-------------|
| `image_from_pixels()` | Create `RustImage` from raw pixel buffer |
| `image_info()` | Get image dimensions and channel count |
| `image_to_vec()` | Export image data as byte vector |
| `rotate90()` | Rotate image 90 degrees clockwise |

### C++ Helper Functions

| Function | Description |
|----------|-------------|
| `vtk_to_rust()` | Convert `vtkImageData` to `RustImage` |
| `rust_to_vtk()` | Convert `RustImage` back to `vtkImageData` |

## Project Structure

```
├── Cargo.toml          # Rust package configuration
├── CMakeLists.txt      # C++ build configuration
├── build.rs            # Rust build script (CXX bridge)
├── src/
│   ├── lib.rs          # Rust library interface
│   ├── main.rs         # Rust standalone example
│   └── cpp_ffi.rs      # CXX FFI bridge definitions
├── cpp_src/
│   └── main.cpp        # C++ VTK integration demo
└── assets/images/      # Sample images
```

## Dependencies

- [cxx](https://crates.io/crates/cxx) - Safe C++/Rust FFI
- [image](https://crates.io/crates/image) - Rust image processing
- [VTK](https://vtk.org/) - C++ visualization toolkit
