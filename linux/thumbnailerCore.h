#include <fstream>
#include <iostream>
#include <regex>
#include <netinet/in.h>
#include "tinyxml2.h"
#include "base64.h"

#define MAGIC_HDF 0x89484446

#define HDF_FILE_ENDING ".hdf5"

constexpr int FILE_END_LENGTH = sizeof(HDF_FILE_ENDING) - 1;


// Regular expression for finding the xmp header and footer
const std::string XMP_HEADER_FOOTER_REGEX = "(<\\?xpacket begin=(\"|')(.?.?.?)(\"|') "
  "id=(\"|')W5M0MpCehiHzreSzNTczkc9d(\"|')\\?>)|"
  "(<\\?xpacket end=(\"|')(w|r)(\"|')\\?>)";

std::string getThumbnail(std::string& path);
