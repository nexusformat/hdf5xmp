#include <string>
#include <fstream>

#include <H5Cpp.h>

#define DATA_SET "thumb"



// Tested with 500 Files. On average each file took 0.015 seconds
// The tested files had thumbnails with a size of 3x3 which were resized to 256x256
// With caching it will load them instantly the next time
int main(int argc, char* argv[]) {

  std::string path = argv[1];

  H5::H5File file(path, H5F_ACC_RDONLY);

  // Gets the dataset with the thumbnail
  H5::DataSet dataset = file.openDataSet(DATA_SET);

  // Get image size and build char array of that size
  unsigned int size = dataset.getSpace().getSimpleExtentNpoints();
  char *imageData = new char[size];

  // read image into imageData
  dataset.read(imageData, dataset.getDataType());

  // write image to file
  std::fstream imageFile(argv[2], std::ios::out | std::ios::binary);
  imageFile.write(imageData, size);
  imageFile.close();

  // cleanup
  file.close();
  delete[] imageData;
}
