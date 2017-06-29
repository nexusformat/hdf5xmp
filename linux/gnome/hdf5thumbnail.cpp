#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include "tinyxml2.h"
#include "base64.h"

#define HDF_FILE_ENDING ".hdf5"

constexpr int FILE_END_LENGTH = sizeof(HDF_FILE_ENDING) - 1;


int main(int argc, char* argv[]) {

  if(argc <= 2) {
    std::cerr << "Error: Invalid number of arguments" << std::endl;
    return -1;
  }
  
  std::string path = argv[1];

  // Check if the file ends with '.hdf5'
  if(path.rfind(HDF_FILE_ENDING) == path.length() - FILE_END_LENGTH) {
    // If yes. Remove the file ending
    path = path.substr(0, path.length() - FILE_END_LENGTH);
  }
  path += ".xmp";

  tinyxml2::XMLDocument xmp;

  xmp.LoadFile(path.c_str());

  if(xmp.Error()) {
    std::cerr << "Error: " << xmp.ErrorName() << std::endl;
    return xmp.ErrorID();
  }

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
              std::string binaryData = base64_decode(base64Data);

              // Write it to the file
              std::fstream imageFile(argv[2], std::ios::out | std::ios::binary);
              imageFile.write(binaryData.c_str(), binaryData.length());
              imageFile.close();

              return 0;
            }
          }
        }
      }
    }
    // Go to the next element
    currentElement = currentElement->NextSiblingElement();
  } while(currentElement);

  std::cerr << "Error: Image data not found" << std::endl;
  
  return -1;
}
