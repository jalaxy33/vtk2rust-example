#include <filesystem>
#include <iostream>
#include <vector>

#include "image_utils.h"
#include "mycrate/src/cpp_ffi.rs.h"
#include "path_utils.h"
#include "vtk_utils.h"

using namespace std;
using namespace filesystem;

using rust::Box;
using rust::Vec;

using mycrate::ImageInfo;
using mycrate::RustImage;

// -- vtkImageData <-> RustImage Conversion --------------------------------------

/// Convert `vtkImageData` to `RustImage`
Box<RustImage> vtk2rust(vtkImageData* vtk_image) {
    if (!vtk_image) {
        cerr << "Warning: vtk_image is null, returning empty RustImage" << endl;
        return mycrate::image_from_bytes(nullptr, 0, 0, 0);
    }

    int* dims = vtk_image->GetDimensions();
    int width = dims[0];
    int height = dims[1];
    int channels = vtk_image->GetNumberOfScalarComponents();

    const uint8_t* bytes = static_cast<const uint8_t*>(vtk_image->GetScalarPointer());

    // Copy to buffer and flip vertically to match Rust image coordinate system (top-left origin)
    size_t total_size = width * height * channels;
    vector<uint8_t> buffer(bytes, bytes + total_size);
    flip_vertical_inplace(buffer.data(), width, height, channels);

    return mycrate::image_from_bytes(buffer.data(), static_cast<uint32_t>(width),
                                     static_cast<uint32_t>(height),
                                     static_cast<uint32_t>(channels));
}

/// Convert `RustImage` to `vtkImageData`
Ptr<vtkImageData> rust2vtk(const RustImage& rs_image) {
    ImageInfo info = mycrate::get_image_info(rs_image);
    Vec<uint8_t> bytes = mycrate::image_to_bytes(rs_image);

    // Return nullptr if no image data
    if (bytes.empty()) {
        return nullptr;
    }

    // Flip bytes to match VTK coordinate system (bottom-left origin)
    flip_vertical_inplace(bytes.data(), info.width, info.height, info.channels);

    auto vtk_image = Ptr<vtkImageData>::New();
    vtk_image->SetDimensions(static_cast<int>(info.width), static_cast<int>(info.height), 1);
    vtk_image->AllocateScalars(VTK_UNSIGNED_CHAR, static_cast<int>(info.channels));

    std::memcpy(vtk_image->GetScalarPointer(), bytes.data(), bytes.size());
    return vtk_image;
}

// -- Utility Functions ------------------------------------------------

void display_vtk_image_info(const Ptr<vtkImageData>& image) {
    if (!image) {
        cerr << "  Error: Null image data." << endl;
        return;
    }

    int* dims = image->GetDimensions();
    int channels = image->GetNumberOfScalarComponents();
    cout << "  vtkImageData - Dimensions: " << dims[0] << "x" << dims[1]
         << ", Channels: " << channels << endl;
}

void display_rust_image_info(const Box<RustImage>& image) {
    ImageInfo info = mycrate::get_image_info(*image);
    cout << "  RustImage - Dimensions: " << info.width << "x" << info.height
         << ", Channels: " << info.channels << endl;
}

int main() {
    path project_root = PROJECT_ROOT;
    path image_dir = project_root / "assets/images";
    path save_dir = project_root / "results";

    assert_path_exists(project_root);
    assert_path_exists(image_dir);

    clean_and_create_dir(save_dir);

    vector<path> image_paths = list_image_paths(image_dir);
    cout << "Found " << image_paths.size() << " images in " << image_dir << endl;

    for (const auto& image_path : image_paths) {
        cout << "-----------------------------\n"
             << "Processing: " << image_path.filename() << endl;

        // Load image with VTK
        Ptr<vtkImageData> vtk_image = load_vtk_image(image_path);
        if (!vtk_image) {
            cerr << "  Failed to load image: " << image_path << endl;
            continue;
        }

        display_vtk_image_info(vtk_image);

        // Convert to Rust image
        Box<RustImage> rust_image = vtk2rust(vtk_image);
        display_rust_image_info(rust_image);

        // Rotate image in Rust
        Box<RustImage> rotated_image = mycrate::rotate90(*rust_image);
        cout << " After rotate90:" << endl;
        display_rust_image_info(rotated_image);

        // Convert back to VTK
        Ptr<vtkImageData> vtk_rotated = rust2vtk(*rotated_image);
        display_vtk_image_info(vtk_rotated);

        // Save rotated image with VTK
        path save_path = save_dir / (image_path.stem().string() + "-rotated.png");
        save_vtk_image(vtk_rotated, save_path);
        cout << " Saved rotated image to: " << save_path << endl;
    }

    cout << "\nProcessing complete. Results saved to: " << save_dir << endl;

    return 0;
}
