#ifndef HDF5THUMBNAIL_CORE_H
#define HDF5THUMBNAIL_CORE_H

#include <string>

#define MAGIC_HDF 0x894844460d0a1a0a
#define XMP_OUR_MAGIC 0x89484D500d0a1a0a

#define HEAD_SIZE 16

#define HDF_FILE_ENDING ".hdf5"

constexpr int HDF_FILE_ENDING_LENGTH = sizeof(HDF_FILE_ENDING) - 1;

// Regular expression for finding the xmp header and footer
const std::string XMP_HEADER_FOOTER_REGEX =
    "(<\\?xpacket begin=(\"|')(.?.?.?)(\"|') "
    "id=(\"|')W5M0MpCehiHzreSzNTczkc9d(\"|')\\?>)|"
    "(<\\?xpacket end=(\"|')(w|r)(\"|')\\?>)";

std::string getThumbnail(std::string path);

#endif
