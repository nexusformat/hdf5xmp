#include "hdf5thumbnail.h"

#include <iostream>
#include <fstream>
#include <string>

#include <H5Cpp.h>

#include <QImage>
#include <QTextStream>

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
  
  // Open the file
  H5::H5File file(path.toLocal8Bit().constData(), H5F_ACC_RDONLY);
  
  // Gets the dataset with the thumbnail
  H5::DataSet dataset = file.openDataSet(DATA_SET);
  
  std::string dataString;
  
  dataset.read(dataString, dataset.getDataType());
  
  QByteArray data(dataString.c_str(), dataString.length());
   
  QImage image;
  
  // Turns the base64 into an image
  image.loadFromData(QByteArray::fromBase64(data));
  img = image.scaled(width, height);
  file.close();
  return true;
}

ThumbCreator::Flags Hdf5Creator::flags() const {
  return (Flags)(0);
}
