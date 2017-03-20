#include <fstream>
#include <netinet/in.h>

// Defines the magic header value of different file-types
#define MAGIC_HDF  0x89484446
#define MAGIC_JPG  0xffd8ffd8
#define MAGIC_JFIF 0xffd8ffe0
#define MAGIC_EXIF 0xffd8ffe1
// This isn't the full magic numberbut there isn't any other starting like that anyways
#define MAGIC_GIF  0x47494638
#define MAGIC_PNG  0x89504e47

int main(int argc, char* argv[]) {
  
  std::string path = argv[1];

  std::ifstream is (path.c_str(), std::ifstream::binary);

  // Read the size of the image
  uint32_t size;
  
  is.seekg(0, std::ios::beg);
  is.read(reinterpret_cast<char *>(&size), 4);

  size = ntohl(size);

  // If the header of the files is the one of a HDF file then abort
  if(size == MAGIC_HDF) {
    is.close();
    return -1;
  }

  // Check for the image-headers
  // If the image-header is missing then it isn't a thumbnail or a unsupported image-format
  uint32_t imageHeader;

  is.seekg(4, std::ios::beg);
  is.read(reinterpret_cast<char *>(&imageHeader), 4);

  imageHeader = ntohl(imageHeader);

  // Check if the imageHeader is one of the approved headers
  if(imageHeader == MAGIC_JPG ||
     imageHeader == MAGIC_JFIF ||
     imageHeader == MAGIC_EXIF ||
     imageHeader == MAGIC_GIF ||
     imageHeader == MAGIC_PNG) {


    // Read the imageData
    char* imageData = new char[size];

    is.seekg(4, std::ios::beg);

    is.read(imageData, size);
    is.close();

    // write image to file
    std::fstream imageFile(argv[2], std::ios::out | std::ios::binary);
    imageFile.write(imageData, size);
    imageFile.close();

    delete[] imageData;

    return 0;
  }
  is.close();
  return -1;
}
