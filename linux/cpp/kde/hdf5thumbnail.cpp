#include "hdf5thumbnail.h"

#include <H5Cpp.h>

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

  // Open the file
  H5::H5File file(path.toLocal8Bit().constData(), H5F_ACC_RDONLY);

  // Gets the dataset with the thumbnail
  H5::DataSet dataset = file.openDataSet(DATA_SET);

  // Get image size and build char array of that size
  unsigned int size = dataset.getSpace().getSimpleExtentNpoints();
  uchar *imageData = new uchar[size];

  // read image into imageData
  dataset.read(imageData, dataset.getDataType());

  // fill img with loaded image
  img.loadFromData(imageData, size, "PNG");

  // cleanup
  file.close();
  delete[] imageData;

  return true;
}

ThumbCreator::Flags Hdf5Creator::flags() const {
  return (Flags)(0);
}
