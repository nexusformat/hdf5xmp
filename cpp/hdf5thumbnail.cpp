#include "hdf5thumbnail.h"

#include <iostream>
#include <fstream>
#include <string>

#include <H5Cpp.h>

#include <QObject>
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


bool Hdf5Creator::create( const QString& path, int width, int height, QImage& img ) {

    // Open the file
    H5::H5File file(path.toLocal8Bit().constData(), H5F_ACC_TRUNC);

    // Gets the dataset with the thumbnail
    H5::DataSet dataset = file.openDataSet(DATA_SET);

    // Checks if the image attribute exists
    if(!dataset.attrExists(ATTRIBUTE)) {
        return false;
    }

    // Gets the base64 encoded image
    std::string imageBase64 = dataset.getComment(ATTRIBUTE);


    // Turns the base64 into a byteArray
    QByteArray base64Data = QByteArray::fromStdString(imageBase64);


    // Creates an image
    QImage image;

    // Turns the base64 into an image
    image.loadFromData(QByteArray::fromBase64(base64Data));

    img = image;


    file.close();

    return true;
}
