#include <string>
#include <fstream>
#include <stdlib.h>
#include <bitset>
#include <netinet/in.h>

int main(int argc, char* argv[]) {
  
  std::string path = argv[1];

  std::ifstream is (path.c_str(), std::ifstream::binary);

  // Read the size of the image
  uint32_t size;
  
  is.seekg(0, std::ios::beg);
  is.read(reinterpret_cast<char *>(&size), 4);

  size = ntohl(size);

  // Read the imageData
  char* imageData = new char[size];

  is.seekg(4, std::ios::beg);

  is.read(imageData, size);
  is.close();

  // write image to file
  std::fstream imageFile(argv[2], std::ios::out | std::ios::binary);
  imageFile.write(imageData, size);
  imageFile.close();

  delete[] imageData;
}
