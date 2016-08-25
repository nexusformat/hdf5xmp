#include <iostream>
#include <fstream>
#include <string>

#include <H5Cpp.h>

#include <QImage>
#include <QTextStream>


#define DATA_SET "thumb"
#define ATTRIBUTE "image"

int main()
{
    QString path = "/home/scm/projects/hdf5Thumbnails/hdf5Files/PLV_160502001.hdf5";
// Open the file
    H5::H5File file(path.toLocal8Bit().constData(), H5F_ACC_RDONLY);

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

    image.save("/home/scm/image.png");


    file.close();

}
