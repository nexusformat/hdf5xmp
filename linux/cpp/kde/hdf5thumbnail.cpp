#include "hdf5thumbnail.h"

#include <string>
#include <fstream>
#include <stdlib.h>
#include <bitset>
#include <netinet/in.h>
#include <iostream>

#include <QImage>

#define DATA_SET "thumb"

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

ThumbCreator::Flags Hdf5Creator::flags() const {
  return (Flags)(0);
}
