fn main() {
    println!("cargo:rerun-if-changed=build.rs");

    let _ = cxx_build::bridge("src/cpp_ffi.rs");
}
