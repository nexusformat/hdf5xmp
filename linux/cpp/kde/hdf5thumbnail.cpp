#include "hdf5thumbnail.h"

#include <fstream>
#include <netinet/in.h>

#include <QImage>

// Defines the magic header value of different file-types
#define MAGIC_HDF  0x89484446
#define MAGIC_JPG  0xffd8ffd8
#define MAGIC_JFIF 0xffd8ffe0
#define MAGIC_EXIF 0xffd8ffe1
// This isn't the full magic numberbut there isn't any other starting like that anyways
#define MAGIC_GIF  0x47494638
#define MAGIC_PNG  0x89504e47

// Configuration for KDE plugin creation
extern "C" {
  Q_DECL_EXPORT ThumbCreator *new_creator() {
    return new Hdf5Creator;
  }

}


Hdf5Creator::Hdf5Creator() {
}

Hdf5Creator::~Hdf5Creator() {
}


bool Hdf5Creator::create( const QString& path, int width, int height, QImage& img ) {


  std::ifstream is (path.toStdString().c_str(), std::ifstream::binary);

  // Read the size of the image
  uint32_t size;
  
  is.seekg(0, std::ios::beg);
  is.read(reinterpret_cast<char *>(&size), 4);

  size = ntohl(size);

  if(size == MAGIC_HDF) {
    is.close();
    return false;
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

    // fill img with loaded image
    img.loadFromData((const uchar*)imageData, size, nullptr);

    // cleanup
    delete[] imageData;

    return true;
  }
  is.close();
  return false;
}

ThumbCreator::Flags Hdf5Creator::flags() const {
  return (Flags)(0);
}
