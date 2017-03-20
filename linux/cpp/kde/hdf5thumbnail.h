#ifndef HDF5THUMBNAIL_H
#define HDF5THUMBNAIL_H

#include <kio/thumbcreator.h>
#include <fstream>
#include <netinet/in.h>
#include <QImage>

// Defines the magic header value of different file-types
#define MAGIC_HDF  0x89484446
#define MAGIC_JPG  0xffd8ffd8
#define MAGIC_JFIF 0xffd8ffe0
#define MAGIC_EXIF 0xffd8ffe1
// This isn't the full magic numberbut there isn't any other starting like that anyways
#define MAGIC_GIF  0x47494638
#define MAGIC_PNG  0x89504e47


class Hdf5Creator : public ThumbCreator {

 public:
  Hdf5Creator();
  virtual ~Hdf5Creator();
  virtual bool create(const QString &path, int width, int height, QImage &img);
  virtual Flags flags() const;
};


#endif // HDF5THUMBNAIL_H
