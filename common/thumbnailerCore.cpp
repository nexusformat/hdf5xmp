#include <fstream>
#include <iostream>
#include <regex>
#include <vector>
#ifndef WIN32
#include <endian.h>
#else
#include <winsock.h>
#endif
#include "base64.h"
#include "tinyxml2.h"

#include "thumbnailerCore.h"

#ifndef WIN32
#define get_correct_byteorder(x) be64toh(x)
#else
// If the system is big endian just return the value else swap it
#define get_correct_byteorder(x) htonl(47) == 47 ? x : _byteswap_uint64(x)
#endif

bool check_header(std::ifstream &stream, uint64_t position, uint64_t header) {
  uint64_t buffer;
  stream.seekg(position);
  stream.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
  buffer = get_correct_byteorder(buffer);
  return buffer == header;
}

uint64_t read_size(std::ifstream &stream, uint64_t position) {
  uint64_t buffer;
  // Advance 8 bytes to skip the header
  stream.seekg(position + sizeof(XMP_OUR_MAGIC));
  stream.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
  return get_correct_byteorder(buffer);
}

bool header_exists(std::ifstream &stream, uint64_t position) {
  uint64_t buffer;
  stream.seekg(position);
  stream.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
  buffer = get_correct_byteorder(buffer);
  uint64_t mask = 0xffff0000ffffffff;
  return (buffer & mask) == (MAGIC_HDF & mask);
}

std::string removeXMPHeaders(const char *data) {
  std::regex pattern(XMP_HEADER_FOOTER_REGEX);
  return std::regex_replace(data, pattern, "");
}

// Find the imageData in an XMLDocument
std::string findImageData(tinyxml2::XMLDocument &xmp) {
  // Get The first Element in the /x:xmpmeta/rdf:RDF tag
  // This is done with the safe assumption that the XMP file has that hirarchy
  tinyxml2::XMLElement *currentElement =
      xmp.RootElement()->FirstChildElement()->FirstChildElement();
  for (; currentElement;
       currentElement = currentElement->NextSiblingElement()) {
    // Try to find the tag containing the image-data
    if (!currentElement->NoChildren()) {
      // If it has a tag with this name it should take the first element of the
      // list it contains
      if (currentElement->FirstChildElement("xap:Thumbnails")) {
        tinyxml2::XMLElement *thumbnailTag =
            currentElement->FirstChildElement("xap:Thumbnails");

        // Go to the next loop iteration if no children
        if (thumbnailTag->NoChildren())
          continue;

        thumbnailTag = thumbnailTag->FirstChildElement();
        if (thumbnailTag->NoChildren())
          continue;

        thumbnailTag = thumbnailTag->FirstChildElement();
        if (!thumbnailTag->FirstChildElement("xapGImg:image"))
          continue;

        // If it found the 'xapGImg:image' tag extract the data from it
        thumbnailTag = thumbnailTag->FirstChildElement("xapGImg:image");
        return base64_decode(thumbnailTag->GetText());
      }
    }
  }
  return "";
}

// Get the thumbnail out of an XMP block at the current stream position
// with a size of 'size'
std::string readImageFromXMPBySize(std::ifstream &stream, long size) {
  std::vector<char> buffer(size + 1);
  stream.read(buffer.data(), size);
  stream.close();

  std::string xml = removeXMPHeaders(buffer.data());

  tinyxml2::XMLDocument xmp;

  xmp.Parse(xml.c_str());

  if (xmp.Error()) {
    std::cerr << "Error: " << xmp.ErrorName() << std::endl;
    return "";
  }
  return findImageData(xmp);
}

// Read an xmp sidecar file
std::string readXmpFromSidecar(std::string path) {

  // Check if the file ends with '.hdf5'
  if (path.rfind(HDF_FILE_ENDING) == path.length() - HDF_FILE_ENDING_LENGTH) {
    // If yes. Remove the file ending
    path = path.substr(0, path.length() - HDF_FILE_ENDING_LENGTH);
  }
  path += ".xmp";

  std::ifstream xmpFile(path, std::ios::in | std::ios::ate);
  if (!xmpFile.good()) {
    return "";
  }
  std::streamsize size = xmpFile.tellg();
  xmpFile.seekg(0);

  return readImageFromXMPBySize(xmpFile, size);
}

std::string readFromHdfFile(std::string path) {

  std::ifstream file(path, std::ifstream::binary);
  file.seekg(0, std::ios::beg);

  // Check if the file has xmp data
  uint64_t header;
  file.read(reinterpret_cast<char *>(&header), sizeof(header));

  header = get_correct_byteorder(header);

  if (header == MAGIC_HDF) {
    file.close();
    return "";
  }

  uint64_t searchpos = 0;
  while (header_exists(file, searchpos) &&
         !check_header(file, searchpos, XMP_OUR_MAGIC)) {
    // 16 more because of the header
    searchpos += read_size(file, searchpos) + HEAD_SIZE;
  }

  // If at the position we stopped we find our header, read the data and return
  // it
  if (check_header(file, searchpos, XMP_OUR_MAGIC)) {
    uint64_t size = read_size(file, searchpos);
    file.seekg(searchpos + HEAD_SIZE);

    return readImageFromXMPBySize(file, size);
  } else {
    return "";
  }
}

std::string getThumbnail(std::string path) {
  std::string binaryData = readFromHdfFile(path);

  if (binaryData.length() == 0) {
    binaryData = readXmpFromSidecar(path);
  }
  return binaryData;
}
