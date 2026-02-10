#pragma once

#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>

#include <filesystem>

using namespace std;
using namespace filesystem;

// Alias for VTK smart pointer
template <typename T>
using Ptr = vtkSmartPointer<T>;

/// Load `vtkImageData` from file path
inline Ptr<vtkImageData> load_vtk_image(const path& image_path) {
    if (!exists(image_path) || !is_regular_file(image_path)) {
        return nullptr;
    }

    auto reader = Ptr<vtkImageReader2>::Take(
        vtkImageReader2Factory::CreateImageReader2(image_path.string().c_str()));

    if (!reader) {
        cerr << "Error: No reader found for: " << image_path << endl;
        return nullptr;
    }

    reader->SetFileName(image_path.string().c_str());
    reader->Update();

    return reader->GetOutput();
}

inline void save_vtk_image(const Ptr<vtkImageData>& vtk_image, const path& save_path) {
    if (!vtk_image) {
        cerr << "Error: Cannot save null vtkImageData to: " << save_path << endl;
        return;
    }

    // Check if the path has .png extension
    if (save_path.extension() != ".png") {
        cerr << "Error: Save path must have .png extension: " << save_path << endl;
        return;
    }

    auto writer = Ptr<vtkPNGWriter>::New();
    writer->SetFileName(save_path.string().c_str());
    writer->SetInputData(vtk_image);
    writer->Write();
}
