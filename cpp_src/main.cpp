#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkSmartPointer.h>

#include <cstring>
#include <iostream>

#include "cpp_ffi.rs.h"

using namespace rust;

// Alias for VTK smart pointer
template<typename T>
using Ptr = vtkSmartPointer<T>;

// Convert VTK image data to Rust image
Box<RustImage> vtk_to_rust(vtkImageData* imageData) {
    int* dims = imageData->GetDimensions();
    int width = dims[0];
    int height = dims[1];
    int channels = imageData->GetNumberOfScalarComponents();

    unsigned char* pixels =
        static_cast<unsigned char*>(imageData->GetScalarPointer());

    return image_from_pixels(pixels, static_cast<uint32_t>(width),
                             static_cast<uint32_t>(height),
                             static_cast<uint32_t>(channels));
}

// Convert Rust image back to VTK image data
Ptr<vtkImageData> rust_to_vtk(const RustImage& image) {
    ImageInfo info = image_info(image);
    rust::Vec<uint8_t> pixels = image_to_vec(image);

    Ptr<vtkImageData> imageData = Ptr<vtkImageData>::New();
    imageData->SetDimensions(static_cast<int>(info.width),
                             static_cast<int>(info.height), 1);
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR,
                               static_cast<int>(info.channels));

    std::memcpy(imageData->GetScalarPointer(), pixels.data(), pixels.size());

    return imageData;
}

int main() {
    // Load image with VTK
    auto reader = Ptr<vtkJPEGReader>::New();
    reader->SetFileName("assets/images/bus.jpg");
    reader->Update();

    auto vtkImage = reader->GetOutput();
    std::cout << "VTK loaded - Image size: " << vtkImage->GetDimensions()[0]
              << "x" << vtkImage->GetDimensions()[1] << std::endl;

    // Convert to Rust image
    Box<RustImage> rustImage = vtk_to_rust(vtkImage);
    ImageInfo info = image_info(*rustImage);
    std::cout << "Rust FFI - Image size: " << info.width << "x" << info.height
              << std::endl;

    // Convert back to VTK
    auto backToVtk = rust_to_vtk(*rustImage);
    std::cout << "Converted back to VTK - Image size: "
              << backToVtk->GetDimensions()[0] << "x"
              << backToVtk->GetDimensions()[1] << std::endl;

    // Test rotation
    auto rotated = rotate90(*rustImage);
    ImageInfo rotatedInfo = image_info(*rotated);
    std::cout << "Rotated - Image size: " << rotatedInfo.width << "x"
              << rotatedInfo.height << std::endl;

    return 0;
}
