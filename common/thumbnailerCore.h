#ifndef HDF5THUMBNAIL_CORE_H
#define HDF5THUMBNAIL_CORE_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#ifndef WIN32
#include <netinet/in.h>
#endif
#include "base64.h"
#include "tinyxml2.h"

#define MAGIC_HDF 0x89484446
#define XMP_SIG_MAGIC "SIGXMP%"

#define HDF_FILE_ENDING ".hdf5"

constexpr int HDF_FILE_ENDING_LENGTH = sizeof(HDF_FILE_ENDING) - 1;
constexpr int XMP_SIG_MAGIC_LENGTH = sizeof(XMP_SIG_MAGIC) - 1;

// Regular expression for finding the xmp header and footer
const std::string XMP_HEADER_FOOTER_REGEX =
    "(<\\?xpacket begin=(\"|')(.?.?.?)(\"|') "
    "id=(\"|')W5M0MpCehiHzreSzNTczkc9d(\"|')\\?>)|"
    "(<\\?xpacket end=(\"|')(w|r)(\"|')\\?>)";

std::string getThumbnail(std::string path);

#endif
