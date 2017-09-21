//
//  hdf5OSXImpl.cpp
//  HDF5 Thumbnailer
//
//  Created by Marc Schädeli on 21.09.17.
//  Copyright © 2017 Semafor AG. All rights reserved.
//

#include "hdf5OSXImpl.hpp"

#include "thumbnailerCore.h"
#include <string>
#include <fstream>

std::string getData(const char* p, long l) {
    std::string path(p, l);
    if(path.find("file://") == 0) {
        path = path.substr(sizeof("file://") - 1);
    }
    
    return std::string(getThumbnail(path));
}

#ifdef __cplusplus
extern "C" {
#endif
    CGImageRef getThumbnailOSX(const char* path, long length) {
        
        std::string data = getData(path, length);
        
        CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL, data.c_str(), data.length(), NULL);
        
        CGImageRef img = CGImageCreateWithPNGDataProvider(dataProvider, NULL, false, kCGRenderingIntentDefault);
        CGDataProviderRelease(dataProvider);
        return img;
    }
#ifdef __cplusplus
}
#endif
