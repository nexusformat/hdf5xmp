#include <iostream>
#include <fstream>
#include <string>

#include <H5Cpp.h>

#include <QImage>
#include <QTextStream>


#define DATA_SET "thumb"
#define ATTRIBUTE "image"


int main(int argc, char* argv[]) {

  std::string path = argv[1];
  
  H5::H5File file(path, H5F_ACC_RDONLY);

  // Gets the dataset with the thumbnail
  H5::DataSet dataset = file.openDataSet(DATA_SET);

  // Checks if the image attribute exists
  if(!dataset.attrExists(ATTRIBUTE))
    {
      std::cout << "Failure";
    }

  H5::Attribute attr = dataset.openAttribute(ATTRIBUTE);

  std::string imageBase64;

  std::cout << imageBase64 << std::endl;

  // Gets the base64 encoded image
  attr.read(attr.getDataType(), imageBase64);

  attr.close();
  // Turns the base64 into a byteArray
  QByteArray base64Data = QByteArray::fromStdString(imageBase64);


  // Creates an image
  QImage image;

  // Turns the base64 into an image
  image.loadFromData(QByteArray::fromBase64(base64Data));

  image.save(argv[2], "PNG");

  file.close();


}
