#include <iostream>
#include <fstream>
#include <string>

#include <H5Cpp.h>

#include <QImage>
#include <QTextStream>


#define DATA_SET "thumb"
#define ATTRIBUTE "image"



// Tested with 500 Files. On average each file took 0.0.12 Seconds for a thumbnail.
// The tested files had thumbnails with a size of 3x3
// With caching it will load them instantly the next time
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

  // Gets the base64 encoded image
  attr.read(attr.getDataType(), imageBase64);

  attr.close();
  // Turns the base64 into a byteArray
  QByteArray base64Data = QByteArray::fromStdString(imageBase64);


  // Creates an image
  QImage image;

  // Turns the base64 into an image
  image.loadFromData(QByteArray::fromBase64(base64Data));

  if(argc >= 4) {
    int scale = std::atoi(argv[3]);
    image = image.scaled(scale, scale);
  }  

  image.save(argv[2], "PNG");

  file.close();


}
