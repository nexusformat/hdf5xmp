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

std::string removeXMPHeaders(const char* data) {
  std::regex pattern(XMP_HEADER_FOOTER_REGEX);
  return std::regex_replace(data, pattern, "");
}

// Find the imageData in an XMLDocument
std::string findImageData(tinyxml2::XMLDocument &xmp) {
  // Get The first Element in the /x:xmpmeta/rdf:RDF tag
  // This is done with the safe assumption that the XMP file has that hirarchy
  tinyxml2::XMLElement* currentElement = xmp.RootElement()->FirstChildElement()->FirstChildElement();

  do {
    // Try to find the tag containing the image-data
    if(!currentElement->NoChildren()) {
      // If it has a tag with this name it should take the first element of the list it contains
      if(currentElement->FirstChildElement("xap:Thumbnails")) {
        tinyxml2::XMLElement* thumbnailsTag = currentElement->FirstChildElement("xap:Thumbnails");
        
        if(!thumbnailsTag->NoChildren()) {
          tinyxml2::XMLElement* thumbnailList = thumbnailsTag->FirstChildElement();
          
          if(!thumbnailList->NoChildren()) {
            tinyxml2::XMLElement* thumbnailTag = thumbnailList->FirstChildElement();
            
            if(thumbnailTag->FirstChildElement("xapGImg:image")) {
              // Decode the base64 image-data
              const char* base64Data = thumbnailTag->FirstChildElement("xapGImg:image")->GetText();
              return base64_decode(base64Data);
            }
          }
        }
      }
    }
    // Go to the next element
    currentElement = currentElement->NextSiblingElement();
  } while(currentElement);
  return "";
}

// Read an xmp sidecar file
std::string readXmpFromSidecar(std::string path) {
  
  // Check if the file ends with '.hdf5'
  if(path.rfind(HDF_FILE_ENDING) == path.length() - FILE_END_LENGTH) {
    // If yes. Remove the file ending
    path = path.substr(0, path.length() - FILE_END_LENGTH);
  }
  path += ".xmp";

  std::fstream xmpFile(path, std::ios::in | std::ios::ate);
  std::streamsize size = xmpFile.tellg();
  xmpFile.seekg(0);
  
  char* buffer = new char[size];
  xmpFile.read(buffer, size);
  xmpFile.close();

  std::string xml = removeXMPHeaders(buffer);
  delete[] buffer;


  tinyxml2::XMLDocument xmp;

  xmp.Parse(xml.c_str());

  if(xmp.Error()) {
    std::cerr << "Error: " << xmp.ErrorName() << std::endl;
    return "";
  }
  return findImageData(xmp);
}

std::string readFromHdfFile(std::string path) {
  
  std::ifstream file(path, std::ifstream::binary);
  file.seekg(0, std::ios::beg);

  // Check if the file has xmp data
  uint32_t header;
  file.read(reinterpret_cast<char *>(&header), 4);
  header = ntohl(header);

  if(header == MAGIC_HDF) {
    file.close();
    return "";
  }

  file.seekg(0, std::ios::beg);

  std::string data;
  std::getline(file, data, '\0');
  file.close();

  std::string xml = removeXMPHeaders(data.c_str());

  tinyxml2::XMLDocument xmp;

  xmp.Parse(xml.c_str());

  if(xmp.Error()) {
    std::cerr << "Error: " << xmp.ErrorName() << std::endl;
    return "";
  }
  return findImageData(xmp);
}

int main(int argc, char* argv[]) {

  if(argc <= 2) {
    std::cerr << "Error: Invalid number of arguments" << std::endl;
    return -1;
  }
  
  std::string path = argv[1];

  std::string binaryData = readFromHdfFile(path);

  if(binaryData.length() == 0) {
    binaryData = readXmpFromSidecar(path);
    if(binaryData.length() == 0) {
      std::cout << "No thumbnails Found";
      return -2;
    }
  }

  // Write it to the file
  std::fstream imageFile(argv[2], std::ios::out | std::ios::binary);
  imageFile.write(binaryData.c_str(), binaryData.length());
  imageFile.close();

  return 0;
}
