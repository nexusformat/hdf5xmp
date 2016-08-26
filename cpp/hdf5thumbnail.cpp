#include "hdf5thumbnail.h"

#include <iostream>
#include <fstream>
#include <string>

#include <H5Cpp.h>

#include <QImage>
#include <QTextStream>

#define DATA_SET "thumb"
#define ATTRIBUTE "image"

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
  
  // Checks if the image attribute exists
  if(!dataset.attrExists(ATTRIBUTE)) {
    std::cout << "Failure";
    return false;
  }
  
  H5::Attribute attr = dataset.openAttribute(ATTRIBUTE);

  std::string imageBase64;
  // Gets the base64 encoded image
  attr.read(attr.getDataType(), imageBase64);
  attr.close();
  // Turns the base64 into a byteArray
  QByteArray base64Data = QByteArray::fromStdString(imageBase64);

  // Creates an image
  QImage image;
  // Turns the base64 into an image
  image.loadFromData(QByteArray::fromBase64(base64Data));
  img = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  file.close();
  return true;
}

ThumbCreator::Flags Hdf5Creator::flags() const {
  return (Flags)(0);
}
