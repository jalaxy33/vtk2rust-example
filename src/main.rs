use std::path::PathBuf;

fn main() {
    let image_path = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("assets/images/bus.jpg");

    let img = image::open(&image_path).expect("Failed to open image");
    let (width, height) = mycrate::image_dimensions(&img);

    println!("Image dimensions: {}x{}", width, height);
}
