#ifndef HDF5THUMBNAIL_CORE_H
#define HDF5THUMBNAIL_CORE_H

#include <string>

#define MAGIC_HDF 0x894844460d0a1a0a
#define XMP_OUR_MAGIC 0x89484D500d0a1a0a

#define HEAD_SIZE 16

#define HDF_FILE_ENDING ".hdf5"

#ifndef WIN32
#include <endian.h>
#else
#include <winsock.h>
#endif

#ifndef WIN32
#define get_correct_byteorder(x) be64toh(x)
#else
// If the system is big endian just return the value else swap it
#define get_correct_byteorder(x) htonl(47) == 47 ? x : _byteswap_uint64(x)
#endif

constexpr int HDF_FILE_ENDING_LENGTH = sizeof(HDF_FILE_ENDING) - 1;

// Regular expression for finding the xmp header and footer
const std::string XMP_HEADER_FOOTER_REGEX =
    "(<\\?xpacket begin=(\"|')(.?.?.?)(\"|') "
    "id=(\"|')W5M0MpCehiHzreSzNTczkc9d(\"|')\\?>)|"
    "(<\\?xpacket end=(\"|')(w|r)(\"|')\\?>)";

bool check_header(std::istream &stream, uint64_t position, uint64_t header);
uint64_t read_size(std::istream &stream, uint64_t position);
bool header_exists(std::istream &stream, uint64_t position);
std::string removeXMPHeaders(const char *data);

std::string readXmpFromSidecar(std::string path);
std::string readFromHdfFile(std::string path);
std::string readImageFromXMPBySize(std::ifstream &stream, long size);

std::string getThumbnail(std::string path);

#endif
