mod cpp_ffi;

pub use cpp_ffi::RustImage;
use image::{DynamicImage, GenericImageView};

/// Get image dimensions as (width, height)
pub fn image_dimensions(image: &DynamicImage) -> (u32, u32) {
    image.dimensions()
}

/// Rotate image 90 degrees clockwise
pub fn rotate_image_90(image: &DynamicImage) -> DynamicImage {
    image.rotate90()
}
