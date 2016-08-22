#ifndef HDF5THUMBNAIL_H
#define HDF5THUMBNAIL_H

#include <kio/thumbcreator.h>

class Hdf5Creator : public ThumbCreator {

    public:
        virtual bool create(const QString& path, int width, int height, QImage& img);
};


#endif // HDF5THUMBNAIL_H
