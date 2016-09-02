#include <string>

#include <H5Cpp.h>

#include <ImageMagick-6/Magick++.h>

#define DATA_SET "thumb"



// Tested with 500 Files. On average each file took 0.015 seconds
// The tested files had thumbnails with a size of 3x3 which were resized to 256x256
// With caching it will load them instantly the next time
int main(int argc, char* argv[]) {

  std::string path = argv[1];
  
  H5::H5File file(path, H5F_ACC_RDONLY);

  // Gets the dataset with the thumbnail
  H5::DataSet dataset = file.openDataSet(DATA_SET);

  std::string dataString;
  
  dataset.read(dataString, dataset.getDataType());

  Magick::Blob blob;

  blob.base64(dataString);
  
  Magick::Image image;

  image.read(blob);

  // Checks the amount of arguments. If there are enough the image gets resized
  if(argc >= 4) {
    image.scale(argv[3]);
  }

  image.magick("PNG");

  image.write(argv[2]);

  file.close();


}
