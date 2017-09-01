#include <iostream>

#include "hdf5thumbnail.h"

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
  
  std::string binaryData = getThumbnail(path.toStdString());

  if(binaryData.length() == 0) {
    std::cout << "No thumbnails Found";
    return false;
  }

  img.loadFromData((const uchar*)binaryData.c_str(), binaryData.length(), nullptr);

  return true;
}

ThumbCreator::Flags Hdf5Creator::flags() const {
  return (Flags)(0);
}
