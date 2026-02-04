use image::{DynamicImage, GenericImageView, GrayImage, RgbImage, RgbaImage};

// FFI bridge between C++ (VTK) and Rust
#[cxx::bridge]
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

        unsafe fn image_from_pixels(
            pixels: *const u8,
            width: u32,
            height: u32,
            channels: u32,
        ) -> Box<RustImage>;

        fn image_info(image: &RustImage) -> ImageInfo;
        fn image_to_vec(image: &RustImage) -> Vec<u8>;
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
}

/// Create RustImage from raw pixel buffer
///
/// # Safety
/// The `pixels` pointer must be valid and contain at least `width * height * channels` bytes
unsafe fn image_from_pixels(
    pixels: *const u8,
    width: u32,
    height: u32,
    channels: u32,
) -> Box<RustImage> {
    assert!(!pixels.is_null(), "Pixel pointer is null");
    assert!(width > 0 && height > 0, "Invalid image dimensions");

    let data_len = (width * height * channels) as usize;
    let pixel_data = unsafe { std::slice::from_raw_parts(pixels, data_len) };

    let dynamic_image = match channels {
        3 => RgbImage::from_raw(width, height, pixel_data.to_vec())
            .map(DynamicImage::ImageRgb8)
            .expect("Failed to create RGB image"),
        4 => RgbaImage::from_raw(width, height, pixel_data.to_vec())
            .map(DynamicImage::ImageRgba8)
            .expect("Failed to create RGBA image"),
        1 => GrayImage::from_raw(width, height, pixel_data.to_vec())
            .map(DynamicImage::ImageLuma8)
            .expect("Failed to create grayscale image"),
        _ => panic!("Unsupported channel count: {}", channels),
    };

    Box::new(RustImage::new(dynamic_image))
}

/// Get image metadata
fn image_info(image: &RustImage) -> ImageInfo {
    let (w, h) = image.dimensions();
    ImageInfo {
        width: w,
        height: h,
        channels: image.channel_count(),
    }
}

/// Extract raw pixel data as byte vector
fn image_to_vec(image: &RustImage) -> Vec<u8> {
    match &image.inner {
        DynamicImage::ImageLuma8(img) => img.as_raw().clone(),
        DynamicImage::ImageRgb8(img) => img.as_raw().clone(),
        DynamicImage::ImageRgba8(img) => img.as_raw().clone(),
        _ => image.inner.to_rgb8().as_raw().clone(),
    }
}

/// Rotate image 90 degrees clockwise
fn rotate90(image: &RustImage) -> Box<RustImage> {
    Box::new(RustImage::new(image.inner.rotate90()))
}
