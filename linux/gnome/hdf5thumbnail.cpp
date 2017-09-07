#include <fstream>
#include <iostream>

#include "thumbnailerCore.h"

int main(int argc, char *argv[]) {

  if (argc <= 2) {
    std::cerr << "Error: Invalid number of arguments" << std::endl;
    return -1;
  }

  std::string path = argv[1];

  std::string binaryData = getThumbnail(path);

  if (binaryData.length() == 0) {
    std::cout << "No thumbnails Found in " << path;
    return -2;
  }

  // Write it to the file
  std::fstream imageFile(argv[2], std::ios::out | std::ios::binary);
  imageFile.write(binaryData.c_str(), binaryData.length());
  imageFile.close();

  return 0;
}
