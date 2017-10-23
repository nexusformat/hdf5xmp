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
    std::string::size_type pos = 0u;
    while((pos = path.find("%20", pos)) != std::string::npos) {
        path.replace(pos, 3, " ");
        pos += 1;
    }
    
    std::ofstream test("/tmp/path");
    test.write(path.c_str(), path.length());
    
    return std::string(getThumbnail(path));
}

#ifdef __cplusplus
extern "C" {
#endif
    void getThumbnailOSX(const char* path, long length) {
        
        std::string data = getData(path, length);
        std::ofstream test("/tmp/thumbnail.png", std::ios::binary);
        test.write(data.c_str(), data.length());
    }
#ifdef __cplusplus
}
#endif
