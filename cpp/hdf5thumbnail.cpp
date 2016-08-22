#include "hdf5thumbnail.h"
#include <QObject>

extern "C" {
    Q_DECL_EXPORT ThumbCreator *new_creator() {
        return new Hdf5Creator;
    }

}


bool Hdf5Creator::create( const QString& path, int width, int height, QImage& img ) {
    return true;
}
