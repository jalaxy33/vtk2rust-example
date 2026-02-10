use image::{DynamicImage, GenericImageView};

// FFI bridge between C++ (VTK) and Rust
#[cxx::bridge(namespace = "mycrate")]
mod ffi {
    // Image metadata shared with C++
    struct ImageInfo {
        width: u32,
        height: u32,
        channels: u32,
    }

    // Rust functions exposed to C++
    extern "Rust" {
        type RustImage;

        unsafe fn image_from_bytes(
            pixels: *const u8,
            width: u32,
            height: u32,
            channels: u32,
        ) -> Box<RustImage>;
        fn image_to_bytes(image: &RustImage) -> Vec<u8>;
        fn get_image_info(image: &RustImage) -> ImageInfo;
        fn is_image_empty(image: &RustImage) -> bool;

        fn rotate90(image: &RustImage) -> Box<RustImage>;
    }
}

use ffi::ImageInfo;

/// Wrapper for VTK image data processing in Rust
pub struct RustImage {
    inner: DynamicImage,
}

impl RustImage {
    fn new(inner: DynamicImage) -> Self {
        Self { inner }
    }

    /// Get image dimensions
    fn dimensions(&self) -> (u32, u32) {
        self.inner.dimensions()
    }

    /// Get number of channels
    fn channel_count(&self) -> u32 {
        match &self.inner {
            DynamicImage::ImageLuma8(_) => 1,
            DynamicImage::ImageRgb8(_) => 3,
            DynamicImage::ImageRgba8(_) => 4,
            _ => 3,
        }
    }

    /// Check if image is empty (width == 0 or height == 0).
    pub fn is_empty(&self) -> bool {
        self.inner.width() == 0 || self.inner.height() == 0
    }
}

/// Create a RustImage from raw pixel buffer.
/// `pixels` must be valid for `width * height * channels` bytes.
/// Supports 1 (grayscale), 3 (RGB), or 4 (RGBA) channels.
pub unsafe fn image_from_bytes(
    bytes: *const u8,
    width: u32,
    height: u32,
    channels: u32,
) -> Box<RustImage> {
    // Allow zero-size images as empty placeholders
    if width == 0 || height == 0 {
        return Box::new(RustImage::new(image::DynamicImage::new_rgba8(0, 0)));
    }
    assert!(!bytes.is_null(), "bytes pointer is null");
    assert!(width > 0 && height > 0, "Invalid image dimensions");

    let data_len = (width * height * channels) as usize;
    let pixel_data = unsafe { std::slice::from_raw_parts(bytes, data_len) };

    let dynamic_image = match channels {
        1 => image::GrayImage::from_raw(width, height, pixel_data.to_vec())
            .map(DynamicImage::ImageLuma8)
            .expect("Failed to create grayscale image"),
        3 => image::RgbImage::from_raw(width, height, pixel_data.to_vec())
            .map(DynamicImage::ImageRgb8)
            .expect("Failed to create RGB image"),
        4 => image::RgbaImage::from_raw(width, height, pixel_data.to_vec())
            .map(DynamicImage::ImageRgba8)
            .expect("Failed to create RGBA image"),
        _ => panic!("Unsupported channel count: {}", channels),
    };

    Box::new(RustImage::new(dynamic_image))
}

/// Extract raw pixel data as byte vector
pub fn image_to_bytes(image: &RustImage) -> Vec<u8> {
    // Return empty vec for empty image
    if image.is_empty() {
        return Vec::new();
    }
    match &image.inner {
        DynamicImage::ImageLuma8(img) => img.as_raw().clone(),
        DynamicImage::ImageRgb8(img) => img.as_raw().clone(),
        DynamicImage::ImageRgba8(img) => img.as_raw().clone(),
        _ => image.inner.to_rgb8().as_raw().clone(),
    }
}

/// Get image metadata
fn get_image_info(image: &RustImage) -> ImageInfo {
    let (w, h) = image.dimensions();
    ImageInfo {
        width: w,
        height: h,
        channels: image.channel_count(),
    }
}

/// Check if image is empty (width == 0 or height == 0).
/// Useful for checking if get_result_annotated returned valid data.
pub fn is_image_empty(image: &RustImage) -> bool {
    image.is_empty()
}

/// Rotate image 90 degrees clockwise
fn rotate90(image: &RustImage) -> Box<RustImage> {
    Box::new(RustImage::new(image.inner.rotate90()))
}
