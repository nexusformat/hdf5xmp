#ifndef HDF5THUMBNAIL_H
#define HDF5THUMBNAIL_H

#include <kio/thumbcreator.h>

class Hdf5Creator : public ThumbCreator {

 public:
  Hdf5Creator();
  virtual ~Hdf5Creator();
  virtual bool create(const QString &path, int width, int height, QImage &img);
  virtual Flags flags() const;
};


#endif // HDF5THUMBNAIL_H
