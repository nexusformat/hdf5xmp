//
//  hdf5OSXImpl.hpp
//  HDF5 Thumbnailer
//
//  Created by Marc Schädeli on 21.09.17.
//  Copyright © 2017 Semafor AG. All rights reserved.
//

#ifndef hdf5OSXImpl_hpp
#define hdf5OSXImpl_hpp

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
    void getThumbnailOSX(const char* path, long length);
#ifdef __cplusplus
}
#endif
#endif /* hdf5OSXImpl_hpp */
